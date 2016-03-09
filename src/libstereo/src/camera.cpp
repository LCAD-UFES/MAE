#include "camera.hpp"

int nearest_power_of_2 (int value)
{
	int i;
	int larger = 0;
	
	for (i = 0; i < 32; i++)
	{
		if ((value >> i) & 1)
			larger = i;
	}
	i = 1 << larger;
	if ((value % i) != 0)
		i = i << 1;

	return (i);
}

// Global variables
double CCDHorizontalSizes[] = {3.2, 4.8, 6.4, 8.8, 12.8}; // [mm]
double CCDVerticalSizes[] = {2.4, 3.6, 4.8, 6.6, 9.6};    // [mm]

// ----------------------------------------------------------------------
// TCamera::TCamera - Creates a camera object
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

TCamera::TCamera (int nXMaxResolution, int nYMaxResolution, TCCDFormat eCCDFormat)
{
	P.SetSize (3,4);
	K.SetSize (3,3);
	R.SetSize (3,3);
	t.SetSize (3,1);

	x_max_resolution = nXMaxResolution;
	y_max_resolution = nYMaxResolution;
	ccd_format = eCCDFormat;

	image_width = nearest_power_of_2 (x_max_resolution);
	image_height = nearest_power_of_2 (y_max_resolution);

	if ((p_nUndistortedImage = new int [image_width * image_height]) == NULL)
		throw TModuleError ("Cannot create the camera image.");
}



// ----------------------------------------------------------------------
// TCamera::~TCamera - Destroys a camera object
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

TCamera::~TCamera (void)
{
	delete p_nUndistortedImage;
}



// ----------------------------------------------------------------------
// TCamera::SetCameraParameters
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::SetCameraParameters (int nXMaxResolution, int nYMaxResolution, TCCDFormat eCCDFormat)
{
	x_max_resolution = nXMaxResolution;
	y_max_resolution = nYMaxResolution;
	ccd_format = eCCDFormat;

	image_width = nearest_power_of_2 (x_max_resolution);
	image_height = nearest_power_of_2 (y_max_resolution);

	if (p_nUndistortedImage != NULL)
		delete p_nUndistortedImage;

	if ((p_nUndistortedImage = new int [image_width * image_height]) == NULL)
		throw TModuleError ("Cannot resize the camera image.");
}



