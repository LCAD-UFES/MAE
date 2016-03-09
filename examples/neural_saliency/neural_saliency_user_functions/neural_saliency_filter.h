#ifndef _NEURAL_SALIENCY_FILTERS_H
#define _NEURAL_SALIENCY_FILTERS_H

// Includes
#include "filter.h"
#include "../neural_saliency.h"

void
translate_nl_filter(FILTER_DESC *filter_desc);

void
scale_nl_filter (FILTER_DESC *filter_desc);

void
crop_nl_filter (FILTER_DESC *filter_desc);

void hamming_nl_filter(FILTER_DESC *filter_desc);

#endif
