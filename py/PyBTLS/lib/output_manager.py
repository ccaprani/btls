from .output_config import OutputConfig
from pathlib import Path
import pandas as pd
import glob

__all__ = ["OutputManager"]


class OutputManager:

    def __init__(self, output_root:Path, sim_tag:str, output_config:OutputConfig):
        """
        Constructor of the OutputManager class.

        Parameters
        ----------
        output_root : Path\n
            The root directory of all the output folders.
        sim_tag : str\n
            The tag of the simulation, which is same as the output folder name.
        output_config : OutputConfig\n
            The output configuration for the corresponding simulation.
        """

        self._output_root = output_root
        self._this_output_dir = sim_tag
        self._output_config = output_config
        self._summary = {}  # Stores the paths of the output files.

        self._fetch_summary()

    def relocate(self, output_root:Path) -> None:
        """
        Relocate the output root directory.

        Parameters
        ----------
        output_root : Path\n
            The new root directory of all the output folders.
        """

        self._output_root = output_root
        self._fetch_summary()

    def get_summary(self) -> dict:
        """
        Get a dict storing the paths of the output files.

        Returns
        -------
        dict\n
            The paths.
        """

        return self._summary

    def read_data(self, key:str) -> list[pd.DataFrame]:
        """
        Read the data from the recorded output file path. 

        Parameters
        ----------
        key : str\n
            The key of the output file.

        Returns
        -------
        list[pd.DataFrame]\n
            The data.
        """

        if self._summary[key] is None:
            raise ValueError(f"Output {key} is invalid.")

        return_list: list[pd.DataFrame] = []

        for file in self._summary[key]:
            return_list.append(pd.read_csv(file))
            
        return return_list

    @property
    def tag(self) -> str:
        return self._this_output_dir

    def _fetch_summary(self) -> None:

        self._summary = {}

        if self._output_config is None:

            self._summary["time_history_file"] = self._search_file(self._output_root / (self._this_output_dir + "dir1"), "TH_") + self._search_file(self._output_root / (self._this_output_dir + "dir2"), "TH_")

            self._summary["all_events_file"] = self._search_file(self._output_root / (self._this_output_dir + "dir1"), "BL_*_AllEvents") + self._search_file(self._output_root / (self._this_output_dir + "dir2"), "BL_*_AllEvents")

        else:

            self._summary["time_history"] = self._search_file(self._output_root / self._this_output_dir, "TH_") if self._output_config._Output.WRITE_TIME_HISTORY else None

            self._summary["all_events"] = self._search_file(self._output_root / self._this_output_dir, "BL_*_AllEvents") if self._output_config._Output.WRITE_EACH_EVENT else None

            self._summary["traffic"] = self._search_file(self._output_root / self._this_output_dir, self._output_config._Output.VehicleFile.VEHICLE_FILENAME.split(".txt")[0]) if self._output_config._Output.VehicleFile.WRITE_VEHICLE_FILE else None

            self._summary["BM_by_no_trucks"] = self._search_file(self._output_root / self._this_output_dir, "BM_V_", "BM_V_*_All") if self._output_config._Output.BlockMax.WRITE_BM_VEHICLES else None

            self._summary["BM_by_mixed"] = self._search_file(self._output_root / self._this_output_dir, "BM_V_*_All") if self._output_config._Output.BlockMax.WRITE_BM_MIXED else None

            self._summary["BM_summary"] = self._search_file(self._output_root / self._this_output_dir, "BM_S_") if self._output_config._Output.BlockMax.WRITE_BM_SUMMARY else None

            self._summary["POT_vehicle"] = self._search_file(self._output_root / self._this_output_dir, "PT_V_") if self._output_config._Output.POT.WRITE_POT_VEHICLES else None

            self._summary["POT_summary"] = self._search_file(self._output_root / self._this_output_dir, "PT_S_") if self._output_config._Output.POT.WRITE_POT_SUMMARY else None

            self._summary["POT_counter"] = self._search_file(self._output_root / self._this_output_dir, "PT_C_") if self._output_config._Output.POT.WRITE_POT_COUNTER else None
            
            self._summary["traffic_statistics"] = self._search_file(self._output_root / self._this_output_dir, "FlowData_") if self._output_config._Output.VehicleFile.WRITE_FLOW_STATS else None

            self._summary["LE_cumulative_statistics"] = self._search_file(self._output_root / self._this_output_dir, "SS_C_") if self._output_config._Output.Stats.WRITE_SS_CUMULATIVE else None

            self._summary["LE_interval_statistics"] = self._search_file(self._output_root / self._this_output_dir, "SS_S_") if self._output_config._Output.Stats.WRITE_SS_INTERVALS else None

            self._summary["fatigue_events"] = self._search_file(self._output_root / self._this_output_dir, "BL_*_Fatigue") if self._output_config._Output.WRITE_FATIGUE_EVENT else None

            self._summary["fatigue_rainflow"] = self._search_file(self._output_root / self._this_output_dir, "RC_") if self._output_config._Output.Fatigue.DO_FATIGUE_RAINFLOW else None

    def _search_file(self, directory:str, pattern:str, exclude:str=None) -> list[str]:

        pattern = f"{directory}/{pattern}*.txt"
        files_containing_pattern = glob.glob(pattern)

        if exclude:
            files_to_exclude = glob.glob(f"{directory}/{exclude}*.txt")
            files_containing_pattern = list(set(files_containing_pattern) - set(files_to_exclude))

        return files_containing_pattern  # in absolute path
