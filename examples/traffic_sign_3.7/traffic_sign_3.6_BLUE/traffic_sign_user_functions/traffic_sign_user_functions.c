#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "traffic_sign_user_functions.h"
#include "traffic_sign_utils.h"

//############### Global Variables ###############
int g_signUniqueID = -1; //id da classe da placa
int g_signID = -1; //id da placa
int g_sign_frameID = -1; //frame de uma placa (uma placa aparece varias vezes)

int g_fptr;

int g_nStatus;
int g_nTry = N_RECALL_TRIES;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;

int g_nTotalImagesDatasetTest = 0;
int g_nTotalImagesDatasetTrain = 0;
int g_nCurrentImageDatasetTest = 0;
int g_nCurrentImageDatasetTrain = 0;

char g_strSignsFileName[256];
FILE *g_signs_data_file = NULL;
FILE *hit_statistics_csv_file = NULL;

#define	NUM_CANDIDATES	N_DEFINED_SIGNS

struct _sign_count
{
	int sign_code;
	float sign_count;
	float confidence;
};

typedef struct _sign_count SIGN_COUNT;

SIGN_COUNT best_sign;

SIGN_COUNT sign_tries[N_RECALL_TRIES];

SIGN_COUNT sign_count[N_DEFINED_SIGNS];

SIGN_COUNT sign_count_cadidates[NUM_CANDIDATES * N_RECALL_TRIES];


//############### User Functions ###############
//***********************************************************
//* Function: ReadFrameInput
//* Description:
//***********************************************************
int
ReadFrameInput (INPUT_DESC *input, char *strFileName)
{
	int y, x, h, w;
	int r, g, b;
	uchar *image_pixel;
	IplImage *img = NULL;

	/* Clear image */
	for (y = 0; y <= input->tfw * input->tfh * 3; y++)
		input->image[y] = (GLubyte) 0;

	if (input->neuron_layer->output_type == COLOR)
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_COLOR);
	else if (input->neuron_layer->output_type == GREYSCALE)
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_GRAYSCALE);
	else
		img = cvLoadImage(strFileName, CV_LOAD_IMAGE_UNCHANGED);

	h = img->height;
	w = img->width;
	g_img_x1 = input->vpw/2-w/2;
	g_img_y1 = input->vph/2-h/2;

	for (y = h-1; y > 0; y--)
	{
		for (x = 0; x < w; x++)
		{
			image_pixel = (uchar*) (img->imageData + (h-1-y) * img->widthStep);
			switch (img->nChannels)
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

			input->image[3 * ((g_img_y1+y) * input->tfw + (g_img_x1+x)) + 0] = (GLubyte) r;
			input->image[3 * ((g_img_y1+y) * input->tfw + (g_img_x1+x)) + 1] = (GLubyte) g;
			input->image[3 * ((g_img_y1+y) * input->tfw + (g_img_x1+x)) + 2] = (GLubyte) b;
		}
	}

	cvReleaseImage(&img);

	return (0);
}

