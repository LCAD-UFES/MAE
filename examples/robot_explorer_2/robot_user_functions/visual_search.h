#ifndef __VISUAL_SEARCH_H
#define __VISUAL_SEARCH_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "filter.h"
#include "../robot.h"

// Definitions

// Macros

// Structs

// Prototypes
void output_handler_max_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state);
void output_handler_min_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state);
void output_handler_mean_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state);
void jump (INPUT_DESC *input);
void saccade (INPUT_DESC *input);
float SimilarityFunction (int nVectorLength, float *pVectorA, float *pVectorB, float *pSigmaVector);
float ComputeMaxSimilarity (int nVectorsNumber, int nVectorLength, float *pVectorsArray, float *pVector, float *pSigmaVector);
void v1_activation_map_filter (FILTER_DESC *filter_desc);

#endif
