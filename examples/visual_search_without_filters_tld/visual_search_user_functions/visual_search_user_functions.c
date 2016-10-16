#include <locale.h>
#include <sys/stat.h>
#include <errno.h>
#include "visual_search_user_functions.h"

#define CONFIDENCE_LEVEL 0.40


typedef struct
{
	int ImgNo, ClassID;
	float leftCol, topRow, rightCol, bottomRow;
	int found;
	int used_for_trainning;
	double best_confidence;
	int best_x;
	int best_y;
	double best_scale_factor;
} IMAGE_INFO;

char *g_input_path = NULL;
IMAGE_INFO *g_image_info = NULL;
int g_num_image_info = 0;

int g_nImageOrder = -1;
int g_nCurrentImageInfo = -1;

int g_NumTrafficSignsSearched = 0;
int g_NumTrafficSignsFound = 0;

int g_nNetworkStatus;

double g_scale_factor = 0.5;

double g_confidence;

double g_halph_band_width = 1.0;

int g_numImage;


void
read_images_info_file(char *file_name)
{
	FILE *image_list = NULL;
	char file_line[5000];
	int num_lines = 0;

	if ((image_list = fopen(file_name, "r")) == NULL)
	{
		printf("Error: could not open file '%s' in read_images_info_file().\n", file_name);
		exit(1);
	}
	
	while (fgets(file_line, 256, image_list))
		num_lines++;

	rewind(image_list);
	
	if (g_image_info != NULL)
		free(g_image_info);
		
	g_image_info = (IMAGE_INFO *) calloc(num_lines, sizeof(IMAGE_INFO));
	if (!g_image_info)
	{
		printf("Error: could not allocate memory in read_images_info_file().\n");
		exit(1);
	}
	
	for (g_num_image_info = 0; g_num_image_info < num_lines; g_num_image_info++)
	{
		fgets(file_line, 256, image_list);
		if (g_nStatus == TRAINING_PHASE)
		{
			g_image_info[g_num_image_info].ImgNo = g_num_image_info + 1;
			sscanf(file_line, "%f,%f,%f,%f", &(g_image_info[g_num_image_info].leftCol),
								   &(g_image_info[g_num_image_info].topRow), &(g_image_info[g_num_image_info].rightCol), 
								   &(g_image_info[g_num_image_info].bottomRow));
		}
		else // RECALL_PHASE
		{
			sscanf(file_line, "%d.jpg.pnm", &(g_image_info[g_num_image_info].ImgNo));
			g_image_info[g_num_image_info].leftCol = -1.0;
			g_image_info[g_num_image_info].topRow = -1.0;
			g_image_info[g_num_image_info].rightCol = -1.0;
			g_image_info[g_num_image_info].bottomRow = -1.0;
			g_image_info[g_num_image_info].ClassID = -1;
			g_image_info[g_num_image_info].found = -1;
			g_image_info[g_num_image_info].used_for_trainning = -1;
			g_image_info[g_num_image_info].best_confidence = -1.0;
			g_image_info[g_num_image_info].best_x = -1;
			g_image_info[g_num_image_info].best_y = -1;
			g_image_info[g_num_image_info].best_scale_factor = -1.0;
		}
	}
	fclose(image_list);
}


void
get_traffic_sign_file_name(char *strFileName)
{
	int sample_num = -1;
	int last_image_with_traffic_sign_within_criterion;
	int i;

	if (g_nStatus == TRAINING_PHASE)
	{
		g_nCurrentImageInfo = 0;
		while (g_nCurrentImageInfo < g_num_image_info)
		{
			i = last_image_with_traffic_sign_within_criterion = g_nCurrentImageInfo;
			while ((i < g_num_image_info) && (g_image_info[i].ImgNo == g_image_info[last_image_with_traffic_sign_within_criterion].ImgNo))
				i++;

			sample_num++;
			if (sample_num == g_nImageOrder)
				break;
			else
				g_nCurrentImageInfo = i;
		}
	}
	else // RECALL_PHASE
		g_nCurrentImageInfo = g_nImageOrder;

	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
}


int
get_specific_type_traffic_sign_file_name(char *strFileName, int type)
{
	int sample_num = -1;
	int found = 0;

	g_nCurrentImageInfo = 0;
	while (g_nCurrentImageInfo < g_num_image_info)
	{
		if ((g_image_info[g_nCurrentImageInfo].ClassID == type))
		{
			found = 1;
			break;
		}
		g_nCurrentImageInfo++;
	}
	
	if (found)
		g_nImageOrder = sample_num;	
	
	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
	
	return (found);
}


