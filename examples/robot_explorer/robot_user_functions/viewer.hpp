#ifndef _VIEWER_H
#define _VIEWER_H

// Definitions
#define VIEWER_VERSION                  "3.0"
#define VIEWER_WINDOW_TITLE		"Map 3D"
#define VIEWER_DEFAULT_FOVY		50.0f
#define VIEWER_DEFAULT_ASPECT_RATIO	1.0f
#define VIEWER_DEFAULT_NEAR		.1f
#define VIEWER_DEFAULT_FAR		2500.0f
#define VIEWER_DEFAULT_FOCUS		400.0f
#define MAP_FILE_NAME                   "./maps/map.dat"
#define IMAGE_FILE_NAME                 "./maps/map_image.pnm"
#define VIEWER_SWAP_EVENT               0
#define EXTERNAL_VIEW                   0
#define WORLD_MAP_VIEW                  1
#define DEPTH_MAP_VIEW                  2
#define OCCUPANCY_MAP_VIEW              3
#define EVIDENCE_MAP_VIEW		4
#define POINT_SIZE			2

// Real parameters type
#define VIEWER_FOVY			0
#define VIEWER_FAR			1
#define VIEWER_NEAR			2
#define VIEWER_FLOOR			3
#define VIEWER_FOCUS			4
#define VIEWER_ROBOT_RADIUS		5
#define VIEWER_TERRAIN_SIZE		6

// Interger parameters type
#define VIEWER_WINDOW_WIDTH		0
#define VIEWER_WINDOW_HEIGHT		1
#define VIEWER_MAP_WIDTH		2
#define VIEWER_MAP_HEIGHT		3
#define VIEWER_IMAGE_WIDTH		4
#define VIEWER_IMAGE_HEIGHT		5
#define VIEWER_MAP_LAYERS_NUMBER	6

//#define _TESTING_VIEWER

// Macros

// Types

// Exported Variables

// Prototypes

#ifdef __cplusplus
extern "C" {
#endif
// Must be called before the module initialization
int ViewerSetParameteri (int nParameter, int nValue);
int ViewerSetParameterf (int nParameter, float fltValue);

int ViewerInitialize (void);
int ViewerUpdateMap (float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue);
int ViewerEraseMap(void);
int ViewerTransformMap (float fltAngle, float fltStep);
int ViewerGenerateOccupancyMap (void);
int ViewerUpdateEvidenceMap (float fltMinDistance);
int ViewerGetFuzzyInputs (double *p_dblFuzzyInputs, int nInputsNumber, float fltMinAlpha, float fltMaxAlpha);
int ViewerMakePath (void);
int ViewerMakePath2Point (float fltX, float fltY);
int ViewerGetPath (float *p_fltDistance, float *p_fltAngle);
int ViewerQuit (void);
#ifdef __cplusplus
}
#endif

#endif
