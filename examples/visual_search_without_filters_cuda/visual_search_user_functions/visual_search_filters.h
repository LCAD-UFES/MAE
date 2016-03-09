#ifndef _VISUAL_SEARCH_FILTERS_H
#define _VISUAL_SEARCH_FILTERS_H

// Includes
#include "cuda_functions.h"
#include "filter.h"
#include "../visual_search.h"

// Definitions

// Macros

// Types

// Prototypes
void translate_nl_filter (FILTER_DESC *filter_desc);
void v1_activation_map_filter (FILTER_DESC *filter_desc);

// Global Variables
int g_nNetworkStatus = RUNNING;

#endif
