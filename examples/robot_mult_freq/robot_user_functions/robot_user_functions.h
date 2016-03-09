#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H

// Includes
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include "forms.h"
#include "mae_control.h"
#include "filter.h"
#include "mapping.h"
#include "../../../src/libtsai/cal_main.h"
#include "../robot.h"

// Definitions
#define STIMULUS_LEFT			0
#define STIMULUS_RIGHT			1

#define INVALID_SOCKET			-1

#define ST_WAIT					0
#define ST_GET_IMAGE			1

#define MSG_SNAP_MONO_256		"snap_mono_256"
#define MSG_SNAP_332			"snap_332"
#define MSG_SNAP_MONO_8			"snap_mono_8"

#define COLOR_MONO_256			0
#define COLOR_332				1
#define COLOR_MONO_8			2

#define INPUT_IMAGE_LEFT		0
#define INPUT_IMAGE_RIGHT		1

#define MAX_PACKAGE				100000

#define VERT_GAP				0

#define MAP_WINDOW_WIDTH		400
#define MAP_WINDOW_HEIGHT		300

// Igual a FOCAL_DISTANCE
#define LAMBDA					400.0

// Distancia maxima (para visualizacao apenas) no eixo Z
#define MAX_Z_VIEW				1000.0

#define FILE_TMAP				"tmap.dat"
#define FILE_TMAP_ASCII			"tmap_ascii.dat"

#define POINT_SIZE				2

//#define LOG_FACTOR				2.0

// Tamanho do robo em centímetros (diametro)
#define ROBOT_SIZE				35.0

#define NUM_SAMPLES				8

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
