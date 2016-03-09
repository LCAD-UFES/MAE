#include "smv_api.h"



/************************************************************************/
/* Name: maeInitialize							*/
/* Description:	initializes the mae enviroment				*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeInitialize ()
{
	init_pattern_xor_table ();
	init_measuraments ();
	build_network ();
	init_user_functions ();
	g_pList = listCreate ();
	g_pUndistortedImage = (int *) malloc (IMG_WIDTH * IMG_HEIGHT * sizeof (int));

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "w");
	fprintf (g_pLogFile, "\nmaeInitialize\n");
	fclose (g_pLogFile);
#endif
    	return;
}



/************************************************************************/
/* Name: maeQuit							*/
/* Description:	quits mae enviroment					*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeQuit ()
{
    	listDestroy (g_pList);
 	free (g_pUndistortedImage);
	
#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeQuit\n");
	fclose (g_pLogFile);
#endif
	return;
}



/************************************************************************/
/* Name: maeSetCameraDistance						*/
/* Description:	sets the distance between left and right cameras	*/
/* Inputs: the new distance between cameras				*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeSetCameraDistance (double fltNewCameraDistance)
{
	CAMERA_DISTANCE = fltNewCameraDistance;

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeSetCameraDistance\n");
	fprintf (g_pLogFile, "\tfltNewCameraDistance = %f\n", CAMERA_DISTANCE);
	fclose (g_pLogFile);
#endif
	return;
}



/************************************************************************/
/* Name: maeSetVergenceParameters					*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

DLLEXPORT void maeSetVergenceParameters (int nCorrection, double dblVergenceWindowSize)
{
	g_nCorrection = nCorrection;
	
	g_fltVergenceWindowSize = (dblVergenceWindowSize < 0.0) ? 0.0 : 
				  (dblVergenceWindowSize > 1.0) ? 1.0 : 
				  (float) dblVergenceWindowSize;
	
#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeSetVergenceParameters\n");
	fprintf (g_pLogFile, "\tCorrection = %d\n", g_nCorrection);
	fprintf (g_pLogFile, "\tVergenceWindowSize = %f\n", g_fltVergenceWindowSize);
	fclose (g_pLogFile);
#endif
	return;
}



/************************************************************************/
/* Name: maeSetCameraParameters						*/
/* Description:	sets the camera parameters				*/
/* Inputs: the camera side and the new camera parameters		*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeSetCameraParameters (int nCameraSide, double* pNewCameraParameters)
{
	struct calibration_constants calibrationConstants;
	struct camera_parameters calibrationParameters;
	
	calibrationParameters.Ncx = pNewCameraParameters[0];
	calibrationParameters.Nfx = pNewCameraParameters[1];
	calibrationParameters.dx  = pNewCameraParameters[2];
	calibrationParameters.dy  = pNewCameraParameters[3];
	calibrationParameters.dpx = pNewCameraParameters[4];
	calibrationParameters.dpy = pNewCameraParameters[5];
	calibrationParameters.Cx  = pNewCameraParameters[6];
	calibrationParameters.Cy  = pNewCameraParameters[7];
	calibrationParameters.sx  = pNewCameraParameters[8];
	
	calibrationConstants.f  = pNewCameraParameters[9];
	calibrationConstants.kappa1 = pNewCameraParameters[10];
	calibrationConstants.Tx = pNewCameraParameters[11];
	calibrationConstants.Ty = pNewCameraParameters[12];
	calibrationConstants.Tz = pNewCameraParameters[13];
	calibrationConstants.Rx = pNewCameraParameters[14];
	calibrationConstants.Ry = pNewCameraParameters[15];
	calibrationConstants.Rz = pNewCameraParameters[16];
	calibrationConstants.p1 = pNewCameraParameters[17];
	calibrationConstants.p2 = pNewCameraParameters[18];
	
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cameraLeftCalibrationConstants  = calibrationConstants;
			cameraLeftCalibrationParameters = calibrationParameters;
			break;
		case RIGHT_CAMERA:
			cameraRightCalibrationConstants  = calibrationConstants;
			cameraRightCalibrationParameters = calibrationParameters;
			break;
	}
	
#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeSetCameraParameters\n");
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			fprintf (g_pLogFile, "\tnCameraSide = LEFT_CAMERA\n");
			break;
		case RIGHT_CAMERA:
			fprintf (g_pLogFile, "\tnCameraSide = RIGHT_CAMERA\n");
			break;
	}
	fprintf (g_pLogFile, "\tNcx = %f\n", calibrationParameters.Ncx);
	fprintf (g_pLogFile, "\tNfx = %f\n", calibrationParameters.Nfx);
	fprintf (g_pLogFile, "\tdx  = %f\n", calibrationParameters.dx);
	fprintf (g_pLogFile, "\tdy  = %f\n", calibrationParameters.dy);
	fprintf (g_pLogFile, "\tdpx = %f\n", calibrationParameters.dpx);
	fprintf (g_pLogFile, "\tdpy = %f\n", calibrationParameters.dpy);
	fprintf (g_pLogFile, "\tCx  = %f\n", calibrationParameters.Cx);
	fprintf (g_pLogFile, "\tCy  = %f\n", calibrationParameters.Cy);
	fprintf (g_pLogFile, "\tsx  = %f\n", calibrationParameters.sx);
	fprintf (g_pLogFile, "\tf  = %f\n", calibrationConstants.f);
	fprintf (g_pLogFile, "\tkappa1 = %f\n", calibrationConstants.kappa1);
	fprintf (g_pLogFile, "\tTx = %f\n", calibrationConstants.Tx);
	fprintf (g_pLogFile, "\tTy = %f\n", calibrationConstants.Ty);
	fprintf (g_pLogFile, "\tTz = %f\n", calibrationConstants.Tz);
	fprintf (g_pLogFile, "\tRx = %f\n", calibrationConstants.Rx);
	fprintf (g_pLogFile, "\tRy = %f\n", calibrationConstants.Ry);
	fprintf (g_pLogFile, "\tRz = %f\n", calibrationConstants.Rz);
	fprintf (g_pLogFile, "\tp1 = %f\n", calibrationConstants.p1);
	fprintf (g_pLogFile, "\tp2 = %f\n", calibrationConstants.p2);
	fclose (g_pLogFile);
#endif
    	return;
}



/************************************************************************/
/* Name: maeGetCameraParameters						*/
/* Description:	gets the camera parameters				*/
/* Inputs: the camera side						*/
/* Output: the camera parameters					*/
/************************************************************************/

