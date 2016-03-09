#include <locale.h>
#include <sys/stat.h>
#include <errno.h>
#include "visual_search_user_functions.h"

#define CONFIDENCE_LEVEL 0.41

#define RAMDOM_IMAGES_TRAIN	"random_images_t.txt"
#define RAMDOM_IMAGES_RECALL	"random_images_r.txt"

// Criterions
#define	NONE			-1
#define	PROHIBITORY_AND_SCALED	0
#define	PROHIBITORY		1
#define	FOUND			2


typedef struct
{
	int image;
	double probability;
	double confidence;
	int x;
	int y;
	double scale_factor;
	int found;
} INPUT_DATA;


INPUT_DATA *g_data = NULL;
int g_num_data = 0;

char *g_input_path = NULL;


int prohibitory[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 15, 16}; 				// (circular, white ground with red border)
int prohibitory_size = 12;
int mandatory[] =   {33, 34, 35, 36, 37, 38, 39, 40};					// (circular, blue ground)
int mandatory_size = 8;
int danger[] = 	    {11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; 	// (triangular, white ground with red border)
int danger_size = 15;

int g_nImageOrder = -1;

double g_scale_factor = 0.5;

double g_confidence;

double g_delta_x_d, g_delta_y_d;

int g_flip_horizontaly = 0;
int g_flip_verticaly = 0;

int g_x_trained;
int g_y_trained;

int g_delta_x_x;
int g_delta_x_y;
int g_delta_y_x;
int g_delta_y_y;

int MISS_COUNT = 0;
int HIT_COUNT = 0;


int
is_prohibitory(int ClassID)
{
	int i;
	
	for (i = 0; i < prohibitory_size; i++)
		if (ClassID == prohibitory[i])
			return (1);

	return (0);
}


void
get_traffic_sign_file_name(char *strFileName)
{
	sprintf(strFileName, "%s%05d.ppm", g_input_path, g_data[g_nImageOrder].image);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
}


/*
***********************************************************
* Function: GetNextTrafficSignFileName
* Description:
* Inputs: strFileName -
*	  nDirection -
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

void
GetNextTrafficSignFileName(char *strFileName, int nDirection)
{
	if (nDirection == DIRECTION_FORWARD)
	{
		if (g_nImageOrder == (g_num_data - 1))
			g_nImageOrder = 0;	// first valid image number
		else
			g_nImageOrder++;
	}
	else
	{
		if (g_nImageOrder == 0)
			g_nImageOrder = g_num_data - 1;
		else
			g_nImageOrder--;
	}
	get_traffic_sign_file_name(strFileName);
}


double
estimated_halph_traffic_sign_width(INPUT_DATA *traffic_sign)
{
	return (20.0 / traffic_sign->scale_factor);
}


void
draw_traffic_sign_rectangles(INPUT_DESC *input)
{
	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(1, sizeof(RECTANGLE));

	input->rectangle_list_size = 1;

	input->rectangle_list[0].x = g_data[g_nImageOrder].x - estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));
	input->rectangle_list[0].y = g_data[g_nImageOrder].y - estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));
	input->rectangle_list[0].w = 2.0 * estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));
	input->rectangle_list[0].h = 2.0 * estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));
	input->rectangle_list[0].r = 0.0;
	input->rectangle_list[0].g = 1.0;
	input->rectangle_list[0].b = 0.0;
}


/*
***********************************************************
* Function: LoadImage
* Description: Loads a PNM image to the MAE input
* Inputs: input - input image
*	  strFileName - Image file name
* Output: 0 if OK, -1 otherwise
* Comment: Image format: PNM
***********************************************************
*/

void 
LoadImage(INPUT_DESC *input, char *strFileName)
{
	load_input_image(input, strFileName);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
	update_input_image (input);

	g_scale_factor = g_data[g_nImageOrder].scale_factor * 1.5;
	g_flip_horizontaly = 0;
	g_flip_verticaly = 0;

	draw_traffic_sign_rectangles(input);
	
	input->wxd = g_data[g_nImageOrder].x;
	input->wyd = g_data[g_nImageOrder].y;
	if (input->wxd < 0)
		input->wxd = 0;
	if (input->wxd >= input->ww)
		input->wxd = input->ww - 1;

	if (input->wyd < 0)
		input->wyd = 0;
	if (input->wyd >= input->wh)
		input->wyd = input->wh - 1;
		
	move_input_window(input->name, input->wxd, input->wyd);
}


/*
***********************************************************
* Function: GetNewImage
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
GetNewImage(INPUT_DESC *input, int nDirection)
{
	char strFileName[128];

	GetNextTrafficSignFileName(strFileName, nDirection);
	LoadImage(input, strFileName);
	
	return (0);
}


/*
********************************************************
* Function: make_input_image_visual_search	       *
* Description: 		      			       *
* Inputs: input layer, width, height		       *
* Output: none  				       *
********************************************************
*/

void 
make_input_image_visual_search(INPUT_DESC *input, int w, int h)
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
read_input_data_file()
{
	char *file_name;
	FILE *image_list = NULL;
	char file_line[512];
	int num_lines = 0;

	file_name = "input_data.txt";
	
	image_list = fopen(file_name, "r");
	if (!image_list)
		show_message("Could not read input data file named: ", file_name, "");

	if ((image_list = fopen(file_name, "r")) == NULL)
	{
		printf("Error: could not open file '%s' in read_images_info_file().\n", file_name);
		exit(1);
	}
	
	while (fgets(file_line, 256, image_list))
		num_lines++;

	rewind(image_list);
	
	if (g_data != NULL)
		free(g_data);
		
	g_data = (INPUT_DATA *) calloc(num_lines, sizeof(INPUT_DATA));
	if (!g_data)
	{
		printf("Error: could not allocate memory in LoadInputData().\n");
		exit(1);
	}
	
	for (g_num_data = 0; g_num_data < num_lines; g_num_data++)
	{
		fgets(file_line, 256, image_list);
		// example line: p = 0.5011; confidence = 0.4100; image = 00479; scale_factor = 0.76538; x = 1123; y = 356
		sscanf(file_line, "p = %lf; confidence = %lf; image = %d; scale_factor = %lf; x = %d; y = %d", 
			&(g_data[g_num_data].probability), &(g_data[g_num_data].confidence), &(g_data[g_num_data].image), 
			&(g_data[g_num_data].scale_factor), &(g_data[g_num_data].x), &(g_data[g_num_data].y));
		g_data[g_num_data].scale_factor *= 1.3;
	}
	fclose(image_list);
}

 
/*
********************************************************
* Function: init_user_functions 		       *
* Description:  				       *
* Inputs: none  				       *
* Output:					       *
********************************************************
*/

int 
init_user_functions()
{
	char strCommand[128];
	char *locale_string;
	char strFileName[128];

	locale_string = setlocale (LC_ALL, "C");
	if (locale_string == NULL)
	{
	        fprintf (stderr, "Could not set locale.\n");
	        exit (1);
	}
	else
        	printf ("Locale set to %s.\n", locale_string);
	
	g_input_path = TRAINING_INPUT_PATH;
	read_input_data_file();
	
	GetNextTrafficSignFileName(strFileName, DIRECTION_FORWARD);
	if (strcmp(strFileName, "") != 0)
		LoadImage(&in_pattern, strFileName);

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);
		
	update_input_neurons (&in_pattern);
	all_filters_update();
	all_outputs_update ();
	
	srand(5);
	
	return (0);
}



