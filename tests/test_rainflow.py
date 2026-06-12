"""
Unit tests for the CRainflow streaming counter (via the _Rainflow binding):
ASTM E1049-85 reference counts, buffer-cadence independence (DR-13), and
the residue-splicing identity used by the chunk merge.
"""

import random

from pybtls.lib import libbtls


ASTM_SERIES = [-2.0, 1.0, -3.0, 5.0, -1.0, 3.0, -4.0, 4.0, -2.0]


def _count(series_parts, decimal=1, cutoff=0.0, final=True):
    counter = libbtls._Rainflow(decimal, cutoff)
    for part in series_parts[:-1]:
        counter.processData(part)
        counter.calcCycles(False)
    counter.processData(series_parts[-1])
    counter.calcCycles(final)
    return counter.getRainflowOutput()


def test_astm_e1049_reference_counts():
    # ASTM E1049-85 X3.1 example series: half cycles at ranges 3, 4, 8, 9,
    # 8, 6 and one full cycle at range 4. The implementation terminates the
    # series at zero load, which adds a trailing -2 -> 0 half cycle.
    out = _count([ASTM_SERIES])
    assert out == {
        2.0: 0.5,  # -2 -> 0 termination
        3.0: 0.5,
        4.0: 1.5,  # half (1 -> -3) + full (-1 -> 3)
        6.0: 0.5,
        8.0: 1.0,  # -3 -> 5 and -4 -> 4 halves
        9.0: 0.5,
    }


def test_output_independent_of_buffer_cadence():
    # DR-13: intermediate calcCycles(False) flushes must not change the
    # result - unclosed cycles are carried, not chopped.
    rng = random.Random(42)
    series = [rng.uniform(-100.0, 100.0) for _ in range(5000)]

    one_shot = _count([series])
    for n_parts in (2, 7, 50):
        size = len(series) // n_parts
        parts = [series[i * size : (i + 1) * size] for i in range(n_parts - 1)]
        parts.append(series[(n_parts - 1) * size :])
        assert _count(parts) == one_shot, f"cadence {n_parts} changed the output"


def test_residue_splicing_identity():
    # The chunk-merge identity: closed(S1) + closed(S2) + closure(res1 + res2)
    # equals the one-shot whole-series count.
    rng = random.Random(7)
    series = [rng.uniform(-50.0, 50.0) for _ in range(2000)]
    s1, s2 = series[:1100], series[1100:]

    totals = {}
    residuals = []
    for part in (s1, s2):
        counter = libbtls._Rainflow(1, 0.0)
        counter.processData(part)
        counter.calcCycles(False)  # closed cycles only
        for rng_, cnt in counter.getRainflowOutput().items():
            totals[rng_] = totals.get(rng_, 0.0) + cnt
        residuals.extend(counter.getResiduals())

    closure = libbtls._Rainflow(1, 0.0)
    closure.processData(residuals)
    closure.calcCycles(True)
    for rng_, cnt in closure.getRainflowOutput().items():
        totals[rng_] = totals.get(rng_, 0.0) + cnt

    assert totals == _count([series])


def test_cutoff_and_decimal_respected():
    out = _count([ASTM_SERIES], decimal=0, cutoff=4.0)
    assert all(amplitude >= 4.0 for amplitude in out)
    assert all(amplitude == round(amplitude) for amplitude in out)