DLLEXPORT double* maeGetCameraParameters (int nCameraSide)
{	
	struct calibration_constants calibrationConstants;
	struct camera_parameters calibrationParameters;
	
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			calibrationConstants  = cameraLeftCalibrationConstants;
			calibrationParameters = cameraLeftCalibrationParameters;
			break;
		case RIGHT_CAMERA:
			calibrationConstants  = cameraRightCalibrationConstants;
			calibrationParameters = cameraRightCalibrationParameters;
			break;
	}
	
	g_pCameraParameters[0] = calibrationParameters.Ncx;
	g_pCameraParameters[1] = calibrationParameters.Nfx;
	g_pCameraParameters[2] = calibrationParameters.dx;
	g_pCameraParameters[3] = calibrationParameters.dy;
	g_pCameraParameters[4] = calibrationParameters.dpx;
	g_pCameraParameters[5] = calibrationParameters.dpy;
	g_pCameraParameters[6] = calibrationParameters.Cx;
	g_pCameraParameters[7] = calibrationParameters.Cy;
	g_pCameraParameters[8] = calibrationParameters.sx;
	
	g_pCameraParameters[9]  = calibrationConstants.f;
	g_pCameraParameters[10] = calibrationConstants.kappa1;
	g_pCameraParameters[11] = calibrationConstants.Tx;
	g_pCameraParameters[12] = calibrationConstants.Ty;
	g_pCameraParameters[13] = calibrationConstants.Tz;
	g_pCameraParameters[14] = calibrationConstants.Rx;
	g_pCameraParameters[15] = calibrationConstants.Ry;
	g_pCameraParameters[16] = calibrationConstants.Rz;
	g_pCameraParameters[17] = calibrationConstants.p1;
	g_pCameraParameters[18] = calibrationConstants.p2;

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeGetCameraParameters\n");
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			fprintf (g_pLogFile, "\tnCameraSide = LEFT_CAMERA\n");
			break;
		case RIGHT_CAMERA:
			fprintf (g_pLogFile, "\tnCameraSide = RIGHT_CAMERA\n");
			break;
	}
	fclose (g_pLogFile);
