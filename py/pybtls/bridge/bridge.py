"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from ..lib.BTLS import _Bridge
from .influence_line import InfluenceLine, InfluenceSurface
from ..output.output_config import OutputConfig
from collections import defaultdict
from typing import Union
from numpy import isclose

__all__ = ["Bridge"]


class Bridge:
    """A bridge definition: span length, lanes, and load effects given by influence lines or surfaces."""

    _Bridge_Index = 0

    def __init__(self, length: float, no_lane: int):
        """
        A Bridge instance in Python stores the data for creating a CBridge instance in C++.

        Parameters
        ----------
        length : float\n
            Bridge length in m.

        no_lane : int\n
            Number of bridge lanes.
        """

        Bridge._Bridge_Index += 1
        self._bridge_index = Bridge._Bridge_Index

        self._length = length
        self._no_lane = no_lane

        self._no_load_effect = 0

        self._inf_file_dict = defaultdict(dict)
        self._threshold_list = []

    def add_load_effect(
        self,
        inf_line_surf: Union[InfluenceLine, list[InfluenceLine], InfluenceSurface],
        inf_weight: list[float] = None,
        threshold: float = 0.0,
    ) -> None:
        """
        Add a load effect to the bridge.

        Parameters
        ----------
        inf_line_surf : Union[InfluenceLine,list[InfluenceLine],InfluenceSurface] \n
            An InfluenceLine (same IL for all lanes); \n
            An InfluenceLine list (one IL per lane); \n
            An InfluenceSurface (for all lanes).

        inf_weight : list[float], optional \n
            Influence weight (0.0 to 1.0). \n
            It does not apply to an influence surface: for a lane whose \n
            influence is a surface, only 1.0 is accepted. \n
            The default is 1.0 for all lanes.

        threshold : float, optional \n
            Threshold for POT analysis. \n
            The default is 0.0.

        Returns
        -------
        None.

        Notes
        -----
        The load-effect mode of each influence line or surface (see
        ``set_mode``) is read once, here. Calling ``set_mode`` afterwards
        affects only load effects added after it, so the same influence line
        can carry a different mode for each effect it is added to.
        """

        if isinstance(inf_line_surf, list):
            if not all(
                isinstance(inf_line, InfluenceLine) for inf_line in inf_line_surf
            ):
                raise TypeError("All influence line should be InfluenceLine objects.")

            if len(inf_line_surf) != self._no_lane:
                raise ValueError(
                    "The number of influence lines should be equal to the number of lanes."
                )

            if not all([inf_line._data_assigned for inf_line in inf_line_surf]):
                raise ValueError("All influence lines should have their data assigned.")

        elif isinstance(inf_line_surf, (InfluenceLine, InfluenceSurface)):
            if not inf_line_surf._data_assigned:
                raise ValueError(
                    "The influence line or surface should have its data assigned."
                )

        else:
            raise TypeError(
                "The influence line or surface must be InfluenceLine or InfluenceSurface object(s)."
            )

        if inf_weight is not None and len(inf_weight) != self._no_lane:
            raise ValueError(
                "The length of inf_weight should be equal to the number of lanes."
            )

        lane_inf_lines = (
            inf_line_surf
            if isinstance(inf_line_surf, list)
            else [inf_line_surf] * self._no_lane
        )

        for i, lane_inf_line in enumerate(lane_inf_lines):
            inf_surf = (
                lane_inf_line
                if isinstance(lane_inf_line, InfluenceSurface)
                else lane_inf_line._data_dict["inf_surf"]
            )
            if inf_surf is None:  # this lane is an influence line, not a surface
                continue

            if inf_weight is not None and inf_weight[i] != 1.0:
                raise ValueError(
                    "The influence weight is not applied to an influence surface, "
                    f"but lane {i+1} is given inf_weight {inf_weight[i]}. "
                    "Bake the factor into the influence surface ordinates instead."
                )

            if len(inf_surf._data_dict["lane_position"]) < self._no_lane:
                raise ValueError(
                    f"The influence surface for lane {i+1} defines "
                    f"{len(inf_surf._data_dict['lane_position'])} lanes, "
                    f"fewer than the bridge's {self._no_lane} lanes."
                )

        self._no_load_effect += 1

        self._inf_file_dict[str(self._no_load_effect)]["inf_line"] = lane_inf_lines

        self._inf_file_dict[str(self._no_load_effect)]["weight"] = (
            [1.0] * self._no_lane if inf_weight is None else inf_weight
        )

        # Snapshot the load-effect mode now, exactly as the weight and threshold
        # are snapshotted. The influence line itself is stored by reference, so
        # reading the mode later would let a set_mode() call after this one
        # rewrite an effect that is already added, and would make it impossible
        # for one influence line to carry two modes for two different effects.
        self._inf_file_dict[str(self._no_load_effect)]["mode"] = [
            inf_line._load_effect_mode for inf_line in lane_inf_lines
        ]

        self._threshold_list.append(threshold)

    def _get_bridge(self, output_config: OutputConfig) -> _Bridge:
        """
        Get the created C++ CBridge instance.
        """

        bridge = _Bridge(output_config)
        bridge.setIndex(self._bridge_index)
        bridge.setLength(self._length)
        bridge.setNoLoadEffects(self._no_load_effect)

        bridge.initializeLanes(self._no_lane)

        for load_case in list(self._inf_file_dict.keys()):
            for i in range(self._no_lane):
                temp_inf_file = self._inf_file_dict[load_case]["inf_line"][i]
                temp_weight = self._inf_file_dict[load_case]["weight"][i]
                temp_mode = self._inf_file_dict[load_case]["mode"][i]

                if isinstance(temp_inf_file, InfluenceLine):
                    temp_IL = temp_inf_file._get_IL(temp_mode)
                elif isinstance(temp_inf_file, InfluenceSurface):
                    temp_IL_file = InfluenceLine("surface")
                    temp_IL_file.set_IL(inf_surf=temp_inf_file)
                    temp_IL = temp_IL_file._get_IL(temp_mode)

                temp_IL.setIndex(int(load_case))
                if not isclose(
                    temp_IL.getLength(), self._length, atol=1e-2
                ):  # centimeter level matching
                    raise RuntimeError(
                        f"Influence line or surface for lane {i+1} load case {load_case} is shorter or longer than the bridge."
                    )

                brige_lane = bridge.getBridgeLane(
                    i
                )  # bridge_lane is a CBridgeLane& object from C++.
                brige_lane.addLoadEffect(temp_IL, temp_weight)

        bridge.setThresholds(self._threshold_list)

        return bridge

    @property
    def no_lane(self) -> int:
        return self._no_lane

    @property
    def length(self) -> float:
        return self._length
