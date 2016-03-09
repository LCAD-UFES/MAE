#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#include "mae.h"
#include "../wnn_pred.h"

struct _tt_data_set
{
	long date_l;
	float target_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
	int file_number;
};

typedef struct _tt_data_set TT_DATA_SET;


/* Global variables */
char **g_argv;
int g_argc;

TT_DATA_SET *g_tt_data_set = NULL;
TT_DATA_SET **g_global_data_set = NULL;
int g_tt_num_samples = 0;
int g_no_extra_predicted_sample = 1;


void
read_data_from_file (FILE *data_set_file, int file_number)
{
	char data_set_file_line[1000], date[100], day_week[100];
	float target_return, wnn_predicted_return, wnn_prediction_error;
	int i;
	int num_fields_read;
	
	if (g_tt_num_samples == 0)
	{
		while (fgets (data_set_file_line, 900, data_set_file) != NULL)
		{
			if (data_set_file_line[0] == '#')
				continue;
			g_tt_num_samples++;
		}
		rewind (data_set_file);
	}
	
	if (g_tt_data_set == NULL)
		g_tt_data_set = (TT_DATA_SET *) malloc (g_tt_num_samples * sizeof (TT_DATA_SET));
	
	i = 0;
	while (fgets (data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;

		if ((num_fields_read = sscanf (data_set_file_line, "%s %s ; %f ; %f ; %f  ;", (char *)date, (char *)day_week, 
					       &target_return, &wnn_predicted_return, &wnn_prediction_error)) != 5)
		{
			if (num_fields_read == 2)
			{
				g_no_extra_predicted_sample = 0;
				sscanf (data_set_file_line, "%s %s ;  ; %f ;  ;", (char *)date, (char *)day_week, 
					&wnn_predicted_return);
				target_return = wnn_prediction_error = 0.0;
			}
			else
				break;
		}			
		g_tt_data_set[i].date_l = isodate2long (date);
		g_tt_data_set[i].target_return = target_return;
		g_tt_data_set[i].wnn_predicted_return = wnn_predicted_return;
		g_tt_data_set[i].wnn_prediction_error = wnn_prediction_error;
		g_tt_data_set[i].file_number = file_number;
		i++;
	}
	if ((g_tt_num_samples != i) && (g_no_extra_predicted_sample != 0))
	{
		printf ("Erro: number of samples does not match. File name = %s, g_tt_num_samples = %d, i = %d\n", 
			g_argv[file_number], g_tt_num_samples, i);
		exit (1);
	}
}



void
add_data_to_results (int file_number)
{
	int i;
	
	if (g_global_data_set == NULL)
	{
		g_global_data_set =  (TT_DATA_SET **) malloc (g_argc * sizeof (TT_DATA_SET *));
		g_global_data_set[0] =  (TT_DATA_SET *) malloc (g_tt_num_samples * sizeof (TT_DATA_SET));
		g_global_data_set[file_number] =  (TT_DATA_SET *) malloc (g_tt_num_samples * sizeof (TT_DATA_SET));
		for (i = 0; i < g_tt_num_samples; i++)
		{
			g_global_data_set[0][i] = g_tt_data_set[i];
			g_global_data_set[file_number][i] = g_tt_data_set[i];
		}
		return;
	}
	
	g_global_data_set[file_number] =  (TT_DATA_SET *) malloc (g_tt_num_samples * sizeof (TT_DATA_SET));
	for (i = 0; i < g_tt_num_samples; i++)
	{
		/* O >= previlegia acoes examinadas por ultimo */
		if (g_tt_data_set[i].wnn_predicted_return >= g_global_data_set[0][i].wnn_predicted_return)
		{
			if (g_global_data_set[0][i].date_l != g_tt_data_set[i].date_l)
			{
				printf ("Erro: Date of sample %d of files %s and %s differ\n", 
					i, g_argv[g_global_data_set[0][i].file_number], g_argv[g_tt_data_set[i].file_number]);
				exit (1);
			}
			g_global_data_set[0][i] = g_tt_data_set[i];
		}
		g_global_data_set[file_number][i] = g_tt_data_set[i];
	}
}


void
process_one_file (file_number)
{
	FILE *file;
	
	if ((file = fopen (g_argv[file_number], "r")) == NULL)
	{
		printf ("Erro: Could not open file: %s\n", g_argv[file_number]);
		exit (1);
	}
	
	read_data_from_file (file, file_number);
	add_data_to_results (file_number);
	
	fclose (file);
}



void
print_test_samples ()
{
	int i;

	for (i = SAMPLES2CALIBRATE; i < g_tt_num_samples; i++)
	{
		printf ("%s %s ; % .6f ; % .6f ; % .6f; %s\n", 
			 long2isodate (g_global_data_set[0][i].date_l),
			 long2dows (g_global_data_set[0][i].date_l), 
			 g_global_data_set[0][i].target_return,
			 g_global_data_set[0][i].wnn_predicted_return,
			 g_global_data_set[0][i].wnn_prediction_error,
			 g_argv[g_global_data_set[0][i].file_number]);
	}
}



void
show_statistics ()
{
	int N, i;
	float average_wnn_prediction_error = 0.0;
	float standard_deviation_wnn_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;
	int wnn_same_up = 0, target_up = 0, wnn_same_down = 0, target_down = 0;
	float accumulated_return = 0.0;
	
	print_test_samples ();

	N = g_tt_num_samples - g_no_extra_predicted_sample - SAMPLES2CALIBRATE;
	accumulated_return = 1.0;
	for (i = SAMPLES2CALIBRATE; i < N+SAMPLES2CALIBRATE; i++)
	{
		average_wnn_prediction_error += g_global_data_set[0][i].wnn_prediction_error;
		average_target_return += g_global_data_set[0][i].target_return;

		accumulated_return *= (g_global_data_set[0][i].target_return + 1.0);
		
		if (g_global_data_set[0][i].target_return > 0.0)
		{
			target_up++;
			if (g_global_data_set[0][i].wnn_predicted_return > 0.0)
				wnn_same_up++;
		}
		else if (g_global_data_set[0][i].target_return < 0.0)
		{
			target_down++;
			if (g_global_data_set[0][i].wnn_predicted_return < 0.0)
				wnn_same_down++;
		}
		// printf ("average_wnn_prediction_error = %f\n", average_wnn_prediction_error);
	}
	average_wnn_prediction_error /= (float) N;
	average_target_return /= (float) N;

	for (i = SAMPLES2CALIBRATE; i < g_tt_num_samples; i++)
	{
		standard_deviation_wnn_error += (g_global_data_set[0][i].wnn_prediction_error - average_wnn_prediction_error) *
					        (g_global_data_set[0][i].wnn_prediction_error - average_wnn_prediction_error);
		standard_deviation_target_return += (g_global_data_set[0][i].target_return - average_target_return) *
					            (g_global_data_set[0][i].target_return - average_target_return);
	}
	standard_deviation_wnn_error = sqrt (standard_deviation_wnn_error / (float) (N-1));
	standard_deviation_target_return = sqrt (standard_deviation_target_return / (float) (N-1));
	
	printf ("# **** average_target_return: % .6f, average_wnn_error: % .6f\n",
			average_target_return,
		        average_wnn_prediction_error);

	printf ("# **** standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
			standard_deviation_target_return,
		        standard_deviation_wnn_error);

	printf ("# **** wnn_same_up: %.2f, wnn_same_down: %.2f, wnn_same_signal: %.2f\n", 
			100.0 * ((float) wnn_same_up / (float) target_up),
			100.0 * ((float) wnn_same_down / (float) target_down), 
			100.0 * ((float) (wnn_same_up + wnn_same_down) / (float) (target_up + target_down)));

	printf ("# **** accumulated_return: %.2f%%\n", 100.0 * (accumulated_return - 1.0));
}



void
sumariza_desempenhos (int semana, float *desempenho)
{
	int i, a;
	
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = 0.0;
		for (i = semana-SAMPLES2CALIBRATE; i < semana; i++)
		{
			desempenho[a] += ((g_global_data_set[a][i].target_return >= STOP_GAIN) &&
					  (g_global_data_set[a][i].wnn_predicted_return >= STOP_GAIN)) ? 1.0: 0.0;
		}
		desempenho[a] /= (float) SAMPLES2CALIBRATE;
	}
}



