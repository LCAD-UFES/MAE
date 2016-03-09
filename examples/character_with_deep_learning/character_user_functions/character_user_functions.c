#include <stdlib.h>
#include <stdio.h>
#include "mae.h"

#define NIL -1
#define TRAINING_PHASE 0
#define TESTING_PHASE 1

FILE *char_image_file = NULL;
FILE *char_label_file = NULL;

int label = NIL;
int cont_char = 0;
int char_image_file_size;
int char_label_file_size;
long char_image_file_initial_number_pos;
long char_label_file_initial_number_pos;

int g_new_testing_phase_started = 0;
FILE *g_input_file = NULL;
int g_nStatus;
int g_label = 0;
int g_first_label_set = 1;

int num_neurons_in_the_output;
int *last_best_pattern = NULL, *current_best_pattern = NULL;
int *last_neuron_output = NULL, *current_neuron_output = NULL;


int
set_network_status_interface(int net_status)
{
	switch (net_status)
	{
	case TRAINING_PHASE:
		break;
	case TESTING_PHASE:
		g_new_testing_phase_started = 1;
		break;
	default:
		printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
		return (-1);
	}

	g_nStatus = net_status;
	return (0);
}


NEURON_OUTPUT
SetNetworkStatus(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;
	output.ival = set_network_status_interface(g_nStatus);

	return(output);
}


void
make_forward_in_the_neuron_layers()
{
	forward_objects("nl_encoding");
	forward_objects("nl_decoding");

	forward_objects("nl_encoding_level_1");
	forward_objects("nl_decoding_level_1");

	forward_objects("nl_encoding_level_2");
	forward_objects("nl_decoding_level_2");

	forward_objects("nl_label");
	forward_objects("nl_label_decoding");

	all_outputs_update();
}


void
train_network_with_random_input_and_output(char *network_layer_name)
{
	long i, j, k, connections, num_neurons;
	NEURON_LAYER *nl_encoding;
	NEURON *neuron;

	nl_encoding = get_neuron_layer_by_name(network_layer_name);

	if (nl_encoding == NULL)
		exit(printf("Error: Network Layer '%s' not found!\n", network_layer_name));

	num_neurons = get_num_neurons(nl_encoding->dimentions);

	printf("Performing Random Training in '%s'...\n", network_layer_name);

	for (i = 0; i < num_neurons; i++)
	{
		neuron = &(nl_encoding->neuron_vector[i]);
		connections = neuron->n_in_connections;

		for (j = 0; j < NEURON_MEMORY_SIZE; j++)
		{
			if (neuron->memory[j].pattern == NULL)
				neuron->memory[j].pattern = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));

			for (k = 0; k < N_PATTERNS; k++)
			{
				neuron->memory[j].pattern[k] = rand();
			}

			neuron->memory[j].associated_value.ival = rand() % 256;
		}
	}

	printf("Done\n");
}


void
initialize_last_neuron_and_current_neuron_output(int num_neurons)
{
	if (last_neuron_output == NULL)
		last_neuron_output = (int*) calloc (num_neurons, sizeof(int));

	if (last_best_pattern == NULL)
		last_best_pattern = (int*) calloc (num_neurons, sizeof(int));

	if (current_neuron_output == NULL)
		current_neuron_output = (int*) calloc (num_neurons, sizeof(int));

	if (current_best_pattern == NULL)
		current_best_pattern = (int*) calloc (num_neurons, sizeof(int));

	num_neurons_in_the_output = num_neurons;
}


void
update_new_output_image(NEURON_LAYER *neuron_layer)
{
	int i;
	int num_neurons = get_num_neurons(neuron_layer->dimentions);

	// *******************************************************
	// copy 'current_neuron_output' to 'last_neuron_output'
	// *******************************************************

	memcpy(last_neuron_output, current_neuron_output, num_neurons * sizeof(int));
	memcpy(last_best_pattern, current_best_pattern, num_neurons * sizeof(int));

	// *******************************************************
	// copy the neuron layer output to 'current_neuron_output'
	// *******************************************************

	for (i = 0; i < num_neurons; i++)
	{
		current_neuron_output[i] = neuron_layer->neuron_vector[i].dendrite_state.ival;
		current_best_pattern[i] = neuron_layer->neuron_vector[i].last_best_pattern;
	}
}


