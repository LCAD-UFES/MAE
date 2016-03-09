#include "stereo_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libstereo.h"


/* Global Variables */
mxArray *g_KK_left, *g_KK_right, *g_R, *g_T, *g_om, *g_fc_left, *g_cc_left, *g_kc_left, *g_alpha_c_left, *g_fc_right, *g_cc_right, *g_kc_right, *g_alpha_c_right;
mxArray *g_rect_img_left = NULL, *g_rect_img_right = NULL, *g_img_left, *g_img_right, *g_img, *g_edge_img;
mxArray *g_ind_new_left, *g_ind_1_left, *g_ind_2_left, *g_ind_3_left, *g_ind_4_left, *g_a1_left, *g_a2_left, *g_a3_left, *g_a4_left, *g_ind_new_right, *g_ind_1_right, *g_ind_2_right, *g_ind_3_right, *g_ind_4_right, *g_a1_right, *g_a2_right, *g_a3_right, *g_a4_right;


double 	g_c_KK_left[3*3],
	g_c_KK_right[3*3],
	g_c_R[3*3],
	g_c_T[3*1],
	g_c_om[3*1],
        g_c_fc_left[2*1],
	g_c_cc_left[2*1],
	g_c_kc_left[5*1],
	g_c_alpha_c_left[2*1],
	g_c_fc_right[2*1],
	g_c_cc_right[2*1],
	g_c_kc_right[5*1],
	g_c_alpha_c_right[2*1];

