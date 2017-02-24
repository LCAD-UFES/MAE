#include <stdio.h>
#include <wchar.h>
#include "ita_user_functions.h"

typedef struct _time
{
	int year;
	int month;
	int day;

	int hour;
	int min;
	int sec;
	int msec;

	double tstamp;
} TIME;

typedef struct _prc_qty
{
	double prc;
	int qty;
} PRC_QTY;

typedef struct _book
{
	int id;
	TIME time;

	PRC_QTY bid[MAX_BOOK_DEPTH];
	PRC_QTY ask[MAX_BOOK_DEPTH];
} BOOK;

typedef struct _statistics_exp
{
	int day;
	double capital;
	int	n_ops;
	double total_hits;
} STATISTICS_EXP;

#define MAX_DAYS 200
#define MAX_DATA_SAMPLES 43200 // 12h = 12 * 60 * 60

//#define WIN_POINT_VALUE 0.2
//#define IND_POINT_VALUE 1.0
//#define WDO_POINT_VALUE 10.0
//#define DOL_POINT_VALUE 50.0
//
//#define WIN_QTY 50//1
//#define IND_QTY 10//5
//#define WDO_QTY 25//1
//#define DOL_QTY 5

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

#define TRAIN_HOUR 9
#define TRAIN_MIN 30
#define TEST_HOUR 11
#define TEST_MIN 00
#define TEST_END_HOUR 13
#define TEST_END_MIN 30

// Global Variables
BOOK g_book_data[MAX_DATA_SAMPLES];

TIME TRAIN_TIME;
TIME TEST_TIME;
TIME TEST_END_TIME;

int g_n_ops, g_n_hits, g_n_miss;

double g_t[MAX_DATA_SAMPLES];
double g_y[MAX_DATA_SAMPLES];
int g_t_y_count = 0;

STATISTICS_EXP g_stat_exp[MAX_DAYS];
double g_stat_day[MAX_DAYS][13];
int day_i = 0;

int g_sample = 0;
int g_sample_statistics = 0;
int g_nStatus;
int g_LongShort = 1; // 1 = Long, 0 = Short
int POSE_MIN = 0;
int POSE_MAX = 0;

int g_runing_sum_size = STATISTICS;//25;

int gcc_no_complain;
char *gcc_no_complain_c;

// Variaveis utilizadas no procedimento de teste
int g_results[MAX_DATA_SAMPLES][13];
int g_buy_sell_count;
int g_sell_buy_count;
double g_capital;
double g_mean_correct_positive_pred, g_mean_correct_negative_pred;
double g_mean_reverse_positive_pred, g_mean_reverse_negative_pred;

double g_confidence_up, g_confidence_down;
int g_use_confiance = ! USE_STATISTICS;

char g_days_file_name[257];

double
get_time_in_seconds(TIME t)
{
	return ( (t.hour * 3600) + (t.min * 60) + t.sec + (t.msec / 1000.0) );
}

/*
***********************************************************
* Function: sample_previous_period
* Description:
* Inputs:
* Output:
***********************************************************
*/

int
sample_previous_period(int initial_sample)
{
	int i;
	double initial_time = g_book_data[initial_sample].time.tstamp;
	double current_time = 0.0;

	for (i = initial_sample - 1; i >= POSE_MIN; i--)
	{
		current_time = g_book_data[i].time.tstamp;
		if (current_time <= initial_time - PERIOD)
		{
			//printf("intial_time[%d]=%.3lf current_time[%d]=%.3lf\n", g_sample, intial_time, i, current_time);
			return (i);
		}
	}
	return (-1);
}

/*
***********************************************************
* Function: sample_next_period
* Description:
* Inputs:
* Output:
***********************************************************
*/

int
sample_next_period(int initial_sample)
{
	int i;
	double intial_time = g_book_data[initial_sample].time.tstamp;
	double current_time = 0.0;

	for (i = initial_sample + 1; i < POSE_MAX; i++)
	{
		current_time = g_book_data[i].time.tstamp;
		if (current_time >= intial_time + PERIOD)
		{
			//printf("intial_time[%d]=%.3lf current_time[%d]=%.3lf\n", g_sample, intial_time, i, current_time);
			return (i);
		}
	}

	return (-1);
}

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
//		g_sample++;
//		if (g_sample >= POSE_MAX)
//			g_sample = POSE_MIN;
		g_sample = sample_next_period(g_sample);
	}

	if (nDirection == DIRECTION_REWIND)
	{
//		g_sample--;
//		if (g_sample < POSE_MIN)
//			g_sample = POSE_MAX - 1;
		g_sample = sample_previous_period(g_sample);
	}
	printf("g_sample=%d\n", g_sample);
	return (g_sample);
}

