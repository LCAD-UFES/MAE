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

	float result_WIN;
	float result_IND;
	float result_WDO;
	float result_DOL;
};

typedef struct _returns RETURNS;

typedef struct _statistics_exp
{
	int day;
	double avg_capital;
	int	n_ops;
	double capital_win;
	int	n_ops_win;
	double capital_ind;
	int	n_ops_ind;
	double capital_wdo;
	int	n_ops_wdo;
	double capital_dol;
	int	n_ops_dol;	
} STATISTICS_EXP;

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

#define ACC 9
#define GI 10
#define Gi 11

#define SUBIU 0
#define DESCE 1
#define NEUTR 2

//TODO:
// valores de hora pra ita.cml
/*
#define TRAIN_HOUR 10
#define TRAIN_MIN 32
#define TEST_HOUR 11
#define TEST_MIN 36
#define TEST_END_HOUR 15
#define TEST_END_MIN 00
*/
#define TRAIN_HOUR 10
#define TRAIN_MIN 00
#define TEST_HOUR 11
#define TEST_MIN 30//00
#define TEST_END_HOUR 16//12
#define TEST_END_MIN 45//30


#define MAX_DAYS 200
// Global Variables
STATISTICS_EXP stat_exp[MAX_DAYS];
double stat_day[MAX_DAYS][INPUT_WIDTH][13];
int day_i = 0;
int g_use_results = 0;
char date[256];

RETURNS returns[2][MAX_RETURN_SAMPLES];
int g_sample = 0;
int g_nStatus;
int g_LongShort = 1; // 1 = Long, 0 = Short
int POSE_MIN = 0;
int POSE_MAX = 0;

int g_runing_sum_size = STATISTICS;//25;

int gcc_no_complain;
char *gcc_no_complain_c;

// Variaveis utilizadas no procedimento de teste
int g_results[2][MAX_RETURN_SAMPLES][INPUT_WIDTH][13];
int g_buy_sell_count[2][INPUT_WIDTH];
int g_sell_buy_count[2][INPUT_WIDTH];
double g_capital[2][INPUT_WIDTH];
double g_mean_correct_positive_pred[2][INPUT_WIDTH], g_mean_correct_negative_pred[2][INPUT_WIDTH];
double g_mean_reverse_positive_pred[2][INPUT_WIDTH], g_mean_reverse_negative_pred[2][INPUT_WIDTH];

double g_confidence_up[2][INPUT_WIDTH], g_confidence_down[2][INPUT_WIDTH];
int g_use_confiance = ! USE_STATISTICS;
int g_use_prc = USE_PRC;