int
bits_are_equals(PATTERN *neuron_memory_pattern, PATTERN *input_pattern, int bit_index, int connections)
{
	int pattern_index = bit_index / N_PATTERNS;
	int bit_index_in_the_pattern = bit_index % PATTERN_UNIT_SIZE;

	unsigned char mask = 1 << bit_index_in_the_pattern;
	unsigned char bit_value_in_the_neuron_memory = (neuron_memory_pattern[pattern_index] & mask) >> bit_index_in_the_pattern;
	unsigned char bit_value_in_the_input_pattern = (input_pattern[pattern_index] & mask) >> bit_index_in_the_pattern;

	if (bit_value_in_the_neuron_memory == bit_value_in_the_input_pattern)
		return 1;
	else
		return 0;
}


void
copy_bit_from_input_to_neuron_memory(NEURON neuron, PATTERN *input_pattern, int pattern_to_update, int bit_index, int connections)
{
	int pattern_index = bit_index / N_PATTERNS;
	int bit_index_in_the_pattern = bit_index % PATTERN_UNIT_SIZE;

	unsigned char mask = 1 << bit_index_in_the_pattern;
	unsigned char bit_value = (neuron.memory[pattern_to_update].pattern[pattern_index] & mask) >> bit_index_in_the_pattern;

	if (bit_value == 0) /* queremos colocar 1 */
	{
		neuron.memory[pattern_to_update].pattern[pattern_index] = neuron.memory[pattern_to_update].pattern[pattern_index] | mask;
	}
	else /* queremos colocar 0 */
	{
		neuron.memory[pattern_to_update].pattern[pattern_index] = neuron.memory[pattern_to_update].pattern[pattern_index] & (~mask);
	}
}


int
input_pattern_and_neuron_pattern_are_equals(PATTERN *neuron_memory_pattern, PATTERN *input_pattern, int connections)
{
	int i;

	for (i = 0; i < N_PATTERNS; i++)
		if (neuron_memory_pattern[i] != input_pattern[i])
			return 0;

	return 1;
}


void
train_neuron_memory_slightly_changing_bit_pattern(NEURON neuron, PATTERN *input_pattern, int pattern_to_update)
{
	int connections = neuron.n_in_connections;
	int num_bits = PATTERN_UNIT_SIZE * N_PATTERNS;
	int different_bits[num_bits];
	int num_different_bits;
	int random_bit;
	int bit;

	num_different_bits = 0;

	for (bit = 0; bit < num_bits; bit++)
	{
		if (!bits_are_equals(neuron.memory[pattern_to_update].pattern, input_pattern, bit, connections))
		{
			different_bits[num_different_bits] = bit;
			num_different_bits++;
		}
	}

	random_bit = different_bits[rand() % num_different_bits];

	copy_bit_from_input_to_neuron_memory(neuron, input_pattern, pattern_to_update, random_bit, connections);
}


void
train_neuron_memory_with_the_input_pattern(NEURON neuron, PATTERN *input_pattern, int pattern_to_update)
{
	int i;
	int connections = neuron.n_in_connections;

	for (i = 0; i < N_PATTERNS; i++)
		neuron.memory[pattern_to_update].pattern[i] = input_pattern[i];
}


void
get_input_pattern(NEURON_LAYER *neuron_layer, int neuron_index, PATTERN *input_pattern)
{
	NEURON *neuron;
	OUTPUT_TYPE output_type;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	neuron = &(neuron_layer->neuron_vector[neuron_index]);
	neuron_layer_sensitivity = neuron_layer->sensitivity;
	output_type = neuron_layer->output_type;

	(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, neuron_layer_sensitivity, output_type);
}


