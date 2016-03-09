/*
*********************************************************************************
* Module : Filter functions							*
* version: 1.0									*
*    date: 13/02/2001								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/
 
#include "filter.h"
#include "smv_user_functions.h"

/*
#define TEST_V1
#define TEST2_V1
*/
extern int current_kernel;
double  y0_val;
int *receptive_field_distribution = NULL;

#ifdef DEBUG_DISTANCE_FROM_IMAGE_CENTER
	int debug_distance_from_image_center = 1;
#endif

#ifdef	DEBUG_V1_FILTER_OUTPUT_COORDINATE
	int debug_v1_filter_output_coordinate = 1;
	FILE * v1_filter_out_coord_debug_file;
#endif


/*
*********************************************************************************
* Return the ganglion neuron response as a function of the receptive field 	*
* average						 			*
*********************************************************************************
*/

NEURON_OUTPUT
retina_model_ganglion_neuron_response (double receptive_filter_average, double scale_factor, double offset, double rectification, double accomodation, OUTPUT_TYPE output_type)
{
#ifdef DEBUG_GANGLION_RESPONSE
	static FILE *ganglion_response;
	static int file_close = 1;
	NEURON_OUTPUT response;

	if (file_close)
	{
		ganglion_response = fopen("ganglion_neuron_response.dat","w");
		file_close = 0;
	}
#endif
	
#ifndef DEBUG_GANGLION_RESPONSE
	NEURON_OUTPUT response;
	
#endif
	/* response is the sigmoid of the receptive_filter_average times NUM_COLORS-1 */
	
	if (output_type == GREYSCALE_FLOAT)
		response.fval = scale_factor / (1.0 + exp (rectification - accomodation*receptive_filter_average)) + offset;
	else
		response.ival = (int) (((double) (NUM_GREYLEVELS - 1) / (1.0 + exp (rectification - accomodation*receptive_filter_average))) + 0.00005);
		
	
#ifdef DEBUG_GANGLION_RESPONSE
	fprintf(ganglion_response,"%f\t%f\n",receptive_filter_average, (((double) (NUM_GREYLEVELS-1) / (1.0 + exp (rectification-accomodation*receptive_filter_average))) + 0.00005));
#endif
/*	printf("%f\t%f\n",receptive_filter_average, (((double) (NUM_GREYLEVELS-1) / (1.0 + exp (rectification-accomodation*receptive_filter_average))) + 0.00005));*/
	return response;
}


/*
*********************************************************************************
* Compute distance from the log-polar v to input image center	 		*
* How to calculate Y0_VAL 							*
* y0_val = pow ((20.05/(0.26 * C1)), (1.0/0.26)) - 0.08; 			*
*********************************************************************************
*/
#define Y0_VAL	0.3544963658

double
retina_model_distance_from_image_center (int wi, int hi, int w, int h, int v, double log_factor)
{

	double exp_val, x, y;
#ifdef	DEBUG_DISTANCE_FROM_IMAGE_CENTER
	static int file_open = 0;
	static FILE * file_distance_from_image_center;
	static int file_close = 0;

	if (debug_distance_from_image_center && !file_open)
	{
		if ((file_distance_from_image_center = fopen ("distance_from_image_center.dat", "w")) == NULL)
			Erro ("Could not open distance_from_image_center.dat", "", "");
		else
			file_open = 1;
	}
#endif
	if (((int) log_factor) == SERENO_MODEL)
	{
		x = 63.951256 * ((double) v/(double) (w/2));
		y = pow ((20.05 / (0.26 * (C1 - x))), (1.0 / 0.26)) - 0.08;
		y = 30.0 * (y - Y0_VAL) / (30.0 - Y0_VAL);
		exp_val = (double) (wi/2) * (y / 30.0);
	}
	else
	{
		x = ((double) v / (double) (w/2)) * log_factor;
		exp_val = (double) (wi/2) * (exp (( log (log_factor) * (x - log_factor)) / log_factor) - exp ( -log (log_factor))) * ( 1.0 / (1.0 - exp ( -log (log_factor))));
	}
	
#ifdef DEBUG_DISTANCE_FROM_IMAGE_CENTER
	if (debug_distance_from_image_center && file_open)
		fprintf(file_distance_from_image_center, "%d\t%.16f\n", v, exp_val);

	if (!debug_distance_from_image_center && file_open && !file_close)
	{
		fclose(file_distance_from_image_center);
		file_close = 1;
	}
#endif
	
	return (exp_val);

}

/*
*********************************************************************************
* Compute the receptive descriptor			 			*
*********************************************************************************
*/