/*
***********************************************************
* Function: LoadDataToInput
* Description: Reads returns to the MAE input
* Inputs: input - MAE input
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int
LoadDataToInput(INPUT_DESC *input, INPUT_DESC *input2)
{
	int x, y;
	int y_dimension = input->neuron_layer->dimentions.y;
	int x_dimension = input->neuron_layer->dimentions.x;
	int sample = g_sample;
	int y_center = y_dimension / 2;

	//printf("LOAD INPUT\n");
	//fflush(stdout);
	static int first_time = 1;
	static double first_ask = 0;
	static double first_bid = 0;

	for ( x = 0; x < x_dimension; x++ )
	{
		sample = sample_previous_period(sample);
		if (sample < 0)
		{
			printf("ERROR\n");
			exit(0);
			return (-1);
		}

		//if ( x == 0 )
		//if ( first_time == 1 )
		{
			//first_time = 0;
			first_ask = g_book_data[sample].ask[0].prc;
			first_bid = g_book_data[sample].bid[0].prc;
		}

		for ( y = 0; y < USE_BOOK_DEPTH; y++ )
		{
            int ask_id = y_center + y;
            int bid_id = (y_center - 1) - y;

    		input->neuron_layer->neuron_vector[ask_id * x_dimension + (x_dimension - 1 - x)].output.ival = g_book_data[sample].ask[y].prc - first_ask;
//            input->neuron_layer->neuron_vector[ask_id * x_dimension + (x_dimension - 1 - x)].output.ival = g_book_data[sample].ask[y].prc;
    		input2->neuron_layer->neuron_vector[ask_id * x_dimension + (x_dimension - 1 - x)].output.ival = g_book_data[sample].ask[y].qty;

    		input->neuron_layer->neuron_vector[bid_id * x_dimension + (x_dimension - 1 - x)].output.ival = -g_book_data[sample].bid[y].prc + first_bid;
//    		input->neuron_layer->neuron_vector[bid_id * x_dimension + (x_dimension - 1 - x)].output.ival = g_book_data[sample].bid[y].prc;
    		input2->neuron_layer->neuron_vector[bid_id * x_dimension + (x_dimension - 1 - x)].output.ival = g_book_data[sample].bid[y].qty;
		}
	}

	check_input_bounds(input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_image(input);

	check_input_bounds(input2, input2->wx + input2->ww/2, input2->wy + input2->wh/2);
	input2->up2date = 0;
	update_input_image(input2);

	//printf("\n");
	return (0);
}

/*
***********************************************************
* Function: LoadDataToOutput
* Description:
* Inputs:
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

double
calculate_best_mid_prc(BOOK offer)
{
	return ( offer.ask[0].prc + offer.bid[0].prc ) / 2.0;
}

double
calculate_period_result(void)
{
	double result = 0;
	int sample = sample_previous_period(g_sample);

	double enter_prc = 0.0;
	double exit_prc = 0.0;

	double cost_pts = 0;
	if ( USE_COST == 1 )
	{
		cost_pts = SYMBOL_COST / SYMBOL_POINT_VALUE;
	}

	if ( g_LongShort == 1 )
	{
		enter_prc = calculate_best_mid_prc(g_book_data[sample]);
		exit_prc = calculate_best_mid_prc(g_book_data[g_sample]);
	}
	else
	{
		exit_prc = calculate_best_mid_prc(g_book_data[sample]);
		enter_prc = calculate_best_mid_prc(g_book_data[g_sample]);
	}

	result = exit_prc - enter_prc - 2 * cost_pts;

	return result;
}

int
LoadDataToOutput(OUTPUT_DESC *output)
{
	int x, y;
	int y_dimention = output->neuron_layer->dimentions.y;
	int x_dimention = output->neuron_layer->dimentions.x;

	double res = calculate_period_result();

	//if (g_nStatus == TRAINING_PHASE)
	//	printf("res=%.2lf\n", res);
	//printf("%02d:%02d:%02d.%03d \n", data[g_sample].time.hour, data[g_sample].time.min, data[g_sample].time.sec, data[g_sample].time.msec);
	for (y = 0; y < y_dimention; y++)
	{
		for (x = 0; x < x_dimention; x++)
		{
			output->neuron_layer->neuron_vector[y * x_dimention + x].output.fval = res;
		}
	}

	return (0);
}


float
get_neurons_output(OUTPUT_DESC *output)
{
	int y, x;
	int y_dimension = output->neuron_layer->dimentions.y;
	int x_dimension = output->neuron_layer->dimentions.x;
	float n_up, n_down, n_stable, sum_up, sum_down, result;

	sum_up = sum_down = n_up = n_down = n_stable = 0.0;
	for ( y = 0; y < y_dimension; y++ )
	{
		for ( x = 0; x < x_dimension; x++ )
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
get_neurons_output_confidence(OUTPUT_DESC *output)
{
	int y, x;
	int y_dimension = output->neuron_layer->dimentions.y;
	int x_dimension = output->neuron_layer->dimentions.x;

	float n_up, n_down, n_stable, sum_up, sum_down;

	sum_up = sum_down = n_up = n_down = n_stable = 0.0;
	for ( y = 0; y < y_dimension; y++ )
	{
		for ( x = 0; x < x_dimension; x++ )
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
	}

/*	if ((n_up > n_down))// && (n_up > n_stable))
		result = sum_up / n_up;
	else if ((n_down > n_up))// && (n_down > n_stable))
		result = sum_down / n_down;
	else
		result = 0.0;
*/

	if (n_up == 0)
		g_confidence_up = 0.0;
	else
		g_confidence_up = 100. * (n_up - n_down) / n_up;

	if (n_down == 0)
		g_confidence_down = 0.0;
	else
		g_confidence_down = 100. * (n_down - n_up) / n_down;
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

	int i, k;
	for (k = 0; k < MAX_DATA_SAMPLES; k++)
		for (i = 0; i < 13; i++)
				g_results[k][i] = 0;
	
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
//	GetNextReturns(nDirection);
//	if (LoadDataToInput(input))
//	//if (LoadReturnsToInput(input, 0, -1))
//	//	return (-1);
//
//	check_input_bounds(input, input->wx + input->ww/2, input->wy + input->wh/2);
//	input->up2date = 0;
//	update_input_image(input);
	
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
			if (input->wxd < INPUT_WIDTH / 2)
			{
				GetNewReturns(input, DIRECTION_REWIND);
				//printf("2.4\n");
			}
			else if (input->wxd >= INPUT_WIDTH / 2 + 1)
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

			//ShowStatistics(NULL);
			//ShowStatisticsExp(NULL);

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
	//LoadReturnsToOutput(output, 0);
	LoadDataToOutput(output);

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
r_sum(int signal)
{
	int back_added;
	int sum = 0;

	for (back_added = 0; back_added < g_runing_sum_size; back_added++)
		if ((g_sample_statistics - back_added) >= 0)
			sum += g_results[g_sample_statistics - back_added][signal];

	return (sum);
}