int
train_neurons_that_chaged(NEURON_LAYER *neuron_layer)
{
	int i, num_neurons_that_changed_the_output = 0;
	PATTERN *input_pattern;

	input_pattern = (PATTERN *) calloc (MAX_CONNECTIONS / PATTERN_UNIT_SIZE, sizeof(PATTERN));

	for (i = 0; i < num_neurons_in_the_output; i++)
	{
		if (neuron_layer->neuron_vector[i].last_num_candidates > 1)
		{
			/* train the neuron */
			get_input_pattern(neuron_layer, i, input_pattern);
			train_neuron_memory_with_the_input_pattern(neuron_layer->neuron_vector[i], input_pattern, neuron_layer->neuron_vector[i].last_best_pattern);
			// train_neuron_memory_slightly_changing_bit_pattern(neuron_layer->neuron_vector[i], input_pattern, neuron_layer->neuron_vector[i].last_best_pattern);
			num_neurons_that_changed_the_output++;
		}
	}

	free(input_pattern);
	return num_neurons_that_changed_the_output;
}


void
retrain_neuron_layer_to_avoid_draws(char *neuron_layer_name)
{
	NEURON_LAYER *neuron_layer = get_neuron_layer_by_name(neuron_layer_name);

	if (neuron_layer == NULL)
		exit(printf("Error: Neuron Layer '%s' not found!\n", neuron_layer_name));

	update_new_output_image(neuron_layer);

	printf("Retrainning '%s'\n", neuron_layer_name);

	do
	{
		make_forward_in_the_neuron_layers();
		update_new_output_image(neuron_layer);

	}while (train_neurons_that_chaged(neuron_layer));
}


NEURON_OUTPUT
RetrainUntilStabilization(PARAM_LIST *pParamList)
{
	char *neuron_layer_name = pParamList->next->param.sval;

	/* Remove as aspas da string */
	if (neuron_layer_name[0] == '\"')
		neuron_layer_name++;
	if (neuron_layer_name[strlen(neuron_layer_name) - 1] == '\"')
		neuron_layer_name[strlen(neuron_layer_name) - 1] = '\0';

	retrain_neuron_layer_to_avoid_draws(neuron_layer_name);

	NEURON_OUTPUT output;
	output.ival = 0;
	return(output);
}


void
perform_random_training()
{
	train_network_with_random_input_and_output("nl_encoding");
	train_network_with_random_input_and_output("nl_encoding_level_1");
	train_network_with_random_input_and_output("nl_encoding_level_2");
}


int 
init_user_functions()
{
	srand(time(NULL));

	set_network_status_interface(TRAINING_PHASE);

	interpreter("toggle draw_active;");
	interpreter("toggle move_active;");

	perform_random_training();
	initialize_last_neuron_and_current_neuron_output(get_num_neurons(get_neuron_layer_by_name("nl_encoding")->dimentions));

	return (0);
}


unsigned
l_b(unsigned data)
{
	unsigned long data_out;
	
	data_out = (unsigned long) (data & 0xff) << 24;
	data_out |= (unsigned long) (data & 0xff00) << 8;
	data_out |= (unsigned long) (data & 0xff0000) >> 8;
	data_out |= (unsigned long) (data & 0xff000000) >> 24;

	return (data_out);
}


unsigned int
read_data(FILE *arq, int data_size)
{
	unsigned int val;
	
	if ((fread(&val, 1, data_size, arq)) != data_size)
	{
		printf("Error! Could not read input file\n");
		exit (-1);
	}
	return (val);
}


void
print_header_image(FILE *arq, int *row, int *column, int *sz)
{
	int val;
	int big_endian;
	
	val = read_data(arq,4);
	if (val == 0x00000803)
		big_endian = 1;
	else if ((val = l_b(val)) == 0x00000803)
		big_endian = 0;
	else
	{
		printf("Error! File data format not recognized\n");
		exit (-1);
	}
	
	if (big_endian)
	{
		printf("Magic number:      0x%x\n", val);
		val = read_data(arq,4);
		*sz = val;
		printf("Number of images:  %d\n", val);
		val = read_data(arq,4);
		*row = val;
		printf("Number of rows:    %d\n", val);
		val = read_data(arq,4);
		*column = val;
		printf("Number of columns: %d\n", val);
	}
	else
	{
		printf("Magic number:      0x%x\n", val);
		val = l_b (read_data(arq,4));
		*sz = val;
		printf("Number of images:  %d\n", val);
		val = l_b (read_data(arq,4));
		*row = val;
		printf("Number of rows:    %d\n", val);
		val = l_b(read_data(arq,4));
		*column = val;
		printf("Number of columns: %d\n", val);
	}
}


