// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _TRAFFIC_SIGN_H
#define _TRAFFIC_SIGN_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define INPUT_WIDTH (1628)
#define INPUT_HEIGHT (1236)
#define OUT_WIDTH (16)
#define OUT_HEIGHT (16)
#define SYNAPSES (256)
#define NL_WIDTH (50)
#define NL_HEIGHT (50)
#define INPUT_PATH ("/media/shared_win_dir/Seqs/")
#define GROUNDT_PATH ("/media/shared_win_dir/_mae/")

// Macros

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void traffic_sign_crop (FILTER_DESC *);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT GetFrame (PARAM_LIST *);
extern "C" NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern "C" NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern "C" NEURON_OUTPUT GetNthSignParam (PARAM_LIST *);
extern "C" NEURON_OUTPUT MoveInput (PARAM_LIST *);

// Global Variables
extern "C" NEURON_LAYER nl_traffic_sign;
extern "C" NEURON_LAYER nl_traffic_sign_filtered;
extern "C" NEURON_LAYER nl_landmark;
extern "C" INPUT_DESC traffic_sign;
extern "C" OUTPUT_DESC out_traffic_sign;
extern "C" OUTPUT_DESC out_traffic_sign_filtered;
extern "C" OUTPUT_DESC out_landmark;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void traffic_sign_crop (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT GetFrame (PARAM_LIST *);
extern NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT GetNthSignParam (PARAM_LIST *);
extern NEURON_OUTPUT MoveInput (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_traffic_sign;
NEURON_LAYER nl_traffic_sign_filtered;
NEURON_LAYER nl_landmark;
INPUT_DESC traffic_sign;
OUTPUT_DESC out_traffic_sign;
OUTPUT_DESC out_traffic_sign_filtered;
OUTPUT_DESC out_landmark;

#endif

#endif
