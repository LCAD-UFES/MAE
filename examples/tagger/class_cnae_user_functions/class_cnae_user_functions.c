#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#include "../class_cnae.h"
#include "db_core.h"
#include "jobs_data.h"
#include "io_memory.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define BEGINNING		1
#define END			2


// Variaveis globais
int g_nDocNumber;
int g_nDataBaseId = 0;
int g_nStatus;
int g_tag;
JOB_DATA *g_job_data = NULL;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;



int seek_ttv_file( FILE *table_file, int nDoc, int wo, int ho )
{
	int position = 0, line_size;
	
	TABLE *table;
	TTV_LINE *ttv;
	
	table = (TABLE*) malloc( sizeof( TABLE ) );
	ttv = (TTV_LINE*) malloc( sizeof( TTV_LINE ) );

	if (fread ((void *) table, sizeof (TABLE), 1, table_file) != 1)
	{
		fprintf (stderr, "Could not read table from table_file in discards_headers().\n");
		exit (1);
	}

	position += sizeof(TABLE);
	position += table->name_size * sizeof (char);
	position += table->columns_names_size * sizeof (char);
	position += table->columns_types_size * sizeof (char);
	position += table->associations_size * sizeof (char);

	//sizeof TTV structure + training_and_testing_vector + cnae_codes_vector;
	line_size = table->line_size + wo*ho*sizeof(float) + sizeof(int);
	position += nDoc*line_size + table->line_size;
	
	fseek( table_file, position,  SEEK_SET);
	
	free(table);

	return position;
}

float *ReadAndCompleteDocument( int nDoc, int wo, int ho )
{
	int i;
	float *ttv_vector, peso;
	char file_name[256];
	FILE *ttv;

	ttv_vector = (float*) malloc( wo*ho*sizeof(float) );
	
	if( g_nStatus == TRAINING_PHASE )
		sprintf( file_name, "%s", TRAIN_PATH );
	else
		sprintf( file_name, "%s", TEST_PATH );
	
	
	if( (ttv = fopen( file_name, "r" )) == NULL )
	{
		fprintf( stderr, "Couldn't not open %s for in mode.\n", file_name );
		exit(1);
	}
	
	seek_ttv_file( ttv, nDoc, wo, ho );
	
	for (i = 0; i<wo*ho; i++)
	{
		fread( (void*)&peso, sizeof(float), 1, ttv );
		ttv_vector[i] = peso;
	}
	
	// geting the class (tag) of the documet (word)
	fread( (void*)&g_tag, sizeof(int), 1, ttv );
	
	if( g_nStatus == RECALL_PHASE )
		complete_ttv( g_job_data, ttv_vector );
	
	fclose( ttv );
	
	return ttv_vector;
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
int ReadDocumentInput(INPUT_DESC *input, int nDoc)
{
	int xo, yo, wo, ho, i;
	NEURON *neuron_vector;
	float *ttv_vector;

	neuron_vector = input->neuron_layer->neuron_vector;
	wo = input->neuron_layer->dimentions.x;
	ho = input->neuron_layer->dimentions.y;
	ttv_vector = ReadAndCompleteDocument( nDoc, wo, ho );
	
	for (yo = 0, i = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++, i++)
		{
			neuron_vector[yo * wo + xo].output.fval = ttv_vector[i];
		}
	}

	free( ttv_vector );
	
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
	float f;

	g_nDocNumber = 0;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	
	g_nStatus = TRAINING_PHASE;
	
	make_input_image_class_cnae (input, IN_WIDTH, IN_HEIGHT);

	// Le o documento 0
	ReadDocumentInput(input, 0);
		
	input->up2date = 0;

	f = 1.0;
	while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
		f += 1.0;
	while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
		f *= 0.9;
	glutInitWindowSize ((int) ((float)input->ww * f), (int) ((float)input->wh * f));
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
	if (nDirection == DIRECTION_FORWARD)
		g_nDocNumber++;
	else
		g_nDocNumber--;

	// Rickson, checar os limites acima para a interface na imagem funcionar

	ReadDocumentInput(input, g_nDocNumber);
	
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
			else if (input->wxd >= IN_WIDTH)
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
	int class_cnae;

	class_cnae = g_tag;

	output = get_output_by_name (strOutputName);
	
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

int EvaluateOutput( OUTPUT_DESC *output )
{
	int i;
	int nClassID = 0;
	float nMax = 0;
	NEURON *neuron_vector;
	float *beliefs;
	JOB_DATA *job_data = g_job_data;
	// reusing the last belief vector
	beliefs = job_data->biliefs_vec->beliefs;
	
	neuron_vector = output->neuron_layer->neuron_vector;

	for( i = 0; i < job_data->tagset_size; i++ )
		beliefs[ i ] = 0.0;

	for( i = 0; i < (output->wh * output->ww); i++ )
		beliefs[ neuron_vector[i].output.ival ]++;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de classes. A classe que tiver o maior numero de neuronios 
	// setados sera considerada como o valor da camada de saida.
	
	for( i = 0; i < job_data->tagset_size; i++ )
	{
		beliefs[ i ] /= (float) (output->wh * output->ww);
		
		if( beliefs[ i ] > nMax )
		{
			nMax = beliefs[ i ];
			nClassID = i;
		}
	}

	// passing the actual output to the last position
	job_data->biliefs_vec = job_data->biliefs_vec->next;
	
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
	int nClassCnaeOutput;
	int nClassCnaeID = g_tag;
	int test = 0;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		nClassCnaeOutput = EvaluateOutput(output);
		
		g_nTotalTested++;

		// Compara a saida da rede com a classe real
		if(nClassCnaeOutput == nClassCnaeID)
		{
			test = 1;
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
	int nDoc;

	nDoc = pParamList->next->param.ival;
	printf( "word = %d\n", nDoc );
	
	g_nDocNumber = nDoc;
	
	if (ReadDocumentInput(&document, g_nDocNumber))
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

NEURON_OUTPUT TestStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	if( pParamList->next->param.ival == BEGINNING )
	{
		g_job_data = create_job_data( WORDS_BEFORE, WORDS_AFTER, TAGSET_SIZE );
	}
	else if( pParamList->next->param.ival == END )
	{
		free_jobs_data( g_job_data );
	}
	else
	{
		printf( "Unexpeted option %d in TestStatus\n", pParamList->next->param.ival );
		exit(1);
	}
	
	return (output);
}

NEURON_OUTPUT save_memory (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	save_neural_memory ("TREINAMENTO_COMPLETO");
	
	return (output);
}

NEURON_OUTPUT reload_memory (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	reload_neural_memory ("TREINAMENTO_COMPLETO");
	
	return (output);
}