RECEPTIVE_FIELD_DESCRIPTION *
get_receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int u, int v, int w, int distribution_type)
{
	int index;
	switch(distribution_type)
	{
		case CONSTANT_DISTRIBUTION:
			return &(receptive_field_descriptor[0]);
		case STATISTIC_DISTRIBUTION:
			return &(receptive_field_descriptor[receptive_field_distribution[u + w * v]]);
		case GAUSSIAN_DISTRIBUTION:
			index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			return &(receptive_field_descriptor[index]);
		default:
			Erro ("Distribution Type invalid.", "It can be CONSTANT_DISTRIBUTION, STATISTIC_DISTRIBUTION or GAUSSIAN_DISTRIBUTION.", "");
			return ((RECEPTIVE_FIELD_DESCRIPTION *) NULL);
	}
}

/*
*********************************************************************************
* Compute the receptive descriptor			 			*
*********************************************************************************
*/

void
retina_model_compute_receptive_field_descriptor (FILTER_DESC *filter, int wi, int hi, int w, int h, int distribution_type, double log_factor)
{
	int v;
	int receptive_field_numbers;
	int factor_size;
	
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	int radius;
	
	//y0_val = pow ((20.05/(0.26 * C1)), (1.0/0.26)) - 0.08;

	switch(distribution_type)
	{
		case CONSTANT_DISTRIBUTION:
			receptive_field_numbers = 1;
			break;
		case STATISTIC_DISTRIBUTION:
			receptive_field_numbers = 3;
			break;
		case GAUSSIAN_DISTRIBUTION:
			receptive_field_numbers = w / 2;
			break;
		default:
			Erro ("Distribution Type invalid.", "It can be CONSTANT_DISTRIBUTION, STATISTIC_DISTRIBUTION or GAUSSIAN_DISTRIBUTION.", "");
	}
	receptive_field_descriptor = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof (RECEPTIVE_FIELD_DESCRIPTION) * receptive_field_numbers);
	filter->private_state = (void *) receptive_field_descriptor;

	switch(distribution_type)
	{
		case CONSTANT_DISTRIBUTION:
			compute_receptive_field_points (&(receptive_field_descriptor[0]), (KERNEL_SIZE - 1) / 2);
			break;
		case STATISTIC_DISTRIBUTION:
			for (factor_size = 1, v = 0; v < receptive_field_numbers; v++, factor_size *= 2) 
			{
				radius = factor_size * (KERNEL_SIZE - 1) / 2;
#ifdef	DEBUG_RECEPTIVE_FIELD_DESCRIPTOR
				printf ("%d\t%d\t%d\t%d\t%d\n",w, h, v, radius);
#endif
				compute_receptive_field_points (&(receptive_field_descriptor[v]), radius);
			}
			break;
		case GAUSSIAN_DISTRIBUTION:
			for (v = 1; v <= receptive_field_numbers; v++) 
			{
				radius = (int) ((retina_model_distance_from_image_center (wi, hi, w, h, v, log_factor) - retina_model_distance_from_image_center (wi, hi, w, h, v - 1, log_factor))/2.0 + 0.5);
				if (radius <= 3)
					radius = 3;
#ifdef	DEBUG_RECEPTIVE_FIELD_DESCRIPTOR
				printf ("%d\t%d\t%d\t%d\t%d\n",w, h, v, radius, (2 * radius + 1) * (2 * radius + 1));
#endif
				compute_receptive_field_points (&(receptive_field_descriptor[v-1]), radius);
			}
			break;
		default:
			Erro ("Distribution Type invalid.", "It can be CONSTANT_DISTRIBUTION, STATISTIC_DISTRIBUTION or GAUSSIAN_DISTRIBUTION.", "");
	}
	filter->private_state = (void *) receptive_field_descriptor;
	
#ifdef	DEBUG_DISTANCE_FROM_IMAGE_CENTER
	debug_distance_from_image_center = 0;
#endif
}

/*
*********************************************************************************
* Compute the receptive average in a specific point x, y in the			*
* input at a distance d of the center of the retina	 			*
*********************************************************************************
*/

