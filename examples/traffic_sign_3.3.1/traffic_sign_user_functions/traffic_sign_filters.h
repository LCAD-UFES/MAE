#ifndef _TRAFFIC_SIGN_FILTERS_H
#define _TRAFFIC_SIGN_FILTERS_H

// Includes
#include "filter.h"
#include "../traffic_sign.h"
#include <assert.h>

// Macros

// Types

// Prototypes
void traffic_sign_resize_NN (FILTER_DESC *filter_desc);
void traffic_sign_resize_bilinear (FILTER_DESC *filter_desc);
void dead_band_noise_mask_nl_filter (FILTER_DESC *filter_desc);	//Imported from Visual Search Without Filters
void minimum_energy_channel_filter (FILTER_DESC *filter_desc);

//void traffic_sign_crop (FILTER_DESC *filter_desc);

// Exportable Variables

#endif

