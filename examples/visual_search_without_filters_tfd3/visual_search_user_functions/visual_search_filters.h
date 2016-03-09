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

// Global variables

void rotate_scale_image(IplImage *img, double angle, double scale, int xi, int yi);

extern double g_scale_factor;

#endif
