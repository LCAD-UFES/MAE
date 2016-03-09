#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#include "dbn.h"
#include "character.h"

#define READ_ALL	0
#define PRE_TRAIN	1
#define TRAIN		2
#define TEST		3
#define	UNLOAD		4
#define RELOAD		5

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

int g_translate_x = 0;
int g_translate_y = 0;
int g_translate_x_c = 0;
int g_translate_y_c = 0;

int g_number;
int g_result[10];
int g_difference_between_highest_second_highest;
float g_certainty;
float g_activation;

// Correct and Incorrect counters as global variables
int correct = 0;
int incorrect = 0;

int g_data_set_order[60000];

std::vector<Vector> g_data_set_train(60000);

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
		// Moves cross 1 pixel up
		case 'I':
		case 'i':
			g_translate_y -= 1;
			forward_network();
			break;
		// Moves cross 1 pixel down
		case 'M':
		case 'm':
			g_translate_y += 1;
			forward_network();
			break;
		// Moves cross 1 pixel right
		case 'K':
		case 'k':
			g_translate_x -= 1;
			forward_network();
			break;
		// Moves cross 1 pixel left
		case 'J':
		case 'j':
			g_translate_x += 1;
			forward_network();
			break;
		// Reset pose
		case 'R':
		case 'r':
			g_translate_x = 0;
			g_translate_y = 0;
			forward_network();
			break;
		// Copy translation
		case 'C':
		case 'c':
			g_translate_x_c = g_translate_x;
			g_translate_y_c = g_translate_y;
			forward_network();
			break;
		// Copy translation
		case 'P':
		case 'p':
			g_translate_x = g_translate_x_c;
			g_translate_y = g_translate_y_c;
			forward_network();
			break;
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
		//		printf("Magic number:      0x%x\n", val);
		val = read_data(arq,4);
		*sz = val;
		//		printf("Number of images:  %d\n", val);
		val = read_data(arq,4);
		*row = val;
		//		printf("Number of rows:    %d\n", val);
		val = read_data(arq,4);
		*column = val;
		//		printf("Number of columns: %d\n", val);
	}
	else
	{
		//		printf("Magic number:      0x%x\n", val);
		val = l_b(read_data(arq,4));
		*sz = val;
		//		printf("Number of images:  %d\n", val);
		val = l_b(read_data(arq,4));
		*row = val;
		//		printf("Number of rows:    %d\n", val);
		val = l_b(read_data(arq,4));
		*column = val;
		//		printf("Number of columns: %d\n", val);
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
		//		printf("Magic number:      0x%x\n", val);
		val = read_data(arq, 4);
		*sz = val;
		//		printf("Number of labels:  %d\n", val);
	}
	else
	{
		//		printf("Magic number:      0x%x\n", val);
		val = l_b(read_data(arq, 4));
		*sz = val;
		//		printf("Number of labels:  %d\n", val);
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

	//	printf("File_Name:%s\n", file_name);

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

	//	printf("File_Name:%s\n",file_name);

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

//	printf("image_number = %06d; label = %d\n", g_cont_char, new_label);

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
	read_data_set_order(g_data_set_order, char_image_file_size, "character_train_order_full.csv");
}


void
set_test_data_set(INPUT_DESC *input)
{
	rewind(g_input_file);
	set_current_data_set_from_g_input_file(input);
	set_current_data_set_from_g_input_file(input);
	read_data_set_order(g_data_set_order, char_image_file_size, "character_test_order_full.csv");
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

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	   (input->mouse_state == GLUT_DOWN) &&
	   (draw_active == 1))
	{
		sprintf(command, "draw char_discr_out based on character move;");
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
		output_update(&character_scaled_out);
		output_update(&character_gaussian_out);
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
	int data_size;
	AutoEncoder *network;
	AutoEncoder **p_network;

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

	data_size = wi * hi;

	if (*((int *)filter_desc->filter_params->next->param.pval) == 0)
	{
		network = new AutoEncoder();
		p_network = (AutoEncoder **) &(filter_desc->filter_params->next->param.pval);
		*p_network = network;

		network->build(std::vector<int>{data_size, 500, 30, 500, data_size});

		auto& rbm = network->rbms_[network->max_layer() / 2 - 1];
		rbm->type_ = RBM::Type::LINEAR;

	}
	else
	{
		network = (AutoEncoder *) filter_desc->filter_params->next->param.pval;
	}

	if (g_state == READ_ALL)
	{
		Vector& input = g_data_set_train[g_cont_char];
		input.resize(data_size);

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				input[yi * wi + xi] = ((float)nl_input->neuron_vector[yi * wi + xi].output.ival) / 255.0;
	}
	else if (g_state == PRE_TRAIN)
	{
		LRBM::Conf conf;
		conf.max_epoch_ = 10; conf.max_batches_ = 5; conf.batch_size_ = 10;
		network->pretrain(g_data_set_train, conf, 0);

		conf.max_epoch_ = 10; conf.max_batches_ = 5; conf.batch_size_ = 10;
		network->backprop(g_data_set_train, conf, 0);
	}
	else if (g_state == TRAIN || g_state == TEST)
	{
		static Vector nil;
		Vector input(data_size);
		Vector output(data_size);

		for (yi = 0; yi < hi; yi++)
			for (xi = 0; xi < wi; xi++)
				input[yi * wi + xi] = ((float)nl_input->neuron_vector[yi * wi + xi].output.ival) / 255.0;

		network->predict(input, output, nil);

		for (yo = 0; yo < ho; yo++)
			for (xo = 0; xo < wo; xo++)
				nl_output->neuron_vector[yo * wo + xo].output.fval = (float) (output[yo * wo + xo]);

		update_output_image(get_output_by_neural_layer(nl_output));
	}
	else if (g_state == UNLOAD)
	{
		std::ofstream f("dbn.dat", std::ofstream::binary);
		network->store(f);
	}
	else if (g_state == RELOAD)
	{
		std::ifstream f("dbn.dat", std::ifstream::binary);
		network->load(f);
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
	case READ_ALL:
		set_train_data_set(&character);
		g_cont_char = 0;
		break;
	case PRE_TRAIN:
		set_train_data_set(&character);
		g_cont_char = 0;
		break;
	case TRAIN:
		set_train_data_set(&character);
		g_cont_char = 0;
		break;
	case TEST:
		set_test_data_set(&character);
		g_cont_char = 0;
		break;
	}

	output.ival = 0;
	return (output);
}


