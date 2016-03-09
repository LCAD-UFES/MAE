#include <stdlib.h>
#include "mae.h"
#include "../face_recog.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define POSE_MIN		1
#define POSE_MAX		26
#define MIN_PERSON_ID 		1
#define MAX_MAN_ID 		76
#define MAX_WOMAN_ID		60

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH		768
#define FILE_HEIGHT		576
#define IMAGE_WIDTH		384
#define IMAGE_HEIGHT		288

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define MALE			0
#define FEMALE			1

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

#define RAMDOM_FACES_TEST	"random_faces_t.txt"
#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

// Variaveis globais
char g_cSexID = 'm';
int g_nPersonID;
int g_nPoseID;
int g_nMaxPersonID = MAX_MAN_ID;
int g_vCountPerson[MAX_MAN_ID];
int g_nFaceNum = 1;
int g_nFacePart;
int g_nStatus;
int g_nPos;
int g_nTries = 1;
int g_nTry = 1;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;
int g_nNoOne;

// Imagem
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

// Coordenadas X,Y das partes da face: olho, nariz e boca
int g_nLeftEyeX, g_nLeftEyeY;
int g_nRightEyeX, g_nRightEyeY;
int g_nNoseX, g_nNoseY;
int g_nMouthX, g_nMouthY;

int g_nPersonUniqueID = 1;
char g_strRandomFacesFileName[256];


