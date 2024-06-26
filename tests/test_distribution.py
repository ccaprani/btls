import pybtls
import matplotlib.pyplot as plt  # should remove for CI/CD pytest
import numpy as np


dist = pybtls.Distribution()
n = 5000
x = np.array([None] * n)
bins = int(n / 50)


def test_normal():
    for i in range(n):
        x[i] = dist.gen_normal(0.0, 1.0)

    plt.hist(x, bins=bins)
    plt.show()


def test_triangle():
    for i in range(n):
        x[i] = dist.gen_triangular(0.0, 1.0)

    plt.hist(x, bins=bins)
    plt.show()


def test_MultiModalNormal():
    mmn = pybtls.MultiModalNormal()
    mmn.add_mode(0.4, 20, 4)
    mmn.add_mode(0.1, 30, 8)
    mmn.add_mode(0.5, 45, 3)

    for i in range(n):
        x[i] = dist.gen_multimodalnormal(mmn)

    plt.hist(x, bins=bins)
    plt.show()
