#include <mae.h>
#include "../stereo.h"

double g_scale_factor = 1.0;

int *disparity = NULL;
int *h_distance = NULL;

int
init_user_functions()
{
	return (0);
}


void
set_neuron_layer (NEURON_LAYER *neuron_layer, int activation_value)
{
	int u, v, w, h;

	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;

	for (u = 0; u < w; u++)
		for (v = 0;v < h; v++)
			neuron_layer->neuron_vector[u + w * v].output.ival = activation_value;
}



void 
f_keyboard(char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
		case 'S':
		case 's':
			shift_filter_deltaX++;
			break;
		case 'W':
		case 'w':
			set_neuron_layer (&nl_disparity_map_level0, shift_filter_deltaX);
			set_neuron_layer (&nl_disparity_map_level1, shift_filter_deltaX);
			set_neuron_layer (&nl_disparity_map_level2, shift_filter_deltaX);
			set_neuron_layer (&nl_disparity_map_level3, shift_filter_deltaX);
			set_neuron_layer (&nl_disparity_map_level4, shift_filter_deltaX);
			break;
		case 'C':
		case 'c':
			shift_filter_deltaX = 0;
			if (forward_image_left)
				forward_image_left = 0;
			else
				forward_image_left = 1;
			break;

	}
}


void 
draw_output(char *output_name, char *input_name)
{		
	return;
}


void 
make_input_image_stereo(INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	input->ww = w;
	input->wh = h;

	switch (TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vph = h;
			input->vpw = w;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if (input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}


void 
LoadImage(INPUT_DESC *input, char *strFileName)
{
	load_input_image(input, strFileName);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons(input);
	update_input_image(input);
}


void 
init_stereo(INPUT_DESC *input)
{
#ifndef	NO_INTERFACE
	int x, y;
#endif

	make_input_image_stereo(input, IMAGE_WIDTH, IMAGE_HEIGHT);

#ifndef	NO_INTERFACE
	glutInitWindowSize(input->ww, input->wh);
	if (read_window_position(input->name, &x, &y))
		glutInitWindowPosition(x, y);
	else
		glutInitWindowPosition(-1, -1);
	input->win = glutCreateWindow(input->name);

	glGenTextures(1, (GLuint *)(&(input->tex)));
	input_init(input);
	glutReshapeFunc(input_reshape);
	glutDisplayFunc(input_display);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(input_passive_motion);
	glutMouseFunc(input_mouse);
#endif
	if (strcmp(input->name, "image_left") == 0)
		LoadImage(input, LEFT_IMAGE_FILE_NAME);
	else
		LoadImage(input, RIGHT_IMAGE_FILE_NAME);
}


static void
move_input(INPUT_DESC *input)
{
	if (input->wxd < 0)
		LoadImage(input, RIGHT_IMAGE_FILE_NAME);
	else if (input->wxd >= IMAGE_WIDTH)
		LoadImage(input, LEFT_IMAGE_FILE_NAME);
	else
	{
		check_input_bounds(input, input->wxd, input->wxd);
#ifndef	NO_INTERFACE
		glutSetWindow(input->win);
		input_display();
#endif
	}
}


void 
input_generator(INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_stereo(input);
#ifdef NO_INTERFACE
		input->win = 1;
#endif
	}
	else
	{
		if (status == MOVE)
		{
			move_input(input);
		}
	}
}

void 
input_controler(INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		// Translate the input image & Move the input center cursor
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);		
		interpreter (strCommand);
	}
	input->mouse_button = -1;
	
	return;
}


void
shift_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int xi_target;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

//	if (p_number != 2)
//	{
//		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <xi_target_center> <yi_target_center>.", "", "");
//		return;
//	}

	// Gets the Filter Parameters - The Pointers And The Values - Void pointers must be casted
	xi_target = shift_filter_deltaX;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;


	// Parallel translation filter capabilities where OpenMP available
	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) yo;

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) xo - xi_target;

			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}
	}
}


void selector_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int xo, yo, wo, ho;
	int nl_number, p_number;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
		;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
		;


	nl_output = filter_desc->output;

	if (forward_image_left == 1)
		nl_input = filter_desc->neuron_layer_list->neuron_layer;
	else
		nl_input = filter_desc->neuron_layer_list->next->neuron_layer;


	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xo + yo * wo].output;
		}
	}
}

void
resize_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int xi, yi, wi, hi, xo, yo, wo, ho;
	int nl_number, p_number;
	double _scale_factor;
	int level;
	
	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
		;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
		;

	level = filter_desc->filter_params->next->param.ival;

	nl_output = filter_desc->output;

	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;


	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

