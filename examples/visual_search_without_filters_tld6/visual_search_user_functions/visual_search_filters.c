//#include <opencv/cv.h>
#include "visual_search_filters.h"
#include "visual_search_user_functions.h"
//#include "visual_search_utils.h"
//#include "../visual_search_clahe/clahe.h"


#define MAX	20.0
void 
compute_movement_diff_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int wi, hi, xo, yo, wo, ho, r1, g1, r2, g2, i, r, g, b, rn, gn;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	i = 0;
	for (yo = 0; yo < ho; yo++)
	{			
		for (xo = 0; xo < wo; xo++)
		{
			r1 = RED(nl_movement_table.neuron_vector[i].output.ival);
			g1 = GREEN(nl_movement_table.neuron_vector[i].output.ival);
			r2 = RED(nl_input->neuron_vector[i].output.ival);
			g2 = GREEN(nl_input->neuron_vector[i].output.ival);
			r = (r1-r2);
			g = (g1-g2);
			r = ( r > MAX ? MAX : r );
			r = ( r < -MAX ? -MAX : r );
			g = ( g > MAX ? MAX : g );
			g = ( g < -MAX ? -MAX : g );
			b = 0;
			rn = 0;
			gn = 0;
			if ( r < 0.0 )
			{
				r = -r;
				rn = r;
				b = 1;
			}
			if ( g < 0.0 )
			{
				g = -g;
				gn = g;
				b = 1;
			}
			if ( b )
			{
				b = sqrt(rn*rn + gn*gn);
			}
			r = r * 255.0/MAX + 0.5;
			g = g * 255.0/MAX + 0.5;
			b = b * 255.0/sqrt(2*MAX*MAX) + 0.5;
			nl_output->neuron_vector[i].output.ival = PIXEL(r, g, b);
			i++;
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

	
	scale_factor = 1.0 / g_scale_factor;

	// Parallel translation filter capabilities where OpenMP available
#ifdef	_OPENMP
	#pragma omp parallel for private(yo,yi,xo,xi)
#endif
	for (yo = 0; yo < ho; yo++)
	{			
		yi = (int) (scale_factor * ((float) yo - (float) ho / 2.0) + yi_target_center + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
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

/* Inlined mask functions - Based on mae.h macros */

inline	unsigned int
red_mask(unsigned int pixel)
{
	return(RED(pixel));
}

inline	unsigned int
green_mask(unsigned int pixel)
{
	return(GREEN(pixel));
}

inline	unsigned int
blue_mask(unsigned int pixel)
{
	return(BLUE(pixel));
}

/*
*********************************************************************************
* Function: red_channel_mask							*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void red_mask_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi, yi, wi, hi, wo, ho;
	//int color_channel;
	//unsigned int (*mask_func)(unsigned int);

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The red_mask_filter must have no parameters.", "", "");
		return;
	}

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	// Input-Output width
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	if(wi != wo || hi != ho)
	{
		Erro ("Error: Input and output layers on red_channel_mask filter must have the same 2D size.", "", "");
		return;
	}
	
	for (yi = 0; yi < hi; yi++)
		for(xi = 0; xi < wi; xi++)
			nl_output->neuron_vector[xi + yi * wi].output.ival =  RED(nl_input->neuron_vector[xi + yi * wi].output.ival);
}

void green_mask_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi, yi, wi, hi, wo, ho;
	//int color_channel;
	//unsigned int (*mask_func)(unsigned int);

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The green_mask_filter must have no parameters.", "", "");
		return;
	}

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	// Input-Output width
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	if(wi != wo || hi != ho)
	{
		Erro ("Error: Input and output layers on green_channel_mask filter must have the same 2D size.", "", "");
		return;
	}
	
	for (yi = 0; yi < hi; yi++)
		for(xi = 0; xi < wi; xi++)
			nl_output->neuron_vector[xi + yi * wi].output.ival =  GREEN(nl_input->neuron_vector[xi + yi * wi].output.ival);
}

void blue_mask_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi, yi, wi, hi, wo, ho;
	//int color_channel;
	//unsigned int (*mask_func)(unsigned int);

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The blue_mask_filter must have no parameters.", "", "");
		return;
	}

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	// Input-Output width
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	if(wi != wo || hi != ho)
	{
		Erro ("Error: Input and output layers on blue_channel_mask filter must have the same 2D size.", "", "");
		return;
	}
	
	for (yi = 0; yi < hi; yi++)
		for(xi = 0; xi < wi; xi++)
			nl_output->neuron_vector[xi + yi * wi].output.ival =  BLUE(nl_input->neuron_vector[xi + yi * wi].output.ival);
}


void similarity_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi, yi, wi, hi, wo, ho;
	//int color_channel;
	//unsigned int (*mask_func)(unsigned int);

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The red_mask_filter must have no parameters.", "", "");
		return;
	}

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	// Input-Output width
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	if(wi != wo || hi != ho)
	{
		Erro ("Error: Input and output layers on red_channel_mask filter must have the same 2D size.", "", "");
		return;
	}

	for (yi = 0; yi < hi; yi++)
		for(xi = 0; xi < wi; xi++)
			nl_output->neuron_vector[xi + yi * wi].output.fval =  (float) BLUE(nl_input->neuron_vector[xi + yi * wi].output.ival);
}

void threshold_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi, yi, wi, hi;
	double num_larger_than_zero, max_value, threshold;	

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The red_mask_filter must have no parameters.", "", "");
		return;
	}

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	// Input-Output width
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	num_larger_than_zero = max_value = threshold = 0.0;
	
	for (yi = 0; yi < hi; yi++)
	{
		for(xi = 0; xi < wi; xi++)
		{
			if (nl_input->neuron_vector[xi + yi * wi].output.fval > 0.0)
			{
				if (nl_input->neuron_vector[xi + yi * wi].output.fval > max_value)
					max_value = nl_input->neuron_vector[xi + yi * wi].output.fval;
				threshold += nl_input->neuron_vector[xi + yi * wi].output.fval;
				num_larger_than_zero += 1.0;
			}
		}
	}
	threshold /= num_larger_than_zero;
	threshold = threshold + (max_value - threshold) / 2.0;

	for (yi = 0; yi < hi; yi++)
		for(xi = 0; xi < wi; xi++)
			if (nl_input->neuron_vector[xi + yi * wi].output.fval > threshold)
				nl_output->neuron_vector[xi + yi * wi].output.fval =  nl_input->neuron_vector[xi + yi * wi].output.fval;
			else 
				nl_output->neuron_vector[xi + yi * wi].output.fval =  0.0;


}


void
compute_weigheted_neighboor_filter(FILTER_DESC *filter_desc)
{
	int u, v;
	int w, h;
	double partial_weight;
	double log_factor;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
double threshold, pt;
int i;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;

	log_factor = filter_desc->filter_params->next->param.fval;

	// Input-Output width
	w = nl_input->dimentions.x;
	h = nl_input->dimentions.y;


	threshold = 0.0; pt = 0.0;
	i = 0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			if (BLUE(nl_input->neuron_vector[u + w * v].output.ival) != 0)
				partial_weight = (double) compute_weigheted_neighborhood(nl_input->neuron_vector, w, h, u, v, log_factor);// * (double) BLUE(nl_input->neuron_vector[u + w * v].output.ival);
			else
				partial_weight = 0.0;

			nl_output->neuron_vector[u + w * v].output.fval = partial_weight;
			threshold +=  nl_v1_activation_map_neuron_weight.neuron_vector[i].output.fval;
			pt += ( partial_weight > 0.0 ? 2.0 : 0.0 );
			i++;
		}
	}
/*	threshold /= (double) pt;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			//nl_output->neuron_vector[u + w * v].output.fval = ( nl_output->neuron_vector[u + w * v].output.fval > threshold ? nl_output->neuron_vector[u + w * v].output.fval : 0.0 );
		}
	}
*/
}

void
threshold_cut_filter(FILTER_DESC *filter_desc)
{
	int u, v;
	int w, h;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	float threshold, max, pt, value;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;

	// Input-Output width
	w = nl_input->dimentions.x;
	h = nl_input->dimentions.y;

	threshold = 0.0; pt = 0.0;
	max = 0.0;
/*	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			value = nl_input->neuron_vector[u + w * v].output.fval;
			value = (BLUE(nl_movement_map.neuron_vector[u + w * v].output.ival) > 0 ? value: 0.0);
			nl_output->neuron_vector[u + w * v].output.fval = ( value > threshold + 0.5*(max - threshold) ? value : 0.0 );
		}
	}
*/	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			value = nl_input->neuron_vector[u + w * v].output.fval;
			if (value > 0.0)
			{
				threshold += value;
				pt += 1.0;
				if (value > max) max = value;
			}
		}
	}
	threshold /= (float) pt;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			value = nl_input->neuron_vector[u + w * v].output.fval;
			nl_output->neuron_vector[u + w * v].output.fval = ( value > threshold + 0.5*(max - threshold) ? value : 0.0 );
		}
	}

}

