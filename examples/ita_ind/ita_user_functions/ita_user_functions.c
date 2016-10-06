#include <stdio.h>
#include <wchar.h>
#include "ita_user_functions.h"

struct _returns
{
	int id;
	char time[256];
	double WIN;
	double IND;
	double WDO;
	double DOL;
};

typedef struct _returns RETURNS;

#define MAX_RETURN_SAMPLES 5000


// Global Variables
RETURNS returns[MAX_RETURN_SAMPLES];
int g_nReturnsNum = 0;
int g_nStatus;
int g_LongShort = 1; // 1 = Long, 0 = Short
int POSE_MIN = 0;
int POSE_MAX = 0;

int gcc_no_complain;
char *gcc_no_complain_c;

// Variaveis utilizadas no procedimento de teste
int g_results[INPUT_WIDTH][13];
int g_buy_sell_count[INPUT_WIDTH];
int g_sell_buy_count[INPUT_WIDTH];
double g_capital[INPUT_WIDTH];
double g_mean_correct_positive_pred[INPUT_WIDTH], g_mean_correct_negative_pred[INPUT_WIDTH];
double g_mean_reverse_positive_pred[INPUT_WIDTH], g_mean_reverse_negative_pred[INPUT_WIDTH];


/*
***********************************************************
* Function: GetNextReturns
* Description:
* Inputs: strFileName -
*	  nDirection -
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

int
GetNextReturns(int nDirection)
{
	if (nDirection == DIRECTION_FORWARD)
	{
		g_nReturnsNum++;
		if (g_nReturnsNum >= POSE_MAX)
			g_nReturnsNum = POSE_MIN;
	}

	if (nDirection == DIRECTION_REWIND)
	{
		g_nReturnsNum--;
		if (g_nReturnsNum < POSE_MIN)
			g_nReturnsNum = POSE_MAX - 1;
	}
	
	return (1);
}


/*
***********************************************************
* Function: ReadReturnsInput
* Description: Reads returns to the MAE input
* Inputs: input - MAE input
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int
ReadReturnsInput(INPUT_DESC *input)
{
	int x, y;

	printf("reading returns %d, time %s\n", returns[g_nReturnsNum].id, returns[g_nReturnsNum].time);

	for (y = 0; y < input->neuron_layer->dimentions.y; y++)
	{
		for (x = 0; x < input->neuron_layer->dimentions.x; x++)
		{
			if (x == 0) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum - y].WIN;
			if (x == 1) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum - y].IND;
			if (x == 2) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum - y].WDO;
			if (x == 3) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum - y].DOL;
		}
	}
	
	return (0);
}


/*
***********************************************************
* Function: ReadReturnsOutput
* Description: Reads a RAW image to the MAE input
* Inputs: input - input image
*	  strFileName - Image file name
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int
ReadReturnsOutput(OUTPUT_DESC *output, char *strFileName)
{
	int x, y;

	for (y = 0; y < output->neuron_layer->dimentions.y; y++)
	{
		for (x = 0; x < output->neuron_layer->dimentions.x; x++)
		{
			if (x == 0) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum + 1].WIN;
			if (x == 1) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum + 1].IND;
			if (x == 2) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum + 1].WDO;
			if (x == 3) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[g_nReturnsNum + 1].DOL;
		}
	}

	return (0);
}


/*
***********************************************************
* Function: make_input_image_ita
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void
make_input_image_ita(INPUT_DESC *input, int w, int h)
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
* Function: init_ita
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void
init_ita(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	g_nReturnsNum = POSE_MIN;
	g_nStatus = MOVING_PHASE;

	int i, j;
	for (j = 0; j < INPUT_WIDTH; j++)
		for (i = 0; i < 13; i++)
			g_results[j][i] = 0;
	
	make_input_image_ita(input, INPUT_WIDTH, INPUT_HEIGHT);

	// Le a primeira imagem
	//ReadReturnsInput(input);
	//update_input_image(input);

	input->green_cross = 1;
	input->up2date = 0;

#ifndef NO_INTERFACE
	float f = 1.0;
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

int
init_user_functions(void)
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
* Function: GetNewReturns
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int
GetNewReturns(INPUT_DESC *input, int nDirection)
{
	GetNextReturns(nDirection);
	if (ReadReturnsInput(input))
		return (-1);
			
	check_input_bounds(input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_image(input);
	
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

void
input_generator(INPUT_DESC *input, int status)
{
	FILTER_DESC *filter;

	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_ita(input);
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
 	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewReturns(input, DIRECTION_REWIND);
			else if (input->wxd >= INPUT_WIDTH)
				GetNewReturns(input, DIRECTION_FORWARD);

			// output_update(&out_ita_lp);
			filter = get_filter_by_output(out_ita_lp_f.neuron_layer);
			filter_update(filter);
			output_update(&out_ita_lp_f);

#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}	
}


void
read_current_desired_returns_to_output(OUTPUT_DESC *output)
{
	char strFileName[128];

	ReadReturnsOutput(output, strFileName);

	update_output_image(output);
#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay(output->win);
#endif
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

void
draw_output(char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;

	output = get_output_by_name (strOutputName);
	read_current_desired_returns_to_output(output);
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

void
input_controler(INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (strCommand, "draw out_prediction based on ita move;");
		interpreter(strCommand);
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


int
signal_of_val(float val)
{
	if (val > 0.0)
		return (0);
	else if (val < 0.0)
		return (1);
	else
		return (2);
}


void
compute_prediction_statistics(int n, NEURON *neural_prediction,
							   NEURON *actual_result, NEURON *result_report)
{
	int i;

	for (i = 0; i < n; i++)
	{
		int s1, s2;
		s1 = signal_of_val(neural_prediction[i].output.fval);
		s2 = signal_of_val(actual_result[i].output.fval);

		g_results[i][9] = (int) (actual_result[i].output.fval * 1000000.0 + 0.5);
		g_results[i][10] = (int) (neural_prediction[i].output.fval * 1000000.0 + 0.5);

		result_report[i].output.fval = 0.0;

		if 	((s1 == 0) && (s2 == 0)) {g_results[i][0] += 1; result_report[i].output.fval = 1.0;}
		else if ((s1 == 0) && (s2 == 1))  g_results[i][1] += 1;
		else if ((s1 == 0) && (s2 == 2))  g_results[i][2] += 1;
		else if ((s1 == 1) && (s2 == 0))  g_results[i][3] += 1;
		else if ((s1 == 1) && (s2 == 1)) {g_results[i][4] += 1; result_report[i].output.fval = 1.0; }
		else if ((s1 == 1) && (s2 == 2))  g_results[i][5] += 1;
		else if ((s1 == 2) && (s2 == 0))  g_results[i][6] += 1;
		else if ((s1 == 2) && (s2 == 1))  g_results[i][7] += 1;
		else if ((s1 == 2) && (s2 == 2)) {g_results[i][8] += 1; result_report[i].output.fval = -1.0;}

		if ((g_results[i][0] + g_results[i][1] + g_results[i][2]) != 0)
			g_mean_correct_positive_pred[i] = 100.0 * (double) (3 * g_results[i][0] - 3 * g_results[i][1] - g_results[i][2])
					/ (double) (3 * (g_results[i][0] + g_results[i][1] + g_results[i][2]));
		else
			g_mean_correct_positive_pred[i] = -100.0;

		if ((g_results[i][0] + g_results[i][1] + g_results[i][2]) != 0)
			g_mean_reverse_positive_pred[i] = 100.0 * (double) (3 * g_results[i][1] - 3 * g_results[i][0] - g_results[i][2])
					/ (double) (3 * (g_results[i][0] + g_results[i][1] + g_results[i][2]));
		else
			g_mean_reverse_positive_pred[i] = -100.0;

		if ((g_results[i][4] + g_results[i][3] + g_results[i][5]) != 0)
			g_mean_correct_negative_pred[i] = 100.0 * (double) (3 * g_results[i][4] - 3 * g_results[i][3] - g_results[i][5])
					/ (double) (3 * (g_results[i][4] + g_results[i][3] + g_results[i][5]));
		else
			g_mean_correct_negative_pred[i] = -100.0;

		if ((g_results[i][4] + g_results[i][3] + g_results[i][5]) != 0)
			g_mean_reverse_negative_pred[i] = 100.0 * (double) (3 * g_results[i][3] - 3 * g_results[i][4] - g_results[i][5])
					/ (double) (3 * (g_results[i][4] + g_results[i][3] + g_results[i][5]));
		else
			g_mean_reverse_negative_pred[i] = -100.0;
	}
}


void
compute_capital_evolution(int n, NEURON *neural_prediction, NEURON *actual_result)
{
	int i;

	for (i = 0; i < n; i++)
	{
		int s1;
		s1 = signal_of_val(neural_prediction[i].output.fval);

		double r = neural_prediction[i].output.fval;
		double a_capital = ALAVANCAGEM * g_capital[i];
		double expected_result_buy_sell = r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		double expected_result_sell_buy = -r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;

		r = actual_result[i].output.fval;
		double result_buy_sell = r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		double result_sell_buy = -r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;


		if (g_LongShort == 1)
		{
			if ((s1 == 0) && (expected_result_buy_sell > 0.0) &&
				(g_mean_correct_positive_pred[i] > CERTAINTY))
			{
				double previous_capital = g_capital[i];
				g_capital[i] += result_buy_sell;
				if (g_capital[i] > previous_capital)
					g_results[i][12] += 1;

				g_buy_sell_count[i] += 1;
				g_results[i][11] = 1;
			}
			else
				g_results[i][11] = 0;
		}
		else
		{
			if ((s1 == 1) && (expected_result_sell_buy > 0.0) &&
				(g_mean_correct_negative_pred[i] > CERTAINTY))
			{
				double previous_capital = g_capital[i];
				g_capital[i] += result_sell_buy;
				if (g_capital[i] > previous_capital)
					g_results[i][12] += 1;

				g_sell_buy_count[i] += 1;
				g_results[i][11] = 1;
			}
			else
				g_results[i][11] = 0;
		}
	}
}


/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: person ID
***********************************************************
*/

