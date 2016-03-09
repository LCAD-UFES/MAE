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

#define INPUT_PATH		"/home/hallysson/img/ARDataBase/ARPhotoDataBase/"

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
int g_nFacePart;
int g_nStatus;
int g_nPos;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;
int g_nNoOne;
int g_nEyeCorrect;
int g_nNoseCorrect;
int g_nMouthCorrect;

// Imagem
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

// Coordenadas X,Y das partes da face: olho, nariz e boca
int g_nEyeX, g_nEyeY;
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
		g_nEyeX = g_nEyeY = 0;
		g_nNoseX = g_nNoseY = 0;
		g_nMouthX = g_nMouthY =0;
		return (-1);
	}
	
	fscanf (pFile, "left_eye_coordinates=%d %d\n", &g_nEyeX, &g_nEyeY);
	fscanf (pFile, "nose_coordinates=%d %d\n", &g_nNoseX, &g_nNoseY);
	fscanf (pFile, "mouth_coordinates=%d %d\n", &g_nMouthX, &g_nMouthY);
	
	fclose (pFile);

	printf("Eye:[%03d,%03d] - Nose:[%03d,%03d] - Mouth:[%03d,%03d]\n", g_nEyeX, g_nEyeY, g_nNoseX, g_nNoseY, g_nMouthX, g_nMouthY);
	
	return (0);
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
	input->green_cross_x = g_nEyeX;
	input->green_cross_y = g_nEyeY;

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
	int x, y;
	char strFileName[128];

	g_nEyeX = 0;
	g_nEyeY = 0;
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
	g_nEyeCorrect = 0;
	g_nNoseCorrect = 0;
	g_nMouthCorrect = 0;
	
	make_input_image_face_recog (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Le a primeira imagem
	if (!GetNextFileName(strFileName, DIRECTION_FORWARD))
	{
		ReadFaceInput(input, strFileName);
	}

	input->green_cross = 1;
	input->green_cross_x = g_nEyeX;
	input->green_cross_y = g_nEyeY;
	g_nPos = 0;
		
	input->up2date = 0;
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

	input->green_cross_x = g_nEyeX;
	input->green_cross_y = g_nEyeY;
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
	if (input->win == 0)
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

			switch (g_nFacePart)
			{
				/*case EYE:
				{
					filter = get_filter_by_output(out_eye_v1.neuron_layer);
					filter_update(filter);
					output_update(&out_eye_v1);
					filter = get_filter_by_output(out_eye_h1.neuron_layer);
					filter_update(filter);
					output_update(&out_eye_h1);
				}
				break;*/

				case NOSE:
				{
					filter = get_filter_by_output(out_nose_v1.neuron_layer);
					filter_update(filter);
					output_update(&out_nose_v1);
					filter = get_filter_by_output(out_nose_h1.neuron_layer);
					filter_update(filter);
					output_update(&out_nose_h1);
				}
				break;

				/*case MOUTH:
				{
					filter = get_filter_by_output(out_mouth_v1.neuron_layer);
					filter_update(filter);
					output_update(&out_mouth_v1);
					filter = get_filter_by_output(out_mouth_h1.neuron_layer);
					filter_update(filter);
					output_update(&out_mouth_h1);
				}
				break;*/

				/*case VIEW_LOG_POLAR:
				{
					filter = get_filter_by_output(out_face_recog_lp.neuron_layer);
					filter_update(filter);
					output_update(&out_face_recog_lp);
				}
				break;*/
			}
			
			glutSetWindow (input->win);
			input_display ();
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
	int nAux1, nAux2;
	OUTPUT_DESC *output;

	output = get_output_by_name (strOutputName);

	nAux1 = OUT_NEURONS_BY_PERSON * (g_nPersonUniqueID - 1);
	nAux2 = (output->wh * output->ww) - (nAux1 + OUT_NEURONS_BY_PERSON);

	set_neurons (output->neuron_layer->neuron_vector, 0, nAux1, 0);
	set_neurons (output->neuron_layer->neuron_vector, nAux1, OUT_NEURONS_BY_PERSON, (NUM_COLORS - 1));
	set_neurons (output->neuron_layer->neuron_vector, nAux1 + OUT_NEURONS_BY_PERSON, nAux2, 0);

	update_output_image (output);
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
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
		/*sprintf (strCommand, "draw out_eye based on face_recog move;");
		interpreter (strCommand);*/
		sprintf (strCommand, "draw out_nose based on face_recog move;");
		interpreter (strCommand);
		/*sprintf (strCommand, "draw out_mouth based on face_recog move;");
		interpreter (strCommand);*/
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
	int i, j;
	int nPersonID = 0;
	int nIdxStartNeuron;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	// Percorre a camada de saida (output), pessoa por pessoa, contando
	// o numero de neuronios setados. A faixa de pessoa da camada de saida
	// que tiver o maior numero de neuronios setados, sera considerada
	// como o valor da camada de saida.
	for (i = 0; i < ((output->wh * output->ww) / OUT_NEURONS_BY_PERSON); i++)
	{
		nAux = 0;
		nIdxStartNeuron = OUT_NEURONS_BY_PERSON * i;

		// Conta a quantidade de neuronios setados na faixa
		for (j = nIdxStartNeuron; j < (nIdxStartNeuron + OUT_NEURONS_BY_PERSON); j++)
		{
			if (neuron_vector[j].output.ival != 0)
				nAux++;
		}

		// Verifica se e a faixa com mais neuronios setados ate agora
		// Se for, seta a pessoa e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			nPersonID = i;
		}
	}

	return (nPersonID);
}



