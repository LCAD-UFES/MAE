#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include "mae.h"
#include "../wnn_pred.h"
#ifdef USE_IPC
#include "ipc.h"
#endif
#include "messages.h"
#include "trading_system.h"


//#define	TEST_MISSING_QUOTES
extern double INITIAL_CAPITAL;


double
trade_costs(double capital, double capital_plus_gross_result)
{
	double cost_buy, cost_sell;

#ifdef	MyCAP_TRADING_COSTS
	cost_buy = 5.0; // Custo de compra (corretagem) MyCAP
	cost_buy += cost_buy * (2.0 / 100.0);// ISS sobre a corretagem
	cost_buy += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost_buy += capital * (0.006 / 100.0); // Liquida��o

	cost_sell = 5.0; // Custo de venda (corretagem) MyCAP
	cost_sell += cost_sell * (2.0 / 100.0);// ISS sobre a corretagem
	cost_sell += capital_plus_gross_result * (0.019 / 100.0); // Emolumentos Bovespa
	cost_sell += capital_plus_gross_result * (0.006 / 100.0); // Liquida��o
#else
	cost_buy = 0.0; // Custo fixo de compra (corretagem) 
	cost_buy += capital * (0.070 / 100.0); // Correntagem
	cost_buy += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost_buy += capital * (0.006 / 100.0); // Liquida��o

	cost_sell = 0.0; // Custo fixo de venda (corretagem) 
	cost_sell += capital_plus_gross_result * (0.070 / 100.0); // Correntagem
	cost_sell += capital_plus_gross_result * (0.019 / 100.0); // Emolumentos Bovespa
	cost_sell += capital_plus_gross_result * (0.006 / 100.0); // Liquida��o
#endif
	return (cost_buy + cost_sell);
}



int
main_signal(int week)
{
	int i;
	int sum_signals = 0;
	
	for (i = 1; i < g_argc; i++)
	{
		if (g_tt_data_set[i][week].wnn_predicted_return > 0.0)
			sum_signals++;
		else
			sum_signals--;
	}
	return (sum_signals);
}