void
generate_hough_activation_map(FILTER_DESC *filter_desc)
{
	int u, v, i;
	int b, x, y, xc, yc, xi, yi, xv, yv;
	int wU, hV, wX, hY;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int value;
	double log_factor, partial_weight;

	log_factor = filter_desc->filter_params->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;

	// Output width
	wU = nl_input->dimentions.x;
	hV = nl_input->dimentions.y;

	// Input-Output width
	wX = nl_output->dimentions.x;
	hY = nl_output->dimentions.y;

	//Zera a saida
	for (i = 0; i < wX*hY; i++)
		nl_output->neuron_vector[i].output.fval = 0.0;

	for (v = 0; v < hV; v++)
	{
		for (u = 0; u < wU; u++)
		{
			value = nl_input->neuron_vector[v * wU + u].output.ival;
			b = BLUE(value);

			if (b > 0){
				
				// deslocamento do centro para o neuronio principal   
				xc = gt_x_displecement_from_fovea( GREEN(value) );   
				yc = gt_y_displecement_from_fovea( RED(value) );
				map_v1_to_image(&xi, &yi, wX, hY, u, v, wU, hV, wX/2.0, hY/2.0, 0, log_factor);
				x = (xi - xc);
				y = (yi - yc);
				//printf("x %d y %d\n", x, y);
				partial_weight = nl_v1_activation_map_neuron_weight_thresholded.neuron_vector[v * wU + u].output.fval;
				for (yv = -0; yv <= 0; yv++)
					for (xv = -0; xv <= 0; xv++)
						nl_output->neuron_vector[(x + xv) + wX * (y+yv)].output.fval += partial_weight;
			}
		}
	}
}


