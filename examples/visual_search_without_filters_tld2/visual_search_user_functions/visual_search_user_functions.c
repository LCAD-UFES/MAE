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
	int best_x;
	int best_y;
	double best_confidence;
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
double g_scale_factor = 0.55;
double g_confidence;
double g_halph_band_width = 1.0;
int g_NumImagesTraineds = 0;


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
		if (fgets(file_line, 256, image_list))
		{
			if (g_nStatus == TRAINING_PHASE)
			{
				g_image_info[g_num_image_info].ImgNo = g_num_image_info + 1;
				sscanf(file_line, "%f,%f,%f,%f", &(g_image_info[g_num_image_info].leftCol),
				   &(g_image_info[g_num_image_info].topRow), &(g_image_info[g_num_image_info].rightCol), 
				   &(g_image_info[g_num_image_info].bottomRow));
				g_image_info[g_num_image_info].ClassID = -1;
				g_image_info[g_num_image_info].found = -1;
				g_image_info[g_num_image_info].used_for_trainning = -1;
				g_image_info[g_num_image_info].best_confidence = -1.0;
				g_image_info[g_num_image_info].best_x = -1;
				g_image_info[g_num_image_info].best_y = -1;
				g_image_info[g_num_image_info].best_scale_factor = -1.0;

			}
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

	sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
	// printf("Image file name: %s\n", strFileName);
	fflush(stdout);
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
draw_last_saccades(INPUT_DESC *input)
{
	int i, current;

	if (input->rectangle_list_size != 22)
	{
		input->rectangle_list = (RECTANGLE *) calloc(22, sizeof(RECTANGLE)); // @@@ Alberto: aloca 20 (+ 2 retangulos por conta das funcoes abaixo)
		input->rectangle_list_size = 22;
	}

	current = g_nCurrentImageInfo;

	i = 0;
	while ((i < 20) && (current-i > 0)){
		input->rectangle_list[2+i].x = g_image_info[current-i-1].best_x;
		input->rectangle_list[2+i].y = g_image_info[current-i-1].best_y;
		input->rectangle_list[2+i].w = 1.0;
		input->rectangle_list[2+i].h = 1.0;
		input->rectangle_list[2+i].r = 0.0;
		input->rectangle_list[2+i].g = 0.0;
		input->rectangle_list[2+i].b = 1.0;
		i++;
	}
}


void
draw_ground_truth_rectangle(INPUT_DESC *input)
{
	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(2, sizeof(RECTANGLE)); // @@@ Alberto: aloca dois retangulos por conta da funcao abaixo

	input->rectangle_list_size = 1; // @@@ Alberto: quande desenha este retangula apaga o retangulo abaixo

	input->rectangle_list[0].x = g_image_info[g_nCurrentImageInfo].leftCol;
	input->rectangle_list[0].y = (IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow;
	input->rectangle_list[0].w = g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol;
	input->rectangle_list[0].h = g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow;
	input->rectangle_list[0].r = 1.0;
	input->rectangle_list[0].g = 0.0;
	input->rectangle_list[0].b = 0.0;
}


void
draw_visual_search_attention_rectangle(INPUT_DESC *input)
{
	int largura, altura;

	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(2, sizeof(RECTANGLE)); // @@@ Alberto: aloca dois retangulos por conta da funcao acima

	input->rectangle_list_size = 2;

	altura = (int) (g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow + 0.5);   // @@@ Alberto: este tamanho deveria ser inferido da saida da rede
	largura = (int) (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol + 0.5);
	altura = abs(altura);
	largura = abs(largura);

	input->rectangle_list[1].x = g_image_info[g_nCurrentImageInfo].best_x - largura/2;
	input->rectangle_list[1].y = g_image_info[g_nCurrentImageInfo].best_y - altura/2;
	input->rectangle_list[1].w = largura;
	input->rectangle_list[1].h = altura;
	input->rectangle_list[1].r = 0.0;
	input->rectangle_list[1].g = 1.0;
	input->rectangle_list[1].b = 0.0;

	//imprime rastro de sacadas
	//draw_last_saccades(input);

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
set_scale_factor_old()
{
	double ground_truth_width = (double) (g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);

	if (ground_truth_width > 0.0)
	{
		g_scale_factor = compute_scale_factor(14.0 * ((double) NL_WIDTH / 65.0),
				ground_truth_width,	IMAGE_WIDTH_RESIZED, nl_v1_pattern.dimentions.x, LOG_FACTOR);
	}
	else
	{
		g_scale_factor = 1.0;
	}
}


int
truncate_value(double *value)
{
	double fractpart, intpart;

	fractpart = modf(*value, &intpart);
	fractpart = fractpart > 0.0 ? 0.5 : 0.0;
	*value = fractpart;

	return ((int) intpart);
}

void
set_scale_factor()
{
	double d_width, altura, largura;
	double halph_band_width_fp = 7.0 * ((double) NL_WIDTH / 65.0);
	int halph_band_width_int = truncate_value(&halph_band_width_fp);

	g_halph_band_width = halph_band_width_int + halph_band_width_fp;

	altura = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow);
	largura = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
	if (isnan(altura) || isnan(largura)) // FORA DE CENA
	{
		g_scale_factor = 1.0;
		return;
	}
	d_width = (largura + altura) / 2.0;

	g_scale_factor = compute_scale_factor(2.0 * g_halph_band_width,	d_width, IMAGE_WIDTH_RESIZED, nl_v1_pattern.dimentions.x, LOG_FACTOR);
}


void 
LoadImage(INPUT_DESC *input, char *strFileName)
{
	set_scale_factor();

	load_input_image(input, strFileName);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;

	draw_ground_truth_rectangle(input);
	update_input_neurons (input);
	update_input_image (input);

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
output_handler_draw_receptive_filters_center(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_neurons_of_interest_name = NULL;
	int w, h, w_out, h_out, x, y, u, v, counter;
	double log_factor;
	NEURON_LAYER *nl_neurons_of_interest = NULL;
		
	// Gets the output handler parameters
	nl_neurons_of_interest_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Get neuron layer of interest
	nl_neurons_of_interest = get_neuron_layer_by_name(nl_neurons_of_interest_name);
	w = nl_neurons_of_interest->dimentions.x;
	h = nl_neurons_of_interest->dimentions.y;

	w_out = output->neuron_layer->dimentions.x;
	h_out = output->neuron_layer->dimentions.y;
	if (output->rectangle_list == NULL)
	{
		output->rectangle_list = (RECTANGLE *) calloc(w * h, sizeof(RECTANGLE));
		output->rectangle_list_size = w * h;
	}

	counter = 0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			map_v1_to_image(&x, &y, w_out, h_out, u, v, w, h, (double) w_out / 2.0, (double) h_out / 2.0, (double) h / (double) (h - 1), log_factor);
			output->rectangle_list[counter].x = x;
			output->rectangle_list[counter].y = y;
			output->rectangle_list[counter].w = 1.0;
			output->rectangle_list[counter].h = 1.0;
			output->rectangle_list[counter].r = 0.0;
			output->rectangle_list[counter].g = 1.0;
			output->rectangle_list[counter].b = 0.0;
			counter++;
		}
	}
}


double
compute_confidence_old(NEURON *n, int g_halph_band_width, int w, int h)
{
	int u, v, initial_u, final_u;
	double count, confidence;

	initial_u = (int) (((double) (w-1) / 2.0) - g_halph_band_width + 0.5);
	final_u =   (int) (((double) (w-1) / 2.0) + g_halph_band_width + 0.5);
	confidence = count = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = initial_u; u <= final_u; u++)
		{
			if (n[v * w + u].output.fval > 0.2) // @@@ Alberto: Isso nao deveria ser constante...
				confidence += 1.0;
			count += 1.0;
		}
	}

	return (confidence / count);
}