void
print_test_samples()
{
	int i, N;
	char stock[1000];
	char *stock_name;
	char *aux;
	double capital = INITIAL_CAPITAL, capital_plus_gross_result;

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		strcpy(stock, g_argv[g_tt_data_set[0][i].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		
		if ((g_tt_data_set[0][i].wnn_predicted_return > 0.0) && // nao faz negocio de predicao zero ou negativa
		    (g_tt_data_set[0][i].target_return != 0.0))
		{
			capital_plus_gross_result = capital * (g_tt_data_set[0][i].target_return + 1.0);
			capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
		}

		printf("%s ; % .6f ; % .6f ; % .6f; %s - capital = R$%.2lf  %+d\n", 
			 long2isodate(g_tt_data_set[0][i].date_l),
			 g_tt_data_set[0][i].target_return,
			 g_tt_data_set[0][i].wnn_predicted_return,
			 g_tt_data_set[0][i].wnn_prediction_error,
			 stock_name,
			 capital,
		         main_signal(i));
	}

	if (!g_no_extra_predicted_sample)
	{
		strcpy(stock, g_argv[g_tt_data_set[0][i].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		
		printf("%s %.6f %s\n", 
			 stock_name, 
			 g_tt_data_set[0][i].wnn_predicted_return, 
			 long2isodate(g_tt_data_set[0][i].date_l));
	}
}



void
show_statistics()
{
	int N, i;
	float average_wnn_prediction_error = 0.0;
	float standard_deviation_wnn_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;
	double capital, capital_plus_gross_result;

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
	
	print_test_samples();

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	capital = INITIAL_CAPITAL;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		tgt = g_tt_data_set[0][i].target_return;
		out = g_tt_data_set[0][i].wnn_predicted_return;
		err = g_tt_data_set[0][i].wnn_prediction_error;

		average_target_return += tgt;
		average_wnn_prediction_error += err;
		
		if ((out > 0.0) && // nao faz negocio de predicao zero ou negativa
		    (tgt != 0.0))
		{
			capital_plus_gross_result = capital * (tgt + 1.0);
			capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
		}
		
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

	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		tgt = g_tt_data_set[0][i].target_return;
		err = g_tt_data_set[0][i].wnn_prediction_error;

		standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        (err - average_wnn_prediction_error);
		standard_deviation_target_return += (tgt - average_target_return) *
					            (tgt - average_target_return);
	}
	standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (float) (N-1));
	standard_deviation_target_return = sqrt(standard_deviation_target_return / (float) (N-1));
	
	printf("# **** average_target_return: % .6f, average_wnn_error: % .6f\n",
		       average_target_return,
		       average_wnn_prediction_error);

	printf("# **** standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
		       standard_deviation_target_return,
		       standard_deviation_wnn_error);

	printf("# **** hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n", 100.0 * hr_up, 100.0 * hr_dn, 100.0 * hr);

	printf("# **** capital inicial: R$%.2lf  capital final: R$%.2lf  retorno acumulado: %.2f%%\n", INITIAL_CAPITAL, capital, 100.0 * (capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
}


#define PERFORMANCE_SUMMARY_TYPE 0


#if PERFORMANCE_SUMMARY_TYPE == 0

void
sumariza_desempenhos(int semana, float *desempenho)
{
	int i, a;
	int ultima_semana;
	
	double tgt, out;
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
	
	ultima_semana = (g_no_extra_predicted_sample) ? semana : semana - 1;
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = 0.0;
		for (i = semana-SAMPLES2CALIBRATE; i < ultima_semana; i++)
		{
			tgt = g_tt_data_set[a][i].target_return;
			out = g_tt_data_set[a][i].wnn_predicted_return;

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

		desempenho[a] = hr;
	}
}

#else

void
sumariza_desempenhos(int semana, float *desempenho)
{
	int i, a;
	int ultima_semana;
	
	double average_wnn_prediction_error, standard_deviation_wnn_error, err, N;
	
	ultima_semana = (g_no_extra_predicted_sample) ? semana : semana - 1;
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		standard_deviation_wnn_error = average_wnn_prediction_error = desempenho[a] = 0.0;
		N = (double) SAMPLES2CALIBRATE;
		for (i = semana-SAMPLES2CALIBRATE; i < ultima_semana; i++)
			average_wnn_prediction_error += g_tt_data_set[a][i].wnn_prediction_error;
		average_wnn_prediction_error /= N;
		
		for (i = semana-SAMPLES2CALIBRATE; i < ultima_semana; i++)
		{
			err = g_tt_data_set[a][i].wnn_prediction_error;
			
			standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
							(err - average_wnn_prediction_error);
		}
		standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (N - 1.0));

		desempenho[a] = standard_deviation_wnn_error;
	}
}

#endif


void
get_desempenho_corrente(int semana, float *desempenho)
{
	int a;
	
	desempenho[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		desempenho[a] = g_tt_data_set[a][semana].wnn_predicted_return;
	}
}



void
print_vetor(float *vetor, int tamanho)
{
	int a;
	
	for (a = 1; a < tamanho; a++)
	{
		printf("% .2f ", vetor[a]);
	}
	printf("\n");
}



void
normaliza_vetor(float *vetor, int tamanho)
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
computa_media_desempenhos(float *media_desempenhos, float *desempenho, float *desempenho_corrente, int tamanho)
{
	int a;
	
	for (a = 1; a < tamanho; a++)
	{
//		media_desempenhos[a] = 1.0 / ((1.0 / desempenho[a]) + (1.0 / desempenho_corrente[a]));
		media_desempenhos[a] = (desempenho[a] + 3.0 * desempenho_corrente[a]) / 4.0;
//		media_desempenhos[a] = desempenho_corrente[a] / desempenho[a];
//		if (desempenho_corrente[a] < TRADING_SYSTEM_STOP_GAIN)
//			media_desempenhos[a] = (100.0 * (desempenho[a] - 0.5) * desempenho_corrente[a]);
//		else
//			media_desempenhos[a] = (100.0 * (desempenho[a] - 0.5) * TRADING_SYSTEM_STOP_GAIN);
//		printf("desempenho_corrente = %f, desempenho = %f\n", desempenho_corrente[a], desempenho[a]);
	}
}



void
print_unordered_ranking(float *media_desempenhos, int tamanho, int semana)
{
	int a;
	char stock[1000];
	char *stock_name;
	char *aux;
	
	printf("### Stock Ranking (unordered) ###\n");
	for (a = 1; a < tamanho; a++)
	{
		strcpy(stock, g_argv[g_tt_data_set[a][semana].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		printf("# %s %f  %.2f%%\n", stock_name, media_desempenhos[a],
					    100.0 * g_tt_data_set[a][semana].wnn_predicted_return);
	}
	
	printf("###\n### Predition signal sum = %+d\n", main_signal(semana));

	strcpy(stock, g_argv[g_tt_data_set[0][semana].file_number]);
	stock_name = strrchr(stock, '/') + 1;
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';

	printf("### Melhor acao de hoje a %s = %s, Retorno predito = %.2f%%\n", 
		long2isodate(g_tt_data_set[0][semana].date_l),
		stock_name,
		100.0 * g_tt_data_set[0][semana].wnn_predicted_return);
}


int
escolhe_acao(int semana, float *media_desempenhos, int tamanho)
{
	int a, melhor_acao;
	float max;
//	float rand_val;
	
	melhor_acao = 1;
	max = media_desempenhos[melhor_acao];
	for (a = 2; a < tamanho; a++)
	{
		if (media_desempenhos[a] > max)
		{
			max = media_desempenhos[a];
			melhor_acao = a;
		}
	}

//	rand_val = (double) rand() / (double) RAND_MAX;
//	melhor_acao = (int) (rand_val * (float) (tamanho-2) + 1.5);

	g_tt_data_set[0][semana] = g_tt_data_set[melhor_acao][semana];
	
	return (melhor_acao);
/*
	if (g_tt_data_set[0][semana].target_return >= STOP_GAIN)
		g_tt_data_set[0][semana].target_return = STOP_GAIN;
	else if (g_tt_data_set[0][semana].target_return <= STOP_LOSS)
		g_tt_data_set[0][semana].target_return = STOP_LOSS;
*/	
}




/*
 ***************************************************************
 * Realiza o trade                                             *
 ***************************************************************
 */

void
trade(int semana)
{
	int target_day;
	char stock_name[1000];
	
	load_intraday_data(stock_name, semana);

	target_day = find_day_index_by_date_l(g_tt_data_set[0][semana].date_l);
	
	if (target_day != -1) // Se igual a -1, deve ser a semana predita, na qual seria feito o trade on-line.
		g_tt_data_set[0][semana].target_return = get_return_trading_system(target_day, stock_name);
}


void
print_desempenhos(char *message, int semana, float *desempenho, int num_stocks)
{
	int a;
	
	printf("#\n# semana = %d, %s\n", semana, message);
	for (a = 1; a < num_stocks; a++)
		printf("# %s %f\n", g_argv[g_tt_data_set[a][semana].file_number], desempenho[a]);
}


void
select_stock()
{
	int semana;
	float *desempenho;
	float *desempenho_corrente;
	float *media_desempenhos;
	int melhor_acao;

	desempenho = (float *) malloc(g_argc * sizeof(float));
	desempenho_corrente = (float *) malloc(g_argc * sizeof(float));
	media_desempenhos = (float *) malloc(g_argc * sizeof(float));
	for (semana = SAMPLES2CALIBRATE; semana < g_tt_num_samples; semana++)
	{
		sumariza_desempenhos(semana, desempenho);
		normaliza_vetor(desempenho, g_argc);

		get_desempenho_corrente(semana, desempenho_corrente);
		normaliza_vetor(desempenho_corrente, g_argc);

		computa_media_desempenhos(media_desempenhos, desempenho, desempenho_corrente, g_argc);
		melhor_acao = escolhe_acao(semana, media_desempenhos, g_argc);
		// printf("melhor acao = %s, desempenho = %f, desempenho_corrente = %f\n", 
		//	g_argv[g_tt_data_set[melhor_acao][semana].file_number],
		//	desempenho[melhor_acao], desempenho_corrente[melhor_acao]);
		
		// trade(semana);
	}
	semana--;
	print_unordered_ranking(media_desempenhos, g_argc, semana);

	free(media_desempenhos);
	free(desempenho_corrente);
	free(desempenho);
}



#ifdef	USE_IPC

void
send_quote(float quote, int target_day_index, int trading_system_current_week, int moving_average_current_minute)
{
	FILE *quote_file;
	FILE *control_file;
	char now[1000];
	
	usleep(1000000/5);
	
	// Espera que seja lido o arquivo de quotes existente
	while (fileExists(quotesControltxt))
		;
		
	// Cria e escreve no arquivo de quotes para que o quotes_server_order_router leia
	quote_file = fopen(quotestxt, "w");
	
	get_trading_system_time(now, trading_system_current_week, moving_average_current_minute);
	fprintf(quote_file, "1\n%s\n%s\n%.2f\n%d", now, get_best_stock_name(trading_system_current_week), quote, (int) get_best_stock_volume(target_day_index, moving_average_current_minute));
	
	fclose(quote_file);
	
	// Cria o arquivo de controle que fecha o processo de envio da quote para o quotes_server_order_router
	control_file = fopen(quotesControltxt, "w");
	fprintf(control_file, "quote posted\n");
	fclose(control_file);
}



float
get_best_stock_quote_from_quotes_message(char *best_stock_name, qsor_quotes_message* received_quotes)
{
	int i;
	
	for (i = 0; (i < received_quotes->num_stocks) && (strcmp(received_quotes->stock_symbol[i], best_stock_name) != 0); i++)
		;
	
	if (i == received_quotes->num_stocks)
		return (-1.0);
	
	return (received_quotes->price[i]);
}



int
minutes_within_this_week(char *date_and_time, int trading_system_current_week)
{
	long begin_week_day_date_l;
	long message_day_date_l;
	int days_within_this_week;
	int hour, minute;
	
	begin_week_day_date_l = g_tt_data_set[0][trading_system_current_week].date_l - 6 * SECONDS_IN_A_DAY;
	message_day_date_l = isodate2long(date_and_time);

	days_within_this_week = (message_day_date_l - begin_week_day_date_l) / SECONDS_IN_A_DAY;
	if (days_within_this_week > 2) // Passou de sexta
		days_within_this_week = days_within_this_week - 2;
	
	// formato de date_and_time: 2011-03-26 Sat 22:34
	hour = atoi(date_and_time + 15);
	minute = atoi(date_and_time + 18);
	
	return (days_within_this_week * MINUTES_IN_A_DAY + (hour - TRADING_OPENING_HOUR) * 60 + minute);
}



int
fill_in_eventual_missing_quotes(int message_minute, int trading_system_minute, int target_day_index, float best_stock_quote)
{
	float fill_in_price;
	int i;
	
	if ((message_minute - trading_system_minute) == 1) // no missing quotes
		return (message_minute);
		
	if (trading_system_minute == -1) // first minutes not available: fill in with best stock current quote
		fill_in_price = best_stock_quote;
	else
		fill_in_price = g_price_samples[trading_system_minute];
	
	for (i = trading_system_minute + 1; i < message_minute; i++)
		g_price_samples[i] = fill_in_price;
		
	return (message_minute);
}



int
receive_quote(qsor_quotes_message* received_quotes)
{
	char *best_stock_name;
	float best_stock_quote;
	int message_minute;
	
	best_stock_name = get_best_stock_name(g_trading_system_current_week);
	best_stock_quote = get_best_stock_quote_from_quotes_message(best_stock_name, received_quotes);
	if (best_stock_quote == -1.0) // A cotacao da acao nao veio com o quotes message. Inicio do pregao?
		return (0);
	
	message_minute = minutes_within_this_week(received_quotes->date_and_time, g_trading_system_current_week);
	
	g_moving_average_current_minute = fill_in_eventual_missing_quotes(message_minute, g_moving_average_current_minute, g_target_day_index, best_stock_quote);
	
	if (g_moving_average_current_minute <= MINUTES_IN_A_WEEK)
		g_price_samples[g_moving_average_current_minute] = best_stock_quote;
	else
		Erro("g_moving_average_current_minute >= MINUTES_IN_A_WEEK in receive_quote()", "", "");

	return (1);
}



void
get_best_stock_quote(int target_day_index)
{
#ifdef	IPC_ON_LINE
	return;
#else
	int current_day, i;
	float *intraday_price;
	float last_price;
	int new_moving_average_current_minute;
	
#ifdef	TEST_MISSING_QUOTES
	if ((rand() % (MINUTES_IN_A_WEEK / 4)) != 1)
	{
		new_moving_average_current_minute = g_moving_average_current_minute + 1;
	}
	else
	{
		new_moving_average_current_minute = g_moving_average_current_minute + 1 + rand() % 200;
		current_day = get_current_day(target_day_index, new_moving_average_current_minute);
		intraday_price = g_train_data_set[current_day].intraday_price;
		if ((new_moving_average_current_minute >= (MINUTES_IN_A_WEEK - 20)) || (intraday_price == NULL))
			new_moving_average_current_minute = g_moving_average_current_minute + 1;
	}
#else
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
#endif	
	current_day = get_current_day(target_day_index, new_moving_average_current_minute);
	intraday_price = g_train_data_set[current_day].intraday_price;
	while ((intraday_price == NULL) && (new_moving_average_current_minute < (MINUTES_IN_A_WEEK - 1)))
	{	// Tem um dia ou mais de dados faltando -> repete a �ltima cotacao
		if (new_moving_average_current_minute == 0) // O primeiro dia e negocios (quinta) esta faltando). Pega o maximo preco de compra como valor incial
			last_price = get_best_stock_maximum_buy_price(target_day_index, g_trading_system_current_week);
		else
			last_price = g_price_samples[new_moving_average_current_minute - 1];
		for (i = 0; i < MINUTES_IN_A_DAY; i++)
		{
			g_price_samples[new_moving_average_current_minute] = last_price;
			new_moving_average_current_minute++;
		}
		current_day = get_current_day(target_day_index, new_moving_average_current_minute);
		intraday_price = g_train_data_set[current_day].intraday_price;
	}
	if ((new_moving_average_current_minute < MINUTES_IN_A_WEEK) && (intraday_price != NULL))
	{
		send_quote(intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY], target_day_index, g_trading_system_current_week, new_moving_average_current_minute);
	}
	else if (new_moving_average_current_minute == MINUTES_IN_A_WEEK)
	{
		new_moving_average_current_minute = MINUTES_IN_A_WEEK - 2;
		send_quote(last_price, target_day_index, g_trading_system_current_week, new_moving_average_current_minute);
	}
	else
	{
		last_price = g_price_samples[new_moving_average_current_minute - 1];
		send_quote(last_price, target_day_index, g_trading_system_current_week, new_moving_average_current_minute);
	}
#endif
}

#else


void
get_best_stock_quote(int target_day_index)
{
	int current_day, i;
	float *intraday_price;
	float last_price;
	int new_moving_average_current_minute;
	
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
	
	current_day = get_current_day(target_day_index, new_moving_average_current_minute);
	intraday_price = g_train_data_set[current_day].intraday_price;
	while ((intraday_price == NULL) && (new_moving_average_current_minute < MINUTES_IN_A_WEEK))
	{	// Tem um dia ou mais de dados faltando -> repete a �ltima cotacao
		if (new_moving_average_current_minute == 0) // O primeiro dia e negocios (quinta) esta faltando). Pega o fechamento da quarta anterior
			last_price = g_train_data_set[target_day_index - DAYS_IN_A_WEEK].clo;
		else
			last_price = g_price_samples[new_moving_average_current_minute - 1];
		for (i = 0; i < MINUTES_IN_A_DAY; i++)
		{
			g_price_samples[new_moving_average_current_minute] = last_price;
			new_moving_average_current_minute++;
		}
		current_day = get_current_day(target_day_index, new_moving_average_current_minute);
		intraday_price = g_train_data_set[current_day].intraday_price;
	}
	if (new_moving_average_current_minute < MINUTES_IN_A_WEEK)
		g_price_samples[new_moving_average_current_minute] = intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
	else
		new_moving_average_current_minute--;
	
	g_moving_average_current_minute = new_moving_average_current_minute;
}

#endif



void
print_best_stock_and_trade_week(char *stock_name, int target_day_index)
{
	char date_begin_week[1000];
	char dow_begin_week[1000];
	char date_end_week[1000];
	char dow_last_week[1000];
	long date_begin_week_l;
	long date_end_week_l;

	date_begin_week_l = g_train_data_set[target_day_index - 4].date_l;
	date_end_week_l = timePlusDays(date_begin_week_l, 4);
	
	strcpy(date_begin_week, long2isodate(date_begin_week_l));
	strcpy(dow_begin_week, long2dows(date_begin_week_l));
	strcpy(date_end_week, long2isodate(date_end_week_l));
	strcpy(dow_last_week, long2dows(date_end_week_l));
	// printf ("# Best stock = %s; first trade day = %s %s; last trade day = %s %s\n", 
	//	stock_name, date_begin_week, dow_begin_week, date_end_week, dow_last_week);
}



void
run_trading_system_state_machine()
{
	char stock_name[1000];

	g_trading_system_current_week = SAMPLES2CALIBRATE - 1;
	do // Encontra a primeira semana com suficiente quantidade de dados intraday
	{
		g_trading_system_current_week++;
		load_intraday_data(stock_name, g_trading_system_current_week);
		g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_week].date_l);
		print_best_stock_and_trade_week(stock_name, g_target_day_index);
	} while (!enougth_intraday_data(g_target_day_index));
	
	g_moving_average_current_minute = -1;
	do
	{
		if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_WEEK)
		{
			get_best_stock_quote(g_target_day_index);
		}
		else
		{
			do // Encontra a proxima semana com suficiente quantidade de dados intraday
			{
				g_trading_system_current_week++;
				if (g_trading_system_current_week >= g_tt_num_samples)
					break;
				load_intraday_data(stock_name, g_trading_system_current_week); //@@@ Tem que tratar isso para o caso on-line
				g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_week].date_l);
				print_best_stock_and_trade_week(stock_name, g_target_day_index);
			} while (!enougth_intraday_data(g_target_day_index) && (g_trading_system_current_week < g_tt_num_samples));

			if (g_trading_system_current_week < g_tt_num_samples)
			{	// Semana com suficiente quantidade de dados intraday
				g_moving_average_current_minute = -1;		// Esta linha e a de baixo garantem que havera contacao valida
				get_best_stock_quote(g_target_day_index); 	// no primeiro minuto da semana
			}
		}
	} while (trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_week) != QUIT);
}



