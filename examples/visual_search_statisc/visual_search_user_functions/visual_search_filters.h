#ifndef _VISUAL_SEARCH_FILTERS_H
#define _VISUAL_SEARCH_FILTERS_H

// Includes
#include "filter.h"

// Definitions

// Macros

// Structs

// Prototypes
float SimilarityFunction (int nFeaturesPerColumn, float *pTargetColumnsArray, float *pPointFeatureColumn);
float ComputeMaxSimilarity (int nColumnsNumber, int nFeaturesPerColumn, float *pTargetColumnsArray, float *pPointFeatureColumn);
void v1_activation_map_filter (FILTER_DESC *filter_desc);

// Global Variables

#endif
