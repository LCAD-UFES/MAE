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


struct _performace
{
	double performance;
	int stock;
};

typedef struct _performace PERFORMANCE;
	



double
trade_costs(double capital, double capital_plus_gross_result)
{
	double cost_buy, cost_sell;

#ifdef	MyCAP_TRADING_COSTS
	cost_buy = 5.0; // Custo de compra (corretagem) MyCAP
	cost_buy += cost_buy * (2.0 / 100.0);// ISS sobre a corretagem
	cost_buy += capital * (0.0285 / 100.0); // Emolumentos Bovespa
	cost_buy += capital * (0.006 / 100.0); // Liquida��o

	cost_sell = 5.0; // Custo de venda (corretagem) MyCAP
	cost_sell += cost_sell * (2.0 / 100.0);// ISS sobre a corretagem
	cost_sell += capital_plus_gross_result * (0.0285 / 100.0); // Emolumentos Bovespa
	cost_sell += capital_plus_gross_result * (0.006 / 100.0); // Liquida��o
#else
	cost_buy = 0.0; // Custo fixo de compra (corretagem) 
	cost_buy += capital * (0.070 / 100.0); // Correntagem
	cost_buy += capital * (0.0285 / 100.0); // Emolumentos Bovespa
	cost_buy += capital * (0.006 / 100.0); // Liquida��o

	cost_sell = 0.0; // Custo fixo de venda (corretagem) 
	cost_sell += capital_plus_gross_result * (0.070 / 100.0); // Correntagem
	cost_sell += capital_plus_gross_result * (0.0285 / 100.0); // Emolumentos Bovespa
	cost_sell += capital_plus_gross_result * (0.006 / 100.0); // Liquida��o
#endif
	return (cost_buy + cost_sell);
}



int
main_signal_buy_sell(int week)
{
	int stock;
	int sum_signals = 0;
	
	for (stock = 1; stock < g_argc; stock++)
	{
		if (g_tt_data_set[stock][week].wnn_predicted_return_buy_sell > 0.0)
			sum_signals++;
		else
			sum_signals--;
	}
	return (sum_signals);
}



int
main_signal_sell_buy(int week)
{
	int stock;
	int sum_signals = 0;
	
	for (stock = 1; stock < g_argc; stock++)
	{
		if (g_tt_data_set[stock][week].wnn_predicted_return_sell_buy > 0.0)
			sum_signals++;
		else
			sum_signals--;
	}
	return (sum_signals);
}



