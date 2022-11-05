// ConfigData.h: interface for the CConfigDataCore class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CSVParse.h"

///////////// CConfigDataCore ///////////////

class CConfigDataCore
{

public:
    CConfigDataCore();
    ~CConfigDataCore() {};

    bool ReadData(std::string inFile);

    struct Mode_Config
    {
        size_t PROGRAM_MODE;
    } Mode = {1};

    struct Road_Config
    {
        std::string LANES_FILE;
        size_t NO_LANES_DIR1; // assigned in program
        size_t NO_LANES_DIR2; // assigned in program
        size_t NO_LANES;      // assigned in program
        size_t NO_DIRS;       // assigned in program
    } Road = {"laneflow.csv", 1, 1, 2, 2};

    struct Gen_Config
    {
        std::string TRAFFIC_FOLDER;
        bool GEN_TRAFFIC;
        size_t NO_DAYS;
        double TRUCK_TRACK_WIDTH;
        double LANE_ECCENTRICITY_STD;
        int KERNEL_TYPE;
        double NO_OVERLAP_LENGTH; // assigned in program   
    } Gen = {"../Traffic/Auxerre/", false, 1, 190.0, 0.0, 0, 100.0};

    struct Read_Config
    {
        bool READ_FILE;
        std::string TRAFFIC_FILE;
        std::string GARAGE_FILE;
        std::string KERNEL_FILE;
        std::string NOMINAL_FILE;
        size_t FILE_FORMAT;
        bool USE_CONSTANT_SPEED;
        bool USE_AVE_SPEED;
        double CONST_SPEED;
    } Read = {false, "traffic.txt", "garage.txt", "kernel.csv", "constant_vehicle.csv", 4, false, false, 0.0};

    struct Traffic_Config
    {
        int VEHICLE_MODEL;
        int HEADWAY_MODEL;
        int CLASSIFICATION;
        double CONGESTED_SPACING;
        double CONGESTED_SPEED;
        double CONGESTED_GAP; // assigned automatically (later)
        double CONGESTED_GAP_COEF_VAR;
        double CONSTANT_SPEED;
        double CONSTANT_GAP;
    } Traffic = {0, 0, 1, 5.0, 30.0, 5.0, 0.05, 36.0, 10.0};

    struct Sim_Config
    {
        bool CALC_LOAD_EFFECTS;
        std::string BRIDGE_FILE;
        std::string INFLINE_FILE;
        std::string INFSURF_FILE;
        double CALC_TIME_STEP;
        size_t MIN_GVW;
    } Sim = {false, "bridges.txt", "IL.txt", "IS.csv", 0.1, 35};

    struct Output_Config
    {
        bool WRITE_TIME_HISTORY;
        bool WRITE_EACH_EVENT;
        size_t WRITE_EVENT_BUFFER_SIZE;
        bool WRITE_FATIGUE_EVENT;

        struct VehicleFile_Config
        {
            bool WRITE_VEHICLE_FILE;
            size_t FILE_FORMAT;
            std::string VEHICLE_FILENAME;
            size_t WRITE_VEHICLE_BUFFER_SIZE;
            bool WRITE_FLOW_STATS;
        } VehicleFile;

        struct BlockMax_Config
        {
            bool WRITE_BM;
            bool WRITE_BM_VEHICLES;
            bool WRITE_BM_SUMMARY;
            bool WRITE_BM_MIXED;
            size_t BLOCK_SIZE_DAYS;
            size_t BLOCK_SIZE_SECS;
            size_t WRITE_BM_BUFFER_SIZE;
        } BlockMax;

        struct POT_Config
        {
            bool WRITE_POT;
            bool WRITE_POT_VEHICLES;
            bool WRITE_POT_SUMMARY;
            bool WRITE_POT_COUNTER;
            size_t POT_COUNT_SIZE_DAYS;
            size_t POT_COUNT_SIZE_SECS;
            size_t WRITE_POT_BUFFER_SIZE;
        } POT;

        struct Stats_Config
        {
            bool WRITE_STATS;
            bool WRITE_SS_CUMULATIVE;
            bool WRITE_SS_INTERVALS;
            size_t WRITE_SS_INTERVAL_SIZE;
            size_t WRITE_SS_BUFFER_SIZE;
        } Stats;

