"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from ..lib.BTLS import _InfluenceLine, _InfluenceSurface
from ..utils.IL_compress import compress_discrete_IL
import numpy as np
from typing import Union, Literal

__all__ = ["InfluenceLine", "InfluenceSurface"]


class InfluenceLine:
    _IL_Index = 0

    def __init__(self, IL_type: Literal["discrete", "built-in"]):
        """
        An InfluenceLine instance in Python stores the data for creating a CInfluenceLine instance in C++. \n
        All length units are in m.

        Parameters
        ----------
        IL_type : Literal["discrete","built-in"] \n
            Influence line type.
        """

        self._IL_type = IL_type

        InfluenceLine._IL_Index += 1
        self._IL_index = InfluenceLine._IL_Index

        self._data_dict = {
            "position": None,
            "ordinate": None,
            "id": None,
            "length": None,
            "inf_surf": None,
        }

        self._data_assigned = False

    def __getstate__(self):
        attribute_dict = {}
        attribute_dict["IL_type"] = self._IL_type
        attribute_dict["IL_index"] = self._IL_index
        attribute_dict["data_dict"] = self._data_dict
        attribute_dict["data_assigned"] = self._data_assigned
        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._IL_type = attribute_dict["IL_type"]
        self._IL_index = attribute_dict["IL_index"]
        self._data_dict = attribute_dict["data_dict"]
        self._data_assigned = attribute_dict["data_assigned"]

    def set_IL(self, **kwargs) -> None:
        """
        Set influence line data.

        Keyword Arguments
        -----------------
        For discrete: \n
            - position : Union[list,np.ndarray] \n
                Discrete influence line position (X).

            - ordinate : Union[list,np.ndarray] \n
                Discrete influence line ordinate (Y).

            - compress_tolerance : float, optional \n
                The maximum relative error (0.001~0.1) allowed for a discrete influence line to be simplified (to boost simulation). \n
                The default is None.

        For built-in: \n
            - id : Literal[1,2,3,4,5,6,7,8,9] \n
                Built-in influence line id. \n
                What the id number represents: \n
                (The support numbering starts from the left side.) \n
                1: The mid-span sagging BM of a simply supported beam. \n
                2: The 2nd support hogging BM of a two-span continuous beam. \n
                3: The 1st support SF of a simply supported beam. \n
                4: The 2nd support SF of a simply supported beam. \n
                5: The 3rd support SF of a two-span continuous beam. \n
                6: The 1st support SF of a two-span continuous beam. \n
                7: The total weights of vehicle being on the beam. \n
                8: The 2nd support hogging BM of a three-span continuous beam. \n
                9: The 3rd support hogging BM of a three-span continuous beam. \n
                (Beams' first and last supports are pin.)

            - length : float \n
                Built-in influence line length.

        Returns
        -------
        None.
        """

        if self._IL_type == "discrete":
            self._set_IL_discrete(**kwargs)
        elif self._IL_type == "built-in":
            self._set_IL_built_in(**kwargs)
        elif (
            self._IL_type == "surface"
        ):  # This is hidden in the doc to avoid user confusion, not an error.
            self._set_IL_surface(**kwargs)
        else:
            raise ValueError("Invalid influence line type.")

        self._data_assigned = True

    def _set_IL_discrete(self, **kwargs) -> None:
        position = kwargs.get("position")
        ordinate = kwargs.get("ordinate")
        compress_tolerance = kwargs.get("compress_tolerance")

        if compress_tolerance is not None:
            if compress_tolerance > 0.1:
                raise Warning(
                    "The compress_tolerance is too large, which may cause the corresponding load effect significantly inaccurate."
                )
            position, ordinate = compress_discrete_IL(
                position, ordinate, compress_tolerance
            )

        if position is None or ordinate is None:
            raise ValueError("Uncompleted input for discrete influence line.")

        if not isinstance(position, (list, np.ndarray)) or not isinstance(
            ordinate, (list, np.ndarray)
        ):
            raise TypeError("The position and ordinate should be list or np.ndarray.")

        if len(position) != len(ordinate):
            raise ValueError("The position and ordinate must be equal length.")

        self._data_dict["position"] = position
        self._data_dict["ordinate"] = ordinate

    def _set_IL_built_in(self, **kwargs) -> None:
        id = kwargs.get("id")
        length = kwargs.get("length")

        if id is None or length is None:
            raise ValueError("Uncompleted input for built-in influence line.")

        if not isinstance(id, int) or not isinstance(length, float):
            raise TypeError("The id should be int and length should be float.")

        if id not in [1, 2, 3, 4, 5, 6, 7, 8, 9]:
            raise ValueError("Please refer to the doc for the built-in IL id.")

        self._data_dict["id"] = id
        self._data_dict["length"] = length

    def _set_IL_surface(self, **kwargs) -> None:
        inf_surf = kwargs.get("inf_surf")

        if inf_surf is None:
            raise ValueError("Uncompleted input for surface influence line.")

        if not isinstance(inf_surf, InfluenceSurface):
            raise TypeError("The IS should be an InfluenceSurface instance.")

        self._data_dict["inf_surf"] = inf_surf

    def _get_IL(self) -> _InfluenceLine:
        """
        Get the created C++ CInfluenceLine instance.
        """

        inf_line = _InfluenceLine()

        if self._IL_type == "discrete":
            inf_line.setIL(self._data_dict["position"], self._data_dict["ordinate"])
        elif self._IL_type == "built-in":
            inf_line.setIL(self._data_dict["id"], self._data_dict["length"])
        elif self._IL_type == "surface":
            inf_line.setIL(self._data_dict["inf_surf"]._get_IS())

        return inf_line

    # def show(self) -> None:
    #     """
    #     Plot the influence line.

    #     Returns
    #     -------
    #     None.
    #     """

    #     raise NotImplementedError()


