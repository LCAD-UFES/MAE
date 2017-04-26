#include <stdio.h>
#include <wchar.h>
#include "ita_user_functions.h"
#include <sys/time.h>

#if ITA_BUILD
#include "../ita_signal_driver_vg_ram.h"
#include "../ita_signal_driver_vg_ram_mae.h"
#include "../ita_signal_driver_vg_ram_statistics.h"
#endif

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

typedef struct _prc
{
	double buy;		// ASK side
	double sell;	// BID side
	double mid;		// = (buy + sell) / 2.0
} PRC;

typedef struct _data
{
	int id;
	TIME time;

	PRC WIN;
	PRC IND;
	PRC WDO;
	PRC DOL;
} DATA;

#define WIN_POINT_VALUE 0.2
#define IND_POINT_VALUE 1.0
#define WDO_POINT_VALUE 10.0
#define DOL_POINT_VALUE 50.0

#define WIN_QTY 50//1
#define IND_QTY 10//5
#define WDO_QTY 25//1
#define DOL_QTY 5

#define MAX_DATA_SAMPLES 43200 // 12h = 12 * 60 * 60
DATA data[MAX_DATA_SAMPLES];

TIME TRAIN_TIME;
TIME TEST_TIME;
TIME TEST_END_TIME;

double g_anchor[INPUT_WIDTH];
int g_enable_operation[INPUT_WIDTH];
int g_last_close_operation_sample[INPUT_WIDTH];
int g_n_ops[INPUT_WIDTH], g_n_hits[INPUT_WIDTH], g_n_miss[INPUT_WIDTH];

double g_t[INPUT_WIDTH][MAX_DATA_SAMPLES];
double g_y[INPUT_WIDTH][MAX_DATA_SAMPLES];
int g_t_y_count = 0;

typedef struct _statistics_exp
{
	int day;
	double avg_capital;
	double geral_capital;
	int	n_ops;
	double total_hits;
	double capital_win;
	int	n_ops_win;
	double capital_ind;
	int	n_ops_ind;
	double capital_wdo;
	int	n_ops_wdo;
	double capital_dol;
	int	n_ops_dol;	
} STATISTICS_EXP;

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

#define BEGIN_CAPITAL 125000.0

// PAPER SBPO 2017
#define TRAIN_HOUR 9
#define TRAIN_MIN 30
#define TEST_HOUR 13
#define TEST_MIN 30
#define TEST_END_HOUR 17
#define TEST_END_MIN 30

#define MAX_DAYS 200
// Global Variables
STATISTICS_EXP stat_exp[MAX_DAYS];
double stat_day[MAX_DAYS][INPUT_WIDTH][13];
int day_i = 0;
int g_use_results = 0;
char date[256];

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
int g_results[2][MAX_DATA_SAMPLES][INPUT_WIDTH][13];
int g_buy_sell_count[2][INPUT_WIDTH];
int g_sell_buy_count[2][INPUT_WIDTH];
double g_capital[2][INPUT_WIDTH+1];
double g_mean_correct_positive_pred[2][INPUT_WIDTH], g_mean_correct_negative_pred[2][INPUT_WIDTH];
double g_mean_reverse_positive_pred[2][INPUT_WIDTH], g_mean_reverse_negative_pred[2][INPUT_WIDTH];

double g_confidence_up[2][INPUT_WIDTH], g_confidence_down[2][INPUT_WIDTH];
int g_use_confiance = ! USE_STATISTICS;

char days_file_name[257];

int g_n_period_train[INPUT_WIDTH];
int g_n_period_test[INPUT_WIDTH];
int g_n_period_train_op[INPUT_WIDTH];
int g_n_period_test_op[INPUT_WIDTH];

//int
//GetMAESampleIndex(void)
//{
//	return (g_sample);
//}
//
//
//void
//SetSymbolReturn(int symbol, float symbol_return, int net, int displacement)
//{
//	if (symbol == 0) returns[net][g_sample + displacement].WIN = symbol_return;
//	if (symbol == 1) returns[net][g_sample + displacement].IND = symbol_return;
//	if (symbol == 2) returns[net][g_sample + displacement].WDO = symbol_return;
//	if (symbol == 3) returns[net][g_sample + displacement].DOL = symbol_return;
//}
//
//
//void
//SetSampleIdAndTime(int net, int displacement, int id, char *time)
//{
//	returns[net][g_sample + displacement].id = id;
//	strcpy(returns[net][g_sample + displacement].time, time);
//}
//
//
//double
//GetSymbolReturn(int symbol, int net, int displacement)
//{
//	if (symbol == 0) return ((double) returns[net][g_sample + displacement].WIN);
//	if (symbol == 1) return ((double) returns[net][g_sample + displacement].IND);
//	if (symbol == 2) return ((double) returns[net][g_sample + displacement].WDO);
//	if (symbol == 3) return ((double) returns[net][g_sample + displacement].DOL);
//
//	return (0.0); // This should never occur.
//}

double
GetTimeInSeconds(TIME t)
{
	return ( (t.hour * 3600) + (t.min * 60) + t.sec + (t.msec / 1000.0) );
}

