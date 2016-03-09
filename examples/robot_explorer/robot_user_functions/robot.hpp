#ifndef _ROBOT_H
#define _ROBOT_H

// Includes
#include "common.hpp"
#include "camera.hpp"

// Definitions
#define	LEFT_CAMERA		0
#define	RIGHT_CAMERA		1
#define ROBOT_ANGLE_STEP	5.0f
#define ROBOT_MOVE_STEP		2.5f

// Macros

// Types
class TRobot
{
        // Attributes
        private:
	float height;
	float radius;
	
	// State Variables
	public:
	TPoint position;
	TEuler rotation;
	
	// Components
	//private:
	GLUquadricObj *quadric;
	TCamera *leftEye;
	TCamera *rightEye;
	
	// Methods
	public:
	TRobot (float fltRadius, float fltHeight, float fltCameraSeparation, float fltCameraHeight);
        ~TRobot (void);
        bool Move (float fltStep);
        void GetPosition (float *p_fltX, float *p_fltY) {*p_fltX = position.x; *p_fltY = position.y;}
        void SetPosition (float fltX, float fltY) {position.x = fltX; position.y = fltY;}
        bool Rotate (float fltAngle);
        void GetRotation (float *p_fltAlpha) {*p_fltAlpha = rotation.z;}
        void SetRotation (float fltAlpha) {rotation.z = fltAlpha;}
        void Draw (void);
};

// Global Variables

// Prototypes

#endif
