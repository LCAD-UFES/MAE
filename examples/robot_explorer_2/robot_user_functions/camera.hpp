#ifndef _CAMERA_H
#define _CAMERA_H

// Includes
#include "common.hpp"
#include "convert.hpp"

// Definitions
#define CAMERA_DELTA_ANGLE	.1f
#define CAMERA_ZOOM_STEP	1.5f
#define FOVY			30.0f
#define NEAR			1.0f
#define FAR			2000.0f
#define ASPECT_RATIO            1.0f

// Macros

// Types
class TCamera
{
        public:
        TPoint position;                // camera position
        TPoint fixationpoint;           // camera fixation point
        float viewupdirection[3];       // camera view up direction vector
        int imagewidth;                 // camera image width
        int imageheight;                // camera image height
        float aperture;                 // visualization volume aperture

        private:
        float focallength;              // camera focal distance
	float near;                     // visualization volume far distance
        float far;                      // visualization volume far distance
        float aspectratio;              // camera image aspect ratio
        int windowid;                   // camera window id
        void (*drawscene) (void);       // draw scene function
        
        public:
        TCamera (float fltAperture = FOVY, float fltAspectRatio = ASPECT_RATIO, float fltNear = NEAR, float fltFar = FAR);
        ~TCamera (void);
        void SetPosition (float fltX, float fltY, float fltZ);
        void SetFixationPoint (float fltX, float fltY, float fltZ);
        void SetViewUpDirection (float *p_fltViewUpDirection);
        float GetFocalLength (void);
        void ConfigureVisualizationVolume (void);
        void ResizeImage (int nWindowWidth, int nWindowHeight);
        void RotateAroundFixationPoint (float fltDeltaAlpha, float fltDeltaBeta);
        void Rotate (float fltDeltaAlpha, float fltDeltaBeta);
        void Zoom (float fltZoom);
        void Draw (void);
};

// Global Variables

// Prototypes

#endif