#endif	
    	return (g_pCameraParameters);
}



/************************************************************************/
/* Name: maeInitializeIKWB11AParameters					*/
/* Description:	initializes the IK-WB11A camera parameters		*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void maeInitializeIKWB11AParameters (void)
{
	strcpy (camera_type, "IK-WB11A TOSHIBA");
	
	cp.Ncx = 1.0;				// [sel]
	cp.Nfx = 1.0;				// [pix]
	cp.dx = 1.0;				// [mm/sel]
	cp.dy = 1.0;				// [mm/sel]
	cp.dpx = cp.dx * cp.Ncx / cp.Nfx;	// [mm/pix]
	cp.dpy = cp.dy;				// [mm/pix]
	cp.Cx = 640.0;			// [pix]
	cp.Cy = 480.0;			// [pix] 
	//cp.Cx = 320.0;				// [pix]
	//cp.Cy = 240.0;				// [pix] 
	cp.sx = 1.0;				// []

	return;
}



/************************************************************************/
/* Name: maeInitializeCameraParameters					*/
/* Description:	initializes the camera parameters			*/
/* Inputs: the new camera parameters					*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeInitializeCameraParameters (double *pNewCameraParameters)
{
	strcpy (camera_type, "");

	cp.Ncx = pNewCameraParameters[0];
	cp.Nfx = pNewCameraParameters[1];
	cp.dx  = pNewCameraParameters[2];
	cp.dy  = pNewCameraParameters[3];
	cp.dpx = pNewCameraParameters[4];
	cp.dpy = pNewCameraParameters[5];
	cp.Cx  = pNewCameraParameters[6];
	cp.Cy  = pNewCameraParameters[7];
	cp.sx  = pNewCameraParameters[8];

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeInitializeCameraParameters\n");
	fprintf (g_pLogFile, "\tNcx = %f\n", cp.Ncx);
	fprintf (g_pLogFile, "\tNfx = %f\n", cp.Nfx);
	fprintf (g_pLogFile, "\tdx  = %f\n", cp.dx);
	fprintf (g_pLogFile, "\tdy  = %f\n", cp.dy);
	fprintf (g_pLogFile, "\tdpx = %f\n", cp.dpx);
	fprintf (g_pLogFile, "\tdpy = %f\n", cp.dpy);
	fprintf (g_pLogFile, "\tCx  = %f\n", cp.Cx);
	fprintf (g_pLogFile, "\tCy  = %f\n", cp.Cy);
	fprintf (g_pLogFile, "\tsx  = %f\n", cp.sx);
	fclose (g_pLogFile);
#endif
	return;
}



/************************************************************************/
/* Name: maeCameraCalibration						*/
/* Description:	starts the camera calibration process			*/
/* Inputs: 		 						*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeCameraCalibration (int nCameraSide, int nCalibrationType,
			   double *pPixelVectorX,
			   double *pPixelVectorY,
			   double *pPointsVectorX,
			   double *pPointsVectorY,
			   double *pPointsVectorZ,
			   int nPointsNumber)
{
	// Mounts the calibration data
	for (cd.point_count = 0; cd.point_count < nPointsNumber; cd.point_count++)
	{
		cd.xw[cd.point_count] = pPointsVectorX[cd.point_count];
		cd.yw[cd.point_count] = pPointsVectorY[cd.point_count];
		cd.zw[cd.point_count] = pPointsVectorZ[cd.point_count];
		cd.Xf[cd.point_count] = pPixelVectorX[cd.point_count];
		cd.Yf[cd.point_count] = pPixelVectorY[cd.point_count];
		
		printf ("%2d - (%.3lf, %.3lf) -> (%3.1lf, %3.1lf, %3.1lf)\n",cd.point_count, cd.Xf[cd.point_count],
			cd.Yf[cd.point_count], cd.xw[cd.point_count], cd.yw[cd.point_count], cd.zw[cd.point_count]);
	}
	
	// Initializes the camera photometrics parameters
	//initialize_photometrics_parms ();
	maeInitializeIKWB11AParameters ();
	
	// Calibrates the camera
	switch (nCalibrationType)
	{
		case COPLANAR:
			coplanar_calibration ();
			break;
		case COPLANAR_WITH_FULL_OPTIMIZATION:
			coplanar_calibration_with_full_optimization ();
			break;
		case NONCOPLANAR:
			noncoplanar_calibration ();
			break;
		case NONCOPLANAR_WITH_FULL_OPTIMIZATION:
			noncoplanar_calibration_with_full_optimization ();
			break;
	}
	
	// Saves the results 
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cameraLeftCalibrationParameters = cp;
			cameraLeftCalibrationConstants = cc;
			break;
		case RIGHT_CAMERA:
			cameraRightCalibrationParameters = cp;
			cameraRightCalibrationConstants = cc;
			break;
	}
	
#ifdef DEBUG_SMV_API
	{
		int i;
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\nmaeCameraCalibration\n");
		switch (nCameraSide)
		{
			case LEFT_CAMERA:
				fprintf (g_pLogFile, "\tnCameraSide = LEFT_CAMERA\n");
				break;
			case RIGHT_CAMERA:
				fprintf (g_pLogFile, "\tnCameraSide = RIGHT_CAMERA\n");
				break;
		}
		fprintf (g_pLogFile, "\tpPixelVector\n\tX\tY\n");
		for (i = 0; i < cd.point_count; i++)
			fprintf (g_pLogFile, "\t%lf\t%lf\n", cd.Xf[i], cd.Yf[i]);

		fprintf (g_pLogFile, "\tpPointsVector\n\tX\tY\tY\n");
		for (i = 0; i < cd.point_count; i++)
			fprintf (g_pLogFile, "\t%lf %lf %lf\n", cd.xw[i], cd.yw[i], cd.zw[i]);

		fclose (g_pLogFile);
	}
#endif		
    	return;
}



/************************************************************************/
/* Name: maeUpdateImageLeft						*/
/* Description:	updates the image left input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeUpdateImageLeft (int *pPixelVector, int nWidth, int nHeight)
{
	int xi, yi, wi, hi, xo, yo, ho, wo, pixel, red, green, blue;
	double Xfd, Yfd, Xfu, Yfu;
    	int i, j;
	
	// Gets the input image dimentions
	wi = nWidth;
	hi = nHeight;
	
	// Gets the output image dimentions
	wo = image_left.ww;
	ho = image_left.wh;
		
	for (j = 0; j < nHeight; j++)
	{
        	for (i = 0; i < nWidth; i++)
        	{
		      pixel = pPixelVector[i + (nHeight - j - 1) * nWidth];

		      red   = RED(pixel);
		      green = GREEN(pixel);
		      blue  = BLUE(pixel);

		      image_left.image[3 * (i + j * image_left.tfw) + 0] = red;
		      image_left.image[3 * (i + j * image_left.tfw) + 1] = green;
		      image_left.image[3 * (i + j * image_left.tfw) + 2] = blue;
        	}
	}
	
	update_input_neurons (&(image_left));

#ifdef DEBUG_SMV_API
	{
		int i, j;
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\nmaeUpdateImageLeft\n");
		fclose (g_pLogFile);
	
		g_pImageFile = fopen (IMAGE_LEFT_FILE_NAME, "w");
		fprintf (g_pImageFile, "P3\n");
		fprintf (g_pImageFile, "# Test\n");
		fprintf (g_pImageFile, "%d %d\n", nWidth, nHeight);
		fprintf (g_pImageFile, "255\n");

		for (j = nHeight - 1; j >= 0; j--)
		{
        		for (i = 0; i < nWidth; i++)
        		{
			      red   = image_left.image[3 * (i + j * image_left.tfw) + 0];
			      green = image_left.image[3 * (i + j * image_left.tfw) + 1];
			      blue  = image_left.image[3 * (i + j * image_left.tfw) + 2];
			      fprintf (g_pImageFile, "%d %d %d\n", red, green, blue);
        		}
		}
		
		fclose (g_pImageFile);
	}
#endif
		
	return;
}



/************************************************************************/
/* Name: maeUpdateImageRight						*/
/* Description:	updates the image right input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight)
{
	int xi, yi, wi, hi, xo, yo, ho, wo, pixel, red, green, blue;
	double Xfd, Yfd, Xfu, Yfu;
	int i, j;
	
	// Gets the input image dimentions
	wi = nWidth;
	hi = nHeight;
	
	// Gets the output image dimentions
	wo = image_right.ww;
	ho = image_right.wh;

	for (j = 0; j < nHeight; j++)
	{
		for (i = 0; i < nWidth; i++)
        	{
			pixel = pPixelVector[i + (nHeight - j - 1) * nWidth];
		      
			red   = RED(pixel);
			green = GREEN(pixel);
			blue  = BLUE(pixel);

			image_right.image[3 * (i + j * image_right.tfw) + 0] = red;
			image_right.image[3 * (i + j * image_right.tfw) + 1] = green;
			image_right.image[3 * (i + j * image_right.tfw) + 2] = blue;
		}
	}
    
	update_input_neurons (&(image_right));

#ifdef DEBUG_SMV_API
	{
		int i, j;
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\nmaeUpdateImageRight\n");
		fclose (g_pLogFile);

		g_pImageFile = fopen (IMAGE_RIGHT_FILE_NAME, "w");
		fprintf (g_pImageFile, "P3\n");
		fprintf (g_pImageFile, "# Test\n");
		fprintf (g_pImageFile, "%d %d\n", nWidth, nHeight);
		fprintf (g_pImageFile, "255\n");

		for (j = nHeight; j >= 0 ; j--)
		{
        		for (i = 0; i < nWidth; i++)
        		{
			      red   = image_right.image[3 * (i + j * image_right.tfw) + 0];
			      green = image_right.image[3 * (i + j * image_right.tfw) + 1];
			      blue  = image_right.image[3 * (i + j * image_right.tfw) + 2];
			      fprintf (g_pImageFile, "%d %d %d\n", red, green, blue);
        		}
		}

		fclose (g_pImageFile);
	}
#endif
	
	return;
}



/************************************************************************/
/* Name: maeMapWorldPoints						*/
/* Description:	maps the world points					*/
/* Inputs: the points coordinates and the points number			*/
/* Output: none								*/
/************************************************************************/

