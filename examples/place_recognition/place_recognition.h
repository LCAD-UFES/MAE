// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _PLACE_RECOGNITION_H
#define _PLACE_RECOGNITION_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define IMAGE_WIDTH (640)
#define IMAGE_HEIGHT (380)
#define IMAGE_HEIGHT_CROP (364)
#define NL_WIDTH (96)
#define NL_HEIGHT (54)
#define INPUTS_PER_NEURON (64)
#define GAUSSIAN_RADIUS (30.0)
#define NUMBER_OF_TRAINING_FRAMES (117)
#define NUMBER_OF_TEST_FRAMES (117)
#define DATA_SET_FILE_TRAIN ("basepos-20161021-20171122-30m-30m.txt")
#define DATA_SET_FILE_TEST ("livepos-20161021-20171122-30m-30m.txt")

// Macros

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void crop_nl_filter (FILTER_DESC *);
extern "C" void translate_nl_filter (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT set_network_status (PARAM_LIST *);
extern "C" NEURON_OUTPUT get_frame (PARAM_LIST *);
extern "C" NEURON_OUTPUT read_dataset (PARAM_LIST *);
extern "C" NEURON_OUTPUT randomize (PARAM_LIST *);
extern "C" NEURON_OUTPUT set_network_evaluate (PARAM_LIST *);

// Global Variables
extern "C" float translation_filter_deltaX;
extern "C" float translation_filter_deltaY;
extern "C" NEURON_LAYER nl_v1_pattern;
extern "C" NEURON_LAYER in_pattern_gaussian;
extern "C" NEURON_LAYER in_pattern_translated;
extern "C" NEURON_LAYER in_pattern_cropped;
extern "C" NEURON_LAYER recall;
extern "C" INPUT_DESC in_pattern;
extern "C" OUTPUT_DESC out_recall;
extern "C" OUTPUT_DESC out_in_pattern_cropped;
extern "C" OUTPUT_DESC out_in_pattern_gaussian;
extern "C" OUTPUT_DESC out_nl_v1_pattern;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void crop_nl_filter (FILTER_DESC *);
extern void translate_nl_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT set_network_status (PARAM_LIST *);
extern NEURON_OUTPUT get_frame (PARAM_LIST *);
extern NEURON_OUTPUT read_dataset (PARAM_LIST *);
extern NEURON_OUTPUT randomize (PARAM_LIST *);
extern NEURON_OUTPUT set_network_evaluate (PARAM_LIST *);

// Global Variables
float translation_filter_deltaX;
float translation_filter_deltaY;
NEURON_LAYER nl_v1_pattern;
NEURON_LAYER in_pattern_gaussian;
NEURON_LAYER in_pattern_translated;
NEURON_LAYER in_pattern_cropped;
NEURON_LAYER recall;
INPUT_DESC in_pattern;
OUTPUT_DESC out_recall;
OUTPUT_DESC out_in_pattern_cropped;
OUTPUT_DESC out_in_pattern_gaussian;
OUTPUT_DESC out_nl_v1_pattern;

#endif

#endif
