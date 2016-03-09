
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "visual_search_user_functions.h"

#define ACEPT_IN_THE_IMAGE 55
#define CONFIDENCE_LEVEL 3
#define MIN_CONFIANCE_TO_ACCEPT_RETRAIN 55
#define MIN_CONFIANCE_TO_RETRAIN 55
#define NUM_IMAGES_BEFORE_TO_RETRAIN 4
#define NUM_PIXELS 7
#define	sigma 10.0
#define NUM_PIXELS_WEIGHT 4
#define COLOR_BAND_WIDTH_WEIGHT 2.0/6.0
#define BAND_ZOOM 100
#define JACCARD_FACTOR 0.25


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
	int out_of_scene;
	float tld_leftCol, tld_topRow, tld_rightCol, tld_bottomRow;
} IMAGE_INFO;


char *g_input_path = NULL;
IMAGE_INFO *g_image_info = NULL;
int g_num_image_info = 0;
int g_nImageOrder = -1;
int g_nCurrentImageInfo = -1;
int g_nNetworkStatus;
double g_scale_factor = 0.55;
double g_confidence;
double g_confidence_in_train = -1.0;
double g_halph_band_width = 1.0;
double g_color_band_width;
double g_scale_factor_init = -1.0;
int out_of_scene = 0;
float height_in_train = -1.0;
float width_in_train = -1.0;
float g_confidence_zoom_init = -1.0;
static int quadr_exploration = 0;
double old_g_scale_factor = -1.0;

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
				//TLD
				/*sscanf(file_line, "%f,%f,%f,%f", &(g_image_info[g_num_image_info].leftCol),
				   &(g_image_info[g_num_image_info].topRow), &(g_image_info[g_num_image_info].rightCol), 
				   &(g_image_info[g_num_image_info].bottomRow));
				*///Fim do bloco TLD
				
				//OnlineObjectTracking
				/*sscanf(file_line, "%f,%f,%f,%f", &(g_image_info[g_num_image_info].leftCol),
				   &(g_image_info[g_num_image_info].bottomRow), &(g_image_info[g_num_image_info].rightCol), 
				   &(g_image_info[g_num_image_info].topRow));
				(g_image_info[g_num_image_info].rightCol) += (g_image_info[g_num_image_info].leftCol);  
				(g_image_info[g_num_image_info].topRow) += (g_image_info[g_num_image_info].bottomRow);
				*/// Fim bloco OnlineObjectTracking

				//vot2014
				sscanf(file_line, "%f,%f,%f,%f,%f,%f,%f,%f", &(g_image_info[g_num_image_info].leftCol), &(g_image_info[g_num_image_info].topRow),
					&(g_image_info[g_num_image_info].leftCol),&(g_image_info[g_num_image_info].bottomRow),
					&(g_image_info[g_num_image_info].rightCol), &(g_image_info[g_num_image_info].bottomRow),
				   	&(g_image_info[g_num_image_info].rightCol),&(g_image_info[g_num_image_info].topRow));
				// Fim bloco vot2014

				g_image_info[g_num_image_info].ClassID = -1;
				g_image_info[g_num_image_info].found = -1;
				g_image_info[g_num_image_info].used_for_trainning = -1;
				g_image_info[g_num_image_info].best_confidence = -1.0;
				g_image_info[g_num_image_info].best_x = -1;
				g_image_info[g_num_image_info].best_y = -1;
				g_image_info[g_num_image_info].best_scale_factor = -1.0;
				g_image_info[g_num_image_info].out_of_scene = -1;
			}
		}
	}

	fclose(image_list);
}