int
f_sum(int parameter)
{
	int i;
	int sum = 0;

	for (i = POSE_MIN; i <= g_sample_statistics; i++)
		sum += g_results[i][parameter];

	return (sum);
}

void
compute_prediction_statistics(OUTPUT_DESC *neural_prediction, OUTPUT_DESC *actual_result)
{
	//	OUTPUT_DESC *result_output;
	//result_output = get_output_by_name("out_result");

	int sample = g_sample_statistics;
	g_sample_statistics++;


	float pred_ret = get_neurons_output(neural_prediction);
	float actu_ret = get_neurons_output(actual_result);

	g_y[g_t_y_count] = pred_ret;
	g_t[g_t_y_count] = actu_ret;

	//printf("pred= %f act= %f \n", pred_ret, actu_ret);
	if (sample > 0)
	{
		int s1, s2;
		s1 = signal_of_val((float) g_results[sample - 1][P_RETURN]);
		s2 = signal_of_val((float) g_results[sample - 1][RETURN]);

		//printf("s1=%d s2=%d \n", s1, s2);
		//			NEURON *result_report = result_output->neuron_layer->neuron_vector;
		//			result_report[stock].output.fval = 0.0;

		if 	((s1 == SUBIU) && (s2 == SUBIU)) {g_results[sample][SS] += 1; /*result_report.output.fval = 1.0;*/}
		else if ((s1 == SUBIU) && (s2 == DESCE))  g_results[sample][SD] += 1;
		else if ((s1 == SUBIU) && (s2 == NEUTR))  g_results[sample][SN] += 1;
		else if ((s1 == DESCE) && (s2 == SUBIU))  g_results[sample][DS] += 1;
		else if ((s1 == DESCE) && (s2 == DESCE)) {g_results[sample][DD] += 1; /*result_report[stock].output.fval = 1.0;*/ }
		else if ((s1 == DESCE) && (s2 == NEUTR))  g_results[sample][DN] += 1;
		else if ((s1 == NEUTR) && (s2 == SUBIU))  g_results[sample][NS] += 1;
		else if ((s1 == NEUTR) && (s2 == DESCE))  g_results[sample][ND] += 1;
		else if ((s1 == NEUTR) && (s2 == NEUTR)) {g_results[sample][NN] += 1; /*result_report[stock].output.fval = -1.0;*/}

		int sum = r_sum(SS) + r_sum(SD) + r_sum(SN);
		if (sum != 0)
		{
			g_mean_correct_positive_pred = 100.0 *
					(double) (3 * r_sum(SS) - 3 * r_sum(SD) - r_sum(SN)) / (double) (3 * sum);

			g_mean_reverse_positive_pred = 100.0 *
					(double) (3 * r_sum(SD) - 3 * r_sum(SS) - r_sum(SN)) / (double) (3 * sum);
		}
		else
		{
			g_mean_correct_positive_pred = -100.0;
			g_mean_reverse_positive_pred = -100.0;
		}

		sum = r_sum(DD) + r_sum(DS) + r_sum(DN);
		if (sum != 0)
		{
			g_mean_correct_negative_pred = 100.0 *
					(double) (3 * r_sum(DD) - 3 * r_sum(DS) - r_sum(DN)) / (double) (3 * sum);

			g_mean_reverse_negative_pred = 100.0 *
					(double) (3 * r_sum(DS) - 3 * r_sum(DD) - r_sum(DN)) / (double) (3 * sum);
		}
		else
		{
			g_mean_correct_negative_pred = -100.0;
			g_mean_reverse_negative_pred = -100.0;
		}
	}

	g_results[sample][P_RETURN] = (int) round(pred_ret * 1000.0);
	g_results[sample][RETURN] = (int) round(actu_ret * 1000.0);
}

