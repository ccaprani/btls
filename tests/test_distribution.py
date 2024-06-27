import pybtls
import numpy as np
import pytest


dist = pybtls.Distribution()
n = 5000
x = np.array([None] * n)
bins = int(n / 50)


@pytest.mark.parametrize("mean, std", [(0.0, 1.0), (1.0, 0.5), (5.0, 2.0)])
def test_normal(mean: float, std: float):
    for i in range(n):
        x[i] = dist.gen_normal(mean, std)
    assert np.allclose(np.mean(x), mean, atol=0.1)
    assert np.allclose(np.std(x), std, atol=0.1)


@pytest.mark.parametrize("mean, std", [(0.0, 1.0), (0.5, 2.0), (4.0, 3.0)])
def test_triangle(mean: float, std: float):
    for i in range(n):
        x[i] = dist.gen_triangular(mean, std)
    assert np.allclose(np.mean(x), mean, atol=0.1)
    assert np.allclose(np.std(x), 0.4 * std, atol=0.1)


@pytest.mark.parametrize("mean, std", [(0.0, 1.0), (20.0, 2.0), (30.0, 1.5)])
def test_MultiModalNormal(mean: float, std: float):
    mmn = pybtls.MultiModalNormal()
    mmn.add_mode(0.5, mean, std)
    mmn.add_mode(0.5, 10.0, std)
    for i in range(n):
        x[i] = dist.gen_multimodalnormal(mmn)
    assert np.allclose(np.mean(x), (mean + 10.0) / 2, atol=0.1)
