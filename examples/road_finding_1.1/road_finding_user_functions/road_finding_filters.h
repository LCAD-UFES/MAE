#ifndef _ROAD_FINDING_FILTERS_H
#define _ROAD_FINDING_FILTERS_H

// Includes
#include "filter.h"

// Conditional compiling directives
#ifdef MAE_EXPORT
#include "road_finding.h"
#else
#include "../road_finding.h"
#endif

// Macros

// Types

// For C++ compiling options 
#ifdef __cplusplus
extern "C" {
#endif 

// Prototypes
void road_reshape_filter (FILTER_DESC *filter_desc);
void road_reshape_filter2 (FILTER_DESC *filter_desc);
void scale_nl_filter (FILTER_DESC *filter_desc);
void rotate_nl_filter (FILTER_DESC *filter_desc);
void translate_nl_filter (FILTER_DESC *filter_desc);

#ifdef __cplusplus
}
#endif

// Exportable Variables

#endif
