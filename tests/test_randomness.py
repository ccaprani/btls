from pybtls import Distribution
import multiprocessing
import pytest


def test_randomness():
    values1, values2 = [gen_random_values(i) for i in range(2)]
    assert values1 != values2


@pytest.mark.parametrize("start_method", multiprocessing.get_all_start_methods())
def test_randomness_when_multiprocessing(start_method):
    # a forked worker copies the parent's generator state unless it is reseeded
    with multiprocessing.get_context(start_method).Pool(processes=2) as pool:
        values1, values2 = pool.map(gen_random_values, range(2))
    assert values1 != values2


def gen_random_values(process_id):
    dist = Distribution()
    return [dist.gen_normal(0.0, 1.0) for _ in range(100)]
