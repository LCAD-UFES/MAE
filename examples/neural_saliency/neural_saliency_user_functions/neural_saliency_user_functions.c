#include <locale.h>
#include "neural_saliency_user_functions.h"
 
/*
********************************************************
* Function: init_user_functions 		       *
* Description:  				       *
* Inputs: none  				       *
* Output:					       *
********************************************************
*/

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
	filter_update(get_filter_by_name("in_pattern_cropped_filter"));
	filter_update(get_filter_by_name("in_pattern_gaussian_filter"));
	all_outputs_update ();

	return (0);
}

int get_next_file_name(char *strFileName, char* directory, int nDirection, int number_of_frames)
{
	FILE *pFile = NULL;

	while (pFile == NULL)
	{
		sprintf (strFileName, "%s%03d.ppm", directory, g_frameID);

		printf("%s\n", strFileName);

		if ((pFile = fopen (strFileName, "r")) != NULL)
		{
			fclose (pFile);
			return 0;
		}
		else
		{
			if (nDirection == DIRECTION_FORWARD)
			{
				g_frameID++;

				if(g_frameID > number_of_frames)
					g_frameID = number_of_frames;
			}
			else
			{
				g_frameID--;

				if(g_frameID < 1)
					g_frameID = 1;
			}
		}
	}

	return (0);
}

int get_file_name(char *strFileName, char* directory, int nDirection, int frame, int number_of_frames)
{
	FILE *pFile = NULL;

	sprintf (strFileName, "%s%03d.ppm", directory, frame);

	printf("%s\n", strFileName);

	if ((pFile = fopen (strFileName, "r")) != NULL)
	{
		fclose (pFile);
		return 1;
	}
	else
		return 0;
}

int read_frame_input(INPUT_DESC *input, char *strFileName)
{
	load_input_image (input, strFileName);

	return (0);
}

int get_frame(INPUT_DESC *input, char* directory, int nDirection, int frame)
{
	char strFileName[128];

	if (g_networkStatus == TRAINING_PHASE)
		get_file_name(strFileName, directory, nDirection, frame, NUMBER_OF_TRAINING_FRAMES);
	else
		get_file_name(strFileName, directory, nDirection, frame, NUMBER_OF_TEST_FRAMES);

	if (read_frame_input(input, strFileName))
		return (-1);

	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);

	input->up2date = 0;
	update_input_image (input);

	return (0);
}

int get_next_frame(INPUT_DESC *input, char* directory, int nDirection)
{
	char strFileName[128];

	if (nDirection == DIRECTION_FORWARD)
		g_frameID++;
	else
		g_frameID--;

	if (g_networkStatus == TRAINING_PHASE)
		while (get_next_file_name(strFileName, directory, nDirection, NUMBER_OF_TRAINING_FRAMES));
	else
		while (get_next_file_name(strFileName, directory, nDirection, NUMBER_OF_TEST_FRAMES));

	if (read_frame_input(input, strFileName))
		return (-1);

	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);

	input->up2date = 0;
	update_input_image (input);

	return (0);
}

void make_input_image_neural_saliency(INPUT_DESC *input, int w, int h)
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

void init_neural_saliency (INPUT_DESC *input)
{
	int x, y;
	char strFileName[128];

	make_input_image_neural_saliency(input, IMAGE_WIDTH, IMAGE_HEIGHT);

	get_next_file_name(strFileName, INPUT_TRAINING_PATH, DIRECTION_FORWARD, NUMBER_OF_TRAINING_FRAMES);

	if (strcmp(strFileName,"") != 0)
		read_frame_input(input, strFileName);

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
}

void input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
		init_neural_saliency (input);
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				get_next_frame (input, INPUT_TRAINING_PATH, DIRECTION_REWIND);
			else if(input->wxd > IMAGE_WIDTH)
				get_next_frame (input, INPUT_TRAINING_PATH, DIRECTION_FORWARD);

			check_input_bounds (input, input->wxd, input->wxd);
			glutSetWindow (input->win);

			filter_update(get_filter_by_name("in_pattern_cropped_filter"));
			filter_update(get_filter_by_name("in_pattern_gaussian_filter"));
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

//int evaluate_output(OUTPUT_DESC *output, float *confidence)
//{
//	int i;
//	int nMax1 = 0;
//	int nMax2 = 0;
//	NEURON *neuron_vector;
//
//	neuron_vector = output->neuron_layer->neuron_vector;
//
//	for (i = 0; i < NUMBER_OF_TRAINING_FRAMES + 1; i++)
//		g_outputFrameID[i] = 0;
//
//	for (i = 0; i < (output->wh * output->ww); i++)
//	{
//		if ((neuron_vector[i].output.ival >= 1) && (neuron_vector[i].output.ival <= NUMBER_OF_TRAINING_FRAMES))
//			g_outputFrameID[neuron_vector[i].output.ival] += 1;
//	}
//
//	for (i = 1; i < NUMBER_OF_TRAINING_FRAMES + 1; i++)
//	{
//		if (g_outputFrameID[i] > nMax1)
//		{
//			nMax1 = g_outputFrameID[i];
//			g_outputFrameID[i] = 0;
//			g_frameID = i;
//		}
//	}
//
//	for (i = 1; i < NUMBER_OF_TRAINING_FRAMES + 1; i++)
//	{
//		if (g_outputFrameID[i] > nMax2)
//			nMax2 = g_outputFrameID[i];
//	}
//
//	*confidence = ((float) (nMax1 - nMax2)) / ((float) nMax1);
//	return (g_frameID);
//}

