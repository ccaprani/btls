from .output_config import OutputConfig
from .Read import (
    read_TH,
    read_AE,
    read_traffic,
    read_TS,
    read_BM_V,
    read_BM_All,
    read_BM_S,
    read_POT_S,
    read_POT_V,
    read_POT_C,
    read_E_CS,
    read_E_IS,
    read_FE,
    read_FR,
)

# from .Plot import plot_TH, plot_AE, plot_BM_S
from pathlib import Path
from typing import Literal, Union
import pandas as pd
import glob

__all__ = ["_OutputManager"]


class _OutputManager:
    def __init__(self, output_root: Path, sim_tag: str, output_config: OutputConfig):
        """
        The OutputManager class records the results' paths and provides methods to read these data. \n
        Its instance should not be created by the user.

        Parameters
        ----------
        output_root : Path\n
            The root directory of all the output folders.\n
        sim_tag : str\n
            The tag of the simulation, which is same as the output folder name.\n
        output_config : OutputConfig\n
            The output configuration for the corresponding simulation.
        """

        self._output_root = output_root
        self._this_output_dir = sim_tag
        self._output_config = output_config
        self._summary = {
            "time_history": None,
            "all_events": None,
            "traffic": None,
            "BM_by_no_trucks": None,
            "BM_by_mixed": None,
            "BM_summary": None,
            "POT_vehicle": None,
            "POT_summary": None,
            "POT_counter": None,
            "traffic_statistics": None,
            "E_cumulative_statistics": None,
            "E_interval_statistics": None,
            "fatigue_events": None,
            "fatigue_rainflow": None,
        }  # Stores the paths of the output files.

        self._fetch_summary()

    def relocate(self, output_root: Path) -> None:
        """
        Relocate the output root directory.

        Parameters
        ----------
        output_root : Path\n
            The new root directory of all the output folders.
        """

        self._output_root = (
            Path(output_root).resolve()
            if not isinstance(output_root, Path)
            else output_root.resolve()
        )
        self._fetch_summary()

    def get_summary(self, with_path: bool = False) -> Union[list, dict]:
        """
        Get to know the available outputs.

        Parameters
        ----------
        with_path : bool, optional\n
            Default is False.\n
            If True, the return will include the the output file paths.

        Returns
        -------
        Union[list,dict]\n
            The available outputs.
        """

        filtered_summary = {k: v for k, v in self._summary.items() if v is not None}

        if with_path:
            return filtered_summary
        else:
            return list(filtered_summary.keys())

    def read_data(
        self,
        key: Literal[
            "time_history",
            "all_events",
            "traffic",
            "BM_by_no_trucks",
            "BM_by_mixed",
            "BM_summary",
            "POT_vehicle",
            "POT_summary",
            "POT_counter",
            "traffic_statistics",
            "E_cumulative_statistics",
            "E_interval_statistics",
            "fatigue_events",
            "fatigue_rainflow",
        ],
    ) -> dict[str, pd.DataFrame]:
        """
        Read the data from the recorded output file paths.

        Parameters
        ----------
        key : Literal["time_history", "all_events", "traffic", "BM_by_no_trucks", "BM_by_mixed", "BM_summary", "POT_vehicle", "POT_summary", "POT_counter", "traffic_statistics", "E_cumulative_statistics", "E_interval_statistics", "fatigue_events", "fatigue_rainflow"]\n
            Choose a type of output file.

        Returns
        -------
        dict[str, pd.DataFrame]\n
            The data. The key is the file name without .txt.
        """

        if self._summary[key] is None:
            raise ValueError(f"Output {key} is invalid.")

        def create_file_key(file_path: Path) -> str:
            file_path_split = str(file_path).split("/")
            if self._output_config is None:
                return file_path_split[
                    -2
                ]  # dir1 or dir2, enough for single-vehicle sim
            else:
                return file_path_split[-1].rstrip(".txt")

        return_dict: dict[str, pd.DataFrame] = {}

        if key == "time_history":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_TH(path)
        elif key == "all_events":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_AE(path)
        elif key == "traffic":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_traffic(
                    path, self._output_config._Output.VehicleFile.FILE_FORMAT
                )
        elif key == "BM_by_no_trucks":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_BM_V(path)
        elif key == "BM_by_mixed":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_BM_All(path)
        elif key == "BM_summary":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_BM_S(path)
        elif key == "POT_vehicle":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_POT_V(path)
        elif key == "POT_summary":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_POT_S(path)
        elif key == "POT_counter":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_POT_C(path)
        elif key == "traffic_statistics":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_TS(path)
        elif key == "E_cumulative_statistics":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_E_CS(path)
        elif key == "E_interval_statistics":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_E_IS(path)
        elif key == "fatigue_events":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_FE(path)
        elif key == "fatigue_rainflow":
            for path in self._summary[key]:
                return_dict[create_file_key(path)] = read_FR(path)

        return return_dict

    # def plot_data(self):
    #     pass

    @property
    def tag(self) -> str:
        return self._this_output_dir

    def _fetch_summary(self) -> None:
        if self._output_config is None:
            self._summary["time_history"] = self._search_file(
                self._output_root / (self._this_output_dir + "/dir1"), "TH_"
            ) + self._search_file(
                self._output_root / (self._this_output_dir + "/dir2"), "TH_"
            )

            self._summary["all_events"] = self._search_file(
                self._output_root / (self._this_output_dir + "/dir1"), "BL_*_AllEvents"
            ) + self._search_file(
                self._output_root / (self._this_output_dir + "/dir2"), "BL_*_AllEvents"
            )

        else:
            self._summary["time_history"] = (
                self._search_file(self._output_root / self._this_output_dir, "TH_")
                if self._output_config._Output.WRITE_TIME_HISTORY
                else None
            )

            self._summary["all_events"] = (
                self._search_file(
                    self._output_root / self._this_output_dir, "BL_*_AllEvents"
                )
                if self._output_config._Output.WRITE_EACH_EVENT
                else None
            )

            self._summary["traffic"] = (
                self._search_file(
                    self._output_root / self._this_output_dir,
                    self._output_config._Output.VehicleFile.VEHICLE_FILENAME.split(
                        ".txt"
                    )[0],
                )
                if self._output_config._Output.VehicleFile.WRITE_VEHICLE_FILE
                else None
            )

            self._summary["BM_by_no_trucks"] = (
                self._search_file(
                    self._output_root / self._this_output_dir, "BM_V_", "BM_V_*_All"
                )
                if self._output_config._Output.BlockMax.WRITE_BM_VEHICLES
                else None
            )

            self._summary["BM_by_mixed"] = (
                self._search_file(
                    self._output_root / self._this_output_dir, "BM_V_*_All"
                )
                if self._output_config._Output.BlockMax.WRITE_BM_MIXED
                else None
            )

            self._summary["BM_summary"] = (
                self._search_file(self._output_root / self._this_output_dir, "BM_S_")
                if self._output_config._Output.BlockMax.WRITE_BM_SUMMARY
                else None
            )

            self._summary["POT_vehicle"] = (
                self._search_file(self._output_root / self._this_output_dir, "PT_V_")
                if self._output_config._Output.POT.WRITE_POT_VEHICLES
                else None
            )

            self._summary["POT_summary"] = (
                self._search_file(self._output_root / self._this_output_dir, "PT_S_")
                if self._output_config._Output.POT.WRITE_POT_SUMMARY
                else None
            )

            self._summary["POT_counter"] = (
                self._search_file(self._output_root / self._this_output_dir, "PT_C_")
                if self._output_config._Output.POT.WRITE_POT_COUNTER
                else None
            )

            self._summary["traffic_statistics"] = (
                self._search_file(
                    self._output_root / self._this_output_dir, "FlowData_"
                )
                if self._output_config._Output.VehicleFile.WRITE_FLOW_STATS
                else None
            )

            self._summary["E_cumulative_statistics"] = (
                self._search_file(self._output_root / self._this_output_dir, "SS_C_")
                if self._output_config._Output.Stats.WRITE_SS_CUMULATIVE
                else None
            )

            self._summary["E_interval_statistics"] = (
                self._search_file(self._output_root / self._this_output_dir, "SS_S_")
                if self._output_config._Output.Stats.WRITE_SS_INTERVALS
                else None
            )

            self._summary["fatigue_events"] = (
                self._search_file(
                    self._output_root / self._this_output_dir, "BL_*_Fatigue"
                )
                if self._output_config._Output.WRITE_FATIGUE_EVENT
                else None
            )

            self._summary["fatigue_rainflow"] = (
                self._search_file(self._output_root / self._this_output_dir, "RC_")
                if self._output_config._Output.Fatigue.DO_FATIGUE_RAINFLOW
                else None
            )

    def _search_file(
        self, directory: str, pattern: str, exclude: str = None
    ) -> list[str]:
        pattern = f"{directory}/{pattern}*.txt"
        files_containing_pattern = glob.glob(pattern)

        if exclude:
            files_to_exclude = glob.glob(f"{directory}/{exclude}*.txt")
            files_containing_pattern = list(
                set(files_containing_pattern) - set(files_to_exclude)
            )

        return files_containing_pattern  # in absolute path
