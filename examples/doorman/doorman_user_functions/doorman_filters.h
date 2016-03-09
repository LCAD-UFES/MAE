#ifndef _DOORMAN_FILTERS_H
#define _DOORMAN_FILTERS_H

// Includes
#include "filter.h"

// Definitions

// Macros

// Structs

// Prototypes
float SimilarityFunction (int nVectorLength, float *pVectorA, float *pVectorB, float *pSigmaVector);
float ComputeMaxSimilarity (int nVectorsNumber, int nVectorLength, float *pVectorsArray, float *pVector, float *pSigmaVector);
void v1_activation_map_filter (FILTER_DESC *filter_desc);

// Global Variables

#endif