double
compute_confidence(NEURON *n, int g_halph_band_width, int w, int h)
{
	double count, confidence;
	int u, v;
//	int xi, yi;
//	int ground_truth_w;
//	int ground_truth_h;
	float altura, largura;

	if (g_nCurrentImageInfo < 0)
		return (0.0);
		
	altura = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow);
	largura = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
	if (isnan(altura) || isnan(largura)) // FORA DE CENA
		return (0.0);

//	ground_truth_h = altura * g_scale_factor;
//	ground_truth_w = largura * g_scale_factor;

	confidence = count = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
/*			map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u, v, w, h, 0, 0, (double) h / (double) (h - 1), LOG_FACTOR);
			if (((double) xi > (-(double) ground_truth_w / 2.0)) &&
			    ((double) xi < ((double) ground_truth_w / 2.0)) &&
			    ((double) yi > (-(double) ground_truth_h / 2.0)) &&
			    ((double) yi < ((double) ground_truth_h / 2.0)))
			{
				if (n[v * w + u].output.fval > 0.01) // @@@ Alberto: Isso nao deveria ser constante...
					confidence += 1.0;
				count += 1.0;
			}
*/			
			if ( nl_v1_activation_map_confidence.neuron_vector[v * w + u].output.fval > 0 ){
//				confidence += fabs(n[v * w + u].output.fval - nl_v1_activation_map_confidence.neuron_vector[v * w + u].output.fval);
				if (n[v * w + u].output.fval > 0.0)
				{
					if ( n[v * w + u].output.fval == nl_v1_activation_map_confidence.neuron_vector[v * w + u].output.fval ) 
					{
						confidence += 2.0;
						count += 1.0;
					}
					else
						confidence += 1.0;
				}
				count += 1.0;
			}
		}
	}

