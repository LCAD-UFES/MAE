#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <locale.h>
#include <sys/stat.h>
#include <errno.h>
#include "visual_search_user_functions.h"
#include "visual_search_filters.h"

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
	int image; //g_signUniqueID
	int image1; //g_signID
	int image2; //g_sign_frameID
	double probability;
	double confidence;
	int x;
	int y;
	int roi_x1, roi_y1, roi_x2, roi_y2;
	int width, height;
	int found;
	int id;
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

double g_confidence;
double g_scale_factor = 0.5;
double g_delta_x_d, g_delta_y_d;

int g_flip_horizontaly = 0;
int g_flip_verticaly = 0;

int g_x_trained;
int g_y_trained;

int g_delta_x_x;
int g_delta_x_y;
int g_delta_y_x;
int g_delta_y_y;
int g_initial_delta_x;
int g_initial_delta_y;

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
	g_input_path = TRAINING_INPUT_PATH;
//	sprintf(strFileName, "%s%05d.ppm", g_input_path, g_data[g_nImageOrder].image);
	sprintf(strFileName, "%s%05d_%05d_%05d.ppm", g_input_path, g_data[g_nImageOrder].image,  g_data[g_nImageOrder].image1,  g_data[g_nImageOrder].image2);
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
	return ((float) IMAGE_WIDTH / 3.0) * g_scale_factor;
}

/*
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
*/

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

int
ReadFrameInput (INPUT_DESC *input, char *strFileName)
{
	int y, x;
	int input_height, input_width;
	int r, g, b;
	CvRect roi;
	uchar *image_pixel;
	IplImage *img = NULL;
	IplImage *img_resized = NULL;

	/* Clear image */
	for (y = 0; y <= input->tfw * input->tfh * 3; y++)
		input->image[y] = (GLubyte) 0;

	if (input->neuron_layer->output_type == COLOR)
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_COLOR);
	else if (input->neuron_layer->output_type == GREYSCALE)
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_GRAYSCALE);
	else
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_UNCHANGED);

	input_width = input->vpw;
	input_height = input->vph;

	img_resized = cvCreateImage(cvSize(input_width, input_height), img->depth, img->nChannels);
	
	roi.x = g_data[g_nImageOrder].roi_x1;
	roi.y = g_data[g_nImageOrder].roi_y1;
	roi.width = g_data[g_nImageOrder].roi_x2 - g_data[g_nImageOrder].roi_x1;
	roi.height = g_data[g_nImageOrder].roi_y2 - g_data[g_nImageOrder].roi_y1;

	cvSetImageROI(img, roi);

	cvResize(img, img_resized, CV_INTER_LINEAR);

	rotate_scale_image(img_resized, 0.0, g_scale_factor, input_width/2, input_height/2);

	for (y = input_height-1; y > 0; y--)
	{
		for (x = 0; x < input_width; x++)
		{
			image_pixel = (uchar*) (img_resized->imageData + (input_height-1-y) * img_resized->widthStep);
			switch (img_resized->nChannels)
			{
			case 3:
				r = (int) image_pixel[3*x+2];
				g = (int) image_pixel[3*x+1];
				b = (int) image_pixel[3*x+0];
			break;
			default:
			case 1:
				r = (int) image_pixel[3*x+0];
				g = (int) image_pixel[3*x+0];
				b = (int) image_pixel[3*x+0];
			break;
			}

			input->image[3 * (y * input->tfw + x) + 0] = (GLubyte) r;
			input->image[3 * (y * input->tfw + x) + 1] = (GLubyte) g;
			input->image[3 * (y * input->tfw + x) + 2] = (GLubyte) b;
		}
	}

	cvReleaseImage(&img);
	cvReleaseImage(&img_resized);

	return (0);
}

