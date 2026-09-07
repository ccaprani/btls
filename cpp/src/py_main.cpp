// py_main.cpp
// the main file for the PyBTLS Build

#include "PrepareSim.h"
#include "ConsoleOutput.h"
#include "Distribution.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl/filesystem.h"
#include "pybind11/numpy.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;


// Bulk-extract per-vehicle scalars and flat per-axle arrays from a vehicle list
// in one C++ pass, so the GPU engine avoids ~6 Python->C++ getter calls per
// vehicle. Returns numpy arrays; all the trajectory math stays vectorized in
// Python. Output: (time, speed, dirn, gvw, global_lane, trans, length, accel,
// axle_count[/veh], axle_weight[/axle], axle_spacing[/axle], axle_track[/axle],
// is_car, class_bin, lane_eccentricity).
// Flat per-vehicle scalars + per-axle arrays for the GPU engine, shared by the
// loader path (_extract_axle_data) and the fused generator (_generate_and_extract)
// so a vehicle is unpacked exactly once, the same way, regardless of source.
struct _AxleArrays
{
	std::vector<double> vtime, vspeed, vgvw, vtrans, vlen, vacc, vecc;
	std::vector<std::int64_t> vdir, vlane, vcount, viscar, viscls;
	std::vector<double> aw, asp, at;

	// classifier (optional): when given, viscls[i] = its class-histogram bin for
	// the vehicle (CVehicleClassification::getClassID) — for the flow statistics;
	// otherwise -1 (load-effect runs don't need it).
	void add(CVehicle& v, std::size_t no_lane, CVehicleClassification* classifier)
	{
		vtime.push_back(v.getTime());
		vspeed.push_back(v.getVelocity());
		vgvw.push_back(v.getGVW());
		vtrans.push_back(v.getTrans());
		vlen.push_back(v.getLength());
		vacc.push_back(v.getAcceleration());
		vecc.push_back(v.getLaneEccentricity());
		vdir.push_back((std::int64_t)v.getDirection());
		vlane.push_back((std::int64_t)v.getGlobalLane(no_lane));
		viscar.push_back((std::int64_t)v.IsCar());
		viscls.push_back(classifier ? (std::int64_t)classifier->getClassID(v.getClass()) : -1);
		std::size_t na = v.getNoAxles();
		vcount.push_back((std::int64_t)na);
		for (std::size_t j = 0; j < na; j++)
		{
			aw.push_back(v.getAW(j));
			asp.push_back(v.getAS(j));
			at.push_back(v.getAT(j));
		}
	}

	py::tuple to_tuple() const
	{
		auto d = [](const std::vector<double>& v) { return py::array_t<double>(v.size(), v.data()); };
		auto l = [](const std::vector<std::int64_t>& v) { return py::array_t<std::int64_t>(v.size(), v.data()); };
		// vecc is appended last so the positional indices of the original
		// 14 fields stay stable for existing consumers
		return py::make_tuple(d(vtime), d(vspeed), l(vdir), d(vgvw), l(vlane),
							  d(vtrans), d(vlen), d(vacc), l(vcount), d(aw), d(asp), d(at),
							  l(viscar), l(viscls), d(vecc));
	}
};


static py::tuple _extract_axle_data(const std::vector<CVehicle_sp>& vehs, std::size_t no_lane,
		CVehicleClassification_sp classifier = nullptr)
{
	_AxleArrays a;
	for (const auto& v : vehs)
		a.add(*v, no_lane, classifier.get());
	return a.to_tuple();
}


// Pull the globally-earliest-arriving lane until end_time, in the same order the
// per-vehicle Python loop did (identical RNG draw order -> identical stream).
static int _earliest_lane(const std::vector<CLaneGenTraffic_sp>& lanes, double end_time)
{
	int earliest = -1;
	double best = end_time;
	for (std::size_t i = 0; i < lanes.size(); i++)
	{
		double t = lanes[i]->GetNextArrivalTime();
		if (t < best) { best = t; earliest = (int)i; }
	}
	return earliest;
}


// Generate the next slice of generated traffic up to end_time, returning the
// vehicle objects (used when per-vehicle output, e.g. PT_V, is needed).
static std::vector<CVehicle_sp> _generate_traffic_stream(
		std::vector<CLaneGenTraffic_sp> lanes, double end_time)
{
	std::vector<CVehicle_sp> out;
	int earliest;
	while ((earliest = _earliest_lane(lanes, end_time)) >= 0)
		out.push_back(lanes[earliest]->GetNextVehicle());
	return out;
}


// Fused generate + extract: generate up to end_time and unpack each vehicle
// inline, returning the same flat arrays as _extract_axle_data WITHOUT building a
// Python list of Vehicle objects (the GPU generated-traffic hot path). The stream
// is byte-identical to _generate_traffic_stream + _extract_axle_data.
static py::tuple _generate_and_extract(
		std::vector<CLaneGenTraffic_sp> lanes, double end_time, std::size_t no_lane,
		CVehicleClassification_sp classifier = nullptr)
{
	_AxleArrays a;
	int earliest;
	while ((earliest = _earliest_lane(lanes, end_time)) >= 0)
	{
		CVehicle_sp v = lanes[earliest]->GetNextVehicle();
		a.add(*v, no_lane, classifier.get());
	}
	return a.to_tuple();
}