void
get_traffic_sign_file_name(char *strFileName)
{
	if (g_nStatus == TRAINING_PHASE)
	{
		g_nCurrentImageInfo = g_nImageOrder;
	}
	//TLD
	//sprintf(strFileName, "%s%05d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);

	//OnlineObjectTracking
	//sprintf(strFileName, "%s%04d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);

	//vot2014
	sprintf(strFileName, "%s%08d.jpg.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);


	fflush(stdout);
}


void
get_traffic_sign_file_name_old(char *strFileName)
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
draw_ground_truth_rectangle(INPUT_DESC *input)
{
	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(3, sizeof(RECTANGLE)); // @@@ Alberto: aloca tres retangulos por conta das funcoes abaixo

	input->rectangle_list_size = 1; // @@@ Alberto: quande desenha este retangulo apaga o retangulo abaixo
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
	altura = (int) ((g_scale_factor_init / g_scale_factor) * height_in_train + 0.5);
	largura = (int) ((g_scale_factor_init / g_scale_factor) * width_in_train + 0.5);

	altura = abs(altura);
	largura = abs(largura);

	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(3, sizeof(RECTANGLE)); 

	input->rectangle_list_size = 2;
	input->rectangle_list[1].x = g_image_info[g_nCurrentImageInfo].best_x - largura/2;
	input->rectangle_list[1].y = g_image_info[g_nCurrentImageInfo].best_y - altura/2;
	input->rectangle_list[1].w = largura;
	input->rectangle_list[1].h = altura;
	input->rectangle_list[1].r = 0.0;
	input->rectangle_list[1].g = 1.0;
	input->rectangle_list[1].b = 0.0;

}


void
draw_visual_search_attention_rectangle_point(INPUT_DESC *input)
{
	int largura, altura;
	altura = (int) ((g_scale_factor_init / g_scale_factor) * height_in_train + 0.5);
	largura = (int) ((g_scale_factor_init / g_scale_factor) * width_in_train + 0.5);

	altura = abs(altura);
	largura = abs(largura);

	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(3, sizeof(RECTANGLE)); 

	input->rectangle_list_size = 2;
	input->rectangle_list[1].x = in_pattern.wxd - largura/2;
	input->rectangle_list[1].y = in_pattern.wyd - altura/2;
	input->rectangle_list[1].w = largura;
	input->rectangle_list[1].h = altura;
	input->rectangle_list[1].r = 0.0;
	input->rectangle_list[1].g = 1.0;
	input->rectangle_list[1].b = 0.0;

}


void
draw_ground_truth_tld_rectangle(INPUT_DESC *input)
{
	if (input->rectangle_list == NULL)
		input->rectangle_list = (RECTANGLE *) calloc(3, sizeof(RECTANGLE));

	input->rectangle_list_size = 3;
	input->rectangle_list[2].x = g_image_info[g_nCurrentImageInfo].tld_leftCol;
	input->rectangle_list[2].y = (IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].tld_topRow;
	input->rectangle_list[2].w = g_image_info[g_nCurrentImageInfo].tld_rightCol - g_image_info[g_nCurrentImageInfo].tld_leftCol;
	input->rectangle_list[2].h = g_image_info[g_nCurrentImageInfo].tld_topRow - g_image_info[g_nCurrentImageInfo].tld_bottomRow;
	input->rectangle_list[2].r = 0.0;
	input->rectangle_list[2].g = 0.0;
	input->rectangle_list[2].b = 1.0;

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


NEURON_OUTPUT 
SetScaleFactor(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_scale_factor = pParamList->next->param.fval;
	reset_gaussian_filter_parameters();
	output.ival = 0;
	
	return (output);
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

	g_color_band_width = table.dimentions.x * COLOR_BAND_WIDTH_WEIGHT;
	if ((int)g_color_band_width % 2 == 0) g_color_band_width = (double)((int)g_color_band_width) + 1.0;

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
       		filter_update(get_filter_by_name("in_pattern_filtered_translated_red_filter"));
       		filter_update(get_filter_by_name("in_pattern_filtered_translated_green_filter"));
       		filter_update(get_filter_by_name("in_pattern_filtered_translated_blue_filter"));
       		filter_update(get_filter_by_name("nl_v1_pattern_filter"));
       		filter_update(get_filter_by_name("table_v1_filter"));
		all_dendrites_update(); 
		all_neurons_update();

		forward_objects("nl_v1_activation_map_neuron_weight_filter");
		forward_objects("nl_v1_activation_map_neuron_weight_thresholded_filter");
		forward_objects("nl_activation_map_hough_filter");
		forward_objects("nl_activation_map_hough_gaussian_filter");
		forward_objects("nl_activation_map_hough_v1_filter");
		forward_objects("nl_activation_map_hough_zoom_filter");
		forward_objects("nl_activation_map_hough_zoom_gaussian_filter");

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



int
gt_color_from_x(int x)
{
	return (int)( ( (double)(x - (table.dimentions.x-g_color_band_width)/2)) * 254.0/g_color_band_width + 0.5);
}


int
gt_color_from_y(int y)
{
	return (int)( ( (double)(y - (table.dimentions.y-g_color_band_width)/2)) * 254.0/g_color_band_width + 0.5);
}


int
gt_x_displecement_from_fovea(int green)
{
	return (int)( (double) green * g_color_band_width/254.0 - g_color_band_width/2 + 0.5);
}


int
gt_y_displecement_from_fovea(int red)
{
	return (int)( (double) red * g_color_band_width/254.0 - g_color_band_width/2 + 0.5);
}


int
compute_lim_inf_u(int u_neuron)
{
	if (u_neuron >= NUM_PIXELS_WEIGHT) 
		return (u_neuron - NUM_PIXELS_WEIGHT);
	else 
		return 0;
}


int
compute_lim_inf_v(int v_neuron)
{
	if (v_neuron >= NUM_PIXELS_WEIGHT) 
		return (v_neuron - NUM_PIXELS_WEIGHT);
	else 
		return 0;
}


int
compute_lim_sup_u(int u_neuron, int w)
{
	if (u_neuron < w - NUM_PIXELS_WEIGHT - 1) 
		return (u_neuron + NUM_PIXELS_WEIGHT);
	else 
		return w - 1;
}


int
compute_lim_sup_v(int v_neuron, int h)
{
	if (v_neuron < h - NUM_PIXELS_WEIGHT - 1) 
		return (v_neuron + NUM_PIXELS_WEIGHT);
	else 
		return h - 1;
}


int
compute_lim_inf_u_borda(int u_neuron, int w)
{
	int u_neighbor_neuron;

	u_neighbor_neuron = (w - 1) - u_neuron; 
	return compute_lim_inf_u(u_neighbor_neuron);
}


int
compute_lim_sup_u_borda(int u_neuron, int w)
{
	int u_neighbor_neuron;

	u_neighbor_neuron = (w - 1) - u_neuron; 
	return compute_lim_sup_u(u_neighbor_neuron, w);
}


// deslocamento em x do centro para o neuronio principal
double
compute_desloc_x(int neuron_output, int w, int h, int u_neuron, int v_neuron, double log_factor)
{
	int xi, xi_neuron, yi;
	double dist_x_neuron;

//	xi = gt_x_displecement_from_fovea(GREEN(table_v1.neuron_vector[v_neuron * w + u_neuron].output.ival));
	map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u_neuron, v_neuron, w, h, 0, 0, 0, log_factor);
	xi_neuron = gt_x_displecement_from_fovea(GREEN(neuron_output));
	dist_x_neuron = (double)(xi - xi_neuron);
	
	return dist_x_neuron;
}


// deslocamento em y do centro para o neuronio principal
double
compute_desloc_y(int neuron_output, int w, int h, int u_neuron, int v_neuron, double log_factor)
{
	int yi, yi_neuron,xi;
	double dist_y_neuron;

//	yi = gt_y_displecement_from_fovea(RED(table_v1.neuron_vector[v_neuron * w + u_neuron].output.ival));
	map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u_neuron, v_neuron, w, h, 0, 0, 0, log_factor);
	yi_neuron = gt_y_displecement_from_fovea(RED(neuron_output));
	dist_y_neuron = (double)(yi - yi_neuron);

	return dist_y_neuron;
	
}


//diferenca de deslocamentos
double
compute_dif_desloc(double dist_x, double dist_x_neuron, double dist_y, double dist_y_neuron)
{
	double dif_x, dif_y, dist;

	dif_x = (dist_x - dist_x_neuron)*(dist_x - dist_x_neuron);
	dif_y = (dist_y - dist_y_neuron)*(dist_y - dist_y_neuron);
	dist = exp(- (1.0 / 2.0) * ((dif_x + dif_y) / (sigma * sigma)));

	return dist;
}


double
compute_weight (int lim_inf_u_neighbor, int lim_sup_u_neighbor, int lim_inf_v_neighbor, int lim_sup_v_neighbor, int u_neuron, int v_neuron, int w, int h, double dist_x_neuron, double dist_y_neuron, double log_factor)
{			

	int u_neighbor_neuron, v_neighbor_neuron;
	int neighbor_neuron_output;
	double partial_weight = 0.0;
	double dist_x, dist_y;

	for (v_neighbor_neuron = lim_inf_v_neighbor; v_neighbor_neuron <= lim_sup_v_neighbor; v_neighbor_neuron++)
	{   
		for (u_neighbor_neuron = lim_inf_u_neighbor; u_neighbor_neuron <= lim_sup_u_neighbor; u_neighbor_neuron++)
		{
			if ((u_neighbor_neuron != u_neuron) || (v_neighbor_neuron != v_neuron))
			{
				neighbor_neuron_output = nl_v1_activation_map.neuron_vector[v_neighbor_neuron * w + u_neighbor_neuron].output.ival;
				if (BLUE(neighbor_neuron_output) > 0)
				{
					dist_x = compute_desloc_x(neighbor_neuron_output, w, h, u_neighbor_neuron, v_neighbor_neuron, log_factor);
					dist_y = compute_desloc_y(neighbor_neuron_output, w, h, u_neighbor_neuron, v_neighbor_neuron, log_factor);
					partial_weight += compute_dif_desloc(dist_x, dist_x_neuron, dist_y, dist_y_neuron);
				}
			}
		}
	}

	return partial_weight;	
}


double
compute_weigheted_neighborhood(NEURON *n, int w, int h, int u_neuron, int v_neuron, double log_factor)
{
	double partial_weight, partial_weight_center;
	int u_neighbor_neuron, v_neighbor_neuron, v_neighbor_neuron_aux;
	int lim_inf_u_neighbor, lim_inf_v_neighbor, lim_sup_u_neighbor, lim_sup_v_neighbor;
	int x_neuron, y_neuron;
	double dist, dist_x, dist_y;
	int neighbor_neuron_output, neuron_output, b_neuron_color;
	double dist_x_neuron;
	double dist_y_neuron;
	int value;

	partial_weight = 0.0;
	partial_weight_center = 0.0;

	neuron_output = nl_v1_activation_map.neuron_vector[v_neuron * w + u_neuron].output.ival;
	x_neuron = IMAGE_WIDTH_RESIZED/2.0 + gt_x_displecement_from_fovea(GREEN(neuron_output));   
	y_neuron = IMAGE_HEIGHT_RESIZED/2.0 + gt_y_displecement_from_fovea(RED(neuron_output));
	value = table.neuron_vector[y_neuron * IMAGE_WIDTH_RESIZED + x_neuron].output.ival;

	if (neuron_output != 0 && value != 0){
		b_neuron_color = BLUE(value);

		dist_x_neuron = compute_desloc_x(neuron_output, w, h, u_neuron, v_neuron, log_factor);
		dist_y_neuron = compute_desloc_y(neuron_output, w, h, u_neuron, v_neuron, log_factor);

		if (b_neuron_color > 0)
		{   
			if ( (v_neuron > NUM_PIXELS_WEIGHT - 1) && (v_neuron < h - NUM_PIXELS_WEIGHT))
			{
				lim_inf_v_neighbor = compute_lim_inf_v(v_neuron);
				lim_inf_u_neighbor = compute_lim_inf_u(u_neuron);
				lim_sup_v_neighbor = compute_lim_sup_v(v_neuron, h);
				lim_sup_u_neighbor = compute_lim_sup_u(u_neuron, w);

				partial_weight += compute_weight(lim_inf_u_neighbor, lim_sup_u_neighbor, lim_inf_v_neighbor, lim_sup_v_neighbor, u_neuron, v_neuron, w, h, dist_x_neuron, dist_y_neuron, log_factor);
			}

			// Borda inferior
			else if ( (v_neuron <= NUM_PIXELS_WEIGHT) || (v_neuron >= h - NUM_PIXELS_WEIGHT))
			{
				lim_inf_v_neighbor = v_neuron - NUM_PIXELS_WEIGHT;
				lim_sup_v_neighbor = v_neuron + NUM_PIXELS_WEIGHT;

				for (v_neighbor_neuron_aux = lim_inf_v_neighbor; v_neighbor_neuron_aux <= lim_sup_v_neighbor; v_neighbor_neuron_aux++)
				{   
					if ( v_neighbor_neuron_aux < 0 )
					{
						v_neighbor_neuron = -v_neighbor_neuron_aux - 1;
						lim_inf_u_neighbor = compute_lim_inf_u_borda(u_neuron, w);
						lim_sup_u_neighbor = compute_lim_sup_u_borda(u_neuron, w);
					}
	
					else if ( v_neighbor_neuron_aux >= h )
					{
						v_neighbor_neuron = h - 1 - (v_neighbor_neuron_aux-h);
						lim_inf_u_neighbor = compute_lim_inf_u_borda(u_neuron, w);
						lim_sup_u_neighbor = compute_lim_sup_u_borda(u_neuron, w);
					}
					else
					{
						v_neighbor_neuron = v_neighbor_neuron_aux;
						lim_inf_u_neighbor = compute_lim_inf_u(u_neuron);
						lim_sup_u_neighbor = compute_lim_sup_u(u_neuron, w);
					}

					for (u_neighbor_neuron = lim_inf_u_neighbor; u_neighbor_neuron <= lim_sup_u_neighbor; u_neighbor_neuron++)
					{
						neighbor_neuron_output = nl_v1_activation_map.neuron_vector[v_neighbor_neuron * w + u_neighbor_neuron].output.ival;
						if (BLUE(neighbor_neuron_output) > 0)
						{
							dist_x = compute_desloc_x(neighbor_neuron_output, w, h, u_neighbor_neuron, v_neighbor_neuron, log_factor);
							dist_y = compute_desloc_y(neighbor_neuron_output, w, h, u_neighbor_neuron, v_neighbor_neuron, log_factor);
							partial_weight += compute_dif_desloc(dist_x, dist_x_neuron, dist_y, dist_y_neuron);

							if ((u_neighbor_neuron != u_neuron) || (v_neighbor_neuron != v_neuron))
							{
								partial_weight_center = dist;
							}
						}
					}
				}
				partial_weight -= partial_weight_center;
			}
		}
	}
//printf("peso = %lf\n", partial_weight / ((2*NUM_PIXELS_WEIGHT + 1)*(2*NUM_PIXELS_WEIGHT + 1)-1));
	return partial_weight / ((2*NUM_PIXELS_WEIGHT + 1)*(2*NUM_PIXELS_WEIGHT + 1)-1);
}


double
compute_weigheted_max_neighbor_linear(double *weigheted_xi, double *weigheted_yi, NEURON *n, int w, int h, double log_factor)
{
	int u, v;
	double xi_max = 0.0, yi_max = 0.0;
	double value, value_max = -1, count = 1.0;

	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			value = n[v * w + u].output.fval;
		//if (value != 0.0) printf("value = %f\n", value);
			if (value > value_max)
			{
				value_max = value;
				xi_max = u;
				yi_max = v;
				count = 1.0;
			}
			else if (value == value_max)
			{
				xi_max += u;
				yi_max += v;
				count += 1.0;
			}
		}
	}
	g_confidence = value_max;
	*weigheted_xi = xi_max/count - w/2;
	*weigheted_yi = yi_max/count - h/2;
