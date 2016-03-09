#include <stdlib.h>
#include "mae.h"


/*
Neste exemplo, neste teste, faremos o seguinte:
	1) verificamos quem está mais balanceado (left_minus_right e right_minus_left), será o dominante
	2) apagamos alguns pixels da imagem de controle dominante (para que fique balanceado).
	3) e apagamos a imagem de controle não-dominante.
*/

void 
f_keyboard (char *key_value)
{
}

void
draw_output (char *output_name, char *input_name)
{
	float delta;
	int num_neurons, ny_saved;
	OUTPUT_DESC *output;
	INPUT_DESC *input;
	
	output = get_output_by_name (output_name);
	input = get_input_by_name (input_name);
	
	set_neurons (output->neuron_layer->neuron_vector, 0, output->neuron_layer->dimentions.x * output->neuron_layer->dimentions.y, 0);
	
	delta = (float) (wx_saved - input->wx) / (float) (input->vpw - input->neuron_layer->dimentions.x);
	
	if (delta < 0.0)
	{
		ny_saved = 0;
		delta = -delta;
	}
	else
		ny_saved = output->neuron_layer->dimentions.x / 4;
		
	if (delta > 1.0)
		delta = 1.0;

	num_neurons = (int)(((float)(output->neuron_layer->dimentions.x * output->neuron_layer->dimentions.y) / 4.0) * delta);
	set_neurons (output->neuron_layer->neuron_vector, ny_saved * output->neuron_layer->dimentions.x, num_neurons, NUM_COLORS - 1);

	delta = (float) (wy_saved - input->wy) / (float) (input->vph - input->neuron_layer->dimentions.y);
	
	if (delta < 0.0)
	{
		ny_saved = output->neuron_layer->dimentions.y / 2;
		delta = -delta;
	}
	else
		ny_saved = 3 * (output->neuron_layer->dimentions.y / 4);
	if (delta > 1.0)
		delta = 1.0;

	num_neurons = (int)(((float)(output->neuron_layer->dimentions.x * output->neuron_layer->dimentions.y) / 4.0) * delta);
	set_neurons (output->neuron_layer->neuron_vector, ny_saved * output->neuron_layer->dimentions.x, num_neurons, NUM_COLORS - 1);

	update_output_image (output);
	glutPostWindowRedisplay (output->win);
}


void
input_controler (INPUT_DESC *input, int status)
{
	static OUTPUT_DESC *control_out = NULL, *cortex_out = NULL, *image_out = NULL;
	char command[256];
	static int pass = 0;
	
	if (control_out == NULL)
	{		
		control_out = input->input_controler_params->next->param.output;
		cortex_out  = input->input_controler_params->next->next->param.output;
		image_out   = input->input_controler_params->next->next->next->param.output;
	}
	
	if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_UP) && (draw_active == 0))
	{
		sprintf (command, "copy %s;", input->name);
		interpreter (command);
		sprintf (command, "paste %s;", image_out->name);
		interpreter (command);
		input->mouse_state = 100;
	}
	else if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    	 (input->mouse_state == GLUT_UP) && (draw_active == 1))
	{
		sprintf (command, "draw %s based on %s move;", control_out->name, input->name);
		interpreter (command);
		sprintf (command, "pop position of %s;", input->name);
		interpreter (command);
		interpreter ("toggle draw_active;");
		interpreter ("train network;");
		input->mouse_state = 100;
	}
	else if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
		 (input->mouse_state == GLUT_DOWN) && (draw_active == 0))
	{
		sprintf (command, "push position of %s;", input->name);
		interpreter (command);
		sprintf (command, "clear %s;", control_out->name);
		interpreter (command);
		sprintf (command, "copy %s;", input->name);
		interpreter (command);
		sprintf (command, "paste %s;", cortex_out->name);
		interpreter (command);
		sprintf (command, "paste %s;", cortex_out->name);
		interpreter (command);
		interpreter ("toggle draw_active;");
		input->mouse_state = 100;
	}
	else if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    	 (input->mouse_state == GLUT_DOWN) &&
		 (move_active == 1))
	{
		sprintf (command, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (command);
		input->mouse_state = 100;
	}

	if (status == RUN)
	{
		static float delta_x_previous = 0.0, delta_y_previous = 0.0;
		float var_x_perc, var_y_perc, delta_x, delta_y;
		static float uncertainty = 0.0;
		int num_neurons;
	
		num_neurons = (int)((float)(control_out->neuron_layer->dimentions.x * control_out->neuron_layer->dimentions.y) / 4.0);
		delta_x = (float) number_neurons_on (control_out->neuron_layer->neuron_vector, 0, num_neurons) -
			number_neurons_on (control_out->neuron_layer->neuron_vector, (control_out->neuron_layer->dimentions.y / 4) * control_out->neuron_layer->dimentions.x, num_neurons);
		delta_y = (float) number_neurons_on (control_out->neuron_layer->neuron_vector, (control_out->neuron_layer->dimentions.y / 2) * control_out->neuron_layer->dimentions.x, num_neurons) -
			number_neurons_on (control_out->neuron_layer->neuron_vector, (3 * (control_out->neuron_layer->dimentions.y / 4)) * control_out->neuron_layer->dimentions.x, num_neurons);

		var_x_perc = fabs(delta_x - delta_x_previous) / ((float) num_neurons); 
		var_y_perc = fabs(delta_y - delta_y_previous) / ((float) num_neurons); 
		uncertainty *= 0.35; /* 0.25 = decay rate */
		uncertainty += var_x_perc + var_y_perc;
		if (((pass > 3) && (uncertainty < 0.02)) || (pass > 20))
		{
			input->wx += (int) ((float) (input->vpw - input->neuron_layer->dimentions.x) * ((float) delta_x / (float) num_neurons));
			input->wy += (int) ((float) (input->vph - input->neuron_layer->dimentions.y) * ((float) delta_y / (float) num_neurons));
			printf ("move %s to %d, %d;\n", input->name, input->wxd, input->wyd);
			check_input_bounds (input, (input->wx + input->neuron_layer->dimentions.x / 2), (input->wy + input->neuron_layer->dimentions.y / 2));
			glutPostWindowRedisplay (input->win);
			input->wxd = input->wx + input->neuron_layer->dimentions.x / 2;
			input->wyd = input->wy + input->neuron_layer->dimentions.y / 2;
			pass = 0;
		}
		else
			pass++;
		
		delta_x_previous = delta_x;
		delta_y_previous = delta_y;
	}		
}


int
init_user_functions ()
{
	return (0);
}

void
input_generator (INPUT_DESC *input, int status)
{
	static int flag = 0;
	int x, y;
	OUTPUT_DESC *out = NULL;
	
	if (flag == 0)
	{
		float f;

		make_input_image (input);
		input->up2date = 0;

		flag = 1;

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

       		out = input->input_generator_params->next->param.output;
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

