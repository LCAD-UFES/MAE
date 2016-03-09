/*
This file is part of SMV.

History:

[2006.02.07 - Helio Perroni Filho] Created.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glut.h>

/*
Function Section
*/

void robot_smv_compute_current();

void robot_smv_compute_point_cloud_current();

void robot_smv_compute_all();

void robot_smv_compute_model_3d_volume();

void robot_smv_config_neuron_layers(int half, int side);

void robot_smv_save_working_area(int half, int* workingArea);

void robot_smv_select_half(int half);

void robot_smv_select_side(int side);

int robot_smv_toggle_two_halves();

int get_current_camera_pair();

/*
Graphics Section
*/

void robot_smv_display();

void robot_smv_rectify_left(char* name);

void robot_smv_rectify_right(char* name);

void robot_smv_rotate_x(double value);

void robot_smv_rotate_y(double value);

void robot_smv_rotate_z(double value);

void robot_smv_straight();

void robot_smv_translate_x(double value);

void robot_smv_translate_y(double value);

void robot_smv_translate_z(double value);

void robot_smv_center();

/*
Input / Output Section
*/

void robot_smv_load_config();

void robot_smv_save_config();

/*
Property Section
*/

void robot_smv_set_two_halves(int halves);

#ifdef __cplusplus
};
#endif