PYBIND11_MODULE(libbtls, m) {
	#ifdef VERSION_INFO
		m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
	#else
		m.attr("__version__") = "dev";
	#endif
	m.def("get_info", &preamble, "Print the information of the BTLS library.");
	m.def("set_console_output", [](bool enable) { btls::console_output = enable; },
		"Enable or disable routine console messages (buffer-flush notices). "
		"Default is off; errors and warnings are always printed.",
		py::arg("enable"));
	m.def("_sample_uniform", []() {
		CDistribution d;
		return d.GenerateUniform();
	}, "Internal test helper: draw one uniform [0,1) sample from the process-wide RNG.");
	m.def("_extract_axle_data", &_extract_axle_data, py::arg("vehicles"), py::arg("no_lane"),
		py::arg("classifier") = nullptr,
		"Bulk-extract per-vehicle/per-axle arrays from a vehicle list in one C++ pass "
		"(used by the GPU engine to avoid per-vehicle Python getter calls). With a "
		"classifier, also returns each vehicle's flow-statistics class bin.");
	m.def("_generate_traffic_stream", &_generate_traffic_stream, py::arg("lanes"), py::arg("end_time"),
		"Generate generated-traffic vehicles up to end_time in one C++ pass, pulling the "
		"globally-earliest lane each step (identical interleaving/RNG order to the per-vehicle "
		"loop). Lets the GPU engine stream generation without per-vehicle Python calls.");
	m.def("_generate_and_extract", &_generate_and_extract, py::arg("lanes"), py::arg("end_time"), py::arg("no_lane"),
		py::arg("classifier") = nullptr,
		"Fused generate + extract: generate up to end_time and return the same flat arrays as "
		"_extract_axle_data, without materializing Python Vehicle objects (the GPU generated-"
		"traffic hot path). Byte-identical to _generate_traffic_stream + _extract_axle_data.");
	m.def("seed", &CRNGWrapper::seed,
		R"(
		Seed the process-wide random number generator.

		If never called, the generator is seeded non-deterministically at
		library load time (the default behaviour). Calling seed() makes all
		subsequent traffic generation and sampling deterministic for the
		given value.

		In multiprocessing contexts (spawn start method), each worker
		process has its own copy of the generator, so calling
		``seed(master_seed + worker_id)`` in each worker produces
		independent, reproducible streams.

		Parameters
		----------
		s : int
			Seed value (unsigned 64-bit integer).
		)",
		py::arg("s"));
	m.def("run", &run, 
		R"(
		Run the simulation in the traditional BTLS way. 

		Parameters
		----------
		BTLSin_file : str
			The path to the BTLS input file.
			Refer to the BTLS manual for what it means.
		)", 
		py::arg("BTLSin_file"));
	py::class_<CConfigDataCore> cconfigdatacore(m, "_ConfigDataCore");
		cconfigdatacore.def(py::init<>())
			.def_readwrite("_Road", &CConfigDataCore::Road)
			.def_readwrite("_Gen", &CConfigDataCore::Gen)
			.def_readwrite("_Traffic", &CConfigDataCore::Traffic)
			.def_readwrite("_Output", &CConfigDataCore::Output)
			.def("_setRoad", &CConfigDataCore::setRoad, "set CConfigDataCore.Road attribute.", py::arg("no_lane"), py::arg("no_dir"), py::arg("no_lanes_dir1"), py::arg("no_lanes_dir2"))
			.def(py::pickle(
				[](CConfigDataCore& self) {  // __getstate__
					py::dict attribute_dict;

					py::dict road_dict;
					road_dict["NO_LANES_DIR1"] = self.Road.NO_LANES_DIR1;
					road_dict["NO_LANES_DIR2"] = self.Road.NO_LANES_DIR2;
					road_dict["NO_LANES"] = self.Road.NO_LANES;
					road_dict["NO_DIRS"] = self.Road.NO_DIRS;

					py::dict gen_dict;
					gen_dict["TRAFFIC_FOLDER"] = self.Gen.TRAFFIC_FOLDER;
					gen_dict["TRUCK_TRACK_WIDTH"] = self.Gen.TRUCK_TRACK_WIDTH;
					gen_dict["LANE_ECCENTRICITY_STD"] = self.Gen.LANE_ECCENTRICITY_STD;
					gen_dict["KERNEL_TYPE"] = self.Gen.KERNEL_TYPE;

					py::dict traffic_dict;
					traffic_dict["CLASSIFICATION"] = self.Traffic.CLASSIFICATION;
					traffic_dict["CONGESTED_SPACING"] = self.Traffic.CONGESTED_SPACING;
					traffic_dict["CONGESTED_SPEED"] = self.Traffic.CONGESTED_SPEED;
					traffic_dict["CONGESTED_GAP"] = self.Traffic.CONGESTED_GAP;
					traffic_dict["CONGESTED_GAP_COEF_VAR"] = self.Traffic.CONGESTED_GAP_COEF_VAR;
					traffic_dict["CONSTANT_SPEED"] = self.Traffic.CONSTANT_SPEED;
					traffic_dict["CONSTANT_GAP"] = self.Traffic.CONSTANT_GAP;

					py::dict output_dict;
					output_dict["OUTPUT_DIR"] = self.Output.OUTPUT_DIR;
					output_dict["WRITE_TIME_HISTORY"] = self.Output.WRITE_TIME_HISTORY;
					output_dict["WRITE_EACH_EVENT"] = self.Output.WRITE_EACH_EVENT;
					output_dict["WRITE_EVENT_BUFFER_SIZE"] = self.Output.WRITE_EVENT_BUFFER_SIZE;
					output_dict["WRITE_FATIGUE_EVENT"] = self.Output.WRITE_FATIGUE_EVENT;

					py::dict vehiclefile_dict;
					vehiclefile_dict["WRITE_VEHICLE_FILE"] = self.Output.VehicleFile.WRITE_VEHICLE_FILE;
					vehiclefile_dict["FILE_FORMAT"] = self.Output.VehicleFile.FILE_FORMAT;
					vehiclefile_dict["VEHICLE_FILENAME"] = self.Output.VehicleFile.VEHICLE_FILENAME;
					vehiclefile_dict["WRITE_VEHICLE_BUFFER_SIZE"] = self.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE;
					vehiclefile_dict["WRITE_FLOW_STATS"] = self.Output.VehicleFile.WRITE_FLOW_STATS;
					output_dict["VehicleFile"] = vehiclefile_dict;

					py::dict blockmax_dict;
					blockmax_dict["WRITE_BM"] = self.Output.BlockMax.WRITE_BM;
					blockmax_dict["WRITE_BM_VEHICLES"] = self.Output.BlockMax.WRITE_BM_VEHICLES;
					blockmax_dict["WRITE_BM_SUMMARY"] = self.Output.BlockMax.WRITE_BM_SUMMARY;
					blockmax_dict["WRITE_BM_MIXED"] = self.Output.BlockMax.WRITE_BM_MIXED;
					blockmax_dict["BLOCK_SIZE_DAYS"] = self.Output.BlockMax.BLOCK_SIZE_DAYS;
					blockmax_dict["BLOCK_SIZE_SECS"] = self.Output.BlockMax.BLOCK_SIZE_SECS;
					blockmax_dict["WRITE_BM_BUFFER_SIZE"] = self.Output.BlockMax.WRITE_BM_BUFFER_SIZE;
					output_dict["BlockMax"] = blockmax_dict;

					py::dict pot_dict;
					pot_dict["WRITE_POT"] = self.Output.POT.WRITE_POT;
					pot_dict["WRITE_POT_VEHICLES"] = self.Output.POT.WRITE_POT_VEHICLES;
					pot_dict["WRITE_POT_SUMMARY"] = self.Output.POT.WRITE_POT_SUMMARY;
					pot_dict["WRITE_POT_COUNTER"] = self.Output.POT.WRITE_POT_COUNTER;
					pot_dict["POT_COUNT_SIZE_DAYS"] = self.Output.POT.POT_COUNT_SIZE_DAYS;
					pot_dict["POT_COUNT_SIZE_SECS"] = self.Output.POT.POT_COUNT_SIZE_SECS;
					pot_dict["WRITE_POT_BUFFER_SIZE"] = self.Output.POT.WRITE_POT_BUFFER_SIZE;
					output_dict["POT"] = pot_dict;

					py::dict stats_dict;
					stats_dict["WRITE_STATS"] = self.Output.Stats.WRITE_STATS;
					stats_dict["WRITE_SS_CUMULATIVE"] = self.Output.Stats.WRITE_SS_CUMULATIVE;
					stats_dict["WRITE_SS_INTERVALS"] = self.Output.Stats.WRITE_SS_INTERVALS;
					stats_dict["WRITE_SS_INTERVAL_SIZE"] = self.Output.Stats.WRITE_SS_INTERVAL_SIZE;
					stats_dict["WRITE_SS_BUFFER_SIZE"] = self.Output.Stats.WRITE_SS_BUFFER_SIZE;
					output_dict["Stats"] = stats_dict;

					py::dict fatigue_dict;
					fatigue_dict["DO_FATIGUE_RAINFLOW"] = self.Output.Fatigue.DO_FATIGUE_RAINFLOW;
					fatigue_dict["RAINFLOW_DECIMAL"] = self.Output.Fatigue.RAINFLOW_DECIMAL;
					fatigue_dict["RAINFLOW_CUTOFF"] = self.Output.Fatigue.RAINFLOW_CUTOFF;
					fatigue_dict["WRITE_FATIGUE_BUFFER_SIZE"] = self.Output.Fatigue.WRITE_FATIGUE_BUFFER_SIZE;
					fatigue_dict["WRITE_RAINFLOW_RESIDUALS"] = self.Output.Fatigue.WRITE_RAINFLOW_RESIDUALS;
					output_dict["Fatigue"] = fatigue_dict;

					attribute_dict["Road"] = road_dict;
					attribute_dict["Gen"] = gen_dict;
					attribute_dict["Traffic"] = traffic_dict;
					attribute_dict["Output"] = output_dict;

					return attribute_dict;
				},
				[](py::dict attribute_dict){  // __setstate__
					CConfigDataCore config;

					config.Road.NO_LANES_DIR1 = attribute_dict["Road"]["NO_LANES_DIR1"].cast<size_t>();
					config.Road.NO_LANES_DIR2 = attribute_dict["Road"]["NO_LANES_DIR2"].cast<size_t>();
					config.Road.NO_LANES = attribute_dict["Road"]["NO_LANES"].cast<size_t>();
					config.Road.NO_DIRS = attribute_dict["Road"]["NO_DIRS"].cast<size_t>();

					config.Gen.TRAFFIC_FOLDER = attribute_dict["Gen"]["TRAFFIC_FOLDER"].cast<std::string>();
					config.Gen.TRUCK_TRACK_WIDTH = attribute_dict["Gen"]["TRUCK_TRACK_WIDTH"].cast<double>();
					config.Gen.LANE_ECCENTRICITY_STD = attribute_dict["Gen"]["LANE_ECCENTRICITY_STD"].cast<double>();
					config.Gen.KERNEL_TYPE = attribute_dict["Gen"]["KERNEL_TYPE"].cast<int>();

					config.Traffic.CLASSIFICATION = attribute_dict["Traffic"]["CLASSIFICATION"].cast<int>();
					config.Traffic.CONGESTED_SPACING = attribute_dict["Traffic"]["CONGESTED_SPACING"].cast<double>();
					config.Traffic.CONGESTED_SPEED = attribute_dict["Traffic"]["CONGESTED_SPEED"].cast<double>();
					config.Traffic.CONGESTED_GAP = attribute_dict["Traffic"]["CONGESTED_GAP"].cast<double>();
					config.Traffic.CONGESTED_GAP_COEF_VAR = attribute_dict["Traffic"]["CONGESTED_GAP_COEF_VAR"].cast<double>();
					config.Traffic.CONSTANT_SPEED = attribute_dict["Traffic"]["CONSTANT_SPEED"].cast<double>();
					config.Traffic.CONSTANT_GAP = attribute_dict["Traffic"]["CONSTANT_GAP"].cast<double>();

					if (attribute_dict["Output"].cast<py::dict>().contains("OUTPUT_DIR"))  // absent before pybtls 1.2.0
						config.Output.OUTPUT_DIR = attribute_dict["Output"]["OUTPUT_DIR"].cast<std::string>();
					config.Output.WRITE_TIME_HISTORY = attribute_dict["Output"]["WRITE_TIME_HISTORY"].cast<bool>();
					config.Output.WRITE_EACH_EVENT = attribute_dict["Output"]["WRITE_EACH_EVENT"].cast<bool>();
					config.Output.WRITE_EVENT_BUFFER_SIZE = attribute_dict["Output"]["WRITE_EVENT_BUFFER_SIZE"].cast<size_t>();
					config.Output.WRITE_FATIGUE_EVENT = attribute_dict["Output"]["WRITE_FATIGUE_EVENT"].cast<bool>();

					config.Output.VehicleFile.WRITE_VEHICLE_FILE = attribute_dict["Output"]["VehicleFile"]["WRITE_VEHICLE_FILE"].cast<bool>();
					config.Output.VehicleFile.FILE_FORMAT = attribute_dict["Output"]["VehicleFile"]["FILE_FORMAT"].cast<size_t>();
					config.Output.VehicleFile.VEHICLE_FILENAME = attribute_dict["Output"]["VehicleFile"]["VEHICLE_FILENAME"].cast<std::string>();
					config.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = attribute_dict["Output"]["VehicleFile"]["WRITE_VEHICLE_BUFFER_SIZE"].cast<size_t>();
					config.Output.VehicleFile.WRITE_FLOW_STATS = attribute_dict["Output"]["VehicleFile"]["WRITE_FLOW_STATS"].cast<bool>();

					config.Output.BlockMax.WRITE_BM = attribute_dict["Output"]["BlockMax"]["WRITE_BM"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_VEHICLES = attribute_dict["Output"]["BlockMax"]["WRITE_BM_VEHICLES"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_SUMMARY = attribute_dict["Output"]["BlockMax"]["WRITE_BM_SUMMARY"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_MIXED = attribute_dict["Output"]["BlockMax"]["WRITE_BM_MIXED"].cast<bool>();
					config.Output.BlockMax.BLOCK_SIZE_DAYS = attribute_dict["Output"]["BlockMax"]["BLOCK_SIZE_DAYS"].cast<size_t>();
					config.Output.BlockMax.BLOCK_SIZE_SECS = attribute_dict["Output"]["BlockMax"]["BLOCK_SIZE_SECS"].cast<size_t>();
					config.Output.BlockMax.WRITE_BM_BUFFER_SIZE = attribute_dict["Output"]["BlockMax"]["WRITE_BM_BUFFER_SIZE"].cast<size_t>();

					config.Output.POT.WRITE_POT = attribute_dict["Output"]["POT"]["WRITE_POT"].cast<bool>();
					config.Output.POT.WRITE_POT_VEHICLES = attribute_dict["Output"]["POT"]["WRITE_POT_VEHICLES"].cast<bool>();
					config.Output.POT.WRITE_POT_SUMMARY = attribute_dict["Output"]["POT"]["WRITE_POT_SUMMARY"].cast<bool>();
					config.Output.POT.WRITE_POT_COUNTER = attribute_dict["Output"]["POT"]["WRITE_POT_COUNTER"].cast<bool>();
					config.Output.POT.POT_COUNT_SIZE_DAYS = attribute_dict["Output"]["POT"]["POT_COUNT_SIZE_DAYS"].cast<size_t>();
					config.Output.POT.POT_COUNT_SIZE_SECS = attribute_dict["Output"]["POT"]["POT_COUNT_SIZE_SECS"].cast<size_t>();
					config.Output.POT.WRITE_POT_BUFFER_SIZE = attribute_dict["Output"]["POT"]["WRITE_POT_BUFFER_SIZE"].cast<size_t>();

					config.Output.Stats.WRITE_STATS = attribute_dict["Output"]["Stats"]["WRITE_STATS"].cast<bool>();
					config.Output.Stats.WRITE_SS_CUMULATIVE = attribute_dict["Output"]["Stats"]["WRITE_SS_CUMULATIVE"].cast<bool>();
					config.Output.Stats.WRITE_SS_INTERVALS = attribute_dict["Output"]["Stats"]["WRITE_SS_INTERVALS"].cast<bool>();
					config.Output.Stats.WRITE_SS_INTERVAL_SIZE = attribute_dict["Output"]["Stats"]["WRITE_SS_INTERVAL_SIZE"].cast<size_t>();
					config.Output.Stats.WRITE_SS_BUFFER_SIZE = attribute_dict["Output"]["Stats"]["WRITE_SS_BUFFER_SIZE"].cast<size_t>();

					config.Output.Fatigue.DO_FATIGUE_RAINFLOW = attribute_dict["Output"]["Fatigue"]["DO_FATIGUE_RAINFLOW"].cast<bool>();
					config.Output.Fatigue.RAINFLOW_DECIMAL = attribute_dict["Output"]["Fatigue"]["RAINFLOW_DECIMAL"].cast<int>();
					config.Output.Fatigue.RAINFLOW_CUTOFF = attribute_dict["Output"]["Fatigue"]["RAINFLOW_CUTOFF"].cast<double>();
					config.Output.Fatigue.WRITE_FATIGUE_BUFFER_SIZE = attribute_dict["Output"]["Fatigue"]["WRITE_FATIGUE_BUFFER_SIZE"].cast<size_t>();
					if (attribute_dict["Output"]["Fatigue"].cast<py::dict>().contains("WRITE_RAINFLOW_RESIDUALS"))  // absent before pybtls 1.2.0
						config.Output.Fatigue.WRITE_RAINFLOW_RESIDUALS = attribute_dict["Output"]["Fatigue"]["WRITE_RAINFLOW_RESIDUALS"].cast<bool>();

					return config;
				}
			));
		py::class_<CConfigDataCore::Road_Config> road_config(cconfigdatacore, "_Road_Config");
			road_config.def_readwrite("LANES_FILE", &CConfigDataCore::Road_Config::LANES_FILE)
				.def_readwrite("NO_LANES_DIR1", &CConfigDataCore::Road_Config::NO_LANES_DIR1)
				.def_readwrite("NO_LANES_DIR2", &CConfigDataCore::Road_Config::NO_LANES_DIR2)
				.def_readwrite("NO_LANES", &CConfigDataCore::Road_Config::NO_LANES)
				.def_readwrite("NO_DIRS", &CConfigDataCore::Road_Config::NO_DIRS);
		py::class_<CConfigDataCore::Gen_Config> gen_config(cconfigdatacore, "_Gen_Config");
			gen_config.def_readwrite("TRAFFIC_FOLDER", &CConfigDataCore::Gen_Config::TRAFFIC_FOLDER)
				.def_readwrite("GEN_TRAFFIC", &CConfigDataCore::Gen_Config::GEN_TRAFFIC)
				.def_readwrite("NO_DAYS", &CConfigDataCore::Gen_Config::NO_DAYS)
				.def_readwrite("TRUCK_TRACK_WIDTH", &CConfigDataCore::Gen_Config::TRUCK_TRACK_WIDTH)
				.def_readwrite("LANE_ECCENTRICITY_STD", &CConfigDataCore::Gen_Config::LANE_ECCENTRICITY_STD)
				.def_readwrite("KERNEL_TYPE", &CConfigDataCore::Gen_Config::KERNEL_TYPE)
				.def_readwrite("NO_OVERLAP_LENGTH", &CConfigDataCore::Gen_Config::NO_OVERLAP_LENGTH);
		py::class_<CConfigDataCore::Traffic_Config> traffic_config(cconfigdatacore, "_Traffic_Config");
			traffic_config.def_readwrite("CLASSIFICATION", &CConfigDataCore::Traffic_Config::CLASSIFICATION)
				.def_readwrite("VEHICLE_MODEL", &CConfigDataCore::Traffic_Config::VEHICLE_MODEL)
				.def_readwrite("HEADWAY_MODEL", &CConfigDataCore::Traffic_Config::HEADWAY_MODEL)
				.def_readwrite("CONGESTED_SPACING", &CConfigDataCore::Traffic_Config::CONGESTED_SPACING)
				.def_readwrite("CONGESTED_SPEED", &CConfigDataCore::Traffic_Config::CONGESTED_SPEED)
				.def_readwrite("CONGESTED_GAP", &CConfigDataCore::Traffic_Config::CONGESTED_GAP)
				.def_readwrite("CONGESTED_GAP_COEF_VAR", &CConfigDataCore::Traffic_Config::CONGESTED_GAP_COEF_VAR)
				.def_readwrite("CONSTANT_SPEED", &CConfigDataCore::Traffic_Config::CONSTANT_SPEED)
				.def_readwrite("CONSTANT_GAP", &CConfigDataCore::Traffic_Config::CONSTANT_GAP);
		py::class_<CConfigDataCore::Output_Config> output_config(cconfigdatacore, "_Output_Config");
			output_config.def_readwrite("OUTPUT_DIR", &CConfigDataCore::Output_Config::OUTPUT_DIR)
				.def_readwrite("WRITE_TIME_HISTORY", &CConfigDataCore::Output_Config::WRITE_TIME_HISTORY)
				.def_readwrite("WRITE_EACH_EVENT", &CConfigDataCore::Output_Config::WRITE_EACH_EVENT)
				.def_readwrite("WRITE_EVENT_BUFFER_SIZE", &CConfigDataCore::Output_Config::WRITE_EVENT_BUFFER_SIZE)
				.def_readwrite("WRITE_FATIGUE_EVENT", &CConfigDataCore::Output_Config::WRITE_FATIGUE_EVENT)
				.def_readwrite("VehicleFile", &CConfigDataCore::Output_Config::VehicleFile)
				.def_readwrite("BlockMax", &CConfigDataCore::Output_Config::BlockMax)
				.def_readwrite("POT", &CConfigDataCore::Output_Config::POT)
				.def_readwrite("Stats", &CConfigDataCore::Output_Config::Stats)
				.def_readwrite("Fatigue", &CConfigDataCore::Output_Config::Fatigue);
			py::class_<CConfigDataCore::Output_Config::VehicleFile_Config> vehiclefile_config(output_config, "VehicleFile_Config");
				vehiclefile_config.def_readwrite("WRITE_VEHICLE_FILE", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_VEHICLE_FILE)
						.def_readwrite("FILE_FORMAT", &CConfigDataCore::Output_Config::VehicleFile_Config::FILE_FORMAT)
						.def_readwrite("VEHICLE_FILENAME", &CConfigDataCore::Output_Config::VehicleFile_Config::VEHICLE_FILENAME)
						.def_readwrite("WRITE_VEHICLE_BUFFER_SIZE", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_VEHICLE_BUFFER_SIZE)
						.def_readwrite("WRITE_FLOW_STATS", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_FLOW_STATS);
			py::class_<CConfigDataCore::Output_Config::BlockMax_Config> blockmax_config(output_config, "BlockMax_Config");
				blockmax_config.def_readwrite("WRITE_BM", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM)
						.def_readwrite("WRITE_BM_VEHICLES", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_VEHICLES)
						.def_readwrite("WRITE_BM_SUMMARY", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_SUMMARY)
						.def_readwrite("WRITE_BM_MIXED", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_MIXED)
						.def_readwrite("BLOCK_SIZE_DAYS", &CConfigDataCore::Output_Config::BlockMax_Config::BLOCK_SIZE_DAYS)
						.def_readwrite("BLOCK_SIZE_SECS", &CConfigDataCore::Output_Config::BlockMax_Config::BLOCK_SIZE_SECS)
						.def_readwrite("WRITE_BM_BUFFER_SIZE", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::POT_Config> pot_config(output_config, "POT_Config");
				pot_config.def_readwrite("WRITE_POT", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT)
					.def_readwrite("WRITE_POT_VEHICLES", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_VEHICLES)
					.def_readwrite("WRITE_POT_SUMMARY", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_SUMMARY)
					.def_readwrite("WRITE_POT_COUNTER", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_COUNTER)
					.def_readwrite("POT_COUNT_SIZE_DAYS", &CConfigDataCore::Output_Config::POT_Config::POT_COUNT_SIZE_DAYS)
					.def_readwrite("POT_COUNT_SIZE_SECS", &CConfigDataCore::Output_Config::POT_Config::POT_COUNT_SIZE_SECS)
					.def_readwrite("WRITE_POT_BUFFER_SIZE", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::Stats_Config> stats_config(output_config, "Stats_Config");
				stats_config.def_readwrite("WRITE_STATS", &CConfigDataCore::Output_Config::Stats_Config::WRITE_STATS)
					.def_readwrite("WRITE_SS_CUMULATIVE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_CUMULATIVE)
					.def_readwrite("WRITE_SS_INTERVALS", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_INTERVALS)
					.def_readwrite("WRITE_SS_INTERVAL_SIZE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_INTERVAL_SIZE)
					.def_readwrite("WRITE_SS_BUFFER_SIZE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::Fatigue_Config> fatigue_config(output_config, "Fatigue_Config");
					fatigue_config.def_readwrite("DO_FATIGUE_RAINFLOW", &CConfigDataCore::Output_Config::Fatigue_Config::DO_FATIGUE_RAINFLOW)
						.def_readwrite("RAINFLOW_DECIMAL", &CConfigDataCore::Output_Config::Fatigue_Config::RAINFLOW_DECIMAL)
						.def_readwrite("RAINFLOW_CUTOFF", &CConfigDataCore::Output_Config::Fatigue_Config::RAINFLOW_CUTOFF)
						.def_readwrite("WRITE_FATIGUE_BUFFER_SIZE", &CConfigDataCore::Output_Config::Fatigue_Config::WRITE_FATIGUE_BUFFER_SIZE)
						.def_readwrite("WRITE_RAINFLOW_RESIDUALS", &CConfigDataCore::Output_Config::Fatigue_Config::WRITE_RAINFLOW_RESIDUALS);

	py::class_<CRainflow> crainflow(m, "_Rainflow");
		crainflow.doc() = "ASTM E1049-85 rainflow cycle counter. Used to close spliced chunk residuals exactly.";
		crainflow.def(py::init<int, double>(), py::arg("decimal"), py::arg("cutoff"))
			.def("processData", &CRainflow::processData, py::arg("series"),
				"Feed a load-effect series (or a residual reversal sequence) into the reversal buffer.")
			.def("calcCycles", &CRainflow::calcCycles, py::arg("is_final"),
				"Run the rainflow count; pass True to close the residual at end of data.")
			.def("getRainflowOutput", &CRainflow::getRainflowOutput,
				py::return_value_policy::copy,
				"Get the accumulated output: dict of rounded range -> cycle count.")
			.def("getResiduals", &CRainflow::getResiduals,
				py::return_value_policy::copy,
				"Get the residual (unclosed) reversal sequence after calcCycles(False).");

	py::class_<CInfluenceLine> cinfluenceline(m, "_InfluenceLine");
		cinfluenceline.def(py::init<>())
			.def("setIndex", &CInfluenceLine::setIndex, py::arg("IL_index"))
			.def("setIL", py::overload_cast<size_t, double>(&CInfluenceLine::setIL), py::arg("built_in_IL_no"), py::arg("length"))
			.def("setIL", py::overload_cast<std::vector<double>, std::vector<double> >(&CInfluenceLine::setIL), py::arg("positions"), py::arg("ordinates"))
			.def("setIL", py::overload_cast<CInfluenceSurface>(&CInfluenceLine::setIL), py::arg("inf_surface"))
			.def("setWeight", &CInfluenceLine::setWeight, py::arg("weight"))
			.def("setLoadEffectMode", &CInfluenceLine::setLoadEffectMode, py::arg("mode"),
				 "Load-effect mode: 0 = vertical (default), 1 = centrifugal, 2 = braking. "
				 "For centrifugal, the per-axle force becomes AxleWeight * Speed^2 / g (per-vehicle v^2); "
				 "the caller bakes the bridge geometric constants (k_e and 1 / R) into the IL ordinates "
				 "so that the convolved bearing reaction is in kN. For braking, the per-axle force "
				 "becomes AxleWeight * ``|Acceleration|`` / g (per-vehicle deceleration), with a scalar "
				 "fallback set via setBrakingFactor.")
			.def("setBrakingFactor", &CInfluenceLine::setBrakingFactor, py::arg("braking_factor"),
				 "Set braking-mode dimensionless fallback factor (deceleration / g). Used when the "
				 "per-axle CAxle::m_Acceleration is zero (e.g. constant-velocity vehicle stream).")
			.def("getLength", &CInfluenceLine::getLength);
	py::class_<CInfluenceSurface> cinfluencesurface(m, "_InfluenceSurface");
		cinfluencesurface.def(py::init<>())
			.def("setLanes", py::overload_cast<std::vector<std::pair<double,double>>>(&CInfluenceSurface::setLanes), py::arg("lane_positions"))
			.def("setIS", &CInfluenceSurface::setIS, py::arg("IS_matrix"));

	py::class_<CBridge, CBridge_sp> cbridge(m, "_Bridge");
		cbridge.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("setIndex", &CBridge::setIndex, py::arg("index"))
			.def("setLength", &CBridge::setLength, "in metre.", py::arg("length"))
			.def("setNoLoadEffects", &CBridge::setNoLoadEffects, py::arg("no_LE"))
			.def("initializeLanes", &CBridge::InitializeLanes, py::arg("no_lane"))
			.def("getBridgeLane", &CBridge::getBridgeLane, py::arg("lane_index"), py::return_value_policy::reference)
			.def("setThresholds", &CBridge::setThresholds, py::arg("threshold_list"))
			.def("addVehicle", &CBridge::AddVehicle, py::arg("vehicle"))
			.def("setCalcTimeStep", &CBridge::setCalcTimeStep, py::arg("time_step"))
			.def("update", &CBridge::Update, py::arg("next_arrival_time"), py::arg("current_time"))
			.def("finish", py::overload_cast<>(&CBridge::Finish))
			.def("finish", py::overload_cast<double>(&CBridge::Finish), py::arg("sim_end_time"), "Finish, filling silent trailing blocks/intervals up to the simulated end time.")
			.def("initializeDataMgr", &CBridge::InitializeDataMgr, py::arg("sim_start_time"));
	py::class_<CBridgeLane> cbridgelane(m, "_BridgeLane");
		cbridgelane.def("addLoadEffect", &CBridgeLane::addLoadEffect, py::arg("IL"), py::arg("weight"));


	py::class_<CVehicle, CVehicle_sp> cvehicle(m, "Vehicle", "A vehicle: axle weights/spacings/widths, speed, lane, direction and arrival time.");
		cvehicle.def(py::init<size_t>(), 
				R"(
				The Vehicle class is inherited from the CVehicle class in the C++ BTLS library. 
				It is used to create a customized vehicle object. 

				Parameters
				----------
				no_axles : int
					Number of axles.
				)", 
				py::arg("no_axles"))
			.def("set_velocity", &CVehicle::setVelocity, 
				R"(
				Set vehicle velocity.

				Parameters
				----------
				velocity : float
					The velocity of the vehicle, in m/s.
				)", 
				py::arg("velocity"))
			.def("set_acceleration", &CVehicle::setAcceleration,
				R"(
				Set vehicle longitudinal acceleration.

				Used by the braking mode of CInfluenceLine: each axle's per-time
				deceleration is propagated into the load-effect convolution as
				F_axle = AxleWeight * ``|a|`` / g. Default zero (constant-velocity
				motion).

				Parameters
				----------
				acceleration : float
					The longitudinal acceleration of the vehicle, in m/s^2
					(negative = braking).
				)",
				py::arg("acceleration"))
			.def("set_local_from_global_lane", &CVehicle::setLocalFromGlobalLane, 
				R"(
				Set the local lane index of the vehicle from its 1-based global index.
				This method needs to be called after the vehicle direction is set. 

				Parameters
				----------
				global_lane_index : int
					The 1-based global lane index of the vehicle.
				no_lanes : int
					The number of lanes in the direction of the vehicle.
				)", 
				py::arg("global_lane_index"), py::arg("no_lanes"))
			.def("set_direction", &CVehicle::setDirection, 
				R"(
				Set the direction of the vehicle.

				Parameters
				----------
				direction : Literal[1,2]
					The direction of the vehicle. 
				)", 
				py::arg("direction"))
			.def("set_time", &CVehicle::setTime, 
				R"(
				Set the show-up time of the vehicle.

				Parameters
				----------
				time : float
					The showing time of the vehicle, in seconds.
				)", 
				py::arg("time"))
			.def("set_trans", &CVehicle::setTrans, 
				R"(
				Set the vehicle transverse position.

				Parameters
				----------
				trans : float
					The transverse position of the vehicle on its lane, in metres.
				)", 
				py::arg("trans"))
			.def("_setHead", &CVehicle::setHead, 
				"Set the head id of the vehicle.", 
				py::arg("head"))
			.def("set_local_lane", &CVehicle::setLocalLane, 
				R"(
				Set the local lane index of the vehicle.

				Parameters
				----------
				local_lane_index : int
					The 1-based local lane index of the vehicle.
				)", 
				py::arg("local_lane_index"))
			.def("set_axle_weights", 
				[](CVehicle_sp self, std::vector<double> weights) {
					for (size_t i = 0; i < weights.size(); i++) {
						self->setAW(i, weights[i]);
					}
					self->setGVW(std::accumulate(weights.begin(), weights.end(), 0.0));
				},
				R"(
				Set the vehicle axle weights.
				The gross vehicle weight is calculated as the sum of the axle weights automatically.

				Parameters
				----------
				weights : list[float]
					The vehicle axle weights.
				)", 
				py::arg("axle_weights"))
			.def("set_axle_weight", 
				[](CVehicle_sp self, size_t index, double weight) {
					self->setGVW(self->getGVW() + weight - self->getAW(index));
					self->setAW(index, weight);
				},
				R"(
				Set the specified axle weight.
				The gross vehicle weight is updated automatically.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				weight : float
					The axle weight.
				)", 
				py::arg("index"), py::arg("weight"))
			.def("set_axle_spacings", 
				[](CVehicle_sp self, std::vector<double> spacings) {
					for (size_t i = 0; i < spacings.size(); i++) {
						self->setAS(i, spacings[i]);
					}
					self->setLength(std::accumulate(spacings.begin(), spacings.end(), 0.0));
				},
				R"(
				Set the vehicle axle spacings.
				The vehicle length is calculated as the sum of the axle spacings automatically.

				Parameters
				----------
				spacings : list[float]
					The vehicle axle spacings.
					The last spacing should always be zero. 
				)", 
				py::arg("spacings"))
			.def("set_axle_spacing", 
				[](CVehicle_sp self, size_t index, double spacing) {
					self->setLength(self->getLength() + spacing - self->getAS(index));
					self->setAS(index, spacing);
				},
				R"(
				Set the specified axle spacing.
				The vehicle length is updated automatically.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				spacing : float
					The axle spacing.
				)",
				py::arg("index"), py::arg("spacing"))
			.def("set_axle_widths", 
				[](CVehicle_sp self, std::vector<double> widths) {
					for (size_t i = 0; i < widths.size(); i++) {
						self->setAT(i, widths[i]);
					}
				},
				R"(
				Set the vehicle axle widths.

				Parameters
				----------
				widths : list[float]
					The vehicle axle widths.
				)",
				py::arg("widths"))
			.def("set_axle_width", 
				[](CVehicle_sp self, size_t index, double width) {
					self->setAT(index, width);
				},
				R"(
				Set the specified axle width.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				width : float
					The axle width.
				)",
				py::arg("index"), py::arg("width"))
			.def("get_length", &CVehicle::getLength, "Get the vehicle length, in metres.")
			.def("write",
				// Write() normalises a near-zero transverse position in place, so
				// serialise a copy to leave the caller's vehicle untouched
				[](CVehicle_sp self, size_t file_format) { CVehicle veh(*self); return veh.Write(file_format); },
				R"(
				Serialise the vehicle to one line in the given traffic-file format.

				Parameters
				----------
				file_format : int
					The traffic file format (1=CASTOR, 2=BEDIT, 3=DITIS, 4=MON).
				)",
				py::arg("file_format"))
			.def("get_velocity", &CVehicle::getVelocity, "Get the vehicle velocity, in m/s.")
			.def("get_acceleration", &CVehicle::getAcceleration,
				 "Get the vehicle longitudinal acceleration in m/s^2 (negative = braking).")
			.def("get_gvw", &CVehicle::getGVW, "Get the gross vehicle weight of the vehicle.")
			.def("get_no_axles", &CVehicle::getNoAxles, "Get the number of axles of the vehicle.")
			.def("get_axle_weights", 
				[](CVehicle_sp self) {
					std::vector<double> weights;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						weights.push_back(self->getAW(i));
					}
					return weights;
				},
				"Get the vehicle axle weights.")
			.def("get_axle_weight", &CVehicle::getAW, 
				R"(
				Get the specified axle weight.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_axle_spacings", 
				[](CVehicle_sp self) {
					std::vector<double> spacings;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						spacings.push_back(self->getAS(i));
					}
					return spacings;
				},
				"Get the vehicle axle spacings.")
			.def("get_axle_spacing", &CVehicle::getAS, 
				R"(
				Get the value of the specified axle spacing.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_axle_widths", 
				[](CVehicle_sp self) {
					std::vector<double> widths;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						widths.push_back(self->getAT(i));
					}
					return widths;
				},
				"Get the vehicle axle widths.")
			.def("get_axle_width", &CVehicle::getAT, 
				R"(
				Get the value of the specified axle width.

				Parameters
				----------
				index : int
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_time", &CVehicle::getTime, "Get the show-up time of the vehicle, in seconds.")
			.def("get_trans", &CVehicle::getTrans, "Get the vehicle transverse position on its lane, in metres.")
			.def("get_direction", &CVehicle::getDirection, "Get the vehicle direction (1 or 2).")
			.def("_getGlobalLane", &CVehicle::getGlobalLane, "Get the 1-based global lane index of the vehicle.", py::arg("no_lanes"))
			.def("get_local_lane", &CVehicle::getLocalLane, "Get the 1-based local lane index of the vehicle.")
			.def("_get_all_properties", &CVehicle::getPropInTuple, "Get all the vehicle properties in a tuple.")
			.def("_set_all_properties", &CVehicle::setPropByTuple, 
				"Set all the vehicle properties from a tuple.", 
				py::arg("prop_tuple"))
			.def("_create", &CVehicle::create, py::arg("str"), py::arg("format"))
			.def("__eq__", 
				[](CVehicle_sp self, CVehicle_sp other) { CVehicle a(*self), b(*other); return a.Write(4) == b.Write(4); }, 
				py::is_operator())
			.def(py::pickle(
				[](CVehicle_sp self) {  // __getstate__

					// The property tuple does not carry m_Class, but the class
					// drives IsCar() and the flow/statistics truck counts, so it
					// must survive the pickle across process boundaries
					// (multiprocessing sends TrafficLoader vehicles to workers).
					Classification cl = self->getClass();
					return py::make_tuple(self->getPropInTuple(), cl.m_ID, cl.m_String, cl.m_Desc);
				},
				[](py::tuple state) {  // __setstate__

					CVehicle_sp vehicle = std::make_shared<CVehicle>();
					if (state.size() == 4 && py::isinstance<py::tuple>(state[0]))
					{	// current format: (property tuple, class id, pattern, desc)
						vehicle->setPropByTuple(state[0].cast<py::tuple>());
						vehicle->setClass(Classification(state[1].cast<size_t>(),
							state[2].cast<std::string>(), state[3].cast<std::string>()));
					}
					else
						vehicle->setPropByTuple(state);  // legacy flat property tuple

					return vehicle;
				}
			));


	py::class_<CVehicleClassification, CVehicleClassification_sp> cvehicleclassification(m, "_VehicleClassification");
	py::class_<CVehClassAxle, CVehicleClassification, CVehClassAxle_sp> cvehclassaxle(m, "_VehClassAxle");
		cvehclassaxle.def(py::init<>());
	py::class_<CVehClassPattern, CVehicleClassification, CVehClassPattern_sp> cvehclasspattern(m, "_VehClassPattern");
		cvehclasspattern.def(py::init<>());


	py::class_<CLane, CLane_sp> clane(m, "_Lane");
		clane.def("getNextArrivalTime", &CLane::GetNextArrivalTime);
	py::class_<CLaneFileTraffic, CLane, CLaneFileTraffic_sp> clanefiletraffic(m, "_TrafficLoader");
		clanefiletraffic.def(py::init<>())
			.def("setLaneData", &CLaneFileTraffic::setLaneData, py::arg("dirn"), py::arg("lane_index"))
			.def("addVehicle", &CLaneFileTraffic::addVehicle, py::arg("vehicle"))
			.def("setFirstArrivalTime", &CLaneFileTraffic::setFirstArrivalTime)
			.def("getNoVehicles", &CLaneFileTraffic::GetNoVehicles)
			.def("getNextVehicle", &CLaneFileTraffic::GetNextVehicle, py::return_value_policy::take_ownership);
	py::class_<CLaneGenTraffic, CLane, CLaneGenTraffic_sp> clanegentraffic(m, "_TrafficGenerator");
		clanegentraffic.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("setLaneData", py::overload_cast<CLaneFlowComposition, CVehicleGenerator_sp, CFlowGenerator_sp, const double>(&CLaneGenTraffic::setLaneData), py::arg("lane_flow_composition"), py::arg("vehicle_generator"), py::arg("headway_generator"), py::arg("start_time"))
			.def("initLane", &CLaneGenTraffic::initLane, py::arg("flow_model_data"))
			.def("getNextVehicle", &CLaneGenTraffic::GetNextVehicle, py::return_value_policy::take_ownership);


	py::class_<CLaneFlowComposition> claneflowcomposition(m, "_LaneFlowComposition");
		claneflowcomposition.def(py::init<size_t, size_t, size_t>(), py::arg("lane_index"), py::arg("lane_dir"), py::arg("block_size"))
			.def("addBlockData", &CLaneFlowComposition::addBlockData, py::arg("hourly_data"))
			.def("completeData", &CLaneFlowComposition::completeData);

	py::class_<CGenerator, CGenerator_sp> cgenerator(m, "_Generator");
	py::class_<CModelData, CModelData_sp> cmodeldata(m, "_ModelData");

	py::class_<CVehicleGenerator, CGenerator, CVehicleGenerator_sp> cvehiclegenerator(m, "_VehicleGenerator");
	py::class_<CVehicleGenNominal, CVehicleGenerator, CVehicleGenNominal_sp> cvehiclegennominal(m, "_VehicleGenNominal");
		cvehiclegennominal.def(py::init<CVehModelDataNominal_sp>(), py::arg("model_data"));
	py::class_<CVehicleGenGarage, CVehicleGenerator, CVehicleGenGarage_sp> cvehiclegengarage(m, "_VehicleGenGarage");
		cvehiclegengarage.def(py::init<CVehModelDataGarage_sp>(), py::arg("model_data"));
	py::class_<CVehicleGenGrave, CVehicleGenerator, CVehicleGenGrave_sp> cvehiclegengrave(m, "_VehicleGenGrave");
		cvehiclegengrave.def(py::init<CVehModelDataGrave_sp>(), py::arg("model_data"));

	py::class_<CVehicleModelData, CModelData, CVehicleModelData_sp> cvehiclemodeldata(m, "_VehicleModelData");
	py::class_<CVehModelDataNominal, CVehicleModelData, CVehModelDataNominal_sp> cvehmodeldatanominal(m, "_VehModelDataNominal");
		cvehmodeldatanominal.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition, CVehicle_sp, std::vector<double>>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"), py::arg("nominal_vehicle"), py::arg("COV_list"));
	py::class_<CVehModelDataGarage, CVehicleModelData, CVehModelDataGarage_sp> cvehmodeldatagarage(m, "_VehModelDataGarage");
		cvehmodeldatagarage.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition, std::vector<CVehicle_sp>, std::vector<std::vector<double>>>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"), py::arg("garage"), py::arg("kernel"));
	py::class_<CVehModelDataGrave, CVehicleModelData, CVehModelDataGrave_sp> cvehmodeldatagrave(m, "_VehModelDataGrave");
		cvehmodeldatagrave.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"));

	py::class_<CFlowGenerator, CGenerator, CFlowGenerator_sp> cflowgenerator(m, "_FlowGenerator");
		cflowgenerator.def("_setMaxBridgeLength", &CFlowGenHeDS::setMaxBridgeLength, py::arg("max_bridge_length"));
	py::class_<CFlowGenHeDS, CFlowGenerator, CFlowGenHeDS_sp> cflowgenheds(m, "_FlowGenHeDS");
		cflowgenheds.def(py::init<CFlowModelDataHeDS_sp>(), py::arg("model_data"));
	py::class_<CFlowGenCongested, CFlowGenerator, CFlowGenCongested_sp> cflowgencongested(m, "_FlowGenCongested");
		cflowgencongested.def(py::init<CFlowModelDataCongested_sp>(), py::arg("model_data"));
	py::class_<CFlowGenPoisson, CFlowGenerator, CFlowGenPoisson_sp> cflowgenpoisson(m, "_FlowGenPoisson");
		cflowgenpoisson.def(py::init<CFlowModelDataPoisson_sp>(), py::arg("model_data"));
	py::class_<CFlowGenConstant, CFlowGenerator, CFlowGenConstant_sp> cflowgenconstant(m, "_FlowGenConstant");
		cflowgenconstant.def(py::init<CFlowModelDataConstant_sp>(), py::arg("model_data"));

	py::class_<CFlowModelData, CModelData, CFlowModelData_sp> cflowmodeldata(m, "_FlowModelData");
	py::class_<CFlowModelDataHeDS, CFlowModelData, CFlowModelDataHeDS_sp> cflowmodeldataheds(m, "_FlowModelDataHeDS");
		cflowmodeldataheds.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataCongested, CFlowModelData, CFlowModelDataCongested_sp> cflowmodeldatacongested(m, "_FlowModelDataCongested");
		cflowmodeldatacongested.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataPoisson, CFlowModelData, CFlowModelDataPoisson_sp> cflowmodeldatapoisson(m, "_FlowModelDataPoisson");
		cflowmodeldatapoisson.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataConstant, CFlowModelData, CFlowModelDataConstant_sp> cflowmodeldataconstant(m, "_FlowModelDataConstant");
		cflowmodeldataconstant.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));


	py::class_<CVehicleTrafficFile> cvehicletrafficfile(m, "_VehicleTrafficFile");
		cvehicletrafficfile.def(py::init<CVehicleClassification_sp, bool, bool, double>(), py::arg("vehicle_classification"), py::arg("use_const_speed"), py::arg("use_average_speed"), py::arg("const_speed_value"))
			.def("read", &CVehicleTrafficFile::Read, py::arg("file"), py::arg("format"))
			.def("assignTraffic", &CVehicleTrafficFile::AssignTraffic, py::arg("vehicle_list"))
			.def("getNoDays", &CVehicleTrafficFile::getNoDays)
			.def("getNoLanes", &CVehicleTrafficFile::getNoLanes)
			.def("getNoDirn", &CVehicleTrafficFile::getNoDirn)
			.def("getNoLanesDir1", &CVehicleTrafficFile::getNoLanesDir1)
			.def("getNoLanesDir2", &CVehicleTrafficFile::getNoLanesDir2)
			.def("getNoVehicles", &CVehicleTrafficFile::getNoVehicles)
			.def("getNextVehicle", &CVehicleTrafficFile::getNextVehicle, py::return_value_policy::take_ownership)
			.def("getVehicles", &CVehicleTrafficFile::getVehicles, py::return_value_policy::take_ownership);

	py::class_<CVehicleBuffer> cvehiclebuffer(m, "_VehicleBuffer");
		cvehiclebuffer.def(py::init<CConfigDataCore&, CVehicleClassification_sp, double>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("start_time"))
			.def("addVehicle", &CVehicleBuffer::AddVehicle, py::arg("vehicle"))
			.def("flushBuffer", py::overload_cast<>(&CVehicleBuffer::FlushBuffer))
			.def("flushBuffer", py::overload_cast<double>(&CVehicleBuffer::FlushBuffer), py::arg("sim_end_time"), "Flush, filling silent trailing FlowData hours up to the simulated end time.");


	py::class_<CMultiModalNormal> cmultimodalnormal(m, "_MultiModalNormal",
		"A mixture of one or more normal modes, each with its own weight, mean and "
		"standard deviation. Build it with add_mode(), then sample it via "
		"Distribution.gen_multimodalnormal.");
		cmultimodalnormal.def(py::init<>())
			.def("add_mode", &CMultiModalNormal::AddMode,
				R"(
				Append one normal mode to the mixture.

				Parameters
				----------
				w : float
					Mode weight. Weights should sum to 1 across all modes; a mode
					is drawn with probability proportional to its weight.
				m : float
					Mode mean, in the sampled quantity's native unit.
				s : float
					Mode standard deviation, in the sampled quantity's native unit.
				)",
				py::arg("w"), py::arg("m"), py::arg("s"))
			.def("get_no_modes", &CMultiModalNormal::getNoModes,
				"Get the number of modes currently in the mixture.");
	py::class_<CDistribution> cdistribution(m, "_Distribution",
		"A family of random-variate generators sharing configurable location, scale "
		"and shape parameters and the process-wide RNG. Each gen_* method draws one "
		"sample from the corresponding distribution using the currently set "
		"parameters (or explicit arguments where provided). Values are in the sampled "
		"quantity's native unit.");
		cdistribution.def(py::init<>())
			.def(py::init<double, double, double>(),
				"Construct with explicit location (loc), scale (scale) and shape "
				"(shape) parameters. shape is only used by gen_gev.",
				py::arg("loc"), py::arg("scale"), py::arg("shape"))
			.def("set_shape", &CDistribution::setShape,
				"Set the shape parameter (used by gen_gev).", py::arg("shape"))
			.def("set_scale", &CDistribution::setScale,
				"Set the scale parameter.", py::arg("scale"))
			.def("set_location", &CDistribution::setLocation,
				"Set the location parameter.", py::arg("loc"))
			.def("get_shape", &CDistribution::getShape, "Get the shape parameter.")
			.def("get_scale", &CDistribution::getScale, "Get the scale parameter.")
			.def("get_location", &CDistribution::getLocation, "Get the location parameter.")
			.def("gen_uniform", &CDistribution::GenerateUniform,
				"Draw one uniform sample from [0, 1). Ignores location/scale/shape.")
			.def("gen_normal", py::overload_cast<>(&CDistribution::GenerateNormal),
				"Draw one normal sample with mean = location and standard deviation = scale.")
			.def("gen_normal", py::overload_cast<double,double>(&CDistribution::GenerateNormal),
				"Draw one normal sample with the given mean (mean) and standard "
				"deviation (stdev).",
				py::arg("mean"), py::arg("stdev"))
			.def("gen_multimodalnormal", &CDistribution::GenerateMultiModalNormal,
				R"(
				Draw one sample from a multi-modal normal mixture: pick a mode
				weighted by its weight, then draw from that mode's normal. Uses the
				mixture's own parameters, not this object's location/scale/shape.

				Parameters
				----------
				mmn : MultiModalNormal
					The mixture to sample from.
				)",
				py::arg("mmn"))
			.def("gen_exponential", &CDistribution::GenerateExponential,
				"Draw one exponential sample shifted by location, with scale = scale "
				"(the mean of the exponential part equals scale).")
			.def("gen_lognormal", &CDistribution::GenerateLogNormal,
				"Draw one lognormal sample whose underlying normal (in log space) has "
				"mean = location and standard deviation = scale.")
			.def("gen_gamma", &CDistribution::GenerateGamma,
				"Draw one gamma sample parameterised by location and scale.")
			.def("gen_gumbel", &CDistribution::GenerateGumbel,
				"Draw one Gumbel (extreme-value type I) sample with mode = location "
				"and dispersion controlled by scale.")
			.def("gen_poisson", &CDistribution::GeneratePoisson,
				"Draw one (normal-approximated) Poisson sample with mean = location "
				"and variance = scale.")
			.def("gen_gev", &CDistribution::GenerateGEV,
				"Draw one Generalised Extreme Value sample using location, scale and shape.")
			.def("gen_triangular", py::overload_cast<>(&CDistribution::GenerateTriangular),
				"Draw one symmetric triangular sample centred at location with "
				"half-width = scale.")
			.def("gen_triangular", py::overload_cast<double,double>(&CDistribution::GenerateTriangular),
				"Draw one symmetric triangular sample centred at loc with half-width w.",
				py::arg("loc"), py::arg("w"));
};

