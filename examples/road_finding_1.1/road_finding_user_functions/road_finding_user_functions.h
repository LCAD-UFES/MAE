#ifndef _ROAD_FINDING_USER_FUNCTIONS_H
#define _ROAD_FINDING_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mae.h"

#ifdef MAE_EXPORT
#include "road_finding.h"
#else
#include "../road_finding.h"
#endif

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define MIN_PERSON_ID 		0

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

// For conditional C++ linkage
#ifdef __cplusplus
extern "C" {
#endif 

// Types

// Prototypes
void	set_neuron_layer_band_float (NEURON_LAYER *,int,int,int,int,float,float);
void	set_neuron_layer_band_int (NEURON_LAYER *,int,int,int,int,int,int);
void	set_neuron_layer_band_int_inside_only (NEURON_LAYER *neuron_layer,int,int,int,int,int);

int	write_neuron_layer_with_gaussian_algorithm_equivalent(NEURON_LAYER *neuron_layer,char *strFileName);

//void	init_road_finding(INPUT_DESC *input);
//int	init_user_functions (void);
//void	input_generator (INPUT_DESC *input, int status);

void	draw_output (char *strOutputName, char *strInputName);
void	f_keyboard (char *key_value);

// Exportable Variables
extern int g_nStatus;

#ifdef __cplusplus
}
#endif

#endif
