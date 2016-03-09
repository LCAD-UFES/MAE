#ifndef __ROBOT_NL_FUNCTIONS_H
#define __ROBOT_NL_FUNCTIONS_H

#include "mae.h"

/*
Se��o de Vari�veis Exportadas
*/

extern double g_dblNearVergenceCutPlane;

extern double g_dblFarVergenceCutPlane;

extern int g_nCurrDisparity;

/*
Se��o de Tipos de Dados
*/

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

/*
Se��o de Fun��es
*/

void robot_check_input_bounds (INPUT_DESC *input, int wx, int wy);

void rectify_all_images();

void update_simple_mono_cells_filters ();

void add_subpixel_disparity_to_disparity_map_plan (void);

long compute_horizontal_offset();

void alloc_and_init_generate_disparity_map_plan_variables(
	DISP_DATA **p_dispData,
	float **p_dispAux,
	NEURON_LAYER* gaussian_map,
	NEURON_LAYER* disparity_map);

void generate_disparity_map_plan(NEURON_LAYER* gaussian, NEURON_LAYER* gaussian_map, NEURON_LAYER* disparity_map, char* prefix);

void free_generate_disparity_map_plan_variables (DISP_DATA *dispData, float *dispAux);

void filter_find_update(char* prefix);


#endif