//	_scale_factor = (double) wi / (double) wo;
	_scale_factor = 1.0 / pow(scale_factor, (double) level);

	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) (_scale_factor * ((double) yo) + 0.5);

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (_scale_factor * ((double) xo) + 0.5);
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}

	}
}

void
build_disparity_and_hamming_distance_vectors(NEURON_LAYER *nl, int wo, int ho, int level, double weight)
{
	int xi, yi, wi, hi, xo, yo;
	double _scale_factor = 1.0;
	
	if (disparity == NULL)
	{
		disparity = (int *) malloc(wo * ho * 5 * sizeof(int));
	}

	if (h_distance == NULL)
	{
		h_distance = (int *) malloc(wo * ho * 5 * sizeof(int));
	}
	wi = nl->dimentions.x;
	hi = nl->dimentions.y;
//	_scale_factor = (double) wi / (double) wo;
	_scale_factor = pow(scale_factor, (double) level);

	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) (_scale_factor * ((double) yo));

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (_scale_factor * ((double) xo));
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
			{
				disparity[yo * wo * 5 + xo * 5 + level] = nl->neuron_vector[xi + yi * wi].output.ival;
				h_distance[yo * wo * 5 + xo * 5 + level] = (int)(nl->neuron_vector[xi + yi * wi].last_hamming_distance * weight);
			}
			else
			{
				disparity[yo * wo * 5 + xo * 5 + level] = 0;
				h_distance[yo * wo * 5 + xo * 5 + level] = 100000;
			}
		}

	}
}


int 
compair_disparity(const void *p_neuron1, const void *p_neuron2)
{
	NEURON *neuron1, *neuron2;
	
	neuron1 = *((NEURON **) p_neuron1);
	neuron2 = *((NEURON **) p_neuron2);
	if (neuron1->output.ival == neuron2->output.ival)
		return 0;
	else
		return ((neuron1->output.ival > neuron2->output.ival) ? 1 : -1);
}


int 
compair_hamming_distance(const void *p_neuron1, const void *p_neuron2)
{
	NEURON *neuron1, *neuron2;
	
	neuron1 = *((NEURON **) p_neuron1);
	neuron2 = *((NEURON **) p_neuron2);
	if (neuron1->last_hamming_distance == neuron2->last_hamming_distance)
		return 0;
	else
		return ((neuron1->last_hamming_distance > neuron2->last_hamming_distance) ? 1 : -1);
}


int 
compair_disparity_and_hamming_distance(const void *p_neuron1, const void *p_neuron2)
{
	NEURON *neuron1, *neuron2;
	
	neuron1 = *((NEURON **) p_neuron1);
	neuron2 = *((NEURON **) p_neuron2);
	if (neuron1->output.ival == neuron2->output.ival)
	{
		if (neuron1->last_hamming_distance == neuron2->last_hamming_distance)
			return 0;
		else
			return ((neuron1->last_hamming_distance > neuron2->last_hamming_distance) ? 1 : -1);
	}
	else
		return ((neuron1->output.ival > neuron2->output.ival) ? 1 : -1);
}


int 
compair_hamming_distance_and_disparity(const void *p_neuron1, const void *p_neuron2)
{
	NEURON *neuron1, *neuron2;
	
	neuron1 = *((NEURON **) p_neuron1);
	neuron2 = *((NEURON **) p_neuron2);
	if (neuron1->last_hamming_distance == neuron2->last_hamming_distance)
	{
		if (neuron1->output.ival == neuron2->output.ival)
			return 0;
		else
			return ((neuron1->output.ival > neuron2->output.ival) ? 1 : -1);
	}
	else
		return ((neuron1->last_hamming_distance > neuron2->last_hamming_distance) ? 1 : -1);
}


int 
compair_hamming_distance_and_level(const void *p_neuron1, const void *p_neuron2)
{
	NEURON *neuron1, *neuron2;
	
	neuron1 = *((NEURON **) p_neuron1);
	neuron2 = *((NEURON **) p_neuron2);
	if (neuron1->last_hamming_distance == neuron2->last_hamming_distance)
	{
		if (neuron1->holder_neuron_layer->dimentions.x == neuron2->holder_neuron_layer->dimentions.x)
			return 0;
		else
			return ((neuron1->holder_neuron_layer->dimentions.x < neuron2->holder_neuron_layer->dimentions.x) ? 1 : -1);
	}
	else
		return ((neuron1->last_hamming_distance > neuron2->last_hamming_distance) ? 1 : -1);
}