DLLEXPORT void maeMapWorldPoints (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber)
{
	IMAGE_COORDINATE leftPoint, rightPoint;
	KEY *element;
	int i;

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeMapWorldPoints\n");
	fclose (g_pLogFile);
#endif			

	for (i = 0; i < nPointsNumber; i++)
	{
        	element = (KEY *) malloc (sizeof (KEY));
                
		element->dominantEyePoint.x = pPointsVectorX[i];
		element->dominantEyePoint.y = pPointsVectorY[i];
		  
		// Sets the vergence 
       		element->nonDominantEyePoint = calculateVergencePoint (element->dominantEyePoint);
	    	leftPoint.x = (double) (image_left.wxd);
		leftPoint.y = (double) (image_left.wyd);
		rightPoint.x = (double) (image_right.wxd);
		rightPoint.y = (double) (image_right.wyd);

#ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\tVergence point\n");
		fprintf (g_pLogFile, "\t(%d, %d) <-> (%d, %d)\n", leftPoint.x, leftPoint.y, rightPoint.x, rightPoint.y);
		fclose (g_pLogFile);
#endif

		// Calculates the world point position
		element->world_point = calculateWorldPoint (leftPoint, rightPoint);

#ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\t\nWorld point\n");
		fprintf (g_pLogFile, "\t(%f,%f,%f)\n", element->world_point.x, element->world_point.y, element->world_point.z);
		fclose (g_pLogFile);
#endif
        	listAppend (g_pList, (void *) element);
	}
    
    	return;
}