//***********************************************************
//* Function: MakeInputImage
//* Description:
//***********************************************************
void MakeInputImage (INPUT_DESC *input, int w, int h)
{
	char message[256];

	//texture frame width and height, must be powers of 2
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	//window width and height
	input->ww = w;
	input->wh = h;

	switch(TYPE_SHOW)
	{
	case SHOW_FRAME:
		//visible part (of the window) width and height
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
	//origin x and y of the input window
	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if(input->image == NULL)
	{
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
	}
}


void
get_input_by_file_line_number(int line_number)
{
	int width, height, roi_x1, roi_y1, roi_x2, roi_y2, id, i;
	char file_name[256];
	char file_path[256];

	rewind(g_signs_data_file);
	i = 0;
	while (i < line_number)
	{
		if (fscanf (g_signs_data_file, "%s %d %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2, &id) != 8)
			Erro("Could not read line from signs_data_file in get_sign_by_index()", "", "");
		i++;
	}

	sscanf (file_name, "%05d_%05d_%05d.ppm", &g_signUniqueID, &g_signID, &g_sign_frameID);
	g_img_w = width;
	g_img_h = height;
	g_roi_x1 = roi_x1;
	g_roi_y1 = roi_y1;
	g_roi_x2 = roi_x2;
	g_roi_y2 = roi_y2;
	if (g_nStatus == TRAINING_PHASE)
		sprintf (file_path, "%s%s", TRAINING_INPUT_PATH, file_name);
	else if (g_nStatus == RECALL_PHASE)
		sprintf (file_path, "%s%s", TESTING_INPUT_PATH, file_name);
	else
		Erro("Invalid net status in get_input_by_file_line_number()", "", "");

	/*read sign image*/
	ReadFrameInput (&traffic_sign, file_path);

	check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
	traffic_sign.up2date = 0;
	update_input_neurons (&traffic_sign);
	update_input_image (&traffic_sign);
}

//***********************************************************
//* Function: get_first_sign_id
//* Description:
//***********************************************************
int get_first_sign_id ()
{
	fscanf(g_signs_data_file, "%05d_%05d_%05d %d %d %d %d %d %d\n", &g_signUniqueID, &g_signID, &g_sign_frameID, &g_img_w, &g_img_h, &g_roi_x1, &g_roi_y1, &g_roi_x2, &g_roi_y2);
	return 0;
}

//***********************************************************
//* Function: get_next_sign_id
//* Description:
//***********************************************************
void
get_next_sign_id(int direction)
{
	int *index;
	int *limit;
	if (g_nStatus == TRAINING_PHASE)
	{
		index = &g_nCurrentImageDatasetTrain;
		limit = &g_nTotalImagesDatasetTrain;
	}
	if (g_nStatus == RECALL_PHASE)
	{
		index = &g_nCurrentImageDatasetTest;
		limit = &g_nTotalImagesDatasetTest;
	}
	if (direction == DIRECTION_FORWARD)
	{
		if (*index < *limit)
			(*index)++;
		get_input_by_file_line_number(*index);
	}
	else //direction == DIRECTION_REWIND
	{
		if (*index > 1)
			(*index)--;
		get_input_by_file_line_number(*index);
	}
}

//***********************************************************
//* Function: get_file_lines_len
//* Description:
//***********************************************************
int 
get_file_lines_len (char *file)
{
	int len;
	FILE * linhas;
	char command[128];

	sprintf(command, "wc -l %s > linhas.deleteme;", file);
	system(command);
	linhas = fopen("linhas.deleteme", "r");
	fscanf(linhas, "%d %*s\n", &len);
	system("rm linhas.deleteme");

	return len;
}

int 
set_network_status_interface (int net_status)
{
	switch (net_status)
	{
	case TRAINING_PHASE:
		g_nTotalImagesDatasetTrain = get_file_lines_len(TRAINING_INPUT_FILES);
		if ((g_signs_data_file = fopen (TRAINING_INPUT_FILES, "r")) == NULL)
		{
			printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TRAINING_INPUT_FILES);
			return (-1);
		}
		break;
	case RECALL_PHASE:
		g_nTotalTested = 0;
		g_nTotalImagesDatasetTest = get_file_lines_len(TESTING_INPUT_FILES);
		if ((g_signs_data_file = fopen (TESTING_INPUT_FILES, "r")) == NULL)
		{
			printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TESTING_INPUT_FILES);
			return (-1);
		}
		break;
	default:
		printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
		return (-1);
	}

	g_nStatus = net_status;
	return (0);
}


//***********************************************************
//* Function: init_traffic_sign
//* Description:
//***********************************************************

void init_traffic_sign (INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif
	//char file_name[128];

	g_nStatus = TRAINING_PHASE;
	set_network_status_interface(g_nStatus);
	g_nTotalTested = 0;
	g_nCorrect = 0;

	MakeInputImage (input, INPUT_WIDTH, INPUT_HEIGHT);

	input->up2date = 0;
	update_input_neurons (input);

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);

	if (read_window_position (input->name, &x, &y))
	{
		glutInitWindowPosition (x, y);
	}
	else
	{
		glutInitWindowPosition (-1, -1);
	}
	input->win = glutCreateWindow (input->name);

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
#endif
}

