"""
Shared check for the ``**kwargs`` entry points: a misspelled option must fail
loudly instead of silently falling back to its default.
"""


def reject_unknown_kwargs(func_name: str, kwargs: dict, allowed) -> None:
    unknown = sorted(set(kwargs) - set(allowed))
    if unknown:
        raise TypeError(f"{func_name}() got unexpected keyword argument(s): {unknown}")
