#include <stdio.h>
#include <wchar.h>
#include "ita_user_functions.h"

#if ITA_BUILD
#include "../ita_signal_driver_vg_ram.h"
#include "../ita_signal_driver_vg_ram_mae.h"
#include "../ita_signal_driver_vg_ram_statistics.h"
#endif

struct _returns
{
	int id;
	char time[256];
	float WIN;
	float IND;
	float WDO;
	float DOL;
};

typedef struct _returns RETURNS;

#define MAX_RETURN_SAMPLES 5000

#define SS 0
#define SD 1
#define SN 2
#define DS 3
#define DD 4
#define DN 5
#define NS 6
#define ND 7
#define NN 8

#define RETURN 9
#define P_RETURN 10
#define INVEST 11
#define HITS 12

#define SUBIU 0
#define DESCE 1
#define NEUTR 2

// Global Variables
RETURNS returns[2][MAX_RETURN_SAMPLES];
int g_sample = 0;
int g_nStatus;
int g_LongShort = 1; // 1 = Long, 0 = Short
int POSE_MIN = 0;
int POSE_MAX = 0;

int g_runing_sum_size = 30;

int gcc_no_complain;
char *gcc_no_complain_c;

// Variaveis utilizadas no procedimento de teste
int g_results[2][MAX_RETURN_SAMPLES][INPUT_WIDTH][13];
int g_buy_sell_count[2][INPUT_WIDTH];
int g_sell_buy_count[2][INPUT_WIDTH];
double g_capital[2][INPUT_WIDTH];
double g_mean_correct_positive_pred[2][INPUT_WIDTH], g_mean_correct_negative_pred[2][INPUT_WIDTH];
double g_mean_reverse_positive_pred[2][INPUT_WIDTH], g_mean_reverse_negative_pred[2][INPUT_WIDTH];


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
		g_sample++;
		if (g_sample >= POSE_MAX)
			g_sample = POSE_MIN;
	}

	if (nDirection == DIRECTION_REWIND)
	{
		g_sample--;
		if (g_sample < POSE_MIN)
			g_sample = POSE_MAX - 1;
	}
	
	return (g_sample);
}


int
GetMAESampleIndex(void)
{
	return (g_sample);
}


void
SetSymbolReturn(int symbol, float symbol_return, int net, int displacement)
{
	if (symbol == 0) returns[net][g_sample + displacement].WIN = symbol_return;
	if (symbol == 1) returns[net][g_sample + displacement].IND = symbol_return;
	if (symbol == 2) returns[net][g_sample + displacement].WDO = symbol_return;
	if (symbol == 3) returns[net][g_sample + displacement].DOL = symbol_return;
}


void
SetSampleIdAndTime(int net, int displacement, int id, char *time)
{
	returns[net][g_sample + displacement].id = id;
	strcpy(returns[net][g_sample + displacement].time, time);
}


double
GetSymbolReturn(int symbol, int net, int displacement)
{
	if (symbol == 0) return ((double) returns[net][g_sample + displacement].WIN);
	if (symbol == 1) return ((double) returns[net][g_sample + displacement].IND);
	if (symbol == 2) return ((double) returns[net][g_sample + displacement].WDO);
	if (symbol == 3) return ((double) returns[net][g_sample + displacement].DOL);

	return (0.0); // This should never occur.
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
LoadReturnsToInput(INPUT_DESC *input, int net, int displacement)
{
	int x, y;

//#if ITA_BUILD
	printf("reading returns %d, time %s\n", returns[net][g_sample].id + displacement, returns[net][g_sample + displacement].time);
//#endif

	for (y = 0; y < input->neuron_layer->dimentions.y; y++)
	{
		for (x = 0; x < input->neuron_layer->dimentions.x; x++)
		{
			if (x == 0) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].WIN;
			if (x == 1) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].IND;
			if (x == 2) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].WDO;
			if (x == 3) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].DOL;
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
LoadReturnsToOutput(OUTPUT_DESC *output, int net)
{
	int x, y;

	for (y = 0; y < output->neuron_layer->dimentions.y; y++)
	{
		for (x = 0; x < output->neuron_layer->dimentions.x; x++)
		{
			if (x == 0) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample].WIN;
			if (x == 1) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample].IND;
			if (x == 2) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample].WDO;
			if (x == 3) output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample].DOL;
		}
	}

	return (0);
}


