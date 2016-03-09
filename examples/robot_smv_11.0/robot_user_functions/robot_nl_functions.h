#ifndef __ROBOT_NL_FUNCTIONS_H
#define __ROBOT_NL_FUNCTIONS_H

#include "mae.h"

/*
Seção de Variáveis Exportadas
*/

extern double g_dblNearVergenceCutPlane;

extern double g_dblFarVergenceCutPlane;

extern int g_nCurrDisparity;

/*
Seção de Tipos de Dados
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
Seção de Funções
*/

void robot_check_input_bounds (INPUT_DESC *input, int wx, int wy);

void rectify_image ();

void update_simple_mono_cells_filters ();

void update_mt_cells_filters ();

void add_subpixel_disparity_to_disparity_map_plan (void);

void alloc_and_init_generate_disparity_map_plan_variables (DISP_DATA **p_dispData, float **p_dispAux);

void generate_disparity_map_plan (void);

void free_generate_disparity_map_plan_variables (DISP_DATA *dispData, float *dispAux);

#endif