void
print_header_label(FILE *arq, int *sz)
{
	int val;
	int big_endian;
	
	val = read_data(arq, 4);
	if (val == 0x00000801)
		big_endian = 1;
	else if ((val = l_b (val)) == 0x00000801)
		big_endian = 0;
	else
	{
		printf("Error! File data format not recognized\n");
		exit (-1);
	}
	
	if (big_endian)
	{
		printf("Magic number:      0x%x\n", val);
		val = read_data(arq, 4);
		*sz = val;
		printf("Number of labels:  %d\n", val);
	}
	else
	{
		printf("Magic number:      0x%x\n", val);
		val = l_b (read_data(arq, 4));
		*sz = val;
		printf("Number of labels:  %d\n", val);
	}
}


int 
open_char_input_image (INPUT_DESC *input, FILE **char_image_file)
{
	char file_name[256];
	int i;
	int sz, row, column;
	
	if (fgets(file_name, 250, g_input_file) == NULL)
		exit(printf("Error: fgets failed\n"));

	i = 0;

	while (isgraph(file_name[i]))
		i++;

	file_name[i] = '\0';

	printf("File_Name:%s\n",file_name);

	if ((*(char_image_file) = fopen(file_name, "r")) == NULL)
	{
		printf("Error! Could not open %s\n", file_name);
		exit(-1);
	}

	print_header_image(*char_image_file, &row, &column, &sz);

	return (sz);
}


int 
open_char_input_label (INPUT_DESC *input, FILE **char_label_file)
{
	char file_name[256];
	int i;
	int sz;
		
	if (fgets (file_name, 250, g_input_file) == NULL)
		exit(printf("Error: fgets failed\n"));

	i = 0;

	while (isgraph (file_name[i]))
		i++;

	file_name[i] = '\0';

	printf("File_Name:%s\n",file_name);

	if ((*(char_label_file) = fopen (file_name, "r")) == NULL)
	{
		printf("Error! Could not open %s\n", file_name);
		exit (-1);
	}

	print_header_label(*char_label_file, &sz);
	
	return (sz);
}


void 
read_char_input_image (INPUT_DESC *input, FILE *char_image_file)
{
	int x, y;

	for (y = input->wh - 1; y >= 0; y--)
		for (x = 0; x < input->ww; x++)
			input->neuron_layer->neuron_vector[y * input->ww + x].output.ival = read_data(char_image_file, 1);
}


unsigned char
read_char_input_label (FILE *char_label_file)
{
	return (read_data(char_label_file, 1));
}


int
read_char_input(INPUT_DESC *input, FILE *char_label_file, FILE *char_image_file, int direction)
{
	int new_label;
	
	if (direction == -1)
	{
		if (cont_char == 0)
			cont_char = char_image_file_size - 1;
		else
			cont_char--;
	}
	else if (direction == 1)
	{
		if (cont_char < (char_image_file_size - 1))
			cont_char++;
		else
			cont_char = 0;
	}
	
	fseek(char_image_file, char_image_file_initial_number_pos + cont_char * (input->wh * input->ww), SEEK_SET);
	fseek(char_label_file, char_label_file_initial_number_pos + cont_char, SEEK_SET);
	read_char_input_image(input, char_image_file);
	new_label = read_char_input_label(char_label_file);

	printf("image_number = %06d; label = %d\n", cont_char, new_label);
	
	return(new_label);
}


