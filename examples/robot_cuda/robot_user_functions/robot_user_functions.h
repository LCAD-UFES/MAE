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
#include "cuda_functions.h"
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

#define NUM_SAMPLES				5

// Macros
#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}
#define closesocket(s)			close(s);

// Structs
typedef int SOCKET;

typedef struct
{
	double val;
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

// Prototypes of CUDA functions
void cuda_biological_gabor_nls (float *d_s_m, int *d_xi, int *d_yi, int *d_image_vector, int *xi, int *yi, int wi, int hi,
				int size, int rf_num_points, int *rf_points_x, int *rf_points_y, int *image_vector,
				float *rf_points_w, float *rf_points_g, float global_factor, float c_delta_area, float teste);
void cuda_biological_gabor_nls_old (float *s_m, int *xi, int *yi, int wi, int hi, int size, int rf_num_points, int *rf_points_x,
					int *rf_points_y, int *image_vector, float *rf_points_w, float *rf_points_g,
					float global_factor, float c_delta_area, float teste);
void cuda_add_nls (NEURON *C, NEURON *A, NEURON *B, int size);
void cuda_mult_nls (NEURON *C, NEURON *A, NEURON *B, int size);
void cuda_div_nls (NEURON *D, NEURON *A, NEURON *B, NEURON *C, int size, float k_param);
void cuda_bigfilter_nls (float *E, float *A, float *B, float *C, float *D, int size, float k_param);
void cuda_gaussian_nls (float *mt_gaussian, float *d_mt_gaussian, float *d_mt, int *rf_points_x, int *rf_points_y, 
			float *rf_points_g, int num_points, int wo, int ho, int wi, int hi);
void cuda_gaussian_nls_old (float *mt_gaussian, float *mt, int *rf_points_x, int *rf_points_y, float *rf_points_g,
				int num_points, int wo, int ho, int wi, int hi);
void cuda_v1_mt_nls (float *mt_gaussian, int *rf_points_x, int *rf_points_y, float *rf_points_g, int num_points,
			int wo, int ho, int wi, int hi, float *s_r, float *s_r_q, float *s_l, float *s_l_q,
			int num_neurons, float k);
void cuda_alloc_device_data_structures (V1_MT_PRIVATE_STATE *v1_mt_private_state, int wi, int hi, int size);
void cuda_map_v1_to_image (int *xi, int *yi, int wi, int hi, int w, int h, int x_center, int y_center, float correction,
				float log_factor, int shift);
#endif
