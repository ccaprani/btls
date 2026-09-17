"""
Recorded traffic: TrafficLoader lane handling, its speed overrides, the BTLS
calendar its dates must follow, and the window it is replayed in.
"""

import datetime
from pathlib import Path

import numpy as np
import pandas as pd
import pytest

import pybtls as pb
from utils import (
    loader_from_rows,
    make_4lane_bridge,
    make_il7_bridge,
    write_mon_traffic,
)

TRAFFIC_FILE = Path(__file__).parent / "test_data/test_traffic_file.txt"  # MON, 4 lanes


def test_traffic_loader_empty_lane_warns_and_continues():
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])
    vehicle.set_direction(1)
    vehicle.set_local_lane(1)
    vehicle.set_time(0.0)

    # Directly drive _get_traffic_loader() with lane 1 populated and lane 2
    # empty, bypassing add_traffic()'s file/list-derived lane-count logic
    # (which cannot itself represent a declared-but-empty lane).
    traffic_loader = pb.TrafficLoader(no_lane=2)
    traffic_loader._no_lane_dir_1 = 2
    traffic_loader._no_lane_dir_2 = 0
    traffic_loader._lanes_vehicles = [[vehicle], []]

    with pytest.warns(UserWarning):
        loader_list = traffic_loader._get_traffic_loader()

    assert len(loader_list) == 2


def test_traffic_loader_empty_lane_sim_still_processes_vehicles(tmp_path):
    """An initially-empty lane (next arrival stuck at 0.0) must be excluded
    from the merge loop instead of ending the whole simulation on the first
    iteration with zero vehicles processed."""
    from pathlib import Path

    src = Path(__file__).parent / "test_data/test_traffic_file.txt"
    vehicles = pb.garage.read_garage_file(garage_path=src, garage_format=4)
    # keep only dir-1 local-lane-2 vehicles: the file's max lane number stays
    # 2 (so add_traffic accepts no_lane=2) while global lane 1 has no vehicles
    lane2 = [v for v in vehicles if v.get_direction() == 1 and v.get_local_lane() == 2][
        :20
    ]
    traffic_file = tmp_path / "lane2_only.txt"
    pb.garage.write_garage_file(lane2, traffic_file, 4)

    loader = pb.TrafficLoader(no_lane=2)
    loader.add_traffic(
        traffic=traffic_file,
        traffic_format=4,
        use_average_speed=False,
        use_const_speed=True,
        const_speed_value=40.0,
    )

    il = pb.InfluenceLine(IL_type="built-in")
    il.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=il, inf_weight=[1.0, 1.0], threshold=0.0)

    output_config = pb.OutputConfig()
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="proof_traffic_file.txt",
    )

    sim = pb.Simulation(output_dir=tmp_path / "out")
    sim.add_sim(
        bridge=bridge,
        traffic=loader,
        output_config=output_config,
        time_step=0.5,
        min_gvw=35,
        tag="EmptyLane",
        track_progress=False,
    )
    sim.run(no_core=1)

    traffic_data = sim.get_output()["EmptyLane"].read_data("traffic")
    assert traffic_data, "no traffic output produced"
    df = next(iter(traffic_data.values()))
    # every vehicle read within the simulated window is written out
    assert len(df) == len(lane2), f"only {len(df)} of {len(lane2)} processed"


def _loaded_speeds(**speed_kwargs):
    loader = pb.TrafficLoader(no_lane=4)
    loader.add_traffic(traffic=TRAFFIC_FILE, traffic_format=4, **speed_kwargs)
    return np.array([v.get_velocity() for lane in loader._lanes_vehicles for v in lane])


def test_loader_use_average_speed_applies_file_mean_to_all_vehicles():
    recorded = _loaded_speeds()
    assert len(np.unique(recorded)) > 1  # the file really has varying speeds

    averaged = _loaded_speeds(use_average_speed=True)

    assert len(np.unique(averaged)) == 1
    assert averaged[0] == pytest.approx(recorded.mean())


def test_loader_use_const_speed_applies_given_value():
    speeds = _loaded_speeds(use_const_speed=True, const_speed_value=54.0)

    assert np.all(speeds == pytest.approx(54.0 / 3.6))


# --- the BTLS calendar -------------------------------------------------------