//		printf("weigheted_xi= %lf g_confidence = %lf value_max = %lf\n", *weigheted_xi, g_confidence, value_max);

	return (g_confidence);
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
	
	compute_weigheted_max_neighbor_linear(&xi, &yi, output->neuron_layer->neuron_vector, w, h, log_factor);

	// Saves the max value position
	nl_target_coordinates->neuron_vector[0].output.fval = xi;
	nl_target_coordinates->neuron_vector[1].output.fval = yi;

}


void 
output_handler_resize(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
/*	static int first_time = 1;
	
	if (first_time <= 2)
	{
		glutSetWindow(output->win);
		glutReshapeWindow(1500, 5);
		glutPostRedisplay();
		first_time = first_time + 1;
	}
*/	
}


void
set_input_layer_translation(INPUT_DESC *input, int x, int y)
{
	input->wxd = x;
	input->wyd = y;
	
	translation_filter_deltaX = (float) input->wxd;
	translation_filter_deltaY = (float) input->wyd;
	move_input_window(input->name, input->wxd, input->wyd);
	ForwardVisualSearchNetwork(NULL);
}


void
saccade(INPUT_DESC *input)
{
	float x, y;
	int count = 0;
	double d_delta_x, d_delta_y;
	int delta_x, delta_y;

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
		g_image_info[g_nCurrentImageInfo].best_x = in_pattern.wxd;
		g_image_info[g_nCurrentImageInfo].best_y = in_pattern.wyd;
		draw_visual_search_attention_rectangle(input);
		//draw_ground_truth_tld_rectangle(input);

		count++;

		#ifdef	CUDA_COMPILED
		x = nl_target_coordinates.host_neuron_vector[0].output.fval;
		y = nl_target_coordinates.host_neuron_vector[1].output.fval;
		#else
		x = nl_target_coordinates.neuron_vector[0].output.fval;
		y = nl_target_coordinates.neuron_vector[1].output.fval;
		#endif
	} while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));