void
get_new_char (INPUT_DESC *input, int direction)
{
	label = (int) read_char_input(input, char_label_file, char_image_file, direction);
	check_input_bounds(input, input->wx + input->ww/2, input->wy + input->wh/2);

	update_input_image(input);
	update_input_neurons(input);

	glutSetWindow(input->win);
	glBindTexture(GL_TEXTURE_2D, (GLuint) input->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
	glutPostWindowRedisplay (input->win);
}


void 
make_input_image_character(INPUT_DESC *input)
{
	int i;
	char message[256];
	int w, h;
	
	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;
	
	// Computes the input image dimentions
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	// Saves the image dimentions
	input->ww = w;
	input->wh = h;
	
	// Computes the input image dimentions
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	switch(TYPE_SHOW)
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
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = w - input->vph;
			
	if (input->image == (GLubyte *) NULL)
	{
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
		for (i = 0; i < input->tfh * input->tfw * 3; i++)
			input->image[i] = 0;
	}		

	return;
}


void
open_datasets_file(char *basename)
{
	char file_name[256];

	strcpy (file_name, basename);
	strcat (file_name, ".in");

	if ((g_input_file = fopen (file_name, "r")) == NULL)
		Erro ("cannot open input file: ", file_name, "");
}


void
read_next_char_and_label_files(INPUT_DESC *input)
{
	int x, y;
	float f;

	if (char_image_file != NULL)
		fclose(char_image_file);

	if (char_label_file != NULL)
		fclose(char_label_file);

	char_image_file_size = open_char_input_image(input, &char_image_file);
	char_image_file_initial_number_pos = ftell(char_image_file);

	char_label_file_size = open_char_input_label(input, &char_label_file);
	char_label_file_initial_number_pos = ftell(char_label_file);

	if (char_image_file_size != char_label_file_size)
	{
		printf("Image and label Train files with diferent sizes\n");
		exit (-1);
	}

	if ((char_image_file_size == 0) || (char_label_file_size == 0))
	{
		printf("Empty input file\n");
		exit (-1);
	}

	make_input_image_character(input);

	f = 1.0;

	while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
		f += 1.0;
	while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
		f *= 0.9;

	glutInitWindowSize ((int) ((float)input->ww * f), (int) ((float)input->wh * f));

	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);

	input->win = glutCreateWindow (input->name);
	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init(input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutMouseFunc (input_mouse);
	glutPassiveMotionFunc (input_passive_motion);
	glutKeyboardFunc (keyboard);
}


void
init_character_input(INPUT_DESC *input)
{
	// **********************************
	open_datasets_file(input->name);
	// **********************************
	// char basename[512];
	// sprintf(basename, "/home/filipe/MAE/examples/character_with_deep_learning/character");
	// open_datasets_file(basename);
	// **********************************

	read_next_char_and_label_files(input);
}


void
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
		   (input->mouse_state == GLUT_DOWN) &&
		   (draw_active == 1))
	{
		sprintf(command, "draw out_nl_label based on character move;");
		interpreter(command);
	}
	if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (move_active == 1))
	{
		sprintf(command, "move character to %d, %d;", input->wxd, input->wyd);
		interpreter(command);
	}

	input->mouse_state = -1;
}


void
input_generator (INPUT_DESC *input, int status)
{
	static int flag = 0;
		
	if (flag == 0)
	{
		init_character_input(input);
		flag = 1;
	}
	
	if (status == MOVE)
	{
		if (input->wx >= 0)
			get_new_char(input, 1);
		else
			get_new_char(input, -1);

		make_forward_in_the_neuron_layers();
	}
}


void 
user_set_neurons (NEURON *n, OUTPUT_DESC *output, int label, int value)
{
	int xi, yi, xf, yf, aux;
	
	if ((label >= 0) && (label <= 4))
	{
		xi = 0;
		xf = output->ww/2;
	}
	else if ((label >= 5) && (label <= 9))
	{
		xi = output->ww / 2;
		xf = output->ww;
	}
	else
		exit(printf("Error: Invalid label\n"));
	
	yi = (label % 5) * (output->wh / 5);
	yf = ((label % 5) + 1) * (output->wh / 5);
	
	for (; xi < xf; xi++)
	{
		for(aux = yi; aux < yf; aux++)
			n[(aux * output->ww) + xi].output.ival = value;
	}
}


