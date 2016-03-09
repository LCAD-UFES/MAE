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
#define NUM_FRAMES (602)
#define IMAGE_WIDTH (320)
#define IMAGE_HEIGHT (240)
#define RAMDOM_IMAGES_TRAIN ("/dados/vot2014/ball/groundtruth.txt")
#define TLD_GT ("/dados/TLD/TLD/10_panda/TLD1.0.txt")
#define RAMDOM_IMAGES_RECALL ("/dados/vot2014/ball/image_list.txt")
#define TRAINING_INPUT_PATH ("/dados/vot2014/ball/")
#define REACALL_INPUT_PATH ("vot2014/ball/")
#define INPUT_PATH ("vot2014/ball/")
#define INVALIDS_FRAMES (0)
#define MAX_SCALE (2.31169225)
#define MIN_SCALE (0.6604835)
#define IMAGE_WIDTH_RESIZED (201)
#define IMAGE_HEIGHT_RESIZED (201)
#define INPUTS_PER_NEURON (256)
#define GAUSSIAN_RADIUS (8.0)
#define LOG_FACTOR (2.0)
#define BAND_WIDTH (0.125)
#define TRAINNING (0)
#define RUNNING (1)
#define CUT_POINT (0.7)
#define TAM_NL_ZOOM (1601)

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
extern "C" void output_handler_resize (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void translate_nl_filter (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" void red_mask_filter (FILTER_DESC *);
extern "C" void green_mask_filter (FILTER_DESC *);
extern "C" void blue_mask_filter (FILTER_DESC *);
extern "C" void map_image_v1 (FILTER_DESC *);
extern "C" void compute_weigheted_neighboor_filter (FILTER_DESC *);
extern "C" void threshold_cut_filter (FILTER_DESC *);
extern "C" void generate_hough_activation_map (FILTER_DESC *);
extern "C" void generate_hough_zoom_activation_map (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImage (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToTargetCenter (PARAM_LIST *);
extern "C" NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern "C" NEURON_OUTPUT EvaluateDetection (PARAM_LIST *);
extern "C" NEURON_OUTPUT ImageToRetrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetImageOrder (PARAM_LIST *);
extern "C" NEURON_OUTPUT SaveTLDReasultsFile (PARAM_LIST *);
extern "C" NEURON_OUTPUT ForwardVisualSearchNetwork (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetConfidence (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetScaleFactorInTrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetMinConfidenceToRetrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetConfidenceLevel (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetNumPixels (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetX (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetY (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetScaleBefore (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetScaleFactorZoom (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetOutOfScene (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetX_before (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetY_before (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetScaleToRetrain (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetImageOutOfScene (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetConfidenceZoom (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetDistanceZoom (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetMaxScale (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetMinScale (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetNumFrames (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetOutOfScene (PARAM_LIST *);
extern "C" NEURON_OUTPUT DisposeMemory (PARAM_LIST *);

// Global Variables
extern "C" int g_kernel_size;
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" float g_sigma;
extern "C" float HIGHEST_OUTPUT;
extern "C" NEURON_LAYER nl_v1_activation_map;
extern "C" NEURON_LAYER nl_v1_activation_map_neuron_weight;
extern "C" NEURON_LAYER nl_v1_activation_map_neuron_weight_thresholded;
extern "C" NEURON_LAYER nl_target_coordinates;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated;
extern "C" NEURON_LAYER in_pattern_filtered_translated_red;
extern "C" NEURON_LAYER in_pattern_filtered_translated_green;
extern "C" NEURON_LAYER in_pattern_filtered_translated_blue;
extern "C" NEURON_LAYER table;
extern "C" NEURON_LAYER table_v1;
extern "C" NEURON_LAYER nl_activation_map_hough;
extern "C" NEURON_LAYER nl_activation_map_hough_gaussian;
extern "C" NEURON_LAYER nl_activation_map_hough_v1;
extern "C" NEURON_LAYER nl_activation_map_hough_zoom;
extern "C" NEURON_LAYER nl_activation_map_hough_zoom_gaussian;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC nl_v1_activation_map_neuron_weight_thresholded_out;
extern "C" OUTPUT_DESC nl_v1_pattern_out;
extern "C" OUTPUT_DESC table_v1_out;
extern "C" OUTPUT_DESC out_v1_activation_map;
extern "C" OUTPUT_DESC out_nl_v1_activation_map_neuron_weight_thresholded;
extern "C" OUTPUT_DESC out_target_coordinates;
extern "C" OUTPUT_DESC out_in_pattern_filtered_translated;
extern "C" OUTPUT_DESC out_table;
extern "C" OUTPUT_DESC out_nl_activation_map_hough_zoom;
extern "C" OUTPUT_DESC out_nl_activation_map_hough_zoom_gaussian;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_weighted_average_value_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void output_handler_resize (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void translate_nl_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern void red_mask_filter (FILTER_DESC *);
extern void green_mask_filter (FILTER_DESC *);
extern void blue_mask_filter (FILTER_DESC *);
extern void map_image_v1 (FILTER_DESC *);
extern void compute_weigheted_neighboor_filter (FILTER_DESC *);
extern void threshold_cut_filter (FILTER_DESC *);
extern void generate_hough_activation_map (FILTER_DESC *);
extern void generate_hough_zoom_activation_map (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT get_target_coordinates (PARAM_LIST *);
extern NEURON_OUTPUT GetImage (PARAM_LIST *);
extern NEURON_OUTPUT MoveToTargetCenter (PARAM_LIST *);
extern NEURON_OUTPUT CheckTrafficSignDetection (PARAM_LIST *);
extern NEURON_OUTPUT EvaluateDetection (PARAM_LIST *);
extern NEURON_OUTPUT ImageToRetrain (PARAM_LIST *);
extern NEURON_OUTPUT SetScaleFactor (PARAM_LIST *);
extern NEURON_OUTPUT SetImageOrder (PARAM_LIST *);
extern NEURON_OUTPUT SaveTLDReasultsFile (PARAM_LIST *);
extern NEURON_OUTPUT ForwardVisualSearchNetwork (PARAM_LIST *);
extern NEURON_OUTPUT GetConfidence (PARAM_LIST *);
extern NEURON_OUTPUT GetScaleFactorInTrain (PARAM_LIST *);
extern NEURON_OUTPUT GetMinConfidenceToRetrain (PARAM_LIST *);
extern NEURON_OUTPUT GetConfidenceLevel (PARAM_LIST *);
extern NEURON_OUTPUT GetNumPixels (PARAM_LIST *);
extern NEURON_OUTPUT GetX (PARAM_LIST *);
extern NEURON_OUTPUT GetY (PARAM_LIST *);
extern NEURON_OUTPUT GetScaleBefore (PARAM_LIST *);
extern NEURON_OUTPUT GetScaleFactorZoom (PARAM_LIST *);
extern NEURON_OUTPUT GetOutOfScene (PARAM_LIST *);
extern NEURON_OUTPUT MoveToPoint (PARAM_LIST *);
extern NEURON_OUTPUT GetX_before (PARAM_LIST *);
extern NEURON_OUTPUT GetY_before (PARAM_LIST *);
extern NEURON_OUTPUT GetScaleToRetrain (PARAM_LIST *);
extern NEURON_OUTPUT GetImageOutOfScene (PARAM_LIST *);
extern NEURON_OUTPUT GetConfidenceZoom (PARAM_LIST *);
extern NEURON_OUTPUT GetDistanceZoom (PARAM_LIST *);
extern NEURON_OUTPUT GetMaxScale (PARAM_LIST *);
extern NEURON_OUTPUT GetMinScale (PARAM_LIST *);
extern NEURON_OUTPUT GetNumFrames (PARAM_LIST *);
extern NEURON_OUTPUT SetOutOfScene (PARAM_LIST *);
extern NEURON_OUTPUT DisposeMemory (PARAM_LIST *);

// Global Variables
int g_kernel_size;
float translation_filter_deltaX;
float translation_filter_deltaY;
float g_sigma;
float HIGHEST_OUTPUT;
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_v1_activation_map_neuron_weight;
NEURON_LAYER nl_v1_activation_map_neuron_weight_thresholded;
NEURON_LAYER nl_target_coordinates;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_translated;
NEURON_LAYER in_pattern_filtered_translated;
NEURON_LAYER in_pattern_filtered_translated_red;
NEURON_LAYER in_pattern_filtered_translated_green;
NEURON_LAYER in_pattern_filtered_translated_blue;
NEURON_LAYER table;
NEURON_LAYER table_v1;
NEURON_LAYER nl_activation_map_hough;
NEURON_LAYER nl_activation_map_hough_gaussian;
NEURON_LAYER nl_activation_map_hough_v1;
NEURON_LAYER nl_activation_map_hough_zoom;
NEURON_LAYER nl_activation_map_hough_zoom_gaussian;
INPUT_DESC in_pattern;
OUTPUT_DESC nl_v1_activation_map_neuron_weight_thresholded_out;
OUTPUT_DESC nl_v1_pattern_out;
OUTPUT_DESC table_v1_out;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_nl_v1_activation_map_neuron_weight_thresholded;
OUTPUT_DESC out_target_coordinates;
OUTPUT_DESC out_in_pattern_filtered_translated;
OUTPUT_DESC out_table;
OUTPUT_DESC out_nl_activation_map_hough_zoom;
OUTPUT_DESC out_nl_activation_map_hough_zoom_gaussian;

#endif

#endif
