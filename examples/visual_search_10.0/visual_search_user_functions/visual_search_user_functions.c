#include "visual_search_user_functions.h"


#define CONFIDENCE_LEVEL 0.41

 
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
			
	g_nTargetX = in_pattern.wxd = in_pattern.ww / 2;
	g_nTargetY = in_pattern.wyd = in_pattern.wh / 2;

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



/*
********************************************************
* Function: GetNextFileName 		       	       *
* Description:  				       *
* Inputs: file name, direction			       *
* Output:					       *
********************************************************
*/

void GetNextFileName(char *strFileName, int direction)
{
	FILE *file;
	char strAux[256];
	int nAttempt;

	nAttempt = 0;

	file = NULL;
	while ((file == NULL) && (nAttempt < 3))
	{
		sprintf(strAux, "%s%04d%02d.raw", INPUT_PATH, g_nPerson, g_nIdxPhoto);

		// DEBUG
		printf("FileName: %s\n", strAux);
		fflush(stdout);

		if ((file = fopen(strAux,"r")) != NULL)
		{
			strcpy(strFileName, strAux);
			fclose(file);
			return;
		}
		else
		{
			switch (nAttempt)
			{
				case 0:
				{
					if (direction == DIRECTION_FORWARD)
					{
						g_nIdxPhoto = IDXPHOTO_MIN;
						g_nPerson++;
					}
					else
					{
						g_nIdxPhoto = IDXPHOTO_MAX;
						g_nPerson--;
					}
				}
				break;

				case 1:
				{
					if (direction == DIRECTION_FORWARD)
					{
						g_nIdxPhoto = IDXPHOTO_MIN;
						g_nPerson = PERSON_MIN;
					}
					else
					{
						g_nIdxPhoto = IDXPHOTO_MAX;
						g_nPerson = PERSON_MAX;
					}
				}
				break;

				case 2:	// Nao ha mais arquivos para ler
				{
					strcpy(strFileName,"");
					return;
				}
				break;
			}
			nAttempt++;
		}
	}
}



/*
********************************************************
* Function: ReadCoordinate			       *
* Description: reads 3 ASCII bytes and transforms it   *
* 	       in one integer
* Inputs: input file				       *
* Output: integer  				       *
********************************************************
*/

int ReadCoordinate(FILE *file)
{
	char strCoord[3];

	strCoord[0] = fgetc(file);
	strCoord[1] = fgetc(file);
	strCoord[2] = fgetc(file);

	return atoi(strCoord);
}



/*
********************************************************
* Function: ReadFaceInput			       *
* Description: 		      			       *
* Inputs: input layer, file name		       *
* Output: none  				       *
********************************************************
*/

