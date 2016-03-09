#ifndef _STEREO_H
#define _STEREO_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definitions */
#define LEFT_CAMERA				0
#define RIGHT_CAMERA			1

#define CONVERT_TO_OPENCV 0
#define CONVERT_TO_MAE 1

#if defined(_WIN32)
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT 
#endif

//#define DEBUG_STEREO_API

/* Macros */

#ifndef ERROR
#define STEREO_ERROR_LOG				"Stereo3Error.log"
#define ERROR(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (STEREO_ERROR_LOG, "a"); \
						fprintf (pLogFile, "Error: %s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif

#ifndef MESSAGE
#define MESSAGE(str1,str2,str3)			{printf ("%s%s%s\n", str1, str2, str3); }
#endif

#ifdef DEBUG_STEREO_API
#ifndef DEBUG
#define STEREO_EXECUTION				"Stereo3Execution.log"
#define DEBUG(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (STEREO_EXECUTION, "a"); \
						fprintf (pLogFile, "%s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif
#else
#define DEBUG(str1,str2,str3)
#endif

#ifdef __cplusplus 
extern "C" {
#endif

/* Types */
typedef struct ImagePoint
{
    double x;
    double y;
}
ImagePoint;

typedef struct WorldPoint
{
    double x;
    double y;
    double z;
}
WorldPoint;

/* Prototypes */
DLLEXPORT int StereoInitialize (int image_width, int image_height); 
DLLEXPORT int StereoTerminate (void);
DLLEXPORT int LoadStereoParameters (const char *strFileName);
DLLEXPORT int LoadStereoRectificationIndexes (void);
DLLEXPORT int StereoTriangulation (int num_points, double *world_point_left, double *world_point_right, double *image_point_left, double *image_point_right);
DLLEXPORT int RectifyStereoPair (unsigned char *rect_img_left, unsigned char *rect_img_right, unsigned char *img_left, unsigned char *img_right);
DLLEXPORT int RectifyLeftImage (unsigned char *rect_img_left, unsigned char *img_left);
DLLEXPORT int RectifyRightImage (unsigned char *rect_img_right, unsigned char *img_right);
DLLEXPORT int GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
DLLEXPORT int CameraProjection (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
DLLEXPORT int GetWorldPointAtDistanceRight (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
DLLEXPORT int CameraProjectionRight (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
DLLEXPORT int TransformPoints (int num_points, double *output_points, double *input_points, double *euler_angles, double *displacement, double *mass_center);

/* New Prototypes */
DLLEXPORT int StereoTriangulationSinglePoint(double *src_point, double *point_3D);
DLLEXPORT int StereoTriangulationNew(int num_points, double **src_points, double **points_3D);
DLLEXPORT int StereoTriangulationLeft(int num_points, double *image_point_left, double *image_point_right, double *world_point_left);
DLLEXPORT int StereoTriangulationRight(int num_points, double *image_point_left, double *image_point_right, double *world_point_right);
DLLEXPORT int RectifyLeftImageNew (void* p_img_left, void* p_rect_img_left);
DLLEXPORT int RectifyRightImageNew (void* p_img_right, void* p_rect_img_right);
DLLEXPORT int SaveStereoMaps (int image_width, int image_height, int p_print_disparity, int p_print_normalized_disparity, int p_print_depth);
DLLEXPORT int GetCameraParameters(ImagePoint* fov, ImagePoint* principal_point, double* focal_length, double* aspect_ratio, int nCameraSide);
DLLEXPORT int ConvertImage(unsigned char *p_raw_image_data, void *image, int p_type);
DLLEXPORT double GetStereoBaseline(void);

/* Exportable Variables */

#ifdef __cplusplus 
}
#endif

#endif