NEURON_OUTPUT
retina_model_receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_p, 
		 INPUT_DESC *input, GLubyte *image, int x, int y, int w, int h, 
		 int cone_organization, int concetrically_region, int color_model, OUTPUT_TYPE output_type)
{
	double receptive_field_average;
	int xr, yr, i, num_points;
	double intensity, yellow, rRGB, gRGB, bRGB, red, green, blue;
	double im, k, im_min, im_max; 
	int pixel;
	

#ifdef	TEST_V1
	OUTPUT_DESC *output2;
	
	output2 = get_output_by_name ("teste2_out");
#endif

	im = 0.0;
	im_min = 512.0;
	im_max = -512.0;
	receptive_field_average = 0.0;
	num_points = receptive_field_p->num_points;
	
	for (i = 0; i < receptive_field_p->num_points; i++)
	{
		xr = x + receptive_field_p->points[i].x;
		yr = y + receptive_field_p->points[i].y;
		
		if (xr < 0)
			continue;
		if (xr >= w)
			continue;
		if (yr < 0)
			continue;
		if (yr >= h)
			continue;
				
		pixel = input->neuron_layer->neuron_vector[yr * w + xr].output.ival;
		
		rRGB = (double) RED (pixel);
		gRGB = (double) GREEN (pixel);
		bRGB = (double) BLUE (pixel);
		
		if(color_model == HSI)
		{
			k = 255.0;
			if ((rRGB + gRGB + bRGB) == 0)
			{
				red 	= 0.0;
				green 	= 0.0;
				blue 	= 0.0;
			}
			else
			{
				red 	= rRGB / (rRGB + gRGB + bRGB);
				green 	= gRGB / (rRGB + gRGB + bRGB);
				blue 	= bRGB / (rRGB + gRGB + bRGB);
			}
		}
		else
		{
			k = 1.0;
			red 	= rRGB;
			green 	= gRGB;
			blue 	= bRGB;
		}

		switch (cone_organization)
		{
			case MCELLS:
				intensity = (k * (red + green + blue) / 3.0);
				if (concetrically_region == ONCENTER)
				{
					im = 
					 (receptive_field_p->points[i].gaussian_big - receptive_field_p->points[i].gaussian_small) * intensity;
#ifdef	TEST_V1
					if ((image != NULL) && output2->neuron_layer->neuron_vector[yr*w + xr].output.ival < 
					    (int) ((NUM_GREYLEVELS - 1) * (((receptive_field_p->points[i].gaussian_big - receptive_field_p->points[i].gaussian_small) - receptive_field_p->min_dog) / 
					    (receptive_field_p->max_dog - receptive_field_p->min_dog))))
						output2->neuron_layer->neuron_vector[yr*w + xr].output.ival = (int) ((NUM_GREYLEVELS - 1) * (((receptive_field_p->points[i].gaussian_big - receptive_field_p->points[i].gaussian_small) - receptive_field_p->min_dog) / 
					    								 (receptive_field_p->max_dog - receptive_field_p->min_dog)));
#endif
				}
				else
				{
					im = 
					 (receptive_field_p->points[i].gaussian_small - receptive_field_p->points[i].gaussian_big) * intensity ;
				}
				break;
				
			case RED_GREEN:
				if (concetrically_region == ONCENTER)
				{
					im = 	(red - green) *
						(receptive_field_p->points[i].gaussian_big -
						receptive_field_p->points[i].gaussian_small);
					  
				} 
				else
				{
					im = 	(red - green) *
						(- receptive_field_p->points[i].gaussian_big + 
						receptive_field_p->points[i].gaussian_small);
				} 
				break;
			case GREEN_RED:
				if (concetrically_region == ONCENTER)
				{
					im = 	(green - red)*
						(receptive_field_p->points[i].gaussian_big -
						receptive_field_p->points[i].gaussian_small);
					  
				} 
				else
				{
					im = 	(green - red)*
						(- receptive_field_p->points[i].gaussian_big + 
						receptive_field_p->points[i].gaussian_small);
				} 
				break;
			case BLUE_YELLOW:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
				{
					im = (blue - yellow) *
					 (receptive_field_p->points[i].gaussian_small -
					  receptive_field_p->points[i].gaussian_big);
					  
				} 
				else
				{
					im = (blue - yellow) *
					 (- receptive_field_p->points[i].gaussian_small +
					  receptive_field_p->points[i].gaussian_big);
				} 
				break;
			case YELLOW_BLUE:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
				{
					im = (- blue + yellow) *
					 (receptive_field_p->points[i].gaussian_small -
					  receptive_field_p->points[i].gaussian_big);
					  
				} 
				else
				{
					im = (- blue + yellow) *
					 (- receptive_field_p->points[i].gaussian_small +
					  receptive_field_p->points[i].gaussian_big);
				} 
				break;
		}
		receptive_field_average += im;
		if(im < im_min) im_min = im;
		if(im > im_max) im_max = im;
	}
	
	
	
	receptive_field_average = receptive_field_average / (double) (receptive_field_p->num_points);
	
	return (retina_model_ganglion_neuron_response (receptive_field_average, 1.0, 0.0, 1.0, 5.0, output_type));
}

