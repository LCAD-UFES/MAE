#include "smv_api.h"

// Global variables
FILE *g_pLogFile = NULL;

/*! 
********************************************************************************
* \brief Initializes the MAE framework.
* \pre None.
* \post The MAE initialized.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeInitialize (void)
{
	init_pattern_xor_table ();
	init_measuraments ();
	build_network ();
	//init_user_functions ();

	// Initializes the stereo system module
	if (StereoInitialize (INPUT_WIDTH, INPUT_HEIGHT))
	{
		ERROR ("While initializing the MAE framework, stereo system module initialization failure (MaeInitialize).", "", "");
		return (-1);
	}
	
	LoadStereoParameters ("Calib_Results_stereo_rectified.mat");
	LoadStereoRectificationIndexes ("Calib_Results_rectification_indexes.mat");

	DEBUG("MaeInitialize","","");

    	return (0);
}



/*! 
********************************************************************************
* \brief Exits the MAE framework.
* \pre The MAE environment initialized.
* \post The MAE finalized.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeQuit (void)
{
    	if (StereoTerminate ())
	{
		ERROR ("While exiting the MAE framework, stereo system module finalization failure (MaeQuit).", "", "");
		return (-1);
	}

	DEBUG("MaeQuit","","");

	return (0);
}



/*! 
********************************************************************************
* \brief Sets the vergence parameters.
* \param[in] dblNearCutPlane The nearest vergence cut plane.
* \param[in] dblFarCutPlane The fartest vergence cut plane.
* \pre The MAE environment initialized.
* \post The.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeSetVergenceParameters (double dblNearCutPlane, double dblFarCutPlane)
{
	char strMessage[128];
	
	if (dblNearCutPlane < 0.0)
	{
		ERROR ("Invalid parameters, the cut planes distances must be greater then 0 (MaeSetVergenceParameters).", "", "");
		return (-1);
	}

	if (dblNearCutPlane >= dblFarCutPlane)
	{
		ERROR ("Invalid parameters, the far cut plane distance must be greater then the near cut plane distance (MaeSetVergenceParameters).", "", "");
		return (-1);
	}

	g_dblNearVergenceCutPlane = dblNearCutPlane;
	g_dblFarVergenceCutPlane  = dblFarCutPlane;

	sprintf (strMessage, "MaeSetVergenceParameters\ndblNearCutPlane = %f\ndblFarCutPlane = %f\n", dblNearCutPlane, dblFarCutPlane);
	DEBUG(strMessage,"","");

	return (0);
}



/*! 
********************************************************************************
* \brief Sets the camera constants.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_dblNewCameraConstants A vector with the 19 new camera constants.
* \pre The MAE environment initialized.
* \post The camera constants updated.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeSetCameraConstants (int nCameraSide, double* p_dblNewCameraConstants)
{	
	DEBUG("MaeSetCameraConstants","","");
	
	return 0;
	//return (StereoSetCameraConstants (nCameraSide, p_dblNewCameraConstants));
}



/*! 
********************************************************************************
* \brief Gets the camera constants.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \pre The MAE environment initialized.
* \post None.
* \return A vector with the 19 camera constants if OK, NULL otherwise.
********************************************************************************
*/

