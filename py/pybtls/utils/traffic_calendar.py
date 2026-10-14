"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

import datetime
from typing import Iterable, Literal

import numpy as np

from ..lib.BTLS import Vehicle

__all__ = ["to_btls_calendar"]

# The BTLS calendar counts working days only, 25 to a month and 10 months to a
# year (CConfigData::Time, which CVehicle::getTime counts arrival times in).
_DAYS_PER_MT = 25
_DAYS_PER_YR = _DAYS_PER_MT * 10
_MON_BASE_YEAR = 2010


def _calendar_year(stored_year: int, traffic_format: int) -> int:
    """Calendar year of ``stored_year``, as the ``traffic_format`` reader stores it."""
    # 69 as the century break is the POSIX convention, the one Python's own
    # ``%y`` follows: a recording before 1969 or after 2068 needs its own handling.
    if traffic_format in (1, 2):  # CASTOR, BeDIT: two digits, 69 to 99 in the 1900s
        return stored_year + (1900 if stored_year >= 69 else 2000)
    if traffic_format == 3:  # DITIS: four digits
        return stored_year
    if traffic_format == 4:  # MON: counted from 2010
        return stored_year + _MON_BASE_YEAR
    if traffic_format == 5:  # SiWIM: counted from 2010 from then on, earlier in full
        return stored_year + _MON_BASE_YEAR if stored_year < 1900 else stored_year
    raise ValueError(f"traffic_format must be 1, 2, 3, 4 or 5, not {traffic_format}.")


def to_btls_calendar(
    vehicles: list[Vehicle],
    traffic_format: Literal[1, 2, 3, 4, 5],
    holidays: Iterable[datetime.date] = (),
) -> list[Vehicle]:
    """
    Renumber a recording dated by the real calendar into the BTLS calendar,
    keeping its working days only.

    BTLS simulates working days, counted 25 to a month and 10 months to a year,
    and replaying traffic requires its dates in that calendar. This drops the
    vehicles recorded on a Saturday, a Sunday or one of ``holidays``, and gives
    each remaining vehicle the BTLS date of its working day: the first working
    day of the recording becomes 1 January of its year, and every later working
    day the next BTLS day. Working days without any vehicle are counted too, so
    gaps in the recording are kept. Times of day are unchanged.

    Parameters
    ----------
    vehicles : list[Vehicle]\n
        The recording, as ``garage.read_garage_file`` reads it. Pass every file
        of one recording in one list: each call starts its own numbering.

    traffic_format : Literal[1, 2, 3, 4, 5]\n
        The format the vehicles were read from, which decides how their years
        are stored.\n
        1: CASTOR format (two-digit years, 69 to 99 read as the 1900s and 00 to
        68 as the 2000s, the POSIX convention that Python's ``%y`` follows).\n
        2: BEDIT format (two-digit years, as CASTOR).\n
        3: DITIS format.\n
        4: MON format.\n
        5: SiWIM CSV format.

    holidays : Iterable[datetime.date], optional\n
        Further non-working days to drop. The default is none.

    Returns
    -------
    list[Vehicle]\n
        Copies of the vehicles recorded on working days, in their original
        order, dated in the BTLS calendar.

    Raises
    ------
    ValueError\n
        If a vehicle's date is not a real calendar date.
    """

    properties = [vehicle._get_all_properties() for vehicle in vehicles]
    dates = []
    for p in properties:  # head, day, month, year, ...
        try:
            dates.append(
                datetime.date(_calendar_year(p[3], traffic_format), p[2], p[1])
            )
        except ValueError as e:
            message = f"Vehicle {p[0]} has no valid calendar date: {e}."
            raise ValueError(message) from None

    days = np.array(dates, dtype="datetime64[D]")
    holidays = np.array(list(holidays), dtype="datetime64[D]")
    working = np.is_busday(days, holidays=holidays)
    if not working.any():
        return []

    first = days[working].min()
    first_year = properties[int(np.flatnonzero(working & (days == first))[0])][3]
    # working days before each day since the first: its day number in BTLS time
    day_no = np.busday_count(first, days, holidays=holidays)

    btls_vehicles = []
    for p, is_working, n in zip(properties, working, day_no):
        if not is_working:
            continue
        n = int(n)
        p = list(p)
        p[3] = first_year + n // _DAYS_PER_YR
        p[2] = n % _DAYS_PER_YR // _DAYS_PER_MT + 1
        p[1] = n % _DAYS_PER_YR % _DAYS_PER_MT + 1
        vehicle = Vehicle(0)  # 0 does not matter
        vehicle._set_all_properties(tuple(p))
        btls_vehicles.append(vehicle)

    return btls_vehicles