void
set_most_voted_disparity(NEURON *central_neuron, NEURON **neuron, int size)
{
	int i, num_neurons_with_current_disparity, current_max_disparity_repetition, index_of_neuron_with_most_common_disparity, current_disparity;
	
	qsort((void *) neuron, size, sizeof(NEURON *), compair_disparity_and_hamming_distance);
	
	current_disparity = neuron[0]->output.ival;
	current_max_disparity_repetition = num_neurons_with_current_disparity = 1;
	index_of_neuron_with_most_common_disparity = 0;
	for (i = 1; i < size; i++)
	{
		if (neuron[i]->output.ival == current_disparity)
		{
			num_neurons_with_current_disparity++;
			if ((i == (size - 1)) && (num_neurons_with_current_disparity > current_max_disparity_repetition))
			{
				current_max_disparity_repetition = num_neurons_with_current_disparity;
				index_of_neuron_with_most_common_disparity = i - num_neurons_with_current_disparity + 1;
			}
			
		}
		else
		{
			if (num_neurons_with_current_disparity > current_max_disparity_repetition)
			{
				current_max_disparity_repetition = num_neurons_with_current_disparity;
				index_of_neuron_with_most_common_disparity = i - num_neurons_with_current_disparity;
			}
				
			current_disparity = neuron[i]->output.ival;
			num_neurons_with_current_disparity = 1;
		}
	}

	central_neuron->output.ival = neuron[index_of_neuron_with_most_common_disparity]->output.ival;
	central_neuron->last_hamming_distance =  neuron[index_of_neuron_with_most_common_disparity]->last_hamming_distance;
}


void
set_most_voted_hamming_distance(NEURON *central_neuron, NEURON **neuron, int size)
{
	qsort((void *) neuron, size, sizeof(NEURON *), compair_hamming_distance_and_level);
	
	central_neuron->output.ival = neuron[0]->output.ival;
	central_neuron->last_hamming_distance =  neuron[0]->last_hamming_distance;
}


void
intra_layer_cooperation(NEURON_LAYER *nl)
{
	int y, x, l, h, w, i;
	NEURON *neuron[9];
	NEURON *temp_neuron_vector;
	
	w = nl->dimentions.x;
	h = nl->dimentions.y;
	temp_neuron_vector = (NEURON *) malloc(w * h * sizeof(NEURON));
	for (l = 0; l < number_of_iterations; l++)
	{
		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				i = 0;
				neuron[i] = &(nl->neuron_vector[x + y * w]);
				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == 0))
					neuron[i] = &(nl->neuron_vector[x + (y - 1) * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(x == w - 1))
					neuron[i] = &(nl->neuron_vector[(x + 1) + y * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(x == 0))
					neuron[i] = &(nl->neuron_vector[(x - 1) + y * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == h - 1))
					neuron[i] = &(nl->neuron_vector[x + (y + 1) * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == 0) && !(x == 0))
					neuron[i] = &(nl->neuron_vector[(x - 1) + (y - 1) * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == 0) && !(x == w - 1))
					neuron[i] = &(nl->neuron_vector[(x + 1) + (y - 1) * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == h - 1) && !(x == 0))
					neuron[i] = &(nl->neuron_vector[(x - 1) + (y + 1) * w]);

				i++;
				neuron[i] = neuron[i - 1];
				if (!(y == h - 1) && !(x == w - 1))
					neuron[i] = &(nl->neuron_vector[(x + 1) + (y + 1) * w]);
				
				set_most_voted_disparity(&(temp_neuron_vector[x + y * w]), neuron, 9);
			}
		}

		for (i = 0; i < (w * h); i++)
		{
			nl->neuron_vector[i].output = temp_neuron_vector[i].output;
			nl->neuron_vector[i].last_hamming_distance = temp_neuron_vector[i].last_hamming_distance;
		}
	}

	free(temp_neuron_vector);
}