//	confidence = 1/(1+confidence)*200000;

	return (confidence / count);
}


double
compute_weigheted_u_v(double *weigheted_u, double *weigheted_v, NEURON *n, int initial_u, int final_u, int w, int h)
{
	int i, u, v;
	double current_value, u_weight, v_weight, total_weight, threshold;

	threshold = 0.0;
	for (i = 0; i < w * h; i++)
		threshold += n[i].output.fval;
	threshold /= (double) (w * h) / 8.0;
	
	u_weight = 0.0;
	v_weight = 0.0;
	total_weight = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = initial_u; u < final_u; u++)
		{
			current_value = n[v * w + u].output.fval;
			if (current_value > threshold)
			{
				u_weight += (double) u * current_value;
				v_weight += (double) v * current_value;
				total_weight += current_value;
			}
		}
	}
	if (total_weight > 0.001)
	{
		*weigheted_u = u_weight / total_weight;
		*weigheted_v = v_weight / total_weight;
		
		return (total_weight);
	}
	else
	{
		*weigheted_u = 0.0;
		*weigheted_v = 0.0;

		return (0.0);
	}
}


void 
output_handler_weighted_average_value_position_old(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int w, h, xi, yi; //, step;
	double weigheted_u_left, weigheted_v_left, weigheted_u_right, weigheted_v_right;
	double weigheted_u, weigheted_v, left_weight, right_weight, log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name(nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	left_weight =  compute_weigheted_u_v(&weigheted_u_left, &weigheted_v_left, output->neuron_layer->neuron_vector, 0, w/2, w, h);
	right_weight = compute_weigheted_u_v(&weigheted_u_right, &weigheted_v_right, output->neuron_layer->neuron_vector, w/2, w, w, h);

	if (left_weight > right_weight)
	{
		weigheted_u = weigheted_u_left;
		weigheted_v = weigheted_v_left;
	}
	else
	{
		weigheted_u = weigheted_u_right;
		weigheted_v = weigheted_v_right;
	}
	g_confidence = compute_confidence(nl_v1_activation_map.neuron_vector, g_halph_band_width, w, h);
		
	// Map the max value coordinates to image
	map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, weigheted_u, weigheted_v, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
}


double
compute_weigheted_xi_yi(double *weigheted_xi, double *weigheted_yi, NEURON *n, int w, int h, double log_factor)
{
	int i, u, v, xi, yi;
	double current_value, xi_weight, yi_weight, total_weight, threshold, num_larger_than_zero, max_value;

	num_larger_than_zero = max_value = threshold = 0.0;
	for (i = 0; i < w * h; i++)
	{
		current_value = n[i].output.fval;
		if (current_value > 0.0)
		{
			if (current_value > max_value)
				max_value = current_value;
			threshold += current_value;
			num_larger_than_zero += 1.0;
		}
	}
	threshold /= num_larger_than_zero;
	threshold = threshold + (max_value - threshold) / 2.0;
	
	xi_weight = 0.0;
	yi_weight = 0.0;
	total_weight = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = n[v * w + u].output.fval;
			if (current_value > threshold)
			{
				// Map the max value coordinates to image
				map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u, v, w, h, 0, 0, 0, log_factor);
				xi_weight += (double) xi * current_value;
				yi_weight += (double) yi * current_value;
				total_weight += current_value;
			}
		}
	}
	if (total_weight > 0.001)
	{
		*weigheted_xi = xi_weight / total_weight;
		*weigheted_yi = yi_weight / total_weight;
		
		return (total_weight);
	}
	else
	{
		*weigheted_xi = 0.0;
		*weigheted_yi = 0.0;

		return (0.0);
	}
}


