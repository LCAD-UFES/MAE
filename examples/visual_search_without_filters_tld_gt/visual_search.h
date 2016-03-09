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
#define IMAGE_WIDTH_RESIZED (201)
#define IMAGE_HEIGHT_RESIZED (201)
#define INPUTS_PER_NEURON (64)
#define GAUSSIAN_RADIUS (5.0)
#define LOG_FACTOR (2.0)
#define TRAINNING (0)
#define RUNNING (1)
#define CUT_POINT (0.7)
#define IMAGE_WIDTH (290)
#define IMAGE_HEIGHT (217)
#define TRAINING_INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define RECALL_INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define INVALIDS_FRAMES (1268)

// Macros
#define NL_WIDTH (64 * 4 + 1)
#define NL_HEIGHT (48 * 4 + 1)

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler_max_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void translate_nl_filter (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" void map_image_v1 (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImage (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetAndComputeImage (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetNumImages (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImageByNumberImage (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToNearTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToFarTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetDeltaMove (PARAM_LIST *);
extern "C" NEURON_OUTPUT TrainTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern "C" NEURON_OUTPUT ReportParcialResults (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetCriterion (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetProhibitoryTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT SaveState (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetBestProhibitoryTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetBestProhibitoryTrafficSign_varius (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetBestMandatoryTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetTrafficSignOfSpecificSize (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);

// Global Variables
extern "C" int g_kernel_size;
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" float g_sigma;
extern "C" float HIGHEST_OUTPUT;
extern "C" NEURON_LAYER nl_v1_activation_map;
extern "C" NEURON_LAYER nl_v1_activation_map_f;
extern "C" NEURON_LAYER nl_target_coordinates;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC nl_v1_pattern_out;
extern "C" OUTPUT_DESC out_v1_activation_map;
extern "C" OUTPUT_DESC out_target_coordinates;
extern "C" OUTPUT_DESC out_in_pattern_filtered_translated;
extern "C" OUTPUT_DESC out_v1_activation_map_no_filters;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_max_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void translate_nl_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern void map_image_v1 (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern NEURON_OUTPUT GetImage (PARAM_LIST *);
extern NEURON_OUTPUT GetAndComputeImage (PARAM_LIST *);
extern NEURON_OUTPUT GetNumImages (PARAM_LIST *);
extern NEURON_OUTPUT GetImageByNumberImage (PARAM_LIST *);
extern NEURON_OUTPUT MoveToTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT MoveToNearTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT MoveToFarTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern NEURON_OUTPUT GetDeltaMove (PARAM_LIST *);
extern NEURON_OUTPUT TrainTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern NEURON_OUTPUT ReportParcialResults (PARAM_LIST *);
extern NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern NEURON_OUTPUT SetCriterion (PARAM_LIST *);
extern NEURON_OUTPUT GetProhibitoryTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT SaveState (PARAM_LIST *);
extern NEURON_OUTPUT GetBestProhibitoryTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT GetBestProhibitoryTrafficSign_varius (PARAM_LIST *);
extern NEURON_OUTPUT GetBestMandatoryTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT GetTrafficSignOfSpecificSize (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);

// Global Variables
int g_kernel_size;
float translation_filter_deltaX;
float translation_filter_deltaY;
float g_sigma;
float HIGHEST_OUTPUT;
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_v1_activation_map_f;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_translated;
NEURON_LAYER in_pattern_filtered_translated;
INPUT_DESC in_pattern;
OUTPUT_DESC nl_v1_pattern_out;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_in_pattern_filtered_translated;
OUTPUT_DESC out_v1_activation_map_no_filters;

#endif

#endif
