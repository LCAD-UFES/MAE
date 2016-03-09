#include "road_finding_user_functions.h"

// Global Variables
int g_nPersonID = 0;
int g_nFaceNum = 1;
int g_nFacePart;
int g_nStatus;
int g_nPos;
int g_nTries = 1;
int g_nTry = 1;

// Images filename
char g_strRandomFacesFileName[256];

/*
**********************************************************************************
* Function: set_neuron_layer_band_float
* Description: Writes an specified nueron layer band wiwh some float output value
* Inputs: x1,x2,y1,y2,inside_value,outside_value
* Output: None
**********************************************************************************
*/

void set_neuron_layer_band_float (NEURON_LAYER *neuron_layer, int x1, int x2, int y1,int y2, float inside_value, float outside_value)
{
	int i, x, y, w, h;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	for (i = 0; i < w*h; i++)
		neuron_layer->neuron_vector[i].output.fval = outside_value;

	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				neuron_layer->neuron_vector[x + w * y].output.fval = inside_value;
}

/*
***********************************************************************************
* Function: set_neuron_layer_band_int
* Description: Writes an specified neuron layer band with some integer output value
* Inputs: x1,x2,y1,y2,inside_value,outside_value
* Output: None
***********************************************************************************
*/

void set_neuron_layer_band_int (NEURON_LAYER *neuron_layer, int x1, int x2, int y1,int y2, int inside_value, int outside_value)
{
	int i, x, y, w, h;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	for (i = 0; i < w*h; i++)
		neuron_layer->neuron_vector[i].output.ival = outside_value;

	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				neuron_layer->neuron_vector[x + w * y].output.ival = inside_value;
}

/*
***********************************************************************************
* Function: set_neuron_layer_band_int_inside_only
* Description: Writes an specified neuron layer band with some integer output value
* Inputs: x1,x2,y1,y2,inside_value
* Output: None
***********************************************************************************
*/

void set_neuron_layer_band_int_inside_only (NEURON_LAYER *neuron_layer, int x1, int x2, int y1,int y2, int inside_value)
{
	int x, y, w, h;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;

	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				neuron_layer->neuron_vector[x + w * y].output.ival = inside_value;
}

/*
***********************************************************************************
* Function: write_neuron_layer_with_gaussian_algorithm_equivalent
* Description: Writes an specified neuron layer band with some integer output value
* Inputs:	- neuron_layer
		- strFileName
* Output: 0 if sucess, -1 else
***********************************************************************************
*/

int write_neuron_layer_with_gaussian_algorithm_equivalent(NEURON_LAYER *neuron_layer,char *strFileName)
{
	FILE *pFile = NULL;

	if ((pFile = fopen (strFileName, "rb")) == NULL)
	{
		printf ("Error: cannot open file '%s' (ReadImageInput).\n", strFileName);
		return (-1);
	}
	
	// Loads PNM image into neuron layer
	load_neuron_layer_with_p2_format(neuron_layer,pFile);
	fclose(pFile);
	
	// Successful execution, return 0
	return(0);
}