/*
********************************************************
* Function: init_visual_search			       *
* Description: initialize variables, structures and    *
*	       program procedures		       *
* Inputs: input layer				       *
* Output: none  				       *
********************************************************
*/

void 
init_visual_search(INPUT_DESC *input)
{
#ifndef	NO_INTERFACE
	int x, y;
#endif
	
	make_input_image_visual_search(input, IMAGE_WIDTH, IMAGE_HEIGHT);

#ifndef	NO_INTERFACE
	glutInitWindowSize(input->ww / 2, input->wh / 2);
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
}


static void
move_input(INPUT_DESC *input)
{
	if (input->wxd < 0)
		GetNewImage(input, DIRECTION_REWIND);
	else if (input->wxd >= IMAGE_WIDTH)
		GetNewImage(input, DIRECTION_FORWARD);
	else
	{
		translation_filter_deltaX = (float) input->wxd;
		translation_filter_deltaY = (float) input->wyd;

		check_input_bounds(input, input->wxd, input->wxd);
#ifndef	NO_INTERFACE
		glutSetWindow(input->win);
		input_display();
#endif
		filter_update(get_filter_by_name("in_pattern_translated_filter"));
		filter_update(get_filter_by_name("in_pattern_filtered_translated_filter"));
		filter_update(get_filter_by_name("nl_v1_pattern_filter"));
		all_dendrites_update(); 
		all_neurons_update();
		filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));
		all_outputs_update();
	}
}