char days_file_name[257];
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
//	printf("reading returns %d, time %s\n", returns[net][g_sample].id + displacement, returns[net][g_sample + displacement].time);
//#endif

	float r_i, p_i, p_i_1;
	int height = input->neuron_layer->dimentions.y -1;
	for (y = 0; y < input->neuron_layer->dimentions.y; y++)
	{
		for (x = 0; x < input->neuron_layer->dimentions.x; x++)
		{
			/*if (x == 0)
			{
				printf("i=%d y=%d x=%d ", g_sample - y + displacement, y, x);
				printf("%lf \n", returns[net][g_sample - y + displacement].WIN);
			}*/
			if ( g_use_prc == 0 )
			{
				if (x == 0) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].WIN;
				if (x == 1) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].IND;
				if (x == 2) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].WDO;
				if (x == 3) input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample - y + displacement].DOL;
			}
			else
			{
				if (x == 0)
				{
					r_i = returns[net][g_sample - height + y + displacement].WIN;
					if (y == 0)
						p_i_1 = 1.0;
					else
						p_i_1 = input->neuron_layer->neuron_vector[(height - y + 1) * input->neuron_layer->dimentions.x + x].output.fval;//returns[net][g_sample - height + y - 1 + displacement].WIN;
					p_i = r_i * p_i_1 + p_i_1;

					input->neuron_layer->neuron_vector[(height - y) * input->neuron_layer->dimentions.x + x].output.fval = p_i;
				}
				if (x == 1)
				{
					r_i = returns[net][g_sample - height + y + displacement].IND;
					if (y == 0)
						p_i_1 = 1.0;
					else
						p_i_1 = input->neuron_layer->neuron_vector[(height - y + 1) * input->neuron_layer->dimentions.x + x].output.fval;//returns[net][g_sample - height + y - 1 + displacement].IND;
					p_i = r_i * p_i_1 + p_i_1;

					input->neuron_layer->neuron_vector[(height - y) * input->neuron_layer->dimentions.x + x].output.fval = p_i;
				}
				if (x == 2)
				{
					r_i = returns[net][g_sample - height + y + displacement].WDO;
					if (y == 0)
						p_i_1 = 1.0;
					else
						p_i_1 = input->neuron_layer->neuron_vector[(height - y + 1) * input->neuron_layer->dimentions.x + x].output.fval;//returns[net][g_sample - height + y - 1 + displacement].WDO;
					p_i = r_i * p_i_1 + p_i_1;

					input->neuron_layer->neuron_vector[(height - y) * input->neuron_layer->dimentions.x + x].output.fval = p_i;
				}
				if (x == 3)
				{
					r_i = returns[net][g_sample - height + y + displacement].DOL;
					if (y == 0)
						p_i_1 = 1.0;
					else
						p_i_1 = input->neuron_layer->neuron_vector[(height - y + 1) * input->neuron_layer->dimentions.x + x].output.fval;//returns[net][g_sample - height + y - 1 + displacement].DOL;
					p_i = r_i * p_i_1 + p_i_1;

					input->neuron_layer->neuron_vector[(height - y) * input->neuron_layer->dimentions.x + x].output.fval = p_i;
				}
			}
		}
	}
	
	if (g_use_prc == 1)
	{
		for (y = 0; y < input->neuron_layer->dimentions.y; y++)
		{
			for (x = 0; x < input->neuron_layer->dimentions.x; x++)
			{
				input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval -= 1.0;
				//if (x == 0) printf("%f ", input->neuron_layer->neuron_vector[y * input->neuron_layer->dimentions.x + x].output.fval);

			}
		}
		//printf("\n");
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
			if (x == 0)
			{
				//printf("%f \n", returns[net][g_sample].WIN);
				output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x].output.fval = returns[net][g_sample].WIN;
			}
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


	//g_confidence_up[0][x] = 100. * (n_up - n_down) / n_up;
	//g_confidence_down[0][x] = 100. * (n_down - n_up) / n_down;
	//printf("x=%d n_up=%.0lf n_down=%.0lf c_up=%.2lf c_down=%.2lf\n", x, n_up, n_down, g_confidence_up[0][x], g_confidence_down[0][x]);


	return (result);
}

void
GetNeuronsOutputConfidence(OUTPUT_DESC *output, int x)
{
	int y;
	float n_up, n_down, n_stable, sum_up, sum_down;

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

/*	if ((n_up > n_down))// && (n_up > n_stable))
		result = sum_up / n_up;
	else if ((n_down > n_up))// && (n_down > n_stable))
		result = sum_down / n_down;
	else
		result = 0.0;
*/

	g_confidence_up[0][x] = 100. * (n_up - n_down) / n_up;
	g_confidence_down[0][x] = 100. * (n_down - n_up) / n_down;
	//printf("x=%d n_up=%.0lf n_down=%.0lf c_up=%.2lf c_down=%.2lf\n", x, n_up, n_down, g_confidence_up[0][x], g_confidence_down[0][x]);
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
	//FILTER_DESC *filter;
	//printf("2\n");
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		//printf("2.1\n");
		init_ita(input);
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
 	}
	else
	{
		//printf("2.2\n");
		if (status == MOVE)
		{
			//printf("2.3\n");
			if (input->wxd < 0)
			{
				GetNewReturns(input, DIRECTION_REWIND);
				//printf("2.4\n");
			}
			else if (input->wxd >= INPUT_WIDTH)
			{
				GetNewReturns(input, DIRECTION_FORWARD);
				//printf("2.5\n");
			}

			//printf("g_sample=%d\n", g_sample);
			// output_update(&out_ita_lp);
			//filter = get_filter_by_output(out_ita_lp_f.neuron_layer);

			//filter_update(filter);
			all_filters_update ();

			//update neural layer
			all_dendrites_update ();
			all_neurons_update ();

			//output_update(&out_ita_lp_f);
			all_outputs_update();

			ShowStatistics(NULL);
			ShowStatisticsExp(NULL);

			ShowNeuronsMemory(NULL);
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

	//printf("1\n");
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		//printf("1.1\n");
		sprintf (strCommand, "draw out_prediction based on ita move;");
		interpreter(strCommand);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		//printf("1.2\n");
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter(strCommand);
	}

	//printf("1.3\n");
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
		GetNeuronsOutputConfidence(neural_prediction, i);

		double r = GetNeuronsOutput(neural_prediction, i);
		double a_capital = ALAVANCAGEM * g_capital[net][i];
		double expected_result_buy_sell = r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		double expected_result_sell_buy = -r * a_capital -
				CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;

		double result_buy_sell = 0.0;
		double result_sell_buy = 0.0;

		if ( g_use_results == 1 )
		{
			if (i == 0) result_buy_sell = returns[net][g_sample].result_WIN;
			if (i == 1) result_buy_sell = returns[net][g_sample].result_IND;
			if (i == 2) result_buy_sell = returns[net][g_sample].result_WDO;
			if (i == 3) result_buy_sell = returns[net][g_sample].result_DOL;
		}
		else
		{
			r = GetNeuronsOutput(actual_result, i);
			result_buy_sell = r * a_capital -
					CUSTO_TRASACAO * (2.0 * a_capital + r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
			result_sell_buy = -r * a_capital -
					CUSTO_TRASACAO * (2.0 * a_capital - r * a_capital) - 2.0 * CUSTO_CORRETORA_P;
		}

		//printf("%d\n", g_use_confiance);
		if (g_LongShort == 1)
		{
			if ( ( g_runing_sum_size > 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_confidence_up[net][i] > CERTAINTY) && (g_use_confiance == 1) ) ||
				 ( g_runing_sum_size > 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_mean_correct_positive_pred[net][i] > CERTAINTY) && (g_use_confiance == 0) ) ||
				 ( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) ) 
				)
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
			if ( ( g_runing_sum_size > 0 &&  (s1 == 1) && (expected_result_sell_buy > 0.0) && (g_confidence_up[net][i] > CERTAINTY) && (g_use_confiance == 1) ) ||
				 ( g_runing_sum_size > 0 &&  (s1 == 1) && (expected_result_sell_buy > 0.0) && (g_mean_correct_negative_pred[net][i] > CERTAINTY) && (g_use_confiance == 0) ) ||
				 ( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_sell_buy > 0.0) )
				)
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
	//printf("output_handler\n");

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

