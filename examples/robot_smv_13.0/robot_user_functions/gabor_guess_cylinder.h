#ifndef __GABOR_GUESS_CYLINDER_H
#define __GABOR_GUESS_CYLINDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cylinder.h"
#include "cylinder_list.h"

CYLINDER_LIST* fit_cylinder_list (int* working_area, int halves);

#endif

#ifdef __cplusplus
}
#endif