int
get_specific_type_traffic_sign_of_specific_size_range_file_name(char *strFileName, int type, int min_width, int max_width)
{
	int sample_num = -1;
	int found = 0;
	int width;

	g_nCurrentImageInfo = 0;
	while (g_nCurrentImageInfo < g_num_image_info)
	{
		width = abs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
		if ((g_image_info[g_nCurrentImageInfo].ClassID == type) &&
		    (min_width <= width) &&
		    (width <= max_width))
		{
			found = 1;
			break;
		}
		g_nCurrentImageInfo++;
	}
	
	if (found)
		g_nImageOrder = sample_num;	
	
	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
	
	return (found);
}


int
get_best_specific_type_traffic_sign_file_name(char *strFileName, int type, int first_considered, int last_considered)
{
	int i;
	int width;
	int best_width;
	int found;

	i = 0;
	while (g_image_info[i].ImgNo < first_considered)
		i++;
	
	g_nCurrentImageInfo = -1;
	best_width = 500;
	while (g_image_info[i].ImgNo <= last_considered)
	{
		if (g_image_info[i].ClassID == type)
		{
			width = g_image_info[i].rightCol - g_image_info[i].leftCol;
			if (abs(width - 50) < best_width)
			{
				best_width = abs(width - 50);
				g_nCurrentImageInfo = i;
			}
		}
		i++;
	}
	
	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
	
	if (g_nCurrentImageInfo != -1)
		found = 1;
	else
		found = 0;
		
	return (found);
}


int
get_best_specific_type_traffic_sign_file_name_and_not_trained(char *strFileName, int type, int first_considered, int last_considered)
{
	int i;
	int width;
	int best_width;
	int found;

	i = 0;
	while (g_image_info[i].ImgNo < first_considered)
		i++;
	
	g_nCurrentImageInfo = -1;
	best_width = 500;
	while (g_image_info[i].ImgNo <= last_considered)
	{
		if (g_image_info[i].ClassID == type && g_image_info[i].used_for_trainning!=1) 
		{
			width = g_image_info[i].rightCol - g_image_info[i].leftCol;
			if (abs(width - 50) < best_width)
			{
				best_width = abs(width - 50);
				g_nCurrentImageInfo = i;
			}
		}
		i++;
	}
	
	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	printf("Image file name: %s\n", strFileName);
	fflush(stdout);
	
	if (g_nCurrentImageInfo != -1)
		found = 1;
	else
		found = 0;
		
	return (found);
}


void
GetNextTrafficSignFileName(char *strFileName, int nDirection)
{
	if (nDirection == DIRECTION_FORWARD)
	{
		g_nImageOrder++;
	}
	else
	{
		g_nImageOrder--;
	}
	get_traffic_sign_file_name(strFileName);
}


