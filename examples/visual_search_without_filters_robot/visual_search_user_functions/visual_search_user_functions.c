#include <locale.h>
#include "visual_search_user_functions.h"
#include "camera.hpp"

#define CONFIDENCE_LEVEL 0.41

#define RAMDOM_FACES_TEST	"random_faces_t.txt"
#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

 
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
	char strCommand[128];
	
	make_input_image_visual_search (input, IMAGE_WIDTH, IMAGE_HEIGHT);

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
	visual_search_initialize_camera();

	sprintf (strCommand, "GetRight(0);");
	interpreter (strCommand);
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

	if (input->win == 0)
		init_visual_search (input);
	else
	{
		if (status == MOVE)
		{
			check_input_bounds (input, input->wxd, input->wxd);
			glutSetWindow (input->win);
			input_display ();
			filter_update(get_filter_by_name("in_pattern_filtered_filter"));
			filter_update(get_filter_by_name("in_pattern_filtered_translated_filter"));
			all_dendrites_update (); 
			all_neurons_update ();
			filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));
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
	int x, y;
	
	// Jump to the target
	x = (int) nl_target_coordinates.neuron_vector[0].output.fval;
	y = (int) nl_target_coordinates.neuron_vector[1].output.fval;
		
	input->wxd += x;
	input->wyd += y;

	translation_filter_deltaX = (float)(-IMAGE_WIDTH/2.0 + input->wxd);
	translation_filter_deltaY = (float)(-IMAGE_HEIGHT/2.0 + input->wyd);
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
	int x, y;
	
	output_handler_max_value_position (&out_v1_activation_map, 0, 0, 0);
	
	// Jump to the target
	x = (int) nl_target_coordinates.neuron_vector[0].output.fval;
	y = (int) nl_target_coordinates.neuron_vector[1].output.fval;
		
	input->wxd += x;
	input->wyd += y;

	translation_filter_deltaX = (float)(-IMAGE_WIDTH/2.0 + input->wxd);
	translation_filter_deltaY = (float)(-IMAGE_HEIGHT/2.0 + input->wyd);
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
	x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;
	do
	{
		x = (x > 0.0)? x + 0.5: x - 0.5;
		y = (y > 0.0)? y + 0.5: y - 0.5;
		input->wxd += (int) x;
		input->wyd += (int) y;
		translation_filter_deltaX = (float)(-IMAGE_WIDTH/2.0 + input->wxd);
		translation_filter_deltaY = (float)(-IMAGE_HEIGHT/2.0 + input->wyd);

		move_input_window (input->name, input->wxd, input->wyd);

		count++;
		x = nl_target_coordinates.neuron_vector[0].output.fval;
		y = nl_target_coordinates.neuron_vector[1].output.fval;
	} 
	while (((fabs(x) > 0.5) || (fabs(y) > 0.5)) && (count < 4));

	return;
}

/*
********************************************************
* Function: saccadic_movement			       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void saccadic_movement (INPUT_DESC *input)
{
	float x, y;
	//int count = 0;
	
	// Performs a single saccadic movement
	x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;
	x = (x > 0.0)? x + 0.5: x - 0.5;
	y = (y > 0.0)? y + 0.5: y - 0.5;
	input->wxd += (int) x;
	input->wyd += (int) y;
	translation_filter_deltaX = (float)(-IMAGE_WIDTH/2.0 + input->wxd);
	translation_filter_deltaY = (float)(-IMAGE_HEIGHT/2.0 + input->wyd);
	move_input_window (input->name, input->wxd, input->wyd);


	return;
}

/*
*	FOR TESTING PURPOSES ONLY:
*	Performs a coninuous point tracking test example
*/
void tracking(INPUT_DESC *input)
{
	char strCommand[128];
	sprintf (strCommand, "GetRight(0);");

	while(1)
	{
		interpreter (strCommand);
		saccadic_movement(input);
	}
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
		// Translate the input image & Move the input center cursor ( translated image filter and input window )
		translation_filter_deltaX = (float)(-IMAGE_WIDTH/2.0 + input->wxd);
		translation_filter_deltaY = (float)(-IMAGE_HEIGHT/2.0 + input->wyd);
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



extern float global_factor;
extern float teste;

extern int g_nNetworkStatus;

void	set_neuron_layer_band (NEURON_LAYER *neuron_layer, int x1, int x2, int y1,int y2, float value)
{
	int i, x, y, w, h;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	for (i = 0; i < w*h; i++)
		neuron_layer->neuron_vector[i].output.fval = .0f;
		
	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				neuron_layer->neuron_vector[x + w * y].output.fval = value;
}

/*
********************************************************
* Function: f_keyboard  			       *
* Description: keyboard events handler  	       *
* Inputs: key_value (pointer to pressed character)     *
* Output: none  				       *
********************************************************
*/

#define	MY_IDXPHOTOMAX 13

void f_keyboard (char *key_value)
{
	char key;
	/*int i, match;
	static int samplesNumber = 0;
	static int matchsNumber  = 0;
	static int rightnessVector[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	static int rightnessVector_total[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	char *matchInfoVector[2] = {"No", "Yes"};
	char *photoInfoVector[13] = {"Normal\t\t", "Smiling\t\t", "Serious\t\t", "Open Mouth\t\t", "Left Light\t\t",
				"Right Light\t\t", "Both Sides Light\t", "Sunglasses\t\t", "Left Light and Sunglasses",
				"Right Light and Sunglasses", "Cap\t\t\t", "Left Light and Cap\t", "Right Light and Cap\t"};*/

	switch (key = key_value[0])
	{
		// Train network
		case 'T':	
		case 't':
			g_nNetworkStatus = TRAINNING;
			set_neuron_layer_band (&nl_v1_activation_map, 27, 36, 0, 48, 1.0f);
			train_neuron_layer("nl_v1_activation_map");
			filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));

			all_outputs_update ();
			g_nNetworkStatus = RUNNING;
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade (&in_pattern);
			break;	
		// Jump to the target 
		case 'J':
			jump_max (&in_pattern);
			break;	
		case 'j':
			jump (&in_pattern);
			break;
		// Force the filters in_pattern_filtered_filter & in_pattern_filtered_translated_filter to be updated
		case 'U':
		case 'u':
			update_input_filters(NULL);
			break;
		// Performs a single sacccadic movement
		case 'm':
		case 'M':
			saccadic_movement(&in_pattern);
			break;
		// Performs the tracking exercise
		case 'r':
		case 'R':
			tracking(&in_pattern);
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
	
	// Zero return on default
	output.ival = 0;
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
	
	// Zero return on default
	output.ival = 0;
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
	
	// Zero returning on default
	output.ival = 0;
	return (output);
}


