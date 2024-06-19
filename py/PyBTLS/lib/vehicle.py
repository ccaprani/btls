from .BTLS_collections import _Vehicle

__all__ = ["Vehicle"]


class Vehicle(_Vehicle):
    def __init__(self, no_axle: int):
        """
        This Vehicle class in Python is inherited from CVehicle in C++. It has several extra methods for setting vehicle properties.

        Parameters
        ----------
        no_axle : int\n
            Number of axles.
        """

        super().__init__()
        self._setNoAxles(no_axle)

        self._setHead(0)
        self.set_time(0.0)
        self.set_velocity(0.0)
        self._setLocalLane(1)
        self.set_direction(1)
        self.set_trans(1.8)

    def set_axle_weights(self, axle_weights: list) -> None:
        """
        Set axle weights.

        Parameters
        ----------
        axle_weights : list\n
            Axle weights in kN. Its lengh should be equal to the number of axles.

        Returns
        -------
        None.
        """

        for i in range(self.get_no_axles()):
            self._setAxleWeight(i, axle_weights[i])

        self._setGVW(sum(axle_weights))

    def set_axle_spacings(self, axle_spacings: list) -> None:
        """
        Set axle spacings.

        Parameters
        ----------
        axle_spacings : list\n
            Axle spacings in m. Its lengh should be equal to the number of axles - 1.

        Returns
        -------
        None.
        """

        for i in range(self.get_no_axles() - 1):
            self._setAxleSpacing(i, axle_spacings[i])

        self._setLength(sum(axle_spacings))

    def set_axle_widths(self, axle_widths: list) -> None:
        """
        Set axle widths.

        Parameters
        ----------
        axle_widths : list\n
            Axle widths in m. Its lengh should be equal to the number of axles.

        Returns
        -------
        None.
        """

        for i in range(self.get_no_axles()):
            self._setAxleWidth(i, axle_widths[i])
