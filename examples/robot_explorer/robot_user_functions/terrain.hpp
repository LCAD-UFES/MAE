#ifndef _TERRAIN_H
#define _TERRAIN_H

// Includes
#include "common.hpp"

// Definitions

// Macros
#define TERRAIN_TEXTURE_FILE_NAME	"./terrain.pnm"
#define WALL_TEXTURE_FILE_NAME		"./wall.pnm"

// Types
class TTerrain
{
        private:
	float size;
	GLuint texFloor;
	GLuint texWall;
	
	public:
        TTerrain (float fltTerrainSize);
        ~TTerrain (void);
	void DrawWall (float fltWidth, float fltLength, float fltHeight);
        void Draw (void);
};

// Global Variables

// Prototypes

#endif