double
compute_operation(void)
{
	return 0;
}

double
compute_operation_mid(void)
{
	double enter_prc = 0;
	double exit_prc = 0;
	int sample;

	sample = sample_previous_period(g_sample);

	if ( g_LongShort == 1 )
	//if ( !(g_LongShort == 1) ) // faz operacao ao contrario
	{
		enter_prc = calculate_best_mid_prc(g_book_data[sample]);
		exit_prc = calculate_best_mid_prc(g_book_data[g_sample]);
	}
	else
	{
		exit_prc = calculate_best_mid_prc(g_book_data[sample]);
		enter_prc = calculate_best_mid_prc(g_book_data[g_sample]);
	}

	return exit_prc - enter_prc;
}

void
compute_capital_evolution(OUTPUT_DESC *neural_prediction, OUTPUT_DESC *actual_result)
{
	double cost = 0;

	int s1;
	double r = get_neurons_output(neural_prediction);
	s1 = signal_of_val(r);
	get_neurons_output_confidence(neural_prediction);

	double expected_result_buy_sell = r;
	double expected_result_sell_buy = r;

	double result_buy_sell = 0.0;
	double result_sell_buy = 0.0;

	if ( USE_SPREAD == 0 )
	{
		r = compute_operation_mid();
	}
	else
	{
		r = compute_operation();
	}

	cost = SYMBOL_COST;
	double point_value = SYMBOL_POINT_VALUE;
	int qty = SYMBOL_QTY;

	if ( USE_COST == 0 )
		cost = 0;

	result_buy_sell = ((r * point_value - 2 * cost) * qty);
	result_sell_buy = ((r * point_value - 2 * cost) * qty);

	//printf("%d\n", g_use_confiance);
	if (g_LongShort == 1)
	{
		if ( ( (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_confidence_up > CERTAINTY) && (g_use_confiance == 1) ) ||
				( g_runing_sum_size > 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_mean_correct_positive_pred > CERTAINTY) && (g_use_confiance == 0) ) ||
				( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) )
		)
		{
			double previous_capital = g_capital;
			g_capital += result_buy_sell;
			g_capital += result_buy_sell;

			if (g_capital > previous_capital)
				g_results[g_sample_statistics-1][HITS] += 1;

			g_buy_sell_count += 1;
			g_results[g_sample_statistics-1][INVEST] = 1;
		}
		else
			g_results[g_sample_statistics-1][INVEST] = 0;
	}
	else
	{
		if ( ( (s1 == 0) && (expected_result_sell_buy > 0.0) && (g_confidence_up > CERTAINTY) && (g_use_confiance == 1) ) ||
				( g_runing_sum_size > 0 &&  (s1 == 0) && (expected_result_sell_buy > 0.0) && (g_mean_correct_positive_pred > CERTAINTY) && (g_use_confiance == 0) ) ||
				( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_sell_buy > 0.0) )
		)
		{
			double previous_capital = g_capital;
			g_capital += result_sell_buy;
			g_capital += result_sell_buy;

			if (g_capital > previous_capital)
				g_results[g_sample_statistics-1][HITS] += 1;

			g_sell_buy_count += 1;
			g_results[g_sample_statistics-1][INVEST] = 1;
		}
		else
			g_results[g_sample_statistics-1][INVEST] = 0;
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
	OUTPUT_DESC *actual_result;

	actual_result = get_output_by_name(out_test.name);
	read_current_desired_returns_to_output(actual_result);

	compute_prediction_statistics(output, actual_result);
	if (g_nStatus == TEST_PHASE)
		compute_capital_evolution(output, actual_result);

	g_t_y_count++;
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
	//printf("output_handler\n");

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
	OUTPUT_DESC* filter_out = get_output_by_name(out_ita_lp_f.name);
	OUTPUT_DESC* output_pred = get_output_by_name(out_prediction.name);
	double pred = 0;
	for(x = 0; x < filter_out->neuron_layer->dimentions.x; x++)
	{
		for (y = 0; y < filter_out->neuron_layer->dimentions.y; y++)
		{
			printf("%d ", filter_out->neuron_layer->neuron_vector[y * filter_out->neuron_layer->dimentions.x + x ].output.ival);
		}
	}
	pred = get_neurons_output(output_pred);
	printf("\n %f \n", pred);
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

