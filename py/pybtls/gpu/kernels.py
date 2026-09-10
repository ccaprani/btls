"""
Optional Triton fused kernel for the 1D load-effect hot path.

Triton is a dependency of the PyTorch CUDA wheels, so it is installed alongside
torch on Linux; this gives a fused interp + scatter-add kernel (one pass over
the pairs, no torch intermediate arrays) — measured ~2.4x over the torch
multi-pass on the scatter-add, and more across many load effects. On platforms
where Triton is absent the engine falls back to the torch path automatically.

The 1D influence line is resampled to a uniform grid on the host (see
``influence.resample_il``), so the kernel only needs a direct-index
interpolation.
"""


def triton_available() -> bool:
    if not _HAVE_TRITON:  # the kernels below fell back to the stubs
        return False
    try:
        import torch

        return bool(torch.cuda.is_available())
    except Exception:
        return False


try:
    import triton
    import triton.language as tl

    @triton.jit
    def _scatter_interp(
        sidx_ptr,
        pos_ptr,
        w_ptr,
        il_ptr,
        n_il,
        dx,
        weight,
        E_ptr,
        P,
        BLOCK: tl.constexpr,
    ):
        offs = tl.program_id(0) * BLOCK + tl.arange(0, BLOCK)
        mask = offs < P
        p = tl.load(pos_ptr + offs, mask=mask, other=0.0)
        ww = tl.load(w_ptr + offs, mask=mask, other=0.0)
        s = tl.load(sidx_ptr + offs, mask=mask, other=0)
        fidx = p / dx  # IL grid starts at x=0
        j = tl.floor(fidx).to(tl.int32)
        j = tl.maximum(0, tl.minimum(j, n_il - 2))
        frac = fidx - j.to(tl.float64)
        il0 = tl.load(il_ptr + j, mask=mask, other=0.0)
        il1 = tl.load(il_ptr + j + 1, mask=mask, other=0.0)
        ordn = il0 + frac * (il1 - il0)
        tl.atomic_add(E_ptr + s, ww * ordn * weight, mask=mask)

    def scatter_interp_1d(sidx, pos, w, il_grid, dx, weight, E_row):
        """Fused interp(IL_grid, pos)*w*weight -> atomic_add into E_row[sidx]."""
        P = sidx.shape[0]
        if P == 0:
            return
        BLOCK = 1024
        grid = (triton.cdiv(P, BLOCK),)
        _scatter_interp[grid](
            sidx, pos, w, il_grid, il_grid.shape[0], dx, weight, E_row, P, BLOCK=BLOCK
        )

    @triton.jit
    def _scatter_interp_surf(
        sidx_ptr,
        pos_ptr,
        yl_ptr,
        yr_ptr,
        w_ptr,
        Z_ptr,
        nx,
        ny,
        x0,
        dx,
        y0,
        dy,
        weight,
        E_ptr,
        P,
        BLOCK: tl.constexpr,
    ):
        offs = tl.program_id(0) * BLOCK + tl.arange(0, BLOCK)
        mask = offs < P
        p = tl.load(pos_ptr + offs, mask=mask, other=0.0)
        yl = tl.load(yl_ptr + offs, mask=mask, other=0.0)
        yr = tl.load(yr_ptr + offs, mask=mask, other=0.0)
        ww = tl.load(w_ptr + offs, mask=mask, other=0.0)
        s = tl.load(sidx_ptr + offs, mask=mask, other=0)

        xmax = x0 + (nx - 1) * dx
        ymax = y0 + (ny - 1) * dy
        # x interpolation indices are shared by both wheel tracks
        fx = (p - x0) / dx
        jx = tl.maximum(0, tl.minimum(tl.floor(fx).to(tl.int32), nx - 2))
        tx = fx - jx.to(tl.float64)
        base = jx * ny  # row-major: Z[i, j] = Z_ptr[i*ny + j]
        x_oob = (p < x0) | (p > xmax)

        # --- left wheel track ---
        fyl = (yl - y0) / dy
        jyl = tl.maximum(0, tl.minimum(tl.floor(fyl).to(tl.int32), ny - 2))
        tyl = fyl - jyl.to(tl.float64)
        ol0 = base + jyl
        cl0 = tl.load(Z_ptr + ol0, mask=mask, other=0.0)
        cl0 += tx * (tl.load(Z_ptr + ol0 + ny, mask=mask, other=0.0) - cl0)
        cl1 = tl.load(Z_ptr + ol0 + 1, mask=mask, other=0.0)
        cl1 += tx * (tl.load(Z_ptr + ol0 + 1 + ny, mask=mask, other=0.0) - cl1)
        vl = cl0 + tyl * (cl1 - cl0)
        vl = tl.where(x_oob | (yl < y0) | (yl > ymax), 0.0, vl)

        # --- right wheel track ---
        fyr = (yr - y0) / dy
        jyr = tl.maximum(0, tl.minimum(tl.floor(fyr).to(tl.int32), ny - 2))
        tyr = fyr - jyr.to(tl.float64)
        or0 = base + jyr
        cr0 = tl.load(Z_ptr + or0, mask=mask, other=0.0)
        cr0 += tx * (tl.load(Z_ptr + or0 + ny, mask=mask, other=0.0) - cr0)
        cr1 = tl.load(Z_ptr + or0 + 1, mask=mask, other=0.0)
        cr1 += tx * (tl.load(Z_ptr + or0 + 1 + ny, mask=mask, other=0.0) - cr1)
        vr = cr0 + tyr * (cr1 - cr0)
        vr = tl.where(x_oob | (yr < y0) | (yr > ymax), 0.0, vr)

        tl.atomic_add(E_ptr + s, ww * 0.5 * (vl + vr) * weight, mask=mask)

    def scatter_interp_2d(
        sidx, pos, yl, yr, w, Z, nx, ny, x0, dx, y0, dy, weight, E_row
    ):
        """Fused two-track bilinear surface interp -> atomic_add into E_row[sidx].

        ``Z`` is the row-major-flattened uniform [nx, ny] surface; ``yl`` / ``yr``
        are the per-pair left/right wheel-track transverse positions. One pass,
        no torch intermediates (mirrors the 1D :func:`scatter_interp_1d`)."""
        P = sidx.shape[0]
        if P == 0:
            return
        BLOCK = 512
        grid = (triton.cdiv(P, BLOCK),)
        _scatter_interp_surf[grid](
            sidx,
            pos,
            yl,
            yr,
            w,
            Z,
            nx,
            ny,
            x0,
            dx,
            y0,
            dy,
            weight,
            E_row,
            P,
            BLOCK=BLOCK,
        )

    _HAVE_TRITON = True

except Exception:  # pragma: no cover - triton ships with the torch CUDA wheels
    # not just ImportError: a broken Triton install raises RuntimeError /
    # FileNotFoundError / AttributeError out of its backend discovery, and the
    # torch fallback path is complete — never fail a run over it
    _HAVE_TRITON = False

    def scatter_interp_1d(*args, **kwargs):
        raise RuntimeError("Triton is not available")

    def scatter_interp_2d(*args, **kwargs):
        raise RuntimeError("Triton is not available")
