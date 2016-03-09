#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"


//#define	TEST_MISSING_QUOTES



double
compute_new_capital(double capital, double *buy_price, double *sell_price, int tt_data_set_index)
{
	int day_index, quantity;
	double transaction_amount, new_capital;
//	double return_difference;

	load_daily_data(g_argv, g_tt_data_set[0][tt_data_set_index].file_number);
	day_index = find_day_index_by_date_l(g_tt_data_set[0][tt_data_set_index].date_l);

	*buy_price = g_train_data_set[day_index].buy;
	*sell_price = g_train_data_set[day_index].sell;
	if (*buy_price == 0.0) // nao conseguiu fazer negocio neste dia
		return (capital);
		
/*	return_difference = g_tt_data_set[0][tt_data_set_index].target_return - ((*sell_price - *buy_price) / *buy_price);
	if (fabs(return_difference) > 0.00001)
		Erro("tt data set return different from train data set return in compute_new_capital()", "", "");
*/
	quantity = capital / *buy_price;
	quantity = quantity - quantity % 100; // Lotes de 100 acoes
	// Compra
	transaction_amount = (float) quantity * *buy_price;
	new_capital = capital - transaction_amount - trade_cost(transaction_amount);
	// Venda
	transaction_amount = (float) quantity * *sell_price;
	new_capital = new_capital + transaction_amount - trade_cost(transaction_amount);
	
	return (new_capital);
}



