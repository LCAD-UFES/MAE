#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"
#include "pwb_analyse_series.h"



#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define TRAINING_PHASE		1
#define RECALL_PHASE		2



// Variaveis globais
int FIRST_SAMPLE	= 4;
int LAST_SAMPLE 	= 1;
DIR *g_train_dir = NULL;
TRAIN_DATA_SET *g_train_data_set = NULL;
int g_train_num_samples = 0;
int g_TrainWeekNumber = 0;
int g_nStatus;
char g_current_data_set_file_name[1000];

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_first_tested = 0;


/*
***********************************************************
* Function: GetNextSample
* Description:
* Inputs: nDirection
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

void 
GetNextSample(int nDirection)
{
	int test = 0;
	
	if (nDirection != NO_DIRECTION)
	{
		test = (nDirection == DIRECTION_FORWARD)? 1:0;

		if ((g_TrainWeekNumber + FIRST_SAMPLE) == LAST_SAMPLE)
		{
			if (test)
				g_TrainWeekNumber = 0;
			else
				g_TrainWeekNumber -= 1;
		} 
		else if (g_TrainWeekNumber == 0)
		{
			if (test)
				g_TrainWeekNumber += 1;  
			else
				g_TrainWeekNumber = LAST_SAMPLE - FIRST_SAMPLE;
		}
		else
		{
			if (test)
				g_TrainWeekNumber += 1;  
			else
				g_TrainWeekNumber -= 1;
		}
	}	
}



FILE *
get_file (char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	
	strcpy (file_name, DATA_PATH);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open file: ", file_name, "");
		return (NULL);
	}
	return (data_set_file);
}



int
count_samples (FILE *data_set_file, char *base_file_name)
{
	int num_samples;
//	char data_set_file_first_line[1000];
	char data_set_file_last_line[1000];
	char aux[1000];
//	float tgt, out, error;
	
/*	if (fgets (data_set_file_first_line, 900, data_set_file) == NULL)
	{
		Erro ("Could not read first line of the dataset: ", base_file_name, ".txt");
		exit (1);
	}
	if (strcmp (data_set_file_first_line, "#TGT, OUT, ERROR\n") != 0)
	{
		Erro ("Could not read first line of the dataset: ", base_file_name, ".txt");
		exit (1);
	}
*/	
	num_samples = 0;
	while (fgets (aux, 900, data_set_file) != NULL)
	{
		num_samples++;
		strcpy (data_set_file_last_line, aux);
	}
	
	rewind (data_set_file);
	
/*	if (sscanf (data_set_file_last_line, "%f , %f , %f", &tgt, &out, &error) != 3)
	{
		Erro ("Could not read last line of the dataset: ", base_file_name, ".txt");
		exit (1);
	}
*/
	return (num_samples);
}



int
read_train_data_set_data (char *base_file_name)
{
	FILE *returns_file;
	char data_set_file_line[1000];
//	char not_used1[1000];
	char not_used2[1000];
	int i;
	
	returns_file = get_file (base_file_name, ".txt");
	g_train_num_samples = count_samples (returns_file, base_file_name);

	if (g_train_data_set != NULL)
		free (g_train_data_set);
	g_train_data_set = (TRAIN_DATA_SET *) alloc_mem (g_train_num_samples * sizeof (TRAIN_DATA_SET));
	
	// fgets (data_set_file_line, 900, returns_file); // le o cabecalho do arquivo
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (fgets (data_set_file_line, 900, returns_file) == NULL)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
		if (sscanf (data_set_file_line, "%s %s %f", g_train_data_set[i].date, not_used2, &(g_train_data_set[i].week_return)) != 3)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
	}

	g_train_num_samples = i;
	
	fclose (returns_file);
	
	return (g_train_num_samples);
}



char *
get_next_train_data_set ()
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;
	
	do
	{
		train_data_set_file_entry = readdir (g_train_dir);
		if (train_data_set_file_entry == NULL)
			return (NULL);
		aux = strrchr (train_data_set_file_entry->d_name, '.');
	} while (strcmp (aux, ".txt") != 0);
	
	strcpy (base_file_name, train_data_set_file_entry->d_name);
	
	aux = strrchr (base_file_name, '.');
	aux[0] = '\0';
	
	LAST_SAMPLE = num_samples = read_train_data_set_data (base_file_name);
	printf ("# Data set: %s, num_samples: %d\n", train_data_set_file_entry->d_name, num_samples);

	return (train_data_set_file_entry->d_name);
}



