#include "place_recognition_user_functions.h"

#include <locale.h>
 
/*
********************************************************
* Function: init_user_functions 		       *
* Description:  				       *
* Inputs: none  				       *
* Output:					       *
********************************************************
*/

int g_outputFrameID[NUMBER_OF_CLASSES];
winner_t g_outputWinner[10];

int init_user_functions ()
{
	char strCommand[128];
	char *locale_string;

	locale_string = setlocale (LC_ALL, "C");
	if (locale_string == NULL)
	{
	        fprintf (stderr, "Could not set locale.\n");
	        exit (1);
	}
	else
        	printf ("Locale set to %s.\n", locale_string);

	in_pattern.wxd = in_pattern.ww / 2;
	in_pattern.wyd = in_pattern.wh / 2;

	sprintf (strCommand, "move %s to %d, %d;", in_pattern.name, in_pattern.wxd, in_pattern.wyd);
	interpreter (strCommand);

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);

	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	update_input_neurons (&in_pattern);
	all_filters_update();
	all_outputs_update ();

	return (0);
}

int read_dataset_file()
{
	int i, number_of_frames;
	FILE* fd;
	char filename[1024];
	double timestamp;
	frame_t* g_frame_list;
	filename[0] = '\0';
	if(g_network_status == TRAINING_PHASE)
	{
		g_frame_list = g_training_frame_list;
		strcat(filename, DATA_SET_FILE_PATH);
		strcat(filename, DATA_SET_FILE_TRAIN);
		printf("%s\n", filename);
		number_of_frames = NUMBER_OF_TRAINING_FRAMES;
	}
	else
	{
		g_frame_list = g_test_frame_list;
		strcat(filename, DATA_SET_FILE_PATH);
		strcat(filename, DATA_SET_FILE_TEST);
		number_of_frames = NUMBER_OF_TEST_FRAMES;
	}

	if((fd = fopen(filename, "r")) == NULL)
		return -1;

	fscanf(fd, "%*[^\n]\n"); //skip header
	for(i = 0; i < number_of_frames; i++)
	{
		fscanf(fd, "%s %d %lf %lf %lf %lf %lf %lf %lf\n", g_frame_list[i].imagename,
				&g_frame_list[i].correspondence,
				&g_frame_list[i].pose.x, &g_frame_list[i].pose.y, &g_frame_list[i].pose.z,
				&g_frame_list[i].pose.roll, &g_frame_list[i].pose.pitch, &g_frame_list[i].pose.yaw,
				&timestamp);
		strcpy(&(g_frame_list[i].imagename[strlen(g_frame_list[i].imagename)-3]), "ppm");
	}

	fclose(fd);
	return 0;
}

int get_file_name(char *strFileName)
{
	int index;
	frame_t* g_frame_list;

	if(g_network_status == TRAINING_PHASE)
	{
		index = g_train_frame_ID;
		g_frame_list = g_training_frame_list;
	}
	else
	{
		index = g_test_frame_ID;
		g_frame_list = g_test_frame_list;
	}

	strcpy(strFileName, g_frame_list[index].imagename);

	//printf("%s\n", strFileName);

	return (0);
}

int read_frame_input(INPUT_DESC *input, char *strFileName)
{
	load_input_image (input, strFileName);

	return (0);
}

int get_frame_index(int nDirection)
{
	int number_of_frames;
	int *index;

	if(g_network_status == TRAINING_PHASE)
	{
		index = &g_train_frame_ID;
		number_of_frames = NUMBER_OF_TRAINING_FRAMES;
	}
	else
	{
		index = &g_test_frame_ID;
		number_of_frames = NUMBER_OF_TEST_FRAMES;
	}

	if (nDirection == DIRECTION_FORWARD)
	{
		(*index)++;

		if((*index) > number_of_frames)
			(*index) = number_of_frames;
	}
	else
	{
		(*index)--;

		if((*index) < 1)
			(*index) = 1;
	}
	return (0);
}

int get_frame_file(INPUT_DESC *input)
{
	char strFileName[256];

	get_file_name(strFileName);

	if (read_frame_input(input, strFileName))
		return (-1);

	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);

	input->up2date = 0;
	update_input_image (input);

	return (0);
}