/*
***********************************************************
* Function: output_handler
* Description:
* Inputs:
* Output: none
***********************************************************
*/

/*void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	static int nEyeOutput;
	static int nNoseOutput;
	static int nMouthOutput;
	static int nFlagEye = 0;
	static int nFlagNose = 0;
	static int nFlagMouth = 0;
	int nPersonOutput;
	char strAux[3];

	if (g_nStatus == RECALL_PHASE)
	{
		// Avalia a saida
		if (strcmp(output->name, out_eye.name) == 0)
		{
			nEyeOutput = EvaluateOutput(output);
			nFlagEye = 1;
		}
		else if (strcmp(output->name, out_nose.name) == 0)
		{
			nNoseOutput = EvaluateOutput(output);
			nFlagNose = 1;
		}
		else if (strcmp(output->name, out_mouth.name) == 0)
		{
			nMouthOutput = EvaluateOutput(output);
			nFlagMouth = 1;
		}

		// Se todas as saidas ja foram avalidas, determina qual eh a pessoa
		if (nFlagEye && nFlagNose && nFlagMouth)
		{
			g_nTotalTested++;
			nFlagEye = 0;
			nFlagNose = 0;
			nFlagMouth = 0;

			// Todas as 3 saidas decidiram a mesma pessoa
			if ((nEyeOutput == nNoseOutput) && (nEyeOutput == nMouthOutput)) // Todos iguais
			{
				nPersonOutput = nEyeOutput;
				g_nEyeCorrect++;
				g_nNoseCorrect++;
				g_nMouthCorrect++;
			}
			else
			{
				// 2 saidas iguais
				if (nEyeOutput == nNoseOutput)
				{
					nPersonOutput = nEyeOutput;
					g_nEyeCorrect++;
					g_nNoseCorrect++;
				}
				else if (nEyeOutput == nMouthOutput)
				{
					nPersonOutput = nEyeOutput;
					g_nEyeCorrect++;
					g_nMouthCorrect++;
				}
				else if (nNoseOutput == nMouthOutput)
				{
					nPersonOutput = nNoseOutput;
					g_nNoseCorrect++;
					g_nMouthCorrect++;
				}
				else
				{
					// Todos os 3 diferentes --> Indecisao
					nPersonOutput = 0;
				}
			}

			// Calcula o percentual de correcao
			// (acertou a pessoa) ou (disse que esta pessoa eh desconhecida sendo que ela realmente nao foi treinada)
			if ((nNoseOutput == g_nPersonID) ||
				((nPersonOutput == 0) && (g_nPersonID > g_nMaxPersonID)))
			{
				g_nCorrect++;
				strcpy(strAux,"");
			}
			else
			{
				strcpy(strAux,"***");
			}

			printf("Person: %3d - Correct Person: %3d - Eye: %3d - Nose: %3d - Mouth: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f %s\n", nPersonOutput, g_nPersonID, nEyeOutput, nNoseOutput, nMouthOutput, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested), strAux);
		}
	}
	glutSetWindow (output->win);
	output_display (output);
}*/


