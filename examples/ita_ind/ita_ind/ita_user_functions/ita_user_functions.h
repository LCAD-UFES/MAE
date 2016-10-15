#ifndef _FACE_RECOG_USER_FUNCTIONS_H
#define _FACE_RECOG_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../ita.h"

#define ITA_BUILD 0

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define TEST_PHASE			2
#define WARM_UP_PHASE		3

#if ITA_BUILD
NEURON_OUTPUT SetNetworkStatus(int i);
NEURON_OUTPUT ShowStatistics(int net);
int signal_of_val(float val);
int GetNextReturns(int nDirection);
void compute_prediction_statistics(int net, int n_stocks, double *neural_prediction, double *actual_result);
int buy_or_sell(int net, int stock, double *neural_prediction);
void SetSymbolReturn(int symbol, float symbol_return, int net, int displacement);
double GetSymbolReturn(int symbol, int net, int displacement);
void SetSampleIdAndTime(int net, int displacement, int id, char *time);
int LoadReturnsToInput(INPUT_DESC *input, int net, int displacement);
int LoadReturnsToOutput(OUTPUT_DESC *output, int net);
int SaveReturns(int last_return);
float GetNeuronOutput(OUTPUT_DESC *output, int out_index);
int GetMAESampleIndex(void);
#endif

#endif
