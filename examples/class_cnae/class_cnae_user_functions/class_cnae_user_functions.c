#include <stdlib.h>
#include "mae.h"
#include "../class_cnae.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0
#define DOC_MIN			1
#define DOC_MAX			3264

#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define IMAGE_WIDTH		384
#define IMAGE_HEIGHT		288

#define LINEAR_MODE		1
#define RAMDOM_MODE		2


// Variaveis globais
int g_nDocNumber;
int g_nDataBaseId = 0;
int g_nStatus;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;


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
	int test = 0;
	
	if(nDirection != NO_DIRECTION)
	{
	
		test = (nDirection == DIRECTION_FORWARD)? 1:0;

		switch(g_nDocNumber)
		{
			case DOC_MAX:	if (test)
						g_nDocNumber = DOC_MIN;
					else
						g_nDocNumber--;
					break;

			case DOC_MIN:	if (test)
						g_nDocNumber++;  
					else
						g_nDocNumber = DOC_MAX;
					break;

			default :	if (test)  /* (DOC_MIN,DOC_MAX) */
						g_nDocNumber++;  
					else
						g_nDocNumber--;
					break;
		}
	}
	
	if (g_nStatus == TRAINING_PHASE)
		sprintf (strFileName, "%s%d.pnm", CNAE_TABLE_PATH, g_nDocNumber);
	else
		sprintf (strFileName, "%s%d.pnm", OBJETOS_SOCIAIS_PATH, g_nDocNumber);
	
	printf ("FileName: %s\n", strFileName);
	fflush (stdout);

	if ((pFile = fopen (strFileName, "r")) != NULL)
	{
		fclose (pFile);
		return (0);
	}
	else // O arquivo especificado nao existe
	{	strcpy (strFileName, "");
		return (-1);
	}
}



/*
***********************************************************
* Function: ReadDocumentInput
* Description: Reads a PNM ASCII image to the MAE input
* Inputs: input - input image
*	  strFileName - Image file name
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int ReadDocumentInput(INPUT_DESC *input, char *strFileName)
{
	load_input_image (input, strFileName);	

	return (0);
}



/*
***********************************************************
* Function: make_input_image_class_cnae
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void make_input_image_class_cnae (INPUT_DESC *input, int w, int h)
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
* Function: init_class_cnae
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_class_cnae(INPUT_DESC *input)
{
	int x, y;
	char strFileName[128];

	g_nDocNumber = DOC_MIN;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	
	g_nStatus = TRAINING_PHASE;
	
	make_input_image_class_cnae (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Le a primeira imagem
	if (!GetNextFileName(strFileName, NO_DIRECTION))
	{
		ReadDocumentInput(input, strFileName);
	}
		
	input->up2date = 0;

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
* Function: GetNewDocument
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int GetNewDocument(INPUT_DESC *input, int nDirection)
{
	char strFileName[128];

	while (GetNextFileName(strFileName, nDirection))
		;

	if (ReadDocumentInput(input, strFileName))
		return (-1);
			
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);

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
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_class_cnae(input);
	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewDocument (input, DIRECTION_REWIND);
			else if (input->wxd >= IN_WIDTH)                   /*BUG => BEFORE: IMAGE_WIDTH, NOW: IN_WIDTH */
				GetNewDocument (input, DIRECTION_FORWARD);
			
			output_update(&nl_class_cnae_out);
			
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
	OUTPUT_DESC *output;
	char strFileName[256];
	FILE *pFile = NULL;
	int class_cnae;

	output = get_output_by_name (strOutputName);

	sprintf (strFileName, "%s%d.cnae", (g_nDataBaseId == CNAE_TABLE_ID)? CNAE_TABLE_PATH: OBJETOS_SOCIAIS_PATH, g_nDocNumber);

	if ((pFile = fopen (strFileName, "r")) != NULL)
	{
		fscanf (pFile, "%d", &class_cnae);
		printf ("class_cnae = %d\n", class_cnae);
		fclose (pFile);
	}
	else
	{
		printf ("Could not read file %s\n", strFileName);
		return;
	}
	
	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, class_cnae);

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
		sprintf (strCommand, "draw nl_class_cnae_out based on class_cnae move;");
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
	int i, j;
	int nClassAux,nClassID = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de classes. A classe que tiver o maior numero de neuronios 
	// setados sera considerada como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		
		nClassAux = neuron_vector[i].output.ival;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.ival == nClassAux)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			nClassID = nClassAux;
		}
	}
	
	return (nClassID);
}