void
get_desempenho_corrente (int semana, float *desempenho)
{
	int a;
	
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = g_global_data_set[a][semana].wnn_predicted_return;
	}
}



void
print_vetor (float *vetor, int tamanho)
{
	int a;
	
	for (a = 1; a < tamanho; a++)
	{
		printf ("% .2f ", vetor[a]);
	}
	printf ("\n");
}



void
normaliza_vetor (float *vetor, int tamanho)
{
	int a;
	float max = -1000.0;
	float min = 1000.0;
	
	for (a = 1; a < tamanho; a++)
	{
		if (vetor[a] > max)
			max = vetor[a];
		if (vetor[a] < min)
			min = vetor[a];
	}
	
	for (a = 1; a < tamanho; a++)
	{
		vetor[a] = 0.000001 + (vetor[a] - min) / (max - min);
	}
}



void
computa_media_desempenhos (float *media_desempenhos, float *desempenho, float *desempenho_corrente, int tamanho)
{
	int a;
	
	for (a = 0; a < tamanho; a++)
	{
//		media_desempenhos[a] = 1.0 / ((1.0 / desempenho[a]) + (1.0 / desempenho_corrente[a]));
//		media_desempenhos[a] = (desempenho[a] + desempenho_corrente[a]) / 2.0;
		media_desempenhos[a] = (desempenho[a] * desempenho_corrente[a]);
	}
}
	