float normal_dist(float x, float mean, float dev)
{
	float e = 2.71829f;

	return (1.0/sqrt(2.0*pi*dev*dev))*(pow(e, ((-1.0)*(((x-mean)*(x-mean))/(2.0*dev*dev)))));
}


void
init_nl_landmark_eval_mask_for_log_polar()
{
	int w, h, x, y;
	NEURON *neuron_vector;
	
	neuron_vector = nl_landmark_eval_mask.neuron_vector;
	w = nl_landmark_eval_mask.dimentions.x;
	h = nl_landmark_eval_mask.dimentions.y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			neuron_vector[y * w + x].output.fval = normal_dist((float)x, (float)(w / 2.0), VOTE_VARIANCE);
		}
	}
	update_output_image (get_output_by_name("nl_landmark_eval_mask_out"));
}


void
init_nl_landmark_eval_mask_for_gaussian()
{
	int w, h, x, y;
	NEURON *neuron_vector;
	
	neuron_vector = nl_landmark_eval_mask.neuron_vector;
	w = nl_landmark_eval_mask.dimentions.x;
	h = nl_landmark_eval_mask.dimentions.y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			neuron_vector[y * w + x].output.fval = normal_dist((float)y, (float)(h / 2.0), VOTE_VARIANCE) *
							       normal_dist((float)x, (float)(w / 2.0), VOTE_VARIANCE);
		}
	}
	update_output_image (get_output_by_name("nl_landmark_eval_mask_out"));
}


//***********************************************************
//* Function: init_user_functions
//* Description:
//***********************************************************
int init_user_functions (void)
{
	char strCommand[128];

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);

	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	init_nl_landmark_eval_mask_for_log_polar();
	//init_nl_landmark_eval_mask_for_gaussian();

	// Open the hit statistics file
	hit_statistics_csv_file = fopen("hit_statistics.csv","w+");
	if(!hit_statistics_csv_file)
	{
		printf("Could not open the Hit statistics file for writing\n");
		exit(1);
	}

	return (0);
}

//***********************************************************
//* Function: input_generator
//* Description:
//***********************************************************
void input_generator (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_traffic_sign(input);
#ifdef NO_INTERFACE
		input->win = 1;
#endif
	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
			{
				get_next_sign_id(DIRECTION_REWIND);
			}
			else if (input->wxd >= INPUT_WIDTH)
			{
				get_next_sign_id(DIRECTION_FORWARD);
			}
			else
				GetConfidence(NULL);

#ifndef NO_INTERFACE            
			glutSetWindow(input->win);
			input_display();
#endif
		}
	}
}

//***********************************************************
//* Function: draw_output
//* Description:
//***********************************************************
void draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;
	output = get_output_by_name (strOutputName);
	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, g_signUniqueID);

	update_output_image (output);
#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
#endif
}

//***********************************************************
//* Function: input_controller
//* Description:
//***********************************************************
void input_controler (INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (strCommand, "draw out_landmark based on traffic_sign move;");
		interpreter (strCommand);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
		interpreter("forward network;");
	}

	input->mouse_button = -1;
}


int
compare_outputs(const void *sign_cout1, const void *sign_cout2)
{
	SIGN_COUNT *val1, *val2;
	
	val1 = (SIGN_COUNT *) sign_cout1;
	val2 = (SIGN_COUNT *) sign_cout2;
	if (val1->confidence == val2->confidence)
		return 0;
	else
		return (val1->confidence < val2->confidence ? 1 : -1);
}

int
compare_outputs_by_counting(const void *sign_cout1, const void *sign_cout2)
{
	SIGN_COUNT *val1, *val2;

	val1 = (SIGN_COUNT *) sign_cout1;
	val2 = (SIGN_COUNT *) sign_cout2;
	if (val1->sign_count == val2->sign_count)
		return 0;
	else
		return (val1->sign_count < val2->sign_count ? 1 : -1);
}

