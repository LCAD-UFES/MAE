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
#define IMAGE_WIDTH (691)
#define IMAGE_HEIGHT (219)
#define NL_WIDTH (100)
#define NL_HEIGHT (48)
#define INPUTS_PER_NEURON (256)
#define GAUSSIAN_RADIUS (5.0)
#define LOG_FACTOR (2.0)
#define BAND_WIDTH (0.125)
#define TRAINNING (0)
#define RUNNING (1)
#define CUT_POINT (0.7)
#define ACT_BAND_WIDTH (8)
#define INPUT_PATH ("TEXT_DETECT/TRAIN_REAL/")

// Macros

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler_max_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" void translate_nl_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT calculate_random_position (PARAM_LIST *);
extern "C" NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetFace (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetRandomFace (PARAM_LIST *);
extern "C" NEURON_OUTPUT Move2Eye (PARAM_LIST *);
extern "C" NEURON_OUTPUT Move2Nose (PARAM_LIST *);
extern "C" NEURON_OUTPUT Move2Mouth (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern "C" NEURON_OUTPUT update_input_filters (PARAM_LIST *);

// Global Variables
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" float sparse_percentage;
extern "C" float HIGHEST_OUTPUT;
extern "C" NEURON_LAYER nl_v1_activation_map;
extern "C" NEURON_LAYER nl_v1_activation_map_f;
extern "C" NEURON_LAYER nl_target_coordinates;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_filtered;
extern "C" NEURON_LAYER in_pattern_filtered_translated;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC in_pattern_filtered_translated_out;
extern "C" OUTPUT_DESC out_v1_activation_map;
extern "C" OUTPUT_DESC out_target_coordinates;
extern "C" OUTPUT_DESC out_v1_activation_map_no_filters;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_max_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void gaussian_filter (FILTER_DESC *);
extern void translate_nl_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT calculate_random_position (PARAM_LIST *);
extern NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern NEURON_OUTPUT GetFace (PARAM_LIST *);
extern NEURON_OUTPUT GetRandomFace (PARAM_LIST *);
extern NEURON_OUTPUT Move2Eye (PARAM_LIST *);
extern NEURON_OUTPUT Move2Nose (PARAM_LIST *);
extern NEURON_OUTPUT Move2Mouth (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT update_input_filters (PARAM_LIST *);

// Global Variables
float translation_filter_deltaX;
float translation_filter_deltaY;
float sparse_percentage;
float HIGHEST_OUTPUT;
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

#endif
