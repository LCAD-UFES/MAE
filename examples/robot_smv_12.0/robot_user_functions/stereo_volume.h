#ifndef STEREO_VOLUME_H
#define STEREO_VOLUME_H

#include "cylinder_list.h"

double compute_average_length(CYLINDER_LIST *face_left, CYLINDER_LIST *face_top, CYLINDER_LIST *face_right);

double compute_average_width(CYLINDER_LIST *face_left, CYLINDER_LIST *face_right);

double compute_average_height(CYLINDER_LIST *face_left, CYLINDER_LIST *face_top, CYLINDER_LIST *face_right);

#endif