void
print_test_samples()
{
	int week, N;
	char stock[1000];
	char *stock_name;
	char *aux;
	double capital = INITIAL_CAPITAL, capital_plus_gross_result;

	N = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	for (week = SAMPLES2CALIBRATE; week < N; week++)
	{
		strcpy(stock, g_argv[g_tt_data_set[0][week].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		
		if (g_tt_data_set[0][week].trade_order == BUY_SELL)
		{
			if ((g_tt_data_set[0][week].wnn_predicted_return_buy_sell > 0.0) && // nao faz negocio de predicao zero ou negativa
			    (g_tt_data_set[0][week].target_return_buy_sell != 0.0))
			{
				capital_plus_gross_result = capital * (g_tt_data_set[0][week].target_return_buy_sell + 1.0);
				capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
			}
			printf("%s ; % .6f ; % .6f ; % .6f; BUY_SELL %s - capital = R$%.2lf  %+d\n", 
				 long2isodate(g_tt_data_set[0][week].date_l),
				 g_tt_data_set[0][week].target_return_buy_sell,
				 g_tt_data_set[0][week].wnn_predicted_return_buy_sell,
				 g_tt_data_set[0][week].wnn_prediction_error_buy_sell,
				 stock_name,
				 capital,
		        	 main_signal_buy_sell(week));
		}
		else
		{
			if ((g_tt_data_set[0][week].wnn_predicted_return_sell_buy > 0.0) && // nao faz negocio de predicao zero ou negativa
			    (g_tt_data_set[0][week].target_return_sell_buy != 0.0))
			{
				capital_plus_gross_result = capital * (g_tt_data_set[0][week].target_return_sell_buy + 1.0);
				capital = capital_plus_gross_result - trade_costs(capital, capital_plus_gross_result);
			}
			printf("%s ; % .6f ; % .6f ; % .6f; SELL_BUY %s - capital = R$%.2lf  %+d\n", 
				 long2isodate(g_tt_data_set[0][week].date_l),
				 g_tt_data_set[0][week].target_return_sell_buy,
				 g_tt_data_set[0][week].wnn_predicted_return_sell_buy,
				 g_tt_data_set[0][week].wnn_prediction_error_sell_buy,
				 stock_name,
				 capital,
		        	 main_signal_sell_buy(week));
		}
	}

	if (!g_no_extra_predicted_sample)
	{
		strcpy(stock, g_argv[g_tt_data_set[0][week].file_number]);
		stock_name = strrchr(stock, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		
		if (g_tt_data_set[0][week].trade_order == BUY_SELL)
			printf("BUY_SELL %s, predicted_return = %.6f, %s\n", 
				 stock_name, 
				 g_tt_data_set[0][week].wnn_predicted_return_buy_sell, 
				 long2isodate(g_tt_data_set[0][week].date_l));
		else
			printf("SELL_BUY %s, predicted_return = %.6f, %s\n", 
				 stock_name, 
				 g_tt_data_set[0][week].wnn_predicted_return_sell_buy, 
				 long2isodate(g_tt_data_set[0][week].date_l));
	}
}



void
show_statistics()
{
	int N, week, last_sample;
	double average_wnn_prediction_error = 0.0;
	double standard_deviation_wnn_error = 0.0;
	double average_target_return = 0.0;
	double standard_deviation_target_return = 0.0;
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

	last_sample = (g_no_extra_predicted_sample) ? g_tt_num_samples : g_tt_num_samples - 1;
	N = 0;
	capital = INITIAL_CAPITAL;
	for (week = SAMPLES2CALIBRATE; week < last_sample; week++)
	{
		if (g_tt_data_set[0][week].trade_order == BUY_SELL)
		{
			tgt = g_tt_data_set[0][week].target_return_buy_sell;
			out = g_tt_data_set[0][week].wnn_predicted_return_buy_sell;
			err = g_tt_data_set[0][week].wnn_prediction_error_buy_sell;
		}
		else
		{
			tgt = g_tt_data_set[0][week].target_return_sell_buy;
			out = g_tt_data_set[0][week].wnn_predicted_return_sell_buy;
			err = g_tt_data_set[0][week].wnn_prediction_error_sell_buy;
		}
		

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
		
		N++;
	}
	average_wnn_prediction_error /= (double) N;
	average_target_return /= (double) N;

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

	for (week = SAMPLES2CALIBRATE; week < last_sample; week++)
	{
		if (g_tt_data_set[0][week].trade_order == BUY_SELL)
		{
			tgt = g_tt_data_set[0][week].target_return_buy_sell;
			err = g_tt_data_set[0][week].wnn_prediction_error_buy_sell;
		}
		else
		{
			tgt = g_tt_data_set[0][week].target_return_sell_buy;
			err = g_tt_data_set[0][week].wnn_prediction_error_sell_buy;
		}
		
		standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        (err - average_wnn_prediction_error);
		standard_deviation_target_return += (tgt - average_target_return) *
					            (tgt - average_target_return);
	}
	standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (double) (N-1));
	standard_deviation_target_return = sqrt(standard_deviation_target_return / (double) (N-1));
	
	printf("# **** average_target_return: % .6f, average_wnn_error: % .6f\n",
		       average_target_return,
		       average_wnn_prediction_error);

	printf("# **** standard_deviation_target_return: % .6f, standard_deviation_wnn_error: % .6f\n",
		       standard_deviation_target_return,
		       standard_deviation_wnn_error);

	printf("# **** hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n", 100.0 * hr_up, 100.0 * hr_dn, 100.0 * hr);

	printf("# **** capital inicial: R$%.2lf  capital final: R$%.2lf  retorno acumulado: %.2f%%\n", INITIAL_CAPITAL, capital, 100.0 * (capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
}



void
get_current_performance_buy_sell(int week, double *performance)
{
	int a;
	
	performance[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		performance[a] = g_tt_data_set[a][week].wnn_predicted_return_buy_sell;
	}
}



void
get_current_performance_sell_buy(int week, double *performance)
{
	int a;
	
	performance[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		performance[a] = g_tt_data_set[a][week].wnn_predicted_return_sell_buy;
	}
}



void
print_vetor(double *vetor, int size)
{
	int a;
	
	for (a = 1; a < size; a++)
	{
		printf("% .2f ", vetor[a]);
	}
	printf("\n");
}



void
normalize_vector(double *vector_out, double *vector_in, int size)
{
	int a;
	double max = -1000.0;
	double min = 1000.0;
	
	for (a = 1; a < size; a++)
	{
		if (vector_in[a] > max)
			max = vector_in[a];
		if (vector_in[a] < min)
			min = vector_in[a];
	}
	
	for (a = 1; a < size; a++)
	{
		if ((max - min) == 0.0)
			vector_out[a] = 0.0000001;
		else
			vector_out[a] = 0.0000001 + (vector_in[a] - min) / (max - min);
	}
}



void
summarize_performance_buy_sell(int week, double *performance)
{
	int N, i, a;
	int last_week;
	
	double average_wnn_prediction_error = 0.0;
	double standard_deviation_wnn_error = 0.0;
	double average_target_return = 0.0;
	double standard_deviation_target_return = 0.0;
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
	
	last_week = (g_no_extra_predicted_sample) ? week : week - 1;
	performance[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		performance[a] = 0.0;
		N = 0;
		for (i = week-SAMPLES2CALIBRATE; i < last_week; i++)
		{
			tgt = g_tt_data_set[a][i].target_return_buy_sell;
			out = g_tt_data_set[a][i].wnn_predicted_return_buy_sell;
			err = g_tt_data_set[a][i].wnn_prediction_error_buy_sell;

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
			
			N++;
		}
		average_wnn_prediction_error /= (double) N;
		average_target_return /= (double) N;

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

		for (i = week-SAMPLES2CALIBRATE; i < last_week; i++)
		{
			tgt = g_tt_data_set[a][i].target_return_buy_sell;
			err = g_tt_data_set[a][i].wnn_prediction_error_buy_sell;

			standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        	(err - average_wnn_prediction_error);
			standard_deviation_target_return += (tgt - average_target_return) *
					        	    (tgt - average_target_return);
		}
		standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (double) (N-1));
		standard_deviation_target_return = sqrt(standard_deviation_target_return / (double) (N-1));

		performance[a] = hr;
		//performance[a] = standard_deviation_wnn_error;
	}
}



void
summarize_performance_sell_buy(int week, double *performance)
{
	int N, i, a;
	int last_week;
	
	double average_wnn_prediction_error = 0.0;
	double standard_deviation_wnn_error = 0.0;
	double average_target_return = 0.0;
	double standard_deviation_target_return = 0.0;
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
	
	last_week = (g_no_extra_predicted_sample) ? week : week - 1;
	performance[0] = 0.0;
	for (a = 1; a < g_argc; a++)
	{
		performance[a] = 0.0;
		N = 0;
		for (i = week-SAMPLES2CALIBRATE; i < last_week; i++)
		{
			tgt = g_tt_data_set[a][i].target_return_sell_buy;
			out = g_tt_data_set[a][i].wnn_predicted_return_sell_buy;
			err = g_tt_data_set[a][i].wnn_prediction_error_sell_buy;

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
			
			N++;
		}
		average_wnn_prediction_error /= (double) N;
		average_target_return /= (double) N;

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

		for (i = week-SAMPLES2CALIBRATE; i < last_week; i++)
		{
			tgt = g_tt_data_set[a][i].target_return_sell_buy;
			err = g_tt_data_set[a][i].wnn_prediction_error_sell_buy;

			standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        	(err - average_wnn_prediction_error);
			standard_deviation_target_return += (tgt - average_target_return) *
					        	    (tgt - average_target_return);
		}
		standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (double) (N-1));
		standard_deviation_target_return = sqrt(standard_deviation_target_return / (double) (N-1));

		performance[a] = hr;
		//performance[a] = standard_deviation_wnn_error;
	}
}



void
compute_average_performance_buy_sell(double *average_performance, double *performance, double *current_performance, int size)
{
	int a;
	
	for (a = 1; a < size; a++)
	{
//		average_performance[a] = 1.0 / ((1.0 / performance[a]) + (1.0 / current_performance[a]));
		average_performance[a] = current_performance[a];
//		average_performance[a] = (performance[a] + 3.0 * current_performance[a]) / 4.0;
//		average_performance[a] = current_performance[a] - 2.0 * performance[a];
//		average_performance[a] = current_performance[a] / performance[a];
//		if (current_performance[a] < TRADING_SYSTEM_STOP_GAIN)
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * current_performance[a]);
//		else
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * TRADING_SYSTEM_STOP_GAIN);
//		printf("current_performance = %f, performance = %f\n", current_performance[a], performance[a]);
	}
}



