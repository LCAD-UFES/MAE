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
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "forms.h"
#include "mae_control.h"
#include "filter.h"
#include "mapping.h"
#include "jpeglib.h"
#include "mpeg2.h"
#include "mpeg2convert.h"
#include "../../../src/libstereo/src/libtsai/cal_main.h"
#include "../robot.h"

// Definitions
#define STIMULUS_LEFT			0
#define STIMULUS_RIGHT			1

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

#define MAX_PACKAGE			100000

#define VERT_GAP			0

#define MAP_WINDOW_WIDTH		400
#define MAP_WINDOW_HEIGHT		300

// Igual a FOCAL_DISTANCE
#define LAMBDA				400.0

// Distancia maxima (para visualizacao apenas) no eixo Z
#define MAX_Z_VIEW			1000.0

#define FILE_TMAP			"tmap.dat"
#define FILE_TMAP_ASCII			"tmap_ascii.dat"

#define POINT_SIZE			2

//#define LOG_FACTOR			2.0

// Tamanho do robo em centimetros (diametro)
#define ROBOT_SIZE			35.0

#define NUM_SAMPLES			5

#define MOTION_STEP			4
#define MOTION_RADIUS			8
#define MOTION_MAX_RADIUS		(MOTION_STEP * MOTION_RADIUS)
#define MOTION_DIRECTIONS		8
#define MOTION_HORIZ			{ 1, 1, 0, -1, -1, -1,  0,  1 }
#define MOTION_VERT			{ 0, 1, 1,  1,  0, -1, -1, -1 }
#define MOTION_MAX_SAMPLES		(MOTION_DIRECTIONS * MOTION_RADIUS + 1)

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
	int vpos;
} SAMPLES;
	
typedef struct _disp_data
{
	NEURON_LAYER *mt_gaussian;
	NEURON_LAYER *confidence;
	INPUT_DESC *left;
	INPUT_DESC *right;
	INPUT_DESC *current;
	INPUT_DESC *next;
	int wxd;
	int wyd;
	int left_vergence;
	int motion_vergence;
	int max_sample;
	int num_samples;
	int num_neurons;
	SAMPLES **samples;
} DISP_DATA;

typedef struct _disp_data_list
{
	DISP_DATA *DispData;
	struct _disp_data_list *next;
} DISP_DATA_LIST;

// Global Variables
struct camera_parameters cameraLeftCalibrationParameters;
struct camera_parameters cameraRightCalibrationParameters;
struct calibration_constants cameraLeftCalibrationConstants;
struct calibration_constants cameraRightCalibrationConstants;

#endif
