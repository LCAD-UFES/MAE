#include "face_recog_filters.h"
#include "face_recog_user_functions.h"

// Global Variables


/*
***********************************************************
* Function: draw_normalized_face_into_input_window
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void draw_normalized_face_into_input_window (INPUT_DESC *input, double target_baseline, double x_offset, double y_offset)
{
	float face_centre_x, face_centre_y, scale_factor, alpha;
	float a, b,/* c, */ m, /*noose_length,*/ baseline;
	GLdouble w, h, d;
	
	glutSetWindow (input->win);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0, 
		   0.0, 0.0, -d,
		   0.0, 1.0, 0.0);
	
	face_centre_x = (float) (g_nLeftEyeX + g_nRightEyeX + g_nNoseX) / 3.0f;
	face_centre_y = (float) (g_nLeftEyeY + g_nRightEyeY + g_nNoseY) / 3.0f;
	//face_centre_x = (float) (g_nLeftEyeX + g_nRightEyeX) / 2.0f;
	//face_centre_y = (float) (g_nLeftEyeY + g_nRightEyeY) / 2.0f;
	
	printf ("Face centre = (%f, %f)\n", face_centre_x, face_centre_y);
	
	m = (float) (g_nLeftEyeY - g_nRightEyeY) / (float) (g_nLeftEyeX - g_nRightEyeX);
	
	/*a = m;	b = -1;	c = g_nRightEyeY - m * g_nRightEyeX;
	
	noose_length = fabs (a * (float) g_nNoseX + b * (float) g_nNoseY + c) /
		      sqrtf (a * a + b * b);

	printf ("Noose length = %f\n", noose_length);*/
	
	a = (g_nLeftEyeX - g_nRightEyeX);
	b = (g_nLeftEyeY - g_nRightEyeY);
	
	baseline = sqrtf (a * a + b * b);
	
	printf ("Baseline = %f\n", baseline);

	alpha = atanf (m);
	
	printf ("Face rotation = %f\n", alpha);
	
	scale_factor = target_baseline / baseline;

	printf ("Scale factor = %f\n", scale_factor);

	glScalef (scale_factor, scale_factor, 1.0f);
	glRotatef (-180.0f/ pi * alpha, .0f, .0f, 1.0f);
	glTranslatef (-face_centre_x + x_offset, -face_centre_y + y_offset, -d);

	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (0.0, 0.0, 0.0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, 0.0); glVertex3i (0, 0, 0);
	glTexCoord2f (0.0, 1.0); glVertex3i (0, input->tfh, 0);
	glTexCoord2f (1.0, 1.0); glVertex3i (input->tfw, input->tfh, 0);
	glTexCoord2f (1.0, 0.0); glVertex3i (input->tfw, 0, 0);
	glEnd ();
	glDisable (GL_TEXTURE_2D);	
	glFlush ();
}



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
* Function: face_reshape_filter                 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void face_reshape_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	INPUT_DESC *input = NULL;
	int nl_number, p_number;
	float baseline_factor, target_baseline, x_offset, y_offset;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The activation_map_filter must have only one parameter <nInputsPerNeuron> <fltGaussianRadius> <fltLogFactor>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	baseline_factor = filter_desc->filter_params->next->param.fval;
	x_offset  	= filter_desc->filter_params->next->next->param.fval;
	y_offset  	= filter_desc->filter_params->next->next->next->param.fval;
	
	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	target_baseline = baseline_factor * nl_output->dimentions.x;
	
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	input = get_input_by_neural_layer (nl_input);
	
	// Draw cutted face into input window
	draw_normalized_face_into_input_window (input, target_baseline, x_offset, y_offset);

	// Copy cutted face image from OpenGL back buffer to neuron' outputs
	copy_window_image_to_neuron_layer (nl_output, input->win);
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



