#include <stdlib.h>
#include "mae.h"
#include "subpixel_disparity.h"
#include "../test_filter.h"


void
add_subpixel_disparity_to_disparity_map_plan (void)
{
	int i, n = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;
	float subpixel_disparity;

	for (i = 0; i < n; i++)
	{
		subpixel_disparity = nl_subpixel_disparity_map.neuron_vector[i].output.fval * 2.0;
		if (fabs (subpixel_disparity) < 1.5)
			nl_disparity_map.neuron_vector[i].output.fval += subpixel_disparity;
	}
			
}


void
generate_subpixel_disparity_map_plan (void)
{
	robot_mt_cell_subpixel_disparity (&nl_subpixel_disparity_map, &nl_disparity_map,
					  &simple_cell_nl_1,
					  &simple_cell_nl_2,
					  &simple_cell_nl_3,
					  &simple_cell_nl_4,
					  &simple_cell_nl_5,
					  &simple_cell_nl_6,
					  &simple_cell_nl_7,
					  &simple_cell_nl_8,
					  PHI_L1, PHI_R1, PHI_L2, PHI_R2, WAVELENGTH);
}


void
draw_output (char *output_name, char *input_name)
{
}


void
input_controler (INPUT_DESC *input, int status)
{
}


int
init_user_functions ()
{
	return (0);
}

void
input_generator (INPUT_DESC *input, int status)
{
	int x, y;
	OUTPUT_DESC *out = NULL;
	
	if (input->win == 0)
	{
		float f;

		make_input_image (input);
		input->up2date = 0;

		f = 1.0;
		while ((((float)input->tfw * f) < 128.0) || (((float)input->tfh * f) < 128.0))
			f += 1.0;
		while ((((float)input->tfw * f) > 1024.0) || (((float)input->tfh * f) > 1024.0))
			f *= 0.9;
		glutInitWindowSize ((int) ((float)input->tfw * f), (int) ((float)input->tfh * f));
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (input_display);
		glutMouseFunc (input_mouse);
		glutPassiveMotionFunc (input_passive_motion);
		glutKeyboardFunc (keyboard);

       		out = input->input_generator_params->param.output;
	}

	check_input_bounds (input, input->wx + input->neuron_layer->dimentions.x / 2, 
				   input->wy + input->neuron_layer->dimentions.y / 2);
	input->up2date = 0;
	update_input_neurons (input);
	glutPostWindowRedisplay (input->win);
	if ((out != NULL) && (out->win != 0))
	{
		update_output_image (out);
		glutPostWindowRedisplay (out->win);
	}
}


void
set_disparity ()
{
	int ho, wo, yo, xo;
	int loy, lox, hiy, hix;

	ho = nl_disparity_map.dimentions.y;
	wo = nl_disparity_map.dimentions.x;

	lox = 200;
	loy = 132;
	hix = lox + 400;
	hiy = loy + 400;
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			if ((xo > lox) && (xo < hix) &&
			    (yo > loy) && (yo < hiy))
				nl_disparity_map.neuron_vector[(yo*wo) + xo].output.fval = -1.0;
			else
				nl_disparity_map.neuron_vector[(yo*wo) + xo].output.fval = 0.0;
		}
	}

}


void 
filter_group_update (char *prefix)
{
	FILTER_LIST *f_list;
	int length = strlen(prefix);
	for (f_list = global_filter_list; f_list != NULL; f_list = f_list->next)
	{
		if (strncmp(f_list->filter_descriptor->name, prefix, length) == 0)
			filter_update (f_list->filter_descriptor);
	}	
}

void
compute_disparity_map (char *mt_filter, char *mt_gaussian_filter, char *disparity_map_filter)
{
	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_INIT;
	filter_group_update (disparity_map_filter);
	
	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_SAMPLE;
	for (DISPARITY = MIN_DISP; DISPARITY < MAX_DISP; DISPARITY++)
	{
		filter_group_update (mt_filter);
		filter_group_update (mt_gaussian_filter);
		filter_group_update (disparity_map_filter);
	}
	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_OPTIMIZE;
	filter_group_update (disparity_map_filter);
}


void 
f_keyboard (char *key_value)
{
	char key = key_value[0];
	
	switch (key)
	{
		case 's':
			generate_subpixel_disparity_map_plan ();

			add_subpixel_disparity_to_disparity_map_plan();

			all_outputs_update();
		break;
		case 'D':
			load_image_to_object ("nl_disparity_map_out", "../robot_smv_12.0/fomento12_0_disparity_map.ppm");
			//set_disparity ();
		break;
		case 'd':
			compute_disparity_map ("nl_mt", "nl_mt_gaussian", "nl_disparity");
		break;
	}
}

