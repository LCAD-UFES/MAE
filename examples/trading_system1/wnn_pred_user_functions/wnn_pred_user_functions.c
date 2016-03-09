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
int LAST_SAMPLE 	= 1;
DIR *g_data_set_dir = NULL;
DATA_SET *g_data_set = NULL;
int g_data_set_num_samples = 0;
int g_current_sample = 0;
int g_nStatus;
char g_current_data_set_file_name[1000];

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_first_sample_tested = 0;


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

		if ((g_current_sample + SAMPLE_GROUP_SIZE) >= (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE)
		{
			if (test)
				g_current_sample = 0;
			else
				g_current_sample -= SAMPLE_GROUP_SIZE;
		} 
		else if (g_current_sample == 0)
		{
			if (test)
				g_current_sample += SAMPLE_GROUP_SIZE;  
			else
				g_current_sample = (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE;
		}
		else
		{
			if (test)
				g_current_sample += SAMPLE_GROUP_SIZE;  
			else
				g_current_sample -= SAMPLE_GROUP_SIZE;
		}
	}	
	//printf("%s %d\n", g_data_set[g_current_sample].minute, g_current_sample);
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
//	double tgt, out, error;
	
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
	int i;
	
	returns_file = get_file (base_file_name, ".txt");
	g_data_set_num_samples = count_samples (returns_file, base_file_name);

	if (g_data_set != NULL)
		free (g_data_set);
	g_data_set = (DATA_SET *) alloc_mem (g_data_set_num_samples * sizeof (DATA_SET));
	
	// fgets (data_set_file_line, 900, returns_file); // le o cabecalho do arquivo
	for (i = 0; i < g_data_set_num_samples; i++)
	{
		if (fgets (data_set_file_line, 900, returns_file) == NULL)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
		if (sscanf (data_set_file_line, "%s %f %f", 
						 g_data_set[i].minute, 
						 &(g_data_set[i].sample_price),
						 &(g_data_set[i].sample_volume)) != 3)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
	}

	g_data_set_num_samples = i;
	
	fclose (returns_file);
	
	return (g_data_set_num_samples);
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
		train_data_set_file_entry = readdir (g_data_set_dir);
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
compute_samples_returns(int now, int reference)
{
	int sample;
	double sample_return;
	
	if (reference < 0) // o primeiro grupo de amostras fica comprometido...
		reference = 0;
#ifdef	VERBOSE
	printf ("ref %04d(%s) - now ", reference, g_data_set[reference].minute);
#endif
	for (sample = now; sample > reference; sample--)
	{
		sample_return = (g_data_set[sample].sample_price - g_data_set[reference].sample_price) / g_data_set[reference].sample_price;
		if (fabs(sample_return) < 0.0000009)
			sample_return = 0.0;
		
		g_data_set[sample].sample_return = sample_return;
#ifdef	VERBOSE
		printf ("%04d(%s)  ", sample, g_data_set[sample].minute);
#endif
	}
#ifdef	VERBOSE
	printf ("\n");
#endif
}



void
compute_sample_group_return_old(int now, int last_sample_in_the_next_sample_group)
{
	int sample;
	double next_return;
	
	if (last_sample_in_the_next_sample_group < (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE)
	{
		// Nao ee o ultimo grupo de amostras
		for (sample = now + 1; sample <= last_sample_in_the_next_sample_group; sample++)
		{
			next_return = (g_data_set[sample].sample_price - g_data_set[now].sample_price) / g_data_set[now].sample_price;
			if (next_return > STOP_GAIN)
			{
				g_data_set[now].sample_group_return = STOP_GAIN;
				break;
			}
			else if (next_return < STOP_LOSS)
			{
				g_data_set[now].sample_group_return = STOP_LOSS;
				break;
			}
			else
				g_data_set[now].sample_group_return = next_return;
		}
	}
	else	// Se for o ultimo grupo de amostras, nao tem sample_group_return...
		g_data_set[now].sample_group_return = 0.0;
}



void
compute_sample_group_return(int now, int last_sample_in_the_next_sample_group)
{
	int next_sample;
	double next_return;
	
	if (last_sample_in_the_next_sample_group < (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE)
	{	// Nao ee o ultimo grupo de amostras
		next_sample = now + SAMPLE_GROUP_SIZE; 
		next_return = (g_data_set[next_sample].sample_price - g_data_set[now].sample_price) / g_data_set[now].sample_price;
		if (fabs(next_return) < 0.0000009)
			next_return = 0.0;

		g_data_set[now].sample_group_return = next_return;
	}
	else	// Se for o ultimo grupo de amostras, nao tem sample_group_return...
		g_data_set[now].sample_group_return = 0.0;
}



void
compute_returns_from_prices()
{
	int sample_group;
	int now, last_sample_in_the_next_sample_group;
	int reference;
	int N;
	
	N = LAST_SAMPLE/SAMPLE_GROUP_SIZE;
	for (sample_group = 0; sample_group < N; sample_group++)
	{
#ifdef	VERBOSE
		printf ("sample g. %04d(%s) = ", sample_group * SAMPLE_GROUP_SIZE, g_data_set[sample_group * SAMPLE_GROUP_SIZE].minute);
#endif
		now = LAST_SAMPLE_IN_THE_GROUP(sample_group * SAMPLE_GROUP_SIZE);
		reference = now - NETWORK_INPUT_SIZE;
		compute_samples_returns(now, reference);

		last_sample_in_the_next_sample_group = LAST_SAMPLE_IN_THE_GROUP((sample_group + 1) * SAMPLE_GROUP_SIZE);
		compute_sample_group_return(now, last_sample_in_the_next_sample_group);
	}
}


void
write_error_file(char *message, float current_return, int i, int neuron_vector_i_output, 
		 int last_sample_in_the_group_neuron_output_number, int current_sample, 
		 int last_sample_in_the_group_current_sample, int last_sample)
{
	FILE *error_file;
	char error_file_name[1000];
	
	strcpy(error_file_name, "ERROR_DIR/");
	strcat(error_file_name, g_current_data_set_file_name);
	error_file = fopen(error_file_name, "a");
	fprintf(error_file, "%s - current_return = %f, i = %d, neuron_vector_i_output = %d\n", message, current_return, i, neuron_vector_i_output);
	fprintf(error_file, "     last_sample_in_the_group_neuron_output_number = %d, current_sample = %d\n", last_sample_in_the_group_neuron_output_number, current_sample);
	fprintf(error_file, "     last_sample_in_the_group_current_sample = %d, last_sample = %d\n", last_sample_in_the_group_current_sample, last_sample);
	fclose(error_file);
}


void
check_error(float current_return, int i, int neuron_vector_i_output, int last_sample_in_the_group_neuron_output_number,
	    int current_sample, int last_sample_in_the_group_current_sample, int last_sample)
{
	if (fabs(current_return) > 1.0)
		write_error_file("modulo current_return > 1.0", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (neuron_vector_i_output >= last_sample)
		write_error_file("neuron_vector_i_output >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (last_sample_in_the_group_neuron_output_number >= last_sample)
		write_error_file("last_sample_in_the_group_neuron_output_number >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (current_sample >= last_sample)
		write_error_file("current_sample >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (last_sample_in_the_group_current_sample >= last_sample)
		write_error_file("last_sample_in_the_group_current_sample >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
}


void
base_output_handler_average (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, double (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	//int wnn_predicted_week;
	double wnn_predicted_return;
	int num_neurons;
	int now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_return = 4.0 * (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		
		if (now != LAST_SAMPLE_IN_THE_GROUP((LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE))
			printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].sample_group_return,
				 g_data_set[now].wnn_predicted_return,
				 g_data_set[now].wnn_prediction_error);
		else
			printf ("%s ;            ; % .6f ;            ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].wnn_predicted_return);
	}
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
base_output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	int predicted_sample;
	double wnn_predicted_return;
	int num_neurons;
	int now, predicted_now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		predicted_sample = (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		predicted_now = LAST_SAMPLE_IN_THE_GROUP(predicted_sample);
#ifdef	VERBOSE
		printf("->  %04d(%s):  ", predicted_now, g_data_set[predicted_now].minute);
#endif
		wnn_predicted_return = g_data_set[predicted_now].sample_group_return;
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		
		if (now != LAST_SAMPLE_IN_THE_GROUP((LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE))
			printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].sample_group_return,
				 g_data_set[now].wnn_predicted_return,
				 g_data_set[now].wnn_prediction_error);
		else
			printf ("%s ;            ; % .6f ;            ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].wnn_predicted_return);
	}

#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
init_data_sets ()
{
	char *first_data_set_name;
	
	if ((g_data_set_dir = opendir (DATA_PATH)) == NULL)
	{
		show_message ("Could not open data set directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_data_set ()) == NULL)
	{
		show_message ("Could not initialize first data set from directory:", DATA_PATH, "");
		exit(1);
	}
	else
	{
		strcpy(g_current_data_set_file_name, first_data_set_name);
		compute_returns_from_prices();
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
	int i;

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
	{
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
		for (i = 0; i < input->tfw * input->tfh * 3; i++)
			input->image[i] = 0;
	}
}


void
initialize_data_set(INPUT_DESC *input)
{
	setlocale (LC_ALL, "C");	
	init_data_sets ();
	g_nTotalTested = 0;
	g_nStatus = TRAINING_PHASE;	

	GetNextSample(NO_DIRECTION);
	ReadSampleInput(input, g_current_sample);
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
	
	srand(5);
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
	ReadSampleInput(input, g_current_sample);
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
		initialize_data_set(input);
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
* Function: input_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void input_generator_next (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_wnn_pred(input);
#ifdef NO_INTERFACE
		input->win = 2;	
#endif
	}
}



/*
***********************************************************
* Function: draw_output
* Description:Draws the output layer
* Inputs: strOutputName -
*	  strInputName -g_current_sample
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
		neuron[i].output.ival = g_current_sample;	// Treina com a semana corrente

#ifdef	VERBOSE
	int now;
	now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);
	printf ("now %04d(%s) O<- %04d(%s)\n", now, g_data_set[now].minute, g_current_sample, g_data_set[g_current_sample].minute);
#endif

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

double 
EvaluateOutputAverage (OUTPUT_DESC *output, int *num_neurons)
{
	int i;
	double current_return, average_return = 0.0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_return = g_data_set[LAST_SAMPLE_IN_THE_GROUP(neuron_vector[i].output.ival)].sample_group_return;
		average_return += current_return;
		check_error(current_return, i, neuron_vector[i].output.ival, LAST_SAMPLE_IN_THE_GROUP(neuron_vector[i].output.ival), 
			    g_current_sample, LAST_SAMPLE_IN_THE_GROUP(g_current_sample), LAST_SAMPLE);
	}
	
	*num_neurons = output->wh * output->ww; /* it is not important in this type of output evaluation */
	
	return (average_return / (double) (output->wh * output->ww));
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: sample ID, num_neurons
***********************************************************
*/

int 
EvaluateOutput(OUTPUT_DESC *output, int *num_neurons)
{
	int i, j;
	int current_sample, selected_sample = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_sample = neuron_vector[i].output.ival;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.ival == current_sample)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			selected_sample = current_sample;
		}
	}
	
	*num_neurons = nMax;
	return (selected_sample);
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
	int N, sample_group;
	double average_wnn_prediction_error = 0.0;
	double standard_deviation_wnn_error = 0.0;
	double average_target_return = 0.0;
	double standard_deviation_target_return = 0.0;
	int wnn_same_up = 0, target_up = 0, wnn_same_down = 0, target_down = 0;
	int now;

	N = g_nTotalTested;

	PWB_ANALYSE_SERIES_TS *tgt;
	PWB_ANALYSE_SERIES_TS *out;

	tgt = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	tgt->v = alloc_mem (sizeof(double) * N);
	out = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	out->v = alloc_mem (sizeof(double) * N);
	
	tgt->n_v = N;
	out->n_v = N;
	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);
		
		tgt->v[sample_group] = g_data_set[now].sample_group_return;
		out->v[sample_group] = g_data_set[now].wnn_predicted_return;
	}
	
	printf ("#  ****  me: %lf, rmse: %lf, mape: %lf, utheil: %lf, R2: %lf\n", 
			  ME(tgt, out), RMSE(tgt, out), MAPE(tgt, out), UTheil(tgt, out), R2(tgt, out));

	printf ("#  ****  hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n", 
			  100.0 * HR_UP(tgt, out),
			  100.0 * HR_DN(tgt, out), 
			  100.0 * HR(tgt, out));
			 

	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);
		
		average_wnn_prediction_error += g_data_set[now].wnn_prediction_error;
		average_target_return += g_data_set[now].sample_group_return;
		
		if (g_data_set[now].sample_group_return > 0.0)
		{
			target_up++;
			if (g_data_set[now].wnn_predicted_return > 0.0)
				wnn_same_up++;
		}
		else if (g_data_set[now].sample_group_return < 0.0)
		{
			target_down++;
			if (g_data_set[now].wnn_predicted_return < 0.0)
				wnn_same_down++;
		}
		//printf ("minute = %d  average_wnn_prediction_error = %f\n", sample, average_wnn_prediction_error);
	}
	average_wnn_prediction_error /= (double) N;
	average_target_return /= (double) N;
	//printf ("%d  %f %f\n", N, average_wnn_prediction_error, average_target_return);

	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);

		standard_deviation_wnn_error += (g_data_set[now].wnn_prediction_error - average_wnn_prediction_error) *
					        (g_data_set[now].wnn_prediction_error - average_wnn_prediction_error);
		standard_deviation_target_return += (g_data_set[now].sample_group_return - average_target_return) *
					            (g_data_set[now].sample_group_return - average_target_return);
	}
	standard_deviation_wnn_error = sqrt (standard_deviation_wnn_error / (double) (N-1));
	standard_deviation_target_return = sqrt (standard_deviation_target_return / (double) (N-1));
	
	printf ("#  ****  average_target_return: % .6f, average_wnn_error: % .6f\n",
			  average_target_return,
		          average_wnn_prediction_error);

	printf ("#  ****  standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
			  standard_deviation_target_return,
		          standard_deviation_wnn_error);

	printf ("#  ****  wnn_same_up: %.2f, wnn_same_down: %.2f, wnn_same_signal: %.2f\n", 
			  100.0 * ((double) wnn_same_up / (double) target_up),
			  100.0 * ((double) wnn_same_down / (double) target_down), 
			  100.0 * ((double) (wnn_same_up + wnn_same_down) / (double) (target_up + target_down)));
			 
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
* Function: GetSample
* Description: Get a sample
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSample (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_current_sample = pParamList->next->param.ival;
		
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}



/*
***********************************************************
* Function: GetSampleGroup
* Description: Get a sample group
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSampleGroup (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_current_sample = SAMPLE_GROUP_SIZE * pParamList->next->param.ival;
		
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}



/*
***********************************************************
* Function: GetNumSamples
* Description: Get the naumber of samples
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumSamples (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE;
	
	return (output);
}



/*
***********************************************************
* Function: GetNumSampleGroups
* Description: Get the naumber of sample groups
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumSampleGroups (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = LAST_SAMPLE / SAMPLE_GROUP_SIZE;
	
	return (output);
}