float
EvaluateOutputForLogPolar(OUTPUT_DESC *output)
{
	int i, j;
	int neuron_output;
	float confidence;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	//clear counting
	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		sign_count[i].sign_code = i;
		sign_count[i].sign_count = 0.0;
		sign_count[i].confidence = 0.0;
	}

	//count signs votes
	for (i = 0; i < output->wh; i++)
	{
		for (j = 0; j < output->ww; j++)
		{
			neuron_output = neuron_vector[i*output->wh + j].output.ival;
			if ((neuron_output >= 0) && (neuron_output < N_DEFINED_SIGNS))
			{
				sign_count[neuron_output].sign_count += 1.0 * normal_dist((float)j, (float)(output->wh/2.0), VOTE_VARIANCE);
			}
		}
	}

	//sort asc by votes
	qsort((void *) sign_count, N_DEFINED_SIGNS, sizeof(SIGN_COUNT), compare_outputs_by_counting);

	if (sign_count[0].sign_count > 0.0f)
	{
		confidence = (sign_count[0].sign_count - sign_count[1].sign_count) / sign_count[0].sign_count;
		if (confidence < 0.0f)
			confidence = 0.0;
	}
	else
	{
		confidence = 0.0;
	}

	return confidence;
}


//***********************************************************
//* Function: output_handler
//* Description:
//***********************************************************
void output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int i;

	if (g_nStatus == RECALL_PHASE)
	{
		//avalia a saida
		if (strcmp (output->name, out_landmark.name) == 0);
		{
			if (g_nTry != 0)
			{
				sign_tries[g_nTry - 1].confidence = EvaluateOutputForLogPolar(output);
				sign_tries[g_nTry - 1].sign_code = sign_count[0].sign_code;
				sign_tries[g_nTry - 1].sign_count = sign_count[0].sign_count;

				for (i = 0; i < NUM_CANDIDATES; i++)
					sign_count_cadidates[i + (g_nTry - 1) * NUM_CANDIDATES] = sign_count[i];

				g_nTry--;
			}
			if (g_nTry == 0)
			{
				g_nTry = N_RECALL_TRIES;
			}
		}
	}
	else
	{
		printf("     %4d %4d ",g_signUniqueID, 0);
		printf("%4d %4d\n", g_signID, g_sign_frameID);
	}

#ifndef NO_INTERFACE    
	glutSetWindow (output->win);
	output_display (output);
#endif
}


NEURON_OUTPUT
GetConfidence(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int j, try;

	output.ival = 0;
	best_sign.confidence = 0.0;
	
	for (try = 1; try <= N_RECALL_TRIES; try++)
	{
		j = try - 1;

		if (best_sign.confidence < sign_tries[j].confidence)
		{
			best_sign = sign_tries[j];
			output.ival = (int) (sign_tries[j].confidence * 100.0);
		}
	}
	return (output);
}


NEURON_OUTPUT
PrintStatistics(PARAM_LIST *pParamList)
{
	float hit_percent;
	int i, j, try;
	NEURON_OUTPUT output;
	
	g_nTotalTested++;

	if (best_sign.sign_code == g_signUniqueID)
	{
		g_nCorrect++;

		hit_percent = (float) (g_nCorrect * 100.0 / g_nTotalTested);
		printf("HIT  %4d %4d ", g_signUniqueID, best_sign.sign_code);
		fprintf(stderr, "HIT  %4d %4d ", g_signUniqueID, best_sign.sign_code);
		printf("%4d %4d ", g_signID, g_sign_frameID);
		fprintf(stderr, "%4d %4d ", g_signID, g_sign_frameID);
		printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_sign.confidence);
		fprintf(stderr, "%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_sign.confidence);
	}
	else
	{
		hit_percent = (float) (g_nCorrect * 100.0 / g_nTotalTested);
		printf("MISS %4d %4d ", g_signUniqueID, best_sign.sign_code);
		fprintf(stderr, "MISS %4d %4d ", g_signUniqueID, best_sign.sign_code);
		printf("%4d %4d ", g_signID, g_sign_frameID);
		fprintf(stderr, "%4d %4d ", g_signID, g_sign_frameID);
		printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_sign.confidence);
		fprintf(stderr, "%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_sign.confidence);

	}
//	fprintf(stderr, "%05d.ppm;%d\n", g_signID, best_sign.sign_code);
	fflush (stderr);
	fflush (stdout);

	for (i = 0; i < NUM_CANDIDATES; i++)
	{
		for (try = 1; try <= N_RECALL_TRIES; try++)
		{
			j = i + (try - 1) * NUM_CANDIDATES;
			fprintf(hit_statistics_csv_file,"%05d.ppm; %d ; %f\n",g_signID,sign_count_cadidates[j].sign_code,sign_count_cadidates[j].sign_count);
		}
	}

	fflush (hit_statistics_csv_file);

	fflush (stdout);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT
SetReshapeFilterParams(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	if (pParamList->next->param.ival == 1)
	{
		reshape_filter_offset_x = (float)mae_uniform_random(-2.0f, 2.0f);
		reshape_filter_offset_y = (float)mae_uniform_random(-2.0f, 2.0f);
		reshape_filter_scale_factor = (float)mae_uniform_random(0.95f, 1.05f);
		reshape_filter_rotation_angle = (float)mae_uniform_random(-10.0f, 10.0f);
	}
	else
	{
		reshape_filter_offset_x = 0.0f;
		reshape_filter_offset_y = 0.0f;
		reshape_filter_scale_factor = 1.0f;
		reshape_filter_rotation_angle = 0.0f;
	}
	return (output);
}

NEURON_OUTPUT
SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	output.ival = set_network_status_interface(g_nStatus);

	return (output);
}


