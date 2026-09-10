"""
Influence-line evaluation helpers for the GPU engine: the built-in influence
line formulas (replicated from the C++ engine) and resampling of a 1D
influence line to a uniform grid for the fused Triton kernel.
"""

import numpy as np

SUPPORTED_BUILTIN_IDS = (1, 2, 3, 4, 5, 6, 7, 8, 9)


def builtin_ordinate(torch, il_id, L, x):
    """Built-in influence-line ordinate at positions ``x`` (a torch tensor),
    replicated from cpp/src/InfluenceLine.cpp getEquationOrdinate.

    IDs: 1 mid-span sagging BM (simply supported), 2 central-support hogging BM
    (2-span), 3/4 left/right shear (SS), 5/6 left/right shear (2-span), 7 total
    load, 8/9 2nd/3rd-support hogging BM (3-span). All return 0 outside [0, L].
    """
    zero = torch.zeros_like(x)

    if il_id == 1:  # mid-span sagging BM, simply supported
        ord = torch.where(x < L / 2.0, x / 2.0, (L - x) / 2.0)

    elif il_id == 3:  # left-hand shear, simply supported
        ord = 1.0 - x / L

    elif il_id == 4:  # right-hand shear, simply supported
        ord = x / L

    elif il_id == 7:  # total load on the span
        ord = torch.ones_like(x)

    elif il_id == 2:  # central-support hogging BM, 2-span continuous
        s = L / 2.0
        y = 2.0 * s - x
        ord = torch.where(
            x < s,
            x * (s * s - x * x) / (4.0 * s * s),
            y * (s * s - y * y) / (4.0 * s * s),
        )

    elif il_id == 5:  # left shear, 2-span continuous
        s = L / 2.0
        y = 2.0 * s - x
        left = (1.0 / s) * (-x * (s * s - x * x) / (4.0 * s * s) + (s - x))
        right = (1.0 / s) * (-y * (s * s - y * y) / (4.0 * s * s))
        ord = torch.where(x < s, left, right)

    elif il_id == 6:  # right shear, 2-span continuous
        s = L / 2.0
        y = 2.0 * s - x
        left = (1.0 / s) * (-x * (s * s - x * x) / (4.0 * s * s))
        right = (1.0 / s) * (-y * (s * s - y * y) / (4.0 * s * s) + (s - y))
        ord = torch.where(x < s, left, right)

    elif il_id == 8:  # 2nd-support hogging BM, 3-span continuous
        s = L / 3.0
        K = 15.0 * s * s
        u1 = x / s
        u2 = (x - s) / s
        u3 = (x - 2.0 * s) / s
        p1 = 2.0 * u1 * (1.0 - u1) * (1.0 + u1) * s * s * (2.0 * s) / K
        p2 = u2 * (1.0 - u2) * s * s * (3.0 * s * (1.0 - u2) + 2.0 * s * (2.0 - u2)) / K
        p3 = -u3 * (1.0 - u3) * (2.0 - u3) * s * s * s / K
        ord = torch.where(x < s, p1, torch.where(x < 2.0 * s, p2, p3))

    elif il_id == 9:  # 3rd-support hogging BM, 3-span continuous
        s = L / 3.0
        K = 15.0 * s * s
        u1 = x / s
        u2 = (x - s) / s
        u3 = (x - 2.0 * s) / s
        p1 = u1 * (1.0 - u1) * (1.0 + u1) * s * s * s / (-K)
        p2 = u2 * (1.0 - u2) * s * s * (3.0 * u2 * s + 2.0 * s * (1.0 + u2)) / K
        p3 = 2.0 * u3 * (1.0 - u3) * (2.0 - u3) * s * s * (2.0 * s) / K
        ord = torch.where(x < s, p1, torch.where(x < 2.0 * s, p2, p3))

    else:
        raise NotImplementedError(f"built-in influence line id {il_id} not supported")

    return torch.where((x < 0.0) | (x > L), zero, ord)


def resample_il(spec, x_grid):
    """Resample a 1D influence-line spec (discrete or built-in) onto the uniform
    ``x_grid`` (numpy array over [0, L]) for the fused Triton kernel. Exact for
    piecewise-linear discrete ILs; negligible error for built-in formulas."""
    if spec["kind"] == "discrete":
        g = np.interp(x_grid, spec["pos"], spec["ord"])
        g[(x_grid < spec["pos"][0]) | (x_grid > spec["pos"][-1])] = 0.0
        return g
    import torch

    return builtin_ordinate(
        torch, spec["id"], spec["length"], torch.as_tensor(x_grid, dtype=torch.float64)
    ).numpy()


def _is_uniform(a, rtol=1e-6):
    """True if ``a`` is strictly increasing with (near-)constant spacing."""
    if len(a) < 2:
        return False
    d = np.diff(a)
    return d[0] > 0 and bool(np.all(np.abs(d - d[0]) <= rtol * abs(d[0])))


def uniform_surface_grid(X, Y, Z):
    """If the influence surface is on a uniform (X, Y) grid, return
    ``(Z[nx,ny], x0, dx, y0, dy)`` for the fused Triton kernel (direct-index
    bilinear, exact); otherwise return ``None`` so the engine uses the torch
    ``searchsorted`` path, which interpolates the non-uniform grid exactly.
    Resampling a non-uniform grid onto a uniform one would smear nodes that the
    uniform grid cannot land on, so we never do it."""
    X = np.asarray(X, float)
    Y = np.asarray(Y, float)
    if not (_is_uniform(X) and _is_uniform(Y)):
        return None
    return (
        np.asarray(Z, float),
        float(X[0]),
        float(X[1] - X[0]),
        float(Y[0]),
        float(Y[1] - Y[0]),
    )
