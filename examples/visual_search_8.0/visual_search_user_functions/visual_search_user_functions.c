#include "filter.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define IDXPHOTO_MIN		1
#define IDXPHOTO_MAX		2
#define PERSON_MIN		1
#define PERSON_MAX 		67

#define FILE_WIDTH		768
#define FILE_HEIGHT		576
#define IMAGE_WIDTH		256
#define IMAGE_HEIGHT		192

#define CUT_POINT		1.0

// The original input size used is 384 x 288
#define X_CORRECTION_FACTOR	(double) IMAGE_WIDTH  / 384.0
#define Y_CORRECTION_FACTOR	(double) IMAGE_HEIGHT / 288.0

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define INPUT_PATH		"/home/hallysson/img/dbf1_new/"
#define INPUT_NAME		"in_pattern"

#define NL_TARGET_COORDINATES_NAME	"nl_target_coordinates"

#define NL_FEATURES_NAME		"nl_features"

#define NL_TARGET_NAME			"nl_target"

// Global Variables
int g_nIdxPhoto, g_nPerson;	// Mount file name: <g_nPerson><g_nIdxPhoto>.raw

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;

// Eye, nose and mouth image coordinates
int g_nEyeX, g_nEyeY;
int g_nNoseX, g_nNoseY;
int g_nMouthX, g_nMouthY;

// Image 
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

float global_max_value = 0.0;
 
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
	INPUT_DESC *inPattern = NULL;
	
	if ((inPattern = get_input_by_name (INPUT_NAME)) == NULL)
		Erro ("Error: cannot get neuron layer ", INPUT_NAME, " by name.");
			
	g_nTargetX = inPattern->wxd = inPattern->ww/2;
	g_nTargetY = inPattern->wyd = inPattern->wh/2;

	sprintf (strCommand, "move %s to %d, %d;", inPattern->name, inPattern->wxd, inPattern->wyd);
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);
	
	update_input_neurons (inPattern);
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

void 
GetNextFileName(char *strFileName, int direction)
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

int 
ReadCoordinate(FILE *file)
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

void 
ReadFaceInput(INPUT_DESC *input, char *strFileName)
{
	int i, nInd;
	int x, y;
	FILE *file;
	int nCol, nLin;
	int nAuxCol, nAuxLin;
	int nFactor;
	int nAux1, nAux2;

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

void 
GetNewFace (INPUT_DESC *input, int direction)
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

void
make_input_image_visual_search (INPUT_DESC *input, int w, int h)
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

void 
init_visual_search (INPUT_DESC *input)
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

void 
input_generator (INPUT_DESC *input, int status)
{
	int x, y;
	int i;
	OUTPUT_DESC *output;
	FILTER_DESC *filter;

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
			all_outputs_update ();			
		}
	}
}



/*
********************************************************
* Function: jump				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void 
jump (INPUT_DESC *input)
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
* Function: saccade				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void 
saccade (INPUT_DESC *input)
{
	static NEURON_LAYER *nl_target_coordinates = NULL;
	int x, y;
	int count = 0;
	
	// Gets the target coordinates neuron layer
	if (nl_target_coordinates == NULL)
		nl_target_coordinates = get_neuron_layer_by_name (NL_TARGET_COORDINATES_NAME);
	
	// Saccade until reach the target
	do
	{
		x = (int) nl_target_coordinates->neuron_vector[0].output.fval;
		y = (int) nl_target_coordinates->neuron_vector[1].output.fval;
		
		input->wxd += x;
		input->wyd += y;

		move_input_window (input->name, input->wxd, input->wyd);
			
		count++;
		
		if (((x + y) == 0) && (global_max_value < CUT_POINT))
		{
			input->wxd = rand () % IMAGE_WIDTH;
			input->wyd = rand () % IMAGE_HEIGHT;
			
			continue;
		}
		
	} while (((x + y) != 0) && (count < 5));
	
	
	return;
}



/*
********************************************************
* Function: memorize_features			       *
* Description: 					       *
* Inputs: 					       *
* Output: 	  				       *
********************************************************
*/