void 
output_handler_weighted_average_value_position(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int w, h;
	double xi, yi;
	double log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name(nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	compute_weigheted_xi_yi(&xi, &yi, output->neuron_layer->neuron_vector, w, h, log_factor);
	g_confidence = compute_confidence(nl_v1_activation_map.neuron_vector, g_halph_band_width, w, h);
	
	// Saves the max value position
	nl_target_coordinates->neuron_vector[0].output.fval = xi;
	nl_target_coordinates->neuron_vector[1].output.fval = yi;
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
set_input_layer_translation(INPUT_DESC *input, int x, int y)
{
	input->wxd = x;
	input->wyd = y;
	
	translation_filter_deltaX = (float) input->wxd;
	translation_filter_deltaY = (float) input->wyd;
	move_input_window(input->name, input->wxd, input->wyd);

}


void
saccade(INPUT_DESC *input)
{
	float x, y;
	int count = 0;
	double d_delta_x, d_delta_y;
	int delta_x, delta_y;
	float altura, largura;

	altura = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow);
	largura = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
	if (isnan(altura) || isnan(largura)) // FORA DE CENA
		return;

	x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;
	do
	{
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

		set_input_layer_translation(input, input->wxd, input->wyd);

		// salvando x e y
		if (g_confidence >= 0.5)
		{
			g_image_info[g_nCurrentImageInfo].best_x = in_pattern.wxd;
			g_image_info[g_nCurrentImageInfo].best_y = in_pattern.wyd;
		}
		else
		{
			g_image_info[g_nCurrentImageInfo].best_x = -1;
			g_image_info[g_nCurrentImageInfo].best_y = -1;
		}
		draw_visual_search_attention_rectangle(input);


		count++;
		#ifdef	CUDA_COMPILED
			x = nl_target_coordinates.host_neuron_vector[0].output.fval;
			y = nl_target_coordinates.host_neuron_vector[1].output.fval;
		#else
			x = nl_target_coordinates.neuron_vector[0].output.fval;
			y = nl_target_coordinates.neuron_vector[1].output.fval;
		#endif
	} while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 8));

	return;
}


