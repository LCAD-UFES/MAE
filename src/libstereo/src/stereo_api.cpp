/*! \file
* \brief Stereo system API module.
*/

// Includes
#include "stereo_system.hpp"
#include "stereo_api.hpp"

// Global variables
TStereoSystem *g_pStereoSystem = NULL;

/*! 
*********************************************************************************
* \brief Initializes the stereo system module.
* \pre None.
* \post The stereo system module initialized.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoInitialize (void)
{
	try
	{
		g_pStereoSystem = new TStereoSystem;
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while starting stereo system module (StereoInitialize).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Exits the stereo system module.	
* \pre None.
* \post The stereo system module finalized.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoQuit (void)
{
	try
	{
		delete g_pStereoSystem;
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while exiting stereo system module (StereoQuit).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Sets the camera parameters
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] nXMaxResolution The camera maximal horizontal resolution.
* \param[in] nYMaxResolution The camera maximal vertical resolution.
* \param[in] nCCDFormat The camera CCD format must be 0 (QUARTER_INCH), 1 (THIRD_INCH), 2 (HALF_INCH), 3 (TWO_THIRDS_INCH) or 4(ONE_INCH).
* \pre The stereo system module initialized.
* \post The camera parameters updated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoSetCameraParameters (int nCameraSide, int nXMaxResolution, int nYMaxResolution, int nCCDFormat)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.SetCameraParameters (nXMaxResolution, nYMaxResolution, (TCCDFormat) nCCDFormat);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.SetCameraParameters (nXMaxResolution, nYMaxResolution, (TCCDFormat) nCCDFormat);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoSetCameraParameters).");
		}
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while setting camera parameters (StereoSetCameraParameters).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Loads the camera constants from file.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] strFileName The file name.
* \pre The stereo module initialized.
* \post The camera constants loaded.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoLoadCameraConstants (int nCameraSide, char *strFileName)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.LoadConstants (strFileName);
				g_pStereoSystem->rightCamera.CalculateProjectionMatrix();
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.LoadConstants (strFileName);
				g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoLoadCameraConstants).");
		}
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while loading camera constants (StereoLoadCameraConstants).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Saves the camera constants to file.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] strFileName The file name.
* \pre The stereo module initialized.
* \post The camera constants saved.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoSaveCameraConstants (int nCameraSide, char *strFileName)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.SaveConstants (strFileName);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.SaveConstants (strFileName);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoSaveCameraConstants).");
		}
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while saving camera constants (StereoSaveCameraConstants).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Shows the camera constants.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \pre The stereo module initialized and the camera calibrated.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoShowCameraConstants (int nCameraSide)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.ShowConstants ();
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.ShowConstants ();
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoShowCameraConstants).");
		}
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while showing camera constants (StereoShowCameraConstants).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Sets the camera constants.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblNewCameraConstants The new camera constants.
* \pre The stereo module initialized.
* \post The camera constants updated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoSetCameraConstants (int nCameraSide, double *p_dblNewCameraConstants)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.SetConstants (p_dblNewCameraConstants);
				g_pStereoSystem->rightCamera.CalculateProjectionMatrix();
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.SetConstants (p_dblNewCameraConstants);
				g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoSetCameraConstants).");
		}
	} 
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while setting camera constants (StereoSetCameraConstants).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Gets the camera constants.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \pre The stereo module initialized and the camera calibrated.
* \post None.
* \return The current camera constants if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetCameraConstants (int nCameraSide)
{
	static double p_dblCurrentCameraConstants[19];

	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.GetConstants(p_dblCurrentCameraConstants);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.GetConstants(p_dblCurrentCameraConstants);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoGetCameraConstants).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting camera constants (StereoGetCameraConstants).\n";
		return (NULL);
	}
		
	return (p_dblCurrentCameraConstants);
}



/*! 
*********************************************************************************
* \brief Gets the camera orientation.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \pre The stereo module initialized and the camera calibrated.
* \post None.
* \return The current camera orientation if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetCameraOrientation (int nCameraSide)
{
	static double p_dblCurrentCameraOrientation[6];

	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.GetOrientation(p_dblCurrentCameraOrientation);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.GetOrientation(p_dblCurrentCameraOrientation);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoGetCameraOrientation).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting camera orientation (StereoGetCameraOrientation).\n";
		return (NULL);
	}
		
	return (p_dblCurrentCameraOrientation);
}



/*! 
*********************************************************************************
* \brief Sets the camera orientation.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblNewCameraOrientation The new camera orientation.
* \pre The stereo module initialized and the camera calibrated.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoSetCameraOrientation (int nCameraSide, double *p_dblNewCameraOrientation)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.SetOrientation(p_dblNewCameraOrientation);
				g_pStereoSystem->rightCamera.CalculateProjectionMatrix();
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.SetOrientation(p_dblNewCameraOrientation);
				g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoGetCameraOrientation).");
		}
		g_pStereoSystem->CalculateFundamentalMatrix();
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting camera orientation (StereoGetCameraOrientation).\n";
		return (-1);
	}
		
	return (0);
}



/*! 
*********************************************************************************
* \brief Calibrates the camera.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] nCalibrationType The calibration type must be 0 (COPLANAR), 1 (COPLANAR_WITH_FULL_OPTIMIZATION), 2 (NONCOPLANAR) or 3 (NONCOPLANAR_WITH_FULL_OPTIMIZATION).
* \param[in] p_dblImagePoints The camera image points [sub-pix].
* \param[in] p_dblWorldPoints The calibration target points in world frame coordinates [mm].
* \param[in] nPointsNumber The number of points.
* \pre The stereo module initialized.
* \post The camera calibrated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoCameraCalibration (int nCameraSide,
				       int nCalibrationType,
				       double *p_dblImagePoints,
				       double *p_dblWorldPoints,
				       int nPointsNumber)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.Calibrate(p_dblImagePoints, p_dblWorldPoints, nPointsNumber, (TCalibrationType) nCalibrationType);
				g_pStereoSystem->rightCamera.CalculateProjectionMatrix();
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.Calibrate(p_dblImagePoints, p_dblWorldPoints, nPointsNumber, (TCalibrationType) nCalibrationType);
				g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoCameraCalibration).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while calibrating camera (StereoCameraCalibration).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Takes of the image distortion.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_nDistortedImage The distorted image.
* \param[in] nImageWidth The distorted image width.
* \param[in] nImageHeight The distorted image height.
* \pre The stereo module initialized and the cameras calibrated.
* \post The camera image undistorted.
* \return a pointer to the undistorted image if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT int *StereoDistorted2UndistortedImage (int nCameraSide, int *p_nDistortedImage, int nImageWidth, int nImageHeight)
{
	try
	{	
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				return (g_pStereoSystem->rightCamera.Distorted2UndistortedImage (p_nDistortedImage, nImageWidth, nImageHeight));
			case LEFT_CAMERA:
				return (g_pStereoSystem->leftCamera.Distorted2UndistortedImage (p_nDistortedImage, nImageWidth, nImageHeight));
			default:
				throw TModuleError ("Invalid camera side (StereoDistorted2UndistortedImage).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while undistorting image (StereoDistorted2UndistortedImage).\n";
		return (NULL);
	}
}


/*! 
*********************************************************************************
* \brief Takes of the image distortion.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_nDistortedImage The distorted image.
* \param[in] nImageWidth The distorted image width.
* \param[in] nImageHeight The distorted image height.
* \pre The stereo module initialized and the cameras calibrated.
* \post The camera image undistorted.
* \return a pointer to the undistorted image if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT int *StereoUndistorted2DistortedImage (int nCameraSide, int *p_nUndistortedImage, int nImageWidth, int nImageHeight)
{
	try
	{	
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				return (g_pStereoSystem->rightCamera.Undistorted2DistortedImage (p_nUndistortedImage, nImageWidth, nImageHeight));
			case LEFT_CAMERA:
				return (g_pStereoSystem->leftCamera.Undistorted2DistortedImage (p_nUndistortedImage, nImageWidth, nImageHeight));
			default:
				throw TModuleError ("Invalid camera side (StereoUndistorted2DistortedImage).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while undistorting image (StereoUndistorted2DistortedImage).\n";
		return (NULL);
	}
}


/*! 
*********************************************************************************
* \brief Builds the epipolar geometry.
* \pre The stereo module initialized and the cameras calibrated. 
* \post The epipolar geometry matrixes computed.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoBuildEpipolarGeometry (void)
{
	try
	{	
		//g_pStereoSystem->rightCamera.CalculateProjectionMatrix();
		//g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
		g_pStereoSystem->CalculateFundamentalMatrix();
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while building epipolar geometry (StereoBuildEpipolarGeometry).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Given a image point x gets its corresponding epipolar line l' in the another image plane.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblImagePoint A image point of the epipolar line associated with the given camera.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return A 3-element vector with the epipolar line description if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetEpipolarLine (int nCameraSide, double *p_dblImagePoint)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				return (g_pStereoSystem->GetLeftEpipolarLine (p_dblImagePoint));
			case LEFT_CAMERA:
				return (g_pStereoSystem->GetRightEpipolarLine (p_dblImagePoint));
			default:
				throw TModuleError ("Invalid camera side (StereoGetEpipolarLine).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while computing corresponding epipolar line (StereoGetEpipolarLine).\n";
		return (NULL);
	}
}



/*! 
*********************************************************************************
* \brief Gets the epipolar line slope passing through a image point.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblImagePoint A image point of the epipolar line associated with the given camera.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The epipolar line slope if OK, -1.0 otherwise.
*********************************************************************************
*/