/************************************************************************/
/* Name: maeManualMapWorldPoints           				*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

DLLEXPORT void maeManualMapWorldPoints (int *pPointsVectorRightX, int *pPointsVectorRightY,int *pPointsVectorLeftX, int *pPointsVectorLeftY, int nPointsNumber)
{
	IMAGE_COORDINATE leftPoint, rightPoint;
	KEY *element;
	int i;

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeManualMapWorldPoints\n");
	fclose (g_pLogFile);
#endif			
		
	for (i = 0; i < nPointsNumber; i++)
	{
        	element = (KEY *) malloc (sizeof (KEY));
	                
		element->dominantEyePoint.x = pPointsVectorRightX[i];
		element->dominantEyePoint.y = pPointsVectorRightY[i];
		element->nonDominantEyePoint.x = pPointsVectorLeftX[i];
		element->nonDominantEyePoint.y = pPointsVectorLeftY[i];
        	
	    	leftPoint.x = (double) (pPointsVectorLeftX[i]);
		leftPoint.y = (double) (pPointsVectorLeftY[i]);
		rightPoint.x = (double) (pPointsVectorRightX[i]);
		rightPoint.y = (double) (pPointsVectorRightY[i]);

#ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\tVergence point\n");
		fprintf (g_pLogFile, "\t(%d, %d) <-> (%d, %d)\n", leftPoint.x, leftPoint.y, rightPoint.x, rightPoint.y);
		fclose (g_pLogFile);
#endif
        
		// Calculates the world point position
		element->world_point = calculateWorldPoint (leftPoint, rightPoint);

 #ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\t\nWorld point\n");
		fprintf (g_pLogFile, "\t(%f,%f,%f)\n", element->world_point.x, element->world_point.y, element->world_point.z);
		fclose (g_pLogFile);
#endif
        
      		listAppend (g_pList, (void *) element);
	}

	return;
}



/************************************************************************/
/* Name: maeGetWorldPoints						*/
/* Description:	gets the mapped world points coordinates		*/
/* Inputs: the number of points 					*/
/* Output: the world point coordinates					*/
/************************************************************************/

