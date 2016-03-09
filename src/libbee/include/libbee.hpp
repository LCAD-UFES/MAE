/**********************************************
		LIBBEE HEADER FILE
***********************************************/

#ifndef __LIBBEE_H
#define __LIBBEE_H

// Only libdc1394 must be exported to C compling
#ifdef __cplusplus
extern "C" {
#endif 

//====================
// libdc1394 Includes
//====================
#include <dc1394/conversions.h>
#include <dc1394/control.h>
#include <dc1394/utils.h>


#ifdef __cplusplus
}
#endif

//====================
// pgrlibdc Includes
//====================

//#include "pgr_registers.h"
#include "pgr_stereocam.h"

int	libbee_writePgm(char*,unsigned char*,int,int);
int	libbee_writePpm(char*,unsigned char*,int,int);
void	libbee_cleanup_camera(dc1394camera_t*);
void	libbee_cleanup_and_exit(dc1394camera_t*);
void	libbbee_initialize_camera_context(void);
void	libbbee_terminate_camera_context(void);
TriclopsColorImage*	libbee_get_rectified_images(int);
//libbee_get_opencv_rectified_images(int);
void		libbee_destroy_flycapture_image(TriclopsColorImage*);

#endif