/*
***********************************************************
* Function: output_handler
* Description: handles the mouse output window events 
* Inputs: output -
*	  type_call -
*	  mouse_button -
*	  mouse_state -
* Output: None
***********************************************************
*/

void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char strFileName[256];
	FILE *pFile = NULL;
	int nClassCnaeOutput;
	int nClassCnaeID;
	int test = 0;
	

	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		nClassCnaeOutput = EvaluateOutput(output);
		
		g_nTotalTested++;
	
		sprintf (strFileName, "%s%d.cnae", OBJETOS_SOCIAIS_PATH, g_nDocNumber);

		if ((pFile = fopen (strFileName, "r")) != NULL)
		{
			while(!feof(pFile))
			{
				fscanf (pFile, "%d", &nClassCnaeID);
				
				// Compara a saida da rede com a classe real
				if(nClassCnaeOutput == nClassCnaeID)
				{
					test = 1;
					break;
				}
				break;
			}
			fclose (pFile);
		}
		else
		{
			printf ("Could not read file %s\n", strFileName);
			return;
		}
		
		if (test)
		{
			g_nCorrect++;
			printf("Class: %3d - Correct Class: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f\n", nClassCnaeOutput, nClassCnaeID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));
		}
		else
		{
			printf("Class: %3d - Correct Class: %3d - Tested: %3d - Correct: %3d - Hit Percent: %3.2f ***\n", nClassCnaeOutput, nClassCnaeID, g_nTotalTested, g_nCorrect, (float)(g_nCorrect * 100.0 / g_nTotalTested));
			// printf("ERROR in the file: %s\n",strFileName);
		}
				
	}
		
	glutSetWindow (output->win);
	output_display (output);
}




/*
***********************************************************
* Function: f_keyboard
* Description: Funcao chamada quando eh pressionada uma tecla 
* Inputs: key_value -
* Output: None
***********************************************************
*/

void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
		case 'N':
		case 'n':
			if (g_nStatus == TRAINING_PHASE)
			{
				g_nStatus = RECALL_PHASE;
			}
			else
			{
				g_nStatus = TRAINING_PHASE;
			}
			break;
	}
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

	//printf("%3.2f\n", (float) (g_nCorrect * 100.0 / g_nTotalTested));

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

	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: GetDoc
* Description: Gets a document
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetDoc (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	int nDoc;
	int mode;
	int nDirection;
	int first_access; 

	nDoc = pParamList->next->param.ival;
	mode = pParamList->next->next->param.ival;
	first_access = pParamList->next->next->next->param.ival;
	
	if(mode == LINEAR_MODE)
	{
		if(first_access)
		{
			g_nDocNumber = nDoc;
			nDirection = NO_DIRECTION;
		}
		else
			nDirection = DIRECTION_FORWARD;
	}
	else    //RANDOM_MODE
	{
		g_nDocNumber = (random() % DOC_MAX) + 1;
		nDirection = NO_DIRECTION;
	}
	
	while (GetNextFileName(strFileName, nDirection))
		;
	
	if (ReadDocumentInput(&document, strFileName))
	{
		printf ("Error: Cannot read document (GetDoc).\n");
		output.ival = -1;
		return (output);
	}
	
	update_input_image (&document);
	check_input_bounds (&document, document.wx + document.ww/2, document.wy + document.wh/2);

	output.ival = 0;
	
	return (output);
}



/*
***********************************************************
* Function: GetDoc
* Description: Gets a document
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetItem (PARAM_LIST *pParamList)
{
	char file_name[256];
	FILE *File;
	NEURON_OUTPUT output;
	int item_number, data_base;

	item_number = pParamList->next->param.ival;
	data_base = pParamList->next->next->param.ival;

	sprintf (file_name, "%s%d.pnm", (data_base == CNAE_TABLE_ID)? CNAE_TABLE_PATH: OBJETOS_SOCIAIS_PATH, item_number);
	
	if ((File = fopen (file_name, "r")) == NULL)
	{
		fclose (File);
		show_message ("Could not get item", file_name, " in GetItem ()");
		output.ival = -1;
		return (output);
	}
	fclose (File);

	if (ReadDocumentInput(&document, file_name))
	{
		show_message ("Could not read document ", file_name, " in GetItem ()");
		output.ival = -1;
		return (output);
	}

	g_nDocNumber = item_number;
	g_nDataBaseId = data_base;
	printf ("FileName: %s\n", file_name);
	fflush (stdout);
	
	update_input_image (&document);
	check_input_bounds (&document, document.wx + document.ww/2, document.wy + document.wh/2);

	output.ival = 0;
	
	return (output);
}
