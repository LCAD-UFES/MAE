#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#include "../xor_rbm.h"
#include "rbm.h"

#define READ	0
#define TRAIN	1
#define	TEST	2

double samples[] = {0.0, 1.0, 1.0, 0.0};

int g_curr_sample = -1;
int g_state = -1;

void
forward_network()
{
   	all_filters_update ();
	all_dendrites_update (); 
	all_neurons_update ();
	all_outputs_update (); 
}


void 
f_keyboard(char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
	}
}


int 
init_user_functions()
{
	srand(time(NULL));
	
	toggle_draw_active();
	toggle_move_active();

	return (0);
}


void
read_char_input(INPUT_DESC *input, int direction)
{
	if (direction == -1)
	{
		if (g_curr_sample == 0)
			g_curr_sample = NEURON_MEMORY_SIZE - 1;
		else
			g_curr_sample--;
	}
	else if (direction == 1)
	{
		if (g_curr_sample < (NEURON_MEMORY_SIZE - 1))
			g_curr_sample++;
		else
			g_curr_sample = 0;
	}
	
	for (int y = 0; y < input->wh; y++)
		for (int x = 0; x < input->ww; x++)
			input->neuron_layer->neuron_vector[y * input->ww + x].output.fval = samples[g_curr_sample * input->ww + x];
	
}


void
get_new_data(INPUT_DESC *input, int direction)
{
	read_char_input(input, direction);

	check_input_bounds(input, input->wx + input->ww / 2, input->wy + input->wh / 2);

	update_input_image(input);

#ifndef NO_INTERFACE
	glutSetWindow(input->win);
	glBindTexture(GL_TEXTURE_2D,(GLuint) input->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
	glutPostWindowRedisplay(input->win);
#endif
}


void 
make_input_image_xor(INPUT_DESC *input)
{
	int i;
	char message[256];
	int w, h;
	
	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;
	
	// Computes the input image dimentions
	input->tfw = nearest_power_of_2(w);
	input->tfh = nearest_power_of_2(h);

	// Saves the image dimentions
	input->ww = w;
	input->wh = h;
	
	// Computes the input image dimentions
	input->tfw = nearest_power_of_2(w);
	input->tfh = nearest_power_of_2(h);

	switch (TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vpw = input->ww;
			input->vph = input->wh;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro((char *) "Invalid Type Show ", message, (char *) " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = w - input->vph;
			
	if (input->image == (GLubyte *) NULL)
	{
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof(GLubyte));

		for (i = 0; i < input->tfh * input->tfw * 3; i++)
			input->image[i] = 0;
	}		

	return;
}


void 
init_data_input(INPUT_DESC *input)
{
#ifndef	NO_INTERFACE
	int x, y;
	float f;
#endif

	make_input_image_xor(input);
	
#ifndef	NO_INTERFACE
	f = 1.0;
	while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
		f += 1.0;
	while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
		f *= 0.9;
	glutInitWindowSize ((int) ((float)input->ww * f), (int) ((float)input->wh * f));

	if (read_window_position(input->name, &x, &y))
		glutInitWindowPosition(x, y);
	else
		glutInitWindowPosition(-1, -1);

	input->win = glutCreateWindow(input->name);
	glGenTextures(1,(GLuint *)(& (input->tex)));
	input_init(input);
	glutReshapeFunc(input_reshape);
	glutDisplayFunc(input_display);
	glutMouseFunc(input_mouse);
	glutPassiveMotionFunc(input_passive_motion);
	glutKeyboardFunc(keyboard);
#endif

}


void
input_controler(INPUT_DESC *input, int status)
{
	char command[256];
	
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	   (input->mouse_state == GLUT_DOWN) &&
	   (draw_active == 1))
	{
		sprintf(command, "draw nl_rbm_out based on input move;");
		interpreter(command);
	}

	if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	   (input->mouse_state == GLUT_DOWN) &&
	   (move_active == 1))
	{
		sprintf(command, "move data to %d, %d;", input->wxd, input->wyd);
		interpreter(command);
	}

	input->mouse_state = -1;
}


void
input_generator(INPUT_DESC *input, int status)
{
	static int flag = 0;
		
	if (flag == 0)
	{
		init_data_input(input);
		flag = 1;
	}
	
	if (status == MOVE)
	{
		if (input->wx >= 0)
			get_new_data(input, 1);
		else
			get_new_data(input, -1);

		all_filters_update();
	}
}


void
draw_output(char *output_name, char *input_name)
{
}


void
output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
}


void
rbm_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	int nl_number, p_number;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int wo, ho, xo, yo;
	int wi, hi, xi, yi;

	// Checks the number of input Neuron Layers
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the number of Parameters
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	if (filter_desc->private_state == NULL)
	{
		//BoltzmannMachine *network = new BoltzmannMachine(2, 4); //Big net 2
		BoltzmannMachine *network = new BoltzmannMachine(2, 1); //Small net
		filter_desc->private_state = (void *) network;
	}

	if (g_state == READ)
	{
		double data[2];

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				data[xi + yi * wi] = nl_input->neuron_vector[xi + yi * wi].output.fval;

		BoltzmannMachine *network = (BoltzmannMachine *)filter_desc->private_state;
		//network->clear_data();
		network->read_data(data, 2);
	}
	else if (g_state == TRAIN)
	{
		BoltzmannMachine *network = (BoltzmannMachine *)filter_desc->private_state;
		network->train_network();
	}
	else if (g_state == TEST)
	{
		BoltzmannMachine *network = (BoltzmannMachine *)filter_desc->private_state;
		network->test_network(1);
		network->print_synapses();
		network->print_biases();

		for (yo = 0; yo < ho; yo++)
			for (xo = 0; xo < wo; xo++)
				nl_output->neuron_vector[xo + yo * wo].output.fval = (float) network->get_neuron(xo + yo * wo);
	}	
}


/*
***********************************************************
* Function: SetNetworkStatus
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT SetNetworkState(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_state = pParamList->next->param.ival;
	
	output.ival = 0;
	return (output);
}