void
draw_traffic_sign_rectangles(INPUT_DESC *input)
{
	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(2, sizeof(RECTANGLE));

	input->rectangle_list_size = 1;

	input->rectangle_list[0].x = g_image_info[g_nCurrentImageInfo].leftCol;
	input->rectangle_list[0].y = (IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow;
	input->rectangle_list[0].w = g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol;
	input->rectangle_list[0].h = g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow;
	input->rectangle_list[0].r = 1.0;
	input->rectangle_list[0].g = 0.0;
	input->rectangle_list[0].b = 0.0;
}


void
reset_gaussian_filter_parameters()
{
	g_sigma = sqrt(g_scale_factor * 0.8 * 128.0 / 35.0);
	g_kernel_size = (int) (6.0 * g_sigma);
	g_kernel_size = ((g_kernel_size % 2) == 0)? g_kernel_size + 1: g_kernel_size;
	if (g_sigma < 1.0)
	{
		g_sigma = 1.0;
		g_kernel_size = 5.0;
	}
}


void 
LoadImage(INPUT_DESC *input, char *strFileName)
{
	load_input_image(input, strFileName);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
	update_input_image (input);

	draw_traffic_sign_rectangles(input);
	g_scale_factor = 15.0 / ((double) (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol) / 2.0); // @@@ Alberto: Este g_scale_factor deve ser compativel com a band_width durante o treinamento.
	reset_gaussian_filter_parameters();
}


int 
GetNewImage(INPUT_DESC *input, int nDirection)
{
	char strFileName[128];

	GetNextTrafficSignFileName(strFileName, nDirection);
	LoadImage(input, strFileName);
	
	return (0);
}


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
	
	g_nStatus = TRAINING_PHASE;
	strcpy(g_strRandomImagesFileName, RAMDOM_IMAGES_TRAIN);
	g_input_path = TRAINING_INPUT_PATH;
	read_images_info_file(g_strRandomImagesFileName);
	GetNextTrafficSignFileName(strFileName, DIRECTION_FORWARD);
	if (strcmp(strFileName, "") != 0)
		LoadImage(&in_pattern, strFileName);

	sprintf (strCommand, "move %s to %d, %d;", in_pattern.name, in_pattern.wxd, in_pattern.wyd);
	interpreter (strCommand);
	
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


void 
input_generator(INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_visual_search(input);
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
output_handler_max_value_position(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_max, v_max, w, h, xi, yi; //, step;
	float current_value, max_value = -FLT_MAX, log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
	int num_max_value = 0;
	NEURON *unfiltered_activation_map;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name(nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	unfiltered_activation_map = nl_v1_activation_map.neuron_vector;
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

			if ((unfiltered_activation_map[v * w + u].output.fval > 0.0) &&
			    (u > (int) ((float) (w-1) / 2.0) - g_halph_band_width) && 
			    (u < (int) ((float) (w-1) / 2.0) + g_halph_band_width))
			{
				g_confidence += 1.0;
			}
		}
	}
	u_max /= num_max_value;
	v_max /= num_max_value;
	g_confidence /= ((g_halph_band_width - 0.5) * 2.0 * (double) h);
	// printf("g_confidence = %lf, g_halph_band_width = %lf, (double) h = %lf\n", g_confidence, g_halph_band_width, (double) h);
	
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
//	do
//	{
		d_delta_x = x / g_scale_factor;
		if (d_delta_x > 0.0)
			delta_x = (int) (d_delta_x + 0.5);
		else if (d_delta_x < 0.0)
			delta_x = (int) (d_delta_x - 0.5);
		else
			delta_x = 0;
		input->wxd += delta_x;
		
		if (input->wxd >= IMAGE_WIDTH)
			input->wxd = IMAGE_WIDTH - 1;		
		if (input->wxd < 0)
			input->wxd = 0;		
		
		d_delta_y = y / g_scale_factor;
		if (d_delta_y > 0.0)
			delta_y = (int) (d_delta_y + 0.5);
		else if (d_delta_y < 0.0)
			delta_y = (int) (d_delta_y - 0.5);
		else
			delta_y = 0;
		input->wyd += delta_y;
		
		if (input->wyd >= IMAGE_HEIGHT)
			input->wyd = IMAGE_HEIGHT - 1;		
		if (input->wyd < 0)
			input->wyd = 0;		
		
		move_input_window(input->name, input->wxd, input->wyd);

		count++;
		#ifdef	CUDA_COMPILED
			x = nl_target_coordinates.host_neuron_vector[0].output.fval;
			y = nl_target_coordinates.host_neuron_vector[1].output.fval;
		#else
			x = nl_target_coordinates.neuron_vector[0].output.fval;
			y = nl_target_coordinates.neuron_vector[1].output.fval;
		#endif
//	} 
//	while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));

	return;
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


float 
normal_dist(float x, float mean, float dev)
{
	float e = 2.71829f;

	return (1.0/sqrt(2.0*pi*dev*dev))*(pow(e, ((-1.0)*(((x-mean)*(x-mean))/(2.0*dev*dev)))));
}


float 
triangular_dist(float x, float mean, float dev)
{
	float inverse_distance;
	
	if (fabs(mean - x) < (dev * 0.5))
		inverse_distance = 1.0;
	else if (fabs(mean - x) < (dev * 0.6))
		inverse_distance = 0.9;
	else if (fabs(mean - x) < (dev * 0.7))
		inverse_distance = 0.8;
	else if (fabs(mean - x) < (dev * 0.8))
		inverse_distance = 0.6;
	else if (fabs(mean - x) < (dev * 0.9))
		inverse_distance = 0.3;
	else if (fabs(mean - x) < (dev * 1.0))
		inverse_distance = 0.1;
	else
		inverse_distance = 0.0;

	return (inverse_distance);
}


void
init_nl_landmark_eval_mask_for_log_polar(NEURON_LAYER *nl_landmark_eval_mask)
{
	int w, h, x, y;
	NEURON *neuron_vector;
	
	neuron_vector = nl_landmark_eval_mask->neuron_vector;
	w = nl_landmark_eval_mask->dimentions.x;
	h = nl_landmark_eval_mask->dimentions.y;
	g_halph_band_width = 7.0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			//neuron_vector[y * w + x].output.fval = normal_dist((float) x, (float) (w-1) / 2.0, 4.0);
			neuron_vector[y * w + x].output.fval = triangular_dist((float) x, (float) (w-1) / 2.0, g_halph_band_width);
		}
	}
}