/*
*********************************************************************************
* Compute image's xi and yi using v1 v and u			 		*
*********************************************************************************
*/

void
retina_model_map_v1_to_image (int *xi, int *yi, int wi, int hi, int u, int v, int w, int h, int x_center, int y_center, double correction, double log_factor)
{
	static int previous_u = -1;
	static double previous_d;
	double d, theta;
	double cost, sint;
	
	if (u < w/2)
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = retina_model_distance_from_image_center (wi, hi, w, h, w/2 - 1 - u, log_factor);

		theta = pi * (((double) h * (3.0 / 2.0) - ((double) v * correction)) / (double) h);
		
	}
	else
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = retina_model_distance_from_image_center (wi, hi, w, h, u - w/2, log_factor);
		
		theta = pi * (((double) h * (3.0 / 2.0) + ((double) v * correction)) / (double) h);
		
	}
	
	cost = cos(theta);
	sint = sin(theta);
	
	*xi = (int) (d * cost + 0.5);
	*yi = (int) (d * sint + 0.5);
	*xi += x_center;
	*yi += y_center;

	previous_u = u;
	previous_d = d;

#ifdef	DEBUG_V1_FILTER_OUTPUT_COORDINATE
	if (debug_v1_filter_output_coordinate)
		fprintf(v1_filter_out_coord_debug_file,"%d\t%d\t%d\t%d\t%f\n",u,v,*xi,*yi,(180.0 * theta / pi));
#endif

}

/*
*********************************************************************************
* Function: v1_filter								*
* Description: apply the V1 layer's filter at the neuron layer 			*
* Inputs:	filter_desc - Filter description				*
*********************************************************************************
*/

void
compute_receptive_field_distribution (int distribution_type, int *receptive_field_distribution, int w, int h)
{
	int i, j, mod;
	double *small_kernel, *medium_kernel, *big_kernel;

	small_kernel = (double *) alloc_mem (w * sizeof(double));
	medium_kernel = (double *) alloc_mem (w * sizeof(double));
	big_kernel = (double *) alloc_mem (w * sizeof(double));

	density_function_cs (small_kernel, w, 50.0, 0.0, 0.5, 0.0, 0.75);
	density_function_cs (big_kernel, w,  55.0, pi, 0.3, 0.75, 0.65);

	for (i = 0; i < w; i++)
	{
		medium_kernel[i] = (1.0 - small_kernel[i] - big_kernel[i]);
		for (j = 0; j < h; j++)
	      	{
	        	mod = rand() % h;

	                if(mod < small_kernel[i] * h)
	        		receptive_field_distribution[i + w * j] = SMALL;
	                else
	                {       
	        	        if(mod < (small_kernel[i] + medium_kernel[i]) * h)
	        		        receptive_field_distribution[i + w * j] = MEDIUM;
	        	        else
	        		        receptive_field_distribution[i + w * j] = BIG;
	                }
	        }
	}
}

/*
*********************************************************************************
* Function: v1_filter								*
* Description: apply the V1 layer's filter at the neuron layer 			*
* Inputs:	filter_desc - Filter description				*
*********************************************************************************
*/

