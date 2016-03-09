#include "traffic_sign_filters.h"
#include "traffic_sign_user_functions.h"

void traffic_sign_crop (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int xi, yi, wi, hi, xo, yo, wo, ho;
	int index_out, index_inp, index_sign;
	float corner0_x, corner0_y, corner1_x, corner1_y,dx,dy,delta,size_bounding_box;
	
	xi=yi=0;
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	// Gets the Filter Output 
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	index_sign = g_curr_sign;
	if (g_number_of_boxes > 0)
	{
		//squarify:
		dx = g_corner1_x[index_sign]-g_corner0_x[index_sign]; 
		dy = g_corner1_y[index_sign]-g_corner0_y[index_sign];
		if (dx > dy) // retangulo deitado
		{
			corner0_x = g_corner0_x[index_sign];
			corner0_y = g_corner0_y[index_sign] - (dx-dy)/2.0f;
			corner1_x = g_corner1_x[index_sign];
			corner1_y = g_corner1_y[index_sign] + (dx-dy)/2.0f;
			if (corner0_y < 0)
			{
				corner0_y = 0;
				corner1_y = g_corner1_y[index_sign] + (dx-dy);
			}
			if (corner1_y >= hi)
			{
				corner1_y = hi-1;
				corner0_y = g_corner0_y[index_sign] - (dx-dy);
			}
		}
		else //retangulo em pe
		{
			corner0_x = g_corner0_x[index_sign] - (dy-dx)/2.0f;
			corner0_y = g_corner0_y[index_sign];
			corner1_x = g_corner1_x[index_sign] + (dy-dx)/2.0f;
			corner1_y = g_corner1_y[index_sign];
			if (corner0_x < 0)
			{
				corner0_x = 0;
				corner1_x = g_corner1_x[index_sign] + (dy-dx);
			}
			if (corner1_x >= wi)
			{
				corner1_x = wi-1;
				corner0_x = g_corner0_x[index_sign] - (dy-dx);
			}
		}
		//delta para bounding boxes quadradas e ho == wo
		size_bounding_box = corner1_y - corner0_y;
		delta = size_bounding_box/ho;
		for (yo = 0; yo < ho; yo++)
		{
			for (xo = 0; xo < wo; xo++)
			{
				xi = ((int)(corner0_x + delta*xo)); //strech weight
				yi = hi - ((int) corner0_y) - delta*yo; //strech height
				
				index_out = xo + (ho-yo)*wo;
				index_inp = xi + yi*wi;
				
				/**/
				if ((xi < (int)corner1_x) && ((hi-yi) < (int)corner1_y) )
				{
					nl_output->neuron_vector[index_out].output = nl_input->neuron_vector[index_inp].output;	
				}
				else
				{nl_output->neuron_vector[xo + (ho-yo)*wo].output.ival = 0;}
				/**/		
			}
		}
	}
	else //no sign, fill input with black pixels
	{
		for (yo = 0; yo < ho; yo++) { for (xo = 0; xo < wo; xo++) {nl_output->neuron_vector[xo + (ho-yo)*wo].output.ival = 0;}}
	}	
}