@pytest.mark.parametrize(
    "field, value",
    [(9, "26"), (11, "11")],  # the MON day (columns 10-11) and month (12-13)
    ids=["day_26", "month_11"],
)
def test_loader_rejects_a_date_outside_the_btls_calendar(tmp_path, field, value):
    # BTLS counts 25 days to a month and 10 months to a year, so a real calendar
    # date past either takes the time of a day in the next month or year and the
    # vehicle would replay out of order
    path = write_mon_traffic(tmp_path / "traffic.txt", 50, 2010)
    lines = path.read_text().splitlines()
    lines[10] = lines[10][:field] + value + lines[10][field + 2 :]
    path.write_text("\n".join(lines) + "\n")

    with pytest.raises(ValueError, match="outside the BTLS calendar"):
        pb.TrafficLoader(no_lane=4).add_traffic(traffic=path, traffic_format=4)
    # a garage does not use the dates: reading one is unaffected
    assert len(pb.garage.read_garage_file(path, garage_format=4)) == 50


def test_loader_rejects_a_siwim_date_outside_the_btls_calendar(tmp_path):
    # SiWIM records real timestamps, which are held to the same calendar
    rows = (Path(__file__).parent / "test_data/siwim.csv").read_text().splitlines()
    rows[3] = "2025-08-31" + rows[3][len("2025-08-02") :]
    path = tmp_path / "siwim.csv"
    path.write_text("\n".join(rows) + "\n")

    with pytest.raises(ValueError, match="day 31 of month 8"):
        pb.TrafficLoader(no_lane=2).add_traffic(traffic=path, traffic_format=5)


# --- renumbering real dates into the BTLS calendar ---------------------------


def _recorded(dates, stored_year=lambda year: year - 2010):
    """A copy of the first SiWIM test vehicle per (year, month, day, hour,
    minute), its year stored as ``stored_year`` has it (SiWIM's by default)."""
    template = pb.utils.vehicle_list_to_df(
        pb.garage.read_garage_file(
            Path(__file__).parent / "test_data/siwim.csv", garage_format=5
        )[:1]
    )
    df = pd.concat([template] * len(dates), ignore_index=True)
    for column, values in zip(("Year", "Month", "Day", "Hour", "Min"), zip(*dates)):
        df[column] = values
    df["Year"] = df["Year"].map(stored_year)
    df["Sec"] = 0.0
    return pb.utils.df_to_vehicle_list(df)


def _dates(vehicles):
    df = pb.utils.vehicle_list_to_df(vehicles)
    return list(zip(df["Year"], df["Month"], df["Day"], df["Hour"], df["Min"]))


def test_to_btls_calendar_numbers_the_working_days():
    # Friday 29 August 2025 is the first working day: BTLS 1 January 2025 (year
    # 15 as SiWIM stores it). The weekend and the Tuesday holiday are dropped,
    # and every later working day is the next BTLS day, recorded or not, so the
    # Monday a week on is day 6.
    recorded = _recorded(
        [
            (2025, 8, 29, 23, 59),  # Friday
            (2025, 8, 30, 12, 0),  # Saturday
            (2025, 8, 31, 12, 0),  # Sunday
            (2025, 9, 1, 0, 1),  # Monday
            (2025, 9, 2, 8, 0),  # Tuesday, the holiday
            (2025, 9, 3, 8, 0),  # Wednesday
            (2025, 9, 8, 8, 0),  # Monday
        ]
    )
    btls = pb.utils.to_btls_calendar(
        recorded, traffic_format=5, holidays=[datetime.date(2025, 9, 2)]
    )

    assert _dates(btls) == [
        (15, 1, 1, 23, 59),
        (15, 1, 2, 0, 1),
        (15, 1, 3, 8, 0),
        (15, 1, 6, 8, 0),
    ]
    assert _dates(recorded)[0] == (15, 8, 29, 23, 59)  # the input is untouched
    # Friday's last vehicle and Monday's first are now two minutes apart
    assert btls[1].get_time() - btls[0].get_time() == pytest.approx(120.0)
    pb.TrafficLoader(no_lane=1).add_traffic(traffic=btls)  # and can be replayed


@pytest.mark.parametrize(
    "traffic_format, year, stored_year",
    [
        (1, 2025, lambda year: year % 100),
        (1, 1998, lambda year: year % 100),  # two digits from 69 on: the 1900s
        (2, 2025, lambda year: year % 100),
        (3, 2025, lambda year: year),
        (4, 2025, lambda year: year - 2010),
        (5, 2025, lambda year: year - 2010),
    ],
    ids=["CASTOR", "CASTOR_1998", "BeDIT", "DITIS", "MON", "SiWIM"],
)
def test_to_btls_calendar_reads_the_year_as_the_format_stores_it(
    traffic_format, year, stored_year
):
    # the first Monday and Tuesday of August (Friday and Monday in 2025): read
    # in another century or from another base year they fall on other weekdays
    # (1 August 2015 was a Saturday, 3 August 2098 is a Sunday)
    first = (1, 4) if year == 2025 else (3, 4)
    recorded = _recorded([(year, 8, day, 8, 0) for day in first], stored_year)
    btls = pb.utils.to_btls_calendar(recorded, traffic_format)
    assert [d[:3] for d in _dates(btls)] == [
        (stored_year(year), 1, 1),
        (stored_year(year), 1, 2),
    ]


