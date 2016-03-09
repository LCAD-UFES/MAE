#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stereo_api.h"
#include "libstereo.h"



/* Global Variables */
mxArray *g_XL = NULL, *g_XR = NULL, *g_xL, *g_xR, *g_D;
mxArray *g_KK_left, *g_KK_right, *g_R, *g_T, *g_om, *g_fc_left, *g_cc_left, *g_kc_left, *g_alpha_c_left, *g_fc_right, *g_cc_right, *g_kc_right, *g_alpha_c_right;
mxArray *g_rect_img_left = NULL, *g_rect_img_right = NULL, *g_img_left, *g_img_right;
mxArray *g_ind_new_left, *g_ind_1_left, *g_ind_2_left, *g_ind_3_left, *g_ind_4_left, *g_a1_left, *g_a2_left, *g_a3_left, *g_a4_left, *g_ind_new_right, *g_ind_1_right, *g_ind_2_right, *g_ind_3_right, *g_ind_4_right, *g_a1_right, *g_a2_right, *g_a3_right, *g_a4_right;

/****************************************************************/
/* StereoInitialize - 						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/
 
DLLEXPORT int StereoInitialize (void)
{
	const int dims[3] = {480, 640, 3};
        int ndim = 3;
	
	DEBUG("StereoInitialize:\n","","");
	
	/* Call library initialization routine and make sure that
         the library was initialized properly */
	DEBUG("\t->mclInitializeApplication\n","","");
	if (!mclInitializeApplication (NULL, 0))
	{
		ERROR("Error: could not initialize the application properly mclInitializeApplication.\n","","");
		return -1;
	}

	DEBUG("\t->stereoInitialize\n","","");
	if (!libstereoInitialize ())
	{
		ERROR("Error: could not initialize the libstereo library properly libstereoInitialize.\n","","");
		return -1;
	}
	
	DEBUG("\t->Creating global variables...\n","","");
	
	g_XL = mxCreateDoubleMatrix (3, 1, mxREAL);
	g_XR = mxCreateDoubleMatrix (3, 1, mxREAL);
	g_xL = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_xR = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_D = mxCreateDoubleMatrix (1, 1, mxREAL);
	
	g_KK_left = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_KK_right = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_R = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_T = mxCreateDoubleMatrix (3, 1, mxREAL);
	g_om = mxCreateDoubleMatrix (3, 1, mxREAL);
        g_fc_left = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_cc_left = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_kc_left = mxCreateDoubleMatrix (5, 1, mxREAL);
	g_alpha_c_left = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_fc_right = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_cc_right = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_kc_right = mxCreateDoubleMatrix (5, 1, mxREAL);
	g_alpha_c_right = mxCreateDoubleMatrix (2, 1, mxREAL);

	g_rect_img_left = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);
	g_rect_img_right  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);	
	g_img_left = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);
	g_img_right  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);

	g_ind_new_left = mxCreateDoubleMatrix (480*640, 1, mxREAL);
	g_ind_1_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_2_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_3_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_4_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a1_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a2_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a3_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a4_left = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_new_right = mxCreateDoubleMatrix (480*640, 1, mxREAL);
	g_ind_1_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_2_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_3_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_ind_4_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a1_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a2_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a3_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	g_a4_right = mxCreateDoubleMatrix (1, 480*640, mxREAL);
	
	return 0;
}



/****************************************************************/
/* StereoTerminate - 						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int StereoTerminate (void)
{
	DEBUG("StereoTerminate:\n","","");

	/* Call the library termination routine */
	DEBUG("\t->libstereoTerminate\n","","");
	libstereoTerminate ();

	/* Free the memory created */
	DEBUG("\t->Destroying global variables...\n","","");
	mxDestroyArray(g_XL); g_XL = 0;
	mxDestroyArray(g_XR); g_XR = 0;
        mxDestroyArray(g_xL); g_xL = 0;
	mxDestroyArray(g_xR); g_xR = 0;
	mxDestroyArray(g_D); g_D = 0;
	mxDestroyArray(g_KK_left); g_KK_left = 0;
	mxDestroyArray(g_KK_right); g_KK_right = 0;
	mxDestroyArray(g_R); g_R = 0;
	mxDestroyArray(g_T); g_T = 0;
	mxDestroyArray(g_om); g_om = 0;
        mxDestroyArray(g_fc_left); g_fc_left = 0;
	mxDestroyArray(g_cc_left); g_cc_left = 0;
	mxDestroyArray(g_kc_left); g_kc_left = 0;
	mxDestroyArray(g_alpha_c_left); g_alpha_c_left = 0;
	mxDestroyArray(g_fc_right); g_fc_right = 0;
	mxDestroyArray(g_cc_right); g_cc_right = 0;
	mxDestroyArray(g_kc_right); g_kc_right = 0;
	mxDestroyArray(g_alpha_c_right); g_alpha_c_right = 0;
	
	DEBUG("\t->mclTerminateApplication\n","","");
	mclTerminateApplication ();

	return 0;
}



