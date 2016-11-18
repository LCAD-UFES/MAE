#ifndef _FACE_RECOG_FILTERS_H
#define _FACE_RECOG_FILTERS_H

// Includes
#include "filter.h"
#include "../ita.h"

// Macros

// Types

// Prototypes
void face_reshape_filter (FILTER_DESC *filter_desc);
void face_reshape_filter2 (FILTER_DESC *filter_desc);
void scale_nl_filter (FILTER_DESC *filter_desc);
void rotate_nl_filter (FILTER_DESC *filter_desc);
void translate_nl_filter (FILTER_DESC *filter_desc);
void gaussian_1D_filter(FILTER_DESC *filter_desc);
// Exportable Variables

#endif