void
compute_average_performance_sell_buy(double *average_performance, double *performance, double *current_performance, int size)
{
	int a;
	
	for (a = 1; a < size; a++)
	{
//		average_performance[a] = 1.0 / ((1.0 / performance[a]) + (1.0 / current_performance[a]));
		average_performance[a] = current_performance[a];
//		average_performance[a] = (performance[a] + 3.0 * current_performance[a]) / 4.0;
//		average_performance[a] = current_performance[a] - 2.0 * performance[a];
//		average_performance[a] = current_performance[a] / performance[a];
//		if (current_performance[a] < TRADING_SYSTEM_STOP_GAIN)
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * current_performance[a]);
//		else
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * TRADING_SYSTEM_STOP_GAIN);
//		printf("current_performance = %f, performance = %f\n", current_performance[a], performance[a]);
	}
}



void
print_unordered_ranking(double *average_performance_buy_sell, double *average_performance_sell_buy, int trade_order, int size, int week)
{
	int stock;
	char stock_name_temp[1000];
	char *stock_name;
	char *aux;
	
	printf("### Stock Ranking (unordered) ###\n");
	for (stock = 1; stock < size; stock++)
	{
		strcpy(stock_name_temp, g_argv[g_tt_data_set[stock][week].file_number]);
		stock_name = strrchr(stock_name_temp, '/') + 1;
		aux = strrchr(stock_name, '.');
		aux[0] = '\0';
		if (trade_order == BUY_SELL)
		{
			printf("# %s %f  %.2f%%\n", stock_name, average_performance_buy_sell[stock],
						    100.0 * g_tt_data_set[stock][week].wnn_predicted_return_buy_sell);
		}
		else
		{
			printf("# %s %f  %.2f%%\n", stock_name, average_performance_sell_buy[stock],
						    100.0 * g_tt_data_set[stock][week].wnn_predicted_return_sell_buy);
		}
	}
	
	if (trade_order == BUY_SELL)
		printf("###\n### Soma dos sinais preditos na direcao BUY_SELL = %+d\n", main_signal_buy_sell(week));
	else
		printf("###\n### Soma dos sinais preditos na direcao SELL_BUY = %+d\n", main_signal_sell_buy(week));

	strcpy(stock_name_temp, g_argv[g_tt_data_set[0][week].file_number]);
	stock_name = strrchr(stock_name_temp, '/') + 1;
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';

	printf("### Melhor acao de hoje, %s = %s, order de negociacao = %s, Retorno predito = %.2f%%\n", 
		long2isodate(g_tt_data_set[0][week].date_l),
		stock_name,
		(trade_order == BUY_SELL)? "BUY_SELL": "SELL_BUY",
		100.0 * g_tt_data_set[0][week].wnn_predicted_return_buy_sell);
}


