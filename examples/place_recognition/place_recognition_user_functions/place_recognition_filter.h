#ifndef _VISUAL_SEARCH_FILTERS_H
#define _VISUAL_SEARCH_FILTERS_H

// Includes
#include "filter.h"
#include "../place_recognition.h"

void
translate_nl_filter(FILTER_DESC *filter_desc);

void
scale_nl_filter(FILTER_DESC *filter_desc);

void
crop_nl_filter(FILTER_DESC *filter_desc);

#endif