void 
input_controler(INPUT_DESC *input, int status)
{
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		// Translate the input image & Move the input center cursor
		translation_filter_deltaX = (float) input->wxd;
		translation_filter_deltaY = (float) input->wyd;
		move_input_window(input->name, input->wxd, input->wyd);
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

//funcao com varias classes

float 
triangular_dist(float x, float mean, float dev)
{
	float inverse_distance, abs_mean_x;
	abs_mean_x = fabs(mean - x);
	
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
//	else if (fabs(mean - x) < (dev * 1.0))
//		inverse_distance = 0.1;
	else if (abs_mean_x < (dev * 1.15))
		inverse_distance = -1.0;
	else
		inverse_distance = 0.0;

	return (inverse_distance);
}

/*
//funcao com varias classes

float 
triangular_dist(float x, float mean, float dev)
{
	float inverse_distance, abs_mean_x;
	abs_mean_x = fabs(mean - x);
	
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
//	else if (fabs(mean - x) < (dev * 1.0))
//		inverse_distance = 0.1;
	else if (abs_mean_x < (dev * 1.1))
		inverse_distance = -1.0;
	else
		inverse_distance = 0.0;

	return (inverse_distance);
}


//usando apenas 3 classes: 1, 0, -1
float 
triangular_dist(float x, float mean, float dev)
{
	float inverse_distance, abs_mean_x;
	abs_mean_x = fabs(mean - x);
	
	if (fabs(mean - x) < (dev * 0.5))
		inverse_distance = 1.0;
	else if (fabs(mean - x) < (dev * 0.6))
		inverse_distance = 1.0;
	else if (fabs(mean - x) < (dev * 0.7))
		inverse_distance = 1.0;
	else if (fabs(mean - x) < (dev * 0.8))
		inverse_distance = 1.0;
	else if (fabs(mean - x) < (dev * 0.9))
		inverse_distance = 1.0;
//	else if (fabs(mean - x) < (dev * 1.0))
//		inverse_distance = 0.1;
	else if (abs_mean_x < (dev * 1.1))
		inverse_distance = -1.0;
	else
		inverse_distance = 0.0;

	return (inverse_distance);
}
*/

void
init_nl_landmark_eval_mask_for_log_polar_old(NEURON_LAYER *nl_landmark_eval_mask)
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
init_nl_landmark_eval_mask_for_log_polar(NEURON_LAYER *nl_landmark_eval_mask)
{
	int w, h, u, v, *half_band_width_vet;
	int xi, yi;
	int ground_truth_w;
	int ground_truth_h;
	NEURON *neuron_vector;
	int halph_band_width = 0;

	ground_truth_w = abs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol) * g_scale_factor;
	ground_truth_h = abs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow) * g_scale_factor;
	w = nl_landmark_eval_mask->dimentions.x;
	h = nl_landmark_eval_mask->dimentions.y;
	neuron_vector = nl_landmark_eval_mask->neuron_vector;

	half_band_width_vet = (int*)malloc( sizeof(int)*h );

	for (v = 0; v < h; v++)
	{
		half_band_width_vet[v] = 0;
		for (u = 0; u < w; u++)
		{
			map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u, v, w, h, 0, 0, (double) h / (double) (h - 1), LOG_FACTOR);
			if (((double) xi > (-(double) ground_truth_w / 2.0)) &&
			    ((double) xi < ((double) ground_truth_w / 2.0)) &&
			    ((double) yi > (-(double) ground_truth_h / 2.0)) &&
			    ((double) yi < ((double) ground_truth_h / 2.0)))
			{
				if ( fabs(u - (w-1) / 2.0) > half_band_width_vet[v] )
					half_band_width_vet[v] = u - (w-1) / 2.0;
			}
		}
	}
	printf("halph_band_width = %d\n", halph_band_width);

	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			neuron_vector[v * w + u].output.fval = triangular_dist((float) u, (float) (w-1) / 2.0, half_band_width_vet[v]);
		}
	}

	free(half_band_width_vet);
}


void
train_visual_search()
{
	double previous_confidence;
	NEURON *neuron_vector;
	long num_neurons;
	int i;

	g_nNetworkStatus = TRAINNING;
	init_nl_landmark_eval_mask_for_log_polar(&nl_v1_activation_map);
	init_nl_landmark_eval_mask_for_log_polar(&nl_v1_activation_map_confidence);

	num_neurons = get_num_neurons (nl_v1_activation_map.dimentions);
	neuron_vector = nl_v1_activation_map.neuron_vector;

//	train_neuron_layer("nl_v1_activation_map");
	for (i = 0; i < num_neurons; i++)
		if ( neuron_vector[i].output.fval >= 0 )
			train_neuron(&nl_v1_activation_map, i);

	previous_confidence = g_image_info[g_nCurrentImageInfo].best_confidence;
	g_image_info[g_nCurrentImageInfo].best_confidence = 1.0;
	g_image_info[g_nCurrentImageInfo].used_for_trainning = 1;
	g_image_info[g_nCurrentImageInfo].best_x = in_pattern.wxd;
	g_image_info[g_nCurrentImageInfo].best_y = in_pattern.wyd;
	g_image_info[g_nCurrentImageInfo].best_scale_factor = g_scale_factor;
	
	g_NumImagesTraineds++;
	printf("Imagem Treinada = %d, confidence = %lf\n", g_image_info[g_nCurrentImageInfo].ImgNo, previous_confidence);

	all_outputs_update();
	g_nNetworkStatus = RUNNING;
}	