int
compare_performances(const void *p1, const void *p2)
{
	PERFORMANCE *performance1;
	PERFORMANCE *performance2;
	
	performance1 = (PERFORMANCE *) p1;
	performance2 = (PERFORMANCE *) p2;
	
	if (performance1->performance > performance2->performance)
		return (-1);
	else if (performance1->performance < performance2->performance)
		return (1);
	else
		return (0);
}



int
get_stock_of_a_given_rank(int rank, double *performance, int size)
{	// o rank 1 ee o primeiro do ranking (nao usar rank=0)
	PERFORMANCE *stock_performance;
	int i, stock;
	
	stock_performance = (PERFORMANCE *) malloc(size * sizeof(PERFORMANCE));
	for (i = 0; i < size; i++)
	{
		stock_performance[i].performance = performance[i];
		stock_performance[i].stock = i;
	}
	qsort((void *)&(stock_performance[1]), size - 1, sizeof(PERFORMANCE), compare_performances);
	stock = stock_performance[rank].stock;

	free(stock_performance);
	
	return (stock);
}



int
select_stock_and_trade_order(int week, 
			     double *performance_buy_sell, double *performance_sell_buy,
			     double *current_performance_buy_sell, double *current_performance_sell_buy,
			     double *average_performance_buy_sell, double *average_performance_sell_buy, int size)
{
	int best_stock_buy_sell, best_stock_sell_buy;
	int trade_order;
	
	best_stock_buy_sell = get_stock_of_a_given_rank(1, average_performance_buy_sell, size);
	
	best_stock_sell_buy = get_stock_of_a_given_rank(1, average_performance_sell_buy, size);

	if (main_signal_buy_sell(week) > main_signal_sell_buy(week))
	//if (((double) main_signal_buy_sell(week) * current_performance_buy_sell[best_stock_buy_sell]) >= 
	//    ((double) main_signal_sell_buy(week) * current_performance_sell_buy[best_stock_sell_buy]))
	//if (performance_buy_sell[best_stock_buy_sell] * current_performance_buy_sell[best_stock_buy_sell] >
	//    performance_sell_buy[best_stock_sell_buy] * current_performance_sell_buy[best_stock_sell_buy])
	//if (average_performance_buy_sell[best_stock_buy_sell] < average_performance_sell_buy[best_stock_sell_buy])
	//if (current_performance_buy_sell[best_stock_buy_sell] < current_performance_sell_buy[best_stock_sell_buy])
	//if (performance_buy_sell[best_stock_buy_sell] < performance_sell_buy[best_stock_sell_buy])
	//if ((performance_buy_sell[best_stock_buy_sell] + 5.0 * current_performance_buy_sell[best_stock_buy_sell]) <
	//    (performance_sell_buy[best_stock_sell_buy] + 5.0 * current_performance_sell_buy[best_stock_sell_buy]))
	//if (g_tt_data_set[best_stock_buy_sell][week].confidence > g_tt_data_set[best_stock_sell_buy][week].confidence)
	//if (g_tt_data_set[best_stock_buy_sell][week].confidence * current_performance_buy_sell[best_stock_buy_sell] > 
	//    g_tt_data_set[best_stock_sell_buy][week].confidence * current_performance_sell_buy[best_stock_sell_buy])
	//if ((g_tt_data_set[best_stock_buy_sell][week].confidence * 
	//    current_performance_buy_sell[best_stock_buy_sell] * (double) main_signal_buy_sell(week) *
	//    performance_buy_sell[best_stock_buy_sell]) > 
	//    (g_tt_data_set[best_stock_sell_buy][week].confidence * 
	//    current_performance_sell_buy[best_stock_sell_buy] * (double) main_signal_sell_buy(week) *
	//    performance_sell_buy[best_stock_sell_buy]))
	{
		trade_order = BUY_SELL;
		g_tt_data_set[0][week] = g_tt_data_set[best_stock_buy_sell][week];
		g_tt_data_set[0][week].trade_order = trade_order;
		g_tt_data_set[0][week].file_number2 = best_stock_sell_buy;
	}
	else
	{
		trade_order = SELL_BUY;
		g_tt_data_set[0][week] = g_tt_data_set[best_stock_sell_buy][week];
		g_tt_data_set[0][week].trade_order = trade_order;
		g_tt_data_set[0][week].file_number2 = best_stock_buy_sell;
	}
		
	return (trade_order);
}


