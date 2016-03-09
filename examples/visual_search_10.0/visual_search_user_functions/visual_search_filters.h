#ifndef _VISUAL_SEARCH_FILTERS_H
#define _VISUAL_SEARCH_FILTERS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

// Definitions

// Macros

// Structs

// Prototypes
float SimilarityFunction (int nVectorLength, float *pVectorA, float *pVectorB, float *pSigmaVector);
float ComputeMaxSimilarity (int nVectorsNumber, int nVectorLength, float *pVectorsArray, float *pVector, float *pSigmaVector);
void v1_activation_map_filter (FILTER_DESC *filter_desc);

// Global Variables

#endif