/*
***********************************************************
* Function: GetNextFileName
* Description:
* Inputs: strFileName -
*	  nDirection -
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

int GetNextFileName(char *strFileName, int nDirection)
{
	FILE *pFile = NULL;
	int nAttempt = 0;

	if (nDirection == DIRECTION_FORWARD)
	{
		if(g_nPersonID == IMG_SET_SIZE-1)
			g_nPersonID = 1;
		else
			g_nPersonID++;
	}
	else if (nDirection == DIRECTION_REWIND)
	{
		if(g_nPersonID == 1)
			g_nPersonID = IMG_SET_SIZE-1;
		else
			g_nPersonID--;
	}
	else
	{
		printf("Unknown Direction.\n");
		return(-1);
	}
			
	while ((pFile == NULL) && (nAttempt < 3))
	{
		
		sprintf (strFileName, "%sright_img%02d.bmp", INPUT_PATH,g_nPersonID);
		printf ("FileName: %s\n", strFileName);
		fflush (stdout);

		if ((pFile = fopen (strFileName, "r")) != NULL)
		{
			fclose (pFile);
		}
		else			// In case of file opening failure
		{
			switch (nAttempt)
			{
				case 2:	// Nao ha mais arquivos para ler ( falha das tres tentativas )
					strcpy (strFileName, "");
					return (-1);
			}
			nAttempt++;
		}
	}
	
	return (0);
}



/*
***********************************************************
* Function: ReadImageInput
* Description:
* Inputs: input -
*	  strFileName -
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

int ReadImageInput(INPUT_DESC *input, char *strFileName)
{
	FILE *pFile = NULL;

	if ((pFile = fopen (strFileName, "rb")) == NULL)
	{
		printf ("Error: cannot open file '%s' (ReadImageInput).\n", strFileName);
		return (-1);
	}
	
	// Loads bitmapped image into input neuron layer
	fseek (pFile,3*18,SEEK_SET);
	load_input_image_with_BMP_format(input,pFile);
	fclose(pFile);
	
	// Successful execution, return 1
	return(0);
}



/*
***********************************************************
* Function: make_input_image_face_recog
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void make_input_image_face_recog (INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = w; //nearest_power_of_2 (w);
	input->tfh = h; //nearest_power_of_2 (h);
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
	
	input->vpxo = 0; 		// Initial X position(?)
	input->vpyo = h - input->vph;	// Initial Y position(?)

	if(input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}




/*
***********************************************************
* Function: init_face_recog
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_road_finding(INPUT_DESC *input)
{

#ifndef NO_INTERFACE
	int x, y;
	char strFileName[128];
#endif

	make_input_image_face_recog (input, IMAGE_WIDTH, IMAGE_HEIGHT);

#ifndef NO_INTERFACE
	// Reads first image input from file (Only if interface is used)
	if (!GetNextFileName(strFileName, DIRECTION_FORWARD))
	{
		ReadImageInput(input, strFileName);
	}
	
	update_input_neurons (input);
	
	// Initiates Glut windows
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



/*
***********************************************************
* Function: init_user_functions
* Description:
* Inputs:none
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int init_user_functions (void)
{
	char strCommand[128];
			
	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	return (0);
}



/*
***********************************************************
* Function: GetNewFace
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int GetNewFace(INPUT_DESC *input, int nDirection)
{
	FILTER_DESC *filter;
	char strFileName[128];

	//Holds until image can be read from file 
	while (GetNextFileName(strFileName, nDirection));

	if (ReadImageInput(input, strFileName))
		return (-1);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);	
	update_input_image (input);

	filter = get_filter_by_output(out_road_finding_input_filtered.neuron_layer);
	filter_update(filter);
	output_update(&out_road_finding_input_filtered);
	
	return (0);
}



/*
***********************************************************
* Function: input_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void input_generator (INPUT_DESC *input, int status)
{
	//FILTER_DESC *filter;

	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_road_finding(input);
#ifdef NO_INTERFACE
		input->win = 1;	//Input already initialized
#endif
 	}
	else
	{
		printf("Switching to another picture\n");
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewFace (input, DIRECTION_REWIND);
			else if (input->wxd >= IMAGE_WIDTH)
				GetNewFace (input, DIRECTION_FORWARD);

			//filter = get_filter_by_output(out_road_finding_input_filtered.neuron_layer);
			//filter_update(filter);
			//output_update(&out_road_finding_input_filtered);
			//Neuron layer output must be also updated

#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}	
}



/*
***********************************************************
* Function: draw_output
* Description:Draws the output layer
* Inputs: strOutputName -
*	  strInputName -
* Output: None
***********************************************************
*/

void draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;

	output = get_output_by_name (strOutputName);

	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, 1/*g_nPersonUniqueID*/);

	update_output_image (output);
#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
#endif
}



/*
***********************************************************
* Function: input_controler
* Description: handles the mouse input window events 
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void input_controler (INPUT_DESC *input, int status)
{
	// Input controller isnt actually needed
	
	char strCommand[128];

	// Window movement must be kept
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
	}

	input->mouse_button = -1;
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: person ID
***********************************************************
*/

#if 0
int EvaluateOutput(OUTPUT_DESC *output)
{
	int i;
	int nPersonID = 0;
	int nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < (MAX_MAN_ID+MAX_WOMAN_ID); i++)
		g_vCountPerson[i] = 0;
		
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < (MAX_MAN_ID+MAX_WOMAN_ID)))
			g_vCountPerson[neuron_vector[i].output.ival] += 1;
	}

	for (i = 0; i < (MAX_MAN_ID+MAX_WOMAN_ID); i++)
	{
		if (g_vCountPerson[i] > nMax)
		{
			nMax = g_vCountPerson[i];
			nPersonID = i;
		}
	}

	return (nPersonID);
}
#endif


/*
***********************************************************
* Function: output_handler
* Description:
* Inputs:
* Output:
***********************************************************
*/

