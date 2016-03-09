#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"
#include "timeutil.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define IMAGE_WIDTH		384
#define IMAGE_HEIGHT		288



// Variaveis globais
extern TRAIN_DATA_SET *g_train_data_set;
extern int g_train_num_samples;
extern char g_current_data_set_name[1000];

int g_TargetDayNumber = 0;

int g_nStatus = TRAINING_PHASE;

double g_train_return_average;
double g_train_return_standard_deviation;

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

		if ((g_TargetDayNumber) == g_train_num_samples)
		{
			if (test)
				g_TargetDayNumber = 0;
			else
				g_TargetDayNumber -= 1;
		} 
		else if (g_TargetDayNumber == 0)
		{
			if (test)
				g_TargetDayNumber += 1;  
			else
				g_TargetDayNumber = g_train_num_samples;
		}
		else
		{
			if (test)
				g_TargetDayNumber += 1;  
			else
				g_TargetDayNumber -= 1;
		}
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


void
initialize_data_set(INPUT_DESC *input)
{
	setlocale (LC_ALL, "C");	
	init_data_sets ();
	g_TargetDayNumber = 0;
	g_nTotalTested = 0;
	g_nStatus = TRAINING_PHASE;	

	GetNextSample(NO_DIRECTION);
	ReadSampleInput(input);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	
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

	make_input_image_wnn_pred (input, IMAGE_WIDTH, IMAGE_HEIGHT);	
		
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
	ReadSampleInput(input);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	
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
		neuron[i].output.ival = g_TargetDayNumber;

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
EvaluateOutputAverage (OUTPUT_DESC *output, float *certainty, int *best_day_index)
{
	int i, j;
	float average_return = 0.0;
	int current_week, selected_week = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;
	int num_neurons;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Calcula a media dos retornos e percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de neuronios com saidas coincidentes. A saida mais frequente ee encontrada e usada para
	// computar certainty. 
	num_neurons = output->wh * output->ww;
	for (i = 0; i < num_neurons; i++)
	{
		current_week = neuron_vector[i].output.ival;
		if ((current_week < 0) || (current_week > (1 + g_train_num_samples)))
		{
			Erro("Neuron learned a week < 0 or > last week in EvaluateOutputAverage()", "", "");
			exit(1);
		}

		average_return += g_train_data_set[current_week].neuron_return;

		nAux = 1;		
		for (j = i + 1; j < num_neurons; j++)
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
	
	*certainty = (double) nMax / (double) num_neurons;
	*best_day_index = selected_week;

	return (average_return / (float) num_neurons);
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: week ID
***********************************************************
*/

float 
EvaluateOutput(OUTPUT_DESC *output, float *certainty, int *best_day_index)
{
	int i, j;
	int current_week, selected_week = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;
	int num_neurons;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	num_neurons = output->wh * output->ww;
	for (i = 0; i < num_neurons; i++)
	{
		current_week = neuron_vector[i].output.ival;
		if ((current_week < 0) || (current_week > (1 + g_train_num_samples)))
		{
			Erro("Neuron learned a week < 0 or > last week in EvaluateOutput()", "", "");
			exit(1);
		}

		nAux = 1;		
		for (j = i + 1; j < num_neurons; j++)
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
	
	*certainty = (double) nMax / (double) num_neurons;
	*best_day_index = selected_week;
	
#if VERBOSE == 1
	// printf("week id = %d\n", selected_week);
	char Tdate[100], Tdow[100]; // Tdata = target date
	
	strcpy (Tdate, long2isodate (g_train_data_set[*best_day_index].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[*best_day_index].date_l));
	printf ("#|%s-%s - > % .6f\n", Tdate, Tdow, g_train_data_set[*best_day_index].neuron_return);
	
#endif
	return (g_train_data_set[*best_day_index].neuron_return);
}



void
base_output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, float (*EvaluateOutput) (OUTPUT_DESC *, float *, int *))
{
	float wnn_predicted_return;
	float certainty;
	int best_day_index;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_return = (*EvaluateOutput) (output, &certainty, &best_day_index);

		if (g_nTotalTested == 0)
			g_first_tested = g_TargetDayNumber;

		if (g_TargetDayNumber < g_train_num_samples)
		{
			g_nTotalTested++;
	
			g_train_data_set[g_TargetDayNumber].wnn_predicted_return = wnn_predicted_return;
			g_train_data_set[g_TargetDayNumber].wnn_prediction_error = g_train_data_set[g_TargetDayNumber].day_return - wnn_predicted_return;

			printf ("%s %s ; % .6f  ; % .6f ; % .6f  ;\n", 
				 long2isodate (g_train_data_set[g_TargetDayNumber].date_l),
				 long2dows (g_train_data_set[g_TargetDayNumber].date_l), 
				 g_train_data_set[g_TargetDayNumber].day_return,
				 g_train_data_set[g_TargetDayNumber].wnn_predicted_return,
				 g_train_data_set[g_TargetDayNumber].wnn_prediction_error);
/*
			float new_return = (g_train_data_set[g_TargetDayNumber].clo - g_train_data_set[g_TargetDayNumber-1].clo)/g_train_data_set[g_TargetDayNumber-1].clo;
			float predicted_price = g_train_data_set[g_TargetDayNumber].wnn_predicted_return + g_train_data_set[g_TargetDayNumber - (SAMPLE_SIZE + 1)].clo;
			float new_predicted_return = (predicted_price - g_train_data_set[g_TargetDayNumber-1].clo)/g_train_data_set[g_TargetDayNumber-1].clo;
			printf ("%s %s ; % .6f  ; % .6f ; % .6f  ;\n", 
				 long2isodate (g_train_data_set[g_TargetDayNumber].date_l),
				 long2dows (g_train_data_set[g_TargetDayNumber].date_l), 
				 new_return,
				 new_predicted_return,
				 new_return - new_predicted_return);
*/		}
		else
		{
			printf ("%s %s ;            ; % .6f ;            ;\n", 
				 long2isodate (timePlusDays(g_train_data_set[g_TargetDayNumber - 1].date_l, 1)),
				 long2dows (timePlusDays(g_train_data_set[g_TargetDayNumber - 1].date_l, 1)), 
				 wnn_predicted_return);
		}		
	
#if	RELEARN_RETURN == 1
		g_train_data_set[best_day_index].neuron_return = g_train_data_set[g_TargetDayNumber].day_return;
#endif
	}
	
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
	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutput);
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
	float standard_deviation_wnn_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;

	double tgt, out, err;
	/* counters */
	int h         = 0;	/* 	r * r^  > 0			*/
	int h_nz      = 0;	/* 	r * r^  !=  0			*/
	int h_up      = 0;	/* 	r > 0 AND r^ > 0		*/
	int h_dn      = 0;	/* 	r < 0 AND r^ < 0		*/	
	
	int r_up = 0;		/* 	r > 0 				*/
	int r_dn = 0;		/* 	r < 0 				*/
	int r_eq = 0;		/* 	r == 0 				*/

	int pr_up = 0;		/* 	r^  > 0 			*/
	int pr_dn = 0;		/* 	r^  > 0 			*/
	int pr_eq = 0;		/* 	r^  > 0 			*/

	double hr     = 0.0;	/* 	h / h_nz  			*/
	double hr_up  = 0.0;	/* 	h_up / pr_up			*/	
	double hr_dn  = 0.0;	/* 	h_dn / pr_dn			*/	
	
	// g_first_tested = pParamList->next->param.ival;;
	N = g_nTotalTested;
	for (i = g_first_tested; i < g_first_tested + N; i++)
	{
		tgt = g_train_data_set[i].day_return;
		out = g_train_data_set[i].wnn_predicted_return;
		err = g_train_data_set[i].wnn_prediction_error;

		average_target_return += tgt;
		average_wnn_prediction_error += err;
		
		/* counters */
		if      ( tgt > 0.0 )	r_up++;
		else if ( tgt < 0.0 )	r_dn++;
		else                    r_eq++;		

		if      ( out > 0.0 )	pr_up++;
		else if ( out < 0.0 )	pr_dn++;
		else                    pr_eq++;		

		/* Hits */
		if ( tgt * out > 0.0 )	h++;
		if ( tgt * out != 0.0 )	h_nz++;

		if ( tgt > 0.0 && out >  0.0 )	h_up++;		
		if ( tgt < 0.0 && out <  0.0 )	h_dn++;						
	}
	average_wnn_prediction_error /= (float) N;
	average_target_return /= (float) N;

	if ( h_nz > 0 )
		hr     = (double) h    / (double) h_nz ;
	else
		hr     = 0.0 ;

	if ( pr_up > 0 )		
		hr_up  = (double) h_up / (double) pr_up;
	else
		hr_up = 0.0;		

	if ( pr_dn > 0 )
		hr_dn  = (double) h_dn / (double) pr_dn;
	else
		hr_dn = 0.0;		

	for (i = g_first_tested; i < g_first_tested + N; i++)
	{
		tgt = g_train_data_set[i].day_return;
		err = g_train_data_set[i].wnn_prediction_error;

		standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        (err - average_wnn_prediction_error);
		standard_deviation_target_return += (tgt - average_target_return) *
					            (tgt - average_target_return);
	}
	standard_deviation_wnn_error = 	   sqrt(standard_deviation_wnn_error / (float) (N-1));
	standard_deviation_target_return = sqrt(standard_deviation_target_return / (float) (N-1));
	
	printf ("# target date  ; tgt. ret.  ; pred. ret.; pred. error; %s\n", g_current_data_set_name);
	printf ("#  ****  average_target_return: % .6f, average_wnn_error: % .6f\n",
			  average_target_return,
		          average_wnn_prediction_error);

	printf ("#  ****  standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
			  standard_deviation_target_return,
		          standard_deviation_wnn_error);

	printf ("#  ****  hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n",  100.0 * hr_up, 100.0 * hr_dn, 100.0 * hr);
			 
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
	INPUT_DESC *input;
	
	g_TargetDayNumber = pParamList->next->param.ival;
		
	GetNextSample (NO_DIRECTION);
	input = &sample;
	ReadSampleInput(input);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

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
		if (strcmp (sample_date, long2isodate (g_train_data_set[i].date_l)) == 0)
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
* Function: GetNumDays
* Description: GetNumDays of intraday data
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumDays (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	output.ival = g_train_num_samples;
	
	return (output);
}



/*
***********************************************************
* Function: ComputeAverageMaxAndMinReturn
* Description: Compute average MIN_RETURN and MAX_RETURN 
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT ComputeTrainReturnAverageAndStadardDeviation (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int first_week_id, last_week_id, i;
	double N;

	first_week_id = pParamList->next->param.ival;
	last_week_id = pParamList->next->next->param.ival;
	
#if (TYPE_OF_WNN_INPUT == 1) || (TYPE_OF_WNN_INPUT == 2) || (TYPE_OF_WNN_INPUT == 3)
	int j, previous_g_TargetDayNumber = g_TargetDayNumber;
	
	N = g_train_return_average = 0.0;
	for (i = first_week_id; i < last_week_id; i++)
	{
		g_TargetDayNumber = i;
		for (j = -SAMPLE_SIZE; j < 0; j++)
		{
			g_train_return_average += compute_sample_return(j);
			N += 1.0;
		}
	}
	g_train_return_average = g_train_return_average / N;
	
	g_train_return_standard_deviation = 0.0;
	for (i = first_week_id; i < last_week_id; i++)
	{
		g_TargetDayNumber = i;
		for (j = -SAMPLE_SIZE; j < 0; j++)
			g_train_return_standard_deviation += pow(compute_sample_return(j) - g_train_return_average, 2.0);
	}
	g_train_return_standard_deviation = sqrt(g_train_return_standard_deviation / N);
	
	g_TargetDayNumber = previous_g_TargetDayNumber;

#else
	g_train_return_average = 0.0;
	for (i = first_week_id - SAMPLE_SIZE; i < last_week_id; i++)
	{
		g_train_return_average += g_train_data_set[i].day_return;
	}
	N = (double) (last_week_id - (first_week_id - SAMPLE_SIZE));
	g_train_return_average = g_train_return_average / N;
	
	g_train_return_standard_deviation = 0.0;
	for (i = first_week_id - SAMPLE_SIZE; i < last_week_id; i++)
	{
		g_train_return_standard_deviation += pow(g_train_data_set[i].day_return - g_train_return_average, 2.0);
	}
	g_train_return_standard_deviation = sqrt(g_train_return_standard_deviation / N);
#endif
	output.ival = 0;
	
	return (output);
}