void
EvaluateOutput(OUTPUT_DESC *output)
{
	NEURON *neural_prediction, *actual_result, *result_report;
	OUTPUT_DESC *test_output;
	OUTPUT_DESC *result_output;

	test_output = get_output_by_name("out_test");
	read_current_desired_returns_to_output(test_output);

	result_output = get_output_by_name("out_result");

	neural_prediction = output->neuron_layer->neuron_vector;
	actual_result = test_output->neuron_layer->neuron_vector;
	result_report = result_output->neuron_layer->neuron_vector;

	int n = output->ww;
	compute_prediction_statistics(n, neural_prediction, actual_result, result_report);
	if (g_nStatus == TEST_PHASE)
		compute_capital_evolution(n, neural_prediction, actual_result);
}


/*
***********************************************************
* Function: output_handler
* Description:
* Inputs:
* Output:
***********************************************************
*/

void
output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	if ((g_nStatus == TEST_PHASE) || (g_nStatus == WARM_UP_PHASE))
	{
		if (strcmp(output->name, out_prediction.name) == 0)
			EvaluateOutput(output);
	}
	
#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display (output);
#endif
}


// ----------------------------------------------------
// f_keyboard - Funcao chamada quando e pressionada uma
//				tecla.
//
// Entrada: key_value - ponteiro para o cacacter pressionado
//
// Saida: Nenhuma
// ----------------------------------------------------