void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int nPersonOutput;

	if (g_nStatus == RECALL_PHASE)
	{
		// Avalia a saida
		if (strcmp (output->name, out_nose.name) == 0)
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
	
	glutSetWindow (output->win);
	output_display (output);
}



// ----------------------------------------------------
// TrainOneFace - Rotina para automatizar o treinamento
//		de uma imagem da face de uma pessoa
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------

/*void TrainOneFace()
{
	char strCommand[128];

	g_nStatus = TRAINING_PHASE;

	// olho (zoio)
	g_nFacePart = EYE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nEyeX;
	face_recog.wyd = g_nEyeY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// nariz (fuca)
	g_nFacePart = NOSE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nNoseX;
	face_recog.wyd = g_nNoseY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// boca (cacapa)
	g_nFacePart = MOUTH;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nMouthX;
	face_recog.wyd = g_nMouthY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// Treina
	interpreter("draw out_eye based on face_recog move;");
	interpreter("draw out_nose based on face_recog move;");
	interpreter("draw out_mouth based on face_recog move;");
	interpreter("train network;");

	glutIdleFunc ((void (* ) (void)) check_forms);

	g_nStatus = MOVING_PHASE;
}*/

// ----------------------------------------------------
// TrainOnePerson - Rotina para automatizar o treinamento
//					das imagens de uma pessoa
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void TrainOnePerson()
{
	// Treina as faces 2, 3 e 4
	g_nPoseID = 1;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();

	// Treina as faces 7 e 8
	g_nPoseID = 6;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();

	// Treina a face 11
	g_nPoseID = 10;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TrainOneFace();

	glutIdleFunc ((void (* ) (void))check_forms);
}*/

// ----------------------------------------------------
// TrainAll - Treina todas as amostras
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void TrainAll()
{
	int i;

	for (i = 1; i <= g_nMaxPersonID; i++)
	{
		g_nPersonID = i;
		TrainOnePerson();
	}
}*/

// ----------------------------------------------------
// TestOneFace - Rotina para automatizar o teste de uma
//				 face.
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void TestOneFace()
{
	char strCommand[128];

	g_nStatus = RECALL_PHASE;

	// olho (zoio)
	g_nFacePart = EYE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nEyeX;
	face_recog.wyd = g_nEyeY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// nariz (fuca)
	g_nFacePart = NOSE;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nNoseX;
	face_recog.wyd = g_nNoseY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// boca (cacapa)
	g_nFacePart = MOUTH;
	face_recog.wxd_old = face_recog.wxd;
	face_recog.wyd_old = face_recog.wyd;
	face_recog.wxd = g_nMouthX;
	face_recog.wyd = g_nMouthY;
	sprintf (strCommand, "move %s to %d, %d;", face_recog.name, face_recog.wxd, face_recog.wyd);
	interpreter (strCommand);

	// Treina
	interpreter("forward neural_layers;");
	interpreter("forward outputs;");

	glutIdleFunc ((void (* ) (void))check_forms);

	g_nStatus = MOVING_PHASE;
}*/

// ----------------------------------------------------
// TestOnePerson - Rotina para automatizar o teste de
//				   uma pessoa
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void TestOnePerson()
{
	// Testa a face 1
	g_nPoseID = 2;
	GetNewFace (&face_recog, DIRECTION_REWIND);
	TestOneFace();

	// Testa as faces 5 e 6
	g_nPoseID = 4;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();

	// Testa as faces 9 e 10
	g_nPoseID = 8;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();

	// Testa as faces 12 e 13
	g_nPoseID = 11;
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();
	GetNewFace (&face_recog, DIRECTION_FORWARD);
	TestOneFace();

	glutIdleFunc ((void (* ) (void))check_forms);
}*/

// ----------------------------------------------------
// TestAll - Testa todas as amostras nao treinadas
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void TestAll()
{
	int i;

	for (i = 1; i <= g_nMaxPersonID; i++)
	{
		g_nPersonID = i;
		TestOnePerson();
	}
}*/

