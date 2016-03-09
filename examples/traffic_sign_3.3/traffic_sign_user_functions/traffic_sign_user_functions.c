#include "traffic_sign_user_functions.h"
#include "traffic_sign_utils.h"

#define N_DEFINED_SIGNS 43

//############### Global Variables ###############
int g_signUniqueID = -1; //id da classe da placa
int g_signID = -1; //id da placa
int g_sign_frameID = -1; //frame de uma placa (uma placa aparece varias vezes)

int g_size_box;
int g_img_w, g_img_h, g_roi_x1, g_roi_y1, g_roi_x2, g_roi_y2;

int g_fptr;

int g_nStatus;
int g_nTries = 3;
int g_nTry = 1;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;

int g_nTotalImagesDatasetTest = 0;
int g_nTotalImagesDatasetTrain = 0;
int g_nCurrentImageDatasetTest = 0;
int g_nCurrentImageDatasetTrain = 0;

char g_strSignsFileName[256];
FILE *g_signs_data_file = NULL;

float	hit_percent;

//############### User Functions ###############
//***********************************************************
//* Function: ReadFrameInput
//* Description:
//***********************************************************
//PNM P6
int 
ReadFrameInput (INPUT_DESC *input, char *strFileName)
{
	FILE *image_file = (FILE *) NULL;
	int i,j;
	int r,g,b;
	char character = 0;

	if ((image_file = fopen (strFileName, "rb")) == (FILE *) NULL)
	{
		Erro ("Cannot open input file (ReadFrameInput): ", strFileName, "");
		return -1;
	}

	/* Discard magic number in the begining of the image file. */
	while ((character = fgetc (image_file)) != '\n')
		;

	/* Discard comments */
	if ((character = fgetc (image_file)) == '#')
	{
		while (character != '\n')
			character = fgetc (image_file);
	}
	else
		ungetc (character, image_file);

	/* Discard image dimensions. */
	while ((character = fgetc (image_file)) != '\n')
		;

	/* Discard max color value. */
	while ((character = fgetc (image_file)) != '\n')
		;

	/* Clear image */
	for (i = 0; i <= input->tfw * input->tfh * 3; i++)
	{
		input->image[i] = 0;
	}

	///////////////////////////////////////

	/* Fit ROI to a square */
	if (g_roi_x1 > g_roi_y1)
		g_roi_y1 = g_roi_x1;
	else
		g_roi_x1 = g_roi_y1;

	if (g_roi_x2 > g_roi_y2)
		g_roi_y2 = g_roi_x2;
	else
		g_roi_x2 = g_roi_y2;

	/* Get ROI box size. */
	if ((g_roi_x2 - g_roi_x1) > (g_roi_y2 - g_roi_y1))
		g_size_box = g_roi_y2 - g_roi_y1;
	else
		g_size_box = g_roi_x2 - g_roi_x1;

	if (g_roi_x1 + g_size_box > g_img_w)
		g_size_box = g_img_w - g_roi_x1;
	if (g_roi_y1 + g_size_box > g_img_h)
		g_size_box = g_img_h - g_roi_y1;

	if ((g_size_box % 2) != 0)
		g_size_box = g_size_box - 1;

	fseek(image_file, g_roi_y1 * g_img_w * 3 + g_roi_x1 * 3, SEEK_CUR);

	for (j = input->vph/2 + g_size_box/2; j > input->vph/2 - g_size_box/2; j--)
	{
		for (i = input->vpw/2 - g_size_box/2; i < input->vpw/2 + g_size_box/2 ; i++)
		{
			r = (int) fgetc (image_file);
			g = (int) fgetc (image_file);
			b = (int) fgetc (image_file);

			input->image[3 * (j * input->tfw + i) + 0] = (GLubyte) r;
			input->image[3 * (j * input->tfw + i) + 1] = (GLubyte) g;
			input->image[3 * (j * input->tfw + i) + 2] = (GLubyte) b;
		}
		fseek(image_file, ((g_img_w - g_roi_x1 - g_size_box) * 3 + g_roi_x1 * 3), SEEK_CUR);
	}
	update_input_neurons (input);
	fclose(image_file);
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

	g_nStatus = RECALL_PHASE;
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
init_nl_landmark_eval_mask()
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
			neuron_vector[y * w + x].output.fval = //normal_dist((float)y, (float)(h / 2.0), VOTE_VARIANCE) *
							       normal_dist((float)x, (float)(w / 2.0), VOTE_VARIANCE);
		}
	}
	update_output_image (get_output_by_name("nl_landmark_eval_mask_out"));
}