void
memorize_features ()
{
	static NEURON_LAYER *nl_features = NULL;
	static FILTER_DESC *filter_desc = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_input  = NULL;
	NEURON_LAYER *nl_output = NULL;
	int index, u, v, w, h, wo, ho;
	int num_params, num_layers;
	int quadrant, quadrants_number;
	int circle, circles_number;
	int feature, features_per_column;
	int column, columns_per_class;
	int class, class_width, class_height;
	float band_width;
		
	if (filter_desc == NULL)
	{
		nl_features = get_neuron_layer_by_name (NL_FEATURES_NAME);
		filter_desc = get_filter_by_output (nl_features);
	}
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 4)
	{
		Erro ("Error: Wrong number of parameters. The features_gather_filter must have 3 parameters.", "", "");
		return;
	}

	// Gets Parameters
	quadrants_number = filter_desc->filter_params->next->param.ival;
	circles_number   = filter_desc->filter_params->next->next->param.ival;
	band_width       = filter_desc->filter_params->next->next->next->param.fval;

	if ((band_width < 0.0) && (band_width > 1.0))
	{
		Erro ("Error: The band width parameter must be in the interval [0.0, 1.0].", "", "");
		return;
	}
	
	// Gets the Output Neuron Layer
	nl_output = filter_desc->output;
	
	// Gets Output Dimentions
	wo = features_per_column = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
			
	// Checks Neuron Layers
	for (num_layers = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, num_layers++)
		;

	if (num_layers != features_per_column)
	{
		Erro ("Error: The number of input neural layers must be the same as the output width.", "", "");
		return;
	}

	for (n_list = filter_desc->neuron_layer_list, feature = 0; feature < features_per_column; n_list = n_list->next, feature++)
	{
		// Gets the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;
		
		// Gets the Input Neuron Layer Dimentions
		w = nl_input->dimentions.x;
		h = nl_input->dimentions.y;
		
		class_width = (int) (band_width * (float) w) / quadrants_number;
		class_height = h / circles_number;
		columns_per_class = class_width * class_height;
		
		//printf ("class_width = %d class_height = %d columns_per_class = %d\n", class_width, class_height, columns_per_class);

		for (v = 0; v < h; v++)
		{
			for (u = (int) ((0.5 - 0.5 * band_width) * (float) w); u < (int) ((0.5 + 0.5 * band_width) * (float) w); u++)
			{
				class = (u - (int) ((0.5 - 0.5 * band_width) * (float) w)) / class_width + (v / class_height) * quadrants_number;
				column = (u - (int) ((0.5 - 0.5 * band_width) * (float) w)) % class_width + (v % class_height) * class_width;
				index = feature + column * features_per_column + class * features_per_column * columns_per_class;
				
				nl_output->neuron_vector[index].output = nl_input->neuron_vector[u + v * w].output;
				//printf ("u = %d v = %d class = %d column = %d index = %d\n", u, v, class, column, index);
			}
		}
	}
	//printf ("end\n");
	
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
input_controler (INPUT_DESC *input, int status)
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

void
draw_output (char *output_name, char *input_name)
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

