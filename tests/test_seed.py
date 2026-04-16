"""
Tests for the CRNGWrapper::seed() API.

Verifies:
1. Seeding produces reproducible output (same seed → same sequence).
2. Different seeds produce different output.
3. Unseeded (default) runs produce different output across spawned
   processes (since each spawned process re-seeds from std::random_device).
"""

import multiprocessing
import pytest
from pybtls.lib import libbtls


N_SAMPLES = 20  # number of RNG draws per test


def _draw_samples(n=N_SAMPLES):
    """Draw n uniform samples from the process-wide C++ RNG."""
    return [libbtls._sample_uniform() for _ in range(n)]


def _draw_seeded_in_subprocess(seed_val):
    """Spawned-process helper: seed, draw, return the samples."""
    from pybtls.lib import libbtls
    if seed_val is not None:
        libbtls.seed(seed_val)
    return [libbtls._sample_uniform() for _ in range(N_SAMPLES)]


# -----------------------------------------------------------------------
# API surface tests
# -----------------------------------------------------------------------

class TestSeedAPI:
    def test_seed_callable(self):
        libbtls.seed(42)

    def test_seed_accepts_large_values(self):
        libbtls.seed(2**63 - 1)

    def test_seed_zero(self):
        libbtls.seed(0)

    def test_seed_docstring(self):
        assert "reproducible" in libbtls.seed.__doc__.lower()


# -----------------------------------------------------------------------
# Reproducibility tests (same process)
# -----------------------------------------------------------------------

class TestSeedReproducibility:
    def test_same_seed_same_output(self):
        """seed(X) → draw N → seed(X) → draw N: both sequences must match."""
        libbtls.seed(42)
        seq_a = _draw_samples()

        libbtls.seed(42)
        seq_b = _draw_samples()

        assert seq_a == seq_b, "Same seed must produce identical sequences"

    def test_different_seed_different_output(self):
        """seed(X) and seed(Y) with X≠Y must produce different sequences."""
        libbtls.seed(42)
        seq_a = _draw_samples()

        libbtls.seed(99)
        seq_b = _draw_samples()

        assert seq_a != seq_b, "Different seeds must produce different sequences"

    def test_reseed_resets_stream(self):
        """Drawing after a reseed should restart the stream, not continue it."""
        libbtls.seed(123)
        first_val = libbtls._sample_uniform()
        # Draw a bunch more to advance the stream
        _draw_samples(100)
        # Reseed to the same value
        libbtls.seed(123)
        restarted_val = libbtls._sample_uniform()

        assert first_val == restarted_val, "Reseed must restart the stream"


# -----------------------------------------------------------------------
# Cross-process independence tests (multiprocessing with spawn)
# -----------------------------------------------------------------------

class TestSpawnedProcessIndependence:
    def test_unseeded_processes_differ(self):
        """Two spawned processes without explicit seed should produce
        different sequences (with overwhelming probability, since each
        gets a fresh std::random_device seed)."""
        ctx = multiprocessing.get_context("spawn")
        with ctx.Pool(2) as pool:
            results = pool.map(_draw_seeded_in_subprocess, [None, None])

        assert results[0] != results[1], (
            "Unseeded spawned processes should get different RNG streams"
        )

    def test_same_seed_across_processes(self):
        """Two spawned processes seeded with the same value must produce
        identical sequences (proves the seed propagates into the C++ RNG)."""
        ctx = multiprocessing.get_context("spawn")
        with ctx.Pool(2) as pool:
            results = pool.map(_draw_seeded_in_subprocess, [42, 42])

        assert results[0] == results[1], (
            "Same seed in different processes must produce identical sequences"
        )

    def test_different_seeds_across_processes(self):
        """Two spawned processes seeded with different values must produce
        different sequences."""
        ctx = multiprocessing.get_context("spawn")
        with ctx.Pool(2) as pool:
            results = pool.map(_draw_seeded_in_subprocess, [42, 99])

        assert results[0] != results[1], (
            "Different seeds in different processes must produce different sequences"
        )
