import numpy as np
import pybtls as pb


def _rel_close(value, expected, rel_tol=1e-2, abs_tol=1e-1):
    diff = abs(value - expected)
    if abs(expected) <= abs_tol:
        return diff <= abs_tol
    return (diff / abs(expected)) <= rel_tol


def test_normal_stats():
    dist = pb.Distribution()
    n = 1000000
    samples = np.empty(n)
    for i in range(n):
        samples[i] = dist.gen_normal(0.0, 1.0)
    mean = np.mean(samples)
    var = np.var(samples)
    # Expected: mean ~0.0 and variance ~1.0
    assert _rel_close(mean, 0.0, rel_tol=1e-2, abs_tol=0.1), f"Normal distribution mean: {mean}"
    assert _rel_close(var, 1.0, rel_tol=1e-2, abs_tol=0.1), f"Normal distribution variance: {var}"


def test_triangle_stats():
    dist = pb.Distribution()
    n = 1000000
    samples = np.empty(n)
    a = -1.0
    b = 1.0
    c = 0.0
    for i in range(n):
        samples[i] = dist.gen_triangular(0.0, 1.0)
    mean = np.mean(samples)
    var = np.var(samples)
    expected_mean = (a + b + c) / 3
    expected_var = (a**2 + b**2 + c**2 - a * b - a * c - b * c) / 18
    # Use relative error except when expected is near zero (fallback to absolute)
    assert _rel_close(mean, expected_mean, rel_tol=1e-2, abs_tol=0.1), f"Triangular distribution mean: {mean}"
    assert _rel_close(var, expected_var, rel_tol=1e-2, abs_tol=0.1), f"Triangular distribution variance: {var}"


def test_multimodal_stats():
    dist = pb.Distribution()
    mmn = pb.MultiModalNormal()
    mmn.add_mode(0.4, 20, 4)
    mmn.add_mode(0.1, 30, 8)
    mmn.add_mode(0.5, 45, 3)
    n = 1000000
    samples = np.empty(n)
    for i in range(n):
        samples[i] = dist.gen_multimodalnormal(mmn)
    mean = np.mean(samples)
    var = np.var(samples)
    # For the multimodal distribution:
    # Expected mean = 0.4*20 + 0.1*30 + 0.5*45 = 33.5.
    # Expected E[X^2] = 0.4*(20^2+4^2) + 0.1*(30^2+8^2) + 0.5*(45^2+3^2) ≈ 1279.8,
    # so variance ≈ 1279.8 - (33.5)^2 = ~157.55.
    assert _rel_close(mean, 33.5, rel_tol=1e-2, abs_tol=0.1), f"Multimodal mean: {mean}"
    assert _rel_close(var, 157.55, rel_tol=1e-2, abs_tol=0.1), f"Multimodal variance: {var}"