NEURON_OUTPUT
ShowStatistics(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i, k, total_tested;

	printf("%04d-%02d-%02d %02d:%02d:%02d\n",
			g_book_data[g_sample].time.year, g_book_data[g_sample].time.month, g_book_data[g_sample].time.day,
			g_book_data[g_sample].time.hour, g_book_data[g_sample].time.min, g_book_data[g_sample].time.sec);

	total_tested = 0;
	for (k = 0; k < g_runing_sum_size; k++)
		//for (k = 0; k < g_sample_statistics; k++)
		for (i = 0; i < 9; i++)
			if ((g_sample - k) >= 0)
				total_tested += g_results[k][i];

	if (total_tested > 0)
	{
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
					ch1, ch2, 100.0 * (double) r_sum(i) / (double) total_tested);
		}
		printf("== = %6.2lf:  ", 100.0 * (double) (r_sum(SS) + r_sum(DD) + r_sum(NN)) / (double) total_tested);

		int sum = r_sum(SS) + r_sum(SD) + r_sum(SN);
		if (sum != 0)
			printf("gi %6.1lf  ", 100.0 *
					(double) (3 * r_sum(SS) - 3 * r_sum(SD) - r_sum(SN)) / (double) (3 * sum));
		else
			printf("gi   ---   ");

		sum = r_sum(DD) + r_sum(DS) + r_sum(DN);
		if (sum != 0)
			printf("g! %6.1lf  ",100.0 *
					(double) (3 * r_sum(DD) - 3 * r_sum(DS) - r_sum(DN)) / (double) (3 * sum));
		else
			printf("g!   ---  ");

		if ((g_buy_sell_count + g_sell_buy_count) > 0)
			printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = %6.1lf, g_confidence_up=%.2lf\n",
					g_buy_sell_count, g_sell_buy_count, g_capital,
					(double) g_results[g_sample_statistics-1][RETURN] / 1000.0, (double) g_results[g_sample_statistics-1][P_RETURN] / 1000.0,
					g_results[g_sample_statistics-1][INVEST],
					100.0 * (double) f_sum(HITS) / (double) (g_buy_sell_count + g_sell_buy_count), g_confidence_up);
		else
			printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = ---, g_confidence_up=%.2lf\n",
					g_buy_sell_count, g_sell_buy_count, g_capital,
					(double) g_results[g_sample_statistics-1][RETURN] / 1000.0, (double) g_results[g_sample_statistics-1][P_RETURN] / 1000.0,
					g_results[g_sample_statistics-1][INVEST], g_confidence_up);
	}

	//printf("total_tested = %d, capital = %.2lf\n", total_tested, capital);
	printf("num_periods_with_operations = %d, total_buy_sell = %d, total_sell_buy = %d, average_final_capital = %.2lf\n",
			total_tested, g_buy_sell_count, g_sell_buy_count, g_capital);
	fflush(stdout);

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT
ShowStatisticsExp(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	//g_stat_day

	//--------------- stat day

	int i, total_tested;
	double total_hits = 0;

	total_tested = 0;
	for ( i = 0; i < 9; i++ )
		total_tested += f_sum(i);

	if (total_tested > 0)
	{
		for (i = 0; i < 9; i++)
		{
			g_stat_day[day_i][i] =  100.0 * (double) f_sum(i) / (double) total_tested;
		}
		g_stat_day[day_i][ACC] = 100.0 * (double) (f_sum(SS) + f_sum(DD) + f_sum(NN)) / (double) total_tested;

		int sum = f_sum(SS) + f_sum(SD) + f_sum(SN);
		if (sum != 0)
		{
			g_stat_day[day_i][GI] = 100.0 *
					(double) (3 * f_sum(SS) - 3 * f_sum(SD) - f_sum(SN)) / (double) (3 * sum);
		}
		else
		{
			g_stat_day[day_i][GI] = 0.0;
		}

		sum = f_sum(DD) + f_sum(DS) + f_sum(DN);
		if (sum != 0)
		{
			g_stat_day[day_i][Gi] = 100.0 *
					(double) (3 * f_sum(DD) - 3 * f_sum(DS) - f_sum(DN)) / (double) (3 * sum);
		}
		else
		{
			//printf("g!   ---  ");
			g_stat_day[day_i][Gi] = 0.0;
		}

		if ((g_buy_sell_count + g_sell_buy_count) > 0)
		{
			g_stat_day[day_i][HITS] = (double) f_sum(HITS) ;
			total_hits += g_stat_day[day_i][HITS];
		}
		else
		{
			g_stat_day[day_i][HITS] = 0.0;
		}
	}



	//--------------- capital day

	//printf("STAT DAY\n");

	g_stat_exp[day_i].day = day_i;
	g_stat_exp[day_i].capital = g_capital;
	g_stat_exp[day_i].n_ops = g_LongShort == 1 ? g_buy_sell_count : g_sell_buy_count;
	g_stat_exp[day_i].total_hits = total_hits;

	printf("day_i=%d; capital=%.2lf; n_ops=%d; hit_rate=%.1lf; ",
			day_i, g_stat_exp[day_i].capital, g_stat_exp[day_i].n_ops,
			100.0 * g_stat_exp[day_i].total_hits / g_stat_exp[day_i].n_ops);

	for (i = 0; i < 9; i++)
	{
		char ch1 = ' ', ch2 = ' ';

		if ((i == 0) || (i == 1) || (i == 2)) ch1 = 'i';
		if ((i == 3) || (i == 4) || (i == 5)) ch1 = '!';
		if ((i == 6) || (i == 7) || (i == 8)) ch1 = '-';

		if ((i == 0) || (i == 3) || (i == 6)) ch2 = 'i';
		if ((i == 1) || (i == 4) || (i == 7)) ch2 = '!';
		if ((i == 2) || (i == 5) || (i == 8)) ch2 = '-';

		printf("%c%c=%.2lf; ", ch1, ch2, g_stat_day[day_i][i]);
	}
	printf("===%.2lf; ", g_stat_day[day_i][ACC]);
	printf("gi=%.1lf; ", g_stat_day[day_i][GI]);
	printf("g!=%.1lf; ", g_stat_day[day_i][Gi]);
	if (g_stat_exp[day_i].n_ops > 0)
		printf("hit_rate=%.1lf; ", 100.0 * g_stat_day[day_i][HITS]/g_stat_exp[day_i].n_ops);
	else
		printf("hit_rate=--; ");

	printf("\n");
	fflush(stdout);

	output.ival = 0;
	return output;
}

