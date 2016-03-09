#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../face_recog.h"
#include "face_recog_user_functions.h"

// Global Variables
int g_nPersonID = 0;
int g_nIllumination = 0;
int g_nMaxPersonID = MAX_PERSON_ID;
int g_vCountPerson[MAX_PERSON_ID+1];
int g_nStatus;
int g_nTries = 1;
int g_nTry = 1;
char g_strRandomFacesFileName[256];


// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;


char *g_chPersonID[] =
{
	"01", "02", "03", "04", "05", "06", "07", "08", "09", "10", 
	"11", "12", "13",       "15", "16", "17", "18", "19", "20", 
	"21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
	"31", "32", "33", "34", "35", "36", "37", "38", "39"
};

char *g_chIllumination[] =
{
	"A+000E+00",
	"A+000E-20",
	"A+000E+20",
	"A+000E-35",
	"A+000E+45",
	"A+000E+90",
	"A-005E-10",
	"A-005E+10",
	"A+005E-10",
	"A+005E+10",
	"A-010E+00",
	"A+010E+00",
	"A-010E-20",
	"A+010E-20",
	"A-015E+20",
	"A+015E+20",
	"A-020E-10",
	"A-020E+10",
	"A+020E-10",
	"A+020E+10",
	"A-020E-40",
	"A+020E-40",
	"A-025E+00",
	"A+025E+00",
	"A-035E+15",
	"A+035E+15",
	"A-035E-20",
	"A+035E-20",
	"A-035E+40",
	"A+035E+40",
	"A-035E+65",
	"A+035E+65",
	"A-050E+00",
	"A+050E+00",
	"A-050E-40",
	"A+050E-40",
	"A-060E-20",
	"A-060E+20",
	"A+060E-20",
	"A+060E+20",
	"A-070E+00",
	"A+070E+00",
	"A-070E-35",
	"A+070E-35",
	"A-070E+45",
	"A+070E+45",
	"A-085E-20",
	"A-085E+20",
	"A+085E-20",
	"A+085E+20",
	"A-095E+00",
	"A+095E+00",
	"A-110E+15",
	"A+110E+15",
	"A-110E-20",
	"A+110E-20",
	"A-110E+40",
	"A+110E+40",
	"A-110E+65",
	"A+110E+65",
	"A-120E+00",
	"A+120E+00",
	"A-130E+20",
	"A+130E+20",
};



/*
***********************************************************
* Function: GetNextFileName
* Description:
* Inputs: strFileName -
*	  nDirection -
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

int 
GetNextFileName(char *strFileName, int nDirection)
{
	FILE *pFile = NULL;

	if (nDirection == DIRECTION_FORWARD)
	{
		if (g_nIllumination > MAX_ILLUMINATION)
		{
			if (g_nPersonID >= MAX_PERSON_ID)
			{
				g_nPersonID = MIN_PERSON_ID;
				g_nIllumination = MIN_ILLUMINATION;
			}
			else
			{
				g_nPersonID++;
				g_nIllumination = MIN_ILLUMINATION;
			}
		}
	}
	else
	{
		if (g_nIllumination < MIN_ILLUMINATION)
		{
			if (g_nPersonID <= MIN_PERSON_ID)
			{
				g_nPersonID = MAX_PERSON_ID;
				g_nIllumination = MAX_ILLUMINATION;
			}
			else
			{
				g_nPersonID--;
				g_nIllumination = MAX_ILLUMINATION;
			}
		}
	}

	sprintf(strFileName, "%s/yaleB%s/yaleB%s_P00%s.pgm", INPUT_PATH, 
			      g_chPersonID[g_nPersonID], g_chPersonID[g_nPersonID], g_chIllumination[g_nIllumination]);
	printf ("%02d %02d - FileName: %s\n", g_nPersonID, g_nIllumination, strFileName);
	fflush (stdout);

	if ((pFile = fopen (strFileName, "r")) != NULL)
	{
		fclose (pFile);
		return (0);
	}
	else
	{
		strcpy (strFileName, "");
		return (-1);
	}
	
	return (0);
}



/*
***********************************************************
* Function: ReadFaceInput
* Description: Reads a RAW image to the MAE input
* Inputs: input - input image
*	  strFileName - Image file name
* Output: 0 if OK, -1 otherwise
* Comment: Image format: [RRRRR...][GGGGG...][BBBBB...]
***********************************************************
*/