//para calibrar
//	} while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 1));
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
generate_color_linear_map(NEURON_LAYER *nl_landmark_eval_mask)
{
	int w, h;
	int x, y;
	double dist_2_center;
	NEURON *neuron_vector;
	int r, g, b;

	w = nl_landmark_eval_mask->dimentions.x;
	h = nl_landmark_eval_mask->dimentions.y;
	neuron_vector = nl_landmark_eval_mask->neuron_vector;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if ( sqrt( (x - w/2)*(x - w/2) + (y - h/2)*(y - h/2) ) < (g_color_band_width/2) )
			{
				g = gt_color_from_x(x);
				r = gt_color_from_y(y);
				dist_2_center = sqrt( (double)( (x-w/2)*(x-w/2) + (y-h/2)*(y-h/2) ) ); 
				b = 255 - (int)( dist_2_center * 255.0/(g_color_band_width/2.0) );
			}
			else
			{
				r = g = b = 0;
			}

			neuron_vector[y * w + x].output.ival = PIXEL (r, g, b);
		}
	}
}


void
mask_output_color(NEURON_LAYER *nl_color, NEURON_LAYER *nl_masked, double log_factor)
{
	int w, h;
	double ground_truth_w;
	double ground_truth_h;
	double leftCol_gt;
	double rightCol_gt;
	double bottomRow_gt;
	double topRow_gt;

	int x, y, u, v;

	if (g_scale_factor_init == -1.0)
	{
		ground_truth_w = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol) * g_scale_factor;
		ground_truth_h = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow) * g_scale_factor;
	}
	else
	{
		ground_truth_w = (int) ((g_scale_factor_init / g_image_info[g_nCurrentImageInfo].best_scale_factor) * width_in_train + 0.5);
		ground_truth_h = (int) ((g_scale_factor_init / g_image_info[g_nCurrentImageInfo].best_scale_factor) * height_in_train + 0.5);
	}

	w = nl_masked->dimentions.x;
	h = nl_masked->dimentions.y;

	leftCol_gt = ((double)(w/2.0)) - ((double)((ground_truth_w / 2.0) + 0.5));
	rightCol_gt = ((double)(w/2.0)) + ((double)((ground_truth_w / 2.0) - 0.5));
	bottomRow_gt = ((double)(h/2.0)) - ((double)((ground_truth_h / 2.0) + 0.5));
	topRow_gt = ((double)(h/2.0)) + ((double)((ground_truth_h / 2.0) - 0.5));

	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			map_v1_to_image(&x, &y, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u, v, w, h, (w/2), (h/2), 0, log_factor);
			
			if (((double) x >= leftCol_gt) &&
			    ((double) x <= rightCol_gt) &&
			    ((double) y >= bottomRow_gt) &&
			    ((double) y <= topRow_gt))
			{
				nl_masked->neuron_vector[v * w + u].output = nl_color->neuron_vector[v * w + u].output;
			}
			else
				nl_masked->neuron_vector[v * w + u].output.ival = 0.0;
		}

	}
}


void
train_visual_search()
{
	long num_neurons;
	int i;
	int h_max_pos, h_min_pos, w_max_pos, w_min_pos;

	if ((height_in_train == -1.0) && (width_in_train == -1))
	{
		height_in_train = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow);
		width_in_train = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);

		g_image_info[g_nCurrentImageInfo].best_confidence = 1.0;
		g_image_info[g_nCurrentImageInfo].best_x = in_pattern.wxd;
		g_image_info[g_nCurrentImageInfo].best_y = in_pattern.wyd;
		g_image_info[g_nCurrentImageInfo].best_scale_factor = g_scale_factor;
		g_image_info[g_nCurrentImageInfo].out_of_scene = 0;
		g_confidence_in_train = -1.0;

		generate_color_linear_map(&table);

	}
	w_max_pos = in_pattern.wxd + 1.0 * (width_in_train)/2.0;
	w_min_pos = in_pattern.wxd - 1.0 * (width_in_train)/2.0;
	h_max_pos = in_pattern.wyd + 1.0 * (height_in_train)/2.0;
	h_min_pos = in_pattern.wyd - 1.0 * (height_in_train)/2.0;

	if (w_max_pos >= IMAGE_WIDTH || w_min_pos < 0 || h_max_pos >= IMAGE_HEIGHT || h_min_pos < 0)
	{
		printf("Not training with this data (OUTSIDE THE IMAGE)\n");
		return;
	} 

	g_confidence_in_train  = -1.0;
	g_nNetworkStatus = TRAINNING;
	g_image_info[g_nCurrentImageInfo].used_for_trainning = 1;
	forward_objects("table_v1_filter");
	mask_output_color(&table_v1, &nl_v1_activation_map, LOG_FACTOR);
	
	num_neurons = get_num_neurons (nl_v1_activation_map.dimentions);

	for (i = 0; i < num_neurons; i++)
		train_neuron(&nl_v1_activation_map, i);

	if (g_scale_factor_init == -1.0)
	{
		g_scale_factor_init = g_scale_factor;
		printf("g_scale_factor_init = %f\n", g_scale_factor_init);
	}

	printf("Imagem Treinada = %d, g_scale_factor = %lf\n", g_image_info[g_nCurrentImageInfo].ImgNo, g_scale_factor);

	g_nNetworkStatus = RUNNING;

}	

void
train_visual_search_tunning()
{
	long num_neurons;
	int i;
	int h_max_pos, h_min_pos, w_max_pos, w_min_pos;

	height_in_train = fabs(g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow);
	width_in_train = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);

	g_image_info[g_nCurrentImageInfo].best_confidence = 1.0;
	g_image_info[g_nCurrentImageInfo].best_x = in_pattern.wxd;
	g_image_info[g_nCurrentImageInfo].best_y = in_pattern.wyd;
	g_image_info[g_nCurrentImageInfo].best_scale_factor = g_scale_factor;
	g_image_info[g_nCurrentImageInfo].out_of_scene = 0;
	g_confidence_in_train = -1.0;

	generate_color_linear_map(&table);

	w_max_pos = in_pattern.wxd + 1.0 * (width_in_train)/2.0;
	w_min_pos = in_pattern.wxd - 1.0 * (width_in_train)/2.0;
	h_max_pos = in_pattern.wyd + 1.0 * (height_in_train)/2.0;
	h_min_pos = in_pattern.wyd - 1.0 * (height_in_train)/2.0;

	if (w_max_pos >= IMAGE_WIDTH || w_min_pos < 0 || h_max_pos >= IMAGE_HEIGHT || h_min_pos < 0)
	{
		printf("Not training with this data (OUTSIDE THE IMAGE)\n");
		return;
	} 

	g_confidence_in_train  = -1.0;
	g_nNetworkStatus = TRAINNING;
	g_image_info[g_nCurrentImageInfo].used_for_trainning = 1;
	forward_objects("table_v1_filter");
	mask_output_color(&table_v1, &nl_v1_activation_map, LOG_FACTOR);
	
	num_neurons = get_num_neurons (nl_v1_activation_map.dimentions);

	for (i = 0; i < num_neurons; i++)
		train_neuron(&nl_v1_activation_map, i);

	g_scale_factor_init = g_scale_factor;
	//printf("g_scale_factor_init = %f\n", g_scale_factor_init);

	//printf("Imagem Treinada = %d, g_scale_factor = %lf\n", g_image_info[g_nCurrentImageInfo].ImgNo, g_scale_factor);

	g_nNetworkStatus = RUNNING;



}	

