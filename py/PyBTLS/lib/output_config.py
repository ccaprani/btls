from .BTLS_collections import _ConfigDataCore

__all__ = ["OutputConfig"]


class OutputConfig(_ConfigDataCore):
    def __init__(self):
        super().__init__()

    def set_event_output(
        self,
        write_time_history: bool = False,
        write_each_event: bool = False,
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for recording all load events to HDD.

        Parameters
        ----------
        write_time_history : bool, optional\n
            Whether to write time history data to HDD. The default is False.

        write_each_event : bool, optional\n
            Whether to write each event to HDD. The default is False.

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.WRITE_TIME_HISTORY = write_time_history
        self._Output.WRITE_EACH_EVENT = write_each_event
        self._Output.WRITE_EVENT_BUFFER_SIZE = buffer_size

    def set_vehicle_file_output(
        self,
        write_vehicle_file: bool = False,
        vehicle_file_format: int = 4,
        vehicle_file_name: str = "output_traffic.txt",
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for recording the vehicles (in the generated traffic) to HDD.

        Parameters
        ----------
        write_vehicle_file : bool, optional\n
            Whether to write vehicles to HDD. The default is False.

        vehicle_file_format : int, optional\n
            The format of the vehicle file. The default is 4.

        vehicle_file_name : str, optional\n
            The name of the vehicle file. The default is "output_traffic.txt".

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.VehicleFile.WRITE_VEHICLE_FILE = write_vehicle_file
        self._Output.VehicleFile.FILE_FORMAT = vehicle_file_format
        self._Output.VehicleFile.VEHICLE_FILENAME = vehicle_file_name
        self._Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = buffer_size

    def set_BM_output(
        self,
        write_vehicle: bool = False,
        write_summary: bool = False,
        write_mixed: bool = False,
        block_size_days: int = 1,
        block_size_secs: int = 0,
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for conducting block-max analysis to load events and writing data to HDD.

        Parameters
        ----------
        write_vehicle : bool, optional\n
            Whether to write vehicles to HDD. The default is False.

        write_summary : bool, optional\n
            Whether to write summary data to HDD. The default is False.

        write_mixed : bool, optional\n
            Whether to write mixed data to HDD. The default is False.

        block_size_days : int, optional\n
            The block size in days. The default is 1.

        block_size_secs : int, optional\n
            The block size in seconds. The default is 0.

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.BlockMax.WRITE_BM = True
        self._Output.BlockMax.WRITE_BM_VEHICLES = write_vehicle
        self._Output.BlockMax.WRITE_BM_SUMMARY = write_summary
        self._Output.BlockMax.WRITE_BM_MIXED = write_mixed
        self._Output.BlockMax.BLOCK_SIZE_DAYS = block_size_days
        self._Output.BlockMax.BLOCK_SIZE_SECS = block_size_secs
        self._Output.BlockMax.WRITE_BM_BUFFER_SIZE = buffer_size

    def set_POT_output(
        self,
        write_vehicle: bool = False,
        write_summary: bool = False,
        write_counter: bool = False,
        POT_size_days: int = 1,
        POT_size_secs: int = 0,
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for conducting peak-over-threshold analysis to load events and writing data to HDD.

        Parameters
        ----------
        write_vehicle : bool, optional\n
            Whether to write vehicles to HDD. The default is False.

        write_summary : bool, optional\n
            Whether to write summary data to HDD. The default is False.

        write_counter : bool, optional\n
            Whether to write counter data to HDD. The default is False.

        POT_size_days : int, optional\n
            The POT size in days. The default is 1.

        POT_size_secs : int, optional\n
            The POT size in seconds. The default is 0.

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.POT.WRITE_POT = True
        self._Output.POT.WRITE_POT_VEHICLES = write_vehicle
        self._Output.POT.WRITE_POT_SUMMARY = write_summary
        self._Output.POT.WRITE_POT_COUNTER = write_counter
        self._Output.POT.POT_COUNT_SIZE_DAYS = POT_size_days
        self._Output.POT.POT_COUNT_SIZE_SECS = POT_size_secs
        self._Output.POT.WRITE_POT_BUFFER_SIZE = buffer_size

    def set_stats_output(
        self,
        write_flow_stats: bool = False,
        write_overall: bool = False,
        write_intervals: bool = False,
        interval_size: int = 3600,
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for doing statistic to the flow and writing data to HDD.

        Parameters
        ----------
        write_flow_stats : bool, optional\n
            Whether to write flow statistics to HDD. The default is False.

        write_overall : bool, optional\n
            Whether to write overall statistics to HDD. The default is False.

        write_intervals : bool, optional\n
            Whether to write interval statistics to HDD. The default is False.

        interval_size : int, optional\n
            The interval size in seconds. The default is 3600.

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.VehicleFile.WRITE_FLOW_STATS = write_flow_stats
        self._Output.Stats.WRITE_STATS = True
        self._Output.Stats.WRITE_SS_CUMULATIVE = write_overall
        self._Output.Stats.WRITE_SS_INTERVALS = write_intervals
        self._Output.Stats.WRITE_SS_INTERVAL_SIZE = interval_size
        self._Output.Stats.WRITE_SS_BUFFER_SIZE = buffer_size

    def set_fatigue_output(
        self,
        write_fatigue_event: bool = False,
        write_rainflow_output: bool = False,
        rainflow_decimal: int = 1,
        rainflow_cut_off: float = 0.0,
        buffer_size: int = 10000,
    ) -> None:
        """
        Config for recording fatigue event, doing rainflow count and writing data to HDD.

        Parameters
        ----------
        write_fatigue_event : bool, optional\n
            Whether to write fatigue event to HDD. The default is False.

        write_rainflow_output : bool, optional\n
            Whether to write rainflow count result to HDD. The default is False.

        rainflow_decimal : int, optional\n
            The decimal of rainflow count. The default is 1.

        rainflow_cut_off : float, optional\n
            The cut off value of rainflow count. The default is 0.0.

        buffer_size : int, optional\n
            The buffer size for writing data to HDD. The default is 10000.
        """

        self._Output.WRITE_FATIGUE_EVENT = write_fatigue_event
        self._Output.Fatigue.DO_FATIGUE_RAINFLOW = write_rainflow_output
        self._Output.Fatigue.RAINFLOW_DECIMAL = rainflow_decimal
        self._Output.Fatigue.RAINFLOW_CUTOFF = rainflow_cut_off
        self._Output.WRITE_EVENT_BUFFER_SIZE = buffer_size
        self._Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE = buffer_size
