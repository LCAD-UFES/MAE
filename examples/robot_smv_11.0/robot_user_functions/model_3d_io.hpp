#ifndef __MODEL_3D_DISPLAY_H
#define __MODEL_3D_DISPLAY_H

#include <stdio.h>
#include "model_3d.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void model_3d_load(MODEL_3D *model, FILE *file);

void model_3d_save(MODEL_3D *model, FILE *file);

void model_3d_view(MODEL_3D *model);

#ifdef __cplusplus
}
#endif

#endif
