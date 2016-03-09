#ifndef _SIMULATOR_H
#define _SIMULATOR_H

// Definitions
#define SIMULATOR_VERSION               "1.1"
#define WINDOW_TITLE			"Virtual 3D Environment"
#define MONO_MODE			0
#define STEREO_MODE			1

// Real parameters type
#define SIMULATOR_FOVY			0
#define SIMULATOR_FAR			1
#define SIMULATOR_NEAR			2
#define SIMULATOR_FLOOR			3
#define SIMULATOR_FOCUS			4
#define SIMULATOR_CAMERA_SEPARATION	5
#define SIMULATOR_ROBOT_RADIUS		6
#define SIMULATOR_ROBOT_HEIGHT		7
#define SIMULATOR_TERRAIN_SIZE		8

// Interger parameters type
#define SIMULATOR_WINDOW_WIDTH		0
#define SIMULATOR_WINDOW_HEIGHT		1

//#define _TESTING_SIMULATOR

// Macros

// Types

// Exportable Variables

// Prototypes

#ifdef __cplusplus
extern "C" {
#endif
// Must be called before the module initialization
int SimulatorSetParameteri (int nParameter, int nValue);
int SimulatorSetParameterf (int nParameter, float fltValue);

int SimulatorInitialize (void);
int SimulatorQuit (void);
int SimulatorRobotTurn (float fltAngle);
int SimulatorRobotMove (float fltStep);
int SimulatorLinkLeftImage (GLubyte *pLeftImage, int nImageWidth, int nImageHeight);
int SimulatorLinkRightImage (GLubyte *RightImage, int nImageWidth, int nImageHeight);
int SimulatorUnlinkLeftImage (void);
int SimulatorUnlinkRightImage (void);
int SimulatorForceUpdate (void);

#ifdef __cplusplus
}
#endif

#endif
