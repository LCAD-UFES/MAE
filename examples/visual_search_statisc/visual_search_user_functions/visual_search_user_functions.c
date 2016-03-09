#include "visual_search_user_functions.h"



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
	UpdateInputImage (&in_pattern, g_pCurrentFace->name);
		
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

void make_input_image_visual_search (INPUT_DESC *input)
{
	char message[256];
	int w, h;
	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;

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
			input->vph = input->ww;
			input->vpw = input->wh;
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
	int nImageSource;

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	switch (nImageSource)
	{
		case GET_IMAGE_FROM_PNM:
                        make_input_image (input);
                        break;
		case GET_IMAGE_FROM_SMV:
                        make_input_image_visual_search (input);
                        break;
		default:
			Erro ("Invalid image source.", "", "");
	}
	make_input_image (input);

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
	int i;
        
        if (input->win == 0)
		init_visual_search (input);
	else
	{	
		update_input_neurons (input);
                check_input_bounds (input, input->wxd, input->wxd);
		glutSetWindow (input->win);
		input_display ();
				
		for (PYRAMID_LEVEL = 0; PYRAMID_LEVEL <= in_pattern.pyramid_height; PYRAMID_LEVEL++)
		{
                        filter_update (get_filter_by_output (&nl_gabor_h_00));
                        filter_update (get_filter_by_output (&nl_gabor_h_90));
                        filter_update (get_filter_by_output (&nl_gabor_v_00));
                        filter_update (get_filter_by_output (&nl_gabor_v_90));
                        filter_update (get_filter_by_output (&nl_gabor_l_00));
                        filter_update (get_filter_by_output (&nl_gabor_l_90));
                        filter_update (get_filter_by_output (&nl_gabor_r_00));
                        filter_update (get_filter_by_output (&nl_gabor_r_90));
                        filter_update (get_filter_by_output (&nl_features));
                }
                PYRAMID_LEVEL = 0;
                
                filter_update (get_filter_by_output (&nl_v1_activation_map));
                filter_update (get_filter_by_output (&nl_v1_pattern));
                
		all_dendrites_update (); 
		all_neurons_update ();
		all_outputs_update ();
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
	int u, v, u_max = 0, v_max = 0, w, h, xi, yi;
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
	int u, v, u_min = 0, v_min = 0, w, h, xi, yi;
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
	int u, v, w, h, xi, yi;
	float x_mean, y_mean, weight, accumulator, log_factor, cut_point;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	cut_point = output->output_handler_params->next->next->param.fval;
	
	// Gets the neuron layer dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Initialize the mean coordinates 
	x_mean = .0f;
 	y_mean = .0f;

	cut_point = cut_point * (output->neuron_layer->max_neuron_output.fval - output->neuron_layer->min_neuron_output.fval) + output->neuron_layer->min_neuron_output.fval;
	
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			// Gets the weight value
			weight = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (weight < cut_point)
			        continue;

			// Map the Mean Coordinates to Image
			map_v1_to_image (&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u, v, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

			x_mean += (float) xi * weight;
			y_mean += (float) yi * weight;
			accumulator += weight;
		}
	}
	
	// Normalize the mean coordinates 
	x_mean /= accumulator;
	y_mean /= accumulator;
	
	// Saves the mean coordinates
	nl_target_coordinates.neuron_vector[0].output.fval = x_mean;
	nl_target_coordinates.neuron_vector[1].output.fval = y_mean;
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
	float x, y;

        x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;

	input->wxd += (x > .0f)? (int) (x + .5f) : (int) (x - .5f);
	input->wyd += (y > .0f)? (int) (y + .5f) : (int) (y - .5f);

	move_input_window (input->name, input->wxd, input->wyd);

	return;
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
	do
	{
		x = nl_target_coordinates.neuron_vector[0].output.fval;
	        y = nl_target_coordinates.neuron_vector[1].output.fval;
	        
	        input->wxd += (x > .0f)? (int) (x + .5f) : (int) (x - .5f);
	        input->wyd += (y > .0f)? (int) (y + .5f) : (int) (y - .5f);
	        
	        move_input_window (input->name, input->wxd, input->wyd);
	} while (((fabs (x) > .5f) || (fabs (y) > .5f)) && (count++ < 7));

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
	// Move the input window	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{      
		move_input_window (input->name, input->wxd, input->wyd);
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
* Function: accumulate_neuron_outputs  		       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void accumulate_neuron_outputs (NEURON_LAYER *neuron_layer, NEURON_LAYER *accumulator_layer)
{		
        int i;
        
        for (i = 0; i < accumulator_layer->num_neurons; i++)
                accumulator_layer->neuron_vector[i].output.fval += neuron_layer->neuron_vector[i].output.fval;
                
        return;
}



/*
********************************************************
* Function: accumulate_squared_neuron_outputs  	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void accumulate_squared_neuron_outputs (NEURON_LAYER *neuron_layer, NEURON_LAYER *accumulator_layer)
{		
        int i;
        
        for (i = 0; i < accumulator_layer->num_neurons; i++)
                accumulator_layer->neuron_vector[i].output.fval += neuron_layer->neuron_vector[i].output.fval * neuron_layer->neuron_vector[i].output.fval;
        
        return;
}



/*
********************************************************
* Function: scale_neuron_outputs  	               *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void scale_neuron_outputs (NEURON_LAYER *neuron_layer, float scale_factor)
{		
        int i;
        
        for (i = 0; i < neuron_layer->num_neurons; i++)
                neuron_layer->neuron_vector[i].output.fval *= scale_factor;
                
        return;
}



/*
********************************************************
* Function: clear_neuron_outputs  		       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void clear_neuron_outputs (NEURON_LAYER *neuron_layer)
{		
        int i;
        
        for (i = 0; i < neuron_layer->num_neurons; i++)
                neuron_layer->neuron_vector[i].output.ival = 0;
                
        return;
}



/*
********************************************************
* Function: clear_neuron_outputs  		       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void copy_neuron_outputs2 (NEURON_LAYER *neuron_layer_orig, NEURON_LAYER *neuron_layer_dest)
{		
        int i;
        
        for (i = 0; i < neuron_layer_dest->num_neurons; i++)
                neuron_layer_dest->neuron_vector[i].output = neuron_layer_orig->neuron_vector[i].output;
                
        return;
}



/*
********************************************************
* Function: memorize_sample_features  		       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void memorize_sample_features (void)
{	
        samples_memorized++;
	accumulate_neuron_outputs (&nl_features, &nl_sum);
	accumulate_squared_neuron_outputs (&nl_features, &nl_squared_sum);
	copy_neuron_outputs2 (&nl_sum, &nl_average);
	scale_neuron_outputs (&nl_average, 1.0f / (float) samples_memorized);
	clear_neuron_outputs (&nl_variance);
	accumulate_squared_neuron_outputs (&nl_average, &nl_variance);
	scale_neuron_outputs (&nl_variance, - (float) samples_memorized);
	accumulate_neuron_outputs (&nl_squared_sum, &nl_variance);
	if (samples_memorized > 1)
	       scale_neuron_outputs (&nl_variance, 1.0f / (float) (samples_memorized - 1));                
        return;
}



/*
********************************************************
* Function: clear_target_representation 	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void clear_target_representation (void)
{		
        samples_memorized = 0;
        clear_neuron_outputs (&nl_sum);
	clear_neuron_outputs (&nl_squared_sum);
	clear_neuron_outputs (&nl_average);
        clear_neuron_outputs (&nl_variance);
                
        return;
}



/*
********************************************************
* Function: UpdateInputImage		       	       *
* Description: 					       *
* Inputs: 					       *
* Output: 	  				       *
********************************************************
*/

void UpdateInputImage (INPUT_DESC *pInput, char *strPhotoName)
{
	char strCommand [256];
	
	// Makes the command
	sprintf (strCommand, "%s %s", GET_FACE_SCRIPT, strPhotoName);
	
	// Gets the input images
    	system (strCommand);
			
	// Updates the input image
	update_input_image (pInput);
	check_input_bounds (pInput, pInput->wxd, pInput->wyd);
	
	// Shows the current photo
	photoShow(g_pCurrentFace);
	
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
	char command_line[256];
	//static int count = 0;
	//int i;
	
	switch (key = key_value[0])
	{
		// Memorizes the sample features
		case 'M':	
		case 'm':
                        memorize_sample_features ();
			break;
		// Clears the target representation
		case 'C':	
		case 'c':
                        clear_target_representation ();
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
		// Moves the input to the first person
		case 'P':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, 1, 0, 0, -1);
			UpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the last person
		case 'L':
			g_pCurrentFace = albumGetLastPhoto (g_pAlbum, g_pCurrentFace->person, 0, 0, 0);
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, g_pCurrentFace->person, 0, 0, -1);
			UpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the last photo
		case 'l':
			g_pCurrentFace = albumGetLastPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);
			UpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the next person
		case 'N':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, (g_pCurrentFace->person + 1), 0, 0, 0);
			UpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
		// Moves the input to the next photo
		case 'n':
			g_pCurrentFace = albumGetNextPhoto (g_pAlbum, g_pCurrentFace->person, g_pCurrentFace->pose, g_pCurrentFace->expression, g_pCurrentFace->date);
			UpdateInputImage (&in_pattern, g_pCurrentFace->name);
			break;
	}
	all_filters_update ();
	all_outputs_update ();
	
	return;
}



