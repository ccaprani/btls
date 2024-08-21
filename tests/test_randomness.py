from pybtls import Distribution
import multiprocessing
import pytest


def gen_randomness(arg: tuple):
    i = arg
    dist = Distribution()
    normal_values = []

    for _ in range(100):
        normal_values.append(dist.gen_normal(0.0, 1.0))

    return normal_values


def test_randomness():
    with multiprocessing.Pool(processes=2) as pool:
        gen_list = pool.map(gen_randomness, [(i,) for i in range(2)])

    # Check if the two returns in gen_list are different
    assert gen_list[0] != gen_list[1]