void
print_performances(char *message, int week, double *performance, int num_stocks)
{
	int a;
	
	printf("#\n# week = %d, %s\n", week, message);
	for (a = 1; a < num_stocks; a++)
		printf("# %s %f\n", g_argv[g_tt_data_set[a][week].file_number], performance[a]);
}


void
select_stock()
{
	int week;
	double *performance_buy_sell;
	double *current_performance_buy_sell;
	double *normalized_performance_buy_sell;
	double *normalized_current_performance_buy_sell;
	double *average_performance_buy_sell;

	double *performance_sell_buy;
	double *current_performance_sell_buy;
	double *normalized_performance_sell_buy;
	double *normalized_current_performance_sell_buy;
	double *average_performance_sell_buy;
	int trade_order;

	performance_buy_sell = (double *) malloc(g_argc * sizeof(double));
	current_performance_buy_sell = (double *) malloc(g_argc * sizeof(double));
	normalized_performance_buy_sell = (double *) malloc(g_argc * sizeof(double));
	normalized_current_performance_buy_sell = (double *) malloc(g_argc * sizeof(double));
	average_performance_buy_sell = (double *) malloc(g_argc * sizeof(double));

	performance_sell_buy = (double *) malloc(g_argc * sizeof(double));
	current_performance_sell_buy = (double *) malloc(g_argc * sizeof(double));
	normalized_performance_sell_buy = (double *) malloc(g_argc * sizeof(double));
	normalized_current_performance_sell_buy = (double *) malloc(g_argc * sizeof(double));
	average_performance_sell_buy = (double *) malloc(g_argc * sizeof(double));
	for (week = SAMPLES2CALIBRATE; week < g_tt_num_samples; week++)
	{
		summarize_performance_buy_sell(week, performance_buy_sell);
		normalize_vector(normalized_performance_buy_sell, performance_buy_sell, g_argc);
		summarize_performance_sell_buy(week, performance_sell_buy);
		normalize_vector(normalized_performance_sell_buy, performance_sell_buy, g_argc);

		get_current_performance_buy_sell(week, current_performance_buy_sell);
		normalize_vector(normalized_current_performance_buy_sell, current_performance_buy_sell, g_argc);
		get_current_performance_sell_buy(week, current_performance_sell_buy);
		normalize_vector(normalized_current_performance_sell_buy, current_performance_sell_buy, g_argc);

		compute_average_performance_buy_sell(average_performance_buy_sell, normalized_performance_buy_sell, normalized_current_performance_buy_sell, g_argc);
		compute_average_performance_sell_buy(average_performance_sell_buy, normalized_performance_sell_buy, normalized_current_performance_sell_buy, g_argc);
		trade_order = select_stock_and_trade_order(week, 
							     performance_buy_sell, performance_sell_buy,
							     current_performance_buy_sell, current_performance_sell_buy,
							     average_performance_buy_sell, average_performance_sell_buy, g_argc);
		
		// trade(week);
	}
	week--;
	print_unordered_ranking(average_performance_buy_sell, average_performance_sell_buy, trade_order, g_argc, week);

	free(average_performance_buy_sell);
	free(current_performance_buy_sell);
	free(performance_buy_sell);
	free(normalized_current_performance_buy_sell);
	free(normalized_performance_buy_sell);

	free(average_performance_sell_buy);
	free(current_performance_sell_buy);
	free(performance_sell_buy);
	free(normalized_current_performance_sell_buy);
	free(normalized_performance_sell_buy);
}