double
print_test_samples()
{
	int i, N;
	char stock[1000];
	char *stock_name;
	char *aux;
	double capital = CAPITAL;
	double buy_price, sell_price;

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		strcpy(stock, g_argv[g_tt_data_set[0][i].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		
		if (g_tt_data_set[0][i].wnn_predicted_return > 0.0) // nao faz negocio de predicao zero ou negativa
		{
			capital = compute_new_capital(capital, &buy_price, &sell_price, i);
		}

		printf("%s ; % .6f ; % .6f ; % .6f; %s - capital = R$%.2lf\tbuy price = R$%.2lf\tsell price = R$%.2lf\n", 
			 long2isodate(g_tt_data_set[0][i].date_l),
			 g_tt_data_set[0][i].target_return,
			 g_tt_data_set[0][i].wnn_predicted_return,
			 g_tt_data_set[0][i].wnn_prediction_error,
			 stock_name,
			 capital, buy_price, sell_price);
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
	
	return (capital);
}



void
show_statistics()
{
	int N, i;
	float average_wnn_prediction_error = 0.0;
	float standard_deviation_wnn_error = 0.0;
	float average_target_return = 0.0;
	float standard_deviation_target_return = 0.0;
	double capital;

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
	
	capital = print_test_samples();

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	for (i = SAMPLES2CALIBRATE; i < N; i++)
	{
		tgt = g_tt_data_set[0][i].target_return;
		out = g_tt_data_set[0][i].wnn_predicted_return;
		err = g_tt_data_set[0][i].wnn_prediction_error;

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

	printf("# **** capital inicial: R$%.2lf  capital final: R$%.2lf  retorno acumulado: %.2f%%\n", CAPITAL, capital, 100.0 * (capital - CAPITAL) / CAPITAL);
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
		media_desempenhos[a] = (desempenho[a] + 2.0 * desempenho_corrente[a]) / 3.0;
//		media_desempenhos[a] = desempenho_corrente[a] / desempenho[a];
//		printf("desempenho_corrente = %f, desempenho = %f\n", desempenho_corrente[a], desempenho[a]);
//		media_desempenhos[a] = (desempenho[a] * desempenho_corrente[a]);
	}
}



void
print_unordered_ranking(float *media_desempenhos, int tamanho, int semana)
{
	int a;
	
	printf("### Stock Ranking (unordered) ###\n");
	for (a = 1; a < tamanho; a++)
		printf("# %s %f\n", g_argv[g_tt_data_set[a][semana].file_number], media_desempenhos[a]);
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
	}
	print_unordered_ranking(media_desempenhos, g_argc, g_tt_num_samples-1);

	free(media_desempenhos);
	free(desempenho_corrente);
	free(desempenho);
}



#ifdef	USE_IPC

void
send_quote(float quote, int target_day_index, int trading_system_current_day, int moving_average_current_minute)
{
	FILE *quote_file;
	FILE *control_file;
	char now[1000];
	
	// Espera que seja lido o arquivo de quotes existente
	while (fileExists(quotesControltxt))
		;
		
	// Cria e escreve no arquivo de quotes para que o quotes_server_order_router leia
	quote_file = fopen(quotestxt, "w");
	
	get_trading_system_time(now, trading_system_current_day, moving_average_current_minute);
	fprintf(quote_file, "1\n%s\n%s\n%.2f\n%d", now, get_best_stock_name(trading_system_current_day), quote, (int) get_best_stock_volume(target_day_index, moving_average_current_minute));
	
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
minutes_within_this_day(char *date_and_time)
{
	int hour, minute;
	
	// formato de date_and_time: 2011-03-26 Sat 22:34
	hour = atoi(date_and_time + 15);
	minute = atoi(date_and_time + 18);
	
	return ((hour - TRADING_OPENING_HOUR) * 60 + minute);
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
	
	best_stock_name = get_best_stock_name(g_trading_system_current_day);
	best_stock_quote = get_best_stock_quote_from_quotes_message(best_stock_name, received_quotes);
	if (best_stock_quote == -1.0) // A cotacao da acao nao veio com o quotes message. Inicio do pregao?
		return (0);
	
	message_minute = minutes_within_this_day(received_quotes->date_and_time);
	
	g_moving_average_current_minute = fill_in_eventual_missing_quotes(message_minute, g_moving_average_current_minute, g_target_day_index, best_stock_quote);
	
	if (g_moving_average_current_minute <= MINUTES_IN_A_DAY)
		g_price_samples[g_moving_average_current_minute] = best_stock_quote;
	else
		Erro("g_moving_average_current_minute >= MINUTES_IN_A_DAY in receive_quote()", "", "");

	return (1);
}



void
get_best_stock_quote(int target_day_index)
{
#ifdef	IPC_ON_LINE
	return;
#else
	int current_day;
	float *intraday_price;
	float last_price;
	int new_moving_average_current_minute;
	
#ifdef	TEST_MISSING_QUOTES
	if ((rand() % (MINUTES_IN_A_DAY / 4)) != 1)
	{
		new_moving_average_current_minute = g_moving_average_current_minute + 1;
	}
	else
	{
		new_moving_average_current_minute = g_moving_average_current_minute + 1 + rand() % 200;
		current_day = get_current_day(target_day_index);
		intraday_price = g_train_data_set[current_day].intraday_price;
		if ((new_moving_average_current_minute >= (MINUTES_IN_A_DAY - 20)) || (intraday_price == NULL))
			new_moving_average_current_minute = g_moving_average_current_minute + 1;
	}
#else
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
#endif	
	current_day = get_current_day(target_day_index);
	intraday_price = g_train_data_set[current_day].intraday_price;
	if (intraday_price == NULL)
		Erro("intraday_price = NULL in get_best_stock_quote()", "", "");
	if (new_moving_average_current_minute < MINUTES_IN_A_DAY)
	{
		send_quote(intraday_price[new_moving_average_current_minute], target_day_index, g_trading_system_current_day, new_moving_average_current_minute);
	}
	else
	{
		last_price = g_price_samples[new_moving_average_current_minute - 1];
		send_quote(last_price, target_day_index, g_trading_system_current_day, new_moving_average_current_minute);
	}
#endif
}

#else


void
get_best_stock_quote(int target_day_index)
{
	int current_day;
	float *intraday_price;
	int new_moving_average_current_minute;
	
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
	
	current_day = get_current_day(target_day_index);
	intraday_price = g_train_data_set[current_day].intraday_price;
	if (intraday_price == NULL)
		Erro("intraday_price = NULL in get_best_stock_quote()", "", "");
	if (new_moving_average_current_minute < MINUTES_IN_A_DAY)
		g_price_samples[new_moving_average_current_minute] = intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
	else
		new_moving_average_current_minute--;
	
	g_moving_average_current_minute = new_moving_average_current_minute;
}

#endif


void
print_best_stock_and_day_of_trade(char *stock_name, int target_day_index)
{
	char day_of_trade[1000];
	char day_of_trade_dow[1000];
	long day_of_trade_date_l;

	if (target_day_index != g_train_num_samples)
		day_of_trade_date_l = g_train_data_set[target_day_index].date_l;
	else
		day_of_trade_date_l = timePlusDays(g_train_data_set[target_day_index-1].date_l, 1); // Se o target_day_index for no proximo dia ainda nao disponivel. Assume que g_train_data_set[] nao tem gaps.
	
	strcpy(day_of_trade, long2isodate(day_of_trade_date_l));
	strcpy(day_of_trade_dow, long2dows(day_of_trade_date_l));
	printf ("# Best stock = %s; day of trade = %s %s\n", 
		stock_name, day_of_trade, day_of_trade_dow);
}



void
run_trading_system_state_machine()
{
	char stock_name[1000];

	g_trading_system_current_day = SAMPLES2CALIBRATE - 1;
	do // Encontra o primeiro dia com dados intraday
	{
		g_trading_system_current_day++;
		load_intraday_data(stock_name, g_trading_system_current_day);
		g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_day].date_l);
		print_best_stock_and_day_of_trade(stock_name, g_target_day_index);
	} while (!intraday_data_available(g_target_day_index));
	
	g_moving_average_current_minute = -1;
	do
	{
		if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_DAY)
		{
			get_best_stock_quote(g_target_day_index);
		}
		else
		{
			do // Encontra a proxima semana com suficiente quantidade de dados intraday
			{
				g_trading_system_current_day++;
				if (g_trading_system_current_day >= g_tt_num_samples)
					break;
				load_intraday_data(stock_name, g_trading_system_current_day); //@@@ Tem que tratar isso para o caso on-line
				g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_day].date_l);
				print_best_stock_and_day_of_trade(stock_name, g_target_day_index);
			} while (!intraday_data_available(g_target_day_index) && (g_trading_system_current_day < g_tt_num_samples));

			if (g_trading_system_current_day < g_tt_num_samples)
			{	// Semana com suficiente quantidade de dados intraday
				g_moving_average_current_minute = -1;		// Esta linha e a de baixo garantem que havera contacao valida
				get_best_stock_quote(g_target_day_index); 	// no primeiro minuto da semana
			}
		}
	} while (trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_day) != QUIT);
}