void
f_keyboard(char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
		// Moves the input to the next photo
		case 'N':
			GetNewReturns(&ita, DIRECTION_FORWARD);
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

NEURON_OUTPUT
ShowStatistics(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i, j, total_tested;

	for (j = 0; j < INPUT_WIDTH; j++)
	{
		total_tested = 0;
		for (i = 0; i < 9; i++)
			total_tested += g_results[j][i];

		if (total_tested > 0)
		{
			if (j == 0) printf("WIN: ");
			if (j == 1) printf("IND: ");
			if (j == 2) printf("WDO: ");
			if (j == 3) printf("DOL: ");

			for (i = 0; i < 9; i++)
			{
				char ch1 = ' ', ch2 = ' ';

				if ((i == 0) || (i == 1) || (i == 2)) ch1 = 'i';
				if ((i == 3) || (i == 4) || (i == 5)) ch1 = '!';
				if ((i == 6) || (i == 7) || (i == 8)) ch1 = '-';

				if ((i == 0) || (i == 3) || (i == 6)) ch2 = 'i';
				if ((i == 1) || (i == 4) || (i == 7)) ch2 = '!';
				if ((i == 2) || (i == 5) || (i == 8)) ch2 = '-';

				printf("%c%c = %5.2lf, ",
					   ch1, ch2, 100.0 * (double) g_results[j][i] / (double) total_tested);
			}
			printf("== = %6.2lf:  ",
				   100.0 * (double) (g_results[j][0] + g_results[j][4] + g_results[j][8]) / (double) total_tested);
			if ((g_results[j][0] + g_results[j][1] + g_results[j][2]) != 0)
				printf("gi %6.1lf  ",
					   100.0 * (double) (3 * g_results[j][0] - 3 * g_results[j][1] - g_results[j][2]) /
							   (double) (3 * (g_results[j][0] + g_results[j][1] + g_results[j][2])));
			else
				printf("gi   ---   ");
			if ((g_results[j][4] + g_results[j][3] + g_results[j][5]) != 0)
				printf("g! %6.1lf  ",
					   100.0 * (double) (3 * g_results[j][4] - 3 * g_results[j][3] - g_results[j][5]) /
							   (double) (3 * (g_results[j][4] + g_results[j][3] + g_results[j][5])));
			else
				printf("g!   ---  ");

			if ((g_buy_sell_count[j] + g_sell_buy_count[j]) > 0)
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = %6.1lf\n",
						g_buy_sell_count[j], g_sell_buy_count[j], g_capital[j],
						(double) g_results[j][9] / 10000.0, (double) g_results[j][10] / 10000.0, g_results[j][11],
						100.0 * (double) g_results[j][12] / (double) (g_buy_sell_count[j] + g_sell_buy_count[j]));
			else
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = ---\n",
						g_buy_sell_count[j], g_sell_buy_count[j], g_capital[j],
						(double) g_results[j][9] / 10000.0, (double) g_results[j][10] / 10000.0, g_results[j][11]);
		}
	}

	double total_capital = 0.0;
	int total_buy_sell = 0;
	int total_sell_buy = 0;
	for (j = 0; j < INPUT_WIDTH; j++)
	{
		total_capital += g_capital[j];
		total_buy_sell += g_buy_sell_count[j];
		total_sell_buy += g_sell_buy_count[j];
	}

	//printf("total_tested = %d, capital = %.2lf\n", total_tested, capital);
	printf("num_periods_with_operations = %d, total_buy_sell = %d, total_sell_buy = %d, average_final_capital = %.2lf\n",
			total_tested, total_buy_sell, total_sell_buy, total_capital / (double) INPUT_WIDTH);
	fflush(stdout);

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