// ----------------------------------------------------------------------
// TCamera::CalculateProjectionMatrix - calculates the camera projection matrix
//
// Inputs: none
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TCamera::CalculateProjectionMatrix (void)
{
	// Mounts the camera calibration matrix
        K(0,0) = constants.f / parameters.dpx; 	K(0,1) = 0.0;           		K(0,2) = parameters.Cx;
        K(1,0) = 0.0;           		K(1,1) = constants.f / parameters.dpy; 	K(1,2) = parameters.Cy;
        K(2,0) = 0.0;           		K(2,1) = 0.0;           		K(2,2) = 1.0;
        //cout << "Camera calibration matrix (K)\n" << K << endl;
        
        // Mounts the camera rotation matrix
	double r1, r2, r3, r4, r5, r6, r7, r8, r9, sa, sb, sg, ca, cb, cg;
	
	SINCOS (constants.Rx, sa, ca);
	SINCOS (constants.Ry, sb, cb);
	SINCOS (constants.Rz, sg, cg);
	r1 = cb * cg;
	r2 = cg * sa * sb - ca * sg;
	r3 = sa * sg + ca * cg * sb;
	r4 = cb * sg;
	r5 = sa * sb * sg + ca * cg;
	r6 = ca * sb * sg - cg * sa;
	r7 = -sb;
	r8 = cb * sa;
	r9 = ca * cb;

	R(0,0) = r1;         R(0,1) = r2;         R(0,2) = r3;
        R(1,0) = r4;         R(1,1) = r5;         R(1,2) = r6;
        R(2,0) = r7;         R(2,1) = r8;         R(2,2) = r9;
        //cout << "Camera rotation matrix (R)\n" << R << endl;

        /*R(0,0) = constants.r1;         R(0,1) = constants.r2;         R(0,2) = constants.r3;
        R(1,0) = constants.r4;         R(1,1) = constants.r5;         R(1,2) = constants.r6;
        R(2,0) = constants.r7;         R(2,1) = constants.r8;         R(2,2) = constants.r9;
        cout << "Camera rotation matrix (R)\n" << R << endl;*/

	/*TMatrix Rx(3,3);	
	Rx(0,0) = 1.0;         Rx(0,1) = 0.0;         	Rx(0,2) = 0.0;
        Rx(1,0) = 0.0;         Rx(1,1) = cos(constants.Rx);   	Rx(1,2) = sin(constants.Rx);
        Rx(2,0) = 0.0;         Rx(2,1) = -sin(constants.Rx);  	Rx(2,2) = cos(constants.Rx);
	
	TMatrix Ry(3,3);	
	Ry(0,0) = cos(constants.Ry);  Ry(0,1) = 0.0;         	Ry(0,2) = -sin(constants.Ry);
        Ry(1,0) = 0.0;         Ry(1,1) = 1.0;   	Ry(1,2) = 0.0;
        Ry(2,0) = sin(constants.Ry);  Ry(2,1) = 0.0;  		Ry(2,2) = cos(constants.Ry);
	
        TMatrix Rz(3,3);	
	Rz(0,0) = cos(constants.Rz);  Rz(0,1) = sin(constants.Rz);    Rz(0,2) = 0.0;
        Rz(1,0) = -sin(constants.Rz); Rz(1,1) = cos(constants.Rz);   	Rz(1,2) = 0.0;
        Rz(2,0) = 0.0;	       Rz(2,1) = 0.0;  		Rz(2,2) = 1.0;

	R = Rx * Ry * Rz;
	cout << "Camera rotation matrix (R)\n" << R << endl;*/

        // Mounts the camera translation vector
        t(0,0) = constants.Tx;
        t(1,0) = constants.Ty;    
        t(2,0) = constants.Tz;
	//cout << "Camera translation vector (t)\n" << t << endl;
	
	// Calculates the camera projection matrix
	P = K * (R | t);
	//cout << "Camera projection matrix (P)\n" << P << endl;

	return (true);
}



// ----------------------------------------------------------------------
// TCamera::Calibrate - Calibrates a camera using the Tsai's method
//
// Inputs: pCalibrationData - a pointer to the camera calibration data
//	   eCalibrationType - calibration type
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TCamera::Calibrate (double *p_dblImagePoints, double *p_dblWorldPoints, int nPointsNumber, TCalibrationType eCalibrationType)
{
	// Mounts the calibration data
	for (cd.point_count = 0; cd.point_count < (int) nPointsNumber; cd.point_count++)
	{
		cd.xw[cd.point_count] = p_dblWorldPoints[3 * cd.point_count + 0]; // World point x
		cd.yw[cd.point_count] = p_dblWorldPoints[3 * cd.point_count + 1]; // World point y
		cd.zw[cd.point_count] = p_dblWorldPoints[3 * cd.point_count + 2]; // World point z
		cd.Xf[cd.point_count] = p_dblImagePoints[2 * cd.point_count + 0]; // Image point x
		cd.Yf[cd.point_count] = p_dblImagePoints[2 * cd.point_count + 1]; // Image point y
	}

	// Initializes the camera photometric parameters
	cp.Ncx = (double) x_max_resolution;				// [sel]
	cp.Nfx = (double) x_max_resolution;				// [pix]
	cp.sx = cp.Ncx / cp.Nfx;					// []
	cp.dx = CCDHorizontalSizes[ccd_format] / cp.Ncx;		// [mm/sel]
	cp.dy = CCDVerticalSizes[ccd_format] / (double) y_max_resolution;// [mm/sel]
	cp.dpx = cp.dx * cp.sx;						// [mm/pix]
	cp.dpy = cp.dy;							// [mm/pix]
	cp.Cx = 0.5 * cp.Nfx;						// [pix]
	cp.Cy = 0.5 * (double) y_max_resolution;    			// [pix]
	
	// Calibrates the camera using Tsai method
	switch (eCalibrationType)
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
		default:
			coplanar_calibration ();
	}
	
	print_error_stats (stderr);

	// Corrects the camera photometric parameters
	/*cp.Ncx = cp.Nfx * cp.sx;					// [sel]					// []
	cp.dx = CCDHorizontalSizes[ccd_format] / cp.Ncx;		// [mm/sel]
	cp.dpx = cp.dx * cp.sx;*/					// [mm/pix]

	// Saves the Tsai's method camera parameters
	parameters = cp;
	
	// Saves the Tsai's method camera constants
	constants = cc;

        return (true);
}