/*
********************************************************
* Function: input_generator			       *
* Description: pattern generator		       *
* Inputs: input layer, status			       *
* Output: none  				       *
********************************************************
*/

void 
input_generator(INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_visual_search(input);
#ifndef	NO_INTERFACE
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



/*
********************************************************
* Function: output_handler_max_value_position	       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

void 
output_handler_max_value_position(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_max, v_max, w, h, xi, yi; //, step;
	float current_value, max_value = -FLT_MAX, log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
	int num_max_value = 0;
	double band_width;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name (nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	band_width = output->neuron_layer->dimentions.x / 5;
	g_confidence = 0.0;
	// Finds the max value position
	for (v = 0, u_max = v_max = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			#ifdef	CUDA_COMPILED
				current_value = output->neuron_layer->host_neuron_vector[v * w + u].output.fval;
			#else
				current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			#endif
			
			if (current_value > max_value)
			{
				max_value = current_value;
				u_max = u;
				v_max = v;
				num_max_value = 1;
			}
			else if (current_value == max_value)
			{
				u_max += u;
				v_max += v;
				num_max_value += 1;
			}

			if ((current_value > 0.05) &&
			    (u > (int) ((float) w / 2.0) - 0.5 * band_width) && 
			    (u < (int) ((float) w / 2.0) + 0.5 * band_width)/* && 
			    ((u < (w / 2 - 1)) ||
			     (u > (w / 2 + 1)))*/)
				g_confidence += 1.0;
		}
	}
	u_max /= num_max_value;
	v_max /= num_max_value;
	g_confidence /= (band_width * (double) h);// - 3.0 * (double) h;
	//printf("g_confidence = %lf, band_width = %lf, (double) h = %lf\n", g_confidence, band_width, (double) h);
	
	// Saves the max value
	global_max_value = max_value;
	
	// Map the max value coordinates to image
	map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u_max, v_max, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	#ifdef	CUDA_COMPILED
		nl_target_coordinates->host_neuron_vector[0].output.fval = (float) xi;
		nl_target_coordinates->host_neuron_vector[1].output.fval = (float) yi;
	#else
		nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
		nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
	#endif
}



