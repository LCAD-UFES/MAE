#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define IMAGE_WIDTH		384
#define IMAGE_HEIGHT		288

#define LINEAR_MODE		1
#define RAMDOM_MODE		2



struct _train_data_set
{
	char *date;
	char *day_week;
	float target_return;
};

typedef struct _train_data_set TRAIN_DATA_SET;


struct _tt_data_set
{
	char *date;
	char *day_week;
	float target_return;
	float rna_predicted_return;
	float rna_prediction_error;
	float wnn_predicted_return;
	float wnn_prediction_error;
};

typedef struct _tt_data_set TT_DATA_SET;


// Variaveis globais
int FIST_SAMPLE	= 0;
int LAST_SAMPLE = 1;
DIR *g_train_dir = NULL;
TRAIN_DATA_SET *g_train_data_set = NULL;
TT_DATA_SET *g_tt_data_set = NULL;
int g_train_num_samples = 0;
int g_tt_num_samples = 0;
int g_TrainSampleNumber = 0;
int g_TTSampleNumber = 0;
int g_nStatus;
char g_current_data_set_file_name[1000];

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int first_tested = 0;


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

		if (g_TrainSampleNumber == LAST_SAMPLE)
		{
			if (test)
				g_TrainSampleNumber = FIST_SAMPLE;
			else
				g_TrainSampleNumber--;
		} 
		else if (g_TrainSampleNumber == FIST_SAMPLE)
		{
			if (test)
				g_TrainSampleNumber++;  
			else
				g_TrainSampleNumber = LAST_SAMPLE;
		}
		else
		{
			if (test)
				g_TrainSampleNumber++;  
			else
				g_TrainSampleNumber--;
		}
	}	
}



/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	float target_return, normalized_target_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = 0;
	// printf ("%03d: %s %s ; ", g_TrainSampleNumber, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE-1].date, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE-1].day_week);
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			target_return = g_train_data_set[g_TrainSampleNumber + sample].target_return;
			// printf ("% .6f  ; ", target_return);
			normalized_target_return = (target_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_target_return * (float) y_h + 0.5);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if (position == y)
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample++;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
	/* Target return for sample g_TrainSampleNumber */
	// printf ("-> % .6f\n", g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].target_return);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

	return (0);
}



