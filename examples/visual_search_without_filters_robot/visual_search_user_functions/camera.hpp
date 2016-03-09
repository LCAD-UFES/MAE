/* visual_search_without_filters_robot camera module header */
#ifndef __CAMERA_H
#define	__CAMERA_H

#include <locale.h>
#include "mae.h"		// MAE functions
#include "filter.h"		// pre-defined MAE filters

#ifdef __cplusplus
extern "C" {
#endif

//void	copy_bumblebee2_image_into_input(INPUT_DESC*,TriclopsImage*);
NEURON_OUTPUT GetLeft(PARAM_LIST*);
NEURON_OUTPUT GetRight(PARAM_LIST*);
void	visual_search_initialize_camera(void);
void	visual_search_terminate_camera(void);

#ifdef __cplusplus
}
#endif

#endif
