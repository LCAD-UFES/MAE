#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifndef WINDOWS
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include "forms.h"
#include "mae_control.h"
#include "ir.hpp"
#endif

#include "filter.h"
#include "mapping.h"
#include "../../../src/libstereo/src/libtsai/cal_main.h"
#include "../robot.h"
#include "viewer.hpp"
#include "simulator.hpp"
#include "fuzzy.hpp"
#include "visual_search.h"

// Definitions
#define INVALID_SOCKET			-1

#define ST_WAIT				0
#define ST_GET_IMAGE			1

#define MSG_SNAP_MONO_256		"snap_mono_256"
#define MSG_SNAP_332			"snap_332"
#define MSG_SNAP_MONO_8			"snap_mono_8"

#define COLOR_MONO_256			0
#define COLOR_332			1
#define COLOR_MONO_8			2

#define INPUT_IMAGE_LEFT		0
#define INPUT_IMAGE_RIGHT		1

#define MAX_PACKAGE			1000000

#define VERT_GAP			0

#define NUM_SAMPLES				5

// Macros
#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}
#define closesocket(s)			close(s);

// Structs
typedef int SOCKET;

typedef struct
{
	double val;
	double conf;	// confidence rate
	int pos;
} SAMPLES;
	
typedef struct
{
	char *neg_slope;
	SAMPLES **samples;
} DISP_DATA;

// Global Variables
struct camera_parameters cameraLeftCalibrationParameters;
struct camera_parameters cameraRightCalibrationParameters;
struct calibration_constants cameraLeftCalibrationConstants;
struct calibration_constants cameraRightCalibrationConstants;

#endif