NEURON_OUTPUT
ShowNeuronsMemory(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT out;

	int y, x;
	OUTPUT_DESC* output = NULL;
	output = get_output_by_name(out_prediction.name);
	for(x = 0; x < output->neuron_layer->dimentions.x; x++)
	{
		for (y = 0; y < output->neuron_layer->dimentions.y; y++)
		{
			printf("%d ", output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x ].last_best_pattern);
		}
		printf("\n");
	}
	printf("\n");
	fflush(stdout);

	out.ival = 0;
	return out;
}


void
f_keyboard(char *key_value)
{
	char key;
	int y, x;
	OUTPUT_DESC* output = NULL;

	key = key_value[0];
	switch (key)
	{
		// Moves the input to the next photo
		case 'n':
			GetNewReturns(&ita, DIRECTION_FORWARD);
			break;

		case 'p':
			output = get_output_by_name(out_prediction.name);
			for(x = 0; x < output->neuron_layer->dimentions.x; x++)
			{
				for (y = 0; y < output->neuron_layer->dimentions.y; y++)
				{
					printf("%d ", output->neuron_layer->neuron_vector[y * output->neuron_layer->dimentions.x + x ].last_best_pattern);
				}
				printf("\n");
			}
			printf("\n");
			fflush(stdout);
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

	printf("%s %s\n", date, returns[0][g_sample].time);
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
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = %6.1lf, g_confidence_up=%.2lf\n",
						g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
						(double) g_results[net][g_sample][stock][RETURN] / 10000.0, (double) g_results[net][g_sample][stock][P_RETURN] / 10000.0,
						g_results[net][g_sample][stock][INVEST],
						100.0 * (double) f_sum(net, stock, HITS) / (double) (g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]), g_confidence_up[net][stock]);
			else
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = ---, g_confidence_up=%.2lf\n",
						g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
						(double) g_results[net][g_sample][stock][RETURN] / 10000.0, (double) g_results[net][g_sample][stock][P_RETURN] / 10000.0,
						g_results[net][g_sample][stock][INVEST], g_confidence_up[net][stock]);
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