DLLEXPORT double *maeGetWorldPointCoordinate (int numPoints,double *pDoubVector)
{
   	int i;
	NO *aux;
	KEY *element;

   	for (i = 0, aux = g_pList->head; i < (numPoints / 3); i++, aux = aux->next)
    	{
		element = (KEY *) aux->info;
        	pDoubVector[3 * i + 0] = (double) element->world_point.x;
        	pDoubVector[3 * i + 1] = (double) element->world_point.y;
        	pDoubVector[3 * i + 2] = (double) element->world_point.z;
		
#ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\nmaeGetWorldPointCoordinate\n");
		fprintf (g_pLogFile, "(%lf, $lf, %lf)\n", pDoubVector[3 * i + 0], pDoubVector[3 * i + 1], pDoubVector[3 * i + 2]);
		fclose (g_pLogFile);
#endif
    	}
   
	return (pDoubVector); 
}



/************************************************************************/
/* Name: maeGetImageLeftPoint						*/
/* Description:	gets the image left point coordinates			*/
/* Inputs: the axis and the point index					*/
/* Output: the image left point coordinates				*/
/************************************************************************/

DLLEXPORT int *maeGetImageLeftPoint (int numPoints, int *pIntVector)
{
	int i;
	NO *aux;
	KEY *element;
	
     	for (i = 0, aux = g_pList->head; i <(numPoints / 2); i++, aux = aux->next)
        {	
    		element = (KEY *) aux->info;
        	pIntVector[2 * i + 0] = (int) element->nonDominantEyePoint.x;
         	pIntVector[2 * i + 1] = (int) element->nonDominantEyePoint.y;
          
#ifdef DEBUG_SMV_API
		g_pLogFile = fopen (LOG_FILE_NAME, "a");
		fprintf (g_pLogFile, "\nmaeGetImageLeftPoint\n");
		fprintf (g_pLogFile, "\(%d, %d)\n",pIntVector[2 * i + 0], pIntVector[2 * i + 1]);
		fclose (g_pLogFile);
#endif	
        }
	
  	return (pIntVector);
}