void
init_nl_landmark_eval_mask_old()
{
	int w, h, x, y;
	double radius, r, xc, yc;
	NEURON *neuron_vector;
	
	neuron_vector = nl_landmark_eval_mask.neuron_vector;
	w = nl_landmark_eval_mask.dimentions.x;
	h = nl_landmark_eval_mask.dimentions.y;
	radius = (double) w / 2.0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			xc = (double) x - (double) w / 2.0 + 0.5;
			yc = (double) y - (double) h / 2.0 + 0.5;
			r = sqrt(xc * xc + yc * yc);
			if (r < radius)
				neuron_vector[y * w + x].output.fval = 1.0;
			else
				neuron_vector[y * w + x].output.fval = 0.0;
				
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

	init_nl_landmark_eval_mask();

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

//***********************************************************
//* Function: EvaluateOutput
//* Description: evaluates the output value
//* Inputs: output
//* Output: person ID
//***********************************************************
int EvaluateOutput_alberto(OUTPUT_DESC *output, float *confidence)
{
	int i;
	int sign_id = 0;
	int max = 0;
	//int sign_count[g_number_of_signs];
	int sign_count[N_DEFINED_SIGNS];
	NEURON *neuron_vector;
	NEURON *neuron_vector_mask;
	
	neuron_vector_mask = nl_landmark_eval_mask.neuron_vector;
	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		sign_count[i] = 0;
	}

	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && 
		    (neuron_vector[i].output.ival < N_DEFINED_SIGNS) &&
		    (neuron_vector_mask[i].output.fval == 1.0))
		{
			sign_count[neuron_vector[i].output.ival] += 1;
		}
	}

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		if (sign_count[i] > max)
		{
			max = sign_count[i];
			sign_id = i;
		}
	}

	*confidence = (float) max / (float) (output->wh * output->ww);
	return sign_id;
}

//***********************************************************
//* Function: EvaluateOutput
//* Description: evaluates the output value
//* Inputs: output
//* Output: person ID
//***********************************************************
int EvaluateOutput(OUTPUT_DESC *output, float *confidence)
{
	int i;
	int sign_id = 0;
	int max = 0;
	//int sign_count[g_number_of_signs];
	int sign_count[N_DEFINED_SIGNS];
	NEURON *neuron_vector;
	//number of signs = N_DEFINED_SIGNS

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		sign_count[i] = 0;
	}

	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < N_DEFINED_SIGNS))
		{
			sign_count[neuron_vector[i].output.ival] += 1;
		}
	}

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		if (sign_count[i] > max)
		{
			max = sign_count[i];
			sign_id = i;
		}
	}

	*confidence = (float) max / (float) (output->wh * output->ww);
	return sign_id;
}

int EvaluateOutput2(OUTPUT_DESC *output, float *confidence)
{
	int i,j;
	int sign_id = 0;
	float max =0;
	//int sign_count[g_number_of_signs];
	float sign_count[N_DEFINED_SIGNS];
	NEURON *neuron_vector;
	//number of signs = N_DEFINED_SIGNS

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		sign_count[i] = 0;
	}

	for (i = 0; i < output->wh; i++)
	{
		for (j = 0; j < output->ww; j++)
		{
			//float weight = (float)(SYNAPSES - neuron_vector[i*output->wh + j].last_hamming_distance)/(float)SYNAPSES;
			if ((neuron_vector[i*output->wh + j].output.ival >= 0) && (neuron_vector[i*output->wh + j].output.ival < N_DEFINED_SIGNS))
			{
				sign_count[neuron_vector[i*output->wh + j].output.ival] += 1.0 *
											   //normal_dist((float)i, (float)(output->wh/2.0), VOTE_VARIANCE) *
											   normal_dist((float)j, (float)(output->wh/2.0), VOTE_VARIANCE);
			}
		}
	}

	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		if (sign_count[i] > max)
		{
			max = sign_count[i];
			sign_id = i;
		}
	}

	*confidence = (float) max / (float) (output->wh * output->ww);
	return sign_id;
}


