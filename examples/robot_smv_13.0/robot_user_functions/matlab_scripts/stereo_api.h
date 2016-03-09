#ifndef _STEREO_H
#define _STEREO_H

/* Includes */

/* Definitions */
#if !defined(IMPORTING) && !defined(__linux__)
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT 
#endif

#define STEREO_ERROR_LOG			"StereoError.log"
#define STEREO_EXECUTION			"StereoExecution.log"

#define LEFT_CAMERA				0
#define RIGHT_CAMERA				1

/* Macros */

#ifndef ERROR
#define ERROR(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (STEREO_ERROR_LOG, "a"); \
						fprintf (pLogFile, "Error: %s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif

#ifndef MESSAGE
#define MESSAGE(str1,str2,str3)			{printf ("%s%s%s\n", str1, str2, str3); }
#endif

/* #define DEBUG_STEREO_API */
#ifdef DEBUG_STEREO_API
#ifndef DEBUG
#define DEBUG(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (STEREO_EXECUTION, "a"); \
						fprintf (pLogFile, "%s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif
#else
#define DEBUG(str1,str2,str3)
#endif

/* Types */

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes */
extern DLLEXPORT int StereoInitialize (int image_width, int image_height); 
extern DLLEXPORT int StereoTerminate (void);
extern DLLEXPORT int LoadStereoParameters (const char *strFileName);
extern DLLEXPORT int LoadStereoRectificationIndexes (const char *strFileName);
extern DLLEXPORT int StereoTriangulation (int num_points, double *XL, double *XR, double *xL, double *xR);
extern DLLEXPORT int RectifyStereoPair (unsigned char *rect_img_left, unsigned char *rect_img_right, unsigned char *img_left, unsigned char *img_right, int w, int h);
extern DLLEXPORT int RectifyLeftImage (unsigned char *rect_img_left, unsigned char *img_left, int w, int h);
extern DLLEXPORT int RectifyRightImage (unsigned char *rect_img_right, unsigned char *img_right, int w, int h);
extern DLLEXPORT int GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
extern DLLEXPORT int CameraProjection (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
extern DLLEXPORT int GetWorldPointAtDistanceRight (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
extern DLLEXPORT int CameraProjectionRight (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
extern DLLEXPORT int TransformPoints (int num_points, double *output_points, double *input_points, double *euler_angles, double *displacement, double *mass_center);
extern DLLEXPORT int ApplyEdgeFilter (unsigned char *edge_img, unsigned char *img, int w, int h);

#ifdef __cplusplus
}
#endif

#endif