NEURON_OUTPUT
ShowStatisticsExp(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	//stat_day

	//--------------- stat day
	//TODO: NET 0
	int net = 0;

	int i, stock, total_tested;
	int n_stocks = INPUT_WIDTH;

	for (stock = 0; stock < n_stocks; stock++)
	{
		total_tested = 0;
		for (i = 0; i < 9; i++)
			total_tested += f_sum(net, stock, i);

		if (total_tested > 0)
		{
			//if (stock == 0) printf("WIN: ");
			//if (stock == 1) printf("IND: ");
			//if (stock == 2) printf("WDO: ");
			//if (stock == 3) printf("DOL: ");

			for (i = 0; i < 9; i++)
			{
				//char ch1 = ' ', ch2 = ' ';
				/*
				if ((i == 0) || (i == 1) || (i == 2)) ch1 = 'i';
				if ((i == 3) || (i == 4) || (i == 5)) ch1 = '!';
				if ((i == 6) || (i == 7) || (i == 8)) ch1 = '-';

				if ((i == 0) || (i == 3) || (i == 6)) ch2 = 'i';
				if ((i == 1) || (i == 4) || (i == 7)) ch2 = '!';
				if ((i == 2) || (i == 5) || (i == 8)) ch2 = '-';
				*/

				stat_day[day_i][stock][i] =  100.0 * (double) f_sum(net, stock, i) / (double) total_tested;

				//printf("%c%c = %5.2lf, ",
				//	   ch1, ch2, 100.0 * (double) f_sum(net, stock, i) / (double) total_tested);
			}
			stat_day[day_i][stock][ACC] = 100.0 * (double) (f_sum(net, stock, SS) + f_sum(net, stock, DD) + f_sum(net, stock, NN)) / (double) total_tested;
			//printf("== = %6.2lf:  ", 100.0 * (double) (f_sum(net, stock, SS) + f_sum(net, stock, DD) + f_sum(net, stock, NN)) / (double) total_tested);

			int sum = f_sum(net, stock, SS) + f_sum(net, stock, SD) + f_sum(net, stock, SN);
			if (sum != 0)
			{
				//printf("gi %6.1lf  ", 100.0 *
				//		(double) (3 * f_sum(net, stock, SS) - 3 * f_sum(net, stock, SD) - f_sum(net, stock, SN)) / (double) (3 * sum));

				stat_day[day_i][stock][GI] = 100.0 *
						(double) (3 * f_sum(net, stock, SS) - 3 * f_sum(net, stock, SD) - f_sum(net, stock, SN)) / (double) (3 * sum);
			}
			else
			{
				//printf("gi   ---   ");
				stat_day[day_i][stock][GI] = 0.0;
			}

			sum = f_sum(net, stock, DD) + f_sum(net, stock, DS) + f_sum(net, stock, DN);
			if (sum != 0)
			{
				//printf("g! %6.1lf  ",100.0 *
				//		(double) (3 * f_sum(net, stock, DD) - 3 * f_sum(net, stock, DS) - f_sum(net, stock, DN)) / (double) (3 * sum));

				stat_day[day_i][stock][Gi] = 100.0 *
						(double) (3 * f_sum(net, stock, DD) - 3 * f_sum(net, stock, DS) - f_sum(net, stock, DN)) / (double) (3 * sum);
			}
			else
			{
				//printf("g!   ---  ");
				stat_day[day_i][stock][Gi] = 0.0;
			}

			if ((g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]) > 0)
			{
				//printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, hit_rate = %6.1lf\n",
				//		g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
				//		100.0 * (double) f_sum(net, stock, HITS) / (double) (g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]));

				//stat_day[day_i][stock][HITS] = 100.0 * (double) f_sum(net, stock, HITS) / (double) (g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]);
				stat_day[day_i][stock][HITS] = (double) f_sum(net, stock, HITS) ;
			}
			else
			{
				//printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, hit_rate = ---\n",
				//		g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock]);

				//stat_day[day_i][stock][HITS] = 100.0;
				stat_day[day_i][stock][HITS] = 0.0;
			}
		}
	}



	//--------------- capital day

	//printf("STAT DAY\n");
	double total_capital = 0.0;
	int total_buy_sell = 0;
	int total_sell_buy = 0;
	//int stock;
	//int n_stocks = 4;
	char symbol[6];
	for (stock = 0; stock < n_stocks; stock++)
	{
		total_capital += g_capital[0][stock];
		total_buy_sell += g_buy_sell_count[0][stock];
		total_sell_buy += g_sell_buy_count[0][stock];
	}

	stat_exp[day_i].day = day_i;
	stat_exp[day_i].avg_capital = total_capital/(double)n_stocks;
	stat_exp[day_i].n_ops = total_buy_sell;

	printf("day_i=%d; avg_capital=%.2lf; n_ops=%d; ", day_i, stat_exp[day_i].avg_capital, stat_exp[day_i].n_ops);

	for (stock = 0; stock < n_stocks; stock++)
	{
		if (stock == 0) strcpy(symbol, "WIN_");//printf("WIN: ");
		if (stock == 1) strcpy(symbol, "IND_");//printf("IND: ");
		if (stock == 2) strcpy(symbol, "WDO_");//printf("WDO: ");
		if (stock == 3) strcpy(symbol, "DOL_");//printf("DOL: ");

		for (i = 0; i < 9; i++)
		{
			char ch1 = ' ', ch2 = ' ';

			if ((i == 0) || (i == 1) || (i == 2)) ch1 = 'i';
			if ((i == 3) || (i == 4) || (i == 5)) ch1 = '!';
			if ((i == 6) || (i == 7) || (i == 8)) ch1 = '-';

			if ((i == 0) || (i == 3) || (i == 6)) ch2 = 'i';
			if ((i == 1) || (i == 4) || (i == 7)) ch2 = '!';
			if ((i == 2) || (i == 5) || (i == 8)) ch2 = '-';

			printf("%s%c%c=%.2lf; ", symbol, ch1, ch2, stat_day[day_i][stock][i]);
		}
		printf("%s===%.2lf; ", symbol, stat_day[day_i][stock][ACC]);
		printf("%sgi=%.1lf; ",symbol, stat_day[day_i][stock][GI]);
		printf("%sg!=%.1lf; ",symbol,  stat_day[day_i][stock][Gi]);
		if (g_buy_sell_count[net][stock] > 0)
			printf("%shit_rate=%.1lf; ",symbol,  100.0 * stat_day[day_i][stock][HITS]/g_buy_sell_count[net][stock]);
		else
			printf("%shit_rate=--; ",symbol);
		printf("%scapital=%.1lf; ",symbol,  g_capital[net][stock]);
		printf("%sn_ops=%d; ",symbol,  g_buy_sell_count[net][stock]);
	}
	printf("\n");


	stat_exp[day_i].capital_win = g_capital[net][0];
	stat_exp[day_i].n_ops_win = g_buy_sell_count[net][0];
	stat_exp[day_i].capital_ind = g_capital[net][1];
	stat_exp[day_i].n_ops_ind = g_buy_sell_count[net][1];
	stat_exp[day_i].capital_wdo = g_capital[net][2];
	stat_exp[day_i].n_ops_wdo = g_buy_sell_count[net][2];
	stat_exp[day_i].capital_dol = g_capital[net][3];
	stat_exp[day_i].n_ops_dol = g_buy_sell_count[net][3];

	/*
	printf("day_i=%d; avg_capital=%.2lf; n_ops=%d; WIN_capital=%.2lf; WIN_n_ops=%d; IND_capital=%.2lf; IND_n_ops=%d; WDO_capital=%.2lf;"
			" WDO_n_ops=%d; DOL_capital=%.2lf; DOL_n_ops=%d;\n", stat_exp[day_i].day, stat_exp[day_i].avg_capital,
			stat_exp[day_i].n_ops, stat_exp[day_i].capital_win, stat_exp[day_i].n_ops_win, stat_exp[day_i].capital_ind,
			stat_exp[day_i].n_ops_ind, stat_exp[day_i].capital_wdo, stat_exp[day_i].n_ops_wdo, stat_exp[day_i].capital_dol,
			stat_exp[day_i].n_ops_dol);
	 */
	fflush(stdout);

	output.ival = 0;
	return output;
}