void make_input_image_neural_global_localizer(INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	input->ww = w;
	input->wh = h;

	switch(TYPE_SHOW)
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

	if(input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}

void init_neural_global_localizer (INPUT_DESC *input)
{
#ifndef	NO_INTERFACE
	int x, y;
#endif
	int i;
	char strFileName[128];

	g_training_frame_list = (frame_t *) malloc (NUMBER_OF_TRAINING_FRAMES * sizeof(frame_t));
	g_test_frame_list = (frame_t *) malloc (NUMBER_OF_TEST_FRAMES * sizeof(frame_t));

	read_dataset_file();

	make_input_image_neural_global_localizer(input, IMAGE_WIDTH, IMAGE_HEIGHT);

	get_file_name(strFileName);

	for(i = 0; i < 10; i++)
	{
		g_outputWinner[i].frame = -1;
		g_outputWinner[i].confidence = 0.0;
	}

	if (strcmp(strFileName,"") != 0)
		read_frame_input(input, strFileName);

#ifndef	NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
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

void input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_neural_global_localizer (input);
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
				get_frame_index(DIRECTION_REWIND);
				get_frame_file (input);
			}
			else if (input->wxd >= IMAGE_WIDTH)
			{
				get_frame_index(DIRECTION_FORWARD);
				get_frame_file (input);
			}
			else
			{
				translation_filter_deltaX = (float) input->wxd;
				translation_filter_deltaY = (float) input->wyd;
			}

			#ifndef NO_INTERFACE
			check_input_bounds (input, input->wxd, input->wyd);
			glutSetWindow (input->win);
			#endif

			all_filters_update ();
			all_outputs_update ();

		}
	}
}

void input_controler (INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		// Move the input window
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
	}
	input->mouse_button = -1;

	return;
}

int evaluate_output(OUTPUT_DESC *output, float *confidence)
{
	int i;
	int nMax1 = 0;
	int nMax2 = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < NUMBER_OF_CLASSES; i++)
		g_outputFrameID[i] = 0;

	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < NUMBER_OF_CLASSES))
			g_outputFrameID[neuron_vector[i].output.ival] += 1;
	}

	for (i = 0; i < NUMBER_OF_CLASSES; i++)
	{
		if (g_outputFrameID[i] > nMax1)
		{
			nMax1 = g_outputFrameID[i];
			g_outputFrameID[i] = 0;
			g_train_frame_ID = i;
		}
	}

	for (i = 0; i < NUMBER_OF_CLASSES; i++)
	{
		if (g_outputFrameID[i] > nMax2)
			nMax2 = g_outputFrameID[i];
	}

	*confidence = ((float) (nMax1 - nMax2)) / ((float) nMax1);
	return (g_train_frame_ID);
}

int get_output_winner(int repeats)
{
	int i, j;
	int frame;
	double confidence_sum[repeats];
	double confidence_max = -1.0;
	int winner_index = 0;
	int ocurrence = 0;

	for(i = 0; i < repeats; i++)
	{
		confidence_sum[i] = 0;
		frame = g_outputWinner[i].frame;
		ocurrence = 0;

		for(j = 0; j < repeats; j++)
		{
			if(g_outputWinner[j].frame == frame)
			{
				ocurrence++;
				confidence_sum[i] += g_outputWinner[j].confidence;
			}
		}

		confidence_sum[i] = ocurrence * confidence_sum[i];
	}

	for(i = 0; i < repeats; i++)
	{
		if(confidence_sum[i] > confidence_max)
		{
			confidence_max = confidence_sum[i];
			winner_index = i;
		}
	}

	return g_outputWinner[winner_index].frame;

}