void
inter_layer_competition(NEURON_LAYER *nl_output, 
			NEURON_LAYER *nl_input_level0, NEURON_LAYER *nl_input_level1, NEURON_LAYER *nl_input_level2, NEURON_LAYER *nl_input_level3, NEURON_LAYER *nl_input_level4)
{
	NEURON *neuron[5];
	double _scale_factor;
	int w, h, x, y, i, xi, yi, wi, hi, l;

	w = nl_output->dimentions.x;
	h = nl_output->dimentions.y;
	for (l = 0; l < number_of_iterations; l++)
	{
		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				i = 0;
				neuron[i] = &(nl_input_level0->neuron_vector[x + y * w]);
				i++;

				_scale_factor = pow(scale_factor, 1.0);
				yi = (int) (_scale_factor * ((double) y));
				xi = (int) (_scale_factor * ((double) x));
				wi = nl_input_level1->dimentions.x;
				hi = nl_input_level1->dimentions.y;
				if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				{
					neuron[i] = &(nl_input_level1->neuron_vector[xi + yi * wi]);
					i++;
				}

				_scale_factor = pow(scale_factor, 2.0);
				yi = (int) (_scale_factor * ((double) y));
				xi = (int) (_scale_factor * ((double) x));
				wi = nl_input_level2->dimentions.x;
				hi = nl_input_level2->dimentions.y;
				if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				{
					neuron[i] = &(nl_input_level2->neuron_vector[xi + yi * wi]);
					i++;
				}

				_scale_factor = pow(scale_factor, 3.0);
				yi = (int) (_scale_factor * ((double) y));
				xi = (int) (_scale_factor * ((double) x));
				wi = nl_input_level3->dimentions.x;
				hi = nl_input_level3->dimentions.y;
				if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				{
					neuron[i] = &(nl_input_level3->neuron_vector[xi + yi * wi]);
					i++;
				}

				_scale_factor = pow(scale_factor, 4.0);
				yi = (int) (_scale_factor * ((double) y));
				xi = (int) (_scale_factor * ((double) x));
				wi = nl_input_level4->dimentions.x;
				hi = nl_input_level4->dimentions.y;
				if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				{
					neuron[i] = &(nl_input_level4->neuron_vector[xi + yi * wi]);
					i++;
				}

				set_most_voted_disparity(&(nl_output->neuron_vector[x + y * w]), neuron, 5);
			}
		}

		for (i = 0; i < (w * h); i++)
		{
			nl_input_level0->neuron_vector[i].output = nl_output->neuron_vector[i].output;
			nl_input_level0->neuron_vector[i].last_hamming_distance = nl_output->neuron_vector[i].last_hamming_distance;
		}
	}
}


void
combining_nl_filter(FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input_level0 = NULL, *nl_input_level1 = NULL, *nl_input_level2, *nl_input_level3, *nl_input_level4;
	
	if (!forward_image_left) // esta treinando
		return;
		
	nl_output = filter_desc->output;

	nl_input_level0 = filter_desc->neuron_layer_list->neuron_layer;
	nl_input_level1 = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_input_level2 = filter_desc->neuron_layer_list->next->next->neuron_layer;
	nl_input_level3 = filter_desc->neuron_layer_list->next->next->next->neuron_layer;
	nl_input_level4 = filter_desc->neuron_layer_list->next->next->next->next->neuron_layer;

	intra_layer_cooperation(nl_input_level0);
	intra_layer_cooperation(nl_input_level1);
	intra_layer_cooperation(nl_input_level2);
	intra_layer_cooperation(nl_input_level3);
	intra_layer_cooperation(nl_input_level4);

	inter_layer_competition(nl_output, nl_input_level0, nl_input_level1, nl_input_level2, nl_input_level3, nl_input_level4);
//	inter_layer_competition(nl_output, nl_input_level0, nl_input_level0, nl_input_level0, nl_input_level0, nl_input_level0);
}


NEURON_OUTPUT
PropagateDisparitiesToTheLeft(PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int max_disparity;
	NEURON_LAYER *nl_output;
	int xo, yo, wo, ho;

	max_disparity = param_list->next->param.ival;
	nl_output = get_neuron_layer_by_name("nl_disparity_map");
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < (max_disparity - 1); xo++)
		{
			nl_output->neuron_vector[yo * wo + xo].output.ival = nl_output->neuron_vector[yo * wo + max_disparity - 1].output.ival;
		}
	}

	output.ival = 0;	
	return (output);
}


NEURON_OUTPUT
TrainNetwork(PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int disparity;
	int x, y, w, h;
	NEURON_LAYER_LIST *nl_list;
	NEURON *neuron_vector;
	NEURON_LAYER *nl_output;
	int main_output_width;

	disparity = param_list->next->param.ival;
	nl_output = get_neuron_layer_by_name("nl_disparity_map");
	main_output_width = nl_output->dimentions.x;
	
	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		neuron_vector = nl_list->neuron_layer->neuron_vector;
		if (neuron_vector[0].synapses != NULL)
		{
			if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
			    (nl_list->neuron_layer->associated_neuron_vector != NULL))
			{
				w = nl_list->neuron_layer->dimentions.x;
				h = nl_list->neuron_layer->dimentions.y;
				for (y = 0; y < h; y++)
				{
					for (x = 0; x < w; x++)
					{
						if (x >= (disparity / (main_output_width / w)))
							train_neuron (nl_list->neuron_layer, y * w + x);
					}
				}
			}
		}
	}

	output.ival = 0;	
	return (output);
}
