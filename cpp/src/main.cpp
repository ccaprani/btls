// main.cpp
// the main file for the BridgeTrafficLoadSim Build

#include "PrepareSim.h"
#include <exception>
#include <iostream>


int main()
{
	// The engine reports unrecoverable input errors by throwing, so that the
	// Python bindings can surface them as exceptions. The standalone binary
	// has to turn them back into a message and a non-zero exit status;
	// letting them escape main() would abort through std::terminate.
	try
	{
		return run("BTLSin.txt");
	}
	catch (const std::exception& e)
	{
		std::cerr << "*** ERROR: " << e.what() << std::endl;
		return 1;
	}
}
