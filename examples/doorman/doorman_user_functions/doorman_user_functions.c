#include "doorman_user_functions.h"


 
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
			
	in_pattern.wxd = in_pattern.ww / 2;
	in_pattern.wyd = in_pattern.wh / 2;

	sprintf (strCommand, "move %s to %d, %d;", in_pattern.name, in_pattern.wxd, in_pattern.wyd);
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);
	
	// Creates the album
	if ((g_pAlbum = albumCreate (ALBUM_PATH)) == NULL)
	{
		printf ("Cannot create the album.\n");
		return;
	}

	// Loads the photos
	if (albumLoad (g_pAlbum, PHOTOS_PATH) != 0)
	{
		printf ("Cannot load the album.\n");
		return;
	}
	
	// Gets the first photo
	g_pCurrentFace = albumGetNextPhoto (g_pAlbum, 1, 0, 0, -1);
	
	update_input_neurons (&in_pattern);
	all_filters_update();
	all_outputs_update ();
	
	return (0);
}



/*
********************************************************
* Function: doormanUpdateInputImage		       *
* Description: 					       *
* Inputs: 					       *
* Output: 	  				       *
********************************************************
*/

void doormanUpdateInputImage (INPUT_DESC *pInput, char *strPhotoName)
{
	char strCommand [256];
	
	// Makes the command
	sprintf (strCommand, "%s %s", GET_FACE_SCRIPT, strPhotoName);
	
	// Gets the input images
    	system (strCommand);
			
	// Updates the input image
	update_input_image (pInput);
	update_input_neurons (pInput);
	check_input_bounds (pInput, pInput->wxd, pInput->wyd);
	input_display ();
	
	// Shows the current photo
	photoShow(g_pCurrentFace);
	return;
}



/*
********************************************************
* Function: doormanInitInputWindow		       *
* Description: initialize variables, structures and    *
*	       program procedures		       *
* Inputs: input layer				       *
* Output: none  				       *
********************************************************
*/

void doormanInitInputWindow (INPUT_DESC *input)
{
	int x, y;
	char strFileName[128];
	
	make_input_image (input);

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
	
	return;
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
	int x, y;
	int i;
	OUTPUT_DESC *output;
	FILTER_DESC *filter;

	if (input->win == 0)
		doormanInitInputWindow (input);
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
			{
				// Gets the previus photo
				g_pCurrentFace = albumGetLastPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);
				
				// Loads the photo
				doormanUpdateInputImage (input, g_pCurrentFace->name);
			}
			else if (input->wxd >= IMAGE_WIDTH)
			{
				// Gets the next photo
				g_pCurrentFace = albumGetNextPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);

				// Loads the photo
				doormanUpdateInputImage (input, g_pCurrentFace->name);
			}
			
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
	int u, v, u_max, v_max, w, h, xi, yi, step;
	float current_value, max_value = FLT_MIN, log_factor;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the max value position
	for (v = 0; v < h; v++)
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
	nl_target_coordinates.neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates.neuron_vector[1].output.fval = (float) yi;
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
	int u, v, u_min, v_min, w, h, xi, yi, step;
	float current_value, min_value = FLT_MAX, log_factor;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the min value position
	for (v = 0; v < h; v++)
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
	nl_target_coordinates.neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates.neuron_vector[1].output.fval = (float) yi;
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
	int u, v, u_max, v_max, w, h, xi, yi;
	float x_mean, y_mean, weight, accumulator, log_factor, cut_point, current_value, max_value, band_width, confidence_count;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	cut_point = output->output_handler_params->next->next->param.fval;
	band_width = output->output_handler_params->next->next->next->param.fval;
	
	// Gets the neuron layer dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Initialize the mean coordinates 
	x_mean = .0f;
 	y_mean = .0f;
	
	cut_point = cut_point * (output->neuron_layer->max_neuron_output.fval - output->neuron_layer->min_neuron_output.fval) + output->neuron_layer->min_neuron_output.fval;
	
	confidence_count = confidence = accumulator = .0f;
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
	nl_target_coordinates.neuron_vector[0].output.fval = x_mean;
	nl_target_coordinates.neuron_vector[1].output.fval = y_mean;	
	
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
	int x, y;
	
	// Jump to the target
	x = (int) nl_target_coordinates.neuron_vector[0].output.fval;
	y = (int) nl_target_coordinates.neuron_vector[1].output.fval;
		
	input->wxd += x;
	input->wyd += y;

	move_input_window (input->name, input->wxd, input->wyd);
	
	return;
}