double
get_initial_capital()
{
	FILE *file;
	double initial_capital;
	int ch;
	
	file = fopen("initial_capital.txt", "r");
	if (file == NULL)
		Erro("Could not open file initial_capital.txt in get_initial_capital()", "", "");
	do
	{
		ch = getc(file);
	} while (!isdigit(ch) && (ch != EOF));
	if (ch == EOF)
		Erro("Could not read a number from initial_capital.txt in get_initial_capital()", "", "");
	ungetc(ch, file);
	
	fscanf(file, "%lf", &initial_capital);
	if (initial_capital <= 0.0)
		Erro("Could not read a number > 0.0 from initial_capital.txt in get_initial_capital()", "", "");
	return (initial_capital);;
}


#ifdef	USE_IPC



void
call_trading_system_state_machine()
{
	char stock_name[1000];
	int state;

	state = trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_week);

	if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_WEEK)
	{
		get_best_stock_quote(g_target_day_index);
	}
	else
	{
		do // Encontra a proxima semana com suficiente quantidade de dados intraday
		{
			g_trading_system_current_week++;
			if (g_trading_system_current_week >= g_tt_num_samples)
				break;
			load_intraday_data(stock_name, g_trading_system_current_week); //@@@ Tem que tratar isso para o caso on-line
			g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_week].date_l);
			print_best_stock_and_trade_week(stock_name, g_target_day_index);
		} while (!enougth_intraday_data(g_target_day_index) && (g_trading_system_current_week < g_tt_num_samples));

		if (g_trading_system_current_week < g_tt_num_samples)
		{	// Semana com suficiente quantidade de dados intraday
			g_moving_average_current_minute = -1;		// Esta linha e a de baixo garantem que havera contacao valida
			get_best_stock_quote(g_target_day_index); 	// no primeiro minuto da semana
		}
		else
			state = trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_week);	// Chegou ao fim -> prepara para o QUIT
	}

	if (state == QUIT)
	{
#ifndef	USE_IPC	
		printf("The trading system has reached the QUIT state.\n");
		
		IPC_disconnect();
	
		free_g_tt_data_set(g_argc);
		free_g_train_data_set();

		exit(0);
#else
		time_t now;
		struct tm *brokentime;

		now = time(NULL);
		brokentime = localtime(&now);
		publish_control_message(2, "The trading system has reached the QUIT state.\n", asctime(brokentime), TS_NAME);
#endif
	}
}