/*
********************************************************
* Function: output_handler	       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

void 
output_handler_delta_x_y(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int u, v, w, h;
	int current_value;
	char delta_x, delta_y;
		
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	g_delta_x_d = g_delta_y_d = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = output->neuron_layer->neuron_vector[v * w + u].output.ival;
			delta_x = current_value & 0xff;
			delta_y = (current_value >> 8) & 0xff;
			g_delta_x_d += delta_x;
			g_delta_y_d += delta_y;
		}
	}
	g_delta_x_d /= (double) (w * h);
	g_delta_y_d /= (double) (w * h);
}


/*
********************************************************
* Function: saccade				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void 
saccade(INPUT_DESC *input)
{
	float x, y;
	int count = 0;
	double d_delta_x, d_delta_y;
	int delta_x, delta_y;
	
	// Saccade until reach the target
	#ifdef	CUDA_COMPILED
		x = nl_target_coordinates.host_neuron_vector[0].output.fval;
		y = nl_target_coordinates.host_neuron_vector[1].output.fval;
	#else
		x = nl_target_coordinates.neuron_vector[0].output.fval;
		y = nl_target_coordinates.neuron_vector[1].output.fval;
	#endif
	do
	{
		d_delta_x = x / g_scale_factor;
		if (d_delta_x > 0.0)
			delta_x = (int) (d_delta_x + 0.5);
		else if (d_delta_x < 0.0)
			delta_x = (int) (d_delta_x - 0.5);
		else
			delta_x = 0;

		d_delta_y = y / g_scale_factor;
		if (d_delta_y > 0.0)
			delta_y = (int) (d_delta_y + 0.5);
		else if (d_delta_y < 0.0)
			delta_y = (int) (d_delta_y - 0.5);
		else
			delta_y = 0;
			
		
		if (g_flip_horizontaly)
			input->wxd -= delta_x;
		else
			input->wxd += delta_x;

		if (g_flip_verticaly)
			input->wyd -= delta_y;
		else
			input->wyd += delta_y;
			
		move_input_window(input->name, input->wxd, input->wyd);

		count++;
		#ifdef	CUDA_COMPILED
			x = nl_target_coordinates.host_neuron_vector[0].output.fval;
			y = nl_target_coordinates.host_neuron_vector[1].output.fval;
		#else
			x = nl_target_coordinates.neuron_vector[0].output.fval;
			y = nl_target_coordinates.neuron_vector[1].output.fval;
		#endif
	} 
	while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));

	return;
}



/*
********************************************************
* Function: input_controler			       *
* Description: input events handler		       *
* Inputs: input layer, input status		       *
* Output: none  				       *
********************************************************
*/

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



/*
********************************************************
* Function: draw_output  			       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void 
draw_output(char *output_name, char *input_name)
{		
	return;
}


void 
set_neuron_layer_band(NEURON_LAYER *neuron_layer, int x1, int x2, int y1,int y2, float value)
{
	int i, x, y, w, h;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	for (i = 0; i < w*h; i++)
		#ifdef	CUDA_COMPILED
			neuron_layer->host_neuron_vector[i].output.fval = .0f;
		#else
			neuron_layer->neuron_vector[i].output.fval = .0f;
		#endif

	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				#ifdef	CUDA_COMPILED
					neuron_layer->host_neuron_vector[x + w * y].output.fval = value;
				#else
					neuron_layer->neuron_vector[x + w * y].output.fval = value;
				#endif
}


/*
********************************************************
* Function: f_keyboard  			       *
* Description: keyboard events handler  	       *
* Inputs: key_value (pointer to pressed character)     *
* Output: none  				       *
********************************************************
*/