// ----------------------------------------------------------------------
// TCamera::Distorted2UndistortedImage - Undistortes the camera image
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------
#define SCALE_FACTOR	2
int *TCamera::Distorted2UndistortedImage (int *p_nDistortedImage, int nImageWidth, int nImageHeight)
{
	int xi, yi, wi, hi, xo, yo, ho, wo;
	double Xfd, Yfd, Xfu, Yfu;
    	static int *p_nVirtualImage = NULL;
	static int wo_old = 0, ho_old = 0;

	// Gets the input image dimentions
	wi = nImageWidth;
	hi = nImageHeight;
	
	// Gets the output image dimentions
	wo = SCALE_FACTOR * nImageWidth;
	ho = SCALE_FACTOR * nImageHeight;

	// Sets the Tsai's method camera parameters
	cp = parameters;

	// Sets the Tsai's method camera constants
	cc = constants;
        
	// Resizes the virtual image
	if ((wo_old != wo) || (ho_old != ho))
	{
		free (p_nVirtualImage);
		p_nVirtualImage = (int *) malloc (wo * ho * sizeof (int));
		wo_old = wo;
		ho_old = ho;
	}

	// Walks along the undistorted output image
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			// Calculates the associated position at the distorted input image
			Xfu = (double) (wo - xo - 1) / (double) SCALE_FACTOR;
			Yfu = (double) yo / (double) SCALE_FACTOR;
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			xi = wi - (int) (Xfd + 0.5) - 1;
			yi = (int) (Yfd + 0.5);
			
			// Gets the input image pixel at the previus calculated position
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
                		p_nVirtualImage[xo + yo * wo] = p_nDistortedImage[xi + yi * wi];
			else
		                p_nVirtualImage[xo + yo * wo] = 0;
		}		
	}

	// Scales the undistorted virtual image to the undistorted output image	
	gluScaleImage (GL_RGBA, wo, ho, GL_UNSIGNED_BYTE, (void *) p_nVirtualImage, wi, hi, GL_UNSIGNED_BYTE, (void *) p_nUndistortedImage);

	return (p_nUndistortedImage);
}



// ----------------------------------------------------------------------
// TCamera::Undistorted2DistortedImage - Distortes the camera image
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------
#define SCALE_FACTOR	2
int *TCamera::Undistorted2DistortedImage (int *p_nDistortedImage, int nImageWidth, int nImageHeight)
{
	int xi, yi, wi, hi, xo, yo, ho, wo;
	double Xfd, Yfd, Xfu, Yfu;
    	static int *p_nVirtualImage = NULL;
	static int wo_old = 0, ho_old = 0;

	// Gets the input image dimentions
	wi = nImageWidth;
	hi = nImageHeight;
	
	// Gets the output image dimentions
	wo = SCALE_FACTOR * nImageWidth;
	ho = SCALE_FACTOR * nImageHeight;

	// Sets the Tsai's method camera parameters
	cp = parameters;

	// Sets the Tsai's method camera constants
	cc = constants;
        
	// Resizes the virtual image
	if ((wo_old != wo) || (ho_old != ho))
	{
		free (p_nVirtualImage);
		p_nVirtualImage = (int *) malloc (wo * ho * sizeof (int));
		wo_old = wo;
		ho_old = ho;
	}

	// Walks along the undistorted output image
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			// Calculates the associated position at the distorted input image
			Xfu = (double) (wo - xo - 1) / (double) SCALE_FACTOR;
			Yfu = (double) yo / (double) SCALE_FACTOR;
			distorted_to_undistorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			xi = wi - (int) (Xfd + 0.5) - 1;
			yi = (int) (Yfd + 0.5);
			
			// Gets the input image pixel at the previus calculated position
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
                		p_nVirtualImage[xo + yo * wo] = p_nDistortedImage[xi + yi * wi];
			else
		                p_nVirtualImage[xo + yo * wo] = 0;
		}		
	}

	// Scales the undistorted virtual image to the undistorted output image	
	gluScaleImage (GL_RGBA, wo, ho, GL_UNSIGNED_BYTE, (void *) p_nVirtualImage, wi, hi, GL_UNSIGNED_BYTE, (void *) p_nUndistortedImage);

	return (p_nUndistortedImage);
}

