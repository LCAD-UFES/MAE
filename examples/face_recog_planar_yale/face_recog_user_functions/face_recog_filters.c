#include "face_recog_filters.h"
#include "face_recog_user_functions.h"

// Global Variables



/*
***********************************************************
* Function: copy_window_image_to_neuron_layer
* Description:
* Inputs:none
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

void copy_window_image_to_neuron_layer (NEURON_LAYER *nl_dest, int img_win)
{
	
	int p_nViewport[4]; 
	int nWidth, nHeight;
	static int nPreviusWidth = 0, nPreviusHeight = 0;
	int xi, yi, wi, hi, xo, yo, wo, ho, r, g, b;
	static GLubyte *pScreenPixels = (GLubyte *) NULL;
	NEURON *neuron_vector;

	glutSetWindow(img_win);

        glGetIntegerv (GL_VIEWPORT, p_nViewport);
	nWidth = p_nViewport[2];
	nHeight = p_nViewport[3];
	
	if ((nWidth != nPreviusWidth) || (nHeight != nPreviusHeight))
	{
		free (pScreenPixels);
		if ((pScreenPixels = (GLubyte *) malloc (3 * nWidth * nHeight * sizeof (GLubyte))) == (GLubyte *) NULL)
		{
			Erro ("Cannot allocate more memory", "", "");
			return;
		}
		nPreviusWidth = nWidth;
		nPreviusHeight = nHeight;
	}
	
	glReadBuffer (GL_BACK);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, pScreenPixels); 
	glDisable(GL_READ_BUFFER);
	
	wi = nWidth;
	hi = nHeight;
	
	neuron_vector = nl_dest->neuron_vector;
        wo = nl_dest->dimentions.x;
        ho = nl_dest->dimentions.y;
            
        for (yo = 0, yi = (hi-ho)/2; yo < ho; yo++, yi++)
        {
		for (xo = 0, xi = (wi-wo)/2; xo < wo; xo++, xi++)
		{
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
			{
        	                r = pScreenPixels[3 * (yi * wi + xi) + 0];
				g = pScreenPixels[3 * (yi * wi + xi) + 1];
				b = pScreenPixels[3 * (yi * wi + xi) + 2];
			}
			else
				r = g = b = 0;
				
			switch (nl_dest->output_type)
			{
				case COLOR:
					neuron_vector[yo * wo + xo].output.ival = PIXEL(r, g, b); 
					break;
				case GREYSCALE:
					neuron_vector[yo * wo + xo].output.ival = (r + g + b) / 3; 
					break;  
				case BLACK_WHITE:
					neuron_vector[yo * wo + xo].output.ival = r > 50? NUM_COLORS - 1: 0;
					break;
				case GREYSCALE_FLOAT:
					neuron_vector[yo * wo + xo].output.fval = (float) (r + g + b) / 3.0; 
					break;
			}
		}
	}
}



/*
*********************************************************************************
* Function: scale_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void scale_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float scale_factor, k;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The scale_nl_filter must have only one parameter <scale_factor>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	scale_factor = filter_desc->filter_params->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	k = 1.0/scale_factor;
	
	for (yo = 0; yo < ho; yo++)
	{	
		yi = (int) (k * (float) yo + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (k * (float) xo + .5f);
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}
	}
}



/*
*********************************************************************************
* Function: subsample_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void subsample_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float subsample_factor;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The scale_nl_filter must have only one parameter <subsample_factor>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	subsample_factor = filter_desc->filter_params->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	for (yo = 0; yo < ho; yo++)
	{	
		yi = (int) (floor ((double) yo / subsample_factor) * subsample_factor + subsample_factor / 2.0);
		
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (floor ((double) xo / subsample_factor) * subsample_factor + subsample_factor / 2.0);
				
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output.fval = (float) nl_input->neuron_vector[xi + yi * wi].output.ival;
			else
				nl_output->neuron_vector[xo + yo * wo].output.fval = 0.0;
		}
	}
}



/*
*********************************************************************************
* Function: rotate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void rotate_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float angle, cos_angle, sin_angle;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have only one parameter <angle>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	angle = filter_desc->filter_params->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	angle *= pi/180.0f;
	cos_angle = cos(angle);
	sin_angle = sin(angle);

	for (yo = 0; yo < ho; yo++)
	{			
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ( cos_angle * (float) xo + sin_angle *(float) yo + .5f);
			yi = (int) (-sin_angle * (float) xo + cos_angle *(float) yo + .5f);
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
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

void translate_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float x_offset, y_offset;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <x_offset> <y_offset>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	x_offset = filter_desc->filter_params->next->param.fval;
	y_offset = filter_desc->filter_params->next->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
		
	for (yo = 0; yo < ho; yo++)
	{			
		yi = (int) ((float) yo + y_offset + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo + x_offset + .5f);
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}
	}
}



/*!
*********************************************************************************
* Function: compute_2d_gaussian_kernel						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

KERNEL *compute_2d_gaussian_kernel (int kernel_size, float sigma)
{
	int i, x, y, index = 0, radius;
	float k1, k2, *kernel_points = NULL, acc = .0f, num_points;
	KERNEL *gaussian_kernel = NULL;

	if ((kernel_size % 2) == 0)
	{
		Erro ("The kernel size must be an odd number (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	if ((gaussian_kernel = (KERNEL *) alloc_mem (sizeof(KERNEL))) == NULL)
	{
		Erro ("Cannot allocate more memory (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	gaussian_kernel->kernel_size = kernel_size;
	
	if ((gaussian_kernel->kernel_points = kernel_points = (float *) alloc_mem (kernel_size * kernel_size * sizeof (float))) == NULL)
	{
		Erro ("Cannot allocate more memory (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	k2 = -.5f / (sigma * sigma);
	radius = kernel_size >> 1; // kernel_size / 2

	for (y = -radius; y <= radius; y++)
	{
		for (x = -radius; x <= radius; x++)
		{
			printf("%1.03f ", kernel_points[index] = exp (k2 * (float) (x * x + y * y)));
			acc += kernel_points[index];
			index++;
		}
		printf("\n");
	}
	
	num_points = kernel_size * kernel_size;
	k1 = 1.0f/acc;
	for(i = 0; i < num_points; i++)
		kernel_points[i] *= k1;
		
	return gaussian_kernel;
}



/*!
*********************************************************************************
* Function: apply_2d_kernel							*
* Description: apply kernel to a neuron vector region centered at point (x,y)	*
* Inputs: 	kernel => kernel points						*
*		kernel_size => kernel points number				*
*		nv => the neuron vector						*
*		(x,y) => center point						*
*		(w,h) => the neuron vector dimentions				*
* Output: the result of the application of the kernel	 			*
*********************************************************************************
*/

int 
apply_2d_kernel (float *kernel, int kernel_size, 
	      NEURON *nv, int x0, int y0, int wi, int hi)
{
	float r_acc = .0f, g_acc = .0f, b_acc = .0f, weight;
	int xi, yi, pixel, x_offset, y_offset, index = 0;
	
	x_offset = x0-(kernel_size>>1);
	y_offset = y0-(kernel_size>>1);
	
	for (yi = y_offset; yi < (y_offset + kernel_size); yi++)
	{	
		for (xi = x_offset; xi < (x_offset + kernel_size); xi++)
		{
			if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
			{
				pixel = nv[xi + yi * wi].output.ival;
				weight = kernel[index++];
				
				r_acc += weight * (float) RED(pixel);
				g_acc += weight * (float) GREEN(pixel);
				b_acc += weight * (float) BLUE(pixel);
			}
		}
	}
	//printf("rgb = [%f, %f, %f]\n",r_acc,g_acc,b_acc);
	return PIXEL(((int)(r_acc+.5f)),((int)(g_acc+.5f)),((int)(b_acc+.5f)));
}



