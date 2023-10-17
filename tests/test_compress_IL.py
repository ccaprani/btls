from PyBTLS.lib.utils import _compress_discrete_IL
import pycba as cba
import matplotlib.pyplot as plt


# For now, this just provides a general idea. It will not be the final test_func. 
def test_compress_IL(x, y, e):
    bridge_string = "P10P10P10P10P"
    (L, EI, R, eType) = cba.parse_beam_string(bridge_string)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=40/100)

    x, y = ils.get_il(40/2, "M")
    e = 0.01
    xs, ys = _compress_discrete_IL(x, y, e)
    
    plt.figure()
    plt.plot(x, y, "b-")
    plt.plot(xs, ys, "r-")
    plt.show()
    
    print(len(x), len(xs))