void
train_traffic_sign()
{
	g_nNetworkStatus = TRAINNING;
/*	set_neuron_layer_band (&nl_v1_activation_map, // @@@ Alberto: Esta faixa na camada deve ser compativel com o calculo da confidencia e g_scale_factor
			      (NL_WIDTH - g_scale_factor * 0.4 * (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol)) / 2, 
			      (NL_WIDTH + g_scale_factor * 0.4 * (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol)) / 2, 0, NL_HEIGHT, 1.0);
	filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));
*/
	init_nl_landmark_eval_mask_for_log_polar(&nl_v1_activation_map_f);
	train_neuron_layer("nl_v1_activation_map");

	g_image_info[g_nCurrentImageInfo].used_for_trainning = 1;
	printf("Imagem Treinada = %d\n", g_image_info[g_nCurrentImageInfo].ImgNo);

	all_outputs_update ();
	g_nNetworkStatus = RUNNING;
}	


void 
f_keyboard(char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
		// Train network
		case 'T':	
		case 't':
			train_traffic_sign();
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade(&in_pattern);
			//printf("image = %05d; scale_factor = %.5lf; x = %4d; y = %3d; confidence = %.2lf; HIT;  T.S. Type = %2d; Width = %3d\n", 
			//	g_image_info[g_nCurrentImageInfo].ImgNo, g_scale_factor, in_pattern.wxd, in_pattern.wyd, g_confidence, 
			//	g_image_info[g_nCurrentImageInfo].ClassID, g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
			break;	
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
		case 'F':
		case 'f':
			printf("The network has forgotten the training\n");
			clear_neural_layers_memory("nl_v1_activation_map");
			break;
	}
	
	return;
}


NEURON_OUTPUT 
get_target_coordinates(PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *x, *y;
	
	// Get the Address of the Target Coordenates
	x = (int *) param_list->next->param.pval;
	y = (int *) param_list->next->next->param.pval;
	
	*x = g_nNoseX;
	*y = g_nNoseY;
	
	// Zero returning on default
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT 
SetNetworkStatus(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			strcpy(g_strRandomImagesFileName, RAMDOM_IMAGES_TRAIN);
			g_input_path = TRAINING_INPUT_PATH;
			break;
		case RECALL_PHASE:
			g_input_path = REACALL_INPUT_PATH;
			strcpy(g_strRandomImagesFileName, RAMDOM_IMAGES_RECALL);
			break;
	}
	read_images_info_file(g_strRandomImagesFileName);
	
	output.ival = 0;

	return (output);
}


NEURON_OUTPUT 
GetImage(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int image_order;
	
	image_order = pParamList->next->param.ival;
	g_nImageOrder = image_order;
	get_traffic_sign_file_name(strFileName);
	LoadImage(&in_pattern, strFileName);
	output.ival = g_image_info[g_nCurrentImageInfo].ClassID;
	
	return (output);
}


int
get_image_order_by_image_info_number(int image_info_number)
{
	int i;
	int num_samples;
	
	i = num_samples = 0; 
	while (i < image_info_number)
	{
		num_samples++;
		i++;
	}

	return (num_samples);
}


