#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#include "rbm.h"
#include "../character.h"

#define READ			0
#define TRAIN			1
#define TRAIN_STEP		2
#define	ANNEALING_TEST	3
#define SET_AND_TEST	4
#define	UNLOAD_LEARNING	5
#define RELOAD_LEARNING	6

int g_state = -1;
int g_label = -1;
int g_cont_char = -1;
int char_image_file_size;
int char_label_file_size;

long char_image_file_initial_number_pos;
long char_label_file_initial_number_pos;

FILE *char_image_file = NULL;
FILE *char_label_file = NULL;
FILE *g_input_file = NULL;

int g_number;
int g_result[10];
int g_difference_between_highest_second_highest;
float g_certainty;
float g_activation;

// Correct and Incorrect counters as global variables
int correct = 0;
int incorrect = 0;

int g_data_set_order[60000];


void
read_data_set_order(int *order, int size, const char *file_name)
{
	int i = 0;
	FILE *file;

	if ((file = fopen(file_name, "r")) ==  NULL)
	{
		while(i < size)
		{
			order[i] = i;
			i++;
		}
	}
	else
	{
		while(!feof(file))
		{
			if (fscanf(file, "%d\n", &order[i]) < 0)
				Erro((char *)"Error reading order from file", (char *)file_name, (char *)"");
			i++;
		}
	}
}


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


unsigned
l_b(unsigned data)
{
	unsigned long data_out;

	data_out = (unsigned long)(data & 0xff) << 24;
	data_out |= (unsigned long)(data & 0xff00) << 8;
	data_out |= (unsigned long)(data & 0xff0000) >> 8;
	data_out |= (unsigned long)(data & 0xff000000) >> 24;

	return (data_out);
}