void
my_v1_filter (FILTER_DESC *filter_desc)
{
	RECEPTIVE_FIELD_DESCRIPTION *current_receptive_field;
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *n_l = NULL;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi, color_model;
	int cone_organization, concetrically_region;
	int x_center, y_center;
	GLubyte *image = NULL;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int distribution_type;
	double correction;
	double log_factor;
	

#ifdef	TEST_V1
	OUTPUT_DESC *output;
	OUTPUT_DESC *output2;
	
	output = get_output_by_name ("teste_out");
	clear_output_by_name ("teste_out");
	output2 = get_output_by_name ("teste2_out");
	clear_output_by_name ("teste2_out");
#endif

#ifdef	DEBUG_V1_FILTER_OUTPUT_COORDINATE
	if (debug_v1_filter_output_coordinate)
	{
		if ((v1_filter_out_coord_debug_file = fopen ("debug_v1_filter_output_coordinate.dat", "w")) == NULL)
			Erro ("Could not open debug_v1_filter_output_coordinate.dat", "", "");
	}
#endif
	previous_output.ival = 0;
	/* Check Neuron Layers */
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1) 
	{
		Erro ("Wrong number of neuron layers. V1 filter must be applied on one neuron layer.", "", "");
		return;
	}

	/* Check Params */
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 6) 
	{
		Erro ("Wrong number of parameters. V1 filter needs three parameters.", 
		"The organization of cone inputs, Concetrically organized regions, ","Color Model, Distribution Type and Log Factor respectivelly.");
		return;
	}

	cone_organization = filter_desc->filter_params->next->param.ival;
	concetrically_region = filter_desc->filter_params->next->next->param.ival;
	color_model = filter_desc->filter_params->next->next->next->param.ival;
	distribution_type = filter_desc->filter_params->next->next->next->next->param.ival;
	log_factor = filter_desc->filter_params->next->next->next->next->next->param.fval;
	n_l = filter_desc->neuron_layer_list->neuron_layer;
		
	if (log_factor <= 0.0)
	{
		if ( ((int) log_factor) != SERENO_MODEL)
		{
			Erro ("Log_factor can't be negative number.", "Log_factor can be SERENO_MODEL or Positive value.","");
			return;
		}
	}
	
	image = get_image_via_neuron_layer (n_l);
	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	if (filter_desc->private_state == NULL)
	{
		switch(distribution_type)
		{
			case CONSTANT_DISTRIBUTION:
				break;
			case STATISTIC_DISTRIBUTION:
				if (receptive_field_distribution == NULL)
				{
					receptive_field_distribution = (int *) alloc_mem (w * h * sizeof(int));
					compute_receptive_field_distribution (distribution_type, receptive_field_distribution, w, h);
				}
				break;
			case GAUSSIAN_DISTRIBUTION:
				break;
			default:
				Erro ("Distribution Type invalid.", "It can be CONSTANT_DISTRIBUTION, STATISTIC_DISTRIBUTION or GAUSSIAN_DISTRIBUTION.", "");
		}
		retina_model_compute_receptive_field_descriptor (filter_desc, wi, hi, w, h, distribution_type, log_factor);
	}

	input = get_input_by_neural_layer (n_l);

	previous_xi = -1;
	previous_yi = -1;
	
	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}
	
	correction = (double) h / (double) (h - 1);

	for (u = 0; u < w; u++) 
	{
		for (v = 0; v < h; v++) 
		{
			retina_model_map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, log_factor);
	
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
			{
				current_receptive_field = get_receptive_field ((RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state,
										u, v, w, distribution_type);
				previous_output = filter_desc->output->neuron_vector[(v * w) + u].output = 
					retina_model_receptive_field (current_receptive_field, input, image, 
					xi, yi, wi, hi, cone_organization, concetrically_region, color_model, 
					filter_desc->output->output_type);
/*				printf("%d %d %d\n",u, v, previous_output.ival);*/
			}
				
			previous_xi = xi;
			previous_yi = yi;
		}
#ifdef	DEBUG_V1_FILTER_OUTPUT_COORDINATE
		if (debug_v1_filter_output_coordinate)
			fprintf(v1_filter_out_coord_debug_file,"\n");
#endif
	}
		
#ifdef	DEBUG_V1_FILTER_OUTPUT_COORDINATE
	if (debug_v1_filter_output_coordinate)
	{
		debug_v1_filter_output_coordinate = 0;
		fclose(v1_filter_out_coord_debug_file);
	}
#endif

#ifdef	TEST2_V1
	for (v = 0; v < wi; v++) 
	{
		for (u = 0; u < hi; u++) 
		{
			xi = v; 
			yi = u;
			current_receptive_field = get_receptive_field ((RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state,
									u, w/2-1-current_kernel, w, distribution_type);
			output->neuron_layer->neuron_vector[(u * wi) + v].output = 
					retina_model_receptive_field (current_receptive_field, input, NULL, 
					xi, yi, wi, hi, cone_organization, concetrically_region, color_model, 
					filter_desc->output->output_type);
		}
	}
#endif
}



/*
*********************************************************************************
* Function: distorted2undistorted_image_filter  				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void 
distorted2undistorted_image_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int xi, yi, wi, hi, xo, yo, ho, wo, nl_number, p_number;
	double Xfd, Yfd, Xfu, Yfu;
	
	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	if (nl_number != 1)
	{
		Erro ("Error: Wrong number of neuron layers. The distortion2undistortion_image_filter must have only one input neuron layer.", "", "");
		return;
	}
	
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	
	// Gets the Image Dimentions
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;
	
	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: Wrong number of parameters. The distortion2undistortion_image_filter don't need parameters.", "", "");
		return;
	}

	// Gets the Filter Output 
	nl_output = filter_desc->output;

	// Gets the Image Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	// Gets the Camera Right Parameters
	cc = cameraRightCalibrationConstants;
	cp = cameraRightCalibrationParameters;
		
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			Xfu = (double) xo;
			Yfu = (double) yo;
			
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			
			xi = (int) (Xfd + 0.5);
			yi = (int) (Yfd + 0.5);
						
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.fval = 0.0;
		}		
	}
}