/*
***********************************************************
* Function: SamplePreviousPeriod
* Description:
* Inputs:
* Output:
***********************************************************
*/

int
SamplePreviousPeriod(int initial_sample)
{
	int i;
	double initial_time = GetTimeInSeconds(data[initial_sample].time);
	double current_time = 0.0;

	for (i = initial_sample - 1; i > POSE_MIN; i--)
	{
		current_time = data[i].time.tstamp;//GetTimeInSeconds(data[i].time);
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
* Function: SampleNextPeriod
* Description:
* Inputs:
* Output:
***********************************************************
*/

int
SampleNextPeriod(int initial_sample)
{
	int i;
	double intial_time = GetTimeInSeconds(data[initial_sample].time);
	double current_time = 0.0;

	for (i = initial_sample + 1; i < POSE_MAX; i++)
	{
		current_time = data[i].time.tstamp;// GetTimeInSeconds(data[i].time);
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
		g_sample = SampleNextPeriod(g_sample);
	}

	if (nDirection == DIRECTION_REWIND)
	{
//		g_sample--;
//		if (g_sample < POSE_MIN)
//			g_sample = POSE_MAX - 1;
		g_sample = SamplePreviousPeriod(g_sample);
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
LoadDataToInput(INPUT_DESC *input)
{
	int x, y;
	int y_dimention = input->neuron_layer->dimentions.y;
	int x_dimention = input->neuron_layer->dimentions.x;
	int sample = g_sample;
	//double div_const = 100.0;

	for (y = 0; y < y_dimention; y++)
	{
		//sample = SampleNextPeriod(sample);
		sample = SamplePreviousPeriod(sample);
		if (sample < 0)
		{
			//printf("ERROR\n");
			//exit(0);
			return (-1);
		}

		//printf("%02d:%02d:%02d.%03d ", data[sample].time.hour, data[sample].time.min, data[sample].time.sec, data[sample].time.msec);
		for (x = 0; x < x_dimention; x++)
		{
			if (x == 0)
				input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval =
					data[sample].WIN.mid;
			if (x == 1)
				input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval =
					data[sample].IND.mid;
			if (x == 2)
				input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval =
					data[sample].WDO.mid;
			if (x == 3)
				input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval =
					data[sample].DOL.mid;
		}
	}

	//for (x = 0; x < x_dimention; x++)
	//	g_anchor[x] = input->neuron_layer->neuron_vector[(y_dimention - 1) * x_dimention + x].output.fval;

	for (y = 0; y < y_dimention; y++)
	{
		for (x = 0; x < x_dimention; x++)
		{
			input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval -= input->neuron_layer->neuron_vector[(y_dimention - 1) * x_dimention + x].output.fval;
			//input->neuron_layer->neuron_vector[y * x_dimention + x].output.fval *= 1000.0 / g_anchor[x];
		}
	}
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
CalculatePeriodReturn(int stock)
{
	double ret = 0;
	int sample = SamplePreviousPeriod(g_sample);

	double enter_prc = 0.0;
	double exit_prc = 0.0;

	double cost_pts = 0;

	if ( stock == 0 ) { /*ref_prc = data[sample].WIN.mid; */cost_pts = COST_QTY_WIN_WDO / WIN_POINT_VALUE; }
	if ( stock == 1 ) { /*ref_prc = data[sample].IND.mid; */cost_pts = COST_QTY_IND_DOL / IND_POINT_VALUE; }
	if ( stock == 2 ) { /*ref_prc = data[sample].WDO.mid; */cost_pts = COST_QTY_WIN_WDO / WDO_POINT_VALUE; }
	if ( stock == 3 ) { /*ref_prc = data[sample].DOL.mid; */cost_pts = COST_QTY_IND_DOL / DOL_POINT_VALUE; }

	if ( stock == 0 )
	{
		if ( g_LongShort == 1 )
		{
			enter_prc = data[sample].WIN.mid;
			exit_prc = data[g_sample].WIN.mid;
		}
		else
		{
			exit_prc = data[sample].WIN.mid;
			enter_prc = data[g_sample].WIN.mid;
		}
	}
	if ( stock == 1 )
	{
		if ( g_LongShort == 1 )
		{
			enter_prc = data[sample].IND.mid;
			exit_prc = data[g_sample].IND.mid;
		}
		else
		{
			exit_prc = data[sample].IND.mid;
			enter_prc = data[g_sample].IND.mid;
		}
	}
	if ( stock == 2 )
	{
		if ( g_LongShort == 1 )
		{
			enter_prc = data[sample].WDO.mid;
			exit_prc = data[g_sample].WDO.mid;
		}
		else
		{
			exit_prc = data[sample].WDO.mid;
			enter_prc = data[g_sample].WDO.mid;
		}
	}
	if ( stock == 3 )
	{
		if ( g_LongShort == 1 )
		{
			enter_prc = data[sample].DOL.mid;
			exit_prc = data[g_sample].DOL.mid;
		}
		else
		{
			exit_prc = data[sample].DOL.mid;
			enter_prc = data[g_sample].DOL.mid;
		}
	}

	if ( USE_COST == 1 )
		ret = exit_prc - enter_prc - 2 * cost_pts;
	else
		ret = exit_prc - enter_prc;

	return ret;
}

int
LoadDataToOutput(OUTPUT_DESC *output)
{
	int x, y;
	int y_dimention = output->neuron_layer->dimentions.y;
	int x_dimention = output->neuron_layer->dimentions.x;

	double win_ret = CalculatePeriodReturn(0);
	double ind_ret = CalculatePeriodReturn(1);
	double wdo_ret = CalculatePeriodReturn(2);
	double dol_ret = CalculatePeriodReturn(3);

	//printf("%02d:%02d:%02d.%03d \n", data[g_sample].time.hour, data[g_sample].time.min, data[g_sample].time.sec, data[g_sample].time.msec);

//	FILE *fp = fopen("data_10_256s.csv", "a+");
//	fprintf(fp, "%04d%02d%02d-%02d:%02d:%02d; %lf; %lf; %lf; %lf;\n",
//			data[g_sample].time.year, data[g_sample].time.month, data[g_sample].time.day,
//			data[g_sample].time.hour, data[g_sample].time.min, data[g_sample].time.sec,
//			win_ret, ind_ret, wdo_ret, dol_ret);
//	fclose(fp);

	for (y = 0; y < y_dimention; y++)
	{
		for (x = 0; x < x_dimention; x++)
		{
			if (x == 0)
			{
				output->neuron_layer->neuron_vector[y * x_dimention + x].output.fval = win_ret;
			}
			if (x == 1)
			{
				output->neuron_layer->neuron_vector[y * x_dimention + x].output.fval = ind_ret;
			}
			if (x == 2)
			{
				output->neuron_layer->neuron_vector[y * x_dimention + x].output.fval = wdo_ret;
			}
			if (x == 3)
			{
				output->neuron_layer->neuron_vector[y * x_dimention + x].output.fval = dol_ret;
			}
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

	if (n_up == 0)
		g_confidence_up[0][x] = 0.0;
	else
		g_confidence_up[0][x] = 100. * (n_up - n_down) / n_up;

	if (n_down == 0)
		g_confidence_down[0][x] = 0.0;
	else
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
		for (k = 0; k < MAX_DATA_SAMPLES; k++)
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
	if (LoadDataToInput(input))
	//if (LoadReturnsToInput(input, 0, -1))
	//	return (-1);
			
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
r_sum(int net, int stock, int signal)
{
	int back_added;
	int sum = 0;

	for (back_added = 0; back_added < g_runing_sum_size; back_added++)
		if ((g_sample_statistics - back_added) >= 0)
			sum += g_results[net][g_sample_statistics - back_added][stock][signal];

	return (sum);
}


int
f_sum(int net, int stock, int parameter)
{
	int i;
	int sum = 0;

	for (i = POSE_MIN; i <= g_sample_statistics; i++)
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
//TODO:
	int sample = g_sample_statistics;
	g_sample_statistics++;

	//int sample2 = SamplePreviousPeriod(g_sample);

	for (stock = 0; stock < n_stocks; stock++)
	{
		float pred_ret = GetNeuronsOutput(neural_prediction, stock);
		float actu_ret = GetNeuronsOutput(actual_result, stock);
#endif

		g_y[stock][g_t_y_count] = pred_ret;
		g_t[stock][g_t_y_count] = actu_ret;

		//if (stock == 0) printf("%f; %f\n", actu_ret /*+ data[sample2].WIN.mid*/, pred_ret /*+ data[sample2].WIN.mid*/);

		//printf("pred= %f act= %f \n", pred_ret, actu_ret);
		if (sample > 0)
		{
			int s1, s2;
			s1 = signal_of_val((float) g_results[net][sample - 1][stock][P_RETURN]);
			s2 = signal_of_val((float) g_results[net][sample - 1][stock][RETURN]);

			//printf("s1=%d s2=%d \n", s1, s2);
			NEURON *result_report = result_output->neuron_layer->neuron_vector;
			result_report[stock].output.fval = 0.0;

			if 	((s1 == SUBIU) && (s2 == SUBIU)) {g_results[net][sample][stock][SS] += 1; result_report[stock].output.fval = 1.0;}
			else if ((s1 == SUBIU) && (s2 == DESCE))  g_results[net][sample][stock][SD] += 1;
			else if ((s1 == SUBIU) && (s2 == NEUTR))  g_results[net][sample][stock][SN] += 1;
			else if ((s1 == DESCE) && (s2 == SUBIU))  g_results[net][sample][stock][DS] += 1;
			else if ((s1 == DESCE) && (s2 == DESCE)) {g_results[net][sample][stock][DD] += 1; result_report[stock].output.fval = 1.0; }
			else if ((s1 == DESCE) && (s2 == NEUTR))  g_results[net][sample][stock][DN] += 1;
			else if ((s1 == NEUTR) && (s2 == SUBIU))  g_results[net][sample][stock][NS] += 1;
			else if ((s1 == NEUTR) && (s2 == DESCE))  g_results[net][sample][stock][ND] += 1;
			else if ((s1 == NEUTR) && (s2 == NEUTR)) {g_results[net][sample][stock][NN] += 1; result_report[stock].output.fval = -1.0;}

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

		g_results[net][sample][stock][P_RETURN] = (int) round(pred_ret * 1000.0);
		g_results[net][sample][stock][RETURN] = (int) round(actu_ret * 1000.0);
	}
}

double
compute_operation(int stock)
{
	double r = 0;

	double prc_buy = 0;
	double prc_sell = 0;
	int sample, sample_buy, sample_sell;

	sample = SamplePreviousPeriod(g_sample);

	if (g_LongShort == 1)// Long
	{
		sample_buy = sample;
		sample_sell = g_sample;
	}
	else
	{
		sample_buy = g_sample;
		sample_sell = sample;
	}

	if (stock == 0) // win
	{
		prc_buy = data[sample_buy].WIN.buy;
		prc_sell = data[sample_sell].WIN.sell;
	}
	else if (stock == 1) // ind
	{
		prc_buy = data[sample_buy].IND.buy;
		prc_sell = data[sample_sell].IND.sell;
	}
	if (stock == 2) // wdo
	{
		prc_buy = data[sample_buy].WDO.buy;
		prc_sell = data[sample_sell].WDO.sell;
	}
	if (stock == 3) // dol
	{
		prc_buy = data[sample_buy].DOL.buy;
		prc_sell = data[sample_sell].DOL.sell;
	}

	r = (prc_sell - prc_buy);
	return r;
}

double
compute_operation_mid(int stock)
{
	double r = 0;

	double prc_buy = 0;
	double prc_sell = 0;
	int sample, sample_buy, sample_sell;

	sample = SamplePreviousPeriod(g_sample);

	if (g_LongShort == 1)// Long
	{
		sample_buy = sample;
		sample_sell = g_sample;
	}
	else
	{
		sample_buy = g_sample;
		sample_sell = sample;
	}

	if (stock == 0) // win
	{
		prc_buy = data[sample_buy].WIN.mid;
		prc_sell = data[sample_sell].WIN.mid;
	}
	else if (stock == 1) // ind
	{
		prc_buy = data[sample_buy].IND.mid;
		prc_sell = data[sample_sell].IND.mid;
	}
	if (stock == 2) // wdo
	{
		prc_buy = data[sample_buy].WDO.mid;
		prc_sell = data[sample_sell].WDO.mid;
	}
	if (stock == 3) // dol
	{
		prc_buy = data[sample_buy].DOL.mid;
		prc_sell = data[sample_sell].DOL.mid;
	}

	r = (prc_sell - prc_buy);
	return r;
}

void
compute_capital_evolution(int net, int n, OUTPUT_DESC *neural_prediction, OUTPUT_DESC *actual_result)
{
	int i;
	double cost = 0;

	for (i = 0; i < n; i++)
	{
		int s1;
		double r = GetNeuronsOutput(neural_prediction, i);
		s1 = signal_of_val(r);
		GetNeuronsOutputConfidence(neural_prediction, i);

		double expected_result_buy_sell = r;
		double expected_result_sell_buy = r;

		double result_buy_sell = 0.0;
		double result_sell_buy = 0.0;

		if (USE_SPREAD == 0)
		{
			//r = GetNeuronsOutput(actual_result, i);
			r = compute_operation_mid(i);
		}
		else
		{
			r = compute_operation(i);
		}

		double point_value = 0.0;
		int qty = 0;

		if (i == 0)
		{
			point_value = WIN_POINT_VALUE;
			qty = WIN_QTY;
			cost = COST_QTY_WIN_WDO;
		}
		if (i == 1)
		{
			point_value = IND_POINT_VALUE;
			qty = IND_QTY;
			cost = COST_QTY_IND_DOL;
		}
		if (i == 2)
		{
			point_value = WDO_POINT_VALUE;
			qty = WDO_QTY;
			cost = COST_QTY_WIN_WDO;
		}
		if (i == 3)
		{
			point_value = DOL_POINT_VALUE;
			qty = DOL_QTY;
			cost = COST_QTY_IND_DOL;
		}

		if ( USE_COST == 0 )
			cost = 0;

		result_buy_sell = ((r * point_value - 2 * cost) * qty) * ALAVANCAGEM;
		result_sell_buy = ((r * point_value - 2 * cost) * qty) * ALAVANCAGEM;

		g_n_period_test[i] += 1;

		//printf("%d\n", g_use_confiance);
		if (g_LongShort == 1)
		{
			if ( ( (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_confidence_up[net][i] > CERTAINTY) && (g_use_confiance == 1) ) ||
				 ( g_runing_sum_size > 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) && (g_mean_correct_positive_pred[net][i] > CERTAINTY) && (g_use_confiance == 0) ) ||
				 ( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_buy_sell > 0.0) ) 
				)
			{
				g_n_period_test_op[i] += 1;

				double previous_capital = g_capital[net][i];
				g_capital[net][i] += result_buy_sell;
				g_capital[net][INPUT_WIDTH] += result_buy_sell;

				if (g_capital[net][i] > previous_capital)
					g_results[net][g_sample_statistics-1][i][HITS] += 1;

				g_buy_sell_count[net][i] += 1;
				g_results[net][g_sample_statistics-1][i][INVEST] = 1;
			}
			else
				g_results[net][g_sample_statistics-1][i][INVEST] = 0;
		}
		else
		{
			if ( ( (s1 == 0) && (expected_result_sell_buy > 0.0) && (g_confidence_up[net][i] > CERTAINTY) && (g_use_confiance == 1) ) ||
				 ( g_runing_sum_size > 0 &&  (s1 == 0) && (expected_result_sell_buy > 0.0) && (g_mean_correct_positive_pred[net][i] > CERTAINTY) && (g_use_confiance == 0) ) ||
				 ( g_runing_sum_size == 0 && (s1 == 0) && (expected_result_sell_buy > 0.0) )
				)
			{
				g_n_period_test_op[i] += 1;

				double previous_capital = g_capital[net][i];
				g_capital[net][i] += result_sell_buy;
				g_capital[net][INPUT_WIDTH] += result_sell_buy;

				if (g_capital[net][i] > previous_capital)
					g_results[net][g_sample_statistics-1][i][HITS] += 1;

				g_sell_buy_count[net][i] += 1;
				g_results[net][g_sample_statistics-1][i][INVEST] = 1;
			}
			else
				g_results[net][g_sample_statistics-1][i][INVEST] = 0;
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

	g_t_y_count++;
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
	OUTPUT_DESC* filter_out = get_output_by_name(out_ita_lp_f.name);
	OUTPUT_DESC* output_pred = get_output_by_name(out_prediction.name);
	double pred = 0;
	for(x = 0; x < filter_out->neuron_layer->dimentions.x; x++)
	{
		for (y = 0; y < filter_out->neuron_layer->dimentions.y; y++)
		{
			printf("%f ", filter_out->neuron_layer->neuron_vector[y * filter_out->neuron_layer->dimentions.x + x ].output.fval);
		}
		pred = GetNeuronsOutput(output_pred, x);
		printf("\n %f \n", pred);
	}
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
//TODO:
	NEURON_OUTPUT output;
	int i, stock, k, total_tested;
	int n_stocks = INPUT_WIDTH;

	//printf("%s %s\n", date, returns[0][g_sample].time);
	printf("%04d-%02d-%02d %02d:%02d:%02d\n",
			data[g_sample].time.year, data[g_sample].time.month, data[g_sample].time.day,
			data[g_sample].time.hour, data[g_sample].time.min, data[g_sample].time.sec);

	for (stock = 0; stock < n_stocks; stock++)
	{
		total_tested = 0;
		for (k = 0; k < g_runing_sum_size; k++)
		//for (k = 0; k < g_sample_statistics; k++)
			for (i = 0; i < 9; i++)
				if ((g_sample - k) >= 0)
					total_tested += g_results[net][k][stock][i];

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
						(double) g_results[net][g_sample_statistics-1][stock][RETURN] / 1000.0, (double) g_results[net][g_sample_statistics-1][stock][P_RETURN] / 1000.0,
						g_results[net][g_sample_statistics-1][stock][INVEST],
						100.0 * (double) f_sum(net, stock, HITS) / (double) (g_buy_sell_count[net][stock] + g_sell_buy_count[net][stock]), g_confidence_up[net][stock]);
			else
				printf("buy_sell_count = %2d, sell_buy_count = %2d, capital = %.2lf, return = %.4lf, p_return = %.4lf, invest = %d, hit_rate = ---, g_confidence_up=%.2lf\n",
						g_buy_sell_count[net][stock], g_sell_buy_count[net][stock], g_capital[net][stock],
						(double) g_results[net][g_sample_statistics-1][stock][RETURN] / 1000.0, (double) g_results[net][g_sample_statistics-1][stock][P_RETURN] / 1000.0,
						g_results[net][g_sample_statistics-1][stock][INVEST], g_confidence_up[net][stock]);
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

	int i, j, stock, total_tested;
	int n_stocks = INPUT_WIDTH;
	double total_hits = 0;

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
				total_hits += stat_day[day_i][stock][HITS];
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
	int g_ops[2][INPUT_WIDTH];
	for (stock = 0; stock < n_stocks; stock++)
	{
		total_capital += g_capital[0][stock];
		total_buy_sell += g_buy_sell_count[0][stock];
		total_sell_buy += g_sell_buy_count[0][stock];
	}

	if ( g_LongShort == 1 )
	{
		for ( i = 0; i < 2; i++ )
			for ( j = 0; j < INPUT_WIDTH; j++ )
				g_ops[i][j] = g_buy_sell_count[i][j];
	}
	else
	{
		for ( i = 0; i < 2; i++ )
			for ( j = 0; j < INPUT_WIDTH; j++ )
				g_ops[i][j] = g_sell_buy_count[i][j];
	}

	stat_exp[day_i].day = day_i;
	stat_exp[day_i].avg_capital = total_capital/(double)n_stocks;
	stat_exp[day_i].geral_capital = g_capital[0][INPUT_WIDTH];
	stat_exp[day_i].n_ops = g_LongShort == 1 ? total_buy_sell : total_sell_buy;
	stat_exp[day_i].total_hits = total_hits;

	printf("day_i=%d; avg_capital=%.2lf; geral_capital=%.2lf; n_ops=%d; hit_rate=%.1lf; ",
			day_i, stat_exp[day_i].avg_capital, stat_exp[day_i].geral_capital, stat_exp[day_i].n_ops,
			100.0 * stat_exp[day_i].total_hits / stat_exp[day_i].n_ops);

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
		if (g_ops[net][stock]/*g_buy_sell_count[net][stock]*/ > 0)
			printf("%shit_rate=%.1lf; ",symbol,  100.0 * stat_day[day_i][stock][HITS]/ g_ops[net][stock]);
		else
			printf("%shit_rate=--; ",symbol);
		printf("%scapital=%.1lf; ",symbol,  g_capital[net][stock]);
		printf("%sn_ops=%d; ",symbol, g_ops[net][stock]);
	}
	printf("\n");


	stat_exp[day_i].capital_win = g_capital[net][0];
	stat_exp[day_i].n_ops_win = g_ops[net][0];
	stat_exp[day_i].capital_ind = g_capital[net][1];
	stat_exp[day_i].n_ops_ind = g_ops[net][1];
	stat_exp[day_i].capital_wdo = g_capital [net][2];
	stat_exp[day_i].n_ops_wdo = g_ops[net][2];
	stat_exp[day_i].capital_dol = g_capital[net][3];
	stat_exp[day_i].n_ops_dol = g_ops[net][3];

	/*
	printf("day_i=%d; avg_capital=%.2lf; n_ops=%d; WIN_capital=%.2lf; WIN_n_ops=%d; IND_capital=%.2lf; IND_n_ops=%d; WDO_capital=%.2lf;"
			" WDO_n_ops=%d; DOL_capital=%.2lf; DOL_n_ops=%d;\n", stat_exp[day_i].day, stat_exp[day_i].avg_capital,
			stat_exp[day_i].n_ops, stat_exp[day_i].capital_win, stat_exp[day_i].n_ops_win, stat_exp[day_i].capital_ind,
			stat_exp[day_i].n_ops_ind, stat_exp[day_i].capital_wdo, stat_exp[day_i].n_ops_wdo, stat_exp[day_i].capital_dol,
			stat_exp[day_i].n_ops_dol);
	 */

	for ( i = 0; i < INPUT_WIDTH; i++ )
		printf("%d - N Period=%d, N Period with Operation=%d, Operations/N_Period=%.2f\n", i, g_n_period_test[i], g_n_period_test_op[i],
				(1.0 * g_n_period_test_op[i])/g_n_period_test[i]);

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
	double total_geral_capital = 0.0;
	double total_capital_win = 0.0;
	double total_capital_ind = 0.0;
	double total_capital_wdo = 0.0;
	double total_capital_dol = 0.0;

	double total_hits = 0.0;

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
		total_geral_capital	+= stat_exp[i].geral_capital;
		total_capital_win += stat_exp[i].capital_win;
		total_capital_ind += stat_exp[i].capital_ind;
		total_capital_wdo += stat_exp[i].capital_wdo;
		total_capital_dol += stat_exp[i].capital_dol;

		total_hits += stat_exp[i].total_hits;

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

	char symbol[6];

	printf("day_i=Mean; avg_capital=%.2lf; avg_geral_capital=%.2lf; n_ops=%.2lf; hit_rate=%.1lf; ",
			total_capital/n, total_geral_capital/n, (1.0*n_ops)/n, 100.0 * total_hits/n_ops);
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
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, 100.0*(total_capital_win/n - BEGIN_CAPITAL)/BEGIN_CAPITAL, symbol, (1.0*n_ops_win)/n);
		}
		if (stock == 1)
		{
			if (n_ops_ind > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_ind);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, 100.0*(total_capital_ind/n - BEGIN_CAPITAL)/BEGIN_CAPITAL, symbol, (1.0*n_ops_ind)/n);
		}
		if (stock == 2)
		{
			if (n_ops_wdo > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_wdo);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, 100.0*(total_capital_wdo/n - BEGIN_CAPITAL)/BEGIN_CAPITAL, symbol, (1.0*n_ops_wdo)/n);
		}
		if (stock == 3)
		{
			if (n_ops_dol > 0) printf("%shit_rate=%.1lf; ",symbol,  100.0 * mean_stat_day[stock][HITS]/n_ops_dol);
			else printf("%shit_rate=--; ",symbol);
			printf("%scapital=%.2lf; %sn_ops=%.2lf; ", symbol, 100.0*(total_capital_dol/n - BEGIN_CAPITAL)/BEGIN_CAPITAL, symbol, (1.0*n_ops_dol)/n);
		}
	}
	printf("\n");
/*
	printf("WIN_capital=%.2lf; WIN_n_ops=%.2lf; IND_capital=%.2lf; "
			"IND_n_ops=%.2lf; WDO_capital=%.2lf; WDO_n_ops=%.2lf; DOL_capital=%.2lf; DOL_n_ops=%.2lf;\n",
			total_capital_win/n, (1.0*n_ops_win)/n, total_capital_ind/n,
			(1.0*n_ops_ind)/n, total_capital_wdo/n, (1.0*n_ops_wdo)/n, total_capital_dol/n, (1.0*n_ops_dol)/n);
*/

	printf("MEAN\n");
	for ( i = 0; i < INPUT_WIDTH; i++ )
		printf("%d - N Period=%d, N Period with Operation=%d, Operations/N_Period=%.2f\n", i, g_n_period_test[i], g_n_period_test_op[i],
				(1.0 * g_n_period_test_op[i])/g_n_period_test[i]);

	fflush(stdout);

//	FILE* f = fopen("data_short_30_256s.csv", "w+");
//	fprintf(f, "WIN_T; WIN_Y; IND_T; IND_Y; WDO_T; WDO_Y; DOL_T; DOL_Y\n");
//	for (i = 0; i < g_t_y_count; i++)
//	{
//		fprintf(f, "%lf; %lf; %lf; %lf; %lf; %lf; %lf; %lf\n", g_t[0][i], g_y[0][i],
//				g_t[1][i], g_y[1][i], g_t[2][i], g_y[2][i], g_t[3][i], g_y[3][i]);
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

	int i, j, k, l;
	for (j = 0; j < INPUT_WIDTH; j++)
	{
		/*
		g_n_period_train[j] = 0;
		g_n_period_test[j] = 0;
		g_n_period_train_op[j] = 0;
		g_n_period_test_op[j] = 0;
		*/

		for (l = 0; l < 2; l++)
		{
			g_capital[l][j] = BEGIN_CAPITAL;
			g_buy_sell_count[l][j] = 0;
			g_sell_buy_count[l][j] = 0;
			for (k = 0; k < MAX_DATA_SAMPLES; k++)
			{
				if ( g_t_y_count == 0 )
				{
					g_t[j][k] = 0;
					g_y[j][k] = 0;
				}
				for (i = 0; i < 13; i++)
					g_results[l][k][j][i] = 0;
			}
		}

		g_anchor[j] = 0.0;
		g_enable_operation[j] = 1;
		g_last_close_operation_sample[j] = 0;
		g_n_ops[j]= 0;
		g_n_hits[j] = 0;
		g_n_miss[j] = 0;
	}
	g_capital[0][INPUT_WIDTH] = BEGIN_CAPITAL;
	g_capital[1][INPUT_WIDTH] = BEGIN_CAPITAL;

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

	//if (LoadReturnsToInput(&ita, 0, -1))
	if (LoadDataToInput(&ita))
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
* Function: LoadData
* Description:
* Inputs:
* Output:
***********************************************************
*/

void
update_time_vars(void)
{
	// update TRAIN_TIME, TEST_TIME and TEST_END_TIME variables
	TRAIN_TIME.year = data[POSE_MIN].time.year;
	TRAIN_TIME.month = data[POSE_MIN].time.month;
	TRAIN_TIME.day = data[POSE_MIN].time.day;
	TRAIN_TIME.hour = TRAIN_HOUR;
	TRAIN_TIME.min = TRAIN_MIN;
	TRAIN_TIME.sec = 0;
	TRAIN_TIME.msec = 0;
	TRAIN_TIME.tstamp = GetTimeInSeconds(TRAIN_TIME);

	TEST_TIME.year = data[POSE_MIN].time.year;
	TEST_TIME.month = data[POSE_MIN].time.month;
	TEST_TIME.day = data[POSE_MIN].time.day;
	TEST_TIME.hour = TEST_HOUR;
	TEST_TIME.min = TEST_MIN;
	TEST_TIME.sec = 0;
	TEST_TIME.msec = 0;
	TEST_TIME.tstamp = GetTimeInSeconds(TEST_TIME);

	TEST_END_TIME.year = data[POSE_MIN].time.year;
	TEST_END_TIME.month = data[POSE_MIN].time.month;
	TEST_END_TIME.day = data[POSE_MIN].time.day;
	TEST_END_TIME.hour = TEST_END_HOUR;
	TEST_END_TIME.min = TEST_END_MIN;
	TEST_END_TIME.sec = 0;
	TEST_END_TIME.msec = 0;
	TEST_END_TIME.tstamp = GetTimeInSeconds(TEST_END_TIME);
}

int
LoadData(char *f_name)
{
	FILE *fp;

	fp = fopen(f_name, "r");

	if (fp == NULL)
	{
		Erro("Error: cannot open file in LoadData(). f_name = ", f_name, "");
		return -1;
	}

	int numlines = 0;
	char f_line[1024];
	char BRT[256];

	fscanf(fp, "%[^\n]s", f_line); // header

	while(!feof(fp))
	{
		fscanf(fp, "%d; %d-%d-%d %d:%d:%d.%d %s %lf; %lf; %lf; %lf; %lf; %lf; %lf; %lf;\n",
				&data[numlines].id,
				&data[numlines].time.year, &data[numlines].time.month,  &data[numlines].time.day,
				&data[numlines].time.hour, &data[numlines].time.min,  &data[numlines].time.sec, &data[numlines].time.msec,
				BRT,
				&data[numlines].WIN.buy, &data[numlines].WIN.sell,
				&data[numlines].IND.buy, &data[numlines].IND.sell,
				&data[numlines].WDO.buy, &data[numlines].WDO.sell,
				&data[numlines].DOL.buy, &data[numlines].DOL.sell);

		data[numlines].WIN.mid = (data[numlines].WIN.buy + data[numlines].WIN.sell) / 2.0;
		data[numlines].IND.mid = (data[numlines].IND.buy + data[numlines].IND.sell) / 2.0;
		data[numlines].WDO.mid = (data[numlines].WDO.buy + data[numlines].WDO.sell) / 2.0;
		data[numlines].DOL.mid = (data[numlines].DOL.buy + data[numlines].DOL.sell) / 2.0;

		data[numlines].time.tstamp = GetTimeInSeconds(data[numlines].time);
		/*
		printf("%d; %04d-%02d-%02d %02d:%02d:%02d.%03d %s %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf; %.2lf;\n",
				data[numlines].id,
				data[numlines].time.year, data[numlines].time.month,  data[numlines].time.day,
				data[numlines].time.hour, data[numlines].time.min,  data[numlines].time.sec, data[numlines].time.msec,
				BRT,
				data[numlines].WIN.buy, data[numlines].WIN.sell, data[numlines].WIN.mid,
				data[numlines].IND.buy, data[numlines].IND.sell, data[numlines].IND.mid,
				data[numlines].WDO.buy, data[numlines].WDO.sell, data[numlines].WDO.mid,
				data[numlines].DOL.buy, data[numlines].DOL.sell, data[numlines].DOL.mid);
		*/

		numlines++;
		if (numlines >= MAX_DATA_SAMPLES)
		{
			Erro("numlines >= MAX_DATA_SAMPLES in LoadData().", "", "");
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

	output.ival = LoadData(day_i_file_name);

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

	//int hour = data[g_sample].time.hour;
	//int min = data[g_sample].time.min;
	//int sec = data[g_sample].time.sec;

	int ret = 0;
	/*if (
		( ( hour == TRAIN_HOUR && min >= TRAIN_MIN ) ||
		( hour > TRAIN_HOUR ) )  &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	*/
	if ( ( data[g_sample].time.tstamp >= TRAIN_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		//LoadReturnsToInput(&ita, 0, -1);
		LoadDataToInput(&ita);
		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

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

	//int hour = data[g_sample].time.hour;
	//int min = data[g_sample].time.min;
	//int sec = data[g_sample].time.sec;

	int ret = 0;
	/*if (
		( ( hour == TRAIN_HOUR && min >= TRAIN_MIN ) ||
		( hour > TRAIN_HOUR && hour < TEST_HOUR ) ||
		( hour == TEST_HOUR && min < TEST_MIN ) ) &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	*/
	if ( ( data[g_sample].time.tstamp >= TRAIN_TIME.tstamp ) &&
		 ( data[g_sample].time.tstamp < TEST_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		//LoadReturnsToInput(&ita, 0, -1);
		LoadDataToInput(&ita);

		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

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

	//int hour = data[g_sample].time.hour;
	//int min = data[g_sample].time.min;
	//int sec = data[g_sample].time.sec;

	int ret = 0;
	/*if (
		( ( hour == TEST_HOUR && min >= TEST_MIN ) ||
		  ( hour > TEST_HOUR && hour < TEST_END_HOUR ) ||
		  ( hour == TEST_END_HOUR && min <= TEST_END_MIN )
			) &&
		g_sample <= POSE_MAX && g_sample >= POSE_MIN
		)
	*/
	if ( ( data[g_sample].time.tstamp >= TEST_TIME.tstamp ) &&
		 ( data[g_sample].time.tstamp < TEST_END_TIME.tstamp ) &&
		 ( g_sample <= POSE_MAX && g_sample >= POSE_MIN )
	   )
	{
		ret = 1;
		//LoadReturnsToInput(&ita, 0, -1);
		LoadDataToInput(&ita);

		//printf("g_current_sample=%d %02d:%02d:%02d\n", g_sample, hour, min, sec);

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
	int i;

	if (param <= 0)
	{
		g_sample = 0;
	}
	else
	{
		for (i = 0; i < param; i++)
			g_sample = SampleNextPeriod(g_sample);
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

NEURON_OUTPUT
GetInputHeight(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = INPUT_HEIGHT;

	return (output);
}

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

NEURON_OUTPUT
GetTime(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

    static struct timeval tvBegin, tvEnd, tvDiff;
    static int n = 0;
    static double val = 0;

	int param = pParamList->next->param.ival;

	if ( param == 0 )
	{
	    // begin
	    gettimeofday(&tvBegin, NULL);
	    timeval_print(&tvBegin);
	}
	else
	{
	    //end
	    gettimeofday(&tvEnd, NULL);
	    timeval_print(&tvEnd);

	    // diff
	    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
	    printf("%ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

	    val += tvDiff.tv_sec + tvDiff.tv_usec / 1000000.0;
	    n += 1;

	    printf("val=%.6lf\n", val / n);
	}

	output.ival = 0;
	return (output);
}