/*!
*********************************************************************************
* Function: scale_and_translate_filter						*
* Description: 									*
* Inputs: Filter Descriptor							*
* Output: 				                			*
*********************************************************************************
*/

void
scale_and_translate_filter(FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *nl_input = NULL;
	int i, ho, wo, hi, wi, xo, yo, xi, yi;
	float scale_factor_x, scale_factor_y;

	// Checks Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. Scale filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Inputs Neuron Layers
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Check Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	// Gets the Input Neuron Layer Dimentions
	wi  = nl_input->dimentions.x;
	hi  = nl_input->dimentions.y;

	// Gets the Output Neuron Layer Dimentions
	wo  = filter_desc->output->dimentions.x;
	ho  = filter_desc->output->dimentions.y;

	scale_factor_x = (float) wi / (float) wo;
	scale_factor_y = (float) hi / (float) ho;

	for (xo = 0; xo < wo; xo++)
	{
		xi = scale_factor_x * xo + g_translate_x;

		for (yo = 0; yo < ho; yo++)
		{
			yi = scale_factor_y * yo + g_translate_y;

			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				filter_desc->output->neuron_vector[(yo * wo) + xo].output = nl_input->neuron_vector[(yi * wi) + xi].output;
			else
				filter_desc->output->neuron_vector[(yo * wo) + xo].output.ival = 0;
		}
	}
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

	g_cont_char = g_data_set_order[pParamList->next->param.ival] - 1;
	get_new_char(&character, 0);

	output.ival = 0;
	return (output);
}


int
evaluate_output(OUTPUT_DESC *output, int *result, int *difference_between_highest_second_highest)
{
	NEURON *neuron;

	int i;
	int number, selected_number = 0, selected_number2 = 0;
	int greater = -1;
//	int max_radius, radius;
//	int x, y;

	neuron = output->neuron_layer->neuron_vector;

//	max_radius = output->ww / 2.0;
	for (number = 0; number < 10; number++)
	{
		result[number] = 0;

		for (i = 0; i < (output->ww * output->wh); i++)
		{
/*			x = (i % output->ww) - output->ww / 2;
			y = (i / output->wh) - output->wh / 2;
			radius = sqrt(x*x + y*y);

			if (radius < max_radius)
			{
				if (neuron[i].output.ival == number)
				{
					result[number] += 1;
				}
			}
			else
			{
				neuron[i].output.ival = 0;
			}
*/
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

	greater = -1;
	for (number = 0; number < 10; number++)
	{
		if (number != selected_number)
		{
			if (result[number] > greater)
			{
				greater = result[number];
				selected_number2 = number;
			}
		}
	}
	*difference_between_highest_second_highest = result[selected_number] - result[selected_number2];

	return (selected_number);
}


/*
***********************************************************
* Function: EvaluateChar
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
EvaluateChar(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float minimum_certainty;

	minimum_certainty = pParamList->next->param.fval;

	g_number = evaluate_output(&character_VGRAM_out, g_result, &g_difference_between_highest_second_highest);
	g_certainty = (float) g_difference_between_highest_second_highest / (float) g_result[g_number];
	g_activation = (float) g_result[g_number] / ((float) ((&character_VGRAM_out)->ww * (&character_VGRAM_out)->wh));

	if (g_certainty >= minimum_certainty)
	{
		output.ival = 1;
	}
	else
	{
		output.ival = 0;
	}

	return (output);
}


/*
***********************************************************
* Function: UpdateStats
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
UpdateStats(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	update_stats(g_number, g_label, g_certainty, g_activation, (&character_VGRAM_out)->ww * (&character_VGRAM_out)->wh, g_result);

	g_translate_x = 0;
	g_translate_y = 0;

	output.fval = g_certainty;
	return (output);
}