#ifdef	USE_IPC



void
call_trading_system_state_machine()
{
	char stock_name[1000];
	int state;

	state = trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_day);

	if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_DAY)
	{
		get_best_stock_quote(g_target_day_index);
	}
	else
	{
		do // Encontra a proxima semana com suficiente quantidade de dados intraday
		{
			g_trading_system_current_day++;
			if (g_trading_system_current_day >= g_tt_num_samples)
				break;
			load_intraday_data(stock_name, g_trading_system_current_day); //@@@ Tem que tratar isso para o caso on-line
			g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_day].date_l);
			print_best_stock_and_day_of_trade(stock_name, g_target_day_index);
		} while (!intraday_data_available(g_target_day_index) && (g_trading_system_current_day < g_tt_num_samples));

		if (g_trading_system_current_day < g_tt_num_samples)
		{	// Semana com suficiente quantidade de dados intraday
			g_moving_average_current_minute = -1;		// Esta linha e a de baixo garantem que havera contacao valida
			get_best_stock_quote(g_target_day_index); 	// no primeiro minuto da semana
		}
		else
			state = trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_day);	// Chegou ao fim -> prepara para o QUIT
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

	g_trading_system_current_day = SAMPLES2CALIBRATE - 1;
	do // Encontra a primeira semana com suficiente quantidade de dados intraday
	{
		g_trading_system_current_day++;
		load_intraday_data(stock_name, g_trading_system_current_day);
		g_target_day_index = find_day_index_by_date_l(g_tt_data_set[0][g_trading_system_current_day].date_l);
		print_best_stock_and_day_of_trade(stock_name, g_target_day_index);
	} while (!intraday_data_available(g_target_day_index));
	
	g_moving_average_current_minute = -1;
	get_best_stock_quote(g_target_day_index);
}



void
hot_start_trading_system_state_machine()
{
	char stock_name[1000];

	get_state_in_a_hot_start();
	
	load_intraday_data(stock_name, g_trading_system_current_day);
	print_best_stock_and_day_of_trade(stock_name, g_target_day_index);
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


void
build_daily_buy_sell_files(int num_stocks, char **stocks_names)
{
	int stock;
	int num_days, day, target_day_index, minute;
	STATE state;
	BUY_SELL_FILES buy_sell_files;

	for (stock = 1; stock < num_stocks; stock++)
	{
		printf("stock = %d %s\n", stock, stocks_names[stock]);
		
		// Le o intraday data apenas uma vez por stock, já que a funcao lê todos os dias disponíveis
		load_intraday_data2(stocks_names, stock);
		
		// Acha  o primerio dia com dados intraday; conta o numero de dias a partir deste primeiro dia com dados intraday ate o presente
		num_days = get_num_days_of_intraday_data(stock, stocks_names);
		
		// Cria um fake_tt_data_set para apenas uma acao em todos os diuas usando o train_data_set; uma linha de tt por dia de train 
		// Marca dias sem dados intraday no tt data set colocando file_number = -1
		build_fake_tt_data_set(stock, num_days);
		
		buy_sell_files = open_buy_sell_files(stocks_names[stock]);
		for (day = 0; day < num_days; day++)
		{
			printf("day = %d of %d days\n", day, num_days);
			state = init_state();
			save_state(state);
			if (g_tt_data_set[0][day].file_number != -1)
			{
				target_day_index = find_day_index_by_date_l(g_tt_data_set[0][day].date_l);
				moving_average_warm_up(target_day_index);
				for (minute = 0; minute < MINUTES_IN_A_DAY; minute++)
				{
					get_best_stock_quote(target_day_index);
					trading_system_state_machine(minute, target_day_index, day);
				}
				state = get_state();
				save_buy_sell_file_line(buy_sell_files, day, state);
			}
		}
		close_buy_sell_files(buy_sell_files);
		free_g_tt_data_set(1);
		free_g_train_data_set();
	}
}



int
main(int argc, char **argv)
{
	g_argv = argv;
	g_argc = argc;

#ifdef	BUILD_DAILY_BUY_SELL_FILES
	build_daily_buy_sell_files(argc, argv);
#else
	int i;
	STATE state;
	
	if (argc < 2)
	{
		printf("Erro: It is necessary at least one input file.\n");
		exit(1);
	}

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
#endif	
	return (0);
}

#endif
