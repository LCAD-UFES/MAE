#include "traffic_sign_filters.h"
#include "traffic_sign_user_functions.h"

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
				
// 				output_update(&out_traffic_sign);
//                 printf("%d (%d %d) (%d %d)\n",nl_output->neuron_vector[x*out_width + y].output.ival,x,y,xi,yi);
//                 getchar();
			}
		}
	}
	
	output_update(&out_traffic_sign);
}

//***********************************************************
//* Function: traffic_sign_resize_bilinear
//* Description: resize an image with bilinear interpolation
//***********************************************************

int linear_interpolation(int v1, int v2, float ratio)
{
	return (int)(v1*(1-ratio) + v2*ratio);
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
	
	//if (xi+1 >= input_width)
	//	p1 = nl_input->neuron_vector[xi*input_width + yi].output;
	//else
		p1 = nl_input->neuron_vector[(xi+1)*input_width + yi].output;
		
	//if ((xi+1 >= input_width) || (yi+1 >= input_height))
// 		p2 = nl_input->neuron_vector[xi*input_width + yi].output;
// 	else
		p2 = nl_input->neuron_vector[(xi+1)*input_width + (yi+1)].output;
	
// 	if  (yi+1 >= input_height)
// 		p3 = nl_input->neuron_vector[xi*input_width + yi].output;
// 	else
		p3 = nl_input->neuron_vector[xi*input_width + (yi+1)].output;
	
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

void traffic_sign_resize_bilinear (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int out_height, out_width, input_width, input_height;
    int x, y;
    float u, v;
	float delta;
	
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	nl_output = filter_desc->output;
	
	input_width = nl_input->dimentions.x;
	input_height = nl_input->dimentions.y;
	assert(input_height == input_width);
	
	out_width = nl_output->dimentions.x;
	out_height = nl_output->dimentions.y;
	
    delta = (float)g_size_box/(float)out_width; 
    
	if (delta)
	{
		for (y = 0; y < out_height; y++)
		{
			for (x = 0; x <= out_width; x++)
			{
				u = x * delta;
				v = y * delta;

                nl_output->neuron_vector[x*out_width + y].output.ival = get_bilinear_output(nl_input, u, v);
            }
        }
    }
    
    output_update(&out_traffic_sign);
    return;
}