// ----------------------------------------------------
// PrintResult - Imprime o resultado dos testes
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void PrintResult()
{
//	printf("------------------------------------------------------------\n");
//	printf("Tested: %3d - Correct: %3d - Hit Percent: %3.2f\n", g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));
//	printf("Hits by face part --> Eye: %3d - Nose: %3d - Mouth: %3d\n", g_nEyeCorrect, g_nNoseCorrect, g_nMouthCorrect);
	printf("%3.2f\n", (float) (g_nCorrect * 100.0 / g_nTotalTested));
}*/

// ----------------------------------------------------
// Go - Vai, Faz ae...!!!!
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------
/*void Go()
{
	TrainAll();
	TestAll();
	PrintResult();
}*/




/*
***********************************************************
* Function: f_keyboard
* Description: handles the keyboard input window events
* Inputs:key_value
* Output: none
***********************************************************
*/

void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	
	switch (key)
	{
		// Man
		case 'M':
		case 'm':
			g_cSexID = 'm';
			g_nMaxPersonID = MAX_MAN_ID;
			break;
		// Woman
		case 'W':
		case 'w':
			g_cSexID = 'w';
			g_nMaxPersonID = MAX_WOMAN_ID;
			break;
		// Treina uma pessoa
		/*case 'U':
		case 'u':
		{
			glutIdleFunc ((void (* ) (void))TrainOnePerson);
		}
		break;

		// Treina uma face
		case 'I':
		case 'i':
		{
			glutIdleFunc ((void (* ) (void))TrainOneFace);
		}
		break;

		// Testa uma face
		case 'O':
		case 'o':
		{
			glutIdleFunc ((void (* ) (void))TestOneFace);
		}
		break;

		// Testa uma pessoa
		case 'P':
		case 'p':
		{
			glutIdleFunc ((void (* ) (void))TestOnePerson);
		}
		break;*/
		// Move para a ultima imagem da pessoa anterior
		case 'A':
		case 'a':
			g_nPoseID = POSE_MIN;
			GetNewFace (&face_recog, DIRECTION_REWIND);
			glutIdleFunc ((void (* ) (void))check_forms);
			break;
		// Move para a primeira imagem da próxima pessoa
		case 'S':
		case 's':
			g_nPoseID = POSE_MAX;
			GetNewFace (&face_recog, DIRECTION_FORWARD);
			glutIdleFunc ((void (* ) (void))check_forms);
			break;
		// Reseta os contadores de teste de face
		case 'R':
		case 'r':
			g_nTotalTested = 0;
			g_nCorrect = 0;
			g_nEyeCorrect = 0;
			g_nNoseCorrect = 0;
			g_nMouthCorrect = 0;
			break;
		// Treina todas as amostras
		/*case 'T':
		case 't':
		{
			glutIdleFunc ((void (* ) (void))TrainAll);
		}
		break;

		// Testa todas as amostras nao treinadas
		case 'Y':
		case 'y':
		{
			glutIdleFunc ((void (* ) (void))TestAll);
		}
		break;

		// Vai Vei!!!
		case 'V':
		case 'v':
		{
			glutIdleFunc ((void (* ) (void))Go);
		}
		break;*/
		case 'Z':
		case 'z':
			if (g_nPos == 0)
			{
				face_recog.green_cross_x = g_nNoseX;
				face_recog.green_cross_y = g_nNoseY;
				g_nPos = 1;
			}
			else if (g_nPos == 1)
			{
				face_recog.green_cross_x = g_nMouthX;
				face_recog.green_cross_y = g_nMouthY;
				g_nPos = 2;
			}
			else
			{
				face_recog.green_cross_x = g_nEyeX;
				face_recog.green_cross_y = g_nEyeY;
				g_nPos = 0;
			}
			glutPostWindowRedisplay (face_recog.win);
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
	face_recog.wxd = g_nEyeX;
	face_recog.wyd = g_nEyeY;
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
	g_nEyeCorrect = 0;
	g_nNoseCorrect = 0;
	g_nMouthCorrect = 0;

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