void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int frame = 0;
	float confidence = 0.0;
	int winner_frame = 0;
	static int frame_counter = 0;
	static int hit_counter = 0;
	char recall_filename[256];

	if (g_network_status == RECALL_PHASE)
	{
		if (strcmp (output->name, out_nl_v1_pattern.name) == 0)
		{
			frame = evaluate_output(output, &confidence);

			g_outputWinner[frame_counter].frame = frame;
			g_outputWinner[frame_counter].confidence = confidence;

			frame_counter++;

			//printf("output frame[%d]: %d (%f)\n", frame_counter, frame, confidence);

			if(g_evaluate_output == 1)
			{
				winner_frame = get_output_winner(1);

				//strcpy(recall_filename, g_test_frame_list[winner_frame].imagename);
				//load_image_to_object(output->output_handler_params->next->param.sval, recall_filename);

				g_evaluate_output = 0;
				frame_counter = 0;

				int hit = ((winner_frame == g_test_frame_list[g_test_frame_ID].correspondence));// ||
						  //(winner_frame == (g_test_frame_list[g_test_frame_ID].correspondence) + 1) ||
						  //(winner_frame == (g_test_frame_list[g_test_frame_ID].correspondence) - 1) ||
						  //(winner_frame == (g_test_frame_list[g_test_frame_ID].correspondence) + 2) ||
						  //(winner_frame == (g_test_frame_list[g_test_frame_ID].correspondence) - 2)) ? 1 : 0);

				hit_counter += hit;

				//printf("%d ", winner_frame);
				//fflush(stdout);
				printf("### winner frame: %d [%d] - %f ###\n", winner_frame, g_test_frame_list[g_test_frame_ID].correspondence , (hit_counter / ((double)NUMBER_OF_TEST_FRAMES)) * 100.0);
				//printf("tested: %6.2f %6.2f %6.2f\n", g_test_frame_list[g_test_frame_ID].pose.x, g_test_frame_list[g_test_frame_ID].pose.y, g_test_frame_list[g_test_frame_ID].pose.z);
				//printf("returned: %6.2f %6.2f %6.2f\n", g_training_frame_list[winner_frame].pose.x, g_training_frame_list[winner_frame].pose.y, g_training_frame_list[winner_frame].pose.z);

			}
		}
	}

	#ifndef NO_INTERFACE
		glutSetWindow (output->win);
		output_display (output);
	#endif
}

void draw_output (char *output_name, char *input_name)
{
	OUTPUT_DESC* output;

	output = get_output_by_name (output_name);

	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, g_training_frame_list[g_train_frame_ID].correspondence);

	output_update(output);

	return;
}

void f_keyboard (char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
		// Train network
		case 'T':
		case 't':
			g_network_status = TRAINING_PHASE;
			break;

		case 'R':
		case 'r':
			g_network_status = RECALL_PHASE;
			break;
			break;

	}

	return;
}

NEURON_OUTPUT set_network_status (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_network_status = pParamList->next->param.ival;

	switch (g_network_status)
	{
		case TRAINING_PHASE:
			break;
		case MOVING_PHASE:
			break;
		case RECALL_PHASE:
			break;

	}

	output.ival = 0;
	return (output);
}


NEURON_OUTPUT get_frame (PARAM_LIST *pParamList)
{

	NEURON_OUTPUT output;

	int index = pParamList->next->param.ival;

	switch (g_network_status)
	{
		case TRAINING_PHASE:
			g_train_frame_ID = index;
			break;
		case MOVING_PHASE:
		case RECALL_PHASE:
			g_test_frame_ID = index;
			break;
	}

	get_frame_file(&in_pattern);

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT set_network_evaluate(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_evaluate_output = 1;

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT read_dataset (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	read_dataset_file();

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT randomize (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	int *value;
	int offset;
	int axis;

	value = (int *) pParamList->next->param.pval;
	offset = pParamList->next->next->param.ival;
	axis = pParamList->next->next->next->param.ival;

	*value =  (((int)(offset*(rand()/(RAND_MAX+1.0)))) % offset);

	if(axis == 0) 	//x case
	{
		if(*value < 0)
			*value = 0;
		if(*value >= IMAGE_WIDTH)
			*value = IMAGE_WIDTH - 1;
	}
	else			//y case
	{
		if(*value < 0)
			*value = 0;
		if(*value >= IMAGE_HEIGHT)
			*value = IMAGE_HEIGHT - 1;
	}

	output.ival = 0;
	return (output);
}