NEURON_OUTPUT
GetSignByIndex (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int line_number;

	line_number = pParamList->next->param.ival;
	get_input_by_file_line_number(line_number);

	g_nTry = N_RECALL_TRIES;
	best_sign.confidence = 0.0;
	
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT
WaitKey (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	getchar();
	return output;
}

void
copy_without_trailings(char *dst, char *src)
{
	strcpy (dst, src + 1); // remove aspas no inicio
	dst[strlen(dst)-1] = '\0'; // remove aspa no fim
}

NEURON_OUTPUT
SaveConfig (PARAM_LIST *pParamList)
{
	SYNAPSE_LIST *s_list = NULL;
	NEURON_LAYER *nl_output = NULL;
	NEURON_LAYER *nl_input = NULL;
	NEURON_OUTPUT output;
	NEURON *neuron;
	FILE *file;
	char filename[255];
	int out_height, out_width;
	int in_height, in_width;
	int synapses = 0;

	copy_without_trailings(&filename[0], pParamList->next->param.sval);

	file = fopen(filename, "w+"); //overwrite file

	output.ival = 0;
	if (file)
	{
		nl_output = &nl_landmark; //TODO: passar na lista de parâmetros
		nl_input = &nl_traffic_sign_gaussian; //TODO: passar na lista de parâmetros

		out_width = nl_output->dimentions.x;
		out_height = nl_output->dimentions.y;

		in_width = nl_input->dimentions.x;
		in_height = nl_input->dimentions.y;

		neuron = &(nl_output->neuron_vector[0]);
		for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
			synapses++;

		//NETWORK CONFIG
		fprintf(file, "O;X;N\n");
		fprintf(file, "%d;%d;%d\n", out_width*out_height, synapses, in_width*in_height);

		output.ival = fclose(file);
	}
	return output;
}

NEURON_OUTPUT
SaveDataset (PARAM_LIST *pParamList)
{
	NEURON_LAYER *nl_input = NULL;
	NEURON_OUTPUT output;
	FILE *file;
	char filename[255];
	int print_header, i;
	int in_height, in_width;

	copy_without_trailings(&filename[0], pParamList->next->param.sval);
	print_header = pParamList->next->next->param.ival;

	if (print_header == 1)
		file = fopen(filename, "w+"); //overwrite if file exists
	else
		file = fopen(filename, "a+"); //append if file exists

	output.ival = 0;
	if (file)
	{
		nl_input = &nl_traffic_sign_gaussian; //TODO: passar na lista de parâmetros

		in_width = nl_input->dimentions.x;
		in_height = nl_input->dimentions.y;

		if (print_header == 1)
		{
			for (i = 0; i < in_width*in_height; i++)
				fprintf(file, "v%d;", i);
			fprintf(file, "ci\n");
		}
		else
		{
			for (i = 0; i < in_width * in_height; i++)
			{
				if (nl_input->output_type == GREYSCALE_FLOAT)
					fprintf (file, "%f;", nl_input->neuron_vector[i].output.fval);
				else
					fprintf (file, "%d;", nl_input->neuron_vector[i].output.ival);

			}
			fprintf(file, "%d\n", g_signUniqueID);//TODO: find a general way to retrieve class label from neuron without training
		}
		output.ival = fclose(file);
	}
	return output;
}

NEURON_OUTPUT
SaveSynapses (PARAM_LIST *pParamList)
{
	SYNAPSE_LIST *s_list = NULL;
	NEURON_LAYER *nl_output = NULL;
	NEURON_OUTPUT output;
	NEURON *neuron;
	FILE *file;
	char filename[255];
	int out_height, out_width;
	int i, synapses = 0;

	copy_without_trailings(&filename[0], pParamList->next->param.sval);

	file = fopen(filename, "w+"); //append if file exists

	output.ival = 0;
	if (file)
	{
		nl_output = &nl_landmark; //TODO: passar na lista de parâmetros

		out_width = nl_output->dimentions.x;
		out_height = nl_output->dimentions.y;

		//PRINT HEADER
		fprintf(file, "O;");
		neuron = &(nl_output->neuron_vector[0]);
		for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
		{
			fprintf(file, "x%d", synapses++);
			if (s_list->next)
				fprintf(file, ";");
		}
		fprintf(file, "\n");

		//PRINT VALUES
		for (i = 0; i < out_width * out_height; i++)
		{
			neuron = &(nl_output->neuron_vector[i]);
			if (neuron)
			{
				if (neuron->synapses)
					fprintf(file, "%d;", neuron->id);
				for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
				{
					neuron = s_list->synapse->source;
					fprintf(file, "%d", neuron->id);
					if (s_list->next)
						fprintf(file, ";");
				}
				fprintf(file, "\n");
			}
		}

		output.ival = fclose(file);
	}
	return output;
}

//***********************************************************
//* Function: get_current_sign_image
//* Description:
//***********************************************************
int get_current_sign_image ()
{
	FILTER_DESC *filter;
	char curr_filename[128];
	char file_path[256];

	sprintf(curr_filename, "%05d_%05d_%05d.ppm", g_signUniqueID, g_signID, g_sign_frameID);

	if (g_nStatus == TRAINING_PHASE)
	{
		sprintf (file_path, "%s%s", TRAINING_INPUT_PATH, curr_filename);
	}
	else if (g_nStatus == RECALL_PHASE)
	{
		sprintf (file_path, "%s%s", TESTING_INPUT_PATH, curr_filename);
	}
	else
	{
		printf ("Error: Invalid net status (get_current_sign_image).\n");
		return -1;
	}

	/*read sign image*/
	ReadFrameInput (&traffic_sign, file_path);
	check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
	traffic_sign.up2date = 0;

	update_input_neurons (&traffic_sign);
	update_input_image (&traffic_sign);

	filter = get_filter_by_output (out_traffic_sign_reshape.neuron_layer);
	filter_update (filter);
	output_update (&out_traffic_sign_reshape);

	filter = get_filter_by_output (out_traffic_sign_gaussian.neuron_layer);
	filter_update (filter);
	output_update (&out_traffic_sign_gaussian);

	return 0;
}

//***********************************************************
//* Function: f_keyboard
//* Description: Called whenever a key is pressed
//***********************************************************
void f_keyboard (char *key_value)
{
	char key;
	//    FILTER_DESC *filter;

	key = key_value[0];
	switch (key)
	{
	case 't': //set training status
		set_network_status_interface(TRAINING_PHASE);
		get_first_sign_id();
		break;

	case 'r': //set recall status
		set_network_status_interface(RECALL_PHASE);
		get_first_sign_id();
		break;

	case 'N':

	case 'n': // next sign
		if ((g_sign_frameID == -1) && (g_signID == -1))
			get_first_sign_id();
		else
			get_next_sign_id(DIRECTION_FORWARD);

		get_current_sign_image();
		break;

	case 'P':

	case 'p': // previous sign
		if ((g_sign_frameID == -1) && (g_signID == -1))
			get_first_sign_id();
		else
			get_next_sign_id(DIRECTION_REWIND);

		get_current_sign_image();
		break;

	default:
		break;
	}
}