void 
LoadImage(INPUT_DESC *input, char *strFileName)
{
	//load_input_image(input, strFileName);
	ReadFrameInput (input, strFileName);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
	update_input_image (input);

	g_flip_horizontaly = 0;
	g_flip_verticaly = 0;

//	draw_traffic_sign_rectangles(input);
	
	input->wxd = g_data[g_nImageOrder].x;// + IMAGE_WIDTH / 12;
	input->wyd = g_data[g_nImageOrder].y;// + IMAGE_WIDTH / 12;
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
get_input_by_file_line_number_gtsrb(int line_number, char *line_value)
{
	int sign_unique_ID, sign_ID, sign_frame_ID;
	int width, height, roi_x1, roi_y1, roi_x2, roi_y2, id;
	char file_name[256];

	if (sscanf (line_value, "%s %d %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2, &id) != 8)
		Erro("Could not read line from signs_data_file in get_input_by_file_line_number_gtsrb()", "", "");

	sscanf (file_name, "%05d_%05d_%05d.ppm", &sign_unique_ID, &sign_ID, &sign_frame_ID);

	g_data[line_number].image = sign_unique_ID;
	g_data[line_number].image1 = sign_ID;
	g_data[line_number].image2 = sign_frame_ID;

	g_data[line_number].width = width;
	g_data[line_number].height = height;

	g_data[line_number].roi_x1 = roi_x1;
	g_data[line_number].roi_y1 = roi_y1;
	g_data[line_number].roi_x2 = roi_x2;
	g_data[line_number].roi_y2 = roi_y2;

	g_data[line_number].x = (int) ((double) (IMAGE_WIDTH) / 2.0 + 0.5);
	g_data[line_number].y = (int) ((double) (IMAGE_HEIGHT) / 2.0 + 0.5);

	g_data[line_number].id = id;
}

void
read_input_data_file_gtrsb()
{
	char *file_name;
	FILE *image_list = NULL;
	char file_line[512];
	int num_lines = 0;

	file_name = "input_data_roi.txt";

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
		if (fgets(file_line, 256, image_list) != NULL)
			get_input_by_file_line_number_gtsrb(g_num_data, file_line);
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
//	read_input_data_file();
	read_input_data_file_gtrsb();
	
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
	int x, y;
	
	make_input_image_visual_search(input, IMAGE_WIDTH, IMAGE_HEIGHT);

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
		glutSetWindow(input->win);
		input_display();
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
		init_visual_search(input);
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
	map_v1_to_image(&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u_max, v_max, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

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
		d_delta_x = x;
		if (d_delta_x > 0.0)
			delta_x = (int) (d_delta_x + 0.5);
		else if (d_delta_x < 0.0)
			delta_x = (int) (d_delta_x - 0.5);
		else
			delta_x = 0;

		d_delta_y = y;
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
	while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < MAX_SACCADE_STEPS));

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
			break;	
		// Move to the good horizontal training point
		case 'H':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wxd = (int) ((0.2 * (double) (IMAGE_WIDTH) / 2.0 + 0.5)/g_scale_factor);
			g_initial_delta_x = (int) ((0.2 * (double) (IMAGE_WIDTH) / 2.0 - (double) (IMAGE_WIDTH) / 2.0 + 0.5)/g_scale_factor);
			in_pattern.wyd = (int) ((double) (IMAGE_WIDTH) / 2.0 + 0.5);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Flip image
		case 'h':
			g_flip_horizontaly = 1;
			g_flip_verticaly = 0;
			in_pattern.wxd = (int) (((double) (IMAGE_WIDTH) - 0.2 * (double) (IMAGE_WIDTH) / 2.0 + 0.5)/g_scale_factor);
			in_pattern.wyd = (int) ((double) (IMAGE_WIDTH) / 2.0 + 0.5);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Move to the good vertical training point
		case 'V':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wyd = (int) ((0.2 * (double) (IMAGE_HEIGHT) / 2.0 + 0.5)/g_scale_factor);
			g_initial_delta_y = (int) ((0.2 * (double) (IMAGE_HEIGHT) / 2.0 - (double) (IMAGE_HEIGHT) / 2.0 + 0.5)/g_scale_factor);
			in_pattern.wxd = (int) (((double) (IMAGE_HEIGHT) / 2.0 + 0.5)/g_scale_factor);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		case 'v':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 1;
			in_pattern.wyd = (int) (((double) (IMAGE_HEIGHT) - 0.2 * (double) (IMAGE_HEIGHT) / 2.0 + 0.5)/g_scale_factor);
			in_pattern.wxd = (int) ((((double) (IMAGE_HEIGHT) / 2.0) + 0.5)/g_scale_factor);
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
			g_data[g_nImageOrder].x -= (int) ((double) g_delta_x_x / 2.0 - g_initial_delta_x + 0.5);
			g_data[g_nImageOrder].y -= (int) ((double) g_delta_y_y / 2.0 - g_initial_delta_y + 0.5);
			g_data[g_nImageOrder].found = 1;
			in_pattern.wxd = g_data[g_nImageOrder].x;
			in_pattern.wyd = g_data[g_nImageOrder].y;
			// TODO: check_if_it_was_a_hit_and_print_info_if_not();
//			draw_traffic_sign_rectangles(&in_pattern);
			move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
			break;	
		// Go to final position after centering
		case 'G':
		case 'g':
			g_flip_horizontaly = 0;
			g_flip_verticaly = 0;
			in_pattern.wxd = g_data[g_nImageOrder].x - (int) ((double) g_delta_x_x / 2.0 - g_initial_delta_x + 0.5);
			in_pattern.wyd = g_data[g_nImageOrder].y - (int) ((double) g_delta_y_y / 2.0 - g_initial_delta_y + 0.5);
//			draw_traffic_sign_rectangles(&in_pattern);
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

	output.ival = (int) ( (double)IMAGE_WIDTH / 4.0)*g_scale_factor;
	
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


double 
getJaccardCoefficient(int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow)
{
	double jaccCoeff = 0.;

	if (!(leftCol > gtRightCol || rightCol < gtLeftCol || topRow > gtBottomRow || bottomRow < gtTopRow))
	{
		int interLeftCol = MAX(leftCol, gtLeftCol);
		int interTopRow = MAX(topRow, gtTopRow);
		int interRightCol = MIN(rightCol, gtRightCol);
		int interBottomRow = MIN(bottomRow, gtBottomRow);

		const double areaIntersection = (abs(interRightCol - interLeftCol) + 1) * (abs(interBottomRow - interTopRow) + 1);
		const double lhRoiSize = (abs(rightCol - leftCol) + 1) * (abs(bottomRow - topRow) + 1);
		const double rhRoiSize = (abs(gtRightCol - gtLeftCol) + 1) * (abs(gtBottomRow - gtTopRow) + 1);

		jaccCoeff = areaIntersection / (lhRoiSize + rhRoiSize - areaIntersection);
	}

	return jaccCoeff;
};


NEURON_OUTPUT
SaveOutputLine(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	FILE *gtdsb_result_file;
	int xs, ys, xr, yr, dx, dy;
	double scale_factor_x, scale_factor_y;

	gtdsb_result_file = fopen("output_data_roi.txt", "a");
	fseek(gtdsb_result_file, 0, SEEK_END);

	if (g_data[g_nImageOrder].found)
	{
		scale_factor_x = (double)IMAGE_WIDTH / (double)g_data[g_nImageOrder].width;
		scale_factor_y = (double)IMAGE_HEIGHT / (double)g_data[g_nImageOrder].height;

		xs = g_data[g_nImageOrder].x;
		ys = (IMAGE_HEIGHT - 1) - g_data[g_nImageOrder].y;

		xs = (int)( ((double)xs) / scale_factor_x );
		ys = (int)( ((double)ys) / scale_factor_y );

		xr = g_data[g_nImageOrder].roi_x1 + (g_data[g_nImageOrder].roi_x2 - g_data[g_nImageOrder].roi_x1)/2;
		yr = g_data[g_nImageOrder].roi_y1 + (g_data[g_nImageOrder].roi_y2 - g_data[g_nImageOrder].roi_y1)/2;
//		xr *= scale_factor_x;
//		yr *= scale_factor_y;

		dx = (int)((double) (xs - xr) /*/ scale_factor_x*/);
		dy = (int)((double) (ys - yr) /*/ scale_factor_y*/);

		if (xs > xr)
		{
			g_data[g_nImageOrder].roi_x1 += dx;
			g_data[g_nImageOrder].roi_x2 += dx;
		}
		else
		{
			g_data[g_nImageOrder].roi_x1 -= dx;
			g_data[g_nImageOrder].roi_x2 -= dx;
		}
		if (ys > yr)
		{
			g_data[g_nImageOrder].roi_y1 += dy;
			g_data[g_nImageOrder].roi_y2 += dy;
		}
		else
		{
			g_data[g_nImageOrder].roi_y1 -= dy;
			g_data[g_nImageOrder].roi_y2 -= dy;
		}

	}
	fprintf(gtdsb_result_file, "%05d_%05d_%05d.ppm %d %d %d %d %d %d %d\n",
			g_data[g_nImageOrder].image, g_data[g_nImageOrder].image1, g_data[g_nImageOrder].image2,
			g_data[g_nImageOrder].width, g_data[g_nImageOrder].height,
			g_data[g_nImageOrder].roi_x1, g_data[g_nImageOrder].roi_y1,
			g_data[g_nImageOrder].roi_x2, g_data[g_nImageOrder].roi_y2,
			g_data[g_nImageOrder].id);

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