DLLEXPORT double StereoGetEpipolarLineSlope (int nCameraSide, double *p_dblImagePoint)
{
	try
	{	
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				return (g_pStereoSystem->GetRightEpipolarLineSlope(p_dblImagePoint));
			case LEFT_CAMERA:
				return (g_pStereoSystem->GetLeftEpipolarLineSlope(p_dblImagePoint));
			default:
				throw TModuleError ("Invalid camera side (StereoGetEpipolarLineSlope).");
		}			
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1.0);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while computing epipolar line slope (StereoGetEpipolarLineSlope).\n";
		return (-1.0);
	}
}



/*! 
*********************************************************************************
* \brief Gets the camera epipole.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The camera epipole if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetEpipole (int nCameraSide)
{
	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				return (g_pStereoSystem->GetLeftEpipole ());
			case LEFT_CAMERA:
				return (g_pStereoSystem->GetRightEpipole ());
			default:
				throw TModuleError ("Invalid camera side (StereoGetEpipole).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting epipole (StereoGetEpipole).\n";
		return (NULL);
	}
}



/*! 
*********************************************************************************
* \brief Maps the image coordinates to the world coordinates.
* \param[in] p_dblRightImagePoint The right image point.
* \param[in] p_dblLeftImagePoint The left image point.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return A pointer to he world point if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoImage2WorldPoint (double *p_dblRightImagePoint, double *p_dblLeftImagePoint)
{
	try
	{	
		return (g_pStereoSystem->Image2WorldPoint(p_dblRightImagePoint, p_dblLeftImagePoint));
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while mapping image to world coordinates (StereoImage2WorldPoint).\n";
		return (NULL);
	}
}



/*! 
*********************************************************************************
* \brief Gets the infinity point.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblImagePoint The image point.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The infinity point if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetInfinityPoint (int nCameraSide, double *p_dblImagePoint)
{
	static double p_dblInfinityPoint[2];

	try
	{
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->GetRightInfinityPoint (p_dblInfinityPoint, p_dblImagePoint);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->GetLeftInfinityPoint (p_dblInfinityPoint, p_dblImagePoint);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoGetInfinityPoint).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting epipole (StereoGetInfinityPoint).\n";
		return (NULL);
	}
	
	return (p_dblInfinityPoint);
}



/*! 
*********************************************************************************
* \brief Gets the point in one image.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblImagePoint The image point.
* \param[in] dblDistance The world point distance.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The infinity point if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoGetWorldPointAtDistance (int nCameraSide, double *p_dblImagePoint, double dblDistance)
{
	static double p_dblWorldPoint[3];
	TMatrix X(4,1);
	TMatrix x(3,1);

	try
	{
		x(0,0) = p_dblImagePoint[0];
		x(1,0) = p_dblImagePoint[1];
		x(2,0) = 1.0;
		
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				X = g_pStereoSystem->rightCamera.GetWorldPointAtDistance (x, dblDistance);
				break;
			case LEFT_CAMERA:
				X = g_pStereoSystem->leftCamera.GetWorldPointAtDistance (x, dblDistance);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoGetWorldPointAtDistance).");
		}

		p_dblWorldPoint[0] = X(0,0) / X(3,0);
		p_dblWorldPoint[1] = X(1,0) / X(3,0);
		p_dblWorldPoint[2] = X(2,0) / X(3,0);
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting epipole (StereoGetInfinityPoint).\n";
		return (NULL);
	}
	
	return (p_dblWorldPoint);
}



/*! 
*********************************************************************************
* \brief Projects a world point in the image plane.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblImagePoint The image point.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The image point if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoProjectWorldPoint (int nCameraSide, double *p_dblWorldPoint)
{
	static double p_dblImagePoint[2];
	TMatrix X(4,1);
	TMatrix x(3,1);

	try
	{
		X(0,0) = p_dblWorldPoint[0];
		X(1,0) = p_dblWorldPoint[1];
		X(2,0) = p_dblWorldPoint[2];
		X(3,0) = 1.0;

		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				x = g_pStereoSystem->rightCamera.ProjectWorldPoint (X);
				break;
			case LEFT_CAMERA:
				x = g_pStereoSystem->leftCamera.ProjectWorldPoint (X);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoProjectWorldPoint).");
		}

		p_dblImagePoint[0] = x(0,0) / x(2,0);
		p_dblImagePoint[1] = x(1,0) / x(2,0);
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while projecting world point (StereoProjectWorldPoint).\n";
		return (NULL);
	}
	
	return (p_dblImagePoint);
}



/*! 
*********************************************************************************
* \brief Sets the baseline length.
** \param[in] dblNewBaselineLength The new baseline length.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoSetBaselineLength (double dblNewBaselineLength)
{
	try
	{	
		TMatrix t(3,1);
		t = g_pStereoSystem->leftCamera.t;
		t = dblNewBaselineLength * t / t.Norm();
		g_pStereoSystem->leftCamera.constants.Tx = t(0,0);
		g_pStereoSystem->leftCamera.constants.Ty = t(1,0);
		g_pStereoSystem->leftCamera.constants.Tz = t(2,0);		
		g_pStereoSystem->leftCamera.CalculateProjectionMatrix();
		g_pStereoSystem->CalculateFundamentalMatrix();
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while setting baseline length (StereoSetBaselineLength).\n";
		return (-1);
	}

	return (0);
}



/*! 
*********************************************************************************
* \brief Gets the baseline length.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The baseline length if OK, -1.0 otherwise.
*********************************************************************************
*/

