#ifndef _TARGET_H
#define _TARGET_H

// Includes
#include "common.hpp"

// Definitions

// Macros
#define TARGET_PIPE_SLICES	20
#define TARGET_PIPE_STACKS	20
#define TARGET_BASE_SLICES	20
#define TARGET_BASE_STACKS	20
#define RADIUS_TARGETS		100.0

// Types
class TTarget
{
        // Attributes
        private:
	float outerRadius;
	float width;
	float height;
	float depth;
	int numberRings;
	
	// State Variables
	public:
       	TPoint position;
       	
       	// Components
       	private:
	GLUquadricObj *quadric;
	
	// Methods
	public:
        TTarget (float fltOuterRadius, float fltWidth, float fltHeight, float fltDepth, int nNumberRings);
        ~TTarget (void);
        void SetPosition (float fltX, float fltY, float fltZ);
        void Draw (void);
};

// Global Variables

// Prototypes

#endif