int ReadFaceInput(INPUT_DESC *input, char *strFileName)
{
	load_input_image (input, strFileName);
	return (0);
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
***********************************************************
* Function: init_face_recog
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_face_recog(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif
	char strFileName[128];

	g_nPersonID = MIN_PERSON_ID;
	g_nIllumination = MIN_ILLUMINATION;
	g_nStatus = MOVING_PHASE;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	
	make_input_image_face_recog (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Le a primeira imagem
	if (!GetNextFileName(strFileName, DIRECTION_FORWARD))
	{
		ReadFaceInput(input, strFileName);
	}
		
	input->up2date = 0;
	update_input_neurons (input);

#ifndef NO_INTERFACE
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
	char strFileName[128];

	if (nDirection == DIRECTION_FORWARD)
		g_nIllumination++;
	else
		g_nIllumination--;


	while (GetNextFileName(strFileName, nDirection))
		;

	if (ReadFaceInput(input, strFileName))
		return (-1);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
		
	update_input_image (input);
		
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
	FILTER_DESC *filter;

	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_face_recog(input);
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
 	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewFace (input, DIRECTION_BACKWARD);
			else if (input->wxd >= IMAGE_WIDTH)
				GetNewFace (input, DIRECTION_FORWARD);

			output_update(&out_face_recog_lp);
			filter = get_filter_by_output(out_face_recog_lp.neuron_layer);
			filter_update(filter);
			output_update(&out_face_recog_lp);
			filter = get_filter_by_output(out_face_recog_lp_f.neuron_layer);
			filter_update(filter);
			output_update(&out_face_recog_lp_f);

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

	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, g_nPersonID);

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
	char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (strCommand, "draw out_landmark based on face_recog move;");
		interpreter (strCommand);
	}

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

int EvaluateOutput(OUTPUT_DESC *output)
{
	int i;
	int nPersonID = 0;
	int nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
		g_vCountPerson[i] = 0;
		
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= MIN_PERSON_ID) && (neuron_vector[i].output.ival <= MAX_PERSON_ID))
			g_vCountPerson[neuron_vector[i].output.ival] += 1;
	}

	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
	{
		if (g_vCountPerson[i] > nMax)
		{
			nMax = g_vCountPerson[i];
			nPersonID = i;
		}
	}

	return (nPersonID);
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: person ID
***********************************************************
*/

int EvaluateOutput2(OUTPUT_DESC *output, float *confidence)
{
	int i;
	int nPersonID = 0;
	int nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
		g_vCountPerson[i] = 0; // este vetor é global apenas para evitar alocação dinâmica ou algo mais inteligente
		
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= MIN_PERSON_ID) && (neuron_vector[i].output.ival <= MAX_PERSON_ID))
			g_vCountPerson[neuron_vector[i].output.ival] += 1;
	}

	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
	{
		if (g_vCountPerson[i] > nMax)
		{
			nMax = g_vCountPerson[i];
			nPersonID = i;
		}
	}

	*confidence = (float) nMax / (float) (output->wh * output->ww);
	return (nPersonID);
}


void output_handler_old (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int nPersonOutput;

	if (g_nStatus == RECALL_PHASE)
	{
		// Avalia a saida
		if (strcmp (output->name, out_landmark.name) == 0)
		{
			g_nTotalTested++;
			
			if ((nPersonOutput = EvaluateOutput(output) + 1) == g_nPersonID)
			{
				g_nCorrect++;
				printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f\n", nPersonOutput, g_nPersonID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));
			}
			else
				printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f ***\n", nPersonOutput, g_nPersonID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));	
		}
	}
	
#ifndef NO_INTERFACE	
	glutSetWindow (output->win);
	output_display (output);
#endif
}



/*
***********************************************************
* Function: output_handler
* Description:
* Inputs:
* Output:
***********************************************************
*/