void ReadFaceInput(INPUT_DESC *input, char *strFileName)
{
	int i, nInd;
	int x, y;
	FILE *file;
	int nCol, nLin;
	int nAuxCol, nAuxLin;
	int nFactor;
	int nAux1;
	//int nAux2;

	nFactor = IMAGE_FACTOR;

	file = fopen(strFileName, "r");

	// Read Image from File
	nAux1 = (FILE_WIDTH * FILE_HEIGHT);
	for (i = 0; i < (nAux1 * 3); i++)
		g_pImageRAW[i] = fgetc(file);
	
	// Reads the eye, nose and mouth coordinates 
	g_nEyeX   = (int) (X_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	g_nEyeY   = (int) (Y_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	g_nNoseX  = (int) (X_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	g_nNoseY  = (int) (Y_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	g_nMouthX = (int) (X_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	g_nMouthY = (int) (Y_CORRECTION_FACTOR * (double) ReadCoordinate(file) + 0.5);
	
	fclose(file);
	
	// Scale Image
	nInd = 0;
	nAux1 = (FILE_WIDTH * FILE_HEIGHT) * 3;
	for (i = 0; i < nAux1; i++)
	{
		nCol = i % FILE_WIDTH;
		nLin = (i / FILE_WIDTH) % FILE_HEIGHT;

		nAuxCol = nCol % nFactor;
		nAuxLin = nLin % nFactor;

		if ((nAuxCol == 0) && (nAuxLin == 0))
			g_pImageRAWReduced[nInd++] = g_pImageRAW[i];
		
	}
	
	// Convert Image
	nAux1 = (IMAGE_WIDTH * IMAGE_HEIGHT);
	i = nAux1 * 3 - 1;
	for (y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (x = 0; x < IMAGE_WIDTH; x++)
		{
			g_pImageBMPReduced[3 * (x + y*IMAGE_WIDTH) + 0] = g_pImageRAWReduced[i - nAux1 * 2];
			g_pImageBMPReduced[3 * (x + y*IMAGE_WIDTH) + 1] = g_pImageRAWReduced[i - nAux1];
			g_pImageBMPReduced[3 * (x + y*IMAGE_WIDTH) + 2] = g_pImageRAWReduced[i];
			i--;
		}
	}
	
	// Copy Image to Input Image
	i = 0;
	for (y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (x = 0; x < IMAGE_WIDTH; x++)
		{
			input->image[3 * (x + y*input->tfw) + 0] = g_pImageBMPReduced[i++];
			input->image[3 * (x + y*input->tfw) + 1] = g_pImageBMPReduced[i++];
			input->image[3 * (x + y*input->tfw) + 2] = g_pImageBMPReduced[i++];
		}
	}
}



/*
********************************************************
* Function: GetNewFace				       *
* Description: 		      			       *
* Inputs: input layer, direction		       *
* Output: none  				       *
********************************************************
*/

void GetNewFace (INPUT_DESC *input, int direction)
{
	char strFileName[128];

	switch (direction)
	{
		case DIRECTION_REWIND:
			g_nIdxPhoto--;
			
			if (g_nIdxPhoto < IDXPHOTO_MIN)
			{
				g_nIdxPhoto = IDXPHOTO_MAX;
				g_nPerson--;
			}
			
			if (g_nPerson < PERSON_MIN)
			       	g_nPerson = PERSON_MAX;
			break;
		case DIRECTION_FORWARD:
			g_nIdxPhoto++;
	
			if (g_nIdxPhoto > IDXPHOTO_MAX)
			{
				g_nIdxPhoto = IDXPHOTO_MIN;
				g_nPerson++;
			}
			
			if (g_nPerson > PERSON_MAX)
	 			g_nPerson = PERSON_MIN;
			break;
	}
	
	GetNextFileName(strFileName, direction);
	if (strcmp(strFileName,"") != 0)
	{
		ReadFaceInput(input, strFileName);
		check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_neurons (input);
		update_input_image (input);
	}
}



/*
********************************************************
* Function: make_input_image_visual_search	       *
* Description: 		      			       *
* Inputs: input layer, width, height		       *
* Output: none  				       *
********************************************************
*/

void make_input_image_visual_search (INPUT_DESC *input, int w, int h)
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



/*
********************************************************
* Function: init_visual_search			       *
* Description: initialize variables, structures and    *
*	       program procedures		       *
* Inputs: input layer				       *
* Output: none  				       *
********************************************************
*/

void init_visual_search (INPUT_DESC *input)
{
	int x, y;
	char strFileName[128];
	
	g_nEyeX   = 0;
	g_nEyeY   = 0;
	g_nNoseX  = 0;
	g_nNoseY  = 0;
	g_nMouthX = 0;
	g_nMouthY = 0;
	
	make_input_image_visual_search (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	GetNextFileName(strFileName, DIRECTION_FORWARD);
	if (strcmp(strFileName,"") != 0)
	{
		ReadFaceInput(input, strFileName);
	}

	update_input_neurons (input);

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



/*
********************************************************
* Function: input_generator			       *
* Description: pattern generator		       *
* Inputs: input layer, status			       *
* Output: none  				       *
********************************************************
*/

void input_generator (INPUT_DESC *input, int status)
{
	//int x, y;
	//int i;
	//OUTPUT_DESC *output;
	//FILTER_DESC *filter;

	if (input->win == 0)
		init_visual_search (input);
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewFace (input, DIRECTION_REWIND);
			else if (input->wxd >= IMAGE_WIDTH)
				GetNewFace (input, DIRECTION_FORWARD);
				
			check_input_bounds (input, input->wxd, input->wxd);
			glutSetWindow (input->win);
			input_display ();
			all_filters_update ();
			all_dendrites_update (); 
			all_neurons_update ();
			all_filters_update ();
			all_outputs_update ();			
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

void output_handler_max_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_max, v_max, w, h, xi, yi; //, step;
	float current_value, max_value = FLT_MIN, log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name (nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the max value position
	for (v = 0, u_max = v_max = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (max_value < current_value)
			{
				max_value = current_value;
				u_max = u;
				v_max = v;
			}
		}	
	}
	
	// Saves the max value
	global_max_value = max_value;
	
	// Map the max value coordinates to image
	map_v1_to_image (&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u_max, v_max, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
}



/*
********************************************************
* Function: output_handler_min_value_position	       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

void output_handler_min_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_min, v_min, w, h, xi, yi; //, step;
	float current_value, min_value = FLT_MAX, log_factor;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name (nl_target_coordinates_name);
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the min value position
	for (v = 0, u_min = v_min = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (min_value > current_value)
			{
				min_value = current_value;
				u_min = u;
				v_min = v;
			}
		}	
	}
	
	// Map the max value coordinates to image
	map_v1_to_image (&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u_min, v_min, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
}


float confidence;

/*
*********************************************************************************
* Function: output_handler_mean_position					*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

void output_handler_mean_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, /* u_max, v_max,*/ w, h, xi, yi;
	float x_mean, y_mean, weight, accumulator, log_factor, cut_point, /*current_value, max_value,*/ band_width, confidence_count;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	cut_point = output->output_handler_params->next->next->next->param.fval;
	band_width = output->output_handler_params->next->next->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name (nl_target_coordinates_name);
	
	// Gets the neuron layer dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Initialize the mean coordinates 
	x_mean = 0.0;
 	y_mean = 0.0;
	
	// Finds the max value
//	max_value = FLT_MIN;	
//	for (v = 0; v < h; v++)
//	{
//		for (u = 0; u < w; u++)
//		{
//			current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
//			
//			if (max_value < current_value)
//				max_value = current_value;
//		}	
//	}
	
	// Calculate the mean coordinates
//	cut_point *= max_value;

	cut_point = cut_point * (output->neuron_layer->max_neuron_output.fval - output->neuron_layer->min_neuron_output.fval) + output->neuron_layer->min_neuron_output.fval;
//	printf ("cut_point = %f\n", cut_point);
	
	confidence_count = confidence = accumulator = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			// Gets the weight value
			weight = output->neuron_layer->neuron_vector[v * w + u].output.fval;

			if ((u >= (int) ((0.5 - 0.5 * band_width) * (float) w)) && (u < (int) ((0.5 + 0.5 * band_width) * (float) w)))
			{
				confidence += weight;
				confidence_count += 1.0;
			}
			
			if (weight < cut_point)
				continue;
				
			// Map the Mean Coordinates to Image
			map_v1_to_image (&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u, v, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

			x_mean += (double) xi * weight;
			y_mean += (double) yi * weight;
			accumulator += weight;
		}
	}
	
	// Normalize the mean coordinates 
	x_mean /= accumulator;
	y_mean /= accumulator;
	
	// Saves the mean coordinates
	nl_target_coordinates->neuron_vector[0].output.fval = x_mean;
	nl_target_coordinates->neuron_vector[1].output.fval = y_mean;	
	
	confidence = confidence / confidence_count;
	printf ("confidence = %f\n", confidence);
}



/*
********************************************************
* Function: jump				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void jump (INPUT_DESC *input)
{
	static NEURON_LAYER *nl_target_coordinates = NULL;
	int x, y;
	
	// Gets the target coordinates neuron layer
	if (nl_target_coordinates == NULL)
		nl_target_coordinates = get_neuron_layer_by_name (NL_TARGET_COORDINATES_NAME);
	
	// Jump to the target
	x = (int) nl_target_coordinates->neuron_vector[0].output.fval;
	y = (int) nl_target_coordinates->neuron_vector[1].output.fval;
		
	input->wxd += x;
	input->wyd += y;

	move_input_window (input->name, input->wxd, input->wyd);
	
	return;
}



/*
********************************************************
* Function: jump				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void jump_max (INPUT_DESC *input)
{
	static NEURON_LAYER *nl_target_coordinates = NULL;
	static OUTPUT_DESC *output = NULL;
	int x, y;
	
	// Gets the output connected to the activation map
	if (output == NULL)
		output = get_output_by_name (OUT_ACTIVATION_MAP);
		
	output_handler_max_value_position (output, 0, 0, 0);
	
	// Gets the target coordinates neuron layer
	if (nl_target_coordinates == NULL)
		nl_target_coordinates = get_neuron_layer_by_name (NL_TARGET_COORDINATES_NAME);
	
	// Jump to the target
	x = (int) nl_target_coordinates->neuron_vector[0].output.fval;
	y = (int) nl_target_coordinates->neuron_vector[1].output.fval;
		
	input->wxd += x;
	input->wyd += y;

	move_input_window (input->name, input->wxd, input->wyd);
	
	return;
}


void
update_all_network_neurons ()
{
	int i;
	
	for (i = 0; i < 15; i++)
	{
		all_dendrites_update (); 
		all_neurons_update ();
	}
	all_outputs_update ();
}



/*
********************************************************
* Function: saccade				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void saccade (INPUT_DESC *input)
{
	static NEURON_LAYER *nl_target_coordinates = NULL;
	float x, y;
	int count = 0;
	
	// Gets the target coordinates neuron layer
	if (nl_target_coordinates == NULL)
		nl_target_coordinates = get_neuron_layer_by_name (NL_TARGET_COORDINATES_NAME);
	
	// Saccade until reach the target
//	update_all_network_neurons ();
	x = nl_target_coordinates->neuron_vector[0].output.fval;
	y = nl_target_coordinates->neuron_vector[1].output.fval;
	do
	{
		x = (x > 0.0)? x + 0.5: x - 0.5;
		y = (y > 0.0)? y + 0.5: y - 0.5;
		input->wxd += (int) x;
		input->wyd += (int) y;

		move_input_window (input->name, input->wxd, input->wyd);
//		update_all_network_neurons ();

		count++;
		x = nl_target_coordinates->neuron_vector[0].output.fval;
		y = nl_target_coordinates->neuron_vector[1].output.fval;

	} while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));

	return;
}


float number_of_features = 1.0;


/*
********************************************************
* Function: memorize_features			       *
* Description: 					       *
* Inputs: 					       *
* Output: 	  				       *
********************************************************
*/

void memorize_features ()
{
	static FILTER_DESC *filter_desc = NULL;
	static OUTPUT_DESC *output = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_input  = NULL;
	NEURON_LAYER *nl_output = NULL;
	int /*index,*/ u, v, x, w, h, wo, ho;
	int num_params;
	int feature, features;
	float band_width;
	
	if (filter_desc == NULL)
	{
		filter_desc = get_filter_by_output (&nl_features);
		output = get_output_by_neural_layer (filter_desc->output);
	}
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 2)
	{
		Erro ("Error: Wrong number of parameters. The features_gather_filter must have 1 parameter.", "", "");
		return;
	}

	// Gets Parameters
	band_width = filter_desc->filter_params->next->param.fval;

	if ((band_width < 0.0) && (band_width > 1.0))
	{
		Erro ("Error: The band width parameter must be in the interval [0.0, 1.0].", "", "");
		return;
	}
	
	// Gets the Output Neuron Layer
	nl_output = filter_desc->output;
	
	// Gets Output Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
			
	// Gets the Input Neuron Layer Dimentions
	w = filter_desc->neuron_layer_list->neuron_layer->dimentions.x;
	h = filter_desc->neuron_layer_list->neuron_layer->dimentions.y;
		
	// Checks Neuron Layers
	for (features = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, features++)
		;

	if (wo != features * (int) (band_width * (float) w))
	{
		Erro ("Error: The output width must be equal to the number of input neural layers * (int) (band_width * (float) input width).", "", "");
		return;
	}

	for (n_list = filter_desc->neuron_layer_list, feature = 0; feature < features; n_list = n_list->next, feature++)
	{
		// Gets the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;
		
		for (v = 0; v < h; v++)
		{
			for (x = 0, u = (int) ((0.5 - 0.5 * band_width) * (float) w); u < (int) ((0.5 + 0.5 * band_width) * (float) w); x++, u++)
				if (number_of_features == 0.0)
					nl_output->neuron_vector[x + feature * (int) (band_width * (float) w) + v * wo].output = nl_input->neuron_vector[u + v * w].output;
				else
					nl_output->neuron_vector[x + feature * (int) (band_width * (float) w) + v * wo].output.fval += nl_input->neuron_vector[u + v * w].output.fval;
		}
	}

	number_of_features += 1.0;
	glutSetWindow (output->win);
	output_display (output);
}



/*
********************************************************
* Function: input_controler			       *
* Description: input events handler		       *
* Inputs: input layer, input status		       *
* Output: none  				       *
********************************************************
*/

void input_controler (INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		// Memorize the target features
		memorize_features ();
	}
	
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



/*
********************************************************
* Function: draw_output  			       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void draw_output (char *output_name, char *input_name)
{		
	return;
}




void
change_sample (int order)
{
	char st_order[256];
	
	switch (order)
	{
		case 1:
			copy_neuron_outputs (&nl_features, &nl_features1);
			break;
		case 2:
//			copy_neuron_outputs (&nl_features, &nl_features2);
//			break;
//		case 3:
//			copy_neuron_outputs (&nl_features, &nl_features3);
//			break;
		default:
			sprintf (st_order, "%d", order);
			Erro ("order not recognized in change_sample = ", st_order, "");
	}
	all_filters_update ();
	all_outputs_update ();
}



int
add_sample_to_memory (int samples_memorized)
{
	char st_samples_memorized[256];
	
	switch (samples_memorized)
	{
		case 1:
			copy_neuron_outputs (&nl_features1, &nl_features);
			move_input_window (in_pattern.name, g_nNoseX, g_nNoseY);
			number_of_features = 0.0;
			memorize_features ();
//			copy_neuron_outputs (&nl_features2, &nl_features);
			break;
		case 2:
			move_input_window (in_pattern.name, g_nNoseX, g_nNoseY);
			number_of_features = 0.0;
			memorize_features ();
//			copy_neuron_outputs (&nl_features3, &nl_features);
			break;
		default:
			sprintf (st_samples_memorized, "%d", samples_memorized);
			Erro ("samples_memorized not recognized in add_sample_to_memory = ", st_samples_memorized, "");
	}
	printf ("New sample memorized.\n");
	return (samples_memorized + 1);
}



int
check_samples_memorized (int samples_memorized)
{
	int currente_sample, best_sample = 1;
	float best_confidence;

	if (samples_memorized > 1)
	{
		best_confidence = confidence;
		best_sample = 1;
		currente_sample = 2;
		do
		{
			change_sample (currente_sample);
			move_input_window (in_pattern.name, 100, 100);
			saccade (&in_pattern);
			if (confidence > best_confidence)
			{
				best_confidence = confidence;
				best_sample = currente_sample;
			}
			currente_sample++;

		} while ((currente_sample <= samples_memorized) && (confidence < CONFIDENCE_LEVEL));

		if (confidence < CONFIDENCE_LEVEL)
		{
			if (samples_memorized < 3)
			{
				samples_memorized = add_sample_to_memory (samples_memorized);
				saccade (&in_pattern);
			}
			else
			{
				printf ("Samples store full.\n");
				if (confidence < best_confidence)
				{
					change_sample (best_sample);
					move_input_window (in_pattern.name, 100, 100);
					saccade (&in_pattern);
				}
			}
		}
	}
	else
	{
		samples_memorized = add_sample_to_memory (samples_memorized);
		saccade (&in_pattern);
	}

	change_sample (1);
	return (samples_memorized);
}


extern float global_factor;
extern float teste;

/*
********************************************************
* Function: f_keyboard  			       *
* Description: keyboard events handler  	       *
* Inputs: key_value (pointer to pressed character)     *
* Output: none  				       *
********************************************************
*/

void f_keyboard (char *key_value)
{
	char key;
	int i, match;
	static int samplesNumber = 0;
	static int matchsNumber  = 0;
	static int rightnessVector[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	char *matchInfoVector[2] = {"No", "Yes"};
	char *photoInfoVector[13] = {"Normal\t\t", "Smiling\t\t", "Serious\t\t", "Open Mouth\t\t", "Left Light\t\t",
				"Right Light\t\t", "Both Sides Light\t", "Sunglasses\t\t", "Left Light an Sunglasses",
				"Right Light and Sunglasses", "Cap\t\t\t", "Left Light and Cap\t", "Right Light and Cap\t"};

	static int samples_memorized = 0;
		
	switch (key = key_value[0])
	{
		// Train network
		case 'T':	
		case 't':
			train_network ();
			run_network (1);
			break;
		// Memorizes the current features
		case 'M':	
		case 'm':
			copy_neuron_outputs (&nl_features, &nl_features1);
//			memorize_features ();
			number_of_features = 1.0;
			samples_memorized = 1;
			break;
		// Report the eye, nose and mouth coordinates
		case 'R':
		case 'r':
			printf ("Eye:[%03d,%03d] - Nose:[%03d,%03d] - Mouth:[%03d,%03d]\n", g_nEyeX, g_nEyeY, g_nNoseX, g_nNoseY, g_nMouthX, g_nMouthY);
			break;
		// Report the partial statistics
		case 'e':
			samplesNumber++;
			matchsNumber += match = sqrtf ((g_nNoseX - in_pattern.wxd) * (g_nNoseX - in_pattern.wxd) + (g_nNoseY - in_pattern.wyd) * (g_nNoseY - in_pattern.wyd)) <= 10.0 ? 1 : 0;
			rightnessVector[(samplesNumber - 1) % IDXPHOTO_MAX] += match;
			printf ("Sample: %d\tMatch: %s\tRightness: %.1f %%\n", samplesNumber, matchInfoVector[match], 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Report the final statistics
		case 'E':
			printf ("*************************************************\n");
			printf ("*            Final Statistics                   *\n");
			printf ("*************************************************\n");
			printf ("Face Type\t\t\tRightness\n");			
			for (i = 0; i < IDXPHOTO_MAX; i++)
				printf ("%2d) %s\t%.1f %%\n", (i+1), photoInfoVector[i], 100.0 * (float) (rightnessVector[i] * IDXPHOTO_MAX) / (float) samplesNumber);
			
			printf ("\nTotal Rightness: %.1f\n", 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Moves the input to the first person
		case 'P':
			g_nIdxPhoto = IDXPHOTO_MIN + 1;
			g_nPerson = PERSON_MIN;
			GetNewFace (&in_pattern, DIRECTION_REWIND);
			break;
		// Moves the input to the last person
		case 'L':
			g_nIdxPhoto = IDXPHOTO_MIN + 1;
			g_nPerson--;
			GetNewFace (&in_pattern, DIRECTION_REWIND);
			break;
		// Moves the input to the last photo
		case 'l':
			GetNewFace (&in_pattern, DIRECTION_REWIND);
			break;
		// Moves the input to the next person
		case 'N':
			g_nIdxPhoto = IDXPHOTO_MIN - 1;
			g_nPerson++;
			GetNewFace (&in_pattern, DIRECTION_FORWARD);
			break;
		// Moves the input to the next photo
		case 'n':
			GetNewFace (&in_pattern, DIRECTION_FORWARD);
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade (&in_pattern);
			//if (confidence < CONFIDENCE_LEVEL)
			//{
			//	samples_memorized = check_samples_memorized (samples_memorized);
			//}
			break;	
		// Jump to the target 
		case 'J':
			jump_max (&in_pattern);
			break;	
		case 'j':
			jump (&in_pattern);
			break;	
		case 'f':
			global_factor += 0.1;
			printf ("global_factor = %f\n", global_factor);
			break;	
		case 'F':
			global_factor -= 0.1;
			printf ("global_factor = %f\n", global_factor);
			break;	
		case 'z':
			number_of_features = 0.0;
			break;	
		case 'Z':
			samples_memorized = add_sample_to_memory (samples_memorized);
			change_sample (1);
			break;	
		case 'v':
			teste -= 10.0;
			printf ("teste = %f\n", teste);
			break;	
		case 'V':
			teste += 10.0;
			printf ("teste = %f\n", teste);
			break;	
	}
	
	return;
}



/*
********************************************************
* Function: v1_to_image_mapping			       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT v1_to_image_mapping (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *xi, *yi;
	int wi, hi;
	int u, v;
	int w, h;
	float log_factor;
	
	// Gets the Address of the Image Coordenates
	xi = (int *) param_list->next->param.pval;
	yi = (int *) param_list->next->next->param.pval;
	
	// Gets the Log-Factor
	log_factor = param_list->next->next->next->param.fval;
	
	// Gets the Image Dimentions
	wi = param_list->next->next->next->param.ival;
	hi = param_list->next->next->next->next->param.ival;
	
	// Gets the V1 Coordenates
	u = param_list->next->next->next->next->next->param.ival;
	v = param_list->next->next->next->next->next->next->param.ival;
	
	// Gets the Image Dimentions
	w = param_list->next->next->next->next->next->next->next->param.ival;
	h = param_list->next->next->next->next->next->next->next->next->param.ival;
	
	map_v1_to_image (xi, yi, wi, hi, u, v, w, h, g_nTargetX, g_nTargetY, (double) h / (double) (h - 1), log_factor);
	return (output);
}
	
	

/*
********************************************************
* Function: calculate_random_position		       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT calculate_random_position (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *xi, *yi;
	int wi, hi;
	
	// Get the Address of the Image Coordenates
	xi = (int *) param_list->next->param.pval;
	yi = (int *) param_list->next->next->param.pval;
	
	// Get the Image Dimentions
	wi = param_list->next->next->next->param.ival;
	hi = param_list->next->next->next->next->param.ival;
	
	*xi = random () / wi;
	*yi = random () / hi;
	
	return (output);
}



/*
********************************************************
* Function: get_target_coordinates		       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT get_target_coordinates (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *x, *y;
	
	// Get the Address of the Target Coordenates
	x = (int *) param_list->next->param.pval;
	y = (int *) param_list->next->next->param.pval;
	
	*x = g_nNoseX;
	*y = g_nNoseY;
	
	return (output);
}


/*
********************************************************
* Function: get_photo				       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT get_photo (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;	
	
	g_nPerson = param_list->next->param.ival;
	g_nIdxPhoto = param_list->next->next->param.ival - 1;
	
	GetNewFace (&in_pattern, DIRECTION_FORWARD);
	
	return (output);
}


/*
********************************************************
* Function: get_photo				       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT set_sample (PARAM_LIST *param_list)
{
	NEURON_OUTPUT sample;
	int i, w;
	
	sample.ival = param_list->next->param.ival;
	w = nl_memoryzed_sample.dimentions.x;

	for (i = 0; i < w; i++)
		nl_memoryzed_sample.neuron_vector[i].output.fval = 0.0;

	nl_memoryzed_sample.neuron_vector[sample.ival].output.fval = 1.0;

	return (sample);
}
