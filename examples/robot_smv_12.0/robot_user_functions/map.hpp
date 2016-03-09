#ifndef _MAP_H
#define _MAP_H

// Includes
#include "common.hpp"
#include "convert.hpp"

// Definitions
#define UNKNOWN         	-1.0f

// Macros

// Types


class TMap
{
	private:
	int width;                      // map matrix width
        int height;                     // map matrix height
        int currentLayer;               // map current layer
        int layersNumber;               // map layers number
	float betaMin;                  // minimal angle from XY plane
        float betaMax;                  // maximal angle from XY plane
	float near;			// nearest cut plane
	float far;			// fartest cut plane
	float Ki;                       // horizontal convertion constant
	float Kj;                       // vertical convertion constant
	TPoint **grid;                  // grid layers vector
        GLubyte **image;                // image layers vector
        float **depthMap;               // depth map layers vector
	GLuint imagePlaneTex;
        
	public:
  TPoint position;                // view position
	TMap (int nWidth, int nHeight, float fltBetaMin, float fltBetaMax, int nLayersNumber = 1, float fltNear = 0.1f, float fltFar = 100.0f, float fltTerrainSize = 500.0f);
	~TMap (void);
	bool Load (char *strFileName);
	bool Save (char *strFileName);
	bool SavePointCloud (char *strFileName);
	bool SaveImage (char *strFileName);
        void Update (float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue);
	void Erase (void);
	void GetNextLayer (void);
	void Draw (void);
	void ShowImage (void);
	void ShowDepthMap (void);
	void SetPosition (float fltX, float fltY, float fltZ);
	void GetPosition (float *fltX, float *fltY, float *fltZ);
	
	protected:
        float GetAlpha (int i);
        float GetBeta (int j);
};

// Prototypes

#endif
