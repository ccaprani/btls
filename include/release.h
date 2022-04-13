
//////// RELEASE HISTORY ///////////////
// After years of development, it is now released to:
// - ETH Zurich - Mark Treacy, PhD Student
// - IFSTTAR - Anil Abdoulhousan, Intern from ENTPE
// This release version is known as v1.0 - 5 July 2012

// v1.0.1 - 21 July 2012
// - Fixed bug in AllEvents output in which unfinished blocks at the end of the simualtion are not output.
// - added version number on screen

// v1.0.2 - 24 July 2012
// - Added a FatigueEvents output file type with max and min values of loading events in it. Only output if AllEvents is output - temp

// v1.0.3 - 27 July
// - Bug fix: truck departures not always correctly calculated - fixed using 1e300 for timeOff variables.
// - Bug fix: reading single lane vehicle files caused crash - fixed.
// - Minor console output changes for more user information

// v 1.0.4 - 13 August 2012
// - Console output for missing files.
// - Bug fix: reading multiple lane vehicle files crashed following v1.0.3 fix for single lanes!

// v 1.0.5 - 27 August 2012
// - Separate discrete influence lines now possible for each lane (IL option 2 in bridge definition file).

// v 1.1.0 - 27 October 2012
// - Peaks over Threshold output
// - Basic statistics output
// - Created inheritance structure for output types
// - Flow Data statistics output
// - Restructured the BTLSin file
// - Fixed some bugs, especially one on flow generation
// - Renamed output files for consistency
// - Applied the min GVW parameter

// v 1.2.0 - 28 November 2012
// - Added influence surface calculations
// - Major alterations to BTLSin file
// - Program can now run without IL or IS files being present in folder, once not required.
// - Added POT counter file output and input specs in BTLSin
// - Added DITIS file type including wheel track width for each axle
// - Added option for vehicle output file format
// - Traffic folder location now can be specified in BTLSin
// - Updated generation of tri-modal-normal distributions to include deterministic and single-generation value
// - Added transverse position in lane variability through BTLSin
// - Added transverse axle track width generation file (ATW.csv) input and default value options

// v 1.2.1 - 20 December 2012
// - Bug fix: influence surface calculations did not take transverse position into account for read-in vehicles – now fixed.
// - Bug fix: crash caused by traffic files with no vehicles in a lane is fixed.