void 
f_keyboard(char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
		// Move input
		case 'J':
		case 'j':
			in_pattern.wxd--;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'K':
		case 'k':
			in_pattern.wxd++;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'I':
		case 'i':
			in_pattern.wyd++;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'M':
		case 'm':
			in_pattern.wyd--;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;
		// Clear neural networks' memory
		case 'F':
		case 'f':
			clear_neural_layers_memory("nl_v1_activation_map");
			break;
		// Train network
		case 'T':	
		case 't':
			g_x_trained = in_pattern.wxd;
			g_y_trained = in_pattern.wyd;
			set_neuron_layer_band(&nl_v1_activation_map, (NL_WIDTH - ACT_BAND_WIDTH)/2 , (NL_WIDTH + ACT_BAND_WIDTH)/2, 0, NL_HEIGHT, HIGHEST_OUTPUT);

			filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));
			train_neuron_layer("nl_v1_activation_map");

			all_outputs_update ();
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade(&in_pattern);
			//printf("g_scale_factor = %lf\n", g_scale_factor);
			break;	
		// Flip image
		case 'H':
		case 'h':
			g_flip_horizontaly = 1;
			g_flip_verticaly = 0;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		case 'V':
		case 'v':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 1;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Go back to initial position
		case 'U':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wxd = g_data[g_nImageOrder].x;
			in_pattern.wyd = g_data[g_nImageOrder].y;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Go back to trained position
		case 'u':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wxd = g_x_trained;
			in_pattern.wyd = g_y_trained;
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Save delta x
		case 'X':
		case 'x':
			g_delta_x_x = g_x_trained - in_pattern.wxd;
			g_delta_x_y = g_y_trained - in_pattern.wyd;
			break;	
		// Save delta y
		case 'Y':
		case 'y':
			g_delta_y_x = g_x_trained - in_pattern.wxd;
			g_delta_y_y = g_y_trained - in_pattern.wyd;
			break;
		// Save new traffic sign center
		case 'W':
		case 'w':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			g_data[g_nImageOrder].x -= (int) ((double) g_delta_x_x / 2.0 + 0.5);
			g_data[g_nImageOrder].y -= (int) ((double) g_delta_y_y / 2.0 + 0.5);
			g_data[g_nImageOrder].found = 1;
			in_pattern.wxd = g_data[g_nImageOrder].x;
			in_pattern.wyd = g_data[g_nImageOrder].y;
			// TODO: check_if_it_was_a_hit_and_print_info_if_not();
			draw_traffic_sign_rectangles(&in_pattern);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Go to final position after centering
		case 'G':
		case 'g':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wxd = g_data[g_nImageOrder].x - (int) ((double) g_delta_x_x / 2.0 + 0.5);
			in_pattern.wyd = g_data[g_nImageOrder].y - (int) ((double) g_delta_y_y / 2.0 + 0.5);
			draw_traffic_sign_rectangles(&in_pattern);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		case 'R':
		case 'r':
			printf("Could not find traffic sign center...\n");
			break;	
	}
	
	return;
}


NEURON_OUTPUT 
GetImage(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int image_order;
	
	image_order = pParamList->next->param.ival;
	if (image_order < g_num_data)
	{
		g_nImageOrder = image_order;
		get_traffic_sign_file_name(strFileName);
		LoadImage(&in_pattern, strFileName);
	}
	else
	{
		sprintf(strFileName, "%d", image_order);
		show_message("Could not find image of order: ", strFileName, "");
	}
	
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT 
GetNumImages(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = g_num_data;
	fprintf(stderr, "Number of traffic signs detected present in the input_data.txt = %d\n", g_num_data);
	
	return (output);
}


NEURON_OUTPUT 
MoveToPoint(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = pParamList->next->param.ival;
	in_pattern.wyd = pParamList->next->next->param.ival;
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT 
GetDeltaMove(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = (int) (1.0 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));
	//printf("delta = %d\n", output.ival);
	
	return (output);
}


NEURON_OUTPUT 
SetScaleFactor(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_scale_factor = pParamList->next->param.fval;

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT 
DeltaXYOK(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	double halph_traffic_sign_width_in_pixels;
	
	halph_traffic_sign_width_in_pixels = estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));

	if ((abs(g_delta_x_y) < (int) (halph_traffic_sign_width_in_pixels / 5.0 + 0.5)) &&
	    (abs(g_delta_y_x) < (int) (halph_traffic_sign_width_in_pixels / 5.0 + 0.5)) &&
	    (abs(g_delta_x_x) < (int) (halph_traffic_sign_width_in_pixels * 1.0 + 0.5)) &&
	    (abs(g_delta_y_y) < (int) (halph_traffic_sign_width_in_pixels * 1.0 + 0.5)))
	{
		//printf("DeltaXYOK = 1\n");
	    	output.ival = 1;
	}
	else
	{
		//printf("DeltaXYOK = 0\n");
		output.ival = 0;
	}
	
	return (output);
}


