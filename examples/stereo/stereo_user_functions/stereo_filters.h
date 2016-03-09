#ifndef _STEREO_FILTERS_H
#define _STEREO_FILTERS_H

// Includes
#include "mae.h"
#include "filter.h"
#include "stereo_user_functions.h"
#include "common.h"

// Definitions

// Macros
#define MAG(real,imag)          sqrt(real*real+imag*imag)
#define PHASE(real,imag)        (real > .0) ? atan (imag / real) : \
                                ((real < .0) ? ((imag >= 0) ? atan (imag / real) + M_PI : atan (imag / real) - M_PI) : \
                                ((imag > .0) ? .5 * M_PI : ((imag < 0) ? -.5 * M_PI : .0f)))
// Types

#endif
