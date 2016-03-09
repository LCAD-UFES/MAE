#ifndef __MODEL_3D_DISPLAY_H
#define __MODEL_3D_DISPLAY_H

#include <stdio.h>
#include "model_3d.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void model_3d_load_config(MODEL_3D *model);

void model_3d_save_config(MODEL_3D *model);

void model_3d_view(MODEL_3D *model);

#ifdef __cplusplus
}
#endif

#endif