float
GetNeuronsOutput(OUTPUT_DESC *output, int x)
{
	int y;
	float n_up, n_down, n_stable, sum_up, sum_down, result;

	sum_up = sum_down = n_up = n_down = n_stable = 0.0;
	for (y = 0; y < output->neuron_layer->dimentions.y; y++)
	{
		float val = output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval;
		if (val > 0.0)
		{
			n_up += 1.0f;
			sum_up += val;
		}
		else if (val < 0.0)
		{
			n_down += 1.0f;
			sum_down += val;
		}
		else
			n_stable += 1.0f;
	}

	if ((n_up > n_down))// && (n_up > n_stable))
		result = sum_up / n_up;
	else if ((n_down > n_up))// && (n_down > n_stable))
		result = sum_down / n_down;
	else
		result = 0.0;

	return (result);
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
		input->image = (GLubyte *) alloc_mem ((unsigned int) (input->tfw * input->tfh) * 3 * sizeof (GLubyte));
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

	g_sample = POSE_MIN;
	g_nStatus = MOVING_PHASE;

	int i, j, k, l;
	for (l = 0; l < 2; l++)
		for (k = 0; k < MAX_RETURN_SAMPLES; k++)
			for (j = 0; j < INPUT_WIDTH; j++)
				for (i = 0; i < 13; i++)
					g_results[l][k][j][i] = 0;
	
	make_input_image_ita(input, INPUT_WIDTH, INPUT_HEIGHT);

	// Le a primeira imagem
	//ReadReturnsInput(input, 0);
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
	if (LoadReturnsToInput(input, 0, -1))
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
	LoadReturnsToOutput(output, 0);

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


int
r_sum(int net, int stock, int signal)
{
	int back_added;
	int sum = 0;

	for (back_added = 0; back_added < g_runing_sum_size; back_added++)
		if ((g_sample - back_added) >= 0)
			sum += g_results[net][g_sample - back_added][stock][signal];

	return (sum);
}


int
f_sum(int net, int stock, int parameter)
{
	int i;
	int sum = 0;

	for (i = POSE_MIN; i <= g_sample; i++)
		sum += g_results[net][i][stock][parameter];

	return (sum);
}


#if ITA_BUILD
void
compute_prediction_statistics(int net, int n_stocks, double *neural_prediction, double *actual_result)
{
	int stock;
	OUTPUT_DESC *result_output;

	result_output = get_output_by_name("out_result");

	for (stock = 0; stock < n_stocks; stock++)
	{
		double pred_ret = neural_prediction[stock];
		double actu_ret = actual_result[stock];
#else
void
compute_prediction_statistics(int net, int n_stocks, OUTPUT_DESC *neural_prediction, OUTPUT_DESC *actual_result)
{
	int stock;
	OUTPUT_DESC *result_output;

	result_output = get_output_by_name("out_result");

	for (stock = 0; stock < n_stocks; stock++)
	{
		float pred_ret = GetNeuronsOutput(neural_prediction, stock);
		float actu_ret = GetNeuronsOutput(actual_result, stock);
#endif

		int s1, s2;
		s1 = signal_of_val((float) g_results[net][g_sample - 1][stock][P_RETURN]);
		s2 = signal_of_val((float) g_results[net][g_sample - 1][stock][RETURN]);

		g_results[net][g_sample][stock][P_RETURN] = (int) round(pred_ret * 1000000.0);
		g_results[net][g_sample][stock][RETURN] = (int) round(actu_ret * 1000000.0);

		NEURON *result_report = result_output->neuron_layer->neuron_vector;
		result_report[stock].output.fval = 0.0;

		if 	((s1 == SUBIU) && (s2 == SUBIU)) {g_results[net][g_sample][stock][SS] += 1; result_report[stock].output.fval = 1.0;}
		else if ((s1 == SUBIU) && (s2 == DESCE))  g_results[net][g_sample][stock][SD] += 1;
		else if ((s1 == SUBIU) && (s2 == NEUTR))  g_results[net][g_sample][stock][SN] += 1;
		else if ((s1 == DESCE) && (s2 == SUBIU))  g_results[net][g_sample][stock][DS] += 1;
		else if ((s1 == DESCE) && (s2 == DESCE)) {g_results[net][g_sample][stock][DD] += 1; result_report[stock].output.fval = 1.0; }
		else if ((s1 == DESCE) && (s2 == NEUTR))  g_results[net][g_sample][stock][DN] += 1;
		else if ((s1 == NEUTR) && (s2 == SUBIU))  g_results[net][g_sample][stock][NS] += 1;
		else if ((s1 == NEUTR) && (s2 == DESCE))  g_results[net][g_sample][stock][ND] += 1;
		else if ((s1 == NEUTR) && (s2 == NEUTR)) {g_results[net][g_sample][stock][NN] += 1; result_report[stock].output.fval = -1.0;}

		int sum = r_sum(net, stock, SS) + r_sum(net, stock, SD) + r_sum(net, stock, SN);
		if (sum != 0)
		{
			g_mean_correct_positive_pred[net][stock] = 100.0 *
				(double) (3 * r_sum(net, stock, SS) - 3 * r_sum(net, stock, SD) - r_sum(net, stock, SN)) / (double) (3 * sum);

			g_mean_reverse_positive_pred[net][stock] = 100.0 *
				(double) (3 * r_sum(net, stock, SD) - 3 * r_sum(net, stock, SS) - r_sum(net, stock, SN)) / (double) (3 * sum);
		}
		else
		{
			g_mean_correct_positive_pred[net][stock] = -100.0;
			g_mean_reverse_positive_pred[net][stock] = -100.0;
		}

		sum = r_sum(net, stock, DD) + r_sum(net, stock, DS) + r_sum(net, stock, DN);
		if (sum != 0)
		{
			g_mean_correct_negative_pred[net][stock] = 100.0 *
				(double) (3 * r_sum(net, stock, DD) - 3 * r_sum(net, stock, DS) - r_sum(net, stock, DN)) / (double) (3 * sum);

			g_mean_reverse_negative_pred[net][stock] = 100.0 *
				(double) (3 * r_sum(net, stock, DS) - 3 * r_sum(net, stock, DD) - r_sum(net, stock, DN)) / (double) (3 * sum);
		}
		else
		{
			g_mean_correct_negative_pred[net][stock] = -100.0;
			g_mean_reverse_negative_pred[net][stock] = -100.0;
		}
	}
}


void
compute_capital_evolution(int net, int n, OUTPUT_DESC *neural_prediction, OUTPUT_DESC *actual_result)
{
	int i;

	for (i = 0; i < n; i++)
	{
		int s1;
		s1 = signal_of_val(GetNeuronsOutput(neural_prediction, i));

		double r = GetNeuronsOutput(neural_prediction, i);
		double a_capital = ALAVANCAGEM * g_capital[net][i];
		double expected_result_buy_sell = r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		double expected_result_sell_buy = -r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;

		r = GetNeuronsOutput(actual_result, i);
		double result_buy_sell = r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		double result_sell_buy = -r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;


		if (g_LongShort == 1)
		{
			if ((s1 == 0) && (expected_result_buy_sell > 0.0) &&
				(g_mean_correct_positive_pred[net][i] > CERTAINTY))
			{
				double previous_capital = g_capital[net][i];
				g_capital[net][i] += result_buy_sell;
				if (g_capital[net][i] > previous_capital)
					g_results[net][g_sample][i][HITS] += 1;

				g_buy_sell_count[net][i] += 1;
				g_results[net][g_sample][i][INVEST] = 1;
			}
			else
				g_results[net][g_sample][i][INVEST] = 0;
		}
		else
		{
			if ((s1 == 1) && (expected_result_sell_buy > 0.0) &&
				(g_mean_correct_negative_pred[net][i] > CERTAINTY))
			{
				double previous_capital = g_capital[net][i];
				g_capital[net][i] += result_sell_buy;
				if (g_capital[net][i] > previous_capital)
					g_results[net][g_sample][i][HITS] += 1;

				g_sell_buy_count[net][i] += 1;
				g_results[net][g_sample][i][INVEST] = 1;
			}
			else
				g_results[net][g_sample][i][INVEST] = 0;
		}
	}
}


int
buy_or_sell(int net, int stock, double *neural_prediction)
{
	int s1;
	s1 = signal_of_val((float) neural_prediction[stock]);

	if ((s1 == 0) && (g_mean_correct_positive_pred[net][stock] > CERTAINTY))
	{
		g_buy_sell_count[net][stock] += 1;
		//g_results[net][g_sample][i][HITS] += 1; // Tem que fazer isso depois do ciclo, por meio de outra compute_prediction_statistics()...
		g_results[net][g_sample][stock][INVEST] = 1;
		return (1);
	}
	else
	{
		g_results[net][g_sample][stock][INVEST] = 0;
		return (0);
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

#if !ITA_BUILD
void
EvaluateOutput(OUTPUT_DESC *output)
{
	OUTPUT_DESC *actual_result;

	actual_result = get_output_by_name("out_test");
	read_current_desired_returns_to_output(actual_result);

	int n_stocks = output->ww;
	compute_prediction_statistics(0, n_stocks, output, actual_result);
	if (g_nStatus == TEST_PHASE)
		compute_capital_evolution(0, n_stocks, output, actual_result);
}
#endif


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
#if !ITA_BUILD
	if ((g_nStatus == TEST_PHASE) || (g_nStatus == WARM_UP_PHASE))
	{
		if (strcmp(output->name, out_prediction.name) == 0)
			EvaluateOutput(output);
	}
#endif
	
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
		case 'n':
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
#if ITA_BUILD
NEURON_OUTPUT
ShowStatistics(int net)
{
	if (net == 0)
		printf("LONG MAE neural net statistics\n");
	else
		printf("SHORT MAE neural net statistics\n");

#else
NEURON_OUTPUT
ShowStatistics(PARAM_LIST *pParamList)
{
	int net = 0;
#endif

	NEURON_OUTPUT output;
	int i, stock, k, total_tested;
	int n_stocks = INPUT_WIDTH;

	for (stock = 0; stock < n_stocks; stock++)
	{
		total_tested = 0;
		for (k = 0; k < g_runing_sum_size; k++)
			for (i = 0; i < 9; i++)
				if ((g_sample - k) >= 0)
					total_tested += g_results[net][g_sample - k][stock][i];

		if (total_tested > 0)
		{
			if (stock == 0) printf("WIN: ");
			if (stock == 1) printf("IND: ");
			if (stock == 2) printf("WDO: ");
			if (stock == 3) printf("DOL: ");

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
					   ch1, ch2, 100.0 * (double) r_sum(net, stock, i) / (double) total_tested);
			}
			printf("== = %6.2lf:  ", 100.0 * (double) (r_sum(net, stock, SS) + r_sum(net, stock, DD) + r_sum(net, stock, NN)) / (double) total_tested);

			int sum = r_sum(net, stock, SS) + r_sum(net, stock, SD) + r_sum(net, stock, SN);
			if (sum != 0)
				printf("gi %6.1lf  ", 100.0 *
						(double) (3 * r_sum(net, stock, SS) - 3 * r_sum(net, stock, SD) - r_sum(net, stock, SN)) / (double) (3 * sum));
			else
				printf("gi   ---   ");

			sum = r_sum(net, stock, DD) + r_sum(net, stock, DS) + r_sum(net, stock, DN);
			if (sum != 0)
				printf("g! %6.1lf  ",100.0 *
						(double) (3 * r_sum(net, stock, DD) - 3 * r_sum(net, stock, DS) - r_sum(net, stock, DN)) / (double) (3 * sum));
			else
				printf("g!   ---  ");

			if ((g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]) > 0)
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = %6.1lf\n",
						g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
						(double) g_results[net][g_sample][stock][RETURN] / 10000.0, (double) g_results[net][g_sample][stock][P_RETURN] / 10000.0,
						g_results[net][g_sample][stock][INVEST],
						100.0 * (double) f_sum(net, stock, HITS) / (double) (g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]));
			else
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = ---\n",
						g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
						(double) g_results[net][g_sample][stock][RETURN] / 10000.0, (double) g_results[net][g_sample][stock][P_RETURN] / 10000.0,
						g_results[net][g_sample][stock][INVEST]);
		}
	}

	double total_capital = 0.0;
	int total_buy_sell = 0;
	int total_sell_buy = 0;
	for (stock = 0; stock < n_stocks; stock++)
	{
		total_capital += g_capital[net][stock];
		total_buy_sell += g_buy_sell_count[net][stock];
		total_sell_buy += g_sell_buy_count[net][stock];
	}

	//printf("total_tested = %d, capital = %.2lf\n", total_tested, capital);
	printf("num_periods_with_operations = %d, total_buy_sell = %d, total_sell_buy = %d, average_final_capital = %.2lf\n",
			total_tested, total_buy_sell, total_sell_buy, total_capital / (double) n_stocks);
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

	int i, j, k, l;
	for (j = 0; j < INPUT_WIDTH; j++)
	{
		for (l = 0; l < 2; l++)
		{
			g_capital[l][j] = 125000.0;
			g_buy_sell_count[l][j] = 0;
			g_sell_buy_count[l][j] = 0;
			for (k = 0; k < MAX_RETURN_SAMPLES; k++)
				for (i = 0; i < 13; i++)
					g_results[l][k][j][i] = 0;
		}
	}

	g_sample = POSE_MIN = 0; // ita.wh;
	POSE_MAX = 10000;

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


#if ITA_BUILD
NEURON_OUTPUT
SetNetworkStatus(int i)
{
	NEURON_OUTPUT output;

	g_nStatus = i;
#else
NEURON_OUTPUT
SetNetworkStatus(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;
#endif
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
	
	g_sample = pParamList->next->param.ival;
	if ((g_sample < POSE_MIN) || (g_sample >= POSE_MAX))
	{
		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetRandomReturns).\n", POSE_MIN, POSE_MAX - 1);
		fflush(stdout);
		output.ival = -1;
		return (output);
	}

	if (LoadReturnsToInput(&ita, 0, -1))
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
	int net = 0;
	while(fgets(file_line, 256, returns_file) != NULL)
	{
		if (sscanf(file_line, "%d; %s %s %s %f; %f; %f; %f;\n",
				&returns[net][numlines].id, date, returns[net][numlines].time, BRT,
				&returns[net][numlines].WIN, &returns[net][numlines].IND, &returns[net][numlines].WDO, &returns[net][numlines].DOL) != 8)
		{
			printf("%d; %s %s %s %f; %f; %f; %f;\n",
					returns[net][numlines].id, date, returns[net][numlines].time, BRT,
					returns[net][numlines].WIN, returns[net][numlines].IND, returns[net][numlines].WDO, returns[net][numlines].DOL);
			Erro("Could not read returns, in LoadReturns(), from file: ", returns_file_name, "");
		}
//		printf("%d; %s %s %s %lf; %lf; %lf; %lf;\n",
//				returns[net][numlines].id, date, returns[net][numlines].time, BRT,
//				returns[net][numlines].WIN, returns[net][numlines].IND, returns[net][numlines].WDO, returns[net][numlines].DOL);
		numlines++;
		if (numlines >= MAX_RETURN_SAMPLES)
			Erro("numlines >= MAX_RETURN_SAMPLES in LoadReturns().", "", "");
	}

	POSE_MAX = numlines - 1;

//	INPUT_DESC *input;
//	input = get_input_by_name("ita");
	g_sample = POSE_MIN = 0; // input->wh;

	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: SaveReturns
* Description:
* Inputs:
* Output:
***********************************************************
*/

int
SaveReturns(int last_return)
{
	FILE *returns_file;
	char returns_file_name[256];

	// Compute file name
	if (strlen(returns[0][POSE_MIN].time) <= 0)
		Erro("Error: cannot compute file name in SaveReturns()", "", "");

	strcpy(returns_file_name, returns[0][POSE_MIN].time);
	char *aux = strchr(returns_file_name, ' ');
	aux[0] = '\0';
	strcat(returns_file_name, ".csv");
	printf("Saving returns observed in: %s\n", returns_file_name);
	returns_file = fopen(returns_file_name, "w");
	if (returns_file == NULL)
		Erro("Error: cannot open file in SaveReturns(). returns_file_name = ", returns_file_name, "");

	fprintf(returns_file, "id; time; WIN<qty=25>; IND<qty=5>; WDO<qty=6>; DOL<qty=5>;\n");

	int net;
	for (net = 0; net < 2; net++)
	{
		int i;
		for (i = POSE_MIN; i <= last_return; i++)
		{
			fprintf(returns_file, "%d; %s; %f; %f; %f; %f;\n",
					returns[net][i].id, returns[net][i].time,
					returns[net][i].WIN, returns[net][i].IND, returns[net][i].WDO, returns[net][i].DOL);
		}
	}

	fclose(returns_file);

	return (0);
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