DLLEXPORT double* MaeGetCameraConstants(int nCameraSide)
{	
	DEBUG("MaeGetCameraConstants","","");

	return NULL;
	//return (StereoGetCameraConstants (nCameraSide));
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

DLLEXPORT int MaeSetCameraParameters (int nCameraSide, int nXMaxResolution, int nYMaxResolution, int nCCDFormat)
{
	DEBUG("MaeSetCameraParameters","","");

	return 0;
	//return (StereoSetCameraParameters (nCameraSide, nXMaxResolution, nYMaxResolution, nCCDFormat));
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

DLLEXPORT int MaeCameraCalibration (int nCameraSide,
			   	    int nCalibrationType,
				    double *p_dblImagePoints,
				    double *p_dblWorldPoints,
				    int nPointsNumber)
{
	DEBUG("MaeCameraCalibration","","");

	return 0;
/*	return (StereoCameraCalibration (nCameraSide,
				       nCalibrationType,
				       p_dblImagePoints,
				       p_dblWorldPoints,
				       nPointsNumber));*/
}



/*! 
*********************************************************************************
* \brief Builds the stereo system geometry.
* \pre The stereo module initialized.
* \post The stereo system geometry calculated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int MaeBuildStereoSystem (void)
{
	DEBUG("MaeBuildStereoSystem","","");

	return 0;
	//return (StereoBuildEpipolarGeometry());
}



/*! 
*********************************************************************************
* \brief Updates the image input.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_nNewImage The new image pixel vector.
* \param[in] nImageWidth The image width.
* \param[in] nImageHeight The image height.
* \pre The stereo module initialized.
* \post The input image updated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int MaeUpdateImage (int nCameraSide, int *p_nNewImage, int nImageWidth, int nImageHeight)
{
        INPUT_DESC *pInput = NULL;
	int i, j, nPixel;
	
	DEBUG("MaeUpdateImage","","");

	// Gets the input descriptor
     	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			pInput = &(image_left);
			break;
		case RIGHT_CAMERA:
			pInput = &(image_right);
			break;
		default:
			ERROR ("Wrong camera side, the camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA) (MaeUpdateImage).", "", "");
			return (-1);
	}
        
	// Cheks the image dimentions
	if ((nImageWidth != pInput->ww) || (nImageHeight != pInput->wh))
	{
		ERROR ("Incompatible image dimentions (MaeUpdateImage).", "", "");
		return (-1);
	}

	// Updates the input image
	for (j = 0; j < nImageHeight; j++)
	{
        	for (i = 0; i < nImageWidth; i++)
        	{
		      nPixel = p_nNewImage[i + (nImageHeight - j - 1) * nImageWidth];

		      pInput->image[3 * (i + j * image_left.tfw) + 0] = (GLubyte) RED(nPixel);
		      pInput->image[3 * (i + j * image_left.tfw) + 1] = (GLubyte) GREEN(nPixel);
		      pInput->image[3 * (i + j * image_left.tfw) + 2] = (GLubyte) BLUE(nPixel);
        	}
	}

	// Updates the input neurons
	pInput->up2date = 0;
	update_input_neurons (pInput);
	
/*	g_pLogFile = fopen (EXECUTION_LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "MaeUpdateImage (%d)\n", nCameraSide);
	fclose (g_pLogFile);

	{
		FILE *pImageFile = fopen (pInput->name, "w");
		fprintf (pImageFile, "P3\n");
		fprintf (pImageFile, "# Test\n");
		fprintf (pImageFile, "%d %d\n", nImageWidth, nImageHeight);
		fprintf (pImageFile, "255\n");

		for (j = nImageHeight; j >= 0 ; j--)
        		for (i = 0; i < nImageWidth; i++)
       			      fprintf (pImageFile, "%d %d %d\n", pInput->image[3 * (i + j * pInput->tfw) + 0],
								 pInput->image[3 * (i + j * pInput->tfw) + 1],
								 pInput->image[3 * (i + j * pInput->tfw) + 2]);
		fclose (pImageFile);
	}*/
	return (0);
}



/*! 
*********************************************************************************
* \brief Maps the image point to world point.
* \param[in] p_nRightImagePoint The right image point coordinates.
* \pre The stereo module initialized and the stereo system created.
* \post The world point calculated.
* \return The world point mapped if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *MaeImage2WorldPoint (int *p_nRightImagePoint)
{
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblLeftWorldPoint[3];
	static double p_dblRightWorldPoint[3];
	char strMessage[128];

	DEBUG("MaeImage2WorldPoint","","");

        /*image_right.wxd = p_nRightImagePoint[0];
        image_right.wyd = p_nRightImagePoint[1];
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	p_dblRightPoint[0] = (double) image_right.wxd;
	p_dblRightPoint[1] = (double) (image_right.wh - image_right.wyd - 1);

	sprintf (strMessage, "x  = (%f, %f)\n", p_dblRightPoint[0], p_dblRightPoint[1]);
	DEBUG(strMessage,"","");

	set_vergence ();

	p_dblLeftPoint[0] = (double) image_left.wxd;
	p_dblLeftPoint[1] = (double) (image_left.wh - image_left.wyd - 1);*/
	
	image_right.wxd = p_nRightImagePoint[0];
	image_right.wyd = p_nRightImagePoint[1];
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	p_dblRightPoint[0] = (double) image_right.wxd;
	p_dblRightPoint[1] = (double) (image_right.wh - image_right.wyd - 1);
	sprintf (strMessage, "x  = (%f, %f)\n", p_dblRightPoint[0], p_dblRightPoint[1]);
	DEBUG(strMessage,"","");

//	set_vergence ();

	p_dblLeftPoint[0] = (double) image_left.wxd;
	p_dblLeftPoint[1] = (double) (image_left.wh - image_left.wyd - 1);
	sprintf (strMessage, "x' = (%f, %f)\n", p_dblLeftPoint[0], p_dblLeftPoint[1]);
	DEBUG(strMessage,"","");

	StereoTriangulation (1, p_dblLeftWorldPoint, p_dblRightWorldPoint, p_dblLeftPoint, p_dblRightPoint);
	
	sprintf (strMessage, "X = (%f, %f, %f)\n", p_dblRightWorldPoint[0], p_dblRightWorldPoint[1], p_dblRightWorldPoint[2]);
	DEBUG(strMessage,"","");

        // Gambi ISASHOW
/*        p_dblRightWorldPoint[0] /= 1000.0;
        p_dblRightWorldPoint[1] /= 1000.0;
        p_dblRightWorldPoint[2] /= 1000.0;*/
 
       	return (p_dblRightWorldPoint);
}