/************************************************************************/
/* Name: maeCleanCurrentPoints		                                */
/* Description:	cleans the current points                               */
/* Inputs: none								*/
/* Output: none	                                                        */
/************************************************************************/

DLLEXPORT void maeCleanCurrentPoints (void)
{
	listEmpty (g_pList);

#ifdef DEBUG_SMV_API
	g_pLogFile = fopen (LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "\nmaeCleanCurrentPoints\n");
	fclose (g_pLogFile);
#endif		
	return;
}



/************************************************************************/
/* Name: maeDistorted2UndistortedImage  				*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

DLLEXPORT int *maeDistorted2UndistortedImage (int *pDistortedImage, int nWidth, int nHeight, int nCameraSide)
{
	int xi, yi, wi, hi, xo, yo, ho, wo;
	double Xfd, Yfd, Xfu, Yfu;
    
	// Gets the input image dimentions
	wi = nWidth;
	hi = nHeight;
	
	// Gets the output image dimentions
	wo = nWidth;
	ho = nHeight;
	
	// Chooses the camera parameters
     	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cp = cameraLeftCalibrationParameters;
			cc = cameraLeftCalibrationConstants;
			break;
		case RIGHT_CAMERA:
			cp = cameraRightCalibrationParameters;
			cc = cameraRightCalibrationConstants;
			break;
	}
        
	// Walks along the undistorted output image
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			// Calculates the associated position at the distorted input image
			Xfu = (double) xo;
			Yfu = (double) yo;
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			xi = (int) (Xfd + 0.5);
			yi = (int) (Yfd + 0.5);
			
			// Gets the input image pixel at the previus calculated position
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
                		g_pUndistortedImage[xo + yo * wo] =  pDistortedImage[xi + yi * wi];
			else
		                g_pUndistortedImage[xo + yo * wo] = 0;
		}		
	}
   
	return (g_pUndistortedImage);
}
// new one -> under test
/*DLLEXPORT int *maeDistorted2UndistortedImage (int *pDistortedImage, int nWidth, int nHeight, int nCameraSide)
{
	int xi, yi, wi, hi, xo, yo, ho, wo;
	double Xfd, Yfd, Xfu, Yfu;
	static GLubyte *pVirtualImage = NULL;
	static int wo_old = 0, ho_old = 0;

	// Gets the input image dimentions
	wi = nWidth;
	hi = nHeight;
	
	// Gets the output image dimentions
	wo = (SCALE_FACTOR) * nWidth;
	ho = (SCALE_FACTOR) * nHeight;
	
	// Resizes the virtual image
	if ((wo_old != wo) || (ho_old != ho))
	{
		free (pVirtualImage);
		pVirtualImage = (GLubyte *) alloc_mem (3 * wo * ho *sizeof (GLubyte));
		wo_old = wo;
		ho_old = ho;
	}
	
	// Chooses the camera parameters
     	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cp = cameraLeftCalibrationParameters;
			cc = cameraLeftCalibrationConstants;
			break;
		case RIGHT_CAMERA:
			cp = cameraRightCalibrationParameters;
			cc = cameraRightCalibrationConstants;
			break;
	}

	// Maps the distorted input image to the undistorted virtual image
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			// Calculates the associated position at the distorted input image
			Xfu = (double) xo;
			Yfu = (double) yo;
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			xi = (int) (Xfd + 0.5) / (double) (SCALE_FACTOR);
			yi = (int) (Yfd + 0.5) / (double) (SCALE_FACTOR);
			
			// Gets the input image pixel at the previus calculated position
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
                		pVirtualImage[xo + yo * wo] =  pDistortedImage[xi + yi * wi];
			else
		                pVirtualImage[xo + yo * wo] = 0;
		}		
	}
 
	// Scales the undistorted virtual image to the undistorted output image	
	gluScaleImage (GL_RGBA, wo, ho, GL_UNSIGNED_BYTE, (GLubyte *) pVirtualImage, wi, hi, GL_UNSIGNED_BYTE, (GLubyte *) g_pUndistortedImage);

	return (g_pUndistortedImage);
}*/