NEURON_OUTPUT 
GetImageByNumberImage(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int image_number;
	int i;
	
	output.ival = -1;
	image_number = pParamList->next->param.ival;
	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, image_number);
	printf("NOME DO ARQUIVO -> %s\n", strFileName);


	for ( i = 0; i < g_num_image_info; i++)
	{
		if (g_image_info[i].ImgNo == image_number)
		{	
			g_nCurrentImageInfo = i;
			g_nImageOrder = get_image_order_by_image_info_number(g_nCurrentImageInfo);
			LoadImage(&in_pattern, strFileName);
			output.ival = g_image_info[i].ClassID;
			break;
		}
	}

	g_numImage = image_number;
	if (output.ival == -1)
	{
		g_nCurrentImageInfo = -2;
		LoadImage(&in_pattern, strFileName);
	}


	return (output);
}


NEURON_OUTPUT 
GetImageByNumber(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int image_number;
	int i;
	
	image_number = pParamList->next->param.ival;
	output.ival = -1;
	for (i = 0; i < g_num_image_info; i++)
	{
		if (g_image_info[i].ImgNo == image_number)
		{
			g_nCurrentImageInfo = i;
			g_nImageOrder = get_image_order_by_image_info_number(g_nCurrentImageInfo);
			get_traffic_sign_file_name(strFileName);
			LoadImage(&in_pattern, strFileName);
			output.ival = g_image_info[g_nCurrentImageInfo].ClassID;
			break;
		}
	}
	
	return (output);
}


NEURON_OUTPUT 
MoveToTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
	in_pattern.wyd = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2;
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
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
MoveToPanda(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = (g_image_info[g_nCurrentImageInfo-1].leftCol + g_image_info[g_nCurrentImageInfo-1].rightCol) / 2;
	in_pattern.wyd = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo-1].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo-1].bottomRow)) / 2;
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT 
GetDeltaMove(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = (int) (0.5 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));
//	output.ival = (int) (1.0 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));
	//printf("delta = %d\n", output.ival);
	
	return (output);
}


NEURON_OUTPUT 
TrainTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	train_traffic_sign();

	output.ival = 0;
	
	return (output);
}


int
max_i(int a, int b)
{
	return (a < b) ? b : a;
}


int
min_i(int a, int b)
{
	return (b < a) ? b : a;
}


double
getJaccardCoefficient(int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow)
{
	double jaccCoeff = 0.;

	if (!(leftCol > gtRightCol || rightCol < gtLeftCol || topRow > gtBottomRow || bottomRow < gtTopRow))
	{
		int interLeftCol = max_i(leftCol, gtLeftCol);
		int interTopRow = max_i(topRow, gtTopRow);
		int interRightCol = min_i(rightCol, gtRightCol);
		int interBottomRow = min_i(bottomRow, gtBottomRow);

		const double areaIntersection = (abs(interRightCol - interLeftCol) + 1) * (abs(interBottomRow - interTopRow) + 1);
		const double lhRoiSize = (abs(rightCol - leftCol) + 1) * (abs(bottomRow - topRow) + 1);
		const double rhRoiSize = (abs(gtRightCol - gtLeftCol) + 1) * (abs(gtBottomRow - gtTopRow) + 1);

		jaccCoeff = areaIntersection / (lhRoiSize + rhRoiSize - areaIntersection);
	}

	return jaccCoeff;
};


double
estimated_halph_traffic_sign_width()
{
	return (20.0 / g_scale_factor);
}