def test_to_btls_calendar_rolls_over_btls_months_and_years():
    # 25 working days make a BTLS month and 250 a year: five weeks after Monday
    # 6 January 2025 is BTLS 1 February, fifty weeks after is 1 January a year on
    recorded = _recorded(
        [(2025, 1, 6, 8, 0), (2025, 2, 10, 8, 0), (2025, 12, 22, 8, 0)]
    )
    btls = pb.utils.to_btls_calendar(recorded, traffic_format=5)
    assert [d[:3] for d in _dates(btls)] == [(15, 1, 1), (15, 2, 1), (16, 1, 1)]


def test_to_btls_calendar_rejects_a_date_the_real_calendar_does_not_have():
    with pytest.raises(ValueError, match="no valid calendar date"):
        pb.utils.to_btls_calendar(_recorded([(2025, 4, 31, 8, 0)]), traffic_format=5)


# --- the replay window -------------------------------------------------------


def _replay(tmp_path, tag, path):
    loader = pb.TrafficLoader(no_lane=4)
    loader.add_traffic(traffic=path, traffic_format=4)
    cfg = pb.OutputConfig()
    cfg.set_vehicle_file_output(write_vehicle_file=True, vehicle_file_format=4)
    cfg.set_BM_output(write_summary=True)
    cfg.set_POT_output(write_summary=True, write_counter=True)
    sim = pb.Simulation(output_dir=tmp_path / tag)
    sim.add_sim(
        bridge=make_4lane_bridge(),
        traffic=loader,
        output_config=cfg,
        time_step=0.1,
        min_gvw=35,
        tag=tag,
        track_progress=False,
    )
    sim.run(no_core=1)
    return loader, sim.get_output()[tag]


def _only(output, key):
    (df,) = output.read_data(key).values()
    return df


def test_loader_replays_traffic_dated_after_day_zero(tmp_path):
    # The same records dated 2019 start on BTLS day 2250. The replay starts at
    # midnight of that day and keeps the dates, so every vehicle is simulated and
    # only the absolute times move. (It used to run from t=0 to the file's number
    # of days and simulate nothing.) Values agree up to the rounding of the
    # larger times.
    n = 400
    day0_file = write_mon_traffic(tmp_path / "y2010.txt", n, 2010)
    later_file = write_mon_traffic(tmp_path / "y2019.txt", n, 2019)
    _, day0 = _replay(tmp_path, "y2010", day0_file)
    loader, later = _replay(tmp_path, "y2019", later_file)
    assert loader.start_time == 2250 * 86400.0

    vehicles = _only(later, "traffic")
    assert len(vehicles) == n
    assert (vehicles["Year"] == 2019 - 2010).all()  # dates as read

    np.testing.assert_allclose(
        _only(later, "BM_summary").values,
        _only(day0, "BM_summary").values,
        atol=0.11,
    )
    pd.testing.assert_frame_equal(
        _only(later, "POT_counter"), _only(day0, "POT_counter")
    )
    pot0, pot9 = _only(day0, "POT_summary"), _only(later, "POT_summary")
    assert len(pot9) == len(pot0) > 0
    np.testing.assert_allclose(pot9["Time"] - 2250 * 86400.0, pot0["Time"], atol=0.11)
    np.testing.assert_allclose(pot9["Peak Value"], pot0["Peak Value"], atol=0.11)


def test_loader_replays_a_last_day_arriving_earlier_than_the_first(tmp_path):
    # Days count from midnight of the first day. Counted from the first arrival
    # (day 0 23:00), the arrival on day 2 at 01:00 fell in a day too few and past
    # the replay window.
    hour = 3600.0
    loader = loader_from_rows(
        [(23 * hour, 100.0, 20.0), (30 * hour, 100.0, 20.0), (49 * hour, 100.0, 20.0)]
    )
    assert loader.sim_day == 3

    cfg = pb.OutputConfig()
    cfg.set_vehicle_file_output(write_vehicle_file=True, vehicle_file_format=4)
    sim = pb.Simulation(output_dir=tmp_path)
    sim.add_sim(
        bridge=make_il7_bridge(),
        traffic=loader,
        output_config=cfg,
        time_step=0.1,
        min_gvw=10,
        tag="last_day",
        track_progress=False,
    )
    sim.run(no_core=1)
    assert len(_only(sim.get_output()["last_day"], "traffic")) == 3
