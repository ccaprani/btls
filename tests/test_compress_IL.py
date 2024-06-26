from pybtls.lib.utils import _compress_discrete_IL
import pycba as cba
import matplotlib.pyplot as plt


# For now, this just provides a general idea. It will not be the final test_func. (x, y, e)
def test_compress_IL():
    bridge_span = 38.0

    bridge_string_base = ["P"] * 4
    bridge_string = str(bridge_span).join(bridge_string_base)
    (L, EI, R, eType) = cba.parse_beam_string(bridge_string)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=bridge_span * 3 / 100)
    position, ordinate = ils.get_il(bridge_span * 3 / 2, "M")

    position_comp, ordinate_comp = _compress_discrete_IL(position, ordinate, 0.01)

    plt.figure()
    plt.plot(position, ordinate, "b-")
    plt.plot(position_comp, ordinate_comp, "r-")
    plt.show()

    print(len(position), len(position_comp))