void
init_trading_system_state_machine()
{
	char stock_name[1000];

	g_trading_system_current_week = SAMPLES2CALIBRATE - 1;
	do // Encontra a primeira semana com suficiente quantidade de dados intraday
	{
		g_trading_system_current_week++;
		load_intraday_data(stock_name, g_trading_system_current_week);
		g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_week].date_l);
		print_best_stock_and_trade_week(stock_name, g_target_day_index);
	} while (!enougth_intraday_data(g_target_day_index));
	
	g_moving_average_current_minute = -1;
	get_best_stock_quote(g_target_day_index);
}



void
hot_start_trading_system_state_machine()
{
	char stock_name[1000];

	get_state_in_a_hot_start();
	
	load_intraday_data(stock_name, g_trading_system_current_week);
	print_best_stock_and_trade_week(stock_name, g_target_day_index);
	get_best_stock_quote(g_target_day_index);
}



int
init_trading_system(int argc, char **argv)
{
	int i;
	STATE state;
	
	if (argc < 2)
	{
		printf("Erro: It is necessary at least one input file.\n");
		exit(1);
	}

	g_argv = argv;
	g_argc = argc;

	INITIAL_CAPITAL = get_initial_capital();

	for (i = 1; i < argc; i++)
		process_one_file(i);

	select_stock();
	// show_statistics();
	
	state = get_state();
	if (state.state == INITIALIZE)
	{
		save_state(state);
		init_trading_system_state_machine();
	}
	else
	{
		hot_start_trading_system_state_machine();
	}
	
	return (0);
}

#else

int
main(int argc, char **argv)
{
	int i;
	STATE state;
	
	if (argc < 2)
	{
		printf("Erro: It is necessary at least one input file.\n");
		exit(1);
	}

	g_argv = argv;
	g_argc = argc;
	
	INITIAL_CAPITAL = get_initial_capital();

	for (i = 1; i < argc; i++)
		process_one_file(i);

	select_stock();
	show_statistics();
	
	state = get_state();
	if (state.state == INITIALIZE)
		save_state(state);

	run_trading_system_state_machine();
	
	free_g_tt_data_set(argc);
	free_g_train_data_set();
	
	return (0);
}

#endif
