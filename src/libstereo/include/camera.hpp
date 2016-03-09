#ifndef _CAMERA_H
#define _CAMERA_H

// Includes
#include "common.hpp"

// Definitions

// Macros

// Types
class TCamera 
{
        public:
        TMatrix P; // Camera projection matrix
        TMatrix K; // Camera calibration matrix
        TMatrix R; // Camera rotation matrix
        TMatrix t; // Camera translation vector

	public:
	TCameraParameters parameters; 	 // Tsai's method camera parameters
        TCalibrationConstants constants; // Tsai's method calibration constants

	private:
	TCCDFormat ccd_format;		 // Camera CCD format
	int x_max_resolution;		 // Camera maximal horizontal resolution [pix]
	int y_max_resolution;  		 // Camera maximal vertical resolution [pix]
        int *p_nUndistortedImage;	 // Camera undistorted image buffer
	int image_width;		 // Camera image width
	int image_height;		 // Camera image height

        public:
        TCamera (int nXMaxResolution = 1280, int nYMaxResolution = 960, TCCDFormat eCCDFormat = HALF_INCH);
        ~TCamera (void);
	bool LoadCalibrationData (char *strDataFileName);
	void SetCameraParameters (int nXMaxResolution, int nYMaxResolution, TCCDFormat eCCDFormat);
	bool Calibrate (double *p_dblImagePoints, double *p_dblWorldPoints, int nPointsNumber, TCalibrationType eCalibrationType = COPLANAR_WITH_FULL_OPTIMIZATION);
	bool SetConstants (double *p_dblNewCameraConstants);
	bool GetConstants (double *p_dblCurrentCameraConstants);
	bool GetOrientation (double *p_dblCurrentCameraOrientation);
	bool SetOrientation (double *p_dblNewCameraOrientation);
	bool LoadConstants (char *strFileName);
	bool SaveConstants (char *strFileName);
	void ShowConstants (void);
	int *Distorted2UndistortedImage (int *p_nDistortedImage, int nImageWidth, int nImageHeight);
	int *Undistorted2DistortedImage (int *p_nUndistortedImage, int nImageWidth, int nImageHeight);
	bool CalculateProjectionMatrix (void);	
	TMatrix GetWorldPointAtDistance (TMatrix x, double dblDistance);
	TMatrix ProjectWorldPoint (TMatrix X);
	void Undistorted2DistortedImageCoordinate (double *p_dblUndistortedImagePoint, double *p_dblDistortedImagePoint);
};

// Exportable variables

// Prototypes

#endif