unsigned int
read_data(FILE *arq, unsigned int data_size)
{
	unsigned int val = 0;

	if (fread(&val, 1, data_size, arq) != data_size)
	{
		printf("Error! Could not read input file\n");
		exit(-1);
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
		exit(-1);
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
		val = l_b(read_data(arq,4));
		*sz = val;
		printf("Number of images:  %d\n", val);
		val = l_b(read_data(arq,4));
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
	else if ((val = l_b(val)) == 0x00000801)
		big_endian = 0;
	else
	{
		printf("Error! File data format not recognized\n");
		exit(-1);
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
		val = l_b(read_data(arq, 4));
		*sz = val;
		printf("Number of labels:  %d\n", val);
	}
}


int 
open_char_input_image(INPUT_DESC *input, FILE **char_image_file)
{
	int i;
	char file_name[256];
	int sz, row, column;

	if (fgets(file_name, 250, g_input_file) == NULL) // this test is necessary to make the compiler happy...
		exit(printf("ERRO: Incorrect fgets\n"));

	i = 0;

	while (isgraph(file_name[i]))
		i++;

	file_name[i] = '\0';

	printf("File_Name:%s\n", file_name);

	if (*(char_image_file) != NULL)
		fclose(*(char_image_file));

	if ((*(char_image_file) = fopen(file_name, "r")) == NULL)
	{
		printf("Error! Could not open %s\n", file_name);
		exit(-1);
	}

	print_header_image(*char_image_file, &row, &column, &sz);
	return (sz);
}


int 
open_char_input_label(INPUT_DESC *input, FILE **char_label_file)
{
	int i;
	int sz;
	char file_name[256];

	if (fgets(file_name, 250, g_input_file) == NULL) // this test is necessary to make the compiler happy...
		exit(printf("ERRO: Incorrect fgets\n"));

	i = 0;

	while (isgraph(file_name[i]))
		i++;

	file_name[i] = '\0';

	printf("File_Name:%s\n",file_name);

	if (*(char_label_file) != NULL)
		fclose(*(char_label_file));

	if ((*(char_label_file) = fopen(file_name, "r")) == NULL)
	{
		printf("Error! Could not open %s\n", file_name);
		exit(-1);
	}

	print_header_label(*char_label_file, &sz);
	return (sz);
}


void 
read_char_input_image(INPUT_DESC *input, FILE *char_image_file)
{
	int x, y;

	for (y = input->wh - 1; y >= 0; y--)
		for (x = 0; x < input->ww; x++)
			input->neuron_layer->neuron_vector[y * input->ww + x].output.ival = read_data(char_image_file, 1);
}


unsigned char
read_char_input_label(FILE *char_label_file)
{
	return (read_data(char_label_file, 1));
}


int
read_char_input(INPUT_DESC *input, FILE *char_label_file, FILE *char_image_file, int direction)
{
	int new_label;

	if (direction == -1)
	{
		if (g_cont_char == 0)
			g_cont_char = char_image_file_size - 1;
		else
			g_cont_char--;
	}
	else if (direction == 1)
	{
		if (g_cont_char < (char_image_file_size - 1))
			g_cont_char++;
		else
			g_cont_char = 0;
	}

	fseek(char_image_file, char_image_file_initial_number_pos + g_cont_char *(input->wh * input->ww), SEEK_SET);
	fseek(char_label_file, char_label_file_initial_number_pos + g_cont_char, SEEK_SET);

	read_char_input_image(input, char_image_file);
	new_label = read_char_input_label(char_label_file);

	// printf("image_number = %06d; label = %d\n", g_cont_char, new_label);

	return (new_label);
}


void
get_new_char(INPUT_DESC *input, int direction)
{
	g_label = (int) read_char_input(input, char_label_file, char_image_file, direction);
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
rewind_char_input(FILE *char_label_file, FILE *char_image_file)
{
	fseek(char_image_file, char_image_file_initial_number_pos, SEEK_SET);
	fseek(char_label_file, char_label_file_initial_number_pos, SEEK_SET);
}


int
read_next_char_input(INPUT_DESC *input, FILE *char_label_file, FILE *char_image_file)
{
	int new_label;

	read_char_input_image(input, char_image_file);
	new_label = read_char_input_label(char_label_file);

	return (new_label);
}


void
get_next_char(INPUT_DESC *input)
{
	g_label = (int) read_next_char_input(input, char_label_file, char_image_file);
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
make_input_image_character(INPUT_DESC *input)
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
		Erro((char *)"Invalid Type Show ", message, (char *)" Error in update_input_image.");
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
set_current_data_set_from_g_input_file(INPUT_DESC *input)
{
	char_image_file_size = open_char_input_image(input, &char_image_file);
	char_image_file_initial_number_pos = ftell(char_image_file);

	char_label_file_size = open_char_input_label(input, &char_label_file);
	char_label_file_initial_number_pos = ftell(char_label_file);

	if (char_image_file_size != char_label_file_size)
	{
		printf("Image and label Train files with diferent sizes\n");
		exit(-1);
	}

	if ((char_image_file_size == 0) || (char_label_file_size == 0))
	{
		printf("Empty input file\n");
		exit(-1);
	}
}


void
set_train_data_set(INPUT_DESC *input)
{
	rewind(g_input_file);
	set_current_data_set_from_g_input_file(input);
	read_data_set_order(g_data_set_order, char_image_file_size, "character_train_order_small.csv");
}


void
set_test_data_set(INPUT_DESC *input)
{
	rewind(g_input_file);
	set_current_data_set_from_g_input_file(input);
	set_current_data_set_from_g_input_file(input);
	read_data_set_order(g_data_set_order, char_image_file_size, "character_test_order_small.csv");
}


void 
init_character_input(INPUT_DESC *input)
{
#ifndef	NO_INTERFACE
	int x, y;
#endif

	float f;
	char file_name[256];

	strcpy(file_name, input->name);
	strcat(file_name, ".in");

	if ((g_input_file = fopen(file_name, "r")) == NULL)
		Erro((char *)"cannot open input file: ", file_name, (char *)"");
	set_train_data_set(&character);

	make_input_image_character(input);

	f = 1.0;

	while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
		f += 1.0;

	while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
		f *= 0.9;

#ifndef	NO_INTERFACE
	glutInitWindowSize((int)((float)input->ww * f),(int)((float)input->wh * f));

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
input_generator(INPUT_DESC *input, int status)
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

		all_filters_update();
	}
}


void 
user_set_neurons(NEURON *n, OUTPUT_DESC *output, int label, int value)
{
	int xi, yi, xf, yf, aux;

	if ((label >= 0) && (label <= 4))
	{
		xi = 0;
		xf = output->ww / 2;
	}
	else if ((label >= 5) && (label <= 9))
	{
		xi = output->ww / 2;
		xf = output->ww;
	}
	else
		exit(printf("Invalid label\n"));

	yi = (label % 5) *(output->wh / 5);
	yf = ((label % 5) + 1) *(output->wh / 5);

	for (; xi < xf; xi++)
	{
		for (aux = yi; aux < yf; aux++)
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

	if ((g_label >= 0) && (g_label <= 9))
	{
		output = get_output_by_name(output_name);

		set_neurons(output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
		// user_set_neurons(output->neuron_layer->neuron_vector, output, g_label, NUM_COLORS - 1);
		fill_in_all_neurons_output_with_label(output->neuron_layer->neuron_vector, output, g_label);
		update_output_image(output);

#ifndef	NO_INTERFACE
		glutSetWindow(output->win);
		glutPostWindowRedisplay(output->win);
#endif
	}
}


void
update_stats(int number, int label, float certainty, float activation, int num_neurons, int result[10])
{
	int i;
	//static int correct = 0, incorrect = 0;

	printf("img = %06d: ", g_cont_char);

	for (i = 0; i < 10; i++)
	{
		printf("%d = %0.2f  ", i, (float) result[i] / (float) num_neurons);
	}

	if (number == label)
	{
		correct++;
		printf("-> %d == %d  %0.2f %0.2f (%0.4f)\n", 
				number, label,
				certainty, activation,
				(float) correct / (float) (correct + incorrect));
	}
	else
	{
		incorrect++;
		printf("-> %d != %d  %0.2f %0.2f (%0.4f) ****\n", 
				number, label,
				certainty, activation,
				(float) correct / (float) (correct + incorrect));
	}
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
	int wo, ho;
	int wi, hi;
	int xo, yo;
	int xi, yi;
	BoltzmannMachine *network;
	BoltzmannMachine **p_network;

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

	if (*((int *)filter_desc->filter_params->next->param.pval) == 0)
	{
		network = new BoltzmannMachine(wi * hi, wo * ho, filter_desc->filter_params->next->next->param.ival);
		p_network = (BoltzmannMachine **) &(filter_desc->filter_params->next->param.pval);
		*p_network = network;
	}
	else
	{
		network = (BoltzmannMachine *) filter_desc->filter_params->next->param.pval;
	}

	if (g_state == READ)
	{
		double data[INPUT_WIDTH * INPUT_HEIGHT];

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				data[yi * wi + xi] = ( ((float)nl_input->neuron_vector[yi * wi + xi].output.ival) / 255.0);

		network->read_data(data, INPUT_WIDTH * INPUT_HEIGHT);
	}
	else if (g_state == TRAIN)
	{
		network->train_network();
	}
	else if (g_state == TRAIN_STEP)
	{
		network->train_step(10);

		for (yo = 0; yo < ho; yo++)
			for (xo = 0; xo < wo; xo++)
				nl_output->neuron_vector[yo * wo + xo].output.fval = (float) (network->get_hidden_neuron_value(yo * wo + xo));

		update_output_image(get_output_by_neural_layer(nl_output));

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				nl_input->neuron_vector[yi * wi + xi].output.ival = (int) (network->get_visible_neuron_value(yi * wi + xi) * 255.0);
		
		update_input_image(get_input_by_neural_layer(nl_input));
	}
	else if (g_state == ANNEALING_TEST)
	{
		network->annealing_test();

		for (yo = 0; yo < ho; yo++)
			for (xo = 0; xo < wo; xo++)
				nl_output->neuron_vector[yo * wo + xo].output.fval = (float) (network->get_hidden_neuron_value(yo * wo + xo));

		update_output_image(get_output_by_neural_layer(nl_output));

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				nl_input->neuron_vector[yi * wi + xi].output.ival = (int) (network->get_visible_neuron_value(yi * wi + xi) * 255.0);
		
		update_input_image(get_input_by_neural_layer(nl_input));
	}
	else if (g_state == SET_AND_TEST)
	{
		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				network->set_visible_neuron_value(yi * wi + xi, ((float)nl_input->neuron_vector[yi * wi + xi].output.ival) / 255.0);
		
		network->test_network(100);

		for (yo = 0; yo < ho; yo++)
			for (xo = 0; xo < wo; xo++)
				nl_output->neuron_vector[yo * wo + xo].output.fval = (float) (network->get_hidden_neuron_value(yo * wo + xo));

		update_output_image(get_output_by_neural_layer(nl_output));

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				nl_input->neuron_vector[yi * wi + xi].output.ival = (int) (network->get_visible_neuron_value(yi * wi + xi) * 255.0);
		
		update_input_image(get_input_by_neural_layer(nl_input));
	}
	else if (g_state == UNLOAD_LEARNING)
	{
		network->unload("character.dump");
	}
	else if (g_state == RELOAD_LEARNING)
	{
		network->reload("character.dump");
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

NEURON_OUTPUT 
SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_state = pParamList->next->param.ival;

	switch (g_state)
	{
		case READ:
			set_train_data_set(&character);
			g_cont_char = 0;
			break;
		case TRAIN:
			set_train_data_set(&character);
			g_cont_char = 0;
			break;
		case TRAIN_STEP:
			set_train_data_set(&character);
			g_cont_char = 0;
			break;
		case ANNEALING_TEST:
			set_train_data_set(&character);
			// set_test_data_set(&character);
			g_cont_char = 0;
			break;
		case SET_AND_TEST:
			set_train_data_set(&character);
			// set_test_data_set(&character);
			g_cont_char = 0;
			break;
	}

	output.ival = 0;
	return (output);
}


/*
 ***********************************************************
 * Function: GetCharN
 * Description:
 * Inputs:
 * Output:
 ***********************************************************
 */

NEURON_OUTPUT 
GetCharN(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_cont_char = pParamList->next->param.ival;
	get_new_char(&character, 0);

	output.ival = 0;
	return (output);
}


/*
 ***********************************************************
 * Function: GetNextChar
 * Description:
 * Inputs:
 * Output:
 ***********************************************************
 */

NEURON_OUTPUT 
GetNextChar(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	get_next_char(&character);

	output.ival = 0;
	return (output);
}


/*
 ***********************************************************
 * Function: RewindCharInput
 * Description:
 * Inputs:
 * Output:
 ***********************************************************
 */

NEURON_OUTPUT 
RewindCharInput(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	rewind_char_input(char_label_file, char_image_file);

	output.ival = 0;
	return (output);
}


/*
 ***********************************************************
 * Function: GetCharOrderN
 * Description:
 * Inputs:
 * Output:
 ***********************************************************
 */

NEURON_OUTPUT
GetCharOrderN(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_cont_char = g_data_set_order[pParamList->next->param.ival];
	get_new_char(&character, 0);

	output.ival = 0;
	return (output);
}