NEURON_OUTPUT
ResetStatistics(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	int i, j;
	for (j = 0; j < INPUT_WIDTH; j++)
	{
		g_capital[j] = 50000.0;
		g_buy_sell_count[j] = 0;
		g_sell_buy_count[j] = 0;
		for (i = 0; i < 13; i++)
			g_results[j][i] = 0;
	}

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

NEURON_OUTPUT
SetNetworkStatus(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			break;
		case TEST_PHASE:
			break;
		case WARM_UP_PHASE:
			break;
	}
	
	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: GetRandomReturns
* Description: Gets a new face
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT
GetRandomReturns(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	g_nReturnsNum = pParamList->next->param.ival;
	if ((g_nReturnsNum < POSE_MIN) || (g_nReturnsNum >= POSE_MAX))
	{
		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetRandomReturns).\n", POSE_MIN, POSE_MAX - 1);
		fflush(stdout);
		output.ival = -1;
		return (output);
	}

	if (ReadReturnsInput(&ita))
	{
		printf ("Error: Cannot read return (GetRandomReturns).\n");
		output.ival = -1;
		fflush(stdout);
		return (output);
	}
		
	check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
	ita.up2date = 0;
	update_input_image(&ita);
		
	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: LoadReturns
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
LoadReturns(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	FILE *returns_file;

	char *returns_file_name = pParamList->next->param.sval;

	returns_file_name = returns_file_name + 1; // tira uma aspas no inicio
	returns_file_name[strlen(returns_file_name) - 1] = '\0'; // tira aspas do fim

	printf("%s\n", returns_file_name);
	returns_file = fopen(returns_file_name, "r");

	if (returns_file == NULL)
		Erro("Error: cannot open file in LoadReturns(). returns_file_name = ", returns_file_name, "");

	int numlines = 0;
	char file_line[257];
	char *aux = fgets(file_line, 256, returns_file); // read header
	aux = aux;
	char date[256];
	char BRT[256];
	while(fgets(file_line, 256, returns_file) != NULL)
	{
		if (sscanf(file_line, "%d; %s %s %s %lf; %lf; %lf; %lf;\n",
				&returns[numlines].id, date, returns[numlines].time, BRT,
				&returns[numlines].WIN, &returns[numlines].IND, &returns[numlines].WDO, &returns[numlines].DOL) != 8)
		{
			printf("%d; %s %s %s %lf; %lf; %lf; %lf;\n",
					returns[numlines].id, date, returns[numlines].time, BRT,
					returns[numlines].WIN, returns[numlines].IND, returns[numlines].WDO, returns[numlines].DOL);
			Erro("Could not read returns, in LoadReturns(), from file: ", returns_file_name, "");
		}
//		printf("%d; %s %s %s %lf; %lf; %lf; %lf;\n",
//				returns[numlines].id, date, returns[numlines].time, BRT,
//				returns[numlines].WIN, returns[numlines].IND, returns[numlines].WDO, returns[numlines].DOL);
		numlines++;
		if (numlines >= MAX_RETURN_SAMPLES)
			Erro("numlines >= MAX_RETURN_SAMPLES in LoadReturns().", "", "");
	}

	POSE_MAX = numlines - 1;

	INPUT_DESC *input;
	input = get_input_by_name("ita");
	g_nReturnsNum = POSE_MIN = input->wh;

	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: SetLongShort
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
SetLongShort(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_LongShort = pParamList->next->param.ival;

	output.ival = 0;
	return (output);
}