void
fill_in_all_neurons_output_with_label(NEURON *n, OUTPUT_DESC *output, int label)
{
	int xi, yi, xf, yf, pos, aux;

	xi = 0;
	xf = output->ww;

	yi = 0;
	yf = output->wh;

	for (; xi < xf; xi++)
	{
		for (aux = yi; aux < yf; aux++)
		{
			pos = aux * output->ww + xi;
			n[pos].output.ival = label;
		}
	}
}


void
draw_output(char *output_name, char *input_name)
{
	OUTPUT_DESC *output;

	if ((label >= 0) && (label <= 9))
	{
		output = get_output_by_name(output_name);
		// set_neurons(output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
		// user_set_neurons(output->neuron_layer->neuron_vector, output, label, NUM_COLORS-1);
		fill_in_all_neurons_output_with_label(output->neuron_layer->neuron_vector, output, label);
		update_output_image(output);

		glutSetWindow(output->win);
		glutPostWindowRedisplay(output->win);
	}
	
}


int
evaluate_output(OUTPUT_DESC *output, int *result)
{
	NEURON *n;
	int xi, yi, xf, yf, aux;
	int number, selected_number = 0;
	int greater = 0;
	
	n = output->neuron_layer->neuron_vector;

	for (number = 0; number < 10; number++)
	{
		result[number] = 0;
		
		if ((number >= 0) && (number <= 4))
		{
			xi = 0;
			xf = output->ww/2;
		}
		else if ((number >= 5)&&(number <= 9))
		{
			xi = output->ww/2;
			xf = output->ww;
		}

		yi = (number % 5) * (output->wh / 5);
		yf = ((number % 5) + 1) * (output->wh / 5);

		for (; xi < xf; xi++)
		{
			for(aux = yi; aux < yf; aux++)
			{
				if (n[(aux * output->ww) + xi].output.ival > NUM_COLORS / 2)
					result[number] += 1;
				if (result[number] > greater)
				{
					greater = result[number];
					selected_number = number;
				}
			}
		}
	}

	return (selected_number);
}


void
output_handler(OUTPUT_DESC *output, int status)
{
	static int flag = 0;
	int result[10];
	int i, number;
	static int correct = 0, incorrect = 0;
	
	if (flag == 0)
	{
		flag = 1;
		return;
	}
	if (flag == 1)
	{
		number = evaluate_output (output, result);
		for (i = 0; i < 10; i++)
		{
			printf("%d = %0.2f  ", i, (float) result[i] / ((float) (output->ww * output->wh)/10.0));
		}
		if (number == label)
		{
			correct++;
			printf("-> %d == %d (%0.4f)\n", number, label, (float) correct / (float) (correct+incorrect));
		}
		else
		{
			incorrect++;
			printf("-> %d != %d (%0.4f)\n", number, label, (float) correct / (float) (correct+incorrect));
		}
		flag = 1;
	}
	else
		flag++;
}


int
evaluate_output2(OUTPUT_DESC *output, int *result)
{
	NEURON *neuron;

	int i;
	int number, selected_number = 0;
	int greater = 0;

	neuron = output->neuron_layer->neuron_vector;

	for (number = 0; number < 10; number++)
	{
		result[number] = 0;

		for (i = 0; i < (output->ww * output->wh); i++)
		{
			if (neuron[i].output.ival == number)
			{
				result[number] += 1;
			}
		}

		if (result[number] > greater)
		{
			greater = result[number];
			selected_number = number;
		}
	}

	return (selected_number);
}


void
output_handler1(OUTPUT_DESC *output, int status)
{
}


void
output_handler2(OUTPUT_DESC *output, int status)
{
}