NEURON_OUTPUT
MeanStatisticsExp(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	double n = (double)(1.0*day_i+1);
	int param = pParamList->next->param.ival;
	if (param != 0)
		n = (double)(1.0 * param);

	double total_capital = 0.0;
	double total_hits = 0.0;
	int	n_ops = 0;

	int i;
	int j;
	double mean_stat_day[13];
	memset(mean_stat_day, 0, 13 * sizeof(double));

	for (i = 0; i <= day_i; i++)
	{
		total_capital	+= g_stat_exp[i].capital;
		total_hits += g_stat_exp[i].total_hits;
		n_ops	+= g_stat_exp[i].n_ops;

		for (j = 0; j < 13; j++)
		{
			mean_stat_day[j] += g_stat_day[i][j];
		}
	}

	printf("day_i=Mean; avg_capital=%.2lf; n_ops=%.2lf; hit_rate=%.1lf; ",
			total_capital/n, (1.0*n_ops)/n, 100.0 * total_hits/n_ops);


	for (j = 0; j < 9; j++)
	{
		char ch1 = ' ', ch2 = ' ';

		if ((j == 0) || (j == 1) || (j == 2)) ch1 = 'i';
		if ((j == 3) || (j == 4) || (j == 5)) ch1 = '!';
		if ((j == 6) || (j == 7) || (j == 8)) ch1 = '-';

		if ((j == 0) || (j == 3) || (j == 6)) ch2 = 'i';
		if ((j == 1) || (j == 4) || (j == 7)) ch2 = '!';
		if ((j == 2) || (j == 5) || (j == 8)) ch2 = '-';

		printf("%c%c=%.2lf; ", ch1, ch2, mean_stat_day[j]/n);
	}
	printf("===%.2lf; ", mean_stat_day[ACC]/n);
	printf("gi=%.1lf; ", mean_stat_day[GI]/n);
	printf("g!=%.1lf; ", mean_stat_day[Gi]/n);

	printf("\n");

	fflush(stdout);

//	FILE* f = fopen("wnn_book_caco.csv", "w+");
//	fprintf(f, "Target; Output;\n");
//	for (i = 0; i < g_t_y_count; i++)
//	{
//		fprintf(f, "%lf; %lf;\n", g_t[i], g_y[i]);
//	}
//	fclose(f);

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

	int i, k;

	g_capital = 125000.0;
	g_buy_sell_count = 0;
	g_sell_buy_count = 0;
	for (k = 0; k < MAX_DATA_SAMPLES; k++)
	{
		if ( g_t_y_count == 0 )
		{
			g_t[k] = 0;
			g_y[k] = 0;
		}
		for (i = 0; i < 13; i++)
			g_results[k][i] = 0;
	}

	g_n_ops= 0;
	g_n_hits = 0;
	g_n_miss = 0;
	g_capital = 125000.0;

	g_sample = POSE_MIN = 0; // ita.wh;
	g_sample_statistics = 0;
	//POSE_MAX = 10000;

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
	
//	g_sample = pParamList->next->param.ival;
//	if ((g_sample < POSE_MIN) || (g_sample >= POSE_MAX))
//	{
//		printf ("Error: Invalid pose ID, it must be within the interval [%d, %d] (GetRandomReturns).\n", POSE_MIN, POSE_MAX - 1);
//		fflush(stdout);
//		output.ival = -1;
//		return (output);
//	}
//
//	//if (LoadReturnsToInput(&ita, 0, -1))
//	if (LoadDataToInput(&ita))
//	{
//		printf ("Error: Cannot read return (GetRandomReturns).\n");
//		output.ival = -1;
//		fflush(stdout);
//		return (output);
//	}
//
//	check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
//	ita.up2date = 0;
//	update_input_image(&ita);
//
//	output.ival = 0;
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
	output.ival = 0;

	int ival = pParamList->next->param.ival;

	if (ival == 0)
		strcpy(g_days_file_name, FILE_TRAIN);
	else if (ival == 1)
		strcpy(g_days_file_name, FILE_TEST);
	else
	{
		Erro("Error: Parameter must be INT: 0 -> TRAIN or 1 -> TEST ", "", "");
		output.ival = -1;
	}

	return output;
}

