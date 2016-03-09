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

/* Prototypes */
#ifndef	IMPORTING 
extern DLLEXPORT int StereoInitialize (int image_width, int image_height); 
extern DLLEXPORT int StereoTerminate (void);
extern DLLEXPORT int LoadStereoParameters (char *strFileName);
extern DLLEXPORT int LoadStereoRectificationIndexes (char *strFileName);
extern DLLEXPORT int StereoTriangulation (int num_points, double *XL, double *XR, double *xL, double *xR);
extern DLLEXPORT int RectifyStereoPair (unsigned char *rect_img_left, unsigned char *rect_img_right, unsigned char *img_left, unsigned char *img_right, int w, int h);
extern DLLEXPORT int RectifyLeftImage (unsigned char *rect_img_left, unsigned char *img_left, int w, int h);
extern DLLEXPORT int RectifyRightImage (unsigned char *rect_img_right, unsigned char *img_right, int w, int h);
extern DLLEXPORT int GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
extern DLLEXPORT int CameraProjection (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
#else 
extern DLLEXPORT int _StereoInitialize (void); 
extern DLLEXPORT int _StereoTerminate (void);
extern DLLEXPORT int _LoadStereoParameters (char *strFileName);
extern DLLEXPORT int _LoadStereoRectificationIndexes (char *strFileName);
extern DLLEXPORT int _StereoTriangulation (double *XL, double *XR, double *xL, double *xR);
extern DLLEXPORT int _RectifyStereoPair (unsigned char *rect_img_left, unsigned char *rect_img_right, unsigned char *img_left, unsigned char *img_right, int w, int h);
extern DLLEXPORT int _RectifyLeftImage (unsigned char *rect_img_left, unsigned char *img_left, int w, int h);
extern DLLEXPORT int _RectifyRightImage (unsigned char *rect_img_right, unsigned char *img_right, int w, int h);
extern DLLEXPORT int _GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide);
extern DLLEXPORT int _CameraProjection (double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide);
#define StereoInitialize _StereoInitialize
#define StereoTerminate _StereoTerminate
#define LoadStereoParameters _LoadStereoParameters
#define LoadStereoRectificationIndexes _LoadStereoRectificationIndexes
#define StereoTriangulation _StereoTriangulation
#define RectifyStereoPair _RectifyStereoPair
#define RectifyLeftImage _RectifyLeftImage
#define RectifyRightImage _RectifyRightImage
#define GetWorldPointAtDistance _GetWorldPointAtDistance
#define CameraProjection _CameraProjection
#endif
/* Exportable Variables */

#endif