/*! 
*********************************************************************************
* \brief Maps the image point to world point manually.
* \param[in] p_nRightImagePoint The right image point coordinates.
* \param[in] p_nLeftImagePoint The left image point coordinates.
* \pre The stereo module initialized and the stereo system created.
* \post The world point calculated.
* \return The world point mapped if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT double *MaeImage2WorldPointManual (int *p_nRightImagePoint, int *p_nLeftImagePoint)
{
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblLeftWorldPoint[3];
	static double p_dblRightWorldPoint[3];
	char strMessage[128];

	DEBUG("MaeImage2WorldPointManual","","");

       	/*image_right.wxd = p_nRightImagePoint[0];
        image_right.wyd = p_nRightImagePoint[1];
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	p_dblRightPoint[0] = (double) image_right.wxd;
	p_dblRightPoint[1] = (double) (image_right.wh - image_right.wyd - 1);

	sprintf (strMessage, "x  = (%f, %f)\n", p_dblRightPoint[0], p_dblRightPoint[1]);
	DEBUG(strMessage,"","");

	image_left.wxd = p_nLeftImagePoint[0];
        image_left.wyd = p_nLeftImagePoint[1];
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

	p_dblLeftPoint[0] = (double) image_left.wxd;
	p_dblLeftPoint[1] = (double) (image_left.wh - image_left.wyd - 1);*/
	
	image_right.wxd = p_nRightImagePoint[0];
        image_right.wyd = p_nRightImagePoint[1];
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	p_dblRightPoint[0] = (double) image_right.wxd;
	p_dblRightPoint[1] = (double) (image_right.wh - image_right.wyd - 1);
	sprintf (strMessage, "x  = (%f, %f)\n", p_dblRightPoint[0], p_dblRightPoint[1]);
	DEBUG(strMessage,"","");

	image_left.wxd = p_nLeftImagePoint[0];
        image_left.wyd = p_nLeftImagePoint[1];
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

	p_dblLeftPoint[0] = (double) image_left.wxd;
	p_dblLeftPoint[1] = (double) (image_left.wh - image_left.wyd - 1);
	sprintf (strMessage, "x' = (%f, %f)\n", p_dblLeftPoint[0], p_dblLeftPoint[1]);
	DEBUG(strMessage,"","");

	StereoTriangulation (1, p_dblLeftWorldPoint, p_dblRightWorldPoint, p_dblLeftPoint, p_dblRightPoint);

	sprintf (strMessage, "X = (%f, %f, %f)\n", p_dblRightWorldPoint[0], p_dblRightWorldPoint[1], p_dblRightWorldPoint[2]);
	DEBUG(strMessage,"","");

        // Gambi ISASHOW
/*        p_dblRightWorldPoint[0] /= 1000.0;
        p_dblRightWorldPoint[1] /= 1000.0;
        p_dblRightWorldPoint[2] /= 1000.0;*/
        
       	return (p_dblRightWorldPoint);
}



