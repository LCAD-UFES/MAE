// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _WNN_PRED_H
#define _WNN_PRED_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define IN_WIDTH (17)
#define IN_HEIGHT (44)
#define SAMPLE_SIZE (4)
#define MAX_RETURN (0.1)
#define NL_WIDTH (10)
#define NL_HEIGHT (10)
#define SYNAPSES (256)
#define TT_PATH ("TT_TEMP/")
#define DATA_PATH ("DATA_TEMP/")

// Macros
#define MIN_RETURN (-0.1)

// Structs

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT GetSample (PARAM_LIST *);
extern NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT GetSamplebyDate (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_wnn_pred;
NEURON_LAYER sample_gaussian;
INPUT_DESC sample;
OUTPUT_DESC nl_wnn_pred_out;
OUTPUT_DESC sample_gaussian_out;
#endif