/*
int
GetXOctante(int i)
{
	return (int)(IMAGE_WIDTH / 8 + ((i % 4) * IMAGE_WIDTH / 4));

}


int
GetYOctante(int i)
{

	if (i <= 3)
		return (int)(IMAGE_HEIGHT / 8);
	else if (i <=7)
		return (int)(IMAGE_HEIGHT / 8 + IMAGE_HEIGHT / 4);
	else if (i <=11)
		return (int)(IMAGE_HEIGHT / 8 + 2 * IMAGE_HEIGHT / 4);
	else
		return (int)(IMAGE_HEIGHT / 8 + 3 * IMAGE_HEIGHT / 4);
}


float
exploration(INPUT_DESC *input, int x, int y, int *x_exp, int *y_exp)
{
	int i, x_o, y_o;
	int best_x , best_y;
	float best_confidence = 0.0;

	best_x = best_y = 0;

	for (i = 0; i < 16; i++)
	{
		x_o = GetXOctante(i);
		y_o = GetYOctante(i);
		in_pattern.wxd = x_o;
		in_pattern.wyd = y_o;
		set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
		saccade(&in_pattern);
		update_input_image(&in_pattern);
		x_o = in_pattern.wxd;
		y_o = in_pattern.wyd;

		if (g_confidence > best_confidence)
		{
			best_x = x_o;
			best_y = y_o;
			best_confidence = g_confidence;
		}
	}

	*x_exp = best_x;
	*y_exp = best_y;

	return best_confidence;
}
*/


int
GetXOctante(int i)
{
	return (int)(IMAGE_WIDTH / 8 + ((i % 4) * IMAGE_WIDTH / 4));

}


int
GetYOctante(int i)
{
	if (i <= 3)
		return (int)(IMAGE_HEIGHT / 4);
	else
		return (int)(IMAGE_HEIGHT / 4 + IMAGE_HEIGHT / 2);

}


float
exploration(INPUT_DESC *input, int x, int y, int *x_exp, int *y_exp)
{
	int i, x_o, y_o;
	int best_x, best_y;

	float best_confidence = 0.0;

	for (i = 0; i < 8; i++)
	{
		x_o = GetXOctante(i);
		y_o = GetYOctante(i);
		in_pattern.wxd = x_o;
		in_pattern.wyd = y_o;
		set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
		saccade(&in_pattern);
		//update_input_image(&in_pattern);
		x_o = in_pattern.wxd;
		y_o = in_pattern.wyd;

		if (g_confidence > best_confidence)
		{
			best_x = x_o;
			best_y = y_o;
			best_confidence = g_confidence;
		}
	}

	*x_exp = best_x;
	*y_exp = best_y;

	return best_confidence;
}


float
exploration2(INPUT_DESC *input, int x, int y, int *x_exp, int *y_exp)
{
	int x_o, y_o;

	x_o = GetXOctante(quadr_exploration);
	y_o = GetYOctante(quadr_exploration);
	in_pattern.wxd = x_o;
	in_pattern.wyd = y_o;
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
	saccade(&in_pattern);
	//update_input_image(&in_pattern);
	*x_exp = in_pattern.wxd;
	*y_exp = in_pattern.wyd;

	if (quadr_exploration < 7) quadr_exploration++;
	else quadr_exploration = 0;
	
	return g_confidence;
}

void
saccade_visual_search()
{
	double x, y;
	float confidence;
	int x_exp, y_exp, best_x, best_y;
	float scale;
	float best_scale;
	float best_confidence;
	int i;
	float z;
	float maior = 0.0;

	x = in_pattern.wxd;
	y = in_pattern.wyd;
	if (out_of_scene == 0)
	{
		saccade(&in_pattern);
		//update_input_image(&in_pattern);
		g_image_info[g_nCurrentImageInfo].best_confidence = g_confidence;
		g_image_info[g_nCurrentImageInfo].best_scale_factor = g_scale_factor;
		g_image_info[g_nCurrentImageInfo].out_of_scene = 0;
		if (g_confidence < CONFIDENCE_LEVEL){
			in_pattern.wxd = x;
			in_pattern.wyd = y;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
			g_image_info[g_nCurrentImageInfo].best_x = -1;
			g_image_info[g_nCurrentImageInfo].best_y = -1;
			g_image_info[g_nCurrentImageInfo].out_of_scene = 1;
			out_of_scene = 1;
		}
		old_g_scale_factor = g_scale_factor;
	}
	else
	{
		// voltando com uma faixa de escala
		scale = MIN_SCALE;
		best_confidence = best_scale = 0.0;
		best_x = best_y = 0;
/*
		while (scale < MAX_SCALE)
		{
			g_scale_factor = scale;
			reset_gaussian_filter_parameters();
			ForwardVisualSearchNetwork(NULL);

			confidence = exploration(&in_pattern, x, y, &x_exp, &y_exp);
			printf("confidence = %f; x = %d; y = %d, g_scale_factor = %f\n", confidence, x_exp, y_exp, g_scale_factor);

			if (confidence > best_confidence)
			{
				best_confidence = confidence;
				best_x = x_exp;
				best_y = y_exp;
				best_scale = g_scale_factor;
			}

			scale = scale * 1.05;
		}
*/

		//voltando com a escala anterior
		/*g_scale_factor = old_g_scale_factor;
		reset_gaussian_filter_parameters();
		ForwardVisualSearchNetwork(NULL);
		best_confidence = exploration2(&in_pattern, x, y, &x_exp, &y_exp);
		best_x = x_exp;
		best_y = y_exp;
		best_scale = g_scale_factor;

		printf("confidence = %f; best_scale = %f\n", best_confidence, best_scale);

		*/

		//voltando com o zoom certo
		best_confidence = exploration2(&in_pattern, x, y, &x_exp, &y_exp);
		best_x = x_exp;
		best_y = y_exp;
		best_scale = g_scale_factor;

		//printf("confidence = %f; best_scale = %f\n", best_confidence, best_scale);

		if (best_confidence > ACEPT_IN_THE_IMAGE)
		{
			g_image_info[g_nCurrentImageInfo].best_confidence = best_confidence;
			g_image_info[g_nCurrentImageInfo].best_scale_factor = best_scale;
			g_image_info[g_nCurrentImageInfo].best_x = best_x;
			g_image_info[g_nCurrentImageInfo].best_y = best_y;
			g_image_info[g_nCurrentImageInfo].out_of_scene = 0;
			out_of_scene = 0;
			in_pattern.wxd = best_x;
			in_pattern.wyd = best_y;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
			old_g_scale_factor = g_scale_factor;
		}
		else
		{
			in_pattern.wxd = x;
			in_pattern.wyd = y;
			g_image_info[g_nCurrentImageInfo].best_confidence = best_confidence;
			g_image_info[g_nCurrentImageInfo].best_scale_factor = best_scale;
			g_image_info[g_nCurrentImageInfo].best_x = -1;
			g_image_info[g_nCurrentImageInfo].best_y = -1;
			g_image_info[g_nCurrentImageInfo].out_of_scene = 1;
			out_of_scene = 1;
			set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
		}
	}

	if (g_confidence_zoom_init == -1.0)
	{
		for (i = 0; i < TAM_NL_ZOOM; i++)
		{
			z = nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
			if ( z != 0)
			{
				if (z > maior)
				{
					maior = z;
				}	
			}
		}
		g_confidence_zoom_init = maior;
	}

}