void
escolhe_acao (int semana, float *media_desempenhos, int tamanho)
{
	int a, melhor_acao;
	float max = -1000.0;
//	float rand_val;
	
	for (a = 1; a < tamanho; a++)
	{
		if (media_desempenhos[a] > max)
		{
			max = media_desempenhos[a];
			melhor_acao = a;
		}
	}
	if (melhor_acao == tamanho)
	{
		printf ("Erro: nao foi encontrada a melhor acao em escolhe_acao ().\n");
		exit (1);
	}

//	rand_val = (double) rand() / (double) RAND_MAX;
//	melhor_acao = (int) (rand_val * (float) (tamanho-2) + 1.5);

	g_global_data_set[0][semana] = g_global_data_set[melhor_acao][semana];
	
	
	if (g_global_data_set[0][semana].target_return >= STOP_GAIN)
		g_global_data_set[0][semana].target_return = STOP_GAIN;
	else if (g_global_data_set[0][semana].target_return <= STOP_LOSS)
		g_global_data_set[0][semana].target_return = STOP_LOSS;
	
}



void
run_trading_system ()
{
	int semana;
	float desempenho[SAMPLES2CALIBRATE];
	float desempenho_corrente[SAMPLES2CALIBRATE];
	float media_desempenhos[SAMPLES2CALIBRATE];
	
	for (semana = SAMPLES2CALIBRATE; semana < SAMPLES2CALIBRATE+SAMPLES2TEST; semana++)
	{
		sumariza_desempenhos (semana, desempenho);
		get_desempenho_corrente (semana, desempenho_corrente);

		normaliza_vetor (desempenho, g_argc);
		print_vetor (desempenho, g_argc);
		
		normaliza_vetor (desempenho_corrente, g_argc);
		print_vetor (desempenho_corrente, g_argc);

		computa_media_desempenhos (media_desempenhos, desempenho, desempenho_corrente, g_argc);

		print_vetor (media_desempenhos, g_argc);
		printf ("\n");

		escolhe_acao (semana, media_desempenhos, g_argc);
	}
}



int
main (int argc, char **argv)
{
	int i;
	
	if (argc < 2)
	{
		printf ("Erro: It is necessary at least one input file.\n");
		exit (1);
	}

	g_argv = argv;
	g_argc = argc;

	for (i = 1; i < argc; i++)
		process_one_file (i);
	
	run_trading_system ();
	show_statistics ();
	
	return (0);
}