/*
********************************************************
* Function: V1ToImageMapping			       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT V1ToImageMapping (PARAM_LIST *param_list)
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
* Function: CalculateRandomPosition		       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT CalculateRandomPosition (PARAM_LIST *param_list)
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
* Function: LoadAlbum				       *
* Description:  				       *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

NEURON_OUTPUT LoadAlbum (PARAM_LIST *param_list)
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
* Function: GetFace                                    *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

NEURON_OUTPUT GetFace (PARAM_LIST *param_list)
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
	UpdateInputImage (&in_pattern, g_pCurrentFace->name);

    
	// Returns the photo data and expression
	result.ival = (g_pCurrentFace->date & 0x0F) << 3 | 
		      (g_pCurrentFace->expression & 0x08);

	return (result);
}



/*
********************************************************
* Function: GetNoseCoordinates			       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT GetNoseCoordinates (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *x, *y;
	
	// Get the Address of the Target Coordenates
	x = (int *) param_list->next->param.pval;
	y = (int *) param_list->next->next->param.pval;
	
	*x = g_nTargetX = (int) ((float) g_pCurrentFace->nose[0] * X_FACTOR);
	*y = g_nTargetY = (int) ((ORIGINAL_IMAGE_HEIGHT - (float) g_pCurrentFace->nose[1] - 1) * Y_FACTOR);
	
	return (output);
}
