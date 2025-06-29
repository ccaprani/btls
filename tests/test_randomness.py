from pybtls import Distribution
import multiprocessing


def test_randomness():
    values1, values2 = [gen_random_values(i) for i in range(2)]
    assert values1 != values2


def test_randomness_when_multiprocessing():
    with multiprocessing.Pool(processes=2) as pool:
        values1, values2 = pool.map(gen_random_values, range(2))
    assert values1 != values2


def gen_random_values(process_id):
    dist = Distribution()
    return [dist.gen_normal(0.0, 1.0) for _ in range(100)]
