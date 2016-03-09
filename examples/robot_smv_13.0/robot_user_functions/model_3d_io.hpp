#ifndef __MODEL_3D_DISPLAY_H
#define __MODEL_3D_DISPLAY_H

#include <stdio.h>

#include "model_3d.hpp"
using mae::smv::Model3D;

#ifdef __cplusplus
extern "C" {
#endif

void model_3d_load_config(Model3D& model3d);

void model_3d_save_config(Model3D& model3d);

void model_3d_view(Model3D& model3d);

#ifdef __cplusplus
}
#endif

#endif
