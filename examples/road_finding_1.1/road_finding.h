// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _ROAD_FINDING_H
#define _ROAD_FINDING_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define INPUT_WIDTH (320)
#define INPUT_HEIGHT (240)
#define SYNAPSES (256)
#define GAUSSIAN_RADIUS (10.0)
#define IMG_SET_SIZE (9)
#define HIGH_OUTPUT_ACTIVATION_VALUE (255)
#define LOW_OUTPUT_ACTIVATION_VALUE (0)
#define LOWER_BOUND (23)
#define UPPER_BOUND (67)
#define LEFT_BOUND (117)
#define RIGHT_BOUND (202)
#define INPUT_PATH ("imgs/")

// Macros
#define NL_WIDTH (INPUT_WIDTH)
#define NL_HEIGHT (INPUT_HEIGHT)
#define OUT_WIDTH (INPUT_WIDTH)
#define OUT_HEIGHT (INPUT_HEIGHT)
#define IMAGE_WIDTH (INPUT_WIDTH)
#define IMAGE_HEIGHT (INPUT_HEIGHT)
#define UNKNOWN (DUMMY_OUTPUT_ACTIVATION_VALUE)
#define DRIVABLE (HIGH_OUTPUT_ACTIVATION_VALUE)
#define NON_DRIVABLE (LOW_OUTPUT_ACTIVATION_VALUE)

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void gaussian_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;

// Global Variables
extern "C" NEURON_LAYER nl_road_finding_input_filtered;
extern "C" NEURON_LAYER nl_road_finding;
extern "C" INPUT_DESC road_finding;
extern "C" OUTPUT_DESC nl_road_finding_out;
extern "C" OUTPUT_DESC out_road_finding_input_filtered;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;

// Global Variables
NEURON_LAYER nl_road_finding_input_filtered;
NEURON_LAYER nl_road_finding;
INPUT_DESC road_finding;
OUTPUT_DESC nl_road_finding_out;
OUTPUT_DESC out_road_finding_input_filtered;

#endif

#endif