// This output handler was not used here
#if 0
void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int person;
	float confidence;
	static float best_confidence = -1.0;
	static int nPersonOutput;

	if (g_nStatus == RECALL_PHASE)
	{
		// Avalia a saida
		if (strcmp (output->name, out_road_finding.name) == 0)
		{
			if (g_nTry != 0)
			{
				person = EvaluateOutput2(output, &confidence);
				if (confidence > best_confidence)
				{
					best_confidence = confidence;
					nPersonOutput = person;
				}
				g_nTry--;
			}
			if (g_nTry == 0)
			{
				//g_nTotalTested++;

				if ((nPersonOutput) == 0)
				//if ((nPersonOutput) == g_nPersonUniqueID)
				{
					//g_nCorrect++;
					//printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));
				}
				else
				{
					//printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f ***\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));	
				}
				fflush (stdout);
				g_nTry = g_nTries;
				best_confidence = -1.0;
			}
		}
	}
	
#ifndef NO_INTERFACE	
	glutSetWindow (output->win);
	output_display (output);
#endif
}
#endif

/*
Commented cursor postition setting, for reuse purposes
*/
#if 0
void
set_curr_pos ()
{
	if (g_nPos == 0)
	{
		g_nRightEyeX = face_recog.wxd;
		g_nRightEyeY = face_recog.wyd;
	}
	else if (g_nPos == 1)
	{
		g_nLeftEyeX = face_recog.wxd;
		g_nLeftEyeY = face_recog.wyd;
	}
	else if (g_nPos == 2)
	{
		g_nNoseX = face_recog.wxd;
		g_nNoseY = face_recog.wyd;
	}
	else
	{
		g_nMouthX = face_recog.wxd;
		g_nMouthY = face_recog.wyd;
	}
}
#endif


/*
***********************************************************
* Function: f_keyboard
* Description: Funcao chamada quando e pressionada uma tecla.
* Inputs: key_value - ponteiro para o cacacter pressionado
* Output: -
***********************************************************
*/


void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
		// Moves the input to the next photo
		case 'N':
		case 'n':
			GetNewFace (&road_finding, DIRECTION_FORWARD);
			g_nPos = 0;
			break;
		// Moves the input to the next previuos photo
		case 'B':
		case 'b':
			GetNewFace (&road_finding, DIRECTION_REWIND);
			g_nPos = 0;
			break;
		#if 0
		// Moves cross 1 pixel up
		case 'I':
		case 'i':
			face_recog.wyd += 1;
			face_recog.green_cross_x = face_recog.wxd;
			face_recog.green_cross_y = face_recog.wyd;
			set_curr_pos ();
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Moves cross 1 pixel down
		case 'M':
		case 'm':
			face_recog.wyd -= 1;
			face_recog.green_cross_x = face_recog.wxd;
			face_recog.green_cross_y = face_recog.wyd;
			set_curr_pos ();
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Moves cross 1 pixel right
		case 'K':
		case 'k':
			face_recog.wxd += 1;
			face_recog.green_cross_x = face_recog.wxd;
			face_recog.green_cross_y = face_recog.wyd;
			set_curr_pos ();
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Moves cross 1 pixel left
		case 'J':
		case 'j':
			face_recog.wxd -= 1;
			face_recog.green_cross_x = face_recog.wxd;
			face_recog.green_cross_y = face_recog.wyd;
			set_curr_pos ();
			glutPostWindowRedisplay (face_recog.win);
			break;
		#endif
		// Write the desired output activation pattern and train the Network
		case 'T':
		case 't':
			g_nStatus = TRAINING_PHASE;
			//255 is the "high" output activation value, 0 is the "low" one.
			set_neuron_layer_band_int(&nl_road_finding, LEFT_BOUND /*x1*/, RIGHT_BOUND /*x2*/, LOWER_BOUND /*y1*/, UPPER_BOUND /*y2*/, 255,0);
			train_neuron_layer("nl_road_finding");
			all_outputs_update ();
			g_nStatus = RECALL_PHASE;
			break;
		// Recall phase - compute output
		case 'R':
		case 'r':
			// It computes the output of the output of the network for a single input image
			//move_input_window (road_finding.name, road_finding.ww/2, road_finding.wh/2);	//returns image cursor to center
			all_dendrites_update();
			//printf("All Dendrites Updated\n");
			all_neurons_update();
			//printf("All Neurons Updated\n");
			all_outputs_update();
			//printf("All Outputs Updated\n");
			break;
		
	}
}

