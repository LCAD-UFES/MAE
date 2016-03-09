/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#ifndef __INPUT_LAYER_H
#define __INPUT_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glut.h>


typedef void* mae_input_layer;

/*
Function Section
*/

mae_input_layer mae_input_layer_get(const char* name);

float mae_input_layer_output_fval(mae_input_layer layer, int index);

int mae_input_layer_output_ival(mae_input_layer layer, int index);

void mae_input_layer_update(mae_input_layer layer);

/*
Input / Output Section
*/

void mae_input_network_load(mae_input_layer layer, const char* path);

void mae_input_network_save(mae_input_layer layer, const char* path);

/*
Property Section
*/

GLubyte* mae_input_layer_get_texture_frame_buffer(mae_input_layer layer);

int mae_input_layer_get_texture_frame_width(mae_input_layer layer);

int mae_input_layer_get_texture_frame_height(mae_input_layer layer);

int mae_input_layer_get_width(mae_input_layer layer);

int mae_input_layer_get_height(mae_input_layer layer);

int mae_input_layer_get_visible_width(mae_input_layer layer);

int mae_input_layer_get_visible_height(mae_input_layer layer);

#ifdef __cplusplus
}
#endif

#endif
