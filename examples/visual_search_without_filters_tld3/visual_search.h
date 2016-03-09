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
#define IMAGE_WIDTH (312)
#define IMAGE_HEIGHT (233)
#define RAMDOM_IMAGES_TRAIN ("/dados/TLD/TLD/10_panda/gt.txt")
#define RAMDOM_IMAGES_RECALL ("/dados/TLD/TLD/10_panda/image_list.txt")
#define TRAINING_INPUT_PATH ("/dados/TLD/TLD/10_panda/")
#define REACALL_INPUT_PATH ("/dados/TLD/TLD/10_panda/")
#define INPUT_PATH ("/dados/TLD/TLD/10_panda/")
#define INVALIDS_FRAMES (270)
#define IMAGE_WIDTH_RESIZED (201)
#define IMAGE_HEIGHT_RESIZED (201)
#define INPUTS_PER_NEURON (256)
#define GAUSSIAN_RADIUS (10.0)
#define LOG_FACTOR (2.0)
#define BAND_WIDTH (0.125)
#define TRAINNING (0)
#define RUNNING (1)
#define CUT_POINT (0.7)

// Macros
#define NL_WIDTH (16 * 4 + 1)
#define NL_HEIGHT (12 * 4)
#define ACT_BAND_WIDTH (2 * 4)

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler_weighted_average_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void translate_nl_filter (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" void map_image_v1 (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImage (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImageNotTrained (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToTargetCenter (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToBestTargetCenter (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern "C" NEURON_OUTPUT TrainTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT TrainVariousTrafficSign (PARAM_LIST *);
extern "C" NEURON_OUTPUT flush_and_retraining (PARAM_LIST *);
extern "C" NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern "C" NEURON_OUTPUT EvaluateDetection (PARAM_LIST *);
extern "C" NEURON_OUTPUT ImageToRetrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetImageOrder (PARAM_LIST *);
extern "C" NEURON_OUTPUT SaveTLDReasultsFile (PARAM_LIST *);
extern "C" NEURON_OUTPUT ForwardVisualSearchNetwork (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetConfidence (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetX (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetY (PARAM_LIST *);

// Global Variables
extern "C" int g_kernel_size;
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" float g_sigma;
extern "C" float HIGHEST_OUTPUT;
extern "C" NEURON_LAYER nl_target_coordinates;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated_temp;
extern "C" NEURON_LAYER nl_v1_activation_map_red_confidence;
extern "C" NEURON_LAYER nl_v1_activation_map_green_confidence;
extern "C" NEURON_LAYER nl_v1_activation_map_blue_confidence;
extern "C" NEURON_LAYER nl_v1_activation_map_red;
extern "C" NEURON_LAYER nl_v1_activation_map_green;
extern "C" NEURON_LAYER nl_v1_activation_map_blue;
extern "C" NEURON_LAYER nl_v1_activation_map_red_f;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC nl_v1_pattern_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_red_confidence_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_green_confidence_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_blue_confidence_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_red_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_green_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_blue_out;
extern "C" OUTPUT_DESC out_v1_activation_map_red_f;
extern "C" OUTPUT_DESC out_target_coordinates;
extern "C" OUTPUT_DESC out_in_pattern_filtered_translated;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_weighted_average_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void translate_nl_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern void map_image_v1 (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern NEURON_OUTPUT GetImage (PARAM_LIST *);
extern NEURON_OUTPUT GetImageNotTrained (PARAM_LIST *);
extern NEURON_OUTPUT MoveToTargetCenter (PARAM_LIST *);
extern NEURON_OUTPUT MoveToBestTargetCenter (PARAM_LIST *);
extern NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern NEURON_OUTPUT TrainTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT TrainVariousTrafficSign (PARAM_LIST *);
extern NEURON_OUTPUT flush_and_retraining (PARAM_LIST *);
extern NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern NEURON_OUTPUT EvaluateDetection (PARAM_LIST *);
extern NEURON_OUTPUT ImageToRetrain (PARAM_LIST *);
extern NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern NEURON_OUTPUT SetImageOrder (PARAM_LIST *);
extern NEURON_OUTPUT SaveTLDReasultsFile (PARAM_LIST *);
extern NEURON_OUTPUT ForwardVisualSearchNetwork (PARAM_LIST *);
extern NEURON_OUTPUT GetConfidence (PARAM_LIST *);
extern NEURON_OUTPUT GetX (PARAM_LIST *);
extern NEURON_OUTPUT GetY (PARAM_LIST *);

// Global Variables
int g_kernel_size;
float translation_filter_deltaX;
float translation_filter_deltaY;
float g_sigma;
float HIGHEST_OUTPUT;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_translated;
NEURON_LAYER in_pattern_filtered_translated;
NEURON_LAYER in_pattern_filtered_translated_temp;
NEURON_LAYER nl_v1_activation_map_red_confidence;
NEURON_LAYER nl_v1_activation_map_green_confidence;
NEURON_LAYER nl_v1_activation_map_blue_confidence;
NEURON_LAYER nl_v1_activation_map_red;
NEURON_LAYER nl_v1_activation_map_green;
NEURON_LAYER nl_v1_activation_map_blue;
NEURON_LAYER nl_v1_activation_map_red_f;
INPUT_DESC in_pattern;
OUTPUT_DESC nl_v1_pattern_out;
OUTPUT_DESC nl_v1_activation_map_red_confidence_out;
OUTPUT_DESC nl_v1_activation_map_green_confidence_out;
OUTPUT_DESC nl_v1_activation_map_blue_confidence_out;
OUTPUT_DESC nl_v1_activation_map_red_out;
OUTPUT_DESC nl_v1_activation_map_green_out;
OUTPUT_DESC nl_v1_activation_map_blue_out;
OUTPUT_DESC out_v1_activation_map_red_f;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_in_pattern_filtered_translated;

#endif

#endif