NEURON_OUTPUT GetFace (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	
	g_cSexID    = (pParamList->next->param.ival) ? 'w' : 'm';
	g_nPersonID = pParamList->next->next->param.ival;
	g_nPoseID   = pParamList->next->next->next->param.ival;

	if ((g_cSexID != 'm') && (g_cSexID != 'w'))
	{
		printf ("Error: Invalid sex ID, it must be 0 for man or 1 for woman (GetFace).\n");
		output.ival = -1;
		return (output);
	}
	
	g_nMaxPersonID = (g_cSexID == 'm') ? MAX_MAN_ID : MAX_WOMAN_ID;
	if ((g_nPersonID < MIN_PERSON_ID) || (g_nPersonID > g_nMaxPersonID))
	{
		printf ("Error: Invalid person ID, it must be within the interval [%d, %d] (GetFace).\n", MIN_PERSON_ID, g_nMaxPersonID);
		output.ival = -1;
		return (output);
	}
	
	if ((g_nPoseID < POSE_MIN) || (g_nPoseID > POSE_MAX))
	{
		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetFace).\n", POSE_MIN, POSE_MAX);
		output.ival = -1;
		return (output);
	}
	
	sprintf(strFileName, "%s%c-%03d-%d.raw", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);
//	sprintf(strFileName, "%s%c-%03d-%d.pnm", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);
	printf("FileName: %s\n", strFileName);
	fflush(stdout);	
	
	check_input_bounds (&in_pattern, in_pattern.wx + in_pattern.ww/2, in_pattern.wy + in_pattern.wh/2);
	in_pattern.up2date = 0;
	update_input_neurons (&in_pattern);
	update_input_image (&in_pattern);

	g_nPersonUniqueID = g_nPersonID * 2 - ((g_cSexID == 'w') ? 0 : 1);
	printf ("g_nPersonID = %d, g_cSexID = %d, g_nPersonUniqueID = %d\n", g_nPersonID, g_cSexID, g_nPersonUniqueID);
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

NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			strcpy (g_strRandomFacesFileName, RAMDOM_FACES_TEST);
			break;
		case RECALL_PHASE:
			strcpy (g_strRandomFacesFileName, RAMDOM_FACES_RECALL);
			break;
	}
	
	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: SetNetworkStatus2
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT SetNetworkStatus2 (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;
	g_nTry = g_nTries = pParamList->next->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			strcpy (g_strRandomFacesFileName, RAMDOM_FACES_TEST);
			break;
		case RECALL_PHASE:
			strcpy (g_strRandomFacesFileName, RAMDOM_FACES_RECALL);
			break;
	}
	
	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: GetRandomFace
* Description: Gets a new face
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT update_input_filters(PARAM_LIST *pParamList)
{
	FILTER_DESC *filter;
	NEURON_OUTPUT output;
	
	filter = get_filter_by_name("in_pattern_filtered_filter");
	
	if(!filter)
	{	output.ival = -1;
		return output;
	}
	else
		filter_update(filter);

	filter = get_filter_by_name("in_pattern_filtered_translated_filter");

	if(!filter)
	{	output.ival = -1;
		return output;
	}
	else
		filter_update(filter);

	output.ival = 0;
	return output;
}