// ----------------------------------------------------------------------
// TCamera::LoadCalibrationData - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::LoadCalibrationData (char *strDataFileName)
{
	FILE *file = NULL;

	if ((file = fopen (strDataFileName, "r")) == NULL)
	{
		fprintf (stderr, "LoadCalibrationData: cannot open file '%s'.\n", strDataFileName);
		return (false);
	}

	cd.point_count = 0;
	while (fscanf (file, "%lf %lf %lf %lf %lf",
		   &(cd.xw[cd.point_count]),
		   &(cd.yw[cd.point_count]),
		   &(cd.zw[cd.point_count]),
		   &(cd.Xf[cd.point_count]),
		   &(cd.Yf[cd.point_count])) != EOF)
	{
		if (cd.point_count++ >= MAX_POINTS)
		{
			fprintf (stderr, "LoadCalibrationData: too many points, compiled in limit is %d\n", MAX_POINTS);
			return (false);
		}
	}

	fclose (file);

	return (true);
}


// ----------------------------------------------------------------------
// TCamera::SetConstants - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::SetConstants (double *p_dblNewCameraConstants)
{
	parameters.Ncx = p_dblNewCameraConstants[0];
	parameters.Nfx = p_dblNewCameraConstants[1];
	parameters.dx  = p_dblNewCameraConstants[2];
	parameters.dy  = p_dblNewCameraConstants[3];
	parameters.dpx = p_dblNewCameraConstants[4];
	parameters.dpy = p_dblNewCameraConstants[5];
	parameters.Cx  = p_dblNewCameraConstants[6];
	parameters.Cy  = p_dblNewCameraConstants[7];
	parameters.sx  = p_dblNewCameraConstants[8];

	constants.f  = p_dblNewCameraConstants[9];
	constants.kappa1 = p_dblNewCameraConstants[10];
	constants.Tx = p_dblNewCameraConstants[11];
	constants.Ty = p_dblNewCameraConstants[12];
	constants.Tz = p_dblNewCameraConstants[13];
	constants.Rx = p_dblNewCameraConstants[14];
	constants.Ry = p_dblNewCameraConstants[15];
	constants.Rz = p_dblNewCameraConstants[16];
	constants.p1 = p_dblNewCameraConstants[17];
	constants.p2 = p_dblNewCameraConstants[18];

	return (true);
}



// ----------------------------------------------------------------------
// TCamera::GetConstants - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::GetConstants (double *p_dblCurrentCameraConstants)
{
	p_dblCurrentCameraConstants[0] = parameters.Ncx;
	p_dblCurrentCameraConstants[1] = parameters.Nfx;
	p_dblCurrentCameraConstants[2] = parameters.dx;
	p_dblCurrentCameraConstants[3] = parameters.dy;
	p_dblCurrentCameraConstants[4] = parameters.dpx;
	p_dblCurrentCameraConstants[5] = parameters.dpy;
	p_dblCurrentCameraConstants[6] = parameters.Cx;
	p_dblCurrentCameraConstants[7] = parameters.Cy;
	p_dblCurrentCameraConstants[8] = parameters.sx;

	p_dblCurrentCameraConstants[9]  = constants.f;
	p_dblCurrentCameraConstants[10] = constants.kappa1;
	p_dblCurrentCameraConstants[11] = constants.Tx;
	p_dblCurrentCameraConstants[12] = constants.Ty;
	p_dblCurrentCameraConstants[13] = constants.Tz;
	p_dblCurrentCameraConstants[14] = constants.Rx;
	p_dblCurrentCameraConstants[15] = constants.Ry;
	p_dblCurrentCameraConstants[16] = constants.Rz;
	p_dblCurrentCameraConstants[17] = constants.p1;
	p_dblCurrentCameraConstants[18] = constants.p2;

	return (true);
}