void
init_data_sets ()
{
	char *first_data_set_name;
	
	if ((g_train_dir = opendir (DATA_PATH)) == NULL)
	{
		show_message ("Could not open DATA directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_data_set ()) == NULL)
	{
		show_message ("Could not initialize first train data set from directory:", DATA_PATH, "");
		exit(1);
	}
}





/*
***********************************************************
* Function: make_input_image_wnn_pred
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void make_input_image_wnn_pred (INPUT_DESC *input, int w, int h)
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
* Function: init_wnn_pred
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_wnn_pred(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	make_input_image_wnn_pred (input, 4*IN_WIDTH, 4*IN_HEIGHT);	
		
	input->up2date = 0;

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

	// Initialize data set
	setlocale (LC_ALL, "C");	
	init_data_sets ();
	g_nTotalTested = 0;
	g_nStatus = TRAINING_PHASE;	

	GetNextSample(NO_DIRECTION);
	ReadSampleInput(input);
	srand(5);
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
* Function: GetNewSample
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

void 
GetNewSample(INPUT_DESC *input, int nDirection)
{
	GetNextSample(nDirection);
	ReadSampleInput(input);
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
		init_wnn_pred(input);
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewSample (input, DIRECTION_REWIND);
			else if (input->wxd >= IN_WIDTH)
				GetNewSample (input, DIRECTION_FORWARD);
			
			all_filters_update ();
			output_update(&sample_gaussian_out);
			output_update(&nl_wnn_pred_out);
			
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
*	  strInputName -g_TrainWeekNumber
* Output: None
***********************************************************
*/

void draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;
	NEURON *neuron;
	int i;

	output = get_output_by_name (strOutputName);

	neuron = output->neuron_layer->neuron_vector;
	for (i = 0; i < output->wh * output->ww; i++)
		neuron[i].output.ival = g_TrainWeekNumber;

	//printf ("train week = %d\n", g_TrainWeekNumber);

	update_output_image (output);
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
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
		sprintf (strCommand, "draw nl_wnn_pred_out based on wnn_pred move;");
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

float 
EvaluateOutputAverage (OUTPUT_DESC *output, int *num_neurons)
{
	int i;
	float average_return = 0.0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		average_return += g_train_data_set[FIRST_SAMPLE + neuron_vector[i].output.ival].week_return;
	}
	
	*num_neurons = output->wh * output->ww; /* it is not important in this type of output evaluation */
	
	return ((2.0 * average_return) / (float) (output->wh * output->ww));
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: week ID, num_neurons
***********************************************************
*/

int 
EvaluateOutput(OUTPUT_DESC *output, int *num_neurons)
{
	int i, j;
	int current_week, selected_week = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_week = neuron_vector[i].output.ival;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.ival == current_week)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			selected_week = current_week;
		}
	}
	
	*num_neurons = nMax;
	return (selected_week);
}



void
base_output_handler_average (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, float (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	//int wnn_predicted_week;
	float wnn_predicted_return;
	int num_neurons;
	
	//printf ("Week = %03d, ", g_TrainWeekNumber);
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_return = 2.0 * (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_tested = g_TrainWeekNumber;

		g_nTotalTested++;
		
		//wnn_predicted_return = g_train_data_set[FIRST_SAMPLE + wnn_predicted_week].week_return;
		g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return = wnn_predicted_return;
		g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error = g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return - wnn_predicted_return;

/*		printf ("Activation = %6.2f; %s ; % .6f  ; % .6f ; % .6f  ;\n", 
			 (100.0 * (float) num_neurons / (float) (output->wh * output->ww)), 
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].date,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error);
*/		printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n", 
		//printf ("week = %d, %s ; % .6f  ; % .6f ; % .6f  ;\n", g_TrainWeekNumber,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].date,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error);
	}
/*	else
		printf ("\n");
*/		
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
base_output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	int wnn_predicted_week;
	float wnn_predicted_return;
	int num_neurons;
	
	//printf ("Week = %03d, ", g_TrainWeekNumber);
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_week = (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_tested = g_TrainWeekNumber;

		g_nTotalTested++;
		
		wnn_predicted_return = g_train_data_set[FIRST_SAMPLE + wnn_predicted_week].week_return;
		g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return = wnn_predicted_return;
		g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error = g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return - wnn_predicted_return;

/*		printf ("Activation = %6.2f; %s ; % .6f  ; % .6f ; % .6f  ;\n", 
			 (100.0 * (float) num_neurons / (float) (output->wh * output->ww)), 
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].date,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error);
*/		printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n", 
		//printf ("week = %d, %s ; % .6f  ; % .6f ; % .6f  ;\n", g_TrainWeekNumber,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].date,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_predicted_return,
			 g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].wnn_prediction_error);
	}
/*	else
		printf ("\n");
*/		
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
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

void 
output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
//	base_output_handler_average (output, type_call, mouse_button, mouse_state, EvaluateOutputAverage);
	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutput);
//	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutputRiRj);
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