void
printing_zoom()
{
	double x;
	int i;
	double count;
	float z;
	float maior = 0.0;
	int i_maior;
	int zoom = 0;

	x = 0.0;
	count = 0.0;

	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		if (nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval != 0)
		{
			x += (double)i * nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
			count += nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
		}
	}

	printf("g_scale_factor inicial= %f\n", g_scale_factor);

	if (count != 0.0)
	{
		printf("x/count = %f\n", x/count);
		if (x/count == 0.0)
			zoom = 0;
		else
		{
			if ((x/count >= (float)(TAM_NL_ZOOM/2 - BAND_ZOOM)) && (x/count <= (float)(TAM_NL_ZOOM/2 + BAND_ZOOM)))
				zoom = 0;
			else
			{
				if ((int)(x/count) > (TAM_NL_ZOOM/2))
					zoom = -1;
				else
					zoom = 1;
			}
		}
	}
	else
		zoom = 0;

	printf("Zoom = %d\n", zoom);
			
	maior = 0;
	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		z = nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
		if ( z != 0)
		{
			if (z > maior)
			{
				maior = z;
				i_maior = i;
			}	
		}
	}

	printf("i_maior gaussian = %d, maior gaussian = %f\n", i_maior, maior);

	maior = 0;
	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		z = nl_activation_map_hough_zoom.neuron_vector[i].output.fval;
		if ( z != 0)
		{
			if (z > maior)
			{
				maior = z;
				i_maior = i;
			}	
		}
	}

	printf("i_maior = %d, maior= %f\n", i_maior, maior);

	if (zoom == 0)
		g_scale_factor = g_scale_factor;
	else if (zoom > 0)
		g_scale_factor = g_scale_factor / 1.05;
	else
		g_scale_factor = g_scale_factor / 0.95;

	printf("g_scale_factor atualizado= %f\n", g_scale_factor);
	reset_gaussian_filter_parameters();
	ForwardVisualSearchNetwork(NULL);
	//f_keyboard("s");
}

void 
f_keyboard(char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
		// Saccade until reach the target
		case 'A':
		case 'a':
			saccade(&in_pattern);
			break;	
		case 'C':	
		case 'c':
			printf("Conf %f\n", g_confidence);
			break;
		// Train network
		case 'T':	
		case 't':
			train_visual_search();
			break;
		// Train network in tunning
		case 'O':	
		case 'o':
			train_visual_search_tunning();
			break;

		// Saccade until reach the target
		case 'S':
		case 's':
			saccade_visual_search();
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
			//printf("The network has forgotten the training\n");
			clear_neural_layers_memory("nl_v1_activation_map");

			ForwardVisualSearchNetwork(NULL);
			
			break;
		//Zoom
		case 'Z':
			printf("g_scale_factor = %f\n", g_scale_factor);
			g_scale_factor *= 0.95;
			reset_gaussian_filter_parameters();
			ForwardVisualSearchNetwork(NULL);
			printf("g_scale_factor = %f\n", g_scale_factor);
			break;
		case 'z':
			printf("g_scale_factor = %f\n", g_scale_factor);
			g_scale_factor *= 1.05;
			reset_gaussian_filter_parameters();
			ForwardVisualSearchNetwork(NULL);
			printf("g_scale_factor = %f\n", g_scale_factor);
			break;
		case 'D':
		case 'd':
			printing_zoom();
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
MoveToPoint(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = pParamList->next->param.ival;
	in_pattern.wyd = pParamList->next->next->param.ival;

	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
	//draw_visual_search_attention_rectangle_point(&in_pattern);
	output.ival = 0;

	return (output);
}


int
max_1(int a, int b)
{
	return (a < b) ? b : a;
}


int
min_1(int a, int b)
{
	return (b < a) ? b : a;
}


double
getJaccardCoefficient(int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow)
{
	double jaccCoeff = 0.;

	if (!(leftCol > gtRightCol || rightCol < gtLeftCol || topRow > gtBottomRow || bottomRow < gtTopRow))
	{
		int interLeftCol = max_1(leftCol, gtLeftCol);
		int interTopRow = max_1(topRow, gtTopRow);
		int interRightCol = min_1(rightCol, gtRightCol);
		int interBottomRow = min_1(bottomRow, gtBottomRow);

		const double areaIntersection = (abs(interRightCol - interLeftCol) + 1) * (abs(interBottomRow - interTopRow) + 1);
		const double lhRoiSize = (abs(rightCol - leftCol) + 1) * (abs(bottomRow - topRow) + 1);
		const double rhRoiSize = (abs(gtRightCol - gtLeftCol) + 1) * (abs(gtBottomRow - gtTopRow) + 1);

		jaccCoeff = areaIntersection / (lhRoiSize + rhRoiSize - areaIntersection);
	}

	return jaccCoeff;
};


int
IsNeedRetrain()
{
	int retrain = 0;
	int i;

	i = g_nCurrentImageInfo;

	if (g_image_info[i].out_of_scene == 1)
		return 0;

	if ((g_confidence < MIN_CONFIANCE_TO_RETRAIN))
		retrain = 1;

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
	int numRetrain = ReturnMaxConfidenceStored(g_nImageOrder, NUM_IMAGES_BEFORE_TO_RETRAIN);
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
   	static double true_positive = 0.0;
   	static double false_positive = 0.0;
   	static double false_negative = 0.0;
   	static double true_negative = 0.0;
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

		if (g_image_info[i].out_of_scene == 1)
			true_negative = true_negative + 1.0;

   		if ((g_image_info[i].best_x != -1) && (g_image_info[i].best_y != -1))
   			false_positive = false_positive + 1.0;

		precision = (true_positive) / (valid_frames + false_positive);
		recall = (true_positive) / (valid_frames);
		f = (2.0 * precision * recall) / (precision + recall);

		printf("image = %05d; scale_factor = % .2lf; confidence = % .2lf; x = %03d; y = %03d; jaccard = % .2lf; precision = % .3lf; recall = % .3lf; f = % .3lf, %s\n",
		g_image_info[i].ImgNo, g_scale_factor, g_confidence, g_image_info[i].best_x, g_image_info[i].best_y, 0.0, precision, recall, f, "FORA DE CENA!");

		printf("true_negative = % .3lf\n ", true_negative);

		output.ival = 2;
	}
	else
	{
		valid_frames += 1.0;		

		altura = (int) ((g_scale_factor_init / g_scale_factor) * height_in_train);
		largura = (int) ((g_scale_factor_init / g_scale_factor) * width_in_train);
		altura = abs(altura);
		largura = abs(largura);

		x1 = (int) ((double) g_image_info[i].best_x - (double) largura / 2.0);
		y1 = (int) ((double) g_image_info[i].best_y - (double) altura / 2.0);
		x2 = x1 + largura;
		y2 = y1 + altura;
		x1_j = x1;
		y1_j = (IMAGE_HEIGHT - 1) - y2;
		x2_j = x2;
		y2_j = (IMAGE_HEIGHT - 1) - y1;

		//TLD
		//jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

		//vot2014 e online
		jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol,  g_image_info[i].bottomRow, g_image_info[i].rightCol,g_image_info[i].topRow);

 		if (jaccard >= JACCARD_FACTOR)
    			true_positive = true_positive + 1.0;
		else
		{
			false_negative = false_negative + 1.0;
   			//false_positive = false_positive + 1.0;
		}

		precision = (true_positive) / (valid_frames + false_positive);
		recall = (true_positive) / (valid_frames);
		f = (2.0 * precision * recall) / (precision + recall);

		printf("image = %05d; scale_factor = % .2lf; confidence = % .2lf; x = %03d; y = %03d; jaccard = % .2lf; precision = % .3lf; recall = % .3lf; f = % .3lf, %s\n",
				g_image_info[i].ImgNo, g_scale_factor, g_confidence,  g_image_info[i].best_x, g_image_info[i].best_y, jaccard, precision, recall, f, (jaccard >= JACCARD_FACTOR)? "HIT": "MISS ***");

		fflush(stdout);

		output.ival = 0;
	}

	return (output);
}



