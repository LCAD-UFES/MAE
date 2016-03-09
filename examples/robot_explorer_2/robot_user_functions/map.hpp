#ifndef _MAP_H
#define _MAP_H

// Includes
#include "common.hpp"
#include "convert.hpp"
#include "list.hpp"

// Definitions
#define UNKNOWN         	-1.0f
#define UNLINKED		-1

// Cell types
#define OCCUPIED_CELL		0xFFFFFF01
#define WARNING_CELL		0xCFCFCF02
#define FREE_CELL		0x00000004
#define UNKNOWN_CELL		0x7F7F7F08
#define UNKNOWN_FRONTIER_CELL	0x5F5F5F10
#define WARNING_FRONTIER_CELL	0xAFAFAF20
#define COLOR_MASK		0xFFFFFF00
#define INDEX_MASK		0x000000FF

#define SCAN_LEFT		0
#define SCAN_RIGHT		1

#define MAX_PATH_DEPTH		5
#define MAX_PATH_LENGTH		1000.0f
#define MAX_NODES		127
#define LEFT_NODE		1.0f
#define RIGHT_NODE		2.0f
#define NULL_NODE		-1.0f

// Macros

// Types

typedef struct 
{
	unsigned int type;
	int length;
	int centroidX;
	int centroidY;
} TFrontier;

typedef struct 
{
	int left;
	int right;
} TFrontierCell;

class TMap
{
	private:
	int width;                      // map matrix width
        int height;                     // map matrix height
        int currentLayer;               // map current layer
        int layersNumber;               // map layers number
	int espheresNumber;             
	float betaMin;                  // minimal angle from XY plane
        float betaMax;                  // maximal angle from XY plane
	float near;			// nearest cut plane
	float far;			// fartest cut plane
	float Ki;                       // horizontal convertion constant
	float Kj;                       // vertical convertion constant
	TPoint ***grid;                  // grid layers vector
        GLubyte ***image;                // image layers vector
        float ***depthMap;               // depth map layers vector
	GLuint imagePlaneTex;
        float *occupancyGrid;        	// occupancy map grid
        unsigned int *evidenceGrid;    	// evidence map grid
        TFrontierCell *frontierGrid;    // frontier map grid
        int gridWidth;			// occupancy map grid width
        int gridLength;			// occupancy map grid length
        float areaBounds[4]; 		// occupancy map area bounds
	float increment;		// occupancy probability incremental step

	public:
        float robotX;			// robot X grid coordinate
	float robotY;			// robot Y grid coordinate
	float robotAlpha;		// robot Y grid coordinate

	public:
	TMap (int nWidth, int nHeight, float fltBetaMin, float fltBetaMax, int nLayersNumber = 1, int nEspheresNumber = 10, float fltNear = 0.1f, float fltFar = 100.0f, float fltTerrainSize = 500.0f);
	~TMap (void);
	bool Load (char *strFileName);
	bool Save (char *strFileName);
	bool SaveImage (char *strFileName);
        void Update (float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue);
	void Erase (void);
	void Rotate (float fltDeltaAlfa);
	void Translate (float fltStepX, float fltStepY);
	void Forward (float fltStep);
	void GetNextLayer (void);
	void Draw (void);
	void ShowImage (void);
	void ShowDepthMap (void);
	void ShowOccupancyMap (void);
	void ShowEvidenceMap (void);
	void UpdateOccupancyGrid (float X, float Y, float Z);
	void ExpandCell (int io, int jo, int nHorizontalMargin, int nVerticalMargin);
	bool HaveFreeAdjacentCell (int io, int jo);
	bool HaveCommonFreeAdjacentCells (int io, int jo, int ko);
	int LinkFrontierCells (int io, int jo,unsigned int nFrontierType, int *p_nFrontierCentroidX, int *p_nFrontierCentroidY);
	void GroupFrontierCells (unsigned int nFrontierType);
	bool GetNearestFrontier (unsigned int nFrontierType, int nMinimalFrontierLength, float *p_fltNearestFrontierX, float *p_fltNearestFrontierY);
	void UpdateEvidenceGrid (float fltRobotRadius);
	void GenerateOccupancyGrid (void);
	void GetFuzzyInputs (double *p_dblFuzzyInputs, int nInputsNumber, float fltMinAlpha, float fltMaxAlpha, float fltFloor);
	bool ScanLine (int nStartX, int nStartY, int nEndX, int nEndY, int *p_nReachedX, int *p_nReachedY,unsigned int nProhibitedCellType);
	bool ScanFrontier (int nFrontierX, int nFrontierY, int nCurrentStartX, int nCurrentStartY, int *p_nNewStartX, int *p_nNewStartY, int nSide);
	float GetPath (float fltStartX, float fltStartY, float fltEndX, float fltEndY, int nNodeIndex);
	void GetShortestPath (float fltGoalX, float fltGoalY);
	void Grid2Floor (int i, int j, float *p_fltX, float *p_fltY);
	void Floor2Grid (float fltX, float fltY, int *i, int *j);
	float FloorDistance (int nStartX, int nStartY, int nEndX, int nEndY);
	float FloorDistance (float fltStartX, float fltStartY, float fltEndX, float fltEndY);
	
	protected:
        float GetAlpha (int i);
        float GetBeta (int j);
};

// Prototypes
extern float g_p_fltPathSearchTree [];

#endif