// ----------------------------------------------------------------------
// TCamera::GetOrientation - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::GetOrientation (double *p_dblCurrentCameraOrientation)
{
	p_dblCurrentCameraOrientation[0] = constants.Tx;
	p_dblCurrentCameraOrientation[1] = constants.Ty;
	p_dblCurrentCameraOrientation[2] = constants.Tz;
	p_dblCurrentCameraOrientation[3] = constants.Rx;
	p_dblCurrentCameraOrientation[4] = constants.Ry;
	p_dblCurrentCameraOrientation[5] = constants.Rz;

	return (true);
}




// ----------------------------------------------------------------------
// TCamera::SetOrientation - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::SetOrientation (double *p_dblNewCameraOrientation)
{
	constants.Tx = p_dblNewCameraOrientation[0];
	constants.Ty = p_dblNewCameraOrientation[1];
	constants.Tz = p_dblNewCameraOrientation[2];
	constants.Rx = p_dblNewCameraOrientation[3];
	constants.Ry = p_dblNewCameraOrientation[4];
	constants.Rz = p_dblNewCameraOrientation[5];

	return (true);
}

// ----------------------------------------------------------------------
// TCamera::LoadConstants - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::LoadConstants (char *strFileName)
{
	FILE *data_fd = NULL;

	if ((data_fd = fopen (strFileName, "r")) == NULL)
		throw TModuleError ("Cannot open camera file.");

	load_cp_cc_data (data_fd, &parameters, &constants);
		
	if (fclose (data_fd))
		throw TModuleError ("Cannot close camera file.");

	this->ShowConstants();

	return (true);
}



// ----------------------------------------------------------------------
// TCamera::SaveConstants - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

bool TCamera::SaveConstants (char *strFileName)
{
	FILE *data_fd = NULL;

	if ((data_fd = fopen (strFileName, "w")) == NULL)
		throw TModuleError ("Cannot open camera file.");

	dump_cp_cc_data (data_fd, &parameters, &constants);
		
	if (fclose (data_fd))
		throw TModuleError ("Cannot close camera file.");

	return (true);
}



// ----------------------------------------------------------------------
// TCamera::ShowConstants - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

void TCamera::ShowConstants (void)
{
	// Prints the calibration report
    	print_cp_cc_data (stderr, &parameters, &constants);
}



// ----------------------------------------------------------------------
// TCamera::GetWorldPointAtDistance - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

TMatrix TCamera::GetWorldPointAtDistance (TMatrix x, double dblDistance)
{
	TMatrix X(2,1);
	TMatrix A(3,2);
	TMatrix b(3,1);
	
	A = (P.Col(0) | P.Col(1));

	b = (x - P.Col(2) - P.Col(3) / dblDistance);

	X = A.PseudoInv() * b;
	
	X.SetSize(4,1);
	X(2,0) = 1.0;
	X(3,0) = 1.0 / dblDistance;
	//cout << "World point at " << dblDistance << " [mm] distance (X):\n " << X << endl;

	return (X);
}




// ----------------------------------------------------------------------
// TCamera::ProjectWorldPoint - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

TMatrix TCamera::ProjectWorldPoint (TMatrix X)
{
	TMatrix x(3,1);
	
	x = P * X;
	//cout << "Image point (x):\n " << x << endl;

	return (x);
}



// ----------------------------------------------------------------------
// TCamera::Undistorted2DistortedImageCoordinate - 
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

void TCamera::Undistorted2DistortedImageCoordinate (double *p_dblUndistortedImagePoint, double *p_dblDistortedImagePoint)
{
	// Sets the Tsai's method camera parameters
	cp = parameters;

	// Sets the Tsai's method camera constants
	cc = constants;

	undistorted_to_distorted_image_coord (p_dblUndistortedImagePoint[0], p_dblUndistortedImagePoint[1], &(p_dblDistortedImagePoint[0]), &(p_dblDistortedImagePoint[1]));
}