NEURON_OUTPUT
MeanStatisticsExp(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	double total_capital = 0.0;
	double total_capital_win = 0.0;
	double total_capital_ind = 0.0;
	double total_capital_wdo = 0.0;
	double total_capital_dol = 0.0;

	int	n_ops = 0;
	int	n_ops_win = 0;
	int	n_ops_ind = 0;
	int	n_ops_wdo = 0;
	int	n_ops_dol = 0;

	int i;
	int j;
	int stock, n_stocks = INPUT_WIDTH;
	double mean_stat_day[INPUT_WIDTH][13];
	memset(mean_stat_day, 0, INPUT_WIDTH * 13 * sizeof(double));

	for (i = 0; i <= day_i; i++)
	{
		total_capital	+= stat_exp[i].avg_capital;
		total_capital_win += stat_exp[i].capital_win;
		total_capital_ind += stat_exp[i].capital_ind;
		total_capital_wdo += stat_exp[i].capital_wdo;
		total_capital_dol += stat_exp[i].capital_dol;

		n_ops	+= stat_exp[i].n_ops;
		n_ops_win	+= stat_exp[i].n_ops_win;
		n_ops_ind	+= stat_exp[i].n_ops_ind;
		n_ops_wdo	+= stat_exp[i].n_ops_wdo;
		n_ops_dol	+= stat_exp[i].n_ops_dol;

		for (stock = 0; stock < n_stocks; stock++)
		{
			for (j = 0; j < 13; j++)
			{
				mean_stat_day[stock][j] += stat_day[i][stock][j];
			}
		}
	}

	double n = (double)(1.0*day_i+1);
	char symbol[6];

	printf("day_i=Mean; avg_capital=%.2lf; n_ops=%.2lf; ", total_capital/n, (1.0*n_ops)/n);
	for (stock = 0; stock < n_stocks; stock++)
	{
		if (stock == 0) strcpy(symbol, "WIN_");//printf("WIN: ");
		if (stock == 1) strcpy(symbol, "IND_");//printf("IND: ");
		if (stock == 2) strcpy(symbol, "WDO_");//printf("WDO: ");
		if (stock == 3) strcpy(symbol, "DOL_");//printf("DOL: ");

		for (j = 0; j < 9; j++)
		{
			char ch1 = ' ', ch2 = ' ';

			if ((j == 0) || (j == 1) || (j == 2)) ch1 = 'i';
			if ((j == 3) || (j == 4) || (j == 5)) ch1 = '!';
			if ((j == 6) || (j == 7) || (j == 8)) ch1 = '-';

			if ((j == 0) || (j == 3) || (j == 6)) ch2 = 'i';
			if ((j == 1) || (j == 4) || (j == 7)) ch2 = '!';
			if ((j == 2) || (j == 5) || (j == 8)) ch2 = '-';

			printf("%s%c%c=%.2lf; ", symbol, ch1, ch2, mean_stat_day[stock][j]/n);
		}
		printf("%s===%.2lf; ", symbol, mean_stat_day[stock][ACC]/n);
		printf("%sgi=%.1lf; ",symbol, mean_stat_day[stock][GI]/n);
		printf("%sg!=%.1lf; ",symbol,  mean_stat_day[stock][Gi]/n);

		if (stock == 0)
		{
			if (n_ops_win > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_win);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, total_capital_win/n, symbol, (1.0*n_ops_win)/n);
		}
		if (stock == 1)
		{
			if (n_ops_ind > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_ind);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, total_capital_ind/n, symbol, (1.0*n_ops_ind)/n);
		}
		if (stock == 2)
		{
			if (n_ops_wdo > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_wdo);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, total_capital_wdo/n, symbol, (1.0*n_ops_wdo)/n);
		}
		if (stock == 3)
		{
			if (n_ops_dol > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_dol);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, total_capital_dol/n, symbol, (1.0*n_ops_dol)/n);
		}
	}
	printf("\n");