/*
*********************************************************************************
* Function: face_reshape_filter2	              				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void face_reshape_filter2 (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	KERNEL *gaussian_kernel = NULL;
	int nl_number, p_number;
	float baseline_factor, target_baseline, x_offset, y_offset;
	float face_centre_x, face_centre_y, scale_factor, alpha;
	float a, b, m, baseline;
	int xi, yi, wi, hi, xo, yo, wo, ho, kernel_size;
	float cos_alpha, sin_alpha, xt, yt, xr, yr, xs, ys, k, sigma;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

/*	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The face_reshape_filter2 must have tree parameters <baseline_factor> <x_offset> <y_offset>", "", "");
		return;
	}*/
	
	if (p_number != 6)
	{
		Erro ("Error: Wrong number of parameters. The activation_map_filter must have five parameters <baseline_factor> <x_offset> <y_offset> <kernel_size> <gaussian_sigma>", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	baseline_factor = filter_desc->filter_params->next->param.fval;
	x_offset  	= filter_desc->filter_params->next->next->param.fval;
	y_offset  	= filter_desc->filter_params->next->next->next->param.fval;
	kernel_size	= filter_desc->filter_params->next->next->next->next->param.ival;
	sigma	  	= filter_desc->filter_params->next->next->next->next->next->param.fval;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	target_baseline = baseline_factor * nl_output->dimentions.x;
	
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	
	if (filter_desc->private_state == NULL)
	{
		if ((gaussian_kernel = compute_2d_gaussian_kernel (kernel_size, sigma)) == NULL)
		{
			Erro ("Cannot create gaussian kernel (gaussian_filter).", "", "");
			return;
		}
		
		filter_desc->private_state = (void *) gaussian_kernel;
	}
	else
		gaussian_kernel = (KERNEL *) filter_desc->private_state; 
		
	//face_centre_x = (float) (g_nLeftEyeX + g_nRightEyeX + g_nNoseX) / 3.0f;
	//face_centre_y = (float) (g_nLeftEyeY + g_nRightEyeY + g_nNoseY) / 3.0f;

	face_centre_x = (float) (g_nLeftEyeX + g_nRightEyeX) / 2.0f;
	face_centre_y = (float) (g_nLeftEyeY + g_nRightEyeY) / 2.0f;
	
	printf ("Face centre = (%f, %f)\n", face_centre_x, face_centre_y);
	
	m = (float) (g_nLeftEyeY - g_nRightEyeY) / (float) (g_nLeftEyeX - g_nRightEyeX);
		
	a = (g_nLeftEyeX - g_nRightEyeX);
	b = (g_nLeftEyeY - g_nRightEyeY);
	
	baseline = sqrtf (a * a + b * b);
	
	printf ("Baseline = %f\n", baseline);

	alpha = -atanf (m);
	cos_alpha = cos(alpha);
	sin_alpha = sin(alpha);
		
	printf ("Face rotation = %f\n", alpha);
	
	scale_factor = target_baseline / baseline;
	k = 1.0/scale_factor;

	printf ("Scale factor = %f\n", scale_factor);

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
		for (xo = 0; xo < wo; xo++)
		{
			xt = (float) xo - (float) wo/2.0f;
			yt = (float) yo - (float) ho/2.0f;
			
			xr =  cos_alpha * xt + sin_alpha * yt;
			yr = -sin_alpha * xt + cos_alpha * yt;
			
			xs = k * xr;
			ys = k * yr;
			
			xt = xs + face_centre_x - x_offset;
			yt = ys + face_centre_y - y_offset;
				
			xi = (int) (xt + .5f);
			yi = (int) (yt + .5f);
				
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
			
/*			nl_output->neuron_vector[xo + yo * wo].output.ival = 
				apply_2d_kernel (gaussian_kernel->kernel_points, 
					      gaussian_kernel->kernel_size, 
					      nl_input->neuron_vector, xi, yi, wi, hi);
*/		}
	}
}

/*
*********************************************************************************
* Function: red_mask_filter	  	            				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void red_mask_filter (FILTER_DESC *filter_desc)
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

/*
*********************************************************************************
* Function: green_mask_filter		              				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void green_mask_filter (FILTER_DESC *filter_desc)
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

/*
*********************************************************************************
* Function: blue_mask_filter		              				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void blue_mask_filter (FILTER_DESC *filter_desc)
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
