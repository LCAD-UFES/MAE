// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _FACE_RECOG_H
#define _FACE_RECOG_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define INPUT_WIDTH (384)
#define INPUT_HEIGHT (288)
#define OUT_WIDTH (32)
#define OUT_HEIGHT (32)
#define SYNAPSES (256)
#define NL_WIDTH (120)
#define NL_HEIGHT (165)
#define BASELINE_FACTOR (0.45)
#define X_OFFSET (0.0)
#define Y_OFFSET (15.0)
#define GAUSSIAN_SIGMA (3.0)
#define KERNEL_SIZE (3)
#define INPUT_PATH ("/home/hallysson/img/ARDataBase/ARPhotoDataBase/")

// Macros

// Structs

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void face_reshape_filter2 (FILTER_DESC *);
extern void subsample_filter (FILTER_DESC *);
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT GetFace (PARAM_LIST *);
extern NEURON_OUTPUT GetRandomFace (PARAM_LIST *);
extern NEURON_OUTPUT Move2Eye (PARAM_LIST *);
extern NEURON_OUTPUT Move2Nose (PARAM_LIST *);
extern NEURON_OUTPUT Move2Nose2 (PARAM_LIST *);
extern NEURON_OUTPUT Move2Mouth (PARAM_LIST *);
extern NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus2 (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_face_recog_lp;
NEURON_LAYER nl_face_recog_ss;
NEURON_LAYER nl_face_recog_lp_f;
NEURON_LAYER nl_landmark;
INPUT_DESC face_recog;
OUTPUT_DESC out_face_recog_lp;
OUTPUT_DESC out_face_recog_ss;
OUTPUT_DESC out_face_recog_lp_f;
OUTPUT_DESC out_landmark;
#endif
