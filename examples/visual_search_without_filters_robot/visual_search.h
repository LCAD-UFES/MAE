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
#define IMAGE_WIDTH (640)
#define IMAGE_HEIGHT (480)
#define NL_WIDTH (63)
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
extern void output_handler_max_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void gaussian_filter (FILTER_DESC *);
extern void translate_nl_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT GetLeft (PARAM_LIST *);
extern NEURON_OUTPUT GetRight (PARAM_LIST *);
extern NEURON_OUTPUT update_input_filters (PARAM_LIST *);

// Global Variables
float translation_filter_deltaX;
float translation_filter_deltaY;
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_v1_activation_map_f;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_filtered;
NEURON_LAYER in_pattern_filtered_translated;
INPUT_DESC in_pattern;
OUTPUT_DESC in_pattern_filtered_translated_out;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_v1_activation_map_no_filters;
#endif