NEURON_OUTPUT
flush_and_retraining(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	printf("retreinando!!!!\n");
	clear_neural_layers_memory ("nl_v1_activation_map");	//clear neural layer memory
	train_visual_search();
	output.ival = 0;
	return(output);
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
			train_visual_search();
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade(&in_pattern);
			update_input_image(&in_pattern);
			g_image_info[g_nCurrentImageInfo].best_confidence = g_confidence;
			break;	
		// Move input
		case 'J':
		case 'j':
			in_pattern.wxd--;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'K':
		case 'k':
			in_pattern.wxd++;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'I':
		case 'i':
			in_pattern.wyd++;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
			break;
		case 'M':
		case 'm':
			in_pattern.wyd--;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
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


NEURON_OUTPUT
GetImageByNumber(int image_order)
{
	NEURON_OUTPUT output;
	char strFileName[256];

	g_nImageOrder = image_order;
	get_traffic_sign_file_name(strFileName);
	LoadImage(&in_pattern, strFileName);
	output.ival = g_image_info[g_nCurrentImageInfo].ClassID;
	
	return (output);
}


int
GetNewImageNotTrained(int image_order)
{
	char strFileName[256];
	int found;

	g_nCurrentImageInfo++;
	g_nImageOrder++;

	if (g_image_info[g_nCurrentImageInfo].used_for_trainning != 1)
	{
		get_traffic_sign_file_name(strFileName);
		LoadImage(&in_pattern, strFileName);
		found = 1;
	}
	else 
		found = 0;
		
	return found;
}


NEURON_OUTPUT
GetImageNotTrained(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int image_order;
	int found;

	image_order = pParamList->next->param.ival;
	g_nCurrentImageInfo = image_order;
	if (g_image_info[g_nCurrentImageInfo].used_for_trainning != 1)
	{
		g_nImageOrder = image_order;
		get_traffic_sign_file_name(strFileName);
		LoadImage(&in_pattern, strFileName);
		output.ival = g_image_info[g_nCurrentImageInfo].ClassID;
	}
	else
	{
		found = 0;
		while (found != 1)
		{
			//printf("Imagem treinada\n");
			found = GetNewImageNotTrained(g_nCurrentImageInfo);
		}
	}

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
MoveToTargetCenter(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
	in_pattern.wyd = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2;
	
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
MoveToBestTargetCenter(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = g_image_info[g_nCurrentImageInfo].best_x;
	in_pattern.wyd = g_image_info[g_nCurrentImageInfo].best_y;
	
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


void
MoveToPointInTrafficSign(int position)
{
	in_pattern.wxd = (g_image_info[position].leftCol + g_image_info[position].rightCol) / 2;
	in_pattern.wyd = (((IMAGE_HEIGHT - 1) - g_image_info[position].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[position].bottomRow)) / 2;
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
}



NEURON_OUTPUT 
MoveToPoint(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = pParamList->next->param.ival;
	in_pattern.wyd = pParamList->next->next->param.ival;
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT 
TrainTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	train_visual_search();

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
TrainVariousTrafficSign(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	int num = pParamList->next->param.ival;

	for (i = 0; i <  g_num_image_info; i++)
	{
		if ((i%(g_num_image_info/num))== 0)
		{
			GetImageByNumber(i);
			MoveToTargetCenter(NULL);
			train_visual_search();
		}
	}
	MoveToPointInTrafficSign(0);
	g_nImageOrder = g_nCurrentImageInfo = -1;
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


double getDeltaXY()
{
	int i;
	int delta_x = 0;
	int delta_y = 0;

	i = g_nImageOrder;
	if (i > 0)
	{
		delta_x = g_image_info[i].best_x - g_image_info[i-1].best_x;
		delta_y = g_image_info[i].best_y - g_image_info[i-1].best_y;
	}	
	return (sqrt(delta_x * delta_x + delta_y * delta_y) / sqrt(IMAGE_WIDTH_RESIZED * IMAGE_WIDTH_RESIZED + IMAGE_WIDTH_RESIZED * IMAGE_WIDTH_RESIZED));
}


int
IsNeedRetrain()
{
   	//double delta_xy;
	int retrain = 0;
	int i;

	//delta_xy = getDeltaXY();

	i = g_nCurrentImageInfo;
	if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow)) // FORA DE CENA
		return 0;
		
	if ((g_confidence < 0.75)) //&& (delta_xy > 0.3))
	{
		retrain = 1;
		//printf("RETREINAR");
	}
	return retrain;
}


int
ReturnMaxConfidenceStored(int imageNumber, int numStored)
{
	int i;
	int index_max_confidence = imageNumber;
	int first_image_considered;

	first_image_considered = imageNumber - numStored;
	if (first_image_considered < 1)
		first_image_considered = 1;
		
	for (i = first_image_considered; i < imageNumber; i++)
	{
		if ((g_image_info[i].best_confidence > g_image_info[index_max_confidence].best_confidence) && (g_image_info[i].used_for_trainning != 1))
			index_max_confidence = i;	
	}
	return index_max_confidence;
}


NEURON_OUTPUT 
EvaluateDetection(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int retrain = IsNeedRetrain();
	output.ival = retrain;
	return (output);
}


NEURON_OUTPUT 
ImageToRetrain(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int numRetrain = ReturnMaxConfidenceStored(g_nImageOrder, 3);
	output.ival = numRetrain;
	return (output);
}


NEURON_OUTPUT 
SetImageOrder(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nImageOrder = pParamList->next->param.ival;
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
CheckTrafficSignDetection(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int largura;
	int altura;
	int i;
	double jaccard;
   	static double actual_true_positive = 0.0;
   	static double true_positive = 0.0;
   	static double false_positive = 0.0;
   	static double false_negative = 0.0;
   	static double valid_frames = 0.0;
   	static double invalid_frames = 0.0;
   	double precision;
   	double recall;
   	double f;
	int x1, y1, x2, y2, x1_j, y1_j, x2_j, y2_j;
	
	i = g_nImageOrder;

	if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow))
	{
		invalid_frames += 1.0;
   		if ((g_image_info[i].best_x != -1) && (g_image_info[i].best_y != -1))
   			false_positive = false_positive + 1.0;

		precision = (true_positive) / (valid_frames + false_positive);
		recall = (true_positive) / (valid_frames);
		f = (2.0 * precision * recall) / (precision + recall);

		printf("image = %05d; scale_factor = % .2lf; confidence = % .2lf; x = %03d; y = %03d; jaccard = % .2lf; precision = % .3lf; recall = % .3lf; f = % .3lf, %s\n",
		g_image_info[i].ImgNo, g_scale_factor, g_confidence, g_image_info[i].best_x, g_image_info[i].best_y, 0.0, precision, recall, f, "FORA DE CENA!");

		output.ival = 2;
	}
	else
	{
		actual_true_positive += 1.0;
		valid_frames += 1.0;
		
		altura =  g_image_info[i].topRow - g_image_info[i].bottomRow;	// @@@ Alberto: deveria ser computado a partir da rede
		largura = g_image_info[i].rightCol - g_image_info[i].leftCol;	// @@@ Alberto: deveria ser computado a partir da rede
		altura = abs(altura);
		largura = abs(largura);

		x1 = (int) ((double) g_image_info[i].best_x - (double) largura / 2.0 + 0.5);
		y1 = (int) ((double) g_image_info[i].best_y - (double) altura / 2.0 + 0.5);
		x2 = x1 + largura;
		y2 = y1 + altura;
		x1_j = x1;
		y1_j = (IMAGE_HEIGHT - 1) - y2;
		x2_j = x2;
		y2_j = (IMAGE_HEIGHT - 1) - y1;

		jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

 		if (jaccard >= 0.5)
    			true_positive = true_positive + 1.0;
		else
			false_negative = false_negative + 1.0;

		precision = (true_positive) / (valid_frames + false_positive);
		recall = (true_positive) / (valid_frames);
		f = (2.0 * precision * recall) / (precision + recall);

		printf("image = %05d; scale_factor = % .2lf; confidence = % .2lf; x = %03d; y = %03d; jaccard = % .2lf; precision = % .3lf; recall = % .3lf; f = % .3lf, %s\n",
				g_image_info[i].ImgNo, g_scale_factor, g_confidence,  g_image_info[i].best_x, g_image_info[i].best_y, jaccard, precision, recall, f, (jaccard >= 0.5)? "HIT": "MISS ***");



		fflush(stdout);

		output.ival = 0;
	}

	return (output);
}


NEURON_OUTPUT
SaveTLDReasultsFile(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	int x1, y1, x2, y2, x1_j, y1_j, x2_j, y2_j;
	float jaccard;
   	static double actual_true_positive = 0.0;
	FILE *result_file;
   	int largura;
   	int altura;
   	static double true_positive = 0.0;
   	static double false_positive = 0.0;
   	static double false_negative = 0.0;
   	static double valid_frames = 0.0;
   	static double invalid_frames = 0.0;

   	float count;
   	float precision;
   	float recall;
   	float f;

   	count = 0.0;

	result_file = fopen("resultadoTLD.txt", "w");
	for (i = 0; i < g_num_image_info; i++)
	{
		count = count + 1.0;

    		if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow))
    		{
			invalid_frames += 1.0;
    			if ((g_image_info[i].best_x != -1) && (g_image_info[i].best_y != -1))
    			{
    				false_positive = false_positive + 1.0;
    				fprintf(result_file, "%05d.jpg.ppm;best_x=%d;best_y=%d\n",
    				g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y);
    			}

    		}
    		else
    		{			

    			if (g_image_info[i].used_for_trainning != 1){
				valid_frames += 1.0;

				actual_true_positive += 1.0;
    				altura = ((int) ((double) (g_image_info[i].topRow - g_image_info[i].bottomRow) + 0.5));
    				largura = ((int) ((double) (g_image_info[i].rightCol - g_image_info[i].leftCol) + 0.5));
    				altura = abs(altura);
    				largura = abs(largura);

				x1 = (int) ((double) g_image_info[i].best_x - (double) largura / 2.0 + 0.5);
				y1 = (int) ((double) g_image_info[i].best_y - (double) altura / 2.0 + 0.5);
    				x2 = x1 + largura;
    				y2 = y1 + altura;
				
				x1_j = x1;
				y1_j = (IMAGE_HEIGHT - 1) - y2;
				x2_j = x2;
				y2_j = (IMAGE_HEIGHT - 1) - y1;

				jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

    				if (jaccard >= 0.5)
    				{
    					fprintf(result_file, "HIT - Jaccard = %f ", jaccard);
    					true_positive = true_positive + 1.0;
    				}

    				else 
				{
					fprintf(result_file, "MISS - Jaccard = %f ", jaccard);
					false_negative = false_negative + 1.0;

				}
    				//imprimindo o bound box
    				fprintf(result_file, "%05d.jpg.ppm;best_x=%d;best_y=%d\n",
    					g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y);

    			}
    		}

    	}

	precision = (true_positive) / (valid_frames + false_positive);
	recall = (true_positive) / (valid_frames);
	f = (2.0 * precision * recall) / (precision + recall);

	fprintf(result_file, "true_positive;false_positive;false_negative;PRECISION;RECALL;F\n");
	fprintf(result_file, "% .3lf; % .3lf; % .3lf; % .3lf; % .3lf; % .3lf\n", true_positive, false_positive, false_negative, precision, recall, f);

	printf("true_positive;false_positive;false_negative;PRECISION;RECALL;F\n");
	printf("% .3lf; % .3lf; % .3lf;% .3lf; % .3lf;% .3lf\n", true_positive, false_positive, false_negative, precision, recall, f);

	printf("total_frames, valid_frames; invalid_frames\n");
	printf("%f; %f; %f\n", valid_frames + invalid_frames, valid_frames, invalid_frames);

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
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);


	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
GetConfidence(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = g_confidence;
	printf("CONFIANCA = %f\n", g_confidence);
	return (output);

}


NEURON_OUTPUT
GetX(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;
	output.ival = g_image_info[i].best_x;
	return (output);

}

NEURON_OUTPUT
GetY(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;
	output.ival = g_image_info[i].best_y;
	return (output);
}


NEURON_OUTPUT
ForwardVisualSearchNetwork(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	forward_objects("in_pattern_translated_filter");
	forward_objects("in_pattern_filtered_translated_filter");
	forward_objects("nl_v1_pattern_filter");
	forward_objects("nl_v1_activation_map");
	forward_objects("nl_v1_activation_map_f_filter");
	all_outputs_update();

	output.ival = 0;
	return (output);
}