/*
***********************************************************
* Function: load_book_data
* Description:
* Inputs:
* Output:
***********************************************************
*/
void
update_time_vars(void)
{
	// update TRAIN_TIME, TEST_TIME and TEST_END_TIME variables
	TRAIN_TIME.year = g_book_data[POSE_MIN].time.year;
	TRAIN_TIME.month = g_book_data[POSE_MIN].time.month;
	TRAIN_TIME.day = g_book_data[POSE_MIN].time.day;
	TRAIN_TIME.hour = TRAIN_HOUR;
	TRAIN_TIME.min = TRAIN_MIN;
	TRAIN_TIME.sec = 0;
	TRAIN_TIME.msec = 0;
	TRAIN_TIME.tstamp = get_time_in_seconds(TRAIN_TIME);

	TEST_TIME.year = g_book_data[POSE_MIN].time.year;
	TEST_TIME.month = g_book_data[POSE_MIN].time.month;
	TEST_TIME.day = g_book_data[POSE_MIN].time.day;
	TEST_TIME.hour = TEST_HOUR;
	TEST_TIME.min = TEST_MIN;
	TEST_TIME.sec = 0;
	TEST_TIME.msec = 0;
	TEST_TIME.tstamp = get_time_in_seconds(TEST_TIME);

	TEST_END_TIME.year = g_book_data[POSE_MIN].time.year;
	TEST_END_TIME.month = g_book_data[POSE_MIN].time.month;
	TEST_END_TIME.day = g_book_data[POSE_MIN].time.day;
	TEST_END_TIME.hour = TEST_END_HOUR;
	TEST_END_TIME.min = TEST_END_MIN;
	TEST_END_TIME.sec = 0;
	TEST_END_TIME.msec = 0;
	TEST_END_TIME.tstamp = get_time_in_seconds(TEST_END_TIME);
}