/*! 
*********************************************************************************
* \brief Gets the image left point coordinates.
* \pre The stereo module initialized, the stereo system created and the vergence ready.
* \post None.
* \return The image left coordinates if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT int *MaeGetImageLeftPoint (void)
{
	static int p_nLeftImagePoint[2];
	char strMessage[128];

	DEBUG("MaeGetImageLeftPoint","","");

     	p_nLeftImagePoint[0] = image_left.wxd;
	p_nLeftImagePoint[1] = image_left.wyd;
	sprintf (strMessage, "x' = (%d, %d)\n", p_nLeftImagePoint[0], p_nLeftImagePoint[1]);
	DEBUG(strMessage,"","");

  	return (p_nLeftImagePoint);
}


/*! 
*********************************************************************************
* \brief Gets the image epipolar line edges.
* \param[in] p_nRightImagePoint The right image point.
* \pre The stereo module initialized, the stereo system created and the vergence ready.
* \post None.
* \return The left epipolar line edges if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT int *MaeGetLeftEpipolarLine (int *p_nRightImagePoint)
{
	/*static int p_nLinesEdges[4];
	int nMinScanX, nMaxScanX;
	
        DEBUG("MaeGetLeftEpipolarLine","","");
		
	nMinScanX = p_nRightImagePoint[0];
	nMaxScanX = p_nRightImagePoint[0] + (image_right.neuron_layer->dimentions.x / 5);
	if (nMaxScanX > image_right.neuron_layer->dimentions.x)
		nMaxScanX = image_right.neuron_layer->dimentions.x;
	
	p_nLinesEdges[0] = nMinScanX;
	p_nLinesEdges[1] = p_nRightImagePoint[1];
	p_nLinesEdges[2] = nMaxScanX;
	p_nLinesEdges[3] = p_nRightImagePoint[1];*/

        static int p_nLinesEdges[4];
        int nMinScanX, nMaxScanX;
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3]; 

	p_dblRightPoint[0] = (double) p_nRightImagePoint[0];
	p_dblRightPoint[1] = (double) (image_right.wh - p_nRightImagePoint[1] - 1);

       	// Gets the fartest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nMinScanX = (int) (p_dblLeftPoint[0] + 0.5);
	nMinScanX = (nMinScanX < 0) ? 0 : nMinScanX;

	// Gets the nearest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nMaxScanX = (int) (p_dblLeftPoint[0] + 0.5);
	nMaxScanX = (nMaxScanX >= image_left.ww) ? image_left.ww - 1: nMaxScanX;

	p_nLinesEdges[0] = nMinScanX;
	p_nLinesEdges[1] = image_left.wh - (int) (p_dblLeftPoint[1] + 0.5) - 1;
	p_nLinesEdges[2] = nMaxScanX;
	p_nLinesEdges[3] = image_left.wh - (int) (p_dblLeftPoint[1] + 0.5) - 1;

	/*int nMinScanX, nMaxScanX;
	double p_dblRightPoint[2], *p_dblLeftPoint = NULL, *p_dblWorldPoint = NULL, *p_dblLeftEpipolarLine = NULL; 

	p_dblRightPoint[0] = (double) (image_right.ww - p_nRightImagePoint[0] - 1);
	p_dblRightPoint[1] = (double) p_nRightImagePoint[1];

	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblFarVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMinScanX = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMinScanX = (nMinScanX < 0) ? 0 : nMinScanX;

	// Gets the nearest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblNearVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMaxScanX = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMaxScanX = (nMaxScanX > image_left.ww) ? image_left.ww : nMaxScanX;

	// Gets the left epipolar line
	p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);

	p_nLinesEdges[0] = nMinScanX;
	p_nLinesEdges[1] = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - nMinScanX - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
	p_nLinesEdges[2] = nMaxScanX;
	p_nLinesEdges[3] = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - nMaxScanX - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
*/
  	return (p_nLinesEdges);
}



/*! 
*********************************************************************************
* \brief Takes of the image distortion.
* \param[in] nCameraSide The camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA).
* \param[in] p_nDistortedImage The distorted image.
* \param[in] nImageWidth The distorted image width.
* \param[in] nImageHeight The distorted image height.
* \pre The MAE framework initialized and the cameras calibrated.
* \post The camera image undistorted.
* \return A pointer to the undistorted image if OK, NULL otherwise.
*********************************************************************************
*/

DLLEXPORT int *MaeDistorted2UndistortedImage (int *p_nUndistortedImage, int nCameraSide, int *p_nDistortedImage, int w, int h)
{
	unsigned char *p_cImageBuffer;
	int i, j, nPixel;
	unsigned char cRed, cGreen, cBlue;
	
	DEBUG("MaeDistorted2UndistortedImage","","");
	
	p_cImageBuffer = (unsigned char *) alloc_mem (w * h * 3 * sizeof (unsigned char));
        for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
                        nPixel = p_nDistortedImage[i + j * w];
                        p_cImageBuffer[j + i * h + 0 * w * h] = RED(nPixel);
                        p_cImageBuffer[j + i * h + 1 * w * h] = GREEN(nPixel);
                        p_cImageBuffer[j + i * h + 2 * w * h] = BLUE(nPixel);			
		}	
        }
        
        // Gets the input descriptor
     	switch (nCameraSide)
	{
		//case LEFT_CAMERA:
                case RIGHT_CAMERA:
			RectifyLeftImage(p_cImageBuffer, p_cImageBuffer, w, h);
			break;
		//case RIGHT_CAMERA:
                case LEFT_CAMERA:
			RectifyRightImage(p_cImageBuffer, p_cImageBuffer, w, h);
			break;
		default:
			ERROR ("Wrong camera side, the camera side must be 0 (RIGHT_CAMERA) or 1 (LEFT_CAMERA) (MaeDistorted2UndistortedImage).", "", "");
			return (NULL);
	}
	
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
                        cRed = p_cImageBuffer[j + i * h + 0 * w * h];
	                cGreen = p_cImageBuffer[j + i * h + 1 * w * h];
        		cBlue = p_cImageBuffer[j + i * h + 2 * w * h];
                        p_nUndistortedImage[i + j * w] = PIXEL(cRed,cGreen,cBlue);
                }
	}

	free (p_cImageBuffer);
	return p_nUndistortedImage;
}