//***********************************************************
//* Function: output_handler
//* Description:
//***********************************************************
void output_handler_alberto(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{

	int sign_output;
	float confidence;//, hit_percent;

	sign_output = EvaluateOutput_alberto(output, &confidence);
	if (g_nStatus == RECALL_PHASE)
	{
		g_nTotalTested++;

		if (sign_output == g_signUniqueID)
		{
			g_nCorrect++;
			hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);

			printf("HIT  %4d %4d ",g_signUniqueID, sign_output);
			printf("%4d %4d ", g_signID, g_sign_frameID);
			printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, confidence);

		}
		else
		{
			hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
			printf("MISS %4d %4d ",g_signUniqueID, sign_output);
			printf("%4d %4d ", g_signID, g_sign_frameID);
			printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, confidence);

		}
		fprintf(stderr, "%05d.ppm;%d\n", g_signID, sign_output);
		fflush (stderr);
		fflush (stdout);
	}
	else
	{
		printf("     %4d %4d ",g_signUniqueID, sign_output);
		printf("%4d %4d\n", g_signID, g_sign_frameID);
	}

#ifndef NO_INTERFACE    
	glutSetWindow (output->win);
	output_display (output);
#endif
}


//***********************************************************
//* Function: output_handler
//* Description:
//***********************************************************
void output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{

	int sign;
	float confidence, hit_percent;
	static float best_confidence = -1.0;
	static int sign_output;

	if (g_nStatus == RECALL_PHASE)
	{
		//avalia a saida
		if (strcmp (output->name, out_landmark.name) == 0);
		//if (strcmp (output->name, out_traffic_sign.name) == 0)
		{
			if (g_nTry != 0)
			{
				sign = EvaluateOutput2(output, &confidence);

				if (confidence > best_confidence)
				{
					best_confidence = confidence;
					sign_output = sign;
				}
				g_nTry--;
			}
			if (g_nTry == 0)
			{
				g_nTotalTested++;

				if (sign_output == g_signUniqueID)
				{
					g_nCorrect++;
					hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);

					printf("HIT  %4d %4d ",g_signUniqueID, sign_output);
					printf("%4d %4d ", g_signID, g_sign_frameID);
					printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_confidence);

				}
				else
				{
					hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
					printf("MISS %4d %4d ",g_signUniqueID, sign_output);
					printf("%4d %4d ", g_signID, g_sign_frameID);
					printf("%4d %4d %3.2f %2.5f\n", g_nTotalTested, g_nCorrect, hit_percent, best_confidence);

				}
				fprintf(stderr, "%05d.ppm;%d\n", g_signID, sign_output);
				fflush (stderr);
				fflush (stdout);
				g_nTry = g_nTries;
				best_confidence = -1.0;
			}
		}
	}
	else
	{
		printf("     %4d %4d ",g_signUniqueID, sign_output);
		printf("%4d %4d\n", g_signID, g_sign_frameID);
	}

#ifndef NO_INTERFACE    
	glutSetWindow (output->win);
	output_display (output);
#endif
}

NEURON_OUTPUT
SetReshapeFilterParams(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	if (pParamList->next->param.ival == 1)
	{
		reshape_filter_offset_x = (float)mae_uniform_random(-4.0, 4.0);
		reshape_filter_offset_y = (float)mae_uniform_random(-4.0, 4.0);
		reshape_filter_scale_factor = (float)mae_uniform_random(1.0, 1.20);
		reshape_filter_rotation_angle = (float)mae_uniform_random(-10.0, 10.0);
	}
	else
	{
		reshape_filter_offset_x = 0.0f;
		reshape_filter_offset_y = 0.0f;
		reshape_filter_scale_factor = 1.10f;
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

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT
PrintResults (PARAM_LIST *pParamList)
{
        NEURON_OUTPUT output;
        printf("%dx%d;%d;%f;%f;%f;%f\n",OUT_WIDTH,OUT_HEIGHT,SYNAPSES,GAUSSIAN_RADIUS_DISTRIBUTION,LOG_FACTOR,VOTE_VARIANCE,hit_percent);
        return output;
}

NEURON_OUTPUT
WaitKey (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	getchar();
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