        struct Fatigue_Config
        {
            bool DO_FATIGUE_RAINFLOW;
            int RAINFLOW_DECIMAL;
            double RAINFLOW_CUTOFF;
            size_t WRITE_RAINFLOW_BUFFER_SIZE;
        } Fatigue;

    } Output = {false, false, 10000, false, 
                {false, 4, "vehicles.txt", 10000, false},   // VehicleFile_Config
                {false, false, false, false, 1, 0, 10000},  // BlockMax_Config
                {false, false, false, false, 1, 0, 10000},  // POT_Config
                {false, false, false, 3600, 10000},         // Stats_Config
                {false, 3, 0.0, 10000}};                    // Fatigue_Config

    struct Time_Config
    {
        size_t DAYS_PER_MT;
        size_t MTS_PER_YR;
        size_t HOURS_PER_DAY;
        size_t SECS_PER_HOUR;
        size_t MINS_PER_HOUR;
        size_t SECS_PER_MIN;
    } Time = {25, 10, 24, 3600, 60, 60};

    void VehGenGrave(std::string lanesFile, std::string trafficFolder, size_t noDays, double truckTrackWidth, double laneEccentricityStd);
    void VehGenGarage(std::string lanesFile, std::string garageFile, size_t fileFormat, std::string kernelFile, size_t noDays, double laneEccentricityStd, int kernelType);
    void VehGenNominal(std::string lanesFile, std::string constantFile, size_t noDays, double laneEccentricityStd, int kernelType);

    void FlowGenNHM(int classification, std::string trafficFolder);
    void FlowGenConstant(int classification, double constantSpeed, double constantGap);
    void FlowGenCongestion(int classification, double congestedSpacing, double congestedSpeed, double congestedGapCOV);
    void FlowGenFreeFlow(int classification);

    void TrafficRead(std::string trafficFile, size_t fileFormat, bool useConstantSpeed, bool useAveSpeed, double constSpeed);

    void Simulation(std::string bridgeFile, std::string infLineFile, std::string infSurfFile, double calcTimeStep, size_t minGVW);

    void OutputGeneral(bool writeTimeHistory = false, bool writeEachEvent = false, bool writeFatigueEvent = false, size_t writeBufferSize = 10000);
    void OutputVehicleFile(bool writeVehicleFile = false, size_t vehicleFileFormat = 4, std::string vehicleFileName = "Vehicles.txt", size_t bufferSize = 10000, bool writeFlowStats = false);
    void OutputBlockMax(bool writeBM = false, bool writeVehicle = false, bool writeSummary = false, bool write_mixed = false, size_t blockSizeDays = 1, size_t blockSizeSecs = 0, size_t bufferSize = 10000);
    void OutputPOT(bool writePOT = false, bool writeVehicle = false, bool writeSummary = false, bool writeCounter = false, size_t POTSizeDays = 1, size_t POTSizeSecs = 0, size_t bufferSize = 10000);
    void OutputStats(bool writeStats = false, bool writeCumulative = false, bool writeIntervals = false, size_t intervalSize = 3600, size_t bufferSize = 10000);
    void OutputFatigue(bool doRainflow = false, int decimal = 3, double cutOffValue = 0.0, size_t bufferSize = 10000);


private:
    CCSVParse m_CSV;
    std::string m_CommentString;

    void doDerivedConstants();
    void ExtractData();
    std::string GetNextDataLine();

};


///////////// CConfigData ///////////////

class CConfigData : public CConfigDataCore
{
    // Using Singleton pattern here:
    // https://stackoverflow.com/questions/1008019/c-singleton-design-pattern/1008289#1008289
public:
    static CConfigData& get()
    {
        static CConfigData instance; // Guaranteed to be destroyed.
        return instance;             // Instantiated on first use.
    }

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.
    CConfigData(CConfigData const &) = delete;
    void operator=(CConfigData const &) = delete;
    // Note: Scott Meyers mentions in his Effective Modern
    //       C++ book, that deleted functions should generally
    //       be public as it results in better error messages
    //       due to the compilers behavior to check accessibility
    //       before deleted status

private:
    CConfigData() {};
};