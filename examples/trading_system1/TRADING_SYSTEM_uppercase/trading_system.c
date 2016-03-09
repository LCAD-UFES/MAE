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
	char time[10];
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
	char data_set_file_line[1000], time[100];
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

		if ((num_fields_read = sscanf (data_set_file_line, "%s ; %f ; %f ; %f  ;", (char *)time, 
					       &target_return, &wnn_predicted_return, &wnn_prediction_error)) != 4)
		{
			if (num_fields_read == 1)
			{
				g_no_extra_predicted_sample = 0;
				sscanf (data_set_file_line, "%s ;  ; %f ;  ;", (char *)time, 
					&wnn_predicted_return);
				target_return = wnn_prediction_error = 0.0;
			}
			else
				break;
		}			
		strcpy(g_tt_data_set[i].time, time);
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
			if (strcmp(g_global_data_set[0][i].time, g_tt_data_set[i].time) != 0)
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


double
trade_costs(double capital, double capital_plus_gross_result)
{
	double cost_buy, cost_sell;
	
	cost_buy = 5.0; // Custo de compra (corretagem) MyCAP
	cost_buy += cost_buy * (2.0 / 100.0);// ISS sobre a corretagem
	cost_buy += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost_buy += capital * (0.006 / 100.0); // Liquidação

	cost_sell = 5.0; // Custo de venda (corretagem) MyCAP
	cost_sell += cost_sell * (2.0 / 100.0);// ISS sobre a corretagem
	cost_sell += capital_plus_gross_result * (0.019 / 100.0); // Emolumentos Bovespa
	cost_sell += capital_plus_gross_result * (0.006 / 100.0); // Liquidação

	return (cost_buy + cost_sell);
}



void
print_test_samples ()
{
	int i, N;
	char stock[1000];
	char *stock_name;
	char *aux;
	double capital = 40000.0, capital_plus_gross_result;

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		strcpy (stock, g_argv[g_global_data_set[0][i].file_number]);
		stock_name = strrchr (stock, '/') + 1;
		aux = strrchr (stock_name, '.');
		aux[0] = '\0';
		
		if (g_global_data_set[0][i].wnn_predicted_return > 0.0) // nao faz negocio de predicao zero ou negativa
		{
			capital_plus_gross_result = capital * (g_global_data_set[0][i].target_return + 1.0);
			capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
		}

		printf ("%s ; % .6f ; % .6f ; % .6f; %s - capital = R$%.2lf\n", 
			 g_global_data_set[0][i].time,
			 g_global_data_set[0][i].target_return,
			 g_global_data_set[0][i].wnn_predicted_return,
			 g_global_data_set[0][i].wnn_prediction_error,
			 stock_name,
			 capital);
	}

	if (!g_no_extra_predicted_sample)
	{
		strcpy (stock, g_argv[g_global_data_set[0][i].file_number]);
		stock_name = strrchr (stock, '/') + 1;
		aux = strrchr (stock_name, '.');
		aux[0] = '\0';
		
		printf ("%s %.6f %s\n", 
			 stock_name, 
			 g_global_data_set[0][i].wnn_predicted_return, 
			 g_global_data_set[0][i].time);
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
	int wnn_up = 0, target_up = 0, wnn_down = 0, target_down = 0;
	double capital, capital_plus_gross_result;
	
	print_test_samples ();

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	capital = 40000.0;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		average_wnn_prediction_error += g_global_data_set[0][i].wnn_prediction_error;
		average_target_return += g_global_data_set[0][i].target_return;
		
		if (g_global_data_set[0][i].wnn_predicted_return > 0.0) // nao faz negocio de predicao zero ou negativa
		{
			capital_plus_gross_result = capital * (g_global_data_set[0][i].target_return + 1.0);
			capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
		}
		
		if (g_global_data_set[0][i].wnn_predicted_return > 0.0)
		{
			wnn_up++;
			if (g_global_data_set[0][i].target_return > 0.0)
				target_up++;
		}
		else if (g_global_data_set[0][i].wnn_predicted_return < 0.0)
		{
			wnn_down++;
			if (g_global_data_set[0][i].target_return < 0.0)
				target_down++;
		}
		// printf ("average_wnn_prediction_error = %f\n", average_wnn_prediction_error);
	}
	average_wnn_prediction_error /= (float) N;
	average_target_return /= (float) N;

	for (i = SAMPLES2CALIBRATE; i < N; i++)
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

	printf ("# **** wnn_up: %.2f, wnn_down: %.2f, wnn_same_signal: %.2f\n", 
			100.0 * ((float) target_up / (float) wnn_up),
			100.0 * ((float) target_down / (float) wnn_down), 
			100.0 * ((float) (target_up + target_down) / (float) (wnn_up + wnn_down)));

	printf ("# **** capital: R$%.2lf\n", capital);
}


