import numpy as np
import pybtls as pb


def test_normal_stats():
    dist = pb.Distribution()
    n = 1000000
    samples = np.empty(n)
    for i in range(n):
        samples[i] = dist.gen_normal(0.0, 1.0)
    mean = np.mean(samples)
    var = np.var(samples)
    # Expected: mean ~0.0 and variance ~1.0
    assert abs(mean - 0.0) < 0.1, f"Normal distribution mean: {mean}"
    assert abs(var - 1.0) < 0.1, f"Normal distribution variance: {var}"


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
    assert abs(mean - expected_mean) < 0.1, f"Triangular distribution mean: {mean}"
    assert abs(var - expected_var) < 0.1, f"Triangular distribution variance: {var}"


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
    assert abs(mean - 33.5) < 0.1, f"Multimodal mean: {mean}"
    assert abs(var - 157.55) < 0.1, f"Multimodal variance: {var}"