void 
f_keyboard (char *key_value)
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
	INPUT_DESC *inPattern = NULL;
	
	if ((inPattern = get_input_by_name (INPUT_NAME)) == NULL)
		Erro ("Error: cannot get input ", INPUT_NAME, " by name.");
		
	switch (key = key_value[0])
	{
		// Train network
		case 'T':	
		case 't':
			train_network ();
			run_network (1);
			break;
		// Memorizes the Current Features
		case 'M':	
		case 'm':
			memorize_features ();
			break;
		// Report the eye, nose and mouth coordinates
		case 'R':
		case 'r':
			printf ("Eye:[%03d,%03d] - Nose:[%03d,%03d] - Mouth:[%03d,%03d]\n", g_nEyeX, g_nEyeY, g_nNoseX, g_nNoseY, g_nMouthX, g_nMouthY);
			break;
		// Report the partial statistics
		case 'e':
			samplesNumber++;
			matchsNumber += match = sqrtf ((g_nNoseX - inPattern->wxd) * (g_nNoseX - inPattern->wxd) + (g_nNoseY - inPattern->wyd) * (g_nNoseY - inPattern->wyd)) <= 10.0 ? 1 : 0;
			rightnessVector[(samplesNumber - 1) % IDXPHOTO_MAX] += match;
			printf ("Sample: %d\tMatch: %s\tRightness: %.1f\%\n", samplesNumber, matchInfoVector[match], 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Report the final statistics
		case 'E':
			printf ("*************************************************\n");
			printf ("*            Final Statistics                   *\n");
			printf ("*************************************************\n");
			printf ("Face Type\t\t\tRightness\n");			
			for (i = 0; i < IDXPHOTO_MAX; i++)
				printf ("%2d) %s\t%.1f\%\n", (i+1), photoInfoVector[i], 100.0 * (float) (rightnessVector[i] * IDXPHOTO_MAX) / (float) samplesNumber);			
			
			printf ("\nTotal Rightness: %.1f\%\n", 100.0 * (float) matchsNumber / (float) samplesNumber);			
			break;
		// Moves the input to the first person
		case 'P':
			g_nIdxPhoto = IDXPHOTO_MIN + 1;
			g_nPerson = PERSON_MIN;
			GetNewFace (inPattern, DIRECTION_REWIND);
			break;
		// Moves the input to the last person
		case 'L':
			g_nIdxPhoto = IDXPHOTO_MIN + 1;
			g_nPerson--;
			GetNewFace (inPattern, DIRECTION_REWIND);
			break;
		// Moves the input to the last photo
		case 'l':
			GetNewFace (inPattern, DIRECTION_REWIND);
			break;
		// Moves the input to the next person
		case 'N':
			g_nIdxPhoto = IDXPHOTO_MIN - 1;
			g_nPerson++;
			GetNewFace (inPattern, DIRECTION_FORWARD);
			break;
		// Moves the input to the next photo
		case 'n':
			GetNewFace (inPattern, DIRECTION_FORWARD);
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade (inPattern);		
		// Jump to the target 
		case 'J':
		case 'j':
			jump (inPattern);
			break;	
	}
	
	return;
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
output_handler_max_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_max, v_max, w, h, xi, yi, step;
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

void 
output_handler_min_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_min, v_min, w, h, xi, yi, step;
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
	nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
}



/*
*********************************************************************************
* Function: output_handler_mean_position					*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

void
output_handler_mean_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char *nl_target_coordinates_name = NULL;
	int u, v, u_max, v_max, w, h, xi, yi;
	float x_mean, y_mean, weight, accumulator, log_factor, cut_point;
	NEURON_LAYER *nl_target_coordinates = NULL;
		
	// Gets the output handler parameters
	nl_target_coordinates_name = output->output_handler_params->next->param.sval;
	log_factor = output->output_handler_params->next->next->param.fval;
	cut_point = output->output_handler_params->next->next->next->param.fval;
	
	// Gets the target coordinates neuron layer
	nl_target_coordinates = get_neuron_layer_by_name (nl_target_coordinates_name);
	
	// Gets the neuron layer dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Initialize the mean coordinates 
	x_mean = 0.0;
 	y_mean = 0.0;
	
	// Initialize the Accumulator 
	accumulator = 0.0;
	
	// Calculate the mean coordinates
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			// Gets the weight value
			weight = output->neuron_layer->neuron_vector[v * h + u].output.fval;
			
			if (weight < cut_point)
				continue;
				
			// Map the Mean Coordinates to Image
			map_v1_to_image (&xi, &yi, IMAGE_WIDTH, IMAGE_HEIGHT, u, v, w, h, 0, 0, (double) h / (double) (h -1), log_factor);

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
}



/*
********************************************************
* Function: v1_to_image_mapping			       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

NEURON_OUTPUT
v1_to_image_mapping (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *xi, *yi;
	int wi, hi;
	int u, v;
	int w, h;
	
	// Get the Address of the Image Coordenates
	xi = (int *) param_list->next->param.pval;
	yi = (int *) param_list->next->next->param.pval;
	
	// Get the Image Dimentions
	wi = param_list->next->next->next->param.ival;
	hi = param_list->next->next->next->next->param.ival;
	
	// Get the V1 Coordenates
	u = param_list->next->next->next->next->next->param.ival;
	v = param_list->next->next->next->next->next->next->param.ival;
	
	// Get the Image Dimentions
	w = param_list->next->next->next->next->next->next->next->param.ival;
	h = param_list->next->next->next->next->next->next->next->next->param.ival;
	
	map_v1_to_image (xi, yi, wi, hi, u, v, w, h, g_nTargetX, g_nTargetY, (double) h / (double) (h -1), 2.0);
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

NEURON_OUTPUT
calculate_random_position (PARAM_LIST *param_list)
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

NEURON_OUTPUT
get_target_coordinates (PARAM_LIST *param_list)
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