void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int person;
	float confidence;
	static float best_confidence = -1.0;
	static int nPersonOutput;

	if (g_nStatus == RECALL_PHASE)
	{
		// Avalia a saida
		if (strcmp (output->name, out_landmark.name) == 0)
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
				g_nTotalTested++;

				if (nPersonOutput == g_nPersonID)
				{
					g_nCorrect++;
					printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f\n", nPersonOutput, g_nPersonID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));
				}
				else
				{
					printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f ***\n", nPersonOutput, g_nPersonID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));	
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



/*
***********************************************************
* Function: SetNetworkStatus
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT SetNetworkStatusN (int status)
{
	NEURON_OUTPUT output;

	g_nStatus = status;

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



// ----------------------------------------------------
// f_keyboard - Funcao chamada quando e pressionada uma
//				tecla.
//
// Entrada: key_value - ponteiro para o cacacter pressionado
//
// Saida: Nenhuma
// ----------------------------------------------------
void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
		// Moves the input to the next photo
		case 'n':
		case 'N':
			GetNewFace (&face_recog, DIRECTION_FORWARD);
			break;
		// Moves the input to the next previuos photo
		case 'B':
		case 'b':
			GetNewFace (&face_recog, DIRECTION_BACKWARD);
			break;
	}
}



int
check_person_parameters (char *function_name)
{
	if ((g_nPersonID < MIN_PERSON_ID) || (g_nPersonID > MAX_PERSON_ID))
	{
		printf ("Error: Invalid person ID in %s. It must be within the interval [%d, %d].\n", function_name, MIN_PERSON_ID, MAX_PERSON_ID);
		return (0);
	}

	if ((g_nIllumination < MIN_ILLUMINATION) || (g_nIllumination > MAX_ILLUMINATION))
	{
		printf ("Error: Invalid illumination in %s. It must be within the interval [%d, %d].\n", function_name, MIN_ILLUMINATION, MAX_ILLUMINATION);
		return (0);
	}
	
	return (1);
}



/*
***********************************************************
* Function: GetFace
* Description: Gets a new face
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetFace (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	
	g_nPersonID     = pParamList->next->param.ival;
	g_nIllumination = pParamList->next->next->param.ival;

	if (!check_person_parameters ("GetFace()"))
	{
		output.ival = -1;
		return (output);
	}
	
	sprintf(strFileName, "%s/yaleB%s/yaleB%s_P00%s.pgm", INPUT_PATH, 
			      g_chPersonID[g_nPersonID], g_chPersonID[g_nPersonID], g_chIllumination[g_nIllumination]);
	printf ("%02d %02d - FileName: %s\n", g_nPersonID, g_nIllumination, strFileName);
	fflush(stdout);
	
	if (ReadFaceInput (&face_recog, strFileName))
	{
		printf ("Error in GetFace(): Cannot read face '%s'.\n", strFileName);
		output.ival = -1;
		return (output);
	}	
	
	check_input_bounds (&face_recog, face_recog.wx + face_recog.ww/2, face_recog.wy + face_recog.wh/2);
	face_recog.up2date = 0;
	update_input_neurons (&face_recog);
	update_input_image (&face_recog);

	printf ("g_nPersonID = %d, g_nIllumination = %d\n", g_nPersonID, g_nIllumination);
	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: ShowStatistics
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT ShowStatistics (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	printf("Final result: %3.2f%%\n", (float) (g_nCorrect * 100.0 / g_nTotalTested));

	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: ResetStatistics
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT ResetStatistics (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	//g_nEyeCorrect = 0;
	//g_nNoseCorrect = 0;
	//g_nMouthCorrect = 0;

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

NEURON_OUTPUT GetRandomFace (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	FILE *pFile = NULL;
	int nRandomFace;
	int i;
	
	nRandomFace = pParamList->next->param.ival;

	if ((pFile = fopen (g_strRandomFacesFileName, "r")) == NULL)
	{
		printf ("Error in GetRandomFace(): cannot open file '%s'.\n", g_strRandomFacesFileName);
		output.ival = -1;
		return (output);
	}
	
	for (i = 0; i < nRandomFace; i++)
		fscanf (pFile, "%d %d\n", &g_nPersonID, &g_nIllumination);
	fclose (pFile);
	
	if (!check_person_parameters ("GetFace()"))
	{
		output.ival = -1;
		return (output);
	}
	
	sprintf(strFileName, "%s/yaleB%s/yaleB%s_P00%s.pgm", INPUT_PATH, 
			      g_chPersonID[g_nPersonID], g_chPersonID[g_nPersonID], g_chIllumination[g_nIllumination]);
	printf ("%02d %02d - FileName: %s\n", g_nPersonID, g_nIllumination, strFileName);
	fflush(stdout);
	
	if (ReadFaceInput (&face_recog, strFileName))
	{
		printf ("Error in GetRandomFace(): Cannot read face '%s'.\n", strFileName);
		output.ival = -1;
		return (output);
	}
		
	check_input_bounds (&face_recog, face_recog.wx + face_recog.ww/2, face_recog.wy + face_recog.wh/2);
	face_recog.up2date = 0;
	update_input_neurons (&face_recog);
	update_input_image (&face_recog);
		
	output.ival = 0;
	return (output);
}
