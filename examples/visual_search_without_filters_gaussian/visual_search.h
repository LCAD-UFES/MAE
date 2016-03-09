// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _VISUAL_SEARCH_H
#define _VISUAL_SEARCH_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define FOV_HORIZONTAL (48.0)
#define TARGET_NEURON_LAYER_NAME ("nl_target_coordinates")
#define IMAGE_WIDTH (384)
#define IMAGE_HEIGHT (288)
#define NL_WIDTH (64)
#define NL_HEIGHT (48)
#define INPUTS_PER_NEURON (256)
#define GAUSSIAN_RADIUS (10.0)
#define LOG_FACTOR (2.0)
#define BAND_WIDTH (0.125)
#define TRAINNING (0)
#define RUNNING (1)
#define CUT_POINT (0.7)
#define INPUT_PATH ("/home/hallysson/img/ARDataBase/ARPhotoDataBase/")

// Macros

// Structs

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_mean_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void copy_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern void v1_activation_map_filter (FILTER_DESC *);
extern void map_image_v1 (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT v1_to_image_mapping (PARAM_LIST *);
extern NEURON_OUTPUT calculate_random_position (PARAM_LIST *);
extern NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern NEURON_OUTPUT GetFace (PARAM_LIST *);
extern NEURON_OUTPUT GetRandomFace (PARAM_LIST *);
extern NEURON_OUTPUT Move2Eye (PARAM_LIST *);
extern NEURON_OUTPUT Move2Nose (PARAM_LIST *);
extern NEURON_OUTPUT Move2Mouth (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_v1_activation_map_f;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER nl_aux;
INPUT_DESC in_pattern;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_v1_pattern;
#endif
