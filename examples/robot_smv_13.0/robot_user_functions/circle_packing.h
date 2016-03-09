#ifndef CIRCLE_PACKING_H
#define CIRCLE_PACKING_H

#include "mae.h"
#include "cylinder_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

void init_circle_packing(INPUT_DESC *input);

void circle_packing_set_cylinder_list(CYLINDER_LIST *list);

double circle_packing_execute();

#ifdef __cplusplus
}
#endif

#endif