/****************************************************************/
/* StereoInitialize - 						*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/
 
DLLEXPORT int StereoInitialize (int image_width, int image_height)
{
	int dims[3];
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
	
	g_KK_left = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_KK_right = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_R = mxCreateDoubleMatrix (3, 3, mxREAL);
	g_T = mxCreateDoubleMatrix (3, 1, mxREAL);
	g_om = mxCreateDoubleMatrix (3, 1, mxREAL);
        g_fc_left = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_cc_left = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_kc_left = mxCreateDoubleMatrix (5, 1, mxREAL);
	g_alpha_c_left = mxCreateDoubleMatrix (1, 1, mxREAL);
	g_fc_right = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_cc_right = mxCreateDoubleMatrix (2, 1, mxREAL);
	g_kc_right = mxCreateDoubleMatrix (5, 1, mxREAL);
	g_alpha_c_right = mxCreateDoubleMatrix (1, 1, mxREAL);

	dims[0] = image_width;
	dims[1] = image_height;
	dims[2] = 3;
	g_rect_img_left = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);
	g_rect_img_right  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);	
	g_img_left = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);
	g_img_right  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);

	g_ind_new_left = mxCreateDoubleMatrix (image_width*image_height, 1, mxREAL);
	g_ind_1_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_2_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_3_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_4_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a1_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a2_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a3_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a4_left = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_new_right = mxCreateDoubleMatrix (image_width*image_height, 1, mxREAL);
	g_ind_1_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_2_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_3_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_ind_4_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a1_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a2_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a3_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	g_a4_right = mxCreateDoubleMatrix (1, image_width*image_height, mxREAL);
	
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

DLLEXPORT int LoadStereoParameters (const char *strFileName)
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
	
	memcpy(g_c_KK_left, mxGetPr(g_KK_left), 3 * 3 * sizeof (double));
	memcpy(g_c_KK_right, mxGetPr(g_KK_right), 3 * 3 * sizeof (double));
	memcpy(g_c_R, mxGetPr(g_R), 3 * 3 * sizeof (double));
	memcpy(g_c_T, mxGetPr(g_T), 3 * 1 * sizeof (double));
	memcpy(g_c_om, mxGetPr(g_om), 3 * 1 * sizeof (double));
	memcpy(g_c_fc_left, mxGetPr(g_fc_left), 2 * 1 * sizeof (double));
	memcpy(g_c_cc_left, mxGetPr(g_cc_left), 2 * 1 * sizeof (double));
	memcpy(g_c_kc_left, mxGetPr(g_kc_left), 5 * 1 * sizeof (double));
	memcpy(g_c_alpha_c_left, mxGetPr(g_alpha_c_left), 1 * 1 * sizeof (double));
	memcpy(g_c_fc_right, mxGetPr(g_fc_right), 2 * 1 * sizeof (double));
	memcpy(g_c_cc_right, mxGetPr(g_cc_right), 2 * 1 * sizeof (double));
	memcpy(g_c_kc_right, mxGetPr(g_kc_right), 5 * 1 * sizeof (double));
	memcpy(g_c_alpha_c_right, mxGetPr(g_alpha_c_right), 1 * 1 * sizeof (double));
	
	mxDestroyArray(FileName); FileName = 0;

	return 0;
}



/****************************************************************/
/* LoadStereoRectificationIndexes -				*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int LoadStereoRectificationIndexes (const char *strFileName)
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

DLLEXPORT int StereoTriangulation (int num_points, double *XL, double *XR, double *xL, double *xR)
{
	int nargout = 2;
	mxArray *ml_XL, *ml_XR, *ml_xL, *ml_xR;
	
	ml_XL = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_XR = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_xL = mxCreateDoubleMatrix (2, num_points, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, num_points, mxREAL);

	memcpy(mxGetPr(ml_xL), xL, num_points * 2 * sizeof (double));
	memcpy(mxGetPr(ml_xR), xR, num_points * 2 * sizeof (double));

	mlfStereoTriangulation(nargout, &ml_XL, &ml_XR, ml_xL, ml_xR, 
			                g_om, g_T, 
					g_fc_left, g_cc_left, g_kc_left, g_alpha_c_left, 
					g_fc_right, g_cc_right, g_kc_right, g_alpha_c_right, 
					g_R);
    
	memcpy(XL, mxGetPr(ml_XL), num_points * 3 * sizeof (double));
	memcpy(XR, mxGetPr(ml_XR), num_points * 3 * sizeof (double));
	
	mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_XR); 
        mxDestroyArray(ml_xL); 
	mxDestroyArray(ml_xR); 
	
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

DLLEXPORT int ApplyEdgeFilter (unsigned char *edge_img, unsigned char *img, int w, int h)
{
	int nargout = 1;
	int dims[3];
        int ndim = 3;
	
	DEBUG("ApplyEdgeFilter:\n","","");
	dims[0] = h;
	dims[1] = w;
	dims[2] = 3;
	g_edge_img  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);
	g_img  = mxCreateNumericArray (ndim, dims, mxUINT8_CLASS, mxREAL);

	DEBUG("\t->Copying data...\n","","");
	memcpy(mxGetData(g_img), img, 3 * w * h * sizeof (unsigned char));
    
	DEBUG("\t->mlfApplyEdgeFilter\n","","");
	mlfApplyEdgeFilter(nargout, &g_edge_img, g_img);

	DEBUG("\t->Copying data...\n","","");
	memcpy(edge_img, mxGetData(g_edge_img), 3 * w * h * sizeof (unsigned char));
	
	mxDestroyArray(g_img); g_img = 0;
	mxDestroyArray(g_edge_img); g_edge_img = 0;
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
	mxArray *ml_XR, *ml_XL, *ml_xR, *ml_D;
	
	ml_XR = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_XL = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, 1, mxREAL);
	ml_D = mxCreateDoubleMatrix (1, 1, mxREAL);

	memcpy(mxGetPr(ml_xR), p_dblImagePoint, 2 * sizeof (double));
	memcpy(mxGetPr(ml_D), &dblDistance, sizeof (double));
	
        mlfGetWorldPointAtDistance (nargout, &ml_XL, &ml_XR, ml_xR, g_KK_left, g_KK_right, g_R, g_T, ml_D);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XL), 3 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XR), 3 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (GetWorldPointAtDistance)", "", "");
                        return -1;
        }
        
	mxDestroyArray(ml_XR); 
        mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_xR); 
	mxDestroyArray(ml_D); 
        
        return 0;
}


/****************************************************************/
/* GetWorldPointAtDistance -					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int GetWorldPointAtDistanceRight (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide)
{
        int nargout = 2;
        char strInfo[128];
	mxArray *ml_XR, *ml_XL, *ml_xR, *ml_D;
	
	ml_XR = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_XL = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, 1, mxREAL);
	ml_D = mxCreateDoubleMatrix (1, 1, mxREAL);

	memcpy(mxGetPr(ml_xR), p_dblImagePoint, 2 * sizeof (double));
	memcpy(mxGetPr(ml_D), &dblDistance, sizeof (double));
	
        mlfGetWorldPointAtDistanceRight (nargout, &ml_XL, &ml_XR, ml_xR, g_KK_left, g_KK_right, g_R, g_T, ml_D);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XL), 3 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XR), 3 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (GetWorldPointAtDistance)", "", "");
                        return -1;
        }
        
	mxDestroyArray(ml_XR); 
        mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_xR); 
	mxDestroyArray(ml_D); 
        
        return 0;
}


/****************************************************************/
/* CameraProjection -    					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int CameraProjection (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide)
{
        int nargout = 2;
	mxArray *ml_XL, *ml_xL, *ml_xR;
	
	ml_XL = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_xL = mxCreateDoubleMatrix (2, num_points, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, num_points, mxREAL);

	memcpy(mxGetPr(ml_XL), p_dblWorldPoint, num_points * 3 * sizeof (double));
	
        mlfCameraProjection (nargout, &ml_xL, &ml_xR, ml_XL, g_KK_left, g_KK_right, g_R, g_T);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xL), num_points * 2 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xR), num_points * 2 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (CameraProjection)", "", "");
                        return -1;
        }        
         
	mxDestroyArray(ml_XL); 
        mxDestroyArray(ml_xL); 
	mxDestroyArray(ml_xR); 
        
        return 0;
}


/****************************************************************/
/* CameraProjection -    					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int CameraProjectionRight (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide)
{
        int nargout = 2;
	mxArray *ml_XR, *ml_xL, *ml_xR;
	
	ml_XR = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_xL = mxCreateDoubleMatrix (2, num_points, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, num_points, mxREAL);

	memcpy(mxGetPr(ml_XR), p_dblWorldPoint, num_points * 3 * sizeof (double));
	
        mlfCameraProjectionRight (nargout, &ml_xL, &ml_xR, ml_XR, g_KK_left, g_KK_right, g_R, g_T);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xL), num_points * 2 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xR), num_points * 2 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (CameraProjection)", "", "");
                        return -1;
        }        
         
	mxDestroyArray(ml_XR); 
        mxDestroyArray(ml_xL); 
	mxDestroyArray(ml_xR); 
        
        return 0;
}





/****************************************************************/
/* TransformPoints -    					*/
/* Inputs:							*/
/* Output: 							*/
/****************************************************************/

DLLEXPORT int TransformPoints (int num_points, double *output_points, double *input_points, double *euler_angles, double *displacement, double *mass_center)
{
        int nargout = 1;
	mxArray *ml_POUT, *ml_PIN, *ml_E, *ml_D, *ml_MC;
	
	ml_POUT = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_PIN  = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_E    = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_D    = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_MC    = mxCreateDoubleMatrix (3, 1, mxREAL);

	memcpy(mxGetPr(ml_PIN), input_points, num_points * 3 * sizeof (double));
	memcpy(mxGetPr(ml_E), euler_angles, 3 * sizeof (double));
	memcpy(mxGetPr(ml_D), displacement, 3 * sizeof (double));
	memcpy(mxGetPr(ml_MC), mass_center, 3 * sizeof (double));
	
        mlfTransformPoints (nargout, &ml_POUT, ml_PIN, ml_E, ml_D, ml_MC);
                        
	memcpy (output_points, mxGetPr(ml_POUT), num_points * 3 * sizeof (double));
     	
	mxDestroyArray(ml_POUT); 
	mxDestroyArray(ml_PIN); 
        mxDestroyArray(ml_E); 
	mxDestroyArray(ml_D); 
	mxDestroyArray(ml_MC); 
        
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