void 
output_handler_average (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	base_output_handler_average (output, type_call, mouse_button, mouse_state, EvaluateOutputAverage);
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
	int N, i;
	float average_wnn_prediction_error = 0.0;
	float standard_deviation_wnn_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;
	int wnn_same_up = 0, target_up = 0, wnn_same_down = 0, target_down = 0;

	N = g_nTotalTested;

	PWB_ANALYSE_SERIES_TS *tgt;
	PWB_ANALYSE_SERIES_TS *out;

	tgt = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	tgt->v = alloc_mem (sizeof(double) * N);
	out = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	out->v = alloc_mem (sizeof(double) * N);
	
	tgt->n_v = N;
	out->n_v = N;
	for (i = g_first_tested; i < g_first_tested + N; i++)
	{
		tgt->v[i-g_first_tested] = g_train_data_set[FIRST_SAMPLE + i].week_return;
		out->v[i-g_first_tested] = g_train_data_set[FIRST_SAMPLE + i].wnn_predicted_return;
	}
	
	printf ("#  ****  me: %lf, rmse: %lf, mape: %lf, utheil: %lf, R2: %lf\n", 
			  ME(tgt, out), RMSE(tgt, out), MAPE(tgt, out), UTheil(tgt, out), R2(tgt, out));

	printf ("#  ****  hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n", 
			  100.0 * HR_UP(tgt, out),
			  100.0 * HR_DN(tgt, out), 
			  100.0 * HR(tgt, out));
			 

	// g_first_tested = pParamList->next->param.ival;;

	for (i = g_first_tested; i < g_first_tested + N; i++)
	{
		average_wnn_prediction_error += g_train_data_set[FIRST_SAMPLE + i].wnn_prediction_error;
		average_target_return += g_train_data_set[FIRST_SAMPLE + i].week_return;
		
		if (g_train_data_set[FIRST_SAMPLE + i].week_return > 0.0)
		{
			target_up++;
			if (g_train_data_set[FIRST_SAMPLE + i].wnn_predicted_return > 0.0)
				wnn_same_up++;
		}
		else if (g_train_data_set[FIRST_SAMPLE + i].week_return < 0.0)
		{
			target_down++;
			if (g_train_data_set[FIRST_SAMPLE + i].wnn_predicted_return < 0.0)
				wnn_same_down++;
		}
		//printf ("week = %d  average_wnn_prediction_error = %f\n", FIRST_SAMPLE + i, average_wnn_prediction_error);
	}
	average_wnn_prediction_error /= (float) N;
	average_target_return /= (float) N;
	//printf ("%d  %f %f\n", N, average_wnn_prediction_error, average_target_return);

	for (i = g_first_tested; i < g_first_tested + N; i++)
	{
		standard_deviation_wnn_error += (g_train_data_set[FIRST_SAMPLE + i].wnn_prediction_error - average_wnn_prediction_error) *
					        (g_train_data_set[FIRST_SAMPLE + i].wnn_prediction_error - average_wnn_prediction_error);
		standard_deviation_target_return += (g_train_data_set[FIRST_SAMPLE + i].week_return - average_target_return) *
					            (g_train_data_set[FIRST_SAMPLE + i].week_return - average_target_return);
	}
	standard_deviation_wnn_error = sqrt (standard_deviation_wnn_error / (float) (N-1));
	standard_deviation_target_return = sqrt (standard_deviation_target_return / (float) (N-1));
	
	printf ("#  ****  average_target_return: % .6f, average_wnn_error: % .6f\n",
			  average_target_return,
		          average_wnn_prediction_error);

	printf ("#  ****  standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
			  standard_deviation_target_return,
		          standard_deviation_wnn_error);

	printf ("#  ****  wnn_same_up: %.2f, wnn_same_down: %.2f, wnn_same_signal: %.2f\n", 
			  100.0 * ((float) wnn_same_up / (float) target_up),
			  100.0 * ((float) wnn_same_down / (float) target_down), 
			  100.0 * ((float) (wnn_same_up + wnn_same_down) / (float) (target_up + target_down)));
			 
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

NEURON_OUTPUT GetSample (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_TrainWeekNumber = pParamList->next->param.ival;
		
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample);

	output.ival = 0;
	
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

NEURON_OUTPUT GetSamplebyDate (PARAM_LIST *param_list)
{
	NEURON_OUTPUT sample_number;
	char *sample_date = NULL;
	char *aux_str;
	int i;
	
	// Gets the parameters
	aux_str = param_list->next->param.sval;
		
	sample_date = alloc_mem (strlen (aux_str) + 1);
	strcpy (sample_date, aux_str + 1);
	sample_date[strlen (aux_str) - 2] = '\0';
	
	for (i = 0; i < g_train_num_samples; i++)
	{
//		if (strcmp (sample_date, long2isodate (g_train_data_set[i].date_l)) == 0)
			break;
	}
	
	if (i == g_train_num_samples)
		sample_number.ival = -1;
	else
		sample_number.ival = i;
	
	free (sample_date);
	
	return (sample_number);
}



/*
***********************************************************
* Function: GetDoc
* Description: Gets a document
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumWeeks (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = (LAST_SAMPLE-FIRST_SAMPLE);
	
	return (output);
}