NEURON_OUTPUT 
CheckTrafficSignDetection(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int traffic_sign_center_x;
	int traffic_sign_center_y;
	int largura;
	int altura;
	int i;
	int found = 0;
	float jaccard;
	int x1, y1, x2, y2, x1_j, y1_j, x2_j, y2_j;
	
	if (g_nStatus == TRAINING_PHASE)
	{
		for (i = ((g_nCurrentImageInfo - 10) > 0)? g_nCurrentImageInfo - 10: 0;
		     (i < (g_nCurrentImageInfo + 10)) && i < g_num_image_info;
		     i ++)
		{
			if (g_image_info[i].ImgNo == g_image_info[g_nCurrentImageInfo].ImgNo)
			{

/*				traffic_sign_center_x = (int) ((double) (g_image_info[i].leftCol + g_image_info[i].rightCol) / 2.0 + 0.5);
				traffic_sign_center_y = (int) ((double) (((IMAGE_HEIGHT - 1) - g_image_info[i].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[i].bottomRow)) / 2.0 + 0.5);
				largura = ((int) (0.5 * ((double) (g_image_info[i].topRow - g_image_info[i].bottomRow) / 2.0 + 0.5)));
				altura = ((int) (0.5 * ((double) (g_image_info[i].rightCol - g_image_info[i].leftCol) / 2.0 + 0.5)));
				largura = abs(largura);
				altura = abs(altura);
*/

				x1 = in_pattern.wxd - (int) (estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
				y1 = in_pattern.wyd - (int) (estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
				x2 = x1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
				y2 = y1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
				x1_j = x1;
				y1_j = (IMAGE_HEIGHT - 1) - y2;
				x2_j = x2;
				y2_j = (IMAGE_HEIGHT - 1) - y1;

				jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

//				if ((abs(in_pattern.wxd - traffic_sign_center_x) <= altura)
//				&& (abs(in_pattern.wyd - traffic_sign_center_y) <= largura))
				if (jaccard >= 0.5)
				{
					printf("FOUND\n");

					g_image_info[i].found = found = 1;
					if (g_confidence > g_image_info[i].best_confidence)
					{
						g_image_info[i].best_confidence = g_confidence;
						g_image_info[i].best_scale_factor = g_scale_factor;
						g_image_info[i].best_x = in_pattern.wxd;
						g_image_info[i].best_y = in_pattern.wyd;
					}
					break;
				}
			}
		}
	}

    if (found)
    	printf("image = %05d; scale_factor = %.5lf; x = %4d; y = %3d; confidence = %.2lf; T.S. Type = %2d; x_g = %3d; y_g = %3d; width %3d; hight %3d; HIT\n",
    			g_image_info[i].ImgNo, g_scale_factor, in_pattern.wxd, in_pattern.wyd, g_confidence,
    			g_image_info[i].ClassID, traffic_sign_center_x, traffic_sign_center_y,
    			(int)(g_image_info[i].rightCol - g_image_info[i].leftCol), (int)(g_image_info[i].bottomRow - g_image_info[i].topRow));
    else
    	printf("image = %05d; scale_factor = %.5lf; x = %4d; y = %3d; confidence = %.2lf; T.S. Type = -1; x_g = -001; y_g = -01; width -01; hight -01; MISS\n",
    		   	g_image_info[g_nCurrentImageInfo].ImgNo, g_scale_factor, in_pattern.wxd, in_pattern.wyd, g_confidence);

    fflush(stdout);
	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
SaveTLDReasultsFile(PARAM_LIST *pParamList)
{
    NEURON_OUTPUT output;
    int i;
    int x1, y1, x2, y2, x1_i, y1_i, x2_i, y2_i;
    float jaccard;
    int traffic_sign_center_x;
    int traffic_sign_center_y;
    FILE *result_file;

    //remove_traffic_signs_detected_multiple_times();

    result_file = fopen("resultadoTLD.txt", "w");
    for (i = 0; i < g_num_image_info; i++)
    {
    	x1 = g_image_info[i].best_x - (int) (estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
        y1 = g_image_info[i].best_y - (int) (estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
        x2 = x1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);
        y2 = y1 + (int) (2.0 * estimated_halph_traffic_sign_width(&(g_image_info[i])) + 0.5);

        x1_i = x1;
        y1_i = (IMAGE_HEIGHT - 1) - y2;
        x2_i = x2;
        y2_i = (IMAGE_HEIGHT - 1) - y1;

        //imprimindo o bound box
        fprintf(result_file, "%05d.jpg.ppm;best_x=%d;best_y=%d;%d;%d;%d;%d\n",  g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y, x1_i, y1_i, x2_i, y2_i);
    }

    fclose(result_file);

    output.ival = 0;

    return (output);
}


NEURON_OUTPUT 
SetScaleFactor(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_scale_factor = pParamList->next->param.fval;
	reset_gaussian_filter_parameters();
		
	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
MoveCloseToTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = ((g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2);
	in_pattern.wyd = ((((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2);
	printf("Move Close To x = %d, y = %d\n", in_pattern.wxd, in_pattern.wyd);
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);


	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
MoveFarWayFromTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_scale_factor = 15.0 / ((double) (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol) / 2.0);
	reset_gaussian_filter_parameters();
	int delta_x =  (int) (1.0 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));

	in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2 - delta_x;
	if (in_pattern.wxd >= IMAGE_HEIGHT){
		in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2 - delta_x;
	}
	in_pattern.wyd = ((((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2);

	printf("Move Far Way x = %d, y = %d\n", in_pattern.wxd, in_pattern.wyd);
	move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
GetConfidence(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = g_confidence;
	return (output);

}