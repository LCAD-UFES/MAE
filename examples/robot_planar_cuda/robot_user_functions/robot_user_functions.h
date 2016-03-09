#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H


// Includes
#ifndef WINDOWS
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "forms.h"
#include "mae_control.h"
#else
#include <time.h>
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filter.h"
#include "cuda_functions.h"
#include "mapping.h"
#include "../../../src/libtsai/cal_main.h"
#include "../../../src/libstereo/include/stereo_api.hpp"
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
typedef int ROBOT_SOCKET;

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

extern int setverg_cont;

// Global Variables
struct camera_parameters cameraLeftCalibrationParameters;
struct camera_parameters cameraRightCalibrationParameters;
struct calibration_constants cameraLeftCalibrationConstants;
struct calibration_constants cameraRightCalibrationConstants;
void copyneuron2floatvec (float *float_vec, NEURON *neuron_vec, int size);

void copyfloatvec2neuron (NEURON *neuron_vec, float *float_vec, int size);

void cuda_alloc_device_data_structures (V1_MT_PRIVATE_STATE *v1_mt_private_state, int wi, int hi, int num_neurons);

// O filtro CUDA_BIOLOGICAL_GABOR_NLS eh a chamada de kernel do filtro V1_MT
void cuda_biological_gabor_nls (float *d_s_m,int *d_image_vector, int wi, int hi,
				int num_neurons, int rf_num_points, int *rf_points_x, int *rf_points_y, int *image_vector,
				float *rf_points_w, float *rf_points_g, float global_factor, float c_delta_area, float teste,int wo, int ho,float min_dog);
// O filtro CUDA_BIOLOGICAL_GABOR_NLS_OLD so pode ser chamado individualmente, na estrutura original
void cuda_biological_gabor_nls_old (float *s_m, int *xi, int *yi, int wi, int hi, int num_neurons, int rf_num_points,
				int *rf_points_x, int *rf_points_y, int *image_vector, float *rf_points_w, float *rf_points_g,
				float global_factor, float c_delta_area, float teste);
// O filtro CUDA_ADD_NLS so pode ser chamado individualmente, na estrutura original
void cuda_add_nls (NEURON *C, NEURON *A, NEURON *B, int size);

// O filtro CUDA_MULT_NLS so pode ser chamado individualmente, na estrutura original
void cuda_mult_nls (NEURON *C, NEURON *A, NEURON *B, int size);

void cuda_div_nls (NEURON *D, NEURON *A, NEURON *B, NEURON *C, int size, float k_param);

// O filtro CUDA_BIGFILTER_NLS eh a chamada de kernel do filtro V1_MT
void cuda_bigfilter_nls (float *d_mt, float *d_s_r, float *d_s_r_q, float *d_s_l, float *d_s_l_q, int num_neurons, float k);
// O filtro CUDA_BIGFILTER_NLS_OLD so pode ser chamado individualmente, na estrutura original
void cuda_bigfilter_nls_old (float *mt, float *s_r, float *s_r_q, float *s_l, float *s_l_q, int num_neurons, float k);

// O filtro CUDA_GAUSSIAN_NLS eh a chamada de kernel do filtro V1_MT
void cuda_gaussian_nls (float *mt_gaussian, float *d_mt_gaussian, float *d_mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi);

// O filtro CUDA_GAUSSIAN_NLS_OLD so pode ser chamado individualmente, na estrutura original
void cuda_gaussian_nls_old (float *mt_gaussian, float *mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi);


void 
cuda_biological_gabor_nls_right (float *d_s_r, float *d_s_r_q,
				 int *d_image_vector, 
				 int wi, int hi, int num_neurons, 
				 int rf_num_points, 
				 int *rf_points_x, 
				 int *rf_points_y, 
				 int *rf_points_x_q, 
				 int *rf_points_y_q,
				 int *image_vector,
				 float *rf_points_w, 
				 float *rf_points_g, 
				 float *rf_points_w_q, 
				 float *rf_points_g_q, 
				 float global_factor, 
				 float c_delta_area, float teste,int wo, int ho,
				 float min_dog,float min_dog_q);

cuda_biological_gabor_plus_bigfilter_nls (float *mt, float *d_s_r, float *d_s_r_q, float *d_s_l, float *d_s_l_q,
				 int *d_image_vector, 
				 int wi, int hi, int num_neurons, 
				 int rf_num_points, 
				 int *rf_points_x, 
				 int *rf_points_y, 
				 int *rf_points_x_q, 
				 int *rf_points_y_q,
				 int *image_vector,
				 float *rf_points_w, 
				 float *rf_points_g, 
				 float *rf_points_w_q, 
				 float *rf_points_g_q, 
				 float global_factor, 
				 float c_delta_area, float teste,int wo, int ho,
				 float min_dog,float min_dog_q,float k);
void 
cuda_biological_gabor_nls_left (float *d_s_r, float *d_s_r_q,
				 int *d_image_vector, 
				 int wi, int hi, int num_neurons, 
				 int rf_num_points, 
				 int *rf_points_x, 
				 int *rf_points_y, 
				 int *rf_points_x_q, 
				 int *rf_points_y_q,
				 int *image_vector,
				 float *rf_points_w, 
				 float *rf_points_g, 
				 float *rf_points_w_q, 
				 float *rf_points_g_q, 
				 float global_factor, 
				 float c_delta_area, float teste,int wo, int ho,
				 float min_dog,float min_dog_q, int d_g_nCurrDisparity,
				 int *I_translated_image_left);

// Exportable Variables
extern double g_dblNearVergenceCutPlane;
extern double g_dblFarVergenceCutPlane;

#endif