class InfluenceSurface:
    _IS_Index = 0

    def __init__(self):
        """
        The InfluenceSurface instance in Python stores the data for creating a CInfluenceSurface instance in C++.
        """

        InfluenceSurface._IS_Index += 1
        self._IS_index = InfluenceSurface._IS_Index

        self._data_dict = {"lane_position": None, "IS_matrix": None}

        self._data_assigned = False

    def __getstate__(self):
        attribute_dict = {}
        attribute_dict["IS_index"] = self._IS_index
        attribute_dict["data_dict"] = self._data_dict
        attribute_dict["data_assigned"] = self._data_assigned
        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._IS_index = attribute_dict["IS_index"]
        self._data_dict = attribute_dict["data_dict"]
        self._data_assigned = attribute_dict["data_assigned"]

    def set_IS(
        self, IS_matrix: Union[list, np.ndarray], lane_position: Union[list, np.ndarray]
    ) -> None:
        """
        Set influence surface data.

        Parameters
        ----------
        IS_matrix : Union[list,np.ndarray]\n
            Influence surface matrix, where the data points are meshed in grid (refer to the BTLS Manual for an example).\n
                First row: The transverse position of the IS data points.\n
                First column: The longitudinal position of the IS data points.\n
                Rest of data: The corresponding values of the IS data points at the positions.\n
            Here's a simple illustration about the bridge coordinate:\n
                Observe the bridge from its plan view\n
                and regard its longitudinal direction as the horizontal.\n
                Now the origin point is at the left down corner.\n

        lane_position : Union[list,np.ndarray]\n
            Lanes' transverse positions.\n
            [(lane 1 left, lane 1 right), ..., (lane n left, lane n right)]\n
            Lane 1 is the nearest lane to the origin point.

        Returns
        -------
        None.
        """

        if not isinstance(lane_position, (list, np.ndarray)) or not isinstance(
            IS_matrix, (list, np.ndarray)
        ):
            raise TypeError(
                "The lane_position and IS_matrix should be list or np.ndarray."
            )

        if not isinstance(IS_matrix[0], (list, np.ndarray)):
            raise ValueError("The IS_matrix should be 2D.")

        self._data_dict["lane_position"] = lane_position
        self._data_dict["IS_matrix"] = IS_matrix

        self._data_assigned = True

    def _get_IS(self) -> _InfluenceSurface:
        """
        Get the created C++ CInfluenceSurface instance.
        """

        inf_surf = _InfluenceSurface()

        inf_surf.setLanes(self._data_dict["lane_position"])
        inf_surf.setIS(self._data_dict["IS_matrix"])

        return inf_surf

    # def show(self) -> None:
    #     """
    #     Plot the influence surface.

    #     Returns
    #     -------
    #     None.
    #     """

    #     raise NotImplementedError()