/*
***********************************************************
* Function: OpenGroundTruths
* Description:
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

int GetGroundTruths (void)
{
	FILE *pFile = NULL;
	char strFileName[256];
		
	sprintf(strFileName, "%s%c-%03d-%d.txt", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);

	if ((pFile = fopen (strFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s'.\n", strFileName);
		g_nLeftEyeX = g_nLeftEyeY = 0;
		g_nNoseX = g_nNoseY = 0;
		g_nMouthX = g_nMouthY = 0;
		g_nRightEyeX = g_nRightEyeY = 0;
		return (-1);
	}
	
	fscanf (pFile, "left_eye_coordinates=%d %d\n", &g_nLeftEyeX, &g_nLeftEyeY);
	fscanf (pFile, "nose_coordinates=%d %d\n", &g_nNoseX, &g_nNoseY);
	fscanf (pFile, "mouth_coordinates=%d %d\n", &g_nMouthX, &g_nMouthY);
	if (fscanf (pFile, "right_eye_coordinates=%d %d\n", &g_nRightEyeX, &g_nRightEyeY) != 2)
	{
		g_nRightEyeX = g_nLeftEyeX;
		g_nRightEyeY = g_nLeftEyeY;
	}
		
	fclose (pFile);

	printf("Eyes:[%03d,%03d][%03d,%03d] - Nose:[%03d,%03d] - Mouth:[%03d,%03d]\n", g_nRightEyeX, g_nRightEyeY, g_nLeftEyeX, g_nLeftEyeY, g_nNoseX, g_nNoseY, g_nMouthX, g_nMouthY);
	
	return (0);
}



// ----------------------------------------------------
// 
//
// Entrada:
//
// Saida:
// ----------------------------------------------------

void SaveGroundTruths (void)
{
	FILE *pFile = NULL;
	char strFileName[256];
		
	sprintf(strFileName, "%s%c-%03d-%d.txt", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);

	if ((pFile = fopen (strFileName, "w")) == NULL)
	{
		printf ("Error: cannot open file '%s'.\n", strFileName);
		return;
	}
	
	fprintf (pFile, "left_eye_coordinates=%d %d\n", g_nLeftEyeX, g_nLeftEyeY);
	fprintf (pFile, "nose_coordinates=%d %d\n", g_nNoseX, g_nNoseY);
	fprintf (pFile, "mouth_coordinates=%d %d\n", g_nMouthX, g_nMouthY);
	fprintf (pFile, "right_eye_coordinates=%d %d\n", g_nRightEyeX, g_nRightEyeY);
	
	fclose (pFile);
	
	return;
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

	while ((pFile == NULL) && (nAttempt < 3))
	{
		sprintf (strFileName, "%s%c-%03d-%d.raw", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);
		printf ("FileName: %s\n", strFileName);
		fflush (stdout);

		if ((pFile = fopen (strFileName, "r")) != NULL)
		{
			fclose (pFile);
			return (0);
		}
		else
		{
			switch (nAttempt)
			{
				case 0:
					if (nDirection == DIRECTION_FORWARD)
					{
						if (g_nPoseID <= POSE_MAX)
							printf ("***** Attempt: 0, Person: %d, Pose: %d *****\n", g_nPersonID, g_nPoseID);
							
						g_nPoseID = POSE_MIN;
						g_nPersonID++;
					}
					else
					{		
						g_nPoseID = POSE_MAX;
						g_nPersonID--;
					}
					break;
				case 1:
					if (nDirection == DIRECTION_FORWARD)
					{
						if (g_nPoseID <= POSE_MAX)
							printf ("***** Attempt: 1, Person: %d, Pose: %d *****\n", g_nPersonID, g_nPoseID);

						g_nPoseID = POSE_MIN;
						g_nPersonID = MIN_PERSON_ID;
					}
					else
					{
						g_nPoseID = POSE_MAX;
						g_nPersonID = g_nMaxPersonID;
					}
					break;
				case 2:	// Nao ha mais arquivos para ler
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
	int i, nInd;
	int x, y;
	FILE *pFile = NULL;
	int nCol, nLin;
	int nAuxCol, nAuxLin;
	int nFactor = IMAGE_FACTOR;
	int nAux1;

	if ((pFile = fopen (strFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s' (ReadFaceInput).\n", strFileName);
		return (-1);
	}

	// Le a imagem
	nAux1 = (FILE_WIDTH * FILE_HEIGHT);
	for (i = 0; i < (nAux1 * 3); i++)
		g_pImageRAW[i] = fgetc(pFile);

	// Le as coordenadas do olho, nariz e boca
	GetGroundTruths ();
	input->green_cross_x = g_nLeftEyeX;
	input->green_cross_y = g_nLeftEyeY;

	fclose(pFile);

	// Reduzir a imagem
	nInd = 0;
	nAux1 = (FILE_WIDTH * FILE_HEIGHT) * 3;
	for (i = 0; i < nAux1; i++)
	{
		nCol = i % FILE_WIDTH;
		nLin = (i / FILE_WIDTH) % FILE_HEIGHT;

		nAuxCol = nCol % nFactor;
		nAuxLin = nLin % nFactor;

		if ((nAuxCol == 0) && (nAuxLin == 0))
		{
			g_pImageRAWReduced[nInd++] = g_pImageRAW[i];
		}
		else
		{
		}
	}

	// Transformar RAW -> Formato que a gente entende!!! :)
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

	// Colocar a imagem na MAE
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

	g_nLeftEyeX = 0;
	g_nLeftEyeY = 0;
	g_nRightEyeX = 0;
	g_nRightEyeY = 0;
	g_nNoseX = 0;
	g_nNoseY = 0;
	g_nMouthX = 0;
	g_nMouthY = 0;

	g_nPoseID = POSE_MIN;
	g_nPersonID = MIN_PERSON_ID;
	g_nFacePart = EYE;
	g_nStatus = MOVING_PHASE;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	g_nNoOne = 0;
	
	make_input_image_face_recog (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Le a primeira imagem
	if (!GetNextFileName(strFileName, DIRECTION_FORWARD))
	{
		ReadFaceInput(input, strFileName);
	}

	input->green_cross = 1;
	input->green_cross_x = g_nLeftEyeX;
	input->green_cross_y = g_nLeftEyeY;
	g_nPos = 0;
		
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
		g_nPoseID++;
	else
		g_nPoseID--;


	while (GetNextFileName(strFileName, nDirection))
		;

	if (ReadFaceInput(input, strFileName))
		return (-1);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
	update_input_image (input);

	input->green_cross_x = g_nLeftEyeX;
	input->green_cross_y = g_nLeftEyeY;
	g_nPos = 0;
	
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
#ifndef NO_INTERFACE
	if (input->win == 0)
#else
	if ((input->win == 0) && (status != MOVE))
#endif
	{
		init_face_recog(input);
	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewFace (input, DIRECTION_REWIND);
			else if (input->wxd >= IMAGE_WIDTH)
				GetNewFace (input, DIRECTION_FORWARD);

			LOG_POLAR_SCALE_FACTOR = sqrtf ((float) ((g_nLeftEyeX - g_nRightEyeX) * (g_nLeftEyeX - g_nRightEyeX) + 
					         	(g_nLeftEyeY - g_nRightEyeY) * (g_nLeftEyeY - g_nRightEyeY))) /
						 	(EYE_BASELINE / FOV_HORIZONTAL * input->neuron_layer->dimentions.x);

			LOG_POLAR_THETA_CORRECTION = (((g_nLeftEyeX + g_nRightEyeX) / 2 - input->wxd) != 0) ? 
					      		-atanf ((float) ((g_nLeftEyeX + g_nRightEyeX) / 2 - input->wxd) / 
						      		(float) ((g_nLeftEyeY + g_nRightEyeY) / 2 - input->wyd)) : 0.0;

			printf ("LOG_POLAR_SCALE_FACTOR = %f [], LOG_POLAR_THETA_CORRECTION = %f [deg]\n", LOG_POLAR_SCALE_FACTOR, 180.0f / M_PI * LOG_POLAR_THETA_CORRECTION);
			
			filter = get_filter_by_output(out_face_recog_lp.neuron_layer);
			filter_update(filter);
			output_update(&out_face_recog_lp);
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

	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, g_nPersonUniqueID - 1);

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

	for (i = 0; i < MAX_MAN_ID; i++)
		g_vCountPerson[i] = 0;
		
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < MAX_MAN_ID))
			g_vCountPerson[neuron_vector[i].output.ival] += 1;
	}

	for (i = 0; i < MAX_MAN_ID; i++)
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

	for (i = 0; i < MAX_MAN_ID; i++)
		g_vCountPerson[i] = 0; // este vetor �global apenas para evitar aloca�o din�ica ou algo mais inteligente
		
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < MAX_MAN_ID))
			g_vCountPerson[neuron_vector[i].output.ival] += 1;
	}

	for (i = 0; i < MAX_MAN_ID; i++)
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
			
			if ((nPersonOutput = EvaluateOutput(output) + 1) == g_nPersonUniqueID)
			{
				g_nCorrect++;
				printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));
			}
			else
				printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f ***\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));	
		}
	}
#ifndef NO_INTERFACE	
	glutSetWindow (output->win);
	output_display (output);
#endif
}


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
				person = EvaluateOutput2(output, &confidence) + 1;
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

				if ((nPersonOutput) == g_nPersonUniqueID)
				{
					g_nCorrect++;
					printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));
				}
				else
				{
					printf("Person: %3d - Correct Person: %3d - Tested: %3d - Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f ***\n", nPersonOutput, g_nPersonUniqueID, g_nTotalTested, g_nCorrect, best_confidence, (float)(g_nCorrect * 100.0 / g_nTotalTested));	
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



/*
***********************************************************
* Function: GetRandomFace
* Description: Gets a new face
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetRandomFaceN (int nRandomFace)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	FILE *pFile = NULL;
	int i;
	
	if ((pFile = fopen (g_strRandomFacesFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s' (GetRandomFace).\n", g_strRandomFacesFileName);
		output.ival = -1;
		return (output);
	}
	
	for (i = 0; i < nRandomFace; i++)
		fscanf (pFile, "%d %c %d %d\n", &g_nPersonUniqueID, &g_cSexID, &g_nPersonID, &g_nPoseID);
	fclose (pFile);
	
	if ((g_cSexID != 'm') && (g_cSexID != 'w'))
	{
		printf ("Error: Invalid sex ID, it must be 0 for man or 1 for woman (GetRandomFace).\n");
		output.ival = -1;
		return (output);
	}
	
	g_nMaxPersonID = (g_cSexID == 'm') ? MAX_MAN_ID : MAX_WOMAN_ID;
	if ((g_nPersonID < MIN_PERSON_ID) || (g_nPersonID > g_nMaxPersonID))
	{
		printf ("Error: Invalid person ID, it must be within the interval [%d, %d] (GetRandomFace).\n", MIN_PERSON_ID, g_nMaxPersonID);
		output.ival = -1;
		return (output);
	}
	
	if ((g_nPoseID < POSE_MIN) || (g_nPoseID > POSE_MAX))
	{
		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetRandomFace).\n", POSE_MIN, POSE_MAX);
		output.ival = -1;
		return (output);
	}
	
	sprintf(strFileName, "%s%c-%03d-%d.raw", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);
	printf("FileName: %s\n", strFileName);
	fflush(stdout);
	
	if (ReadFaceInput (&face_recog, strFileName))
	{
		printf ("Error: Cannot read face (GetRandomFace).\n");
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
		case 'N':
		case 'n':
			SaveGroundTruths ();
			if (g_nStatus == TRAINING_PHASE)
			{
				g_nStatus = RECALL_PHASE;
			}
			else
			{
				g_nStatus = TRAINING_PHASE;
				g_nFaceNum++;
			}
			if (g_nFaceNum >= 351)
			{
				g_nFaceNum = 1;
				g_nStatus = TRAINING_PHASE;
			}
			SetNetworkStatusN (g_nStatus);
			GetRandomFaceN (g_nFaceNum);
			g_nPos = 0;
			break;
		// Moves the input to the next previuos photo
		case 'B':
		case 'b':
			if (g_nStatus == TRAINING_PHASE)
			{
				g_nStatus = RECALL_PHASE;
				g_nFaceNum--;
			}
			else
			{
				g_nStatus = TRAINING_PHASE;
			}
			if (g_nFaceNum <= 0)
			{
				g_nFaceNum = 350;
				g_nStatus = RECALL_PHASE;
			}
			SetNetworkStatusN (g_nStatus);
			GetRandomFaceN (g_nFaceNum);
			g_nPos = 0;
			break;
		// Set target to right eye
		case 'Q':
		case 'q':
			g_nPos = 0;
			if (g_nRightEyeX == g_nLeftEyeX)
			{
				g_nRightEyeX -= 57;
			}
			face_recog.wxd = face_recog.green_cross_x = g_nRightEyeX;
			face_recog.wyd = face_recog.green_cross_y = g_nRightEyeY;
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Set target to left eye
		case 'W':
		case 'w':
			g_nPos = 1;
			face_recog.wxd = face_recog.green_cross_x = g_nLeftEyeX;
			face_recog.wyd = face_recog.green_cross_y = g_nLeftEyeY;
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Set target to nose
		case 'E':
		case 'e':
			g_nPos = 2;
			face_recog.wxd = face_recog.green_cross_x = g_nNoseX;
			face_recog.wyd = face_recog.green_cross_y = g_nNoseY;
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Set target to mouth
		case 'R':
		case 'r':
			g_nPos = 3;
			face_recog.wxd = face_recog.green_cross_x = g_nMouthX;
			face_recog.wyd = face_recog.green_cross_y = g_nMouthY;
			glutPostWindowRedisplay (face_recog.win);
			break;
		// Switch to next target
		case 'Z':
		case 'z':
			if (g_nPos == 0)
			{
				face_recog.wxd = face_recog.green_cross_x = g_nLeftEyeX;
				face_recog.wyd = face_recog.green_cross_y = g_nLeftEyeY;
				g_nPos = 1;
			}
			else if (g_nPos == 1)
			{
				face_recog.wxd = face_recog.green_cross_x = g_nNoseX;
				face_recog.wyd = face_recog.green_cross_y = g_nNoseY;
				g_nPos = 2;
			}
			else if (g_nPos == 2)
			{
				face_recog.wxd = face_recog.green_cross_x = g_nMouthX;
				face_recog.wyd = face_recog.green_cross_y = g_nMouthY;
				g_nPos = 3;
			}
			else
			{
				face_recog.wxd = face_recog.green_cross_x = g_nRightEyeX;
				face_recog.wyd = face_recog.green_cross_y = g_nRightEyeY;
				g_nPos = 0;
			}
			glutPostWindowRedisplay (face_recog.win);
			break;
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
		// Moves the input 14 photos ahead
		case 'X':
		case 'x':
			g_nStatus = TRAINING_PHASE;
			g_nFaceNum += 10;
			if (g_nFaceNum >= 351)
			{
				show_message ("Voltando para a ultima face", "", "");
				g_nFaceNum = 350;
				g_nStatus = RECALL_PHASE;
			}
			SetNetworkStatusN (g_nStatus);
			GetRandomFaceN (g_nFaceNum);
			g_nPos = 0;
			break;
	}
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
	printf("FileName: %s\n", strFileName);
	fflush(stdout);
	
	if (ReadFaceInput (&face_recog, strFileName))
	{
		printf ("Error: Cannot read face (GetFace).\n");
		output.ival = -1;
		return (output);
	}	
	
	check_input_bounds (&face_recog, face_recog.wx + face_recog.ww/2, face_recog.wy + face_recog.wh/2);
	face_recog.up2date = 0;
	update_input_neurons (&face_recog);
	update_input_image (&face_recog);

	g_nPersonUniqueID = g_nPersonID * 2 - ((g_cSexID == 'w') ? 0 : 1);
	printf ("g_nPersonID = %d, g_cSexID = %d, g_nPersonUniqueID = %d\n", g_nPersonID, g_cSexID, g_nPersonUniqueID);
	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: Move2Eye
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT Move2Eye (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nFacePart = EYE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nLeftEyeX;
	face_recog.wyd = g_nLeftEyeY;
	move_input_window (face_recog.name, face_recog.wxd, face_recog.wyd);
	
	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: Move2Nose
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT Move2Nose (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nFacePart = NOSE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nNoseX;
	face_recog.wyd = g_nNoseY;
	move_input_window (face_recog.name, face_recog.wxd, face_recog.wyd);
	
	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: Move2Nose2
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT Move2Nose2 (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int x_disp;
	int y_disp;

	x_disp = pParamList->next->param.ival;
	y_disp = pParamList->next->next->param.ival;

	g_nFacePart = NOSE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nNoseX + x_disp;
	face_recog.wyd = g_nNoseY + y_disp;
	move_input_window (face_recog.name, face_recog.wxd, face_recog.wyd);
	
	output.ival = 0;
	return (output);
}
	


/*
***********************************************************
* Function: Move2Mouth
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT Move2Mouth (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nFacePart = MOUTH;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nMouthX;
	face_recog.wyd = g_nMouthY;
	move_input_window (face_recog.name, face_recog.wxd, face_recog.wyd);
	
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

	printf("%3.2f\n", (float) (g_nCorrect * 100.0 / g_nTotalTested));

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
		printf ("Error: cannot open file '%s' (GetRandomFace).\n", g_strRandomFacesFileName);
		output.ival = -1;
		return (output);
	}
	
	for (i = 0; i < nRandomFace; i++)
		fscanf (pFile, "%d %c %d %d\n", &g_nPersonUniqueID, &g_cSexID, &g_nPersonID, &g_nPoseID);
	fclose (pFile);
	
	if ((g_cSexID != 'm') && (g_cSexID != 'w'))
	{
		printf ("Error: Invalid sex ID, it must be 0 for man or 1 for woman (GetRandomFace).\n");
		output.ival = -1;
		return (output);
	}
	
	g_nMaxPersonID = (g_cSexID == 'm') ? MAX_MAN_ID : MAX_WOMAN_ID;
	if ((g_nPersonID < MIN_PERSON_ID) || (g_nPersonID > g_nMaxPersonID))
	{
		printf ("Error: Invalid person ID, it must be within the interval [%d, %d] (GetRandomFace).\n", MIN_PERSON_ID, g_nMaxPersonID);
		output.ival = -1;
		return (output);
	}
	
	if ((g_nPoseID < POSE_MIN) || (g_nPoseID > POSE_MAX))
	{
		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetRandomFace).\n", POSE_MIN, POSE_MAX);
		output.ival = -1;
		return (output);
	}
	
	sprintf(strFileName, "%s%c-%03d-%d.raw", INPUT_PATH, g_cSexID, g_nPersonID, g_nPoseID);
	printf("FileName: %s\n", strFileName);
	fflush(stdout);
	
	if (ReadFaceInput (&face_recog, strFileName))
	{
		printf ("Error: Cannot read face (GetRandomFace).\n");
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
