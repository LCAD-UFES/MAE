// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _ITA_H
#define _ITA_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define INPUT_WIDTH (4)
#define INPUT_HEIGHT (19)
#define OUT_WIDTH (4)
#define OUT_HEIGHT (11)
#define SYNAPSES (256)
#define ALAVANCAGEM (1.0)
#define CUSTO_BOVESPA (0.0)
#define CUSTO_CORRETORA (0.0)
#define CUSTO_CORRETORA_P (0.0)
#define CUSTO_TRASACAO (0.0)
#define CERTAINTY (20.0)
#define STATISTICS (20)

// Macros

// Structs

// For avoiding symbol table errors on C++ linkage

#ifdef __cplusplus

// Prototypes
extern "C" void input_generator (INPUT_DESC *, int status);
extern "C" void input_controler (INPUT_DESC *, int status);
extern "C" void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern "C" void copy_nl_filter (FILTER_DESC *);
extern "C" NEURON_TYPE minchinton;
extern "C" NEURON_OUTPUT GetRandomReturns (PARAM_LIST *);
extern "C" NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern "C" NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern "C" NEURON_OUTPUT LoadReturns (PARAM_LIST *);
extern "C" NEURON_OUTPUT SetLongShort (PARAM_LIST *);

// Global Variables
extern "C" NEURON_LAYER nl_ita_lp_f;
extern "C" NEURON_LAYER nl_prediction;
extern "C" NEURON_LAYER nl_test;
extern "C" NEURON_LAYER nl_result;
extern "C" NEURON_LAYER nl_ita2_lp_f;
extern "C" NEURON_LAYER nl_prediction2;
extern "C" INPUT_DESC ita;
extern "C" INPUT_DESC ita2;
extern "C" OUTPUT_DESC out_ita_lp_f;
extern "C" OUTPUT_DESC out_prediction;
extern "C" OUTPUT_DESC out_test;
extern "C" OUTPUT_DESC out_result;
extern "C" OUTPUT_DESC out_ita2_lp_f;
extern "C" OUTPUT_DESC out_prediction2;

#else

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void copy_nl_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT GetRandomReturns (PARAM_LIST *);
extern NEURON_OUTPUT ShowStatistics (PARAM_LIST *);
extern NEURON_OUTPUT ResetStatistics (PARAM_LIST *);
extern NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *);
extern NEURON_OUTPUT LoadReturns (PARAM_LIST *);
extern NEURON_OUTPUT SetLongShort (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_ita_lp_f;
NEURON_LAYER nl_prediction;
NEURON_LAYER nl_test;
NEURON_LAYER nl_result;
NEURON_LAYER nl_ita2_lp_f;
NEURON_LAYER nl_prediction2;
INPUT_DESC ita;
INPUT_DESC ita2;
OUTPUT_DESC out_ita_lp_f;
OUTPUT_DESC out_prediction;
OUTPUT_DESC out_test;
OUTPUT_DESC out_result;
OUTPUT_DESC out_ita2_lp_f;
OUTPUT_DESC out_prediction2;

#endif

#endif