void
output_handler_label(OUTPUT_DESC *output, int status)
{
	if (g_nStatus == TESTING_PHASE)
	{
		static int flag = 0;
		int result[10];
		int i, number;
		static int correct = 0, incorrect = 0;

		if (g_new_testing_phase_started)
		{
			correct = 0;
			incorrect = 0;

			g_new_testing_phase_started = 0;
		}

		if (flag == 0)
		{
			flag = 1;
			return;
		}
		if (flag == 1)
		{
			number = evaluate_output2(output, result);

			for (i = 0; i < 10; i++)
			{
				printf("%d = %0.2f  ", i, (float) result[i] / ((float) (output->ww * output->wh)));
			}
			if (number == label)
			{
				correct++;
				printf("-> %d == %d(%0.4f)\n", number, label, (float) correct / (float) (correct + incorrect));
			}
			else
			{
				incorrect++;
				printf("-> %d != %d(%0.4f) ****\n", number, label, (float) correct / (float) (correct + incorrect));
			}
			flag = 1;
		}
		else
			flag++;
	}
}


void
output_handler_label_decoding(OUTPUT_DESC *output, int status)
{
}


void
output_handler_encoding_level_1(OUTPUT_DESC *output, int status)
{
}


void
output_handler_decoding_level_1(OUTPUT_DESC *output, int status)
{
}


void
output_handler_encoding_level_2(OUTPUT_DESC *output, int status)
{
}


void
output_handler_decoding_level_2(OUTPUT_DESC *output, int status)
{
}


void
PerformForwardInDownwardDirection()
{
	forward_objects("nl_encoding_level_2_filter");
	forward_objects("nl_decoding_level_2");

	forward_objects("nl_encoding_level_1_filter");
	forward_objects("nl_decoding_level_1");

	forward_objects("nl_encoding_filter");
	forward_objects("nl_decoding");

	all_outputs_update();
}


void
f_keyboard(char *key_value)
{
	char key = key_value[0];

	switch (key)
	{
		case 'd':
		{
			train_neuron_layer("nl_decoding");
			train_neuron_layer("nl_decoding_level_1");
			train_neuron_layer("nl_decoding_level_2");
			train_neuron_layer("nl_label");
			train_neuron_layer("nl_label_decoding");

			break;
		}
		case 't': //set training status
		{
			set_network_status_interface(TRAINING_PHASE);
			break;
		}
		case 'r': //set testing status
		{
			set_network_status_interface(TESTING_PHASE);
			break;
		}
		case 'e':
		{
			fflush(stdout);
			break;
		}
		case 's':
		{
			retrain_neuron_layer_to_avoid_draws("nl_encoding");
			retrain_neuron_layer_to_avoid_draws("nl_encoding_level_1");
			retrain_neuron_layer_to_avoid_draws("nl_encoding_level_2");

			break;
		}
		case 'c':
		{
			PerformForwardInDownwardDirection();
			break;
		}
		case 'm':
		{
			NEURON_LAYER *nl_label = get_neuron_layer_by_name("nl_label");
			OUTPUT_DESC *output = get_output_by_name("out_nl_label");

			if (g_first_label_set)
			{
				g_label = 0;
				g_first_label_set = 0;
			}
			else
				g_label++;

			if (g_label > 9)
				g_label = 0;

			fill_in_all_neurons_output_with_label(nl_label->neuron_vector, output, g_label);
			update_output_image(output);

			forward_objects("nl_label_decoding");
			all_outputs_update();

			break;
		}
		case 'n':
		{
			NEURON_LAYER *nl_label = get_neuron_layer_by_name("nl_label");
			OUTPUT_DESC *output = get_output_by_name("out_nl_label");

			if (g_first_label_set)
			{
				g_label = 0;
				g_first_label_set = 0;
			}
			else
				g_label--;

			if (g_label < 0)
				g_label = 9;

			fill_in_all_neurons_output_with_label(nl_label->neuron_vector, output, g_label);
			update_output_image(output);

			forward_objects("nl_label_decoding");
			all_outputs_update();

			break;
		}
		default:
		{
			printf("Option not found\n");
			break;
		}
	}
}
