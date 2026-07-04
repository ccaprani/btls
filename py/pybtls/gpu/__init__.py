"""
Experimental GPU load-effect engine (Phase C).

A torch/Triton-based alternative to the C++ streaming engine, for the
compute-dominated regime (long span / many influence lines / fine time step).
It reconstructs axle trajectories from the traffic and computes per-effect
block maxima (BM), peaks-over-threshold (POT), fatigue rainflow (FR), flow
statistics (SS) and time history (TH) via per-vehicle superposition on the GPU,
under vertical / centrifugal / braking load-effect modes. POT and SS rebuild the
C++ event partition from each vehicle's on-bridge window (event geometry tracks
the CPU to ~1 %, the residual being composition changes that fall inside one
time step), and FR feeds device-extracted turning points to the same C++
rainflow counter (peak values / cycle amplitudes are grid-resolution).

Selected through ``Simulation.add_sim(..., engine="cuda")`` (which dispatches to
:func:`run`). PyTorch with CUDA is an optional dependency (``pybtls[gpu]``),
imported lazily only when this engine is used; Triton (installed with the torch
CUDA wheels) provides the fused kernel and is used automatically when present.
"""

from .engine import (
    compute_load_effect_maxima,
    compute_pot,
    is_available,
    GpuEngineError,
)
from .runner import run

__all__ = [
    "run",
    "is_available",
    "compute_load_effect_maxima",
    "compute_pot",
    "GpuEngineError",
]
