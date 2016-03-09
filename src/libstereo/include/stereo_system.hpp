#ifndef _STEREO_SYSTEM_H
#define _STEREO_SYSTEM_H

// Includes
#include "camera.hpp"

// Definitions

// Macros

// Types
class TStereoSystem
{
	public:
	TCamera leftCamera;	// Left camera
	TCamera rightCamera;	// Right camera

	private:
	TMatrix F;	// Fundamental matrix
	TMatrix el;	// Left epipole
	TMatrix er;	// Right epipole

	public:
	TStereoSystem (void);
	~TStereoSystem (void);
	bool CalculateFundamentalMatrix (void);
	double *GetRightEpipolarLine (double *p_dblLeftImagePoint);
	double *GetLeftEpipolarLine (double *p_dblRightImagePoint);
	double *GetRightEpipole (void);
	double *GetLeftEpipole (void);
	double GetRightEpipolarLineSlope (double *p_dblRightImagePoint);
	double GetLeftEpipolarLineSlope (double *p_dblLeftImagePoint);
	double *Image2WorldPoint (double *p_dblRightImagePoint, double *p_dblLeftImagePoint);
	void GetRightInfinityPoint (double *p_dblRightPoint, double *p_dblLeftPoint);
	void GetLeftInfinityPoint (double *p_dblLeftPoint, double *p_dblRightPoint);
};

// Exportable variables

// Prototypes


#endif
