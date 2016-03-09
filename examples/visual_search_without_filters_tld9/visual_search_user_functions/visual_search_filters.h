#ifndef _VISUAL_SEARCH_FILTERS_H
#define _VISUAL_SEARCH_FILTERS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

// Definitions

// Macros

// Types

// Prototypes
void translate_nl_filter (FILTER_DESC *filter_desc);

void reshape_cv_filter (FILTER_DESC *filter_desc);

void gaussian_cv_filter (FILTER_DESC *filter_desc);

// Global variables

extern double g_scale_factor;
extern double g_object_scale_factor;

#endif
