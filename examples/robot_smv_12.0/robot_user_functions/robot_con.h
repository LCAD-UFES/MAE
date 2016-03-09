/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "cylinder_list.h"

/*
Function Section
*/

void robot_con_init();

void robot_con_fill_network(const char* name, int side, int half, const char* format, int* left, int* right);

int robot_con_load_network(const char* name, int side, int half, const char* format);

void robot_con_select_match_region(char* match_layer);

void robot_con_get_points(int* workingArea, double* leftPoints, double* rightPoints, int* colors);

void robot_con_setup_circle_packing(CYLINDER_LIST* list);

long robot_con_merge_offset();

void robot_con_copy_to_merge_map(int w0, int offset);

void robot_con_execute();

/*
Property Section
*/

int robot_con_get_input_height();

int robot_con_get_input_width();

void robot_con_set_image_left(int* left);

void robot_con_set_image_right(int* right);

#ifdef __cplusplus
}
#endif