NEURON_OUTPUT 
MoveRamdomXY(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	double halph_traffic_sign_width_in_pixels;
	int random_number_of_pixels;
	
	g_flip_horizontaly = 0;
	g_flip_verticaly = 0;

	halph_traffic_sign_width_in_pixels = estimated_halph_traffic_sign_width(&(g_data[g_nImageOrder]));
	random_number_of_pixels = (((double) rand() / (double) RAND_MAX) - 0.5) * halph_traffic_sign_width_in_pixels / 2.0;
	in_pattern.wxd = g_data[g_nImageOrder].x + random_number_of_pixels;
	random_number_of_pixels = (((double) rand() / (double) RAND_MAX) - 0.5) * halph_traffic_sign_width_in_pixels / 2.0;
	in_pattern.wyd = g_data[g_nImageOrder].y + random_number_of_pixels;
	
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT 
ScaleUp(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_data[g_nImageOrder].scale_factor = g_data[g_nImageOrder].scale_factor * 0.9;
	g_scale_factor = g_data[g_nImageOrder].scale_factor * 1.5;
	draw_traffic_sign_rectangles(&in_pattern);
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


int 
max(int a, int b)
{
	return (a < b) ? b : a; // or: return comp(a,b)?b:a; for version (2)
}


int 
min(int a, int b)
{
	return (b < a) ? b : a; // or: return !comp(b,a)?a:b; for version (2)
}


double 
getJaccardCoefficient(int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow)
{
	double jaccCoeff = 0.;

	if (!(leftCol > gtRightCol || rightCol < gtLeftCol || topRow > gtBottomRow || bottomRow < gtTopRow))
	{
		int interLeftCol = max(leftCol, gtLeftCol);
		int interTopRow = max(topRow, gtTopRow);
		int interRightCol = min(rightCol, gtRightCol);
		int interBottomRow = min(bottomRow, gtBottomRow);

		const double areaIntersection = (abs(interRightCol - interLeftCol) + 1) * (abs(interBottomRow - interTopRow) + 1);
		const double lhRoiSize = (abs(rightCol - leftCol) + 1) * (abs(bottomRow - topRow) + 1);
		const double rhRoiSize = (abs(gtRightCol - gtLeftCol) + 1) * (abs(gtBottomRow - gtTopRow) + 1);

		jaccCoeff = areaIntersection / (lhRoiSize + rhRoiSize - areaIntersection);
	}

	return jaccCoeff;
};


void
remove_traffic_signs_detected_multiple_times()
{
	int i, j;
	int x1, y1, x2, y2, x1_i, y1_i, x2_i, y2_i, x1_j, y1_j, x2_j, y2_j;
	double similarity_ij;

	for (i = 0; i < g_num_data; i++)
	{
		if (g_data[i].found)
		{
			x1 = g_data[i].x - (int) (estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			y1 = g_data[i].y - (int) (estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			x2 = x1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			y2 = y1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);

			x1_i = x1;
			y1_i = (IMAGE_HEIGHT - 1) - y2;
			x2_i = x2;
			y2_i = (IMAGE_HEIGHT - 1) - y1;
			j = i + 1;
			while ((j < g_num_data) && (g_data[j].found == 1) && (g_data[j].image == g_data[i].image))
			{
				x1 = g_data[j].x - (int) (estimated_halph_traffic_sign_width(&(g_data[j])) + 0.5);
				y1 = g_data[j].y - (int) (estimated_halph_traffic_sign_width(&(g_data[j])) + 0.5);
				x2 = x1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[j])) + 0.5);
				y2 = y1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[j])) + 0.5);

				x1_j = x1;
				y1_j = (IMAGE_HEIGHT - 1) - y2;
				x2_j = x2;
				y2_j = (IMAGE_HEIGHT - 1) - y1;
				similarity_ij = getJaccardCoefficient(x1_i, y1_i, x2_i, y2_i, x1_j, y1_j, x2_j, y2_j);
				if (similarity_ij > 0.3)
				{
					if (g_data[i].probability > g_data[j].probability)
					{
						g_data[j].found = 0;
					}
					else if (g_data[i].probability == g_data[j].probability)
					{
						if (estimated_halph_traffic_sign_width(&(g_data[i])) < estimated_halph_traffic_sign_width(&(g_data[j])))
							g_data[j].found = 0;
						else
							g_data[i].found = 0;
					}
					else
						g_data[i].found = 0;
				}
				j++;
			}
		}
	}
}