/*

void equalize_clahe_image(IplImage *img)
{
	IplImage *b = NULL;
	IplImage *g = NULL;
	IplImage *r = NULL;

	if (img->nChannels == 1)
	{
		cvCLAdaptEqualize(img, img, 2, 2, 256, 1.4, CV_CLAHE_RANGE_FULL);
	}
	else if (img->nChannels == 3)
	{
		b = cvCreateImage(cvGetSize(img), 8, 1);
		g = cvCreateImage(cvGetSize(img), 8, 1);
		r = cvCreateImage(cvGetSize(img), 8, 1);

		cvSplit(img, b, g, r, NULL);

		cvCLAdaptEqualize(b, b, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);
		cvCLAdaptEqualize(g, g, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);
		cvCLAdaptEqualize(r, r, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);

		cvMerge(b,g,r,NULL,img);

		cvReleaseImage(&b);
		cvReleaseImage(&g);
		cvReleaseImage(&r);
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

int
max(int a, int b)
{
	return (a < b) ? b : a;
}


int
min(int a, int b)
{
	return (b < a) ? b : a;
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

	roi_x1 = max(roi_x1, 0);
	roi_y1 = max(roi_y1, 0);

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

	equalize_clahe_image(image_input);

	rotate_scale_image(image_input, 0.0, g_scale_factor, xi_target_center, yi_target_center);

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

	kernel_size = *((int *) (filter_desc->filter_params->next->param.pval));
	sigma	    = *((float *) (filter_desc->filter_params->next->next->param.pval));

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
*/
