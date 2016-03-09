// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _TRAFFIC_SIGN_H
#define _TRAFFIC_SIGN_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define INPUT_WIDTH (204)
#define INPUT_HEIGHT (204)
#define N_SCALE (2)
#define VOTE_PARAM (2.5)
#define SYNAPSES (64)
#define NL_WIDTH (70)
#define NL_HEIGHT (70)
#define GAUSSIAN_RADIUS_DISTRIBUTION (7.0)
#define GAUSSIAN_RADIUS_FILTER (7)
#define GAUSSIAN_SIGMA_FILTER (2.5)
#define LOG_FACTOR (2.0)
#define TRAINING_INPUT_PATH ("/home/jorcyd/GTSRB/_scripts/training_set_raw/")
#define TRAINING_INPUT_FILES ("./desc_training_set_rand_860.csv")
#define TESTING_INPUT_PATH ("/home/jorcyd/GTSRB/_scripts/training_set_raw/")
#define TESTING_INPUT_FILES ("./desc_testing_set_rand_430.csv")

// Macros
#define OUT_WIDTH (17 * (N_SCALE + 1))
#define OUT_HEIGHT (9 * (N_SCALE + 1))
#define VOTE_SCALE (VOTE_PARAM * OUT_WIDTH / OUT_HEIGHT)
#define VOTE_VARIANCE (VOTE_SCALE * N_SCALE)

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void traffic_sign_reshape_bilinear (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT SetReshapeFilterParams (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetSignByIndex (PARAM_LIST *);
extern "C" NEURON_OUTPUT WaitKey (PARAM_LIST *);
extern "C" NEURON_OUTPUT PrintResults (PARAM_LIST *);

// Global Variables
extern "C" float reshape_filter_offset_x;
extern "C" float reshape_filter_offset_y;
extern "C" float reshape_filter_scale_factor;
extern "C" float reshape_filter_rotation_angle;
extern "C" NEURON_LAYER nl_traffic_sign_gaussian;
extern "C" NEURON_LAYER nl_traffic_sign_reshape;
extern "C" NEURON_LAYER nl_landmark;
extern "C" NEURON_LAYER nl_landmark_eval_mask;
extern "C" INPUT_DESC traffic_sign;
extern "C" OUTPUT_DESC nl_landmark_eval_mask_out;
extern "C" OUTPUT_DESC out_traffic_sign_reshape;
extern "C" OUTPUT_DESC out_traffic_sign_gaussian;
extern "C" OUTPUT_DESC out_landmark;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void traffic_sign_reshape_bilinear (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT SetReshapeFilterParams (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT GetSignByIndex (PARAM_LIST *);
extern NEURON_OUTPUT WaitKey (PARAM_LIST *);
extern NEURON_OUTPUT PrintResults (PARAM_LIST *);

// Global Variables
float reshape_filter_offset_x;
float reshape_filter_offset_y;
float reshape_filter_scale_factor;
float reshape_filter_rotation_angle;
NEURON_LAYER nl_traffic_sign_gaussian;
NEURON_LAYER nl_traffic_sign_reshape;
NEURON_LAYER nl_landmark;
NEURON_LAYER nl_landmark_eval_mask;
INPUT_DESC traffic_sign;
OUTPUT_DESC nl_landmark_eval_mask_out;
OUTPUT_DESC out_traffic_sign_reshape;
OUTPUT_DESC out_traffic_sign_gaussian;
OUTPUT_DESC out_landmark;

#endif

#endif