DLLEXPORT double StereoGetBaselineLength (void)
{
	double dblBaselineLength;

	try
	{	
		dblBaselineLength = g_pStereoSystem->leftCamera.t.Norm();
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (-1.0);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while setting baseline length (StereoSetBaselineLength).\n";
		return (-1.0);
	}

	return (dblBaselineLength);
}



/*! 
*********************************************************************************
* \brief Gets the baseline length.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return The baseline length if OK, -1.0 otherwise.
*********************************************************************************
*/

DLLEXPORT double *StereoUndistorted2DistortedImageCoordinate (int nCameraSide, double *p_dblUndistortedImagePoint)
{
	static double p_dblDistortedImagePoint[2];

	try
	{		
		switch (nCameraSide)
		{
			case RIGHT_CAMERA:
				g_pStereoSystem->rightCamera.Undistorted2DistortedImageCoordinate (p_dblUndistortedImagePoint, p_dblDistortedImagePoint);
				break;
			case LEFT_CAMERA:
				g_pStereoSystem->leftCamera.Undistorted2DistortedImageCoordinate (p_dblUndistortedImagePoint, p_dblDistortedImagePoint);
				break;
			default:
				throw TModuleError ("Invalid camera side (StereoUndistorted2DistortedImageCoordinate).");
		}
	}
	catch (TModuleError e)
	{
		cerr << "Error: " << e.Message () << endl;
		return (NULL);
	}
	catch (...)
	{
		cerr << "Error: unexpected error while getting epipole (StereoUndistorted2DistortedImageCoordinate).\n";
		return (NULL);
	}

	return (p_dblDistortedImagePoint);
}


