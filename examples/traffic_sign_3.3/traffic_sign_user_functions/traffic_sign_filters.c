#include "traffic_sign_filters.h"
#include "traffic_sign_user_functions.h"
#include "traffic_sign_utils.h"

//***********************************************************
//* Function: traffic_sign_resize_NN
//* Description: resize an image with nearest neighbor scaling
//***********************************************************
void traffic_sign_resize_NN (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int x, y, offset, xi, yi;
	int out_height, out_width, input_width, input_height;
	float delta;
	
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	nl_output = filter_desc->output;
	
	input_width = nl_input->dimentions.x;
	input_height = nl_input->dimentions.y;
	
	out_width = nl_output->dimentions.x;
	out_height = nl_output->dimentions.y;
	
	offset = input_height/2 - g_size_box/2 + 1;
	
	delta = (float)g_size_box/(float)out_width;	
	
	if (delta)
	{
		for (y = 0; y < out_height; y++)
		{
			for (x = 0; x <= out_width; x++)
			{
				xi = (int)(x*delta) + offset;
				yi = (int)(y*delta) + offset;
				nl_output->neuron_vector[x*out_width + y].output = nl_input->neuron_vector[xi*input_width + yi].output; //rgb
			}
		}
	}
	
	output_update(&out_traffic_sign_gaussian);
}

//***********************************************************
//* Function: traffic_sign_resize_bilinear
//* Description: resize an image with bilinear interpolation
//***********************************************************

int 
linear_interpolation(int v1, int v2, float ratio)
{
	int interpolation;
	
	interpolation = (int) ((double) v1 * (1.0 - ratio) + v2 * ratio);
	if (interpolation > 255)
		interpolation = 255;
	if (interpolation < 0)
		interpolation = 0;
		
	return (interpolation);
}

int get_bilinear_output(NEURON_LAYER * nl_input, float u, float v)
{
	int xi, yi;
	float offs_x, offs_y;
	int input_width, input_height;
	int offset;
	int r, g, b;
	int r0, g0, b0;
	int r1, g1, b1;
	int r2, g2, b2;
	int r3, g3, b3;
	
	NEURON_OUTPUT p0, p1, p2, p3;
    
	xi = (int) u;
	yi = (int) v;
	
	offs_x = u - xi;
	offs_y = v - yi;
	
	input_width = nl_input->dimentions.x;
	input_height = nl_input->dimentions.y;
    
	offset = input_height/2 - g_size_box/2;
	xi += offset + 1;
	yi += offset;
	
	p0 = nl_input->neuron_vector[xi*input_width + yi].output;
	
	p1 = nl_input->neuron_vector[(xi+1)*input_width + yi].output;
	if (p1.ival == 0)
		p1 = p0;
	
	p2 = nl_input->neuron_vector[(xi+1)*input_width + (yi+1)].output;
	if (p2.ival == 0)
		p2 = p0;
	
	p3 = nl_input->neuron_vector[xi*input_width + (yi+1)].output;
	if (p3.ival == 0)
		p3 = p0;
	
	if (p0.ival == 0)
		p3 = p2 = p1 = p0;
	
	r0 = RED (p0.ival);
	g0 = GREEN (p0.ival);
	b0 = BLUE (p0.ival);

	r1 = RED (p1.ival);
	g1 = GREEN (p1.ival);
	b1 = BLUE (p1.ival);

	r2 = RED (p2.ival);
	g2 = GREEN (p2.ival);
	b2 = BLUE (p2.ival);

	r3 = RED (p3.ival);
	g3 = GREEN (p3.ival);
	b3 = BLUE (p3.ival);
	
	r = linear_interpolation(linear_interpolation(r0, r1, offs_x), linear_interpolation(r3, r2, offs_x), offs_y);
	g = linear_interpolation(linear_interpolation(g0, g1, offs_x), linear_interpolation(g3, g2, offs_x), offs_y);
	b = linear_interpolation(linear_interpolation(b0, b1, offs_x), linear_interpolation(b3, b2, offs_x), offs_y);
    
	return (PIXEL (r, g, b));
}

void traffic_sign_reshape_bilinear (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	double rotation_angle_radians;
	int xo, yo, wo, ho;
	double x_center, y_center;
	double cos_alpha, sin_alpha, xt, yt, xr, yr, xs, ys, k, input_output_resize;
	
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	nl_output = filter_desc->output;
	
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	rotation_angle_radians = mae_degrees_to_radians(reshape_filter_rotation_angle);

	cos_alpha = cos(rotation_angle_radians);
	sin_alpha = sin(rotation_angle_radians);

	x_center = (double) wo / 2.0;
	y_center = (double) ho / 2.0;

	input_output_resize = ((double) g_size_box / (double) wo);
	
	k = 1.0 / reshape_filter_scale_factor;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			xt = (double) xo - x_center;
			yt = (double) yo - y_center;

			xr =  cos_alpha * xt + sin_alpha * yt;
			yr = -sin_alpha * xt + cos_alpha * yt;

			xs = k * xr;
			ys = k * yr;
			
			xt = input_output_resize * (xs + x_center + reshape_filter_offset_x);
			yt = input_output_resize * (ys + y_center + reshape_filter_offset_y);

			nl_output->neuron_vector[xo * wo + yo].output.ival = get_bilinear_output(nl_input, xt, yt);
		}
	}
	return;
}

/*
*********************************************************************************
* Function: traffic_sign_reshape	              				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void traffic_sign_reshape (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	double x_center, y_center;
	double rotation_angle_radians;
	int xi, yi, wi, hi, xo, yo, wo, ho;
	double cos_alpha, sin_alpha, xt, yt, xr, yr, xs, ys, k;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	rotation_angle_radians = mae_degrees_to_radians(reshape_filter_rotation_angle);

	cos_alpha = cos(rotation_angle_radians);
	sin_alpha = sin(rotation_angle_radians);

	// printf ("Rotation angle = %f\n", reshape_filter_rotation_angle);

	k = 1.0/reshape_filter_scale_factor;

	// printf ("Scale factor = %f\n", reshape_filter_scale_factor);

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	x_center = (double) wo / 2.0;
	y_center = (double) ho / 2.0;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			xt = (double) xo - x_center;
			yt = (double) yo - y_center;

			xr =  cos_alpha * xt + sin_alpha * yt;
			yr = -sin_alpha * xt + cos_alpha * yt;

			xs = k * xr;
			ys = k * yr;

			xt = xs + x_center - reshape_filter_offset_x;
			yt = ys + y_center - reshape_filter_offset_y;

			xi = (int) (xt + .5);
			yi = (int) (yt + .5);

			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;

		}
	}
}

