#include <opencv/cv.h>
#include <math.h>
#include "visual_search_filters.h"
#include "visual_search_utils.h"

extern int g_flip_horizontaly;
extern int g_flip_verticaly;

/* sigle Neural Layer translation function, obtained in face_recog_planar application */

/*
*********************************************************************************
* Function: translate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void 
translate_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float xi_target_center, yi_target_center;
	float scale_factor;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <xi_target_center> <yi_target_center>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters - The Pointers And The Values - Void pointers must be casted
	xi_target_center = *((float *) (filter_desc->filter_params->next->param.pval));
	yi_target_center = *((float *) (filter_desc->filter_params->next->next->param.pval));

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	scale_factor = 1.0;

	// Parallel translation filter capabilities where OpenMP available
#ifdef	_OPENMP
	#pragma omp parallel for private(yo,yi,xo,xi)
#endif	
	for (yo = 0; yo < ho; yo++)
	{			
		if (g_flip_verticaly)
			yi = (int) (scale_factor * ((float) -yo + (float) ho / 2.0) + yi_target_center + .5f);
		else
			yi = (int) (scale_factor * ((float) yo - (float) ho / 2.0) + yi_target_center + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
			if (g_flip_horizontaly)
				xi = (int) (scale_factor * ((float) -xo + (float) wo / 2.0) + xi_target_center + .5f);
			else
				xi = (int) (scale_factor * ((float) xo - (float) wo / 2.0) + xi_target_center + .5f);
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				#ifdef	CUDA_COMPILED
					nl_output->host_neuron_vector[xo + yo * wo].output = nl_input->host_neuron_vector[xi + yi * wi].output;
				#else
					nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
				#endif
			else
				#ifdef	CUDA_COMPILED
					nl_output->host_neuron_vector[xo + yo * wo].output.ival = 0;
				#else
					nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
				#endif
		}
	}
}


/*
*********************************************************************************
* Function: translate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void 
color_filter(FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int x, y, w, h;
	int r, g, b, intensity;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	w = nl_output->dimentions.x;
	h = nl_output->dimentions.y;
	
	for (y = 0; y < h; y++)
	{			
		y = y;
		
		for (x = 0; x < w; x++)
		{
			x = x;

			r = RED(nl_input->neuron_vector[x + y * w].output.ival);
			g = GREEN(nl_input->neuron_vector[x + y * w].output.ival);
			b = BLUE(nl_input->neuron_vector[x + y * w].output.ival);
			intensity = r - (g + b) / 2;
			if (intensity < 0)
				intensity = 0;
			nl_output->neuron_vector[x + y * w].output.ival = intensity;
		}
	}
}

void
rotate_scale_image(IplImage *img, double angle, double scale, int xi, int yi)
{
	CvSize img_size = cvGetSize(img);
	IplImage *tmp = cvCreateImage(img_size,img->depth, img->nChannels);
	CvMat *rotate = cvCreateMat(2,3,CV_32F);
	CvPoint2D32f center = cvPoint2D32f(
			(double)xi,
			(double)yi);
	cv2DRotationMatrix(center, angle, scale, rotate);
	cvWarpAffine(img, tmp, rotate, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0) );
	cvCopyImage(tmp, img);
	cvReleaseImage(&tmp);
}

void
translate_image(IplImage *img, int target_x, int target_y)
{
	CvRect roi = cvGetImageROI(img);
	CvPoint roi_center = cvPoint( (roi.x+roi.width)/2, (roi.y+roi.height)/2 );
	int dx = target_x - roi_center.x;
	int dy = target_y - roi_center.y;
	roi.x += dx;
	roi.y += dy;
	cvResetImageROI(img);
	cvSetImageROI(img, roi);
}

void
reshape_cv_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL;
	NEURON_LAYER *nl_input = NULL;
	IplImage *image_input = NULL;
	IplImage *image_output = NULL;
	CvRect input_roi, output_roi;
	float xi_target_center;
	float yi_target_center;
	int nl_number, p_number;
	int roi_x1, roi_y1;
	int wi, hi, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <xi_target_center> <yi_target_center>.", "", "");
		return;
	}

	// Gets the Filter Parameters - The Pointers And The Values - Void pointers must be casted
	xi_target_center = *((float *) (filter_desc->filter_params->next->param.pval));
	yi_target_center = *((float *) (filter_desc->filter_params->next->next->param.pval));

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	roi_x1 = (int) ( xi_target_center - (float)wo/2.0 + .5f);
	roi_y1 = (int) ( yi_target_center + (float)ho/2.0 + .5f);

	roi_x1 = MAX(roi_x1, 0);
	roi_y1 = MAX(roi_y1, 0);
	roi_x1 = MIN(roi_x1, wo/2);
	roi_y1 = MIN(roi_y1, ho/2);

	//convert from MAE coordinate system to OpenCV
	yi_target_center = (hi-1) - yi_target_center;
	roi_y1 = (hi-1) - roi_y1;

	input_roi = cvRect (0, 0, wi, hi);
	output_roi = cvRect (roi_x1, roi_y1, wo, ho);

	image_input = cvCreateImage(cvSize(wi, hi), 8, nl_input->output_type == COLOR ? 3 : 1);
	image_output = cvCreateImage(cvSize(wo, ho), 8, nl_input->output_type == COLOR ? 3 : 1);

	cvSetZero(image_input);
	cvSetZero(image_output);

	copy_neuron_layer_to_image(image_input, &input_roi, nl_input);

	cvSetImageROI(image_input, input_roi);

	//equalize_clahe_image(image_input);

	rotate_scale_image(image_input, 0.0, 1.0, xi_target_center, yi_target_center);

	cvResetImageROI(image_input);
	cvSetImageROI(image_input, output_roi);

	cvResize (image_input, image_output, CV_INTER_LINEAR);

	copy_image_to_neuron_layer(nl_output, NULL, image_output);

	cvReleaseImage(&image_input);
	cvReleaseImage(&image_output);
}

void gaussian_cv_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL;
	NEURON_LAYER *nl_input = NULL;
	NEURON_LAYER_LIST *n_list;
	PARAM_LIST *p_list;
	IplImage *image_input = NULL;
	IplImage *image_output = NULL;
	int wi, hi, wo, ho;
	int i, kernel_size;
	float sigma;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. The gaussian_filter must be applied on only one neuron layer (gaussian_filter).", "", "");
		return;
	}

	if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
	{
		Erro ("The output type of input neuron layer is different of the gaussian_filter output (gaussian_filter).", "", "");
		return;
	}

	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (--i != 2)
	{
		Erro ("Wrong number of parameters. The gaussian_filter have 2 parameters: kernel_size and sigma respectivally (gaussian_filter).", "", "");
		return;
	}

	kernel_size = ((int ) (filter_desc->filter_params->next->param.ival));
	sigma	    = ((float ) (filter_desc->filter_params->next->next->param.fval));

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	image_input = cvCreateImage(cvSize(wi, hi), 8, nl_input->output_type == COLOR ? 3 : 1);
	image_output = cvCreateImage(cvSize(wo, ho), 8, nl_input->output_type == COLOR ? 3 : 1);

	cvSetZero(image_input);
	cvSetZero(image_output);

	copy_neuron_layer_to_image(image_input, NULL, nl_input);

	cvSmooth(image_input, image_output, CV_GAUSSIAN, kernel_size, kernel_size, sigma, sigma);

	copy_image_to_neuron_layer(nl_output, NULL, image_output);

	cvReleaseImage(&image_input);
	cvReleaseImage(&image_output);
}
