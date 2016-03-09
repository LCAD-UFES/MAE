#ifndef _VIEW_H
#define _VIEW_H

// Includes
#include "common.hpp"
#include "convert.hpp"

// Definitions
#define CAMERA_DELTA_ANGLE	.5f
#define CAMERA_ZOOM_STEP	2.0f
#define FOVY			30.0f
#define NEAR			1.0f
#define FAR			10000.0f
#define ASPECT_RATIO            1.0f

// Macros

// Types
class TView
{
        public:
        TPoint position;                // view position
        TPoint fixationpoint;           // view fixation point
        float viewupdirection[3];       // view up direction vector
        int imagewidth;                 // image width
        int imageheight;                // image height
        float aperture;                 // visualization volume aperture

        private:
        float focallength;              // focal distance
	float near;                     // visualization volume far distance
        float far;                      // visualization volume far distance
        float aspectratio;              // image aspect ratio
        int windowid;                   // window id
        void (*drawscene) (void);       // draw scene function
        
        public:
        TView (float fltAperture = FOVY, float fltAspectRatio = ASPECT_RATIO, float fltNear = NEAR, float fltFar = FAR);
        ~TView (void);
        void SetPosition (float fltX, float fltY, float fltZ);
	void GetPosition (float *fltX, float *fltY, float *fltZ);
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
