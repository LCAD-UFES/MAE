#include <stdlib.h>
#include "mae.h"


/* This function is only used if not defined by the user 
int
init_user_functions (void)
{
	return (0);
}
*/


int
number_neurons_on (NEURON *n, int n0, int num_neurons)
{
	int i, value;
	
	for (i = value = 0; i < num_neurons; i++)
	{
		if (n[i+n0].output.ival != 0) 
			value += 1;
	}
	return (value);
}


int
number_neurons_on_v (NEURON *n, int n0, int num_neurons)
{
	int i, value, n_valid;
	
	for (i = value = 0; i < num_neurons; i++)
	{
		if (i == 0)
		{
			n_valid = 2;
			if (n[i+n0].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+2].output.ival != 0) 
				n_valid += 1;
			
		}
		else if (i == 1)
		{
			n_valid = 1;
			if (n[i+n0-1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+2].output.ival != 0) 
				n_valid += 1;
		}
		else if (i == (num_neurons - 2))
		{
			n_valid = 1;
			if (n[i+n0-2].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0-1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+1].output.ival != 0) 
				n_valid += 1;
		}
		else if (i == (num_neurons - 1))
		{
			n_valid = 2;
			if (n[i+n0-2].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0-1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0].output.ival != 0) 
				n_valid += 1;
		}
		else
		{
			n_valid = 0;
			if (n[i+n0-2].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0-1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+1].output.ival != 0) 
				n_valid += 1;
			if (n[i+n0+2].output.ival != 0) 
				n_valid += 1;
		}
			
		if (n_valid >= 3) 
			value += 1;
	}
	return (value);
}


int
number_neuronsx_on (NEURON *n, int x0, int nx, int ny, int num_neurons)
{
	int i, x, y, value, n_valid;
	
	x = x0;
	for (i = value = y = 0; i < num_neurons; i++)
	{
		if (y == 0)
		{
			n_valid = 2;
			if (n[x + y*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+2)*nx].output.ival != 0) 
				n_valid += 1;
			
		}
		else if (y == 1)
		{
			n_valid = 1;
			if (n[x + (y-1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + y*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+2)*nx].output.ival != 0) 
				n_valid += 1;
		}
		else if (y == (ny - 2))
		{
			n_valid = 1;
			if (n[x + (y-2)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y-1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + y*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+1)*nx].output.ival != 0) 
				n_valid += 1;
		}
		else if (y == (ny - 1))
		{
			n_valid = 2;
			if (n[x + (y-2)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y-1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + y*nx].output.ival != 0) 
				n_valid += 1;
		}
		else
		{
			n_valid = 0;
			if (n[x + (y-2)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y-1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + y*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+1)*nx].output.ival != 0) 
				n_valid += 1;
			if (n[x + (y+2)*nx].output.ival != 0) 
				n_valid += 1;
		}
			
		y++;
		if (y == ny)
		{
			y = 0;
			x++;
		}
		
		if (n_valid >= 3) 
			value += 1;
	}
	return (value);
}


void
draw_x_output (char *output_name, char *input_name)
{
	float delta;
	int num_neurons, right;
	OUTPUT_DESC *output;
	INPUT_DESC *input;
	
	output = get_output_by_name (output_name);
	input = get_input_by_name (input_name);
	
	set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
	
	delta = (float) (input->wx - wx_saved) / (float) (input->ww/2);
	if (delta < 0.0)
	{
		right = 0;
		delta = -delta;
	}
	else
		right = 1;
	if (delta > 1.0)
		delta = 1.0;
		
	num_neurons = (int)(((float)(output->ww * output->wh) / 2.0) * delta);

	setx_neurons (output->neuron_layer->neuron_vector, right, output->ww, output->wh, num_neurons, NUM_COLORS-1);

	update_output_image (output);
#ifndef NO_INTERFACE
	glutPostWindowRedisplay (output->win);
#endif
}



void
draw_y_output (char *output_name, char *input_name)
{
	float delta;
	int num_neurons, up;
	OUTPUT_DESC *output;
	INPUT_DESC *input;
	
	output = get_output_by_name (output_name);
	input = get_input_by_name (input_name);
	
	set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
	
	delta = (float) (input->wy - wy_saved) / (float) (input->wh/2);
	if (delta < 0.0)
	{
		up = 0;
		delta = -delta;
	}
	else
		up = 1;
	if (delta > 1.0)
		delta = 1.0;

	num_neurons = (int)(((float)(output->ww * output->wh) / 2.0) * delta);

	sety_neurons (output->neuron_layer->neuron_vector, up, output->wh, output->ww, num_neurons, NUM_COLORS-1);
	
	update_output_image (output);
#ifndef NO_INTERFACE
	glutPostWindowRedisplay (output->win);
#endif
}


void
update_position_x (OUTPUT_DESC *out, INPUT_DESC *input)
{
	int num_neurons;
	int delta_x;
	
	num_neurons = (int)((float)(out->ww * out->wh) / 2.0);
	
	delta_x = number_neuronsx_on (out->neuron_layer->neuron_vector, out->ww/2, out->ww, out->wh, num_neurons) -
		  number_neuronsx_on (out->neuron_layer->neuron_vector, 0, out->ww, out->wh, num_neurons);

	input->wx += (int) ((float) (input->ww/2) * ((float) delta_x / (float) num_neurons));

	check_input_bounds (input, (input->wx + input->ww / 2), (input->wy + input->wh / 2));

}


void
update_position_y (OUTPUT_DESC *out, INPUT_DESC *input)
{
	int num_neurons;
	int delta_y;
	
	num_neurons = (int)((float)(out->ww * out->wh) / 2.0);

	delta_y = number_neurons_on (out->neuron_layer->neuron_vector, (out->wh / 2) * out->ww, num_neurons) -
		  number_neurons_on (out->neuron_layer->neuron_vector, 0, num_neurons);

	input->wy += (int) ((float) (input->wh/2) * ((float) delta_y / (float) num_neurons));

	check_input_bounds (input, (input->wx + input->ww / 2), (input->wy + input->wh / 2));
}


void
update_position (OUTPUT_DESC *out, INPUT_DESC *input)
{
	int num_neurons;
	int delta_x, delta_y;
	
	num_neurons = (int)((float)(out->ww * out->wh) / 4.0);
	
	delta_x = number_neurons_on (out->neuron_layer->neuron_vector, 0, num_neurons) -
		  number_neurons_on (out->neuron_layer->neuron_vector, (out->wh / 4) * out->ww, num_neurons);
	delta_y = number_neurons_on (out->neuron_layer->neuron_vector, (out->wh / 2) * out->ww, num_neurons) -
		  number_neurons_on (out->neuron_layer->neuron_vector, (3 * (out->wh / 4)) * out->ww, num_neurons);
	
	input->wx += (int) ((float) (input->vpw - input->ww) * ((float) delta_x / (float) num_neurons));
	input->wy += (int) ((float) (input->vph - input->wh) * ((float) delta_y / (float) num_neurons));

	check_input_bounds (input, (input->wx + input->ww / 2), (input->wy + input->wh / 2));
}