void output_handler_saliency (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{

	int i;
	NEURON *neuron_vector;
	neuron_vector = output->neuron_layer->neuron_vector;

	if(g_networkStatus == RECALL_PHASE)
	{
		for (i = 0; i < (output->wh * output->ww); i++)
		{
			neuron_vector[i].output.ival = neuron_vector[i].last_hamming_distance > 0 ? neuron_vector[i].last_hamming_distance : 0;
		}
	}

	#ifndef NO_INTERFACE
		glutSetWindow (output->win);
		output_display (output);
	#endif
}

void output_handler_saliency_max (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	OUTPUT_DESC* recall_output;
	OUTPUT_DESC* saliency_max_output;
	NEURON* saliency_max_neuron_vector;
	char recall_filename[64];
	int i, j, k;

	if(g_networkStatus == RECALL_PHASE)
	{

		sprintf(recall_filename, "%s%03d.ppm", INPUT_TRAINING_PATH, g_frameID);
		load_image_to_object(output->output_handler_params->next->param.sval, recall_filename);


		recall_output = get_output_by_name (output->output_handler_params->next->param.sval);
		saliency_max_output = get_output_by_name (output->output_handler_params->next->next->param.sval);

		saliency_max_neuron_vector = saliency_max_output->neuron_layer->neuron_vector;

		if (recall_output->rectangle_list == NULL)
			recall_output->rectangle_list = (RECTANGLE *) calloc(NUMBER_OF_SALIENCIES, sizeof(RECTANGLE));

		recall_output->rectangle_list_size = NUMBER_OF_SALIENCIES;

		for(i = 0; i < NUMBER_OF_SALIENCIES; i++)
		{
			recall_output->rectangle_list[k].x = 0;
			recall_output->rectangle_list[k].y = 0;
			recall_output->rectangle_list[k].w = 0;
			recall_output->rectangle_list[k].h = 0;
			recall_output->rectangle_list[k].r = 0;
			recall_output->rectangle_list[k].g = 0;
			recall_output->rectangle_list[k].b = 0;
		}

		k = 0;

		for(i = 0; i < saliency_max_output->wh; i++)
		{
			for (j = 0; j < saliency_max_output->ww; j++)
			{
				if (saliency_max_neuron_vector[i * saliency_max_output->ww + j].output.ival == 255)
				{
					recall_output->rectangle_list[k].x = (j * (IMAGE_WIDTH/NL_WIDTH)) - 5;
					recall_output->rectangle_list[k].y = (i * (IMAGE_HEIGHT_CROP/NL_HEIGHT)) - 5 + (IMAGE_HEIGHT - IMAGE_HEIGHT_CROP);
					recall_output->rectangle_list[k].w = 10;
					recall_output->rectangle_list[k].h = 10;
					recall_output->rectangle_list[k].r = 0.0;
					recall_output->rectangle_list[k].g = 1.0;
					recall_output->rectangle_list[k].b = 0.0;

					k++;
				}
			}
		}
	}
}

void draw_output (char *output_name, char *input_name)
{
	OUTPUT_DESC* output;

	output = get_output_by_name (output_name);

	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, 0);

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
			g_networkStatus = TRAINING_PHASE;
			draw_output("out_nl_saliency_pattern", "in_pattern");

			//all_filters_update();
			filter_update(get_filter_by_name("in_pattern_cropped_filter"));
			filter_update(get_filter_by_name("in_pattern_gaussian_filter"));
			all_outputs_update ();
			train_network();

			g_networkStatus = MOVING_PHASE;

			get_frame(&in_pattern, INPUT_TEST_PATH, DIRECTION_FORWARD, 1);
			check_input_bounds (&in_pattern, in_pattern.wxd, in_pattern.wxd);

			filter_update(get_filter_by_name("in_pattern_cropped_filter"));
			filter_update(get_filter_by_name("in_pattern_gaussian_filter"));
			all_outputs_update ();

			g_networkStatus = RECALL_PHASE;
			all_dendrites_update ();
			all_neurons_update ();
			all_outputs_update();

			filter_update(get_filter_by_name("nl_saliency_pattern_max_filter"));
			all_outputs_update();
			all_outputs_update();
			break;

		case 'R':
		case 'r':

			break;

	}

	return;
}

NEURON_OUTPUT set_network_status (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_networkStatus = pParamList->next->param.ival;

	switch (g_networkStatus)
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


NEURON_OUTPUT get_new_frame (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	int index = pParamList->next->param.ival;
	int path_type = pParamList->next->next->param.ival;

	if(index > g_frameID)
	{
		while(index > g_frameID)
			g_frameID++;
	}
	else
	{
		while(index < g_frameID)
			g_frameID--;
	}

	g_frameID--;

	if(path_type == 0)
		get_next_frame(&in_pattern, INPUT_TRAINING_PATH, DIRECTION_FORWARD);
	else
		get_next_frame(&in_pattern, INPUT_TEST_PATH, DIRECTION_FORWARD);

	output.ival = 0;
	return (output);
}
