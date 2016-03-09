/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Refitting file for new dynamic library API. 
*/

#include "smv_api.h"

#include <stdio.h>
#include <stdlib.h>

#include "robot_con.h"
#include "robot_smv.hpp"
#include "stereo_api.h"

/*
Definitions Section
*/

#define SMV_API_ERROR_LOG			    "ErrorLogFile.txt"
#define SMV_API_EXECUTION_LOG			"ExecutionLogFile.txt"

#ifndef ERROR
#define ERROR(str1,str2,str3) \
{ \
	FILE *pLogFile = NULL; \
	pLogFile = fopen (SMV_API_ERROR_LOG, "a"); \
	fprintf (pLogFile, "Error: %s%s%s\n", str1, str2, str3); \
	fclose (pLogFile); \
}
#endif

#ifndef MESSAGE
#define MESSAGE(str1,str2,str3) {printf ("%s%s%s\n", str1, str2, str3) }
#endif

#ifndef DEBUG
#define DEBUG(str1,str2,str3) \
{ \
	FILE *pLogFile = NULL; \
	pLogFile = fopen (SMV_API_EXECUTION_LOG, "a"); \
	fprintf (pLogFile, "%s%s%s\n", str1, str2, str3); \
	fclose (pLogFile); \
}
#endif

/*
Initialization Section
*/

DLLEXPORT int MaeInitialize()
{
	robot_con_init();

	DEBUG("MaeInitialize","","");

	return (0);
}

DLLEXPORT int MaeQuit()
{
	DEBUG("MaeQuit","","");

	return (0);
}

DLLEXPORT int MaeRectify(int* image, int size, int* rectified)
{
	RobotSMV& robot = RobotSMV::getInstance();
	try
	{
		robot.stereoRegionRight(image, rectified);
		return MAE_OK;
	}
	catch (RobotSMVException& e)
	{
		ERROR(e.getMessage().c_str(), "", "");
		return e.getCode();
	}
}

DLLEXPORT int MaeComputeVolume(MaeVolumeData* data)
{
	RobotSMV& robot = RobotSMV::getInstance();
	try
	{
		MaeWorkingSpace workingArea = data->workingSpace;
		robot.setWorkingArea(0, workingArea.x0, workingArea.y0, workingArea.xn, workingArea.yn);

		MaeImagePair* images[] = {data->front, data->back};
		for (int i = 0, m = data->type; i < m; i++)
			for (int j = 0; j < 3; j++)
				robot.setImagePair(i, j, images[i][j].left, images[i][j].right);

		MODEL_3D& model3d = robot.computeModel3D();
		data->stereoVolume = model3d.stereoVolume;
		data->solidVolume  = model3d.solidVolume;
		data->length       = model3d.length;
		data->height       = model3d.height;
		data->width        = model3d.width;
		
		return MAE_OK;
	}
	catch (RobotSMVException& e)
	{
		ERROR(e.getMessage().c_str(), "", "");
		return e.getCode();
	}
}