NEURON_OUTPUT
CheckDetection(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int largura;
	int altura;
	int i;
	double jaccard;
   	int x1, y1, x2, y2, x1_j, y1_j, x2_j, y2_j;
	
	i = g_nImageOrder;

	if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow))
	{
		output.fval = -1.0;
	}
	else
	{
		altura = (int) ((g_scale_factor_init / g_scale_factor) * height_in_train);
		largura = (int) ((g_scale_factor_init / g_scale_factor) * width_in_train);
		altura = abs(altura);
		largura = abs(largura);

		x1 = (int) ((double) in_pattern.wxd - (double) largura / 2.0);
		y1 = (int) ((double) in_pattern.wyd - (double) altura / 2.0);
		x2 = x1 + largura;
		y2 = y1 + altura;
		x1_j = x1;
		y1_j = (IMAGE_HEIGHT - 1) - y2;
		x2_j = x2;
		y2_j = (IMAGE_HEIGHT - 1) - y1;

		jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);
		
		output.fval = jaccard;
		fflush(stdout);
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
	FILE *result_file;
   	int largura;
   	int altura;
   	double true_positive = 0.0;
   	double false_positive = 0.0;
   	double false_negative = 0.0;
   	double true_negative = 0.0;
   	double valid_frames = 0.0;
   	double invalid_frames = 0.0;

   	float count;
   	float precision;
   	float recall;
   	float f;

   	count = 0.0;

	//TLD
	//result_file = fopen("resultadoTLD.txt", "w");

	//vot
	result_file = fopen("resultadovot2014.txt", "w");

	//Online
	//result_file = fopen("resultadooot.txt", "w");


	for (i = 0; i < g_num_image_info; i++)
	{
		count = count + 1.0;

		if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow))
    		{
    			invalid_frames += 1.0;
	
    			if (g_image_info[i].out_of_scene == 1)
			{
    				true_negative = true_negative + 1.0;
				fprintf(result_file, "HIT-%05d.jpg.ppm;best_x=%d;best_y=%d;%f\n",
    				g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y, g_image_info[i].best_scale_factor);
			}
    			if ((g_image_info[i].best_x != -1) && (g_image_info[i].best_y != -1))
    			{
    				false_positive = false_positive + 1.0;
    				fprintf(result_file, "MISS-%05d.jpg.ppm;best_x=%d;best_y=%d;%f\n",
    				g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y, g_image_info[i].best_scale_factor);
    			}
   		}
   		else
   		{
			valid_frames += 1.0;

			altura = (int) ((g_scale_factor_init / g_image_info[i].best_scale_factor) * height_in_train + 0.5);
			largura = (int) ((g_scale_factor_init / g_image_info[i].best_scale_factor) * width_in_train + 0.5);
 
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

			//TLD
			//jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

			//vot2014 e Online
			jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol,  g_image_info[i].bottomRow, g_image_info[i].rightCol,g_image_info[i].topRow);

    			if (jaccard >= JACCARD_FACTOR)
    			{
    				fprintf(result_file, "HIT-Jaccard=%f;", jaccard);
    				true_positive = true_positive + 1.0;
    			}
   			else
			{
				fprintf(result_file, "MISS-Jaccard=%f;", jaccard);
				false_negative = false_negative + 1.0;
//   				false_positive = false_positive + 1.0;

			}
   			//imprimindo o bound box
   			fprintf(result_file, "%05d.jpg.ppm;best_x=%d;best_y=%d;my_leftcol=%d;my_toprow=%d;my_rightcol=%d;my_bottomrow=%d;my_scale_factor=%f, my_confidence=%f, used_for_training=%d\n",
   				g_image_info[i].ImgNo, g_image_info[i].best_x, g_image_info[i].best_y, x1_j, y1_j, x2_j, y2_j, g_image_info[i].best_scale_factor, g_image_info[i].best_confidence, 					g_image_info[i].used_for_trainning);
		}
   	}

	precision = (true_positive) / (valid_frames + false_positive);
	recall = (true_positive) / (valid_frames);
	f = (2.0 * precision * recall) / (precision + recall);

	fprintf(result_file, "true_positive;false_positive;false_negative;true_negative;PRECISION;RECALL;F\n");
	fprintf(result_file, "% .3lf; % .3lf; % .3lf; % .3lf; % .3lf; % .3lf; % .3lf\n", true_positive, false_positive, false_negative, true_negative, precision, recall, f);

	printf("true_positive;false_positive;false_negative;true_negative;PRECISION;RECALL;F\n");
	printf("% .3lf; % .3lf; % .3lf;% .3lf; % .3lf; % .3lf;% .3lf\n", true_positive, false_positive, false_negative, true_negative, precision, recall, f);

	printf("total_frames, valid_frames; invalid_frames\n");
	printf("%f; %f; %f\n", valid_frames + invalid_frames, valid_frames, invalid_frames);

	fclose(result_file);
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


NEURON_OUTPUT
GetScaleFactorInTrain(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = g_scale_factor_init;
	return (output);

}


