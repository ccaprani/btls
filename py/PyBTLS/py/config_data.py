from PyBTLS.cpp import ConfigDataCore


def singleton(cls):
    _instance = {}

    def inner():
        if cls not in _instance:
            _instance[cls] = cls()
        return _instance[cls]

    return inner


@singleton
class ConfigData(ConfigDataCore):
    pass