#ifdef _TESTING_STEREO_API
int main (int argc, char **argv)
{
	//int i;
	//double *p_dblSystemParameters;

	if (StereoInitialize ())
		cout << "Error: cannot initialize stereo system module.\n";

/*	stereoSystemCalibration (COPLANAR_WITH_FULL_OPTIMIZATION,
				 p_dblRightImagePoints, p_dblLeftImagePoints,
				 p_dblWorldPoints, nPointsNumber);

	if ((p_dblSystemParameters = stereoGetSystemParameters ()) == NULL)
		cout << "Error: cannot get stereo system parameters.\n";

	for (i = 0; i < 38; i++)
		cout << p_dblSystemParameters[i] << endl;*/

	if (StereoQuit ())
		cout << "Error: cannot quit stereo system module.\n";

	/*TMatrix A, null, X;
	A.SetSize(3,3);
	A.Unit();
	A(1,1) = 10;
	cout << A;
	null.Null(3,1);
	//null(1,0) = 5;
	cout << null;
	X.SetSize(3,1);
	X = A.Solve(null);
	cout << X;*/
	
	/*TMatrix M(3,3);
	M.Unit();
	M.SetSize(4,3);
	M(1,1) = -2;
	M(1,2) = 10;
	cout << M.PseudoInv() * M;*/
	
	//double x = 20.0 * M_PI / 180.0;
	//double y = 5.0 * M_PI / 180.0;
	//double z = 88.0 * M_PI / 180.0;

	/*TMatrix Rx(3,3);	
	Rx(0,0) = 1.0;         	Rx(0,1) = 0.0;         	Rx(0,2) = 0.0;
        Rx(1,0) = 0.0;         	Rx(1,1) = cos(x);   	Rx(1,2) = sin(x);
        Rx(2,0) = 0.0;         	Rx(2,1) = -sin(x);  	Rx(2,2) = cos(x);
	
	TMatrix Ry(3,3);	
	Ry(0,0) = cos(y);  	Ry(0,1) = 0.0;         	Ry(0,2) = -sin(y);
        Ry(1,0) = 0.0;         	Ry(1,1) = 1.0;   	Ry(1,2) = 0.0;
        Ry(2,0) = sin(y);  	Ry(2,1) = 0.0;  	Ry(2,2) = cos(y);
	
        TMatrix Rz(3,3);	
	Rz(0,0) = cos(z);  	Rz(0,1) = sin(z);    	Rz(0,2) = 0.0;
        Rz(1,0) = -sin(z); 	Rz(1,1) = cos(z);   	Rz(1,2) = 0.0;
        Rz(2,0) = 0.0;	       	Rz(2,1) = 0.0;  	Rz(2,2) = 1.0;

	TMatrix R(3,3);
	R = Rz * Ry * Rx;
	cout << "Camera rotation matrix (R)\n" << R << endl;*/

	/*double r1,
              r2,
              r3,
              r4,
              r5,
              r6,
              r7,
              r8,
              r9,
              sa,
              sb,
              sg,
              ca,
              cb,
              cg;
	
	SINCOS (x, sa, ca);
	SINCOS (y, sb, cb);
	SINCOS (z, sg, cg);
	r1 = cb * cg;
	r2 = cg * sa * sb - ca * sg;
	r3 = sa * sg + ca * cg * sb;
	r4 = cb * sg;
	r5 = sa * sb * sg + ca * cg;
	r6 = ca * sb * sg - cg * sa;
	r7 = -sb;
	r8 = cb * sa;
	r9 = ca * cb;
	
	TMatrix R(3,3);
	R(0,0) = r1;         R(0,1) = r2;         R(0,2) = r3;
        R(1,0) = r4;         R(1,1) = r5;         R(1,2) = r6;
        R(2,0) = r7;         R(2,1) = r8;         R(2,2) = r9;
        cout << "Camera rotation matrix (R)\n" << R << endl;*/
	return (0);
}
#endif