NEURON_OUTPUT
GetScaleFactorZoom(PARAM_LIST *pParamList)
{
	double x;
	int i;
	double count;
	NEURON_OUTPUT output;
	float maior, z;
	float multiplicador;
	int zoom = 0;

	x = 0.0;
	count = 0.0;
	maior = 0.0;

	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		z = nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
		if ( z != 0)
		{
			x += (double)i * z;
			count += z;
			if (z > maior)
			{
				maior = z;
			}
		}
	}

	if (count != 0.0)
	{
		if (x/count == 0.0)
			zoom = 0;
		else
		{
			if ((x/count >= (float)(TAM_NL_ZOOM/2 - BAND_ZOOM)) && (x/count <= (float)(TAM_NL_ZOOM/2 + BAND_ZOOM)))
				zoom = 0;
			else
			{
				if ((int)(x/count) > (TAM_NL_ZOOM/2))
					zoom = -1;
				else
					zoom = 1;
			}
		}
	}
	else
		zoom = 0;

	if ((g_confidence_zoom_init - maior) > 0)
	{
		multiplicador = (g_confidence_zoom_init - maior) / g_confidence_zoom_init; 
	}
	else
		multiplicador = 1.0;

	if (zoom == 0)
		g_scale_factor = g_scale_factor;
	else if (zoom > 0)
		g_scale_factor = g_scale_factor / (1 + (0.05 * multiplicador));
	else
		g_scale_factor = g_scale_factor / (1 - (0.05 * multiplicador));


	output.fval = g_scale_factor;
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
GetX_before(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;
	output.ival = g_image_info[i-1].best_x;

	return (output);

}


NEURON_OUTPUT
GetY_before(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;
	output.ival = g_image_info[i-1].best_y;

	return (output);
}


NEURON_OUTPUT
GetMinConfidenceToRetrain(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = MIN_CONFIANCE_TO_RETRAIN;

	return (output);

}

NEURON_OUTPUT
GetConfidenceLevel(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = MIN_CONFIANCE_TO_ACCEPT_RETRAIN;

	return (output);

}


NEURON_OUTPUT
GetNumPixels(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.ival = NUM_PIXELS;

	return (output);

}


NEURON_OUTPUT
GetScaleBefore(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	if (g_nCurrentImageInfo > 1)
	{
		// estava fora de cena
		if (g_image_info[g_nCurrentImageInfo-1].out_of_scene == 1)
			output.fval = g_scale_factor;
		else
		{	
			output.fval = g_image_info[g_nCurrentImageInfo-1].best_scale_factor;
		}
	}
	else
		output.fval = g_scale_factor_init;
	return (output);

}


NEURON_OUTPUT
GetScaleToRetrain(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;

	if (g_nCurrentImageInfo > 1)
		output.fval = g_image_info[i].best_scale_factor;
	else
		output.fval = g_scale_factor_init;

	return (output);

}


NEURON_OUTPUT
GetMaxScale(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = MAX_SCALE;
	return (output);

}


NEURON_OUTPUT
GetMinScale(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.fval = MIN_SCALE;
	return (output);

}


NEURON_OUTPUT
GetNumFrames(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.ival = NUM_FRAMES;
	
	return (output);

}


NEURON_OUTPUT
GetOutOfScene(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.ival = out_of_scene;
	
	return (output);

}


NEURON_OUTPUT 
SetOutOfScene(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	out_of_scene = 1;

	output.ival = 0;

	return (output);
}


NEURON_OUTPUT
GetImageOutOfScene(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i = pParamList->next->param.ival;

	if (g_nCurrentImageInfo > 1)
		output.ival = g_image_info[i].out_of_scene;
	else
		output.ival = 0;

	return (output);

}


NEURON_OUTPUT
ForwardVisualSearchNetwork(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	forward_objects("in_pattern_translated_filter");
	forward_objects("in_pattern_filtered_translated_filter");
	forward_objects("in_pattern_filtered_translated_red_filter");
	forward_objects("in_pattern_filtered_translated_green_filter");
	forward_objects("in_pattern_filtered_translated_blue_filter");
	forward_objects("nl_v1_pattern_filter");
	forward_objects("nl_v1_activation_map");
	forward_objects("table_v1_filter");
	forward_objects("nl_v1_activation_map_neuron_weight_filter");
	forward_objects("nl_v1_activation_map_neuron_weight_thresholded_filter");
	forward_objects("nl_activation_map_hough_filter");
	forward_objects("nl_activation_map_hough_gaussian_filter");
	forward_objects("nl_activation_map_hough_v1_filter");
	forward_objects("nl_activation_map_hough_zoom_filter");
	forward_objects("nl_activation_map_hough_zoom_gaussian_filter");
	all_outputs_update();

	output.ival = 0;

	return (output);
}


NEURON_OUTPUT
GetConfidenceZoom(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	float maior = 0.0;
	float z = 0.0;
	int i;

	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		z = nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
		if ( z != 0)
		{
			if (z > maior)
			{
				maior = z;
			}	
		}
	}

	output.fval = maior;

	return (output);

}


NEURON_OUTPUT
GetDistanceZoom(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	float maior = 0.0;
	float z = 0.0;
	int i_maior = 0;
	int i;

	for (i = 0; i < TAM_NL_ZOOM; i++)
	{
		z = nl_activation_map_hough_zoom_gaussian.neuron_vector[i].output.fval;
		if ( z != 0)
		{
			if (z > maior)
			{
				maior = z;
				i_maior = i;
			}	
		}
	}

	output.ival = abs(TAM_NL_ZOOM/2 - i_maior);

	return (output);

}


NEURON_OUTPUT
DisposeMemory(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	if (in_pattern.rectangle_list != NULL)
		free(in_pattern.rectangle_list);
	if (g_image_info != NULL)
		free(g_image_info);

	output.ival = 0;

	return (output);
}


NEURON_OUTPUT
MoveCloseToObject(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	in_pattern.wxd = ((g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2);
	in_pattern.wyd = ((((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2);
	printf("Close;x;%d;y;%d\n", in_pattern.wxd, in_pattern.wyd);
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
MoveFarWayFromObject(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int desloc = 0;

	int x = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
	int x_center = IMAGE_WIDTH / 2;
	int delta_x =  ((double) IMAGE_WIDTH_RESIZED);
	//int delta_x =  (int) (1.0 * (((double) IMAGE_WIDTH_RESIZED) / g_scale_factor));
	if ( x < x_center)
		desloc = x + delta_x;
	else
		desloc = x - delta_x;

	if (desloc > IMAGE_WIDTH)
		desloc = IMAGE_WIDTH - 1;

	else if (desloc < 0)
		desloc = 0;

	in_pattern.wxd = desloc;

	in_pattern.wyd = ((((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2);

	printf("Far;x;%d;y;%d\n", in_pattern.wxd, in_pattern.wyd);
	set_input_layer_translation(&in_pattern, in_pattern.wxd, in_pattern.wyd);

	output.ival = 0;
	return (output);

}


NEURON_OUTPUT
PrintEvalution(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float confidence1 = pParamList->next->param.fval;
	float confidence2 = pParamList->next->next->param.fval;
	float jaccard1 = pParamList->next->next->next->param.fval;
	float jaccard2 = pParamList->next->next->next->next->param.fval;
	float factor = 0.0;

	factor = confidence1-confidence2;
	if (factor < 0 )
	{
		factor = 0.0;
	}
	else
	{
		factor/= confidence1;

	}

	printf("Image;%05d.ppm;Confidence1;%f;Jaccard1;%f;Confidence2;%f;Jaccard2;%f;Factor;%f;Scale;%f\n",
	g_image_info[g_nCurrentImageInfo].ImgNo, confidence1, jaccard1, confidence2, jaccard2, factor, g_scale_factor);

	output.ival = 0;
	return (output);
}