int
load_book_data(char *f_name)
{
	FILE *fp;

	fp = fopen(f_name, "r");
	if (fp == NULL)
	{
		Erro("Error: cannot open file in LoadData(). f_name = ", f_name, "");
		return -1;
	}

	int i;
	int numlines = 0;
	char f_line[MAX_BOOK_DEPTH * 100];
	char BRT[257];

	fscanf(fp, "%[^\n]s", f_line); // header

	while(fgets(f_line, sizeof(f_line), fp))
	{

		char* tokens = strtok(f_line, " ;\n");
		if (tokens != NULL)
		{

			sscanf(tokens, "%d", &g_book_data[numlines].id);
			tokens = strtok (NULL, " ;\n");
			sscanf(tokens, "%d-%d-%d",
					&g_book_data[numlines].time.year, &g_book_data[numlines].time.month,
					&g_book_data[numlines].time.day);

			tokens = strtok (NULL, " ;\n");
			sscanf(tokens, "%d:%d:%d.%d",
					&g_book_data[numlines].time.hour, &g_book_data[numlines].time.min,
					&g_book_data[numlines].time.sec, &g_book_data[numlines].time.msec);

			tokens = strtok (NULL, " ;\n");
			sscanf(tokens, "%s", BRT);

			g_book_data[numlines].time.tstamp = get_time_in_seconds(g_book_data[numlines].time);

			// read bid[i].qty; bid[i].prc;
			for ( i = MAX_BOOK_DEPTH - 1; i >= 0; i-- )
			{
				tokens = strtok (NULL, " ;\n");
				sscanf(tokens, "%d", &g_book_data[numlines].bid[i].qty);
				tokens = strtok (NULL, " ;\n");
				sscanf(tokens, "%lf", &g_book_data[numlines].bid[i].prc);
			}

			// read ask[i].prc; bid[i].qty;
			for ( i = 0; i < MAX_BOOK_DEPTH; i++ )
			{
				tokens = strtok (NULL, " ;\n");
				sscanf(tokens, "%lf", &g_book_data[numlines].ask[i].prc);
				tokens = strtok (NULL, " ;\n");
				sscanf(tokens, "%d", &g_book_data[numlines].ask[i].qty);
			}
		}

//		printf("id = %d:\n", book_data[numlines].id);
//		for ( i = 0; i < BOOK_DEPTH; i++ )
//		{
//			printf("%02d -> %d; %.2lf; %.2lf; %d;\n", i, book_data[numlines].bid[i].qty, book_data[numlines].bid[i].prc,
//					book_data[numlines].ask[i].prc, book_data[numlines].ask[i].qty);
//		}
//		printf("\n\n");
//
//		if (numlines == 25)
//			exit(0);

		numlines++;
		if (numlines >= MAX_DATA_SAMPLES)
		{
			Erro("numlines >= MAX_DATA_SAMPLES in load_book_data().", "", "");
			return -1;
		}
	}

	POSE_MAX = numlines - 1;
	g_sample = POSE_MIN = 0;

	update_time_vars();

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
	
	days_file = fopen(g_days_file_name, "r");

	if (days_file == NULL)
		Erro("Error: cannot open file in LoadDay(). days_file_name = ", g_days_file_name, "");
	
	int numlines = 0;
	char file_line[257];

	while( numlines <= day_i )
	{
		if (feof(days_file))
		{
			printf("End of days file\n");
			exit(0);
		}

		fscanf(days_file, "%s\n", file_line);
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

	fclose(days_file);

	printf("%s\n", day_i_file_name);
	output.ival = load_book_data(day_i_file_name);

	return output;
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

	//int hour = g_book_data[g_sample].time.hour;
	//int min = g_book_data[g_sample].time.min;
	//int sec = book_data[g_sample].time.sec;
	//printf("g_sample=%d h=%d m=%d sec=%d\n", g_sample, hour, min, sec);

	int ret = 0;
	/*if (
		( ( hour == TRAIN_HOUR && min >= TRAIN_MIN ) ||
		( hour > TRAIN_HOUR ) )  &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	*/
	if ( ( g_book_data[g_sample].time.tstamp >= TRAIN_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		LoadDataToInput(&ita, &ita2);
		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);

		check_input_bounds(&ita2, ita2.wx + ita2.ww/2, ita2.wy + ita2.wh/2);
		ita2.up2date = 0;
		update_input_image(&ita2);
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

	//int hour = g_book_data[g_sample].time.hour;
	//int min = g_book_data[g_sample].time.min;
	//int sec = g_book_data[g_sample].time.sec;
	//printf("h=%d m=%d sec=%f\n", hour, min, sec);

	//TODO: REVER ESSES IFs NAS FUNCOES DE TEMPO PQ TEM PROBLEMA NELES
	int ret = 0;
	//if (
	//	( (( hour == TRAIN_HOUR && min >= TRAIN_MIN )/* && (hour <= TEST_HOUR && min <= TEST_MIN )*/ )||
	//	( hour > TRAIN_HOUR && hour < TEST_HOUR ) ||
	//	( hour == TEST_HOUR && min < TEST_MIN ) ) &&
	//	g_sample <= POSE_MAX && g_sample >= POSE_MIN
	//	)
	if ( ( g_book_data[g_sample].time.tstamp >= TRAIN_TIME.tstamp ) &&
		 ( g_book_data[g_sample].time.tstamp < TEST_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		LoadDataToInput(&ita, &ita2);
		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);

		check_input_bounds(&ita2, ita2.wx + ita2.ww/2, ita2.wy + ita2.wh/2);
		ita2.up2date = 0;
		update_input_image(&ita2);
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

	//int hour = g_book_data[g_sample].time.hour;
	//int min = g_book_data[g_sample].time.min;
	//int sec = g_book_data[g_sample].time.sec;
	//printf("h=%d m=%d sec=%d\n", hour, min, sec);

	int ret = 0;
	//if (
	//	( (( hour == TEST_HOUR && min >= TEST_MIN )/* && ( hour < TEST_END_HOUR && min <= TEST_END_MIN )*/)||
	//	  ( hour > TEST_HOUR && hour < TEST_END_HOUR ) ||
	//	  ( hour == TEST_END_HOUR && min <= TEST_END_MIN )
	//		) &&
	//	g_sample <= POSE_MAX && g_sample >= POSE_MIN
	//	)
	if ( ( g_book_data[g_sample].time.tstamp >= TEST_TIME.tstamp ) &&
		 ( g_book_data[g_sample].time.tstamp < TEST_END_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		LoadDataToInput(&ita, &ita2);
		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

		check_input_bounds(&ita, ita.wx + ita.ww/2, ita.wy + ita.wh/2);
		ita.up2date = 0;
		update_input_image(&ita);

		check_input_bounds(&ita2, ita2.wx + ita2.ww/2, ita2.wy + ita2.wh/2);
		ita2.up2date = 0;
		update_input_image(&ita2);
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
	int i;

	if (param <= 0)
	{
		g_sample = 0;
	}
	else
	{
		for (i = 0; i < param; i++)
			g_sample = sample_next_period(g_sample);
	}

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
//
//NEURON_OUTPUT
//GetInputHeight(PARAM_LIST *pParamList)
//{
//	NEURON_OUTPUT output;
//
//	output.ival = INPUT_HEIGHT;
//
//	return (output);
//}
