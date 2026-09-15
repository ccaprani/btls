"""Shared helpers for the "no data rows" case across output/read readers.

Every reader in this package must return an empty ``pd.DataFrame`` with a
fixed set of columns when the pybtls output file it reads has no data rows.
Two idioms for this existed side by side before this module was extracted:

- readers built on ``pd.read_csv`` caught ``pd.errors.EmptyDataError`` and
  returned ``pd.DataFrame(columns=[...])``;
- readers that parse the file with a manual line loop checked
  ``if not data_rows:`` and returned the same kind of empty frame.

This module centralises both constructs (``empty_frame`` for idiom (b),
``read_csv_or_empty`` for idiom (a)) without changing what any reader
returns.

The columns declared for the empty result differ from reader to reader,
and that is preserved as-is: some readers infer part of their schema (the
number of load-effect columns, or which vehicle-classifier variant is in
use) from the data itself, which is not available when there are no data
rows to look at. For example, ``read_POT_C`` can only guarantee the
"Block" column when empty, since the number of "Effect N" columns depends
on data it doesn't have; ``read_TS`` still knows its full column set when
empty because its schema is chosen from the header line, which is read
before any data row is needed.
"""

from pathlib import Path

import pandas as pd

__all__ = ["empty_frame", "read_csv_or_empty"]


def empty_frame(columns) -> pd.DataFrame:
    """Return an empty DataFrame with the given columns.

    Equivalent to ``pd.DataFrame(columns=columns)``; exists only so every
    reader constructs its "no data rows" result the same way.
    """

    return pd.DataFrame(columns=columns)


def read_csv_or_empty(file_path: Path, columns, **read_csv_kwargs) -> pd.DataFrame:
    """Read a whitespace/CSV-style pybtls output file, or return an empty frame.

    Wraps ``pd.read_csv(file_path, **read_csv_kwargs)``, returning
    ``empty_frame(columns)`` instead of raising when the file has no data
    rows (``pd.errors.EmptyDataError``).
    """

    try:
        return pd.read_csv(file_path, **read_csv_kwargs)
    except pd.errors.EmptyDataError:
        return empty_frame(columns)