#ifdef	USE_IPC

void
send_quote(double quote, int target_day_index, int trading_system_current_week, int moving_average_current_minute)
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



double
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
fill_in_eventual_missing_quotes(int message_minute, int trading_system_minute, int target_day_index, double best_stock_quote)
{
	double fill_in_price;
	int i;
	
	if ((message_minute - trading_system_minute) == 1) // no missing quotes
		return (message_minute);
		
	if (trading_system_minute == -1) // first minutes not available: fill in with best stock current quote
		fill_in_price = best_stock_quote;
	else
		fill_in_price = g_stock_price[trading_system_minute];
	
	for (i = trading_system_minute + 1; i < message_minute; i++)
		g_stock_price[i] = fill_in_price;
		
	return (message_minute);
}



int
receive_quote(qsor_quotes_message* received_quotes)
{
	char *best_stock_name;
	double best_stock_quote;
	int message_minute;
	
	best_stock_name = get_best_stock_name(g_trading_system_current_week);
	best_stock_quote = get_best_stock_quote_from_quotes_message(best_stock_name, received_quotes);
	if (best_stock_quote == -1.0) // A cotacao da acao nao veio com o quotes message. Inicio do pregao?
		return (0);
	
	message_minute = minutes_within_this_week(received_quotes->date_and_time, g_trading_system_current_week);
	
	g_moving_average_current_minute = fill_in_eventual_missing_quotes(message_minute, g_moving_average_current_minute, g_target_day_index, best_stock_quote);
	
	if (g_moving_average_current_minute <= MINUTES_IN_A_WEEK)
		g_stock_price[g_moving_average_current_minute] = best_stock_quote;
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
	double *intraday_price;
	double last_price;
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
			last_price = g_stock_price[new_moving_average_current_minute - 1];
		for (i = 0; i < MINUTES_IN_A_DAY; i++)
		{
			g_stock_price[new_moving_average_current_minute] = last_price;
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
		last_price = g_stock_price[new_moving_average_current_minute - 1];
		send_quote(last_price, target_day_index, g_trading_system_current_week, new_moving_average_current_minute);
	}
#endif
}

