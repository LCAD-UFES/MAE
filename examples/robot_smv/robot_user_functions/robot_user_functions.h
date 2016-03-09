#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H

// Includes
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
//#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "filter.h"
#include "mapping.h"
#include "../../../src/libstereo/src/libtsai/cal_main.h"
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

#define NUM_SAMPLES				5

// Macros
#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}
#define closesocket(s)			close(s);

#ifndef M_PI
#define M_PI                            3.1415926535897932384626433832795f
#endif

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

// Prototypes
extern void load_cp_cc_data (FILE *fp, struct camera_parameters *cp, struct calibration_constants *cc);
XY_PAIR calculateVergencePoint (XY_PAIR g_pInputRightPoint);
WORLD_POINT calculateWorldPoint (IMAGE_COORDINATE distortedLeftPoint, IMAGE_COORDINATE distortedRightPoint);
// Global Variables
struct camera_parameters cameraLeftCalibrationParameters;
struct camera_parameters cameraRightCalibrationParameters;
struct calibration_constants cameraLeftCalibrationConstants;
struct calibration_constants cameraRightCalibrationConstants;
int g_nCorrection;

#endif
