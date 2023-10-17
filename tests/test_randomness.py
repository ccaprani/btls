from PyBTLS import Distribution
import multiprocessing


def gen_randomness(id:int):
    dist = Distribution()
    normal_values = []
    
    for _ in range(100):
        normal_values.append(dist.gen_normal(0.0, 1.0))

    print(str(id)+": ", normal_values)


def test_randomness():
    with multiprocessing.Pool(processes=5) as pool:
        pool.map(gen_randomness, range(4))


if __name__ == "__main__":
    test_randomness()