/****************************************************************/
/* LoadStereoParameters -					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int LoadStereoParameters (char *strFileName)
{
	int nargout = 13;
	mxArray* FileName = NULL;
	
	DEBUG("LoadStereoParameters\n","","");
		
	FileName = mxCreateString (strFileName);

	DEBUG("\t->mlfLoadStereoParameters\n","","");
	mlfLoadStereoParameters (nargout, &g_KK_left, &g_KK_right
                                        , &g_R, &g_T, &g_om
                                        , &g_fc_left, &g_cc_left
                                        , &g_kc_left, &g_alpha_c_left
                                        , &g_fc_right, &g_cc_right
                                        , &g_kc_right, &g_alpha_c_right, FileName);
	
	mxDestroyArray(FileName); FileName = 0;

	return 0;
}



/****************************************************************/
/* LoadStereoRectificationIndexes -				*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int LoadStereoRectificationIndexes (char *strFileName)
{
	int nargout = 18;
	mxArray* FileName = NULL;
	
	DEBUG("LoadStereoRectificationIndexes\n","","");
		
	FileName = mxCreateString (strFileName);

	DEBUG("\t->mlfLoadRectificationIndexes\n","","");
	mlfLoadRectificationIndexes(nargout, &g_ind_new_left
                                        , &g_ind_1_left, &g_ind_2_left
                                        , &g_ind_3_left, &g_ind_4_left
                                        , &g_a1_left, &g_a2_left
                                        , &g_a3_left, &g_a4_left
                                        , &g_ind_new_right
                                        , &g_ind_1_right, &g_ind_2_right
                                        , &g_ind_3_right, &g_ind_4_right
                                        , &g_a1_right, &g_a2_right
                                        , &g_a3_right, &g_a4_right, FileName);
	
	mxDestroyArray(FileName); FileName = 0;

	return 0;
}



/****************************************************************/
/* StereoTriangulation -					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int StereoTriangulation (double *XL, double *XR, double *xL, double *xR)
{
	int nargout = 2;
	char strInfo[128];
	
	DEBUG("StereoTriangulation:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetPr(g_xL), xL, 2 * sizeof (double));
	memcpy(mxGetPr(g_xR), xR, 2 * sizeof (double));

	DEBUG("\t->mlfStereoTriangulation\n","","");
	mlfStereoTriangulation(nargout, &g_XL, &g_XR, g_xL, g_xR, g_om, g_T, g_fc_left, g_cc_left, g_kc_left, g_alpha_c_left, g_fc_right, g_cc_right, g_kc_right, g_alpha_c_right);
    
	DEBUG("\t->Copying data...\n","","");
	memcpy(XL, mxGetPr(g_XL), 3 * sizeof (double));
	memcpy(XR, mxGetPr(g_XR), 3 * sizeof (double));
	
	sprintf (strInfo, "x' = (%.2f,%.2f), x = (%.2f,%.2f) -> X = (%.2f,%.2f,%.2f)\n", xL[0],xL[1],xR[0],xR[1],XR[0],XR[1],XR[2]);
        DEBUG(strInfo,"","");
	
	return 0;
}



/****************************************************************/
/* RectifyStereoPair -						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int RectifyStereoPair (unsigned char *rect_img_left, unsigned char *rect_img_right, unsigned char *img_left, unsigned char *img_right, int w, int h)
{
	int nargout = 2;
	
	DEBUG("RectifyStereoPair:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetData(g_img_left), img_left, 3 * w * h * sizeof (unsigned char));
	memcpy(mxGetData(g_img_right), img_right, 3 * w * h * sizeof (unsigned char));

	DEBUG("\t->mlfRectifyStereoPair\n","","");
	mlfRectifyStereoPair(nargout, &g_rect_img_left, &g_rect_img_right, g_img_left, g_img_right,
				       g_ind_new_left, g_ind_1_left, g_ind_2_left, g_ind_3_left, g_ind_4_left, 
				       g_a1_left, g_a2_left, g_a3_left, g_a4_left, 
				       g_ind_new_right, g_ind_1_right, g_ind_2_right, g_ind_3_right, g_ind_4_right,
				       g_a1_right, g_a2_right, g_a3_right, g_a4_right);

	DEBUG("\t->Copying data...\n","","");
	memcpy(rect_img_left, mxGetData(g_rect_img_left), 3 * w * h * sizeof (unsigned char));
	memcpy(rect_img_right, mxGetData(g_rect_img_right), 3 * w * h * sizeof (unsigned char));
	
	return 0;
}


/****************************************************************/
/* RectifyLeftImage -						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int RectifyLeftImage (unsigned char *rect_img_left, unsigned char *img_left, int w, int h)
{
	int nargout = 1;
	
	DEBUG("RectifyLeftImage:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetData(g_img_left), img_left, 3 * w * h * sizeof (unsigned char));

	DEBUG("\t->mlfRectifyLeftImage\n","","");
	mlfRectifyLeftImage(nargout, &g_rect_img_left, g_img_left,
				       g_ind_new_left, g_ind_1_left, g_ind_2_left, g_ind_3_left, g_ind_4_left, 
				       g_a1_left, g_a2_left, g_a3_left, g_a4_left);

	DEBUG("\t->Copying data...\n","","");
	memcpy(rect_img_left, mxGetData(g_rect_img_left), 3 * w * h * sizeof (unsigned char));
	
	return 0;
}



/****************************************************************/
/* RectifyRightImage -						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int RectifyRightImage (unsigned char *rect_img_right, unsigned char *img_right, int w, int h)
{
	int nargout = 1;
	
	DEBUG("RectifyRightImage:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetData(g_img_right), img_right, 3 * w * h * sizeof (unsigned char));
    
	DEBUG("\t->mlfRectifyRightImage\n","","");
	mlfRectifyRightImage(nargout, &g_rect_img_right, g_img_right,
				       g_ind_new_right, g_ind_1_right, g_ind_2_right, g_ind_3_right, g_ind_4_right,
				       g_a1_right, g_a2_right, g_a3_right, g_a4_right);

	DEBUG("\t->Copying data...\n","","");
	memcpy(rect_img_right, mxGetData(g_rect_img_right), 3 * w * h * sizeof (unsigned char));
	
	return 0;
}


/****************************************************************/
/* GetWorldPointAtDistance -					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide)
{
        int nargout = 2;
        char strInfo[128];

        DEBUG("GetWorldPointAtDistance:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetPr(g_xR), p_dblImagePoint, 2 * sizeof (double));
	memcpy(mxGetPr(g_D), &dblDistance, sizeof (double));
	
        mlfGetWorldPointAtDistance (nargout, &g_XL, &g_XR, g_xR, g_KK_left, g_KK_right, g_R, g_T, g_D);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(g_XL), 3 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(g_XR), 3 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (GetWorldPointAtDistance)", "", "");
                        return -1;
        }
        
        sprintf (strInfo, "x = (%.2f,%.2f) -> X = (%.2f,%.2f,%.2f)\n", p_dblImagePoint[0],p_dblImagePoint[1],p_dblWorldPoint[0],p_dblWorldPoint[1],p_dblWorldPoint[2]);
        DEBUG(strInfo,"","");
        
        return 0;
}


/****************************************************************/
/* CameraProjection -    					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int CameraProjection (double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide)
{
        int nargout = 2;
        char strInfo[128];
        
        DEBUG("CameraProjection:\n","","");

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetPr(g_XR), p_dblWorldPoint, 3 * sizeof (double));
	
        mlfCameraProjection (nargout, &g_xL, &g_xR, g_XR, g_KK_left, g_KK_right, g_R, g_T);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(g_xL), 2 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(g_xR), 2 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (CameraProjection)", "", "");
                        return -1;
        }        
         
        sprintf (strInfo, "X = (%.2f,%.2f,%.2f) -> x = (%.2f,%.2f)\n",p_dblWorldPoint[0],p_dblWorldPoint[1],p_dblWorldPoint[2], p_dblImagePoint[0],p_dblImagePoint[1]);
        DEBUG(strInfo,"","");
        
        return 0;
}



/****************************************************************/
/* main - test function						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

/*int main (int argc, char **argv)
{
	double XL[3], XR[3];
	double xL[2] = {234.0, 100.0};
	double xR[2] = {250.0, 100.0};
	
	StereoInitialize ();
	
	LoadStereoParameters ("Calib_Results_stereo_rectified.mat");
	LoadStereoRectificationIndexes ("Calib_Results_rectification_indexes.mat");
	StereoTriangulation (XL, XR, xL, xR);
	printf ("X = (%f,%f,%f)\n", XR[0],XR[1],XR[2]);
		 
	StereoTerminate ();
		
	return 0;
}*/
