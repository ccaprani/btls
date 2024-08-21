from pybtls.utils import compress_discrete_IL
from typing import Literal
import pycba as cba
import pytest


@pytest.mark.parametrize("type", ["M", "V", "R"])
def test_compress_IL(type: Literal["M", "V", "R"]):
    bridge_span = 38.0

    bridge_string_base = ["P"] * 4
    bridge_string = str(bridge_span).join(bridge_string_base)
    (L, EI, R, eType) = cba.parse_beam_string(bridge_string)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=bridge_span * 3 / 100)
    position, ordinate = ils.get_il(bridge_span * 3 / 2, type)

    position_comp, ordinate_comp = compress_discrete_IL(position, ordinate, 0.01)

    assert pytest.approx(max(ordinate_comp), 0.01) == max(ordinate)
    assert pytest.approx(min(ordinate_comp), 0.01) == min(ordinate)
    assert pytest.approx(max(position_comp) - min(position_comp), 0.01) == max(
        position
    ) - min(position)
