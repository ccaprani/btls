__all__ = ["_compress_discrete_IL"]


def _compress_discrete_IL(x, y, e):
    """
    Compresses a set of discrete points (x, y) by removing points that are within a certain error tolerance e of a line
    connecting the start point to a trial point. Returns the compressed set of points as two separate lists.

    Args:
        x (list): List of x-coordinates of the points.
        y (list): List of y-coordinates of the points.
        e (float): Maximum relative error allowed for a point to be considered part of the line.

    Returns:
        tuple: A tuple containing two lists: the compressed x-coordinates and the compressed y-coordinates.
    """
    n = len(x)
    xs = [x[0]]
    ys = [y[0]]
    k1 = 0  # Index of the starting point

    for k in range(1, n):
        m = (y[k] - y[k1]) / (x[k] - x[k1])  # Slope of line from start point to trial point
        emax = 0
        max_j = k1

        for j in range(k1 + 1, k + 1):  # Include point k in the check as well
            yline = m * (x[j] - x[k1]) + y[k1]
            
            # Calculate relative error
            if y[j] == 0:
                etrial = abs(yline)
            else:
                etrial = abs((yline - y[j]) / y[j])
                
            if etrial > emax:
                emax = etrial
                max_j = j  # Store the index where max error occurred

        if emax > e:
            xs.append(x[max_j])
            ys.append(y[max_j])
            k1 = max_j  # Update starting point

    xs.append(x[-1])  # Add the last point
    ys.append(y[-1])
            
    return xs, ys