/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput_bar(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	float target_return, normalized_target_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = 0;
	// printf ("%03d: %s %s ; ", g_TrainSampleNumber, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE-1].date, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE-1].day_week);
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			target_return = g_train_data_set[g_TrainSampleNumber + sample].target_return;
			// printf ("% .6f  ; ", target_return);
			normalized_target_return = (target_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_target_return * (float) y_h + 0.5);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if ((y >= y_h/2) && (y <= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else if ((y < y_h/2) && (y >= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample++;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
	/* Target return for sample g_TrainSampleNumber */
	// printf ("-> % .6f\n", g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].target_return);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

	return (0);
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



int
read_train_data_set_data (FILE *data_set_file)
{
	char data_set_file_line[1000], date[100], day_week[100];
	float target_return;
	int i;
	
	g_train_num_samples = 0;
	while (fgets (data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;
		g_train_num_samples++;
	}
	rewind (data_set_file);
	if (g_train_data_set != NULL)
		free (g_train_data_set);
	g_train_data_set = (TRAIN_DATA_SET *) alloc_mem (g_train_num_samples * sizeof (TRAIN_DATA_SET));
	
	i = 0;
	while (fgets (data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;

		if (sscanf (data_set_file_line, "%s %s %f", (char *)date, (char *)day_week, 
						&target_return) != 3)
			break;
			
		// printf ("%s", data_set_file_line);
		g_train_data_set[i].date = (char *) alloc_mem (strlen (date) + 1);
		strcpy (g_train_data_set[i].date, date);
		if (day_week[strlen(day_week)-1] == ';')
			day_week[strlen(day_week)-1] = '\0';
		g_train_data_set[i].day_week = (char *) alloc_mem (strlen (day_week) + 1);
		strcpy (g_train_data_set[i].day_week, day_week);
		g_train_data_set[i].target_return = target_return;
		// printf ("%s %s ; % .6f  ;\n\n", g_train_data_set[i].date, g_train_data_set[i].day_week, g_train_data_set[i].target_return);
		i++;
	}

	g_train_num_samples = i;
	LAST_SAMPLE = g_train_num_samples - SAMPLE_SIZE;
	return (g_train_num_samples);
}



int
read_tt_data_set_data (FILE *data_set_file)
{
	char data_set_file_line[1000], date[100], day_week[100];
	float target_return, rna_predicted_return, rna_prediction_error;
	int i;
	
	g_tt_num_samples = 0;
	while (fgets (data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;
		g_tt_num_samples++;
	}
	rewind (data_set_file);
	if (g_tt_data_set != NULL)
		free (g_tt_data_set);
	g_tt_data_set = (TT_DATA_SET *) alloc_mem (g_tt_num_samples * sizeof (TT_DATA_SET));
	
	i = 0;
	while (fgets (data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;

		if (sscanf (data_set_file_line, "%s %s ; %f ; %f ; %f", (char *)date, (char *)day_week, 
						&target_return, &rna_predicted_return, &rna_prediction_error) != 5)
			break;
			
		// printf ("%s", data_set_file_line);
		g_tt_data_set[i].date = (char *) alloc_mem (strlen (date) + 1);
		strcpy (g_tt_data_set[i].date, date);
		g_tt_data_set[i].day_week = (char *) alloc_mem (strlen (day_week) + 1);
		strcpy (g_tt_data_set[i].day_week, day_week);
		g_tt_data_set[i].target_return = target_return;
		g_tt_data_set[i].rna_predicted_return = rna_predicted_return;
		g_tt_data_set[i].rna_prediction_error = rna_prediction_error;
		// printf ("%s %s ; % .6f  ; % .6f  ; % .6f  ; \n\n", g_tt_data_set[i].date, g_tt_data_set[i].day_week, g_tt_data_set[i].target_return, g_tt_data_set[i].rna_predicted_return, g_tt_data_set[i].rna_prediction_error);
		i++;
	}
	g_tt_num_samples = i;
	return (g_tt_num_samples);
}



char *
get_next_train_data_set ()
{
	struct dirent *train_data_set_file_entry;
	FILE *data_set_file;
	char file_name[1000];
	int num_samples;
	
	do
	{
		train_data_set_file_entry = readdir (g_train_dir);
		if (train_data_set_file_entry == NULL)
			return (NULL);
	} while (train_data_set_file_entry->d_name[0] == '.');
	
	strcpy (file_name, DATA_PATH);
	strcat (file_name, train_data_set_file_entry->d_name);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
		return (NULL);
	
	num_samples = read_train_data_set_data (data_set_file);
	printf ("# Data set: %s, num_samples: %d\n", train_data_set_file_entry->d_name, num_samples);

	fclose (data_set_file);

	return (train_data_set_file_entry->d_name);
}



int
get_next_tt_data_set (char *data_set_name)
{
	FILE *data_set_file;
	char file_name[1000];
	int num_samples;
	char *aux;
	
	strcpy (g_current_data_set_file_name, data_set_name);
	aux = strrchr (g_current_data_set_file_name, '.');
	aux[0] = '\0';
	
	strcpy (file_name, TT_PATH);
	strcat (file_name, g_current_data_set_file_name);
	strcat (file_name, ".pe");

	if ((data_set_file = fopen (file_name, "r")) == NULL)
		return (0);
	
	num_samples = read_tt_data_set_data (data_set_file);
	printf ("# Data set: %s.pe,  num_samples: %d\n", g_current_data_set_file_name, num_samples);
	
	fclose (data_set_file);
	
	return (1);
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
		show_message ("Could not initialize first train data set from directory:", DATA_PATH, "");	

	if (!get_next_tt_data_set (first_data_set_name))
		show_message ("Could not initialize first tt data set from directory:", TT_PATH, "");	
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
	int x, y;

	make_input_image_wnn_pred (input, IMAGE_WIDTH, IMAGE_HEIGHT);	
		
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

	// Initialize data set
	setlocale (LC_ALL, "C");	
	init_data_sets ();
	g_TrainSampleNumber = FIST_SAMPLE;
	g_nTotalTested = 0;
	g_nStatus = TRAINING_PHASE;	
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
	
	GetNextSample(NO_DIRECTION);
	ReadSampleInput(&sample);

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
	NEURON *neuron;
	int i;

	output = get_output_by_name (strOutputName);

	neuron = output->neuron_layer->neuron_vector;
	for (i = 0; i < output->wh * output->ww; i++)
		neuron[i].output.fval = g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].target_return;

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



void
align_data_sets ()
{
	for (g_TTSampleNumber = 0; g_TTSampleNumber < g_tt_num_samples; g_TTSampleNumber++)
	{
		if (strcmp (g_tt_data_set[g_TTSampleNumber].date, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].date) == 0)
			break;
	}
	if (g_TTSampleNumber == g_tt_num_samples)
		Erro ("Could not align data sets. Current train date: ", 
		g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].date,
		"");

	first_tested = g_TTSampleNumber;
}



void
check_data_sets_aligment ()
{
	g_TTSampleNumber++;

	if (g_TTSampleNumber == g_tt_num_samples)
		Erro ("Trying to access a sample from the tt data set beyond the last sample. Current train, and last tt data sets dates:", 
		g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].date,
		g_tt_data_set[g_TTSampleNumber-1].date);

	if ((strcmp (g_tt_data_set[g_TTSampleNumber].date, g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].date) != 0) ||
	    (g_tt_data_set[g_TTSampleNumber].target_return != g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].target_return))
		Erro ("Data sets misalignment detected. Current train, and tt data sets dates:", 
		g_train_data_set[g_TrainSampleNumber+SAMPLE_SIZE].date,
		g_tt_data_set[g_TTSampleNumber].date);
}



void
base_output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, float (*EvaluateOutput) (OUTPUT_DESC *))
{
	float wnn_predicted_return;
	

	if (g_nStatus == RECALL_PHASE)
	{
		if (g_nTotalTested == 0)
			align_data_sets ();
		else
			check_data_sets_aligment ();
			
		// Obtem a saida da rede
		wnn_predicted_return = (*EvaluateOutput) (output);
		g_nTotalTested++;

		g_tt_data_set[g_TTSampleNumber].wnn_predicted_return = wnn_predicted_return;
		g_tt_data_set[g_TTSampleNumber].wnn_prediction_error = g_tt_data_set[g_TTSampleNumber].target_return - wnn_predicted_return;
		
		printf ("%s %s ; % .6f ; % .6f ; % .6f\n", 
			 g_tt_data_set[g_TTSampleNumber].date,
			 g_tt_data_set[g_TTSampleNumber].day_week, 
			 g_tt_data_set[g_TTSampleNumber].target_return,
			 g_tt_data_set[g_TTSampleNumber].wnn_predicted_return,
			 g_tt_data_set[g_TTSampleNumber].wnn_prediction_error);
		printf ("# %03d: wnn_predicted_return: % .6f, target_return: % .6f, wnn_error: % .6f, rna_error: % .6f\n#\n",
			 g_nTotalTested,
			 g_tt_data_set[g_TTSampleNumber].wnn_predicted_return,
			 g_tt_data_set[g_TTSampleNumber].target_return, 
			 g_tt_data_set[g_TTSampleNumber].wnn_prediction_error,
			 g_tt_data_set[g_TTSampleNumber].rna_prediction_error);
	}
		
	glutSetWindow (output->win);
	output_display (output);
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
EvaluateOutput(OUTPUT_DESC *output)
{
	int i, j;
	float current_return, selected_return = 0.0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_return = neuron_vector[i].output.fval;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.fval == current_return)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			selected_return = current_return;
		}
	}
	
	return (selected_return);
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
	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutput);
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
EvaluateOutputAverage (OUTPUT_DESC *output)
{
	int i;
	float average_return = 0.0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		average_return += neuron_vector[i].output.fval;
	}
	
	return (5.0 * average_return / (float) (output->wh * output->ww));
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
	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutputAverage);
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
	float average_rna_prediction_error = 0.0;
	float standard_deviation_wnn_error = 0.0;
	float standard_deviation_rna_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;
	// int first_tested;
	int wnn_same_up = 0, rna_same_up = 0, target_up = 0, wnn_same_down = 0, rna_same_down = 0, target_down = 0;
	
	// first_tested = pParamList->next->param.ival;;
	N = g_nTotalTested;
	for (i = first_tested; i < first_tested + N; i++)
	{
		average_wnn_prediction_error += g_tt_data_set[i].wnn_prediction_error;
		average_rna_prediction_error += g_tt_data_set[i].rna_prediction_error;
		average_target_return += g_tt_data_set[i].target_return;
		
		if (g_tt_data_set[i].target_return > 0.0)
		{
			target_up++;
			if (g_tt_data_set[i].wnn_predicted_return > 0.0)
				wnn_same_up++;
			if (g_tt_data_set[i].rna_predicted_return > 0.0)
				rna_same_up++;
		}
		else if (g_tt_data_set[i].target_return < 0.0)
		{
			target_down++;
			if (g_tt_data_set[i].wnn_predicted_return < 0.0)
				wnn_same_down++;
			if (g_tt_data_set[i].rna_predicted_return < 0.0)
				rna_same_down++;
		}
		// printf ("average_wnn_prediction_error = %f\n", average_wnn_prediction_error);
	}
	average_wnn_prediction_error /= (float) N;
	average_rna_prediction_error /= (float) N;
	average_target_return /= (float) N;

	for (i = first_tested; i < first_tested + N; i++)
	{
		standard_deviation_wnn_error += (g_tt_data_set[i].wnn_prediction_error - average_wnn_prediction_error) *
					        (g_tt_data_set[i].wnn_prediction_error - average_wnn_prediction_error);
		standard_deviation_rna_error += (g_tt_data_set[i].rna_prediction_error - average_rna_prediction_error) *
					        (g_tt_data_set[i].rna_prediction_error - average_rna_prediction_error);
		standard_deviation_target_return += (g_tt_data_set[i].target_return - average_target_return) *
					            (g_tt_data_set[i].target_return - average_target_return);
	}
	standard_deviation_wnn_error = sqrt (standard_deviation_wnn_error / (float) (N-1));
	standard_deviation_rna_error = sqrt (standard_deviation_rna_error / (float) (N-1));
	standard_deviation_target_return = sqrt (standard_deviation_target_return / (float) (N-1));
	
	printf ("# **** average_target_return: % .6f, average_wnn_error: % .6f, average_rna_error: % .6f\n",
			 average_target_return,
		         average_wnn_prediction_error,
			 average_rna_prediction_error);

	printf ("# **** standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f, standard_deviation_rna_error: % .6f\n",
			 standard_deviation_target_return,
		         standard_deviation_wnn_error,
			 standard_deviation_rna_error);

	printf ("# **** wnn_same_up: %.2f, wnn_same_down: %.2f, rna_same_up: %.2f, rna_same_down: %.2f\n", 
			 100.0 * ((float) wnn_same_up / (float) target_up),
			 100.0 * ((float) wnn_same_down / (float) target_down),
			 100.0 * ((float) rna_same_up / (float) target_up),
			 100.0 * ((float) rna_same_down / (float) target_down));

	printf ("# **** wnn_same_signal: %.2f, rna_same_signal: %.2f\n", 
			 100.0 * ((float) (wnn_same_up + wnn_same_down) / (float) (target_up + target_down)),
			 100.0 * ((float) (rna_same_up + rna_same_down) / (float) (target_up + target_down)));
			 
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

	g_TrainSampleNumber = pParamList->next->param.ival;
		
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
		if (strcmp (sample_date, g_train_data_set[i].date) == 0)
			break;
	}
	
	if (i == g_train_num_samples)
		sample_number.ival = -1;
	else
		sample_number.ival = i;
	
	free (sample_date);
	
	return (sample_number);
}