/*
	printf("WIN_capital=%.2lf; WIN_n_ops=%.2lf; IND_capital=%.2lf; "
			"IND_n_ops=%.2lf; WDO_capital=%.2lf; WDO_n_ops=%.2lf; DOL_capital=%.2lf; DOL_n_ops=%.2lf;\n",
			total_capital_win/n, (1.0*n_ops_win)/n, total_capital_ind/n,
			(1.0*n_ops_ind)/n, total_capital_wdo/n, (1.0*n_ops_wdo)/n, total_capital_dol/n, (1.0*n_ops_dol)/n);
*/
	fflush(stdout);

	output.ival = 0;
	return output;
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
* Function: LoadDayFileName
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
LoadDayFileName(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	char *file_name = pParamList->next->param.sval;
	file_name = file_name + 1; // tira uma aspas no inicio
	file_name[strlen(file_name) - 1] = '\0'; // tira aspas do fim

	strcpy(days_file_name, file_name);

	output.ival = 0;
	return output;
}

/*
***********************************************************
* Function: LoadReturns_
* Description:
* Inputs:
* Output:
***********************************************************
*/
NEURON_OUTPUT
LoadReturns2_(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char *returns_file_name = pParamList->next->param.sval;

	returns_file_name = returns_file_name + 1; // tira uma aspas no inicio
	returns_file_name[strlen(returns_file_name) - 1] = '\0'; // tira aspas do fim

	FILE *returns_file;

	g_use_results = 1;
	//printf("%s\n", returns_file_name);

	returns_file = fopen(returns_file_name, "r");

	if (returns_file == NULL)
		Erro("Error: cannot open file in LoadReturns(). returns_file_name = ", returns_file_name, "");

	int numlines = 0;
	char file_line[257];
	char *aux = fgets(file_line, 256, returns_file); // read header
	aux = aux;
	//char date[256];
	char BRT[256];
	int net = 0;
	while(fgets(file_line, 256, returns_file) != NULL)
	{
		if (sscanf(file_line, "%d; %s %s %s %f; %f; %f; %f; %f; %f; %f; %f;\n",
				&returns[net][numlines].id, date, returns[net][numlines].time, BRT,
				&returns[net][numlines].WIN, &returns[net][numlines].result_WIN, &returns[net][numlines].IND, &returns[net][numlines].result_IND, &returns[net][numlines].WDO, &returns[net][numlines].result_WDO, &returns[net][numlines].DOL, &returns[net][numlines].result_DOL) != 12)
		{
			printf("%d; %s %s %s %f; %f; %f; %f; %f; %f; %f; %f;\n",
					returns[net][numlines].id, date, returns[net][numlines].time, BRT,
					returns[net][numlines].WIN, returns[net][numlines].result_WIN, returns[net][numlines].IND, returns[net][numlines].result_IND, returns[net][numlines].WDO, returns[net][numlines].result_WDO, returns[net][numlines].DOL, returns[net][numlines].result_DOL);
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
	return output;
}

int
LoadReturns_(char *returns_file_name)
{
	FILE *returns_file;

	g_use_results = 1;
	//printf("%s\n", returns_file_name);

	returns_file = fopen(returns_file_name, "r");

	if (returns_file == NULL)
		Erro("Error: cannot open file in LoadReturns(). returns_file_name = ", returns_file_name, "");

//	float r_i, p_i, p_i_1;
	int numlines = 0;
	char file_line[257];
	char *aux = fgets(file_line, 256, returns_file); // read header
	aux = aux;
	//char date[256];
	char BRT[256];
	int net = 0;
	while(fgets(file_line, 256, returns_file) != NULL)
	{
		if (sscanf(file_line, "%d; %s %s %s %f; %f; %f; %f; %f; %f; %f; %f;\n",
				&returns[net][numlines].id, date, returns[net][numlines].time, BRT,
				&returns[net][numlines].WIN, &returns[net][numlines].result_WIN, &returns[net][numlines].IND, &returns[net][numlines].result_IND, &returns[net][numlines].WDO, &returns[net][numlines].result_WDO, &returns[net][numlines].DOL, &returns[net][numlines].result_DOL) != 12)
		{
			printf("%d; %s %s %s %f; %f; %f; %f; %f; %f; %f; %f;\n",
					returns[net][numlines].id, date, returns[net][numlines].time, BRT,
					returns[net][numlines].WIN, returns[net][numlines].result_WIN, returns[net][numlines].IND, returns[net][numlines].result_IND, returns[net][numlines].WDO, returns[net][numlines].result_WDO, returns[net][numlines].DOL, returns[net][numlines].result_DOL);
			Erro("Could not read returns, in LoadReturns(), from file: ", returns_file_name, "");
		}

//		r_i = returns[net][numlines].WIN;
//		if (numlines == 0)
//			p_i_1 = 1.0;
//		else
//			p_i_1 = returns[net][numlines - 1].WIN;
//		p_i = r_i * p_i_1 + p_i_1;
//		returns[net][numlines].WIN = p_i;
//
//		r_i = returns[net][numlines].IND;
//		if (numlines == 0)
//			p_i_1 = 1.0;
//		else
//			p_i_1 = returns[net][numlines - 1].IND;
//		p_i = r_i * p_i_1 + p_i_1;
//		returns[net][numlines].IND = p_i;
//
//		r_i = returns[net][numlines].WDO;
//		if (numlines == 0)
//			p_i_1 = 1.0;
//		else
//			p_i_1 = returns[net][numlines - 1].WDO;
//		p_i = r_i * p_i_1 + p_i_1;
//		returns[net][numlines].WDO = p_i;
//
//		r_i = returns[net][numlines].DOL;
//		if (numlines == 0)
//			p_i_1 = 1.0;
//		else
//			p_i_1 = returns[net][numlines - 1].DOL;
//		p_i = r_i * p_i_1 + p_i_1;
//		returns[net][numlines].DOL = p_i;

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

	return 0;
}
/*
***********************************************************
* Function: LoadDay
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
LoadDay(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	day_i = pParamList->next->param.ival;

	FILE *days_file;

	char day_i_file_name[257];
	
	//printf("%s day_i=%d\n", days_file_name, day_i);
	days_file = fopen(days_file_name, "r");

	if (days_file == NULL)
		Erro("Error: cannot open file in LoadDay(). days_file_name = ", days_file_name, "");
	
	int numlines = 0;
	char file_line[257];

	while( numlines <= day_i )
	{
		if (feof(days_file))
		{
			printf("End of days file\n");
			exit(0);
		}
		//printf("%d\n", numlines);
		//fgets(file_line, 256, days_file);
		fscanf(days_file, "%s\n", file_line);
		//printf("%s\n", file_line);
		if ( day_i == numlines )
		{
			if (sscanf(file_line, "%s\n", day_i_file_name) != 1)
			{
				printf("%s\n", day_i_file_name);
				Erro("Could not read day file name, in LoadDay(), from file: ", day_i_file_name, "");
			}
			break;
		}
		numlines++;
	}

	//printf("%s\n", day_i_file_name);
	fclose(days_file);

	LoadReturns_(day_i_file_name);

	output.ival = 0;
	return output;
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
	//char date[256];
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

/*
***********************************************************
* Function: WillStart
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
WillStart(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	//TODO:
	int net = 0;

	//printf("param=%d g_sample=%d\n", param, g_sample);
	//printf("%s\n", returns[net][g_sample].time);
	fflush(stdout);

	int hour = 0;
	int min = 0;
	float sec = 0.0;
	sscanf(returns[net][g_sample].time, "%d:%d:%f", &hour, &min, &sec);

	//printf("h=%d m=%d sec=%f\n", hour, min, sec);
	//printf("g_sample=%d min=%d max=%d\n", g_sample, POSE_MIN, POSE_MAX);

	int ret = 0;
	if (
		( ( hour == TRAIN_HOUR && min >= TRAIN_MIN ) ||
		( hour > TRAIN_HOUR ) )  &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	{
		ret = 1;
		LoadReturnsToInput(&ita, 0, -1);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);
	}

	output.ival = ret;
	return (output);
}

/*
***********************************************************
* Function: WillTrain
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
WillTrain(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	//TODO:
	int net = 0;

	//printf("param=%d g_sample=%d\n", param, g_sample);
	//printf("%s\n", returns[net][g_sample].time);
	fflush(stdout);

	int hour = 0;
	int min = 0;
	float sec = 0.0;
	sscanf(returns[net][g_sample].time, "%d:%d:%f", &hour, &min, &sec);

	//printf("h=%d m=%d sec=%f\n", hour, min, sec);
	//printf("g_sample=%d min=%d max=%d\n", g_sample, POSE_MIN, POSE_MAX);

	int ret = 0;
	if (
		( ( hour == TRAIN_HOUR && min >= TRAIN_MIN ) ||
		( hour > TRAIN_HOUR && hour < TEST_HOUR ) ||
		( hour == TEST_HOUR && min < TEST_MIN ) ) &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	{
		ret = 1;
		LoadReturnsToInput(&ita, 0, -1);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);
	}

	output.ival = ret;
	return (output);
}

/*
***********************************************************
* Function: WillTest
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
WillTest(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	//TODO:
	int net = 0;

	//printf("param=%d g_sample=%d\n", param, g_sample);
	//printf("%s\n", returns[net][g_sample].time);
	fflush(stdout);

	int hour = 0;
	int min = 0;
	float sec = 0.0;
	sscanf(returns[net][g_sample].time, "%d:%d:%f", &hour, &min, &sec);

	//printf("h=%d m=%d sec=%f\n", hour, min, sec);
	//printf("g_sample=%d min=%d max=%d\n", g_sample, POSE_MIN, POSE_MAX);

	int ret = 0;
	if (
		( ( hour == TEST_HOUR && min >= TEST_MIN ) ||
		  ( hour > TEST_HOUR && hour < TEST_END_HOUR ) ||
		  ( hour == TEST_END_HOUR && min <= TEST_END_MIN )
			) &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	{
		ret = 1;
		LoadReturnsToInput(&ita, 0, -1);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);
	}

	output.ival = ret;
	return (output);
}

/*
***********************************************************
* Function: Increment
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
Increment(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int param = pParamList->next->param.ival;

	g_sample += param;

	//printf("g_sample=%d\n", g_sample);
	output.ival = 0;
	return (output);
}

/*
***********************************************************
* Function: GetInputHeight
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT
GetInputHeight(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = INPUT_HEIGHT;

	return (output);
}