#define FAVOR_MAX_RETURN 0


#if FAVOR_MAX_RETURN

void
sumariza_desempenhos (int semana, float *desempenho)
{
	int i, a;
	int wnn_up = 0, target_up = 0, wnn_down = 0, target_down = 0;
	int ultima_semana;
	
	ultima_semana = (g_no_extra_predicted_sample) ? semana : semana - 1;
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = 0.0;
		wnn_up = target_up = wnn_down = target_down = 0;
		for (i = semana-SAMPLES2CALIBRATE; i < ultima_semana; i++)
		{
			if (g_global_data_set[a][i].wnn_predicted_return >= MAX_RETURN)
			{
				wnn_up++;
				if (g_global_data_set[a][i].target_return >= MAX_RETURN)
					target_up++;
			}
			else if (g_global_data_set[a][i].wnn_predicted_return <= MIN_RETURN)
			{
				wnn_down++;
				if (g_global_data_set[a][i].target_return <= MIN_RETURN)
					target_down++;
			}
		}
		desempenho[a] = (double) (target_up+target_down) / (double) (wnn_up+wnn_down);
	}
}

#else

void
sumariza_desempenhos (int semana, float *desempenho)
{
	int i, a;
	int wnn_up = 0, target_up = 0, wnn_down = 0, target_down = 0;
	int ultima_semana;
	
	ultima_semana = (g_no_extra_predicted_sample) ? semana : semana - 1;
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = 0.0;
		wnn_up = target_up = wnn_down = target_down = 0;
		for (i = semana-SAMPLES2CALIBRATE; i < ultima_semana; i++)
		{
			if (g_global_data_set[a][i].wnn_predicted_return > 0.0)
			{
				wnn_up++;
				if (g_global_data_set[a][i].target_return > 0.0)
					target_up++;
			}
			else if (g_global_data_set[a][i].wnn_predicted_return < 0.0)
			{
				wnn_down++;
				if (g_global_data_set[a][i].target_return < 0.0)
					target_down++;
			}
		}
		desempenho[a] = (double) (target_up) / (double) (wnn_up);
	}
}

#endif


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
		if ((max - min) == 0.0)
			vetor[a] = 0.000001;
		else
			vetor[a] = 0.000001 + (vetor[a] - min) / (max - min);
	}
}



void
computa_media_desempenhos (float *media_desempenhos, float *desempenho, float *desempenho_corrente, int tamanho)
{
	int a;
	
	for (a = 1; a < tamanho; a++)
	{
//		media_desempenhos[a] = 1.0 / ((1.0 / desempenho[a]) + (1.0 / desempenho_corrente[a]));
		media_desempenhos[a] = (desempenho[a] + 3.0 * desempenho_corrente[a]) / 4.0;
//		media_desempenhos[a] = (desempenho[a] * desempenho_corrente[a]);
	}
}
	


void
escolhe_acao (int semana, float *media_desempenhos, int tamanho)
{
	int a, melhor_acao;
	float max = -1000.0;
//	float rand_val;
	
	melhor_acao = tamanho;
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
	float *desempenho;
	float *desempenho_corrente;
	float *media_desempenhos;
	
	desempenho = (float *) malloc(g_argc * sizeof(float));
	desempenho_corrente = (float *) malloc(g_argc * sizeof(float));
	media_desempenhos = (float *) malloc(g_argc * sizeof(float));
	for (semana = SAMPLES2CALIBRATE; semana < g_tt_num_samples; semana++)
	{
		sumariza_desempenhos (semana, desempenho);
		normaliza_vetor (desempenho, g_argc);

		get_desempenho_corrente (semana, desempenho_corrente);
		normaliza_vetor (desempenho_corrente, g_argc);

		computa_media_desempenhos (media_desempenhos, desempenho, desempenho_corrente, g_argc);
		escolhe_acao (semana, media_desempenhos, g_argc);
	}
	free(media_desempenhos);
	free(desempenho_corrente);
	free(desempenho);
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
