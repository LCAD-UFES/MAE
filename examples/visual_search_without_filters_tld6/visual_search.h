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
#define IMAGE_WIDTH (290)
#define IMAGE_HEIGHT (217)
#define RAMDOM_IMAGES_TRAIN ("/dados/TLD/TLD/09_carchase/gt.txt")
#define RAMDOM_IMAGES_RECALL ("/dados/TLD/TLD/09_carchase/image_list.txt")
#define TRAINING_INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define REACALL_INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define INPUT_PATH ("/dados/TLD/TLD/09_carchase/")
#define INVALIDS_FRAMES (1268)
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
extern "C" void compute_weigheted_neighboor_filter (FILTER_DESC *);
extern "C" void compute_movement_diff_filter (FILTER_DESC *);
extern "C" void threshold_cut_filter (FILTER_DESC *);
extern "C" void generate_hough_activation_map (FILTER_DESC *);
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
extern "C" NEURON_OUTPUT GetMinConfidenceToRetrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetConfidenceLevel (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetNumPixels (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetX (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetY (PARAM_LIST *);

// Global Variables
extern "C" int g_kernel_size;
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" float g_sigma;
extern "C" float HIGHEST_OUTPUT;
extern "C" NEURON_LAYER nl_v1_activation_map;
extern "C" NEURON_LAYER nl_movement_table;
extern "C" NEURON_LAYER nl_movement_map;
extern "C" NEURON_LAYER nl_movement_diff_map;
extern "C" NEURON_LAYER nl_v1_activation_map_neuron_weight;
extern "C" NEURON_LAYER nl_v1_activation_map_neuron_weight_thresholded;
extern "C" NEURON_LAYER nl_target_coordinates;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated;
extern "C" NEURON_LAYER table;
extern "C" NEURON_LAYER table_v1;
extern "C" NEURON_LAYER nl_activation_map_hough;
extern "C" NEURON_LAYER nl_activation_map_hough_gaussian;
extern "C" NEURON_LAYER nl_activation_map_hough_v1;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC nl_movement_table_out;
extern "C" OUTPUT_DESC nl_movement_map_out;
extern "C" OUTPUT_DESC nl_movement_diff_map_out;
extern "C" OUTPUT_DESC nl_v1_activation_map_neuron_weight_out;
extern "C" OUTPUT_DESC nl_v1_pattern_out;
extern "C" OUTPUT_DESC in_pattern_filtered_translated_out;
extern "C" OUTPUT_DESC table_v1_out;
extern "C" OUTPUT_DESC nl_activation_map_hough_out;
extern "C" OUTPUT_DESC nl_activation_map_hough_gaussian_out;
extern "C" OUTPUT_DESC nl_activation_map_hough_v1_out;
extern "C" OUTPUT_DESC out_v1_activation_map;
extern "C" OUTPUT_DESC out_nl_v1_activation_map_neuron_weight_thresholded;
extern "C" OUTPUT_DESC out_target_coordinates;
extern "C" OUTPUT_DESC out_in_pattern_filtered_translated;
extern "C" OUTPUT_DESC out_table;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_weighted_average_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void translate_nl_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern void map_image_v1 (FILTER_DESC *);
extern void compute_weigheted_neighboor_filter (FILTER_DESC *);
extern void compute_movement_diff_filter (FILTER_DESC *);
extern void threshold_cut_filter (FILTER_DESC *);
extern void generate_hough_activation_map (FILTER_DESC *);
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
extern NEURON_OUTPUT GetMinConfidenceToRetrain (PARAM_LIST *);
extern NEURON_OUTPUT GetConfidenceLevel (PARAM_LIST *);
extern NEURON_OUTPUT GetNumPixels (PARAM_LIST *);
extern NEURON_OUTPUT GetX (PARAM_LIST *);
extern NEURON_OUTPUT GetY (PARAM_LIST *);

// Global Variables
int g_kernel_size;
float translation_filter_deltaX;
float translation_filter_deltaY;
float g_sigma;
float HIGHEST_OUTPUT;
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_movement_table;
NEURON_LAYER nl_movement_map;
NEURON_LAYER nl_movement_diff_map;
NEURON_LAYER nl_v1_activation_map_neuron_weight;
NEURON_LAYER nl_v1_activation_map_neuron_weight_thresholded;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_translated;
NEURON_LAYER in_pattern_filtered_translated;
NEURON_LAYER table;
NEURON_LAYER table_v1;
NEURON_LAYER nl_activation_map_hough;
NEURON_LAYER nl_activation_map_hough_gaussian;
NEURON_LAYER nl_activation_map_hough_v1;
INPUT_DESC in_pattern;
OUTPUT_DESC nl_movement_table_out;
OUTPUT_DESC nl_movement_map_out;
OUTPUT_DESC nl_movement_diff_map_out;
OUTPUT_DESC nl_v1_activation_map_neuron_weight_out;
OUTPUT_DESC nl_v1_pattern_out;
OUTPUT_DESC in_pattern_filtered_translated_out;
OUTPUT_DESC table_v1_out;
OUTPUT_DESC nl_activation_map_hough_out;
OUTPUT_DESC nl_activation_map_hough_gaussian_out;
OUTPUT_DESC nl_activation_map_hough_v1_out;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_nl_v1_activation_map_neuron_weight_thresholded;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_in_pattern_filtered_translated;
OUTPUT_DESC out_table;

#endif

#endif
