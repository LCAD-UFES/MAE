/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Refitting file for new dynamic library API. 
*/

#include "smv_api.h"

#include <stdio.h>
#include <stdlib.h>

#include <exception>
using std::exception;

#include "logging.hpp"
using mae::logging::report;
using mae::logging::trace;

#include "neuron_network.hpp"

#include "robot_smv.hpp"

/*
Function Section
*/

DLLEXPORT int MaeInitialize()
{
	trace("MaeInitialize");
	try
	{
		NeuronNetwork& network = NeuronNetwork::getInstance();
		network.start();
		return MAE_OK;
	}
	catch (RobotSMVException& e)
	{
		report(e);
		return e.getCode();
	}
	catch (exception& e)
	{
		report(e);
		return MAE_UNKNOWN_ERROR;
	}
	catch (...)
	{
		report("Unknown error");
		return MAE_UNKNOWN_ERROR;
	}
}

DLLEXPORT int MaeQuit()
{
	trace("MaeQuit");

	return (0);
}

DLLEXPORT int MaeRectify()
{
	trace("MaeRectify");
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.extractStereoRegion("image_camera_right.ppm");
		return MAE_OK;
	}
	catch (RobotSMVException& e)
	{
		report(e);
		return e.getCode();
	}
	catch (exception& e)
	{
		report(e);
		return MAE_UNKNOWN_ERROR;
	}
	catch (...)
	{
		report("Unknown error");
		return MAE_UNKNOWN_ERROR;
	}
}

DLLEXPORT int MaeComputeVolume()
{
	trace("MaeComputeVolume");
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.execute("request.txt");
		return MAE_OK;
	}
	catch (RobotSMVException& e)
	{
		report(e);
		return e.getCode();
	}
	catch (exception& e)
	{
		report(e);
		return MAE_UNKNOWN_ERROR;
	}
	catch (...)
	{
		report("Unknown error");
		return MAE_UNKNOWN_ERROR;
	}
}