#else


void
get_best_stock_quote(int target_day_index)
{
	int current_day, i;
	double *intraday_price;
	double *intraday_ref_price;
	double last_price, last_ref_price;
	int new_moving_average_current_minute;
	
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
	
	current_day = get_current_day(target_day_index, new_moving_average_current_minute);
	intraday_price = g_train_data_set[current_day].intraday_price;
	intraday_ref_price = g_ref_data_set[current_day].intraday_price;
	while ((intraday_price == NULL) && (new_moving_average_current_minute < MINUTES_IN_A_WEEK))
	{	// Tem um dia ou mais de dados faltando -> repete a ultima cotacao
		if (new_moving_average_current_minute == 0) // O primeiro dia e negocios (quinta) esta faltando). Pega o fechamento da quarta anterior
		{
			last_price = g_train_data_set[target_day_index - DAYS_IN_A_WEEK].clo;
			last_ref_price = g_ref_data_set[target_day_index - DAYS_IN_A_WEEK].clo;
		}
		else
		{
			last_price = g_stock_price[new_moving_average_current_minute - 1];
			last_ref_price = g_ref_price[new_moving_average_current_minute - 1];
		}
		for (i = 0; i < MINUTES_IN_A_DAY; i++)
		{
			g_stock_price[new_moving_average_current_minute] = last_price;
			g_ref_price[new_moving_average_current_minute] = last_ref_price;
			new_moving_average_current_minute++;
		}
		current_day = get_current_day(target_day_index, new_moving_average_current_minute);
		intraday_price = g_train_data_set[current_day].intraday_price;
		intraday_ref_price = g_ref_data_set[current_day].intraday_price;
	}
	if (new_moving_average_current_minute < MINUTES_IN_A_WEEK)
	{
		g_stock_price[new_moving_average_current_minute] = intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
		g_ref_price[new_moving_average_current_minute] = intraday_ref_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
	}
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
	free_data_set(g_train_data_set);
	free_data_set(g_ref_data_set);
	
	return (0);
}

#endif