/*
void
print_info_about_not_detected_traffic_signs()
{
	load_ground_thruth();
	marc_traffic_signs_found();
	for (i = 0; i < ground_thruth_size; i++)
	{
		if ((info_ground_thruth[i].found == 0) &&
		    (info_ground_thruth[i].image >= 259) && is_prohibitory(&(info_ground_thruth[i])))
		{
			printf("NOT FOUND image = %5d; ", info_ground_thruth[i].image);
			for (j = 0; j < g_num_data; j++)
			{
				if ((info_ground_thruth[i].image == g_data[j].image) &&
				    distance_match(&(info_ground_thruth[i]), &(g_data[j])) &&
				    size_match(&(info_ground_thruth[i]), &(g_data[j])))
					printf("near hit: dx(%%w/2) = %lf, dy(%%w/2) = %lf, size gt = %d, size ex = %d; ", 
						get_dx(&(info_ground_thruth[i]), &(g_data[j])),
						get_dy(&(info_ground_thruth[i]), &(g_data[j])),
						info_ground_thruth[i].width,
						g_data[j].width);
			}
		}   
	}
	Tenter fazer o que esta funcao preve no olho. 
	Rodar e imprimir as ground thruths nao achadas;
	As imagens que tem sinais que nao foram detectados pelo tfd;
	As imagenes (linhas do arquivo vindo do tfd) que nao foram resolvidas via simetria;
	As imagens (linhas do arquivo vindo do tfd) com trechos (nao traffic signs) que foram erradamente achados simetricos;
	Se o problema frequentemente for o tamanho informado (w), usar a simetria para achar o tamanho certo.
}
*/

NEURON_OUTPUT 
SaveGTDSBReasultsFile(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	int x1, y1, x2, y2, x1_i, y1_i, x2_i, y2_i;
	FILE *gtdsb_result_file;
		
	remove_traffic_signs_detected_multiple_times();

	//print_info_about_not_detected_traffic_signs();

	gtdsb_result_file = fopen("output_data.txt", "w");
	for (i = 0; i < g_num_data; i++)
	{
		if (g_data[i].found)
		{
			x1 = g_data[i].x - (int) (estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			y1 = g_data[i].y - (int) (estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			x2 = x1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);
			y2 = y1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_data[i])) + 0.5);

			x1_i = x1;
			y1_i = (IMAGE_HEIGHT - 1) - y2;
			x2_i = x2;
			y2_i = (IMAGE_HEIGHT - 1) - y1;

			fprintf(gtdsb_result_file, "%05d.ppm;%d;%d;%d;%d\n", g_data[i].image, x1_i, y1_i, x2_i, y2_i);
		}
	}
	fclose(gtdsb_result_file);
	
	output.ival = 0;
	
	return (output);
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
SetNetworkStatus(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			g_input_path = TRAINING_INPUT_PATH;
			break;
		case RECALL_PHASE:
			g_input_path = REACALL_INPUT_PATH;
			break;
	}
	
	output.ival = 0;

	return (output);
}