void update_all_network_neurons ()
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
	float x, y;
	int count = 0;
		
	// Saccade until reach the target
	x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;
	do
	{
		x = (x > 0.0)? x + 0.5: x - 0.5;
		y = (y > 0.0)? y + 0.5: y - 0.5;
		input->wxd += (int) x;
		input->wyd += (int) y;

		move_input_window (input->name, input->wxd, input->wyd);

		count++;
		x = nl_target_coordinates.neuron_vector[0].output.fval;
		y = nl_target_coordinates.neuron_vector[1].output.fval;

	} while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));

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
		// Memorizes the current features
		case 'M':	
		case 'm':
			copy_neuron_outputs (&nl_features, &nl_features1);
			break;
		// Report the eye, nose and mouth coordinates
		case 'R':
		case 'r':
			printf ("Eye:[%03d,%03d] - Nose:[%03d,%03d] - Mouth:[%03d,%03d]\n", g_pCurrentFace->leftEye[0], g_pCurrentFace->leftEye[1], g_pCurrentFace->nose[0], g_pCurrentFace->nose[1], g_pCurrentFace->mouth[0], g_pCurrentFace->mouth[1]);
			break;
		// Report the partial statistics
		case 'e':
			samplesNumber++;
			matchsNumber += match = sqrtf ((g_nTargetX - in_pattern.wxd) * (g_nTargetX - in_pattern.wxd) + (g_nTargetY - in_pattern.wyd) * (g_nTargetY - in_pattern.wyd)) <= 10.0 ? 1 : 0;
			rightnessVector[(samplesNumber - 1) % 13] += match;
			printf ("Sample: %d\tMatch: %s\tRightness: %.1f\%\n", samplesNumber, matchInfoVector[match], 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Report the final statistics
		case 'E':
			printf ("*************************************************\n");
			printf ("*            Final Statistics                   *\n");
			printf ("*************************************************\n");
			printf ("Face Type\t\t\tRightness\n");			
			for (i = 0; i < 13; i++)
				printf ("%2d) %s\t%.1f\%\n", (i+1), photoInfoVector[i], 100.0 * (float) (rightnessVector[i] * 13) / (float) samplesNumber);
			
			printf ("\nTotal Rightness: %.1f\%\n", 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Moves the input to the first person
		case 'P':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, 1, 0, 0, -1);
			doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the last person
		case 'L':
			g_pCurrentFace = albumGetLastPhoto (g_pAlbum, g_pCurrentFace->person, 0, 0, 0);
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, g_pCurrentFace->person, 0, 0, -1);
			doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);
			
			break;
		// Moves the input to the last photo
		case 'l':
			g_pCurrentFace = albumGetLastPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);
			doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the next person
		case 'N':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, (g_pCurrentFace->person + 1), 0, 0, 0);
			doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the next photo
		case 'n':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);
			doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade (&in_pattern);
			break;	
		// Jump to the target 
		case 'J':
		case 'j':
			jump (&in_pattern);
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
* Function: calculateRandomPosition		       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT calculateRandomPosition (PARAM_LIST *param_list)
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
* Function: loadAlbum				       *
* Description:  				       *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

NEURON_OUTPUT loadAlbum (PARAM_LIST *param_list)
{
	NEURON_OUTPUT result;
	char *strAlbumName = NULL;
	
	// Gets the parameters
	strAlbumName = param_list->next->param.sval;
	
	if (g_pAlbum != NULL)
	  albumDestroy (g_pAlbum);
	  
	// Creates the album
	if ((g_pAlbum = albumCreate (strAlbumName)) == NULL)
	{
		printf ("Cannot create the album.\n");
		result.ival = -1;
		return (result);
	}

	// Loads the photos
	if (albumLoad (g_pAlbum, PHOTOS_PATH) != 0)
	{
		printf ("Cannot load the album.\n");
		result.ival = -2;
		return (result);
	}

   	result.ival = 0;

	return (result);
}



/*
********************************************************
* Function: getFace                                    *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

NEURON_OUTPUT getFace (PARAM_LIST *param_list)
{
	NEURON_OUTPUT result;
	PHOTO *pPhoto = NULL;
	int nPerson, nPose, nExpression, nDate;
	
	// Gets the parameters
	nPerson = param_list->next->param.ival;
	nPose   = param_list->next->next->param.ival;
	
	pPhoto = albumGetNextPhoto (g_pAlbum, nPerson, nPose, 0, 0);
	
	/// Test if the person exists
	if (nPerson != pPhoto->person)
	{
		result.ival = -1;
		return (result);
	}
    
	// Test if the pose exists
	if (nPose != pPhoto->pose)
	{
		result.ival = -2;
		return (result);
	}
    
	// Sets the current photo
	g_pCurrentFace = pPhoto;
    
	// Loads the image
	doormanUpdateInputImage (&in_pattern, g_pCurrentFace->name);

    
	// Returns the photo data and expression
	result.ival = (g_pCurrentFace->date & 0x0F) << 3 | 
		      (g_pCurrentFace->expression & 0x08);

	return (result);
}



/*
********************************************************
* Function: getNoseCoordinates			       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT getNoseCoordinates (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *x, *y;
	
	// Get the Address of the Target Coordenates
	x = (int *) param_list->next->param.pval;
	y = (int *) param_list->next->next->param.pval;
	
	photoClassify (g_pCurrentFace, GROUND_TRUTHS_PATH);
	
	*x = g_nTargetX = (int) ((float) g_pCurrentFace->nose[0] * X_FACTOR);
	*y = g_nTargetY = (int) ((float) g_pCurrentFace->nose[1] * Y_FACTOR);
	
	return (output);
}
