#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../inflection_point.h"
#include "inflection_point_user_functions.h"
#include "pwb_analyse_series.h"



#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

#define TRAINING_PHASE		1
#define RECALL_PHASE		2



// Variaveis globais
int LAST_SAMPLE 	= 1;
DIR *g_data_set_dir = NULL;
DATA_SET *g_data_set = NULL;
int g_data_set_num_samples = 0;
int g_current_sample = 0;
int g_nStatus;
char g_current_data_set_file_name[1000];
int g_num_max = 0, g_num_max_train = 0;
int g_num_min = 0, g_num_min_train = 0;
int g_num_neutral = 0, g_num_neutral_train = 0;
char g_max_file_name[50];
char g_predicted_file_name[50];
int g_tp[3], g_fn[3], g_fp[3], g_tn[3];
int g_day_inflection_point=0, g_day_wnn_predicted=0;
int g_sample_train = 0;
int g_phase = 0;
float g_money = 40000.00;
int g_last_prediction = 0;
float g_amount_stocks = 0;
float g_current_price = 50;

FILE *g_file_train;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_first_sample_tested = 0;


/*
***********************************************************
* Function: GetNextSample
* Description:
* Inputs: nDirection
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

void 
GetNextSample(int nDirection)
{
	int test = 0;

	if (nDirection != NO_DIRECTION)
	{
		test = (nDirection == DIRECTION_FORWARD)? 1:0;

		//printf("\n\ntest = %i\n",test );

		if ((g_current_sample + WINDOWS_OFFSET) >= (LAST_SAMPLE/WINDOWS_OFFSET)*WINDOWS_OFFSET)
		{
			if (test)
				g_current_sample = 0;
			else
			{
				//g_current_sample -= WINDOWS_OFFSET;
				g_current_sample--;
				//while ((g_current_sample>0) && (g_data_set[g_current_sample].train==0 || g_data_set[g_current_sample].test==0))
				while ((g_current_sample>0) && (g_data_set[g_current_sample].test==0))
					g_current_sample--;
			}
		} 
		else if (g_current_sample == 0)
		{
			if (test)
			{
			
				//g_current_sample += WINDOWS_OFFSET;  
				g_current_sample++;
				//while ((g_current_sample>0) && (g_data_set[g_current_sample].train==0 || g_data_set[g_current_sample].test==0))
				while ((g_current_sample<g_data_set_num_samples) && (g_data_set[g_current_sample].test==0))
					g_current_sample++;
			}
			else
				g_current_sample = (LAST_SAMPLE/WINDOWS_OFFSET)*WINDOWS_OFFSET - WINDOWS_OFFSET;
		}
		else
		{
			if (test)
			{
				//g_current_sample += WINDOWS_OFFSET;  
				g_current_sample++;
				//while ((g_current_sample>0) && (g_data_set[g_current_sample].train==0 || g_data_set[g_current_sample].test==0))
				while ((g_current_sample<g_data_set_num_samples)&&(g_data_set[g_current_sample].test==0))
					g_current_sample++;
			}
			else
			{
				//g_current_sample -= WINDOWS_OFFSET;
				g_current_sample--;
				//while ((g_current_sample>0)&&(g_data_set[g_current_sample].train==0 || g_data_set[g_current_sample].test==0))
				while ((g_current_sample>0)&&(g_data_set[g_current_sample].test==0))
					g_current_sample--;
			}
		}
	}
	
	//printf("g_current_sample = %i minute = %s \n", g_current_sample, g_data_set[g_current_sample].minute);

	//printf("g_current_sample= %i .train= %i teste= %i predictor= %i specialist= %i\n",g_current_sample,g_data_set[g_current_sample].train, g_data_set[g_current_sample].test, g_data_set[g_current_sample+2].wnn_predicted_max_min, g_data_set[g_current_sample+2].max_min);

	
}



FILE *
get_file (char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	
	strcpy (file_name, DATA_PATH);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open file: ", file_name, "");
		return (NULL);
	}
	return (data_set_file);
}



int
count_samples (FILE *data_set_file, char *base_file_name)
{
	int num_samples;
	char data_set_file_last_line[1000];
	char aux[1000];
	
	num_samples = 0;
	while (fgets (aux, 900, data_set_file) != NULL)
	{
		num_samples++;
		strcpy (data_set_file_last_line, aux);
	}
	
	rewind (data_set_file);

	return (num_samples);
}


int
select_max_min_points_to_train_old (){	
	int num_max=0, num_min=0, num_samples, pos, last_sample_train_set;
	
	if (g_num_max < g_num_min){
		num_samples = ((2 * g_num_max) / 3);
	}
	else{
		num_samples = ((2 * g_num_min) / 3);
	}
	
	
	pos=0;
	while ((num_max < num_samples) || (num_min < num_samples)){
		if ((g_data_set[pos].max_min==1)&&(num_min<num_samples)){
			g_data_set[pos].train=1;
			num_min++;
		}
		else if ((g_data_set[pos].max_min==2)&&(num_max<num_samples)){
			g_data_set[pos].train=1;
			num_max++;
		}
		pos++;	
	}
	last_sample_train_set=pos;
	
	srand(time(NULL));
	num_samples = 2 * num_samples;
	while(num_samples > 0){
		pos = rand() % last_sample_train_set;
		
		if (g_data_set[pos].max_min == 0 && g_data_set[pos].train != 1){
			g_data_set[pos].train = 1;
			num_samples --;
		}
	}
	
	
	
	return(last_sample_train_set);
}

void init_contingency_table ()
{	
	int i;

	for(i=0;i<3;i++){
		g_tp[i] = 0;	
		g_fn[i] = 0;	
		g_tn[i] = 0;	
		g_fp[i] = 0;
	}
}

int 
create_file_to_train (int num_samples_train)
{

	int i, qtd_train = 0;
	FILE *fp;
	
	fp = fopen("samples_train.txt", "w");
	for (i = 0 ; i < num_samples_train; i++)
	{
		if (g_data_set[i].train == 1)
		{
			fprintf(fp, "%d\n", (i + POINTS_BEFORE));
			qtd_train++;
		}
	}	
	//printf("qtd_train = %d\n",qtd_train);
	fclose (fp);
	return (qtd_train);
}

int
select_max_min_points_to_train ()
{
	int num_max = 0, num_min = 0, num_samples_train, num_samples_neutral, pos, qtd_train;
	
	num_samples_train = g_data_set_num_samples * FRACTION_TRAIN_SET; 
	
	pos = 0;
	while (pos < num_samples_train)
	{
		if (g_data_set[pos].max_min == 1)
		{
			g_data_set[pos - POINTS_BEFORE].train = 1;
			num_min++;
		}
		else if (g_data_set[pos].max_min == 2)
		{
			g_data_set[pos - POINTS_BEFORE].train = 1;
			num_max++;
		}
		pos++;	
	}
	
	
	if (num_max > num_min)
	{
		num_samples_neutral = 2 * num_max;
	}
	else
	{
		num_samples_neutral = 2 * num_min;
	}
	//printf("num_samples_neutral = %d\n",num_samples_neutral);
	srand(time(NULL));
	while(num_samples_neutral > 0)
	{
		pos = rand() % num_samples_train;
		
		if (g_data_set[pos].max_min == 0 && g_data_set[pos].train != 1 && pos > 2)
		{
			g_data_set[pos - POINTS_BEFORE].train = 1;
			num_samples_neutral --;
		}
	}
	qtd_train = create_file_to_train (num_samples_train);
	
	//printf ("qtd_train = %d\nnum_samples_train = %d\nnum_min = %d\nnum_max = %d\nnum_samples_neutral = %d\n",qtd_train,num_samples_train,num_min,num_max,num_samples_neutral);
	
	return(qtd_train);
}

/*
***********************************************************
* Function:
* Description: 
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetPositionLastSamplesTrain (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int last_sample_train;
	last_sample_train = g_data_set_num_samples * FRACTION_TRAIN_SET;
	output.ival = last_sample_train;
	return (output);
}

/*
***********************************************************
* Function:
* Description: 
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/


NEURON_OUTPUT GetNumSamplesTrain (PARAM_LIST *pParamList)
{
	int i=0,num;
	NEURON_OUTPUT output;
	FILE *file_train;
	file_train = fopen("samples_train.txt", "r");
	while (!feof(file_train))
	{
	 	fscanf (file_train,"%d\n",&num);
		i++;
	}
	fclose (file_train);
	output.ival = i;
	return (output);
}


NEURON_OUTPUT GetNumSamplesTrainToCalibration (PARAM_LIST *pParamList)
{
	int samples_train;
	NEURON_OUTPUT output;
	samples_train = pParamList->next->param.ival * FRACTION_TRAIN_SET;
	//printf("samples_train = %d\n",samples_train);
	output.ival = samples_train;
	return (output);
}

NEURON_OUTPUT GetSampleCurrent (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	output.ival = g_current_sample;
	return (output);
}
int
compute_return (int min, int max, float cost_buy)
{
	float retorno;
	retorno = ((g_money / g_data_set[min].sample_price) * (g_data_set[max].sample_price)) - g_money;
	retorno = retorno - cost_buy;
	if (retorno > (g_money * 0.005))
	{
		printf ("min = %i max = %i price_min = %f price_max = %f retorno = %0.3f\n", min, max, g_data_set[min].sample_price, g_data_set[max].sample_price, retorno);
		return (1);
	}
	else
		return (0);		
}
void
select_max_min_point ()
{
	int i, j, k, window; 
	float cost_buy;
	cost_buy = 5.0; // Custo de compra (corretagem) MyCAP
	cost_buy += cost_buy * (2.0 / 100.0);// ISS sobre a corretagem
	cost_buy += g_money * (0.019 / 100.0); // Emolumentos Bovespa
	cost_buy += g_money * (0.006 / 100.0); // Liquidacao
	window = 120;
	
	printf ("cost_buy = %0.3f\n", cost_buy);
	for (i = 0; i < g_data_set_num_samples; i++)
	{
		if (g_data_set[i].sample_price > g_data_set[i+1].sample_price)
		{
			k=i+1;
			while ( k < g_data_set_num_samples && (g_data_set[k].sample_price >= g_data_set[k+1].sample_price))
				k++;
			if ((k + window) < g_data_set_num_samples){
				j=k;
				while (g_data_set[j].sample_price <= g_data_set[j+1].sample_price && j < (k + window))
					j++;
				if (j < (k + window))
				{
					if (compute_return(k, j, cost_buy))
					{
						g_data_set[k].max_min = 1;
						g_data_set[j].max_min = 2;
				
					}
					else
					{
						for (j = j+1; j<(k + window); j++)
						{
							if (g_data_set[j].sample_price < g_data_set[j+1].sample_price)
							{
								while (g_data_set[j].sample_price <= g_data_set[j+1].sample_price && j < (k + window))
									j++;
								if (j < (k + window))
								{
									
									if (compute_return(k, j, cost_buy))
									{
										if (g_data_set[j].max_min != 2)
										{
											g_data_set[k].max_min = 1;
											g_data_set[j].max_min = 2;
										}
									
										break;
									}
								}	
							}
						}
					}
				}
			}
		}
	}


}
void
read_points_data (char *base_file_name)
{
	int i, indice;
	FILE *max_file, *file_train, *file_test;
	char *minute;
	
	minute = (char *) alloc_mem (11 * sizeof (char));
	
	strcpy (g_max_file_name,MAX_PATH);
	strcat (g_max_file_name,base_file_name);
	strcat (g_max_file_name,"_MAX_MIN.txt");
	
	for (i =0 ; i < g_data_set_num_samples; i++)
	{
		g_data_set[i].max_min = 0;
	}		
	if((max_file = fopen(g_max_file_name, "r")))
	{
		for(i = 0; i < g_data_set_num_samples; i++){
			fscanf(max_file,"%s %d\n",minute,&g_data_set[i].max_min);
			if (g_data_set[i].max_min == 1){
				g_num_min++;
			}
			else if (g_data_set[i].max_min == 2){
				g_num_max++;
			}
			else{
				g_num_neutral++;
			}
			g_data_set[i].train = 0;
		}
	}
	else{
		max_file = fopen(g_max_file_name, "w");
		select_max_min_point ();
		for (i = 0; i < g_data_set_num_samples; i++){
			if (g_data_set[i].max_min != 1 && g_data_set[i].max_min != 2)
				fprintf(max_file,"%s 0\n",g_data_set[i].minute);
			else
				fprintf(max_file,"%s %i\n",g_data_set[i].minute,g_data_set[i].max_min);	
		}
	}
	
	if(!(file_train = fopen("samples_train.txt", "r"))){
		select_max_min_points_to_train ();

		}
	else{
		while (!feof(file_train))
		{
			fscanf(file_train,"%i\n",&indice);
			g_data_set[indice - POINTS_BEFORE].train = 1;
		}
		fclose(file_train);
	}
	
	if((file_test = fopen("samples_test.txt", "r")))
	{
		while (!feof(file_test))
		{
			fscanf(file_test,"%i\n",&indice);
			g_data_set[indice - POINTS_BEFORE].test = 1;
		}
		fclose(file_test);
	}
	fclose (max_file);
}

void
read_predicted_points_data (char *base_file_name)
{
	int i;
	FILE *predicted_file;
	char *minute;
	
	minute = (char *) alloc_mem (11 * sizeof (char));
	
	strcpy (g_predicted_file_name,PREDICTED_PATH);
	strcat (g_predicted_file_name,base_file_name);
	strcat (g_predicted_file_name,"_PREDICTED.txt");
	
	for (i =0 ; i < g_data_set_num_samples; i++)
	{
		g_data_set[i].wnn_predicted_max_min = 0; //para o teste de sanidade colocar 0, caso contrario 3
	}		
	if((predicted_file = fopen(g_predicted_file_name, "r")))
	{
		for(i = 0; i < g_data_set_num_samples; i++){
			fscanf(predicted_file,"%s %d\n",minute,&g_data_set[i].wnn_predicted_max_min);
		}
		fclose (predicted_file);
	}
}



int
read_train_data_set_data (char *base_file_name)
{
	int i;
	FILE *returns_file;
	char data_set_file_line[1000];
	
	returns_file = get_file (base_file_name, ".txt");
	g_data_set_num_samples = count_samples (returns_file, base_file_name);

	if (g_data_set != NULL)
		free (g_data_set);
	g_data_set = (DATA_SET *) alloc_mem (g_data_set_num_samples * sizeof (DATA_SET));
	
	for (i = 0; i < g_data_set_num_samples; i++)
	{
		if (fgets (data_set_file_line, 900, returns_file) == NULL)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
		if (sscanf (data_set_file_line, "%s %f %f", 
						 g_data_set[i].minute, 
						 &(g_data_set[i].sample_price),
						 &(g_data_set[i].sample_volume)) != 3)
		{
			Erro ("Could not read line of the dataset: ", base_file_name, ".txt");
			printf ("line no.: %d\n", i);
			exit (1);
		}
	}

	g_data_set_num_samples = i;
	
	fclose (returns_file);
	
	read_points_data(base_file_name);
	
	read_predicted_points_data(base_file_name);
	
	return (g_data_set_num_samples);
}

char *
get_next_train_data_set ()
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;
	
	do
	{
		train_data_set_file_entry = readdir (g_data_set_dir);
		if (train_data_set_file_entry == NULL)
			return (NULL);
		aux = strrchr (train_data_set_file_entry->d_name, '.');
	} while (strcmp (aux, ".txt") != 0);
	
	strcpy (base_file_name, train_data_set_file_entry->d_name);
	
	aux = strrchr (base_file_name, '.');
	aux[0] = '\0';
	
	LAST_SAMPLE = num_samples = read_train_data_set_data (base_file_name);
	printf ("# Data set: %s, num_samples: %d\n", train_data_set_file_entry->d_name, num_samples);

	return (train_data_set_file_entry->d_name);
}



void
compute_samples_returns(int now, int reference)//
{
	int sample;
	double sample_return;
	if (reference < 0) // o primeiro grupo de amostras fica comprometido...
		reference = 0;
#ifdef	VERBOSE
	printf ("ref %04d(%s) - now ", reference, g_data_set[reference].minute);
#endif
	for (sample = now; sample > reference; sample--)
	{
		sample_return = (g_data_set[sample].sample_price - g_data_set[reference].sample_price) / g_data_set[reference].sample_price;
		if (fabs(sample_return) < 0.0000009)
			sample_return = 0.0;
		
		g_data_set[sample].sample_return = sample_return;
#ifdef	VERBOSE
		printf ("%04d(%s)  ", sample, g_data_set[sample].minute);
#endif
	}
#ifdef	VERBOSE
	printf ("\n");
#endif

}



void
compute_sample_group_return_old(int now, int last_sample_in_the_next_sample_group)
{
	int sample, next_sample;
	double next_return;
	
	if (last_sample_in_the_next_sample_group < (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE)
	{
		// Nao ee o ultimo grupo de amostras
		next_sample = now + SAMPLE_GROUP_SIZE; 
		next_return = (g_data_set[next_sample].sample_price - g_data_set[now].sample_price) / g_data_set[now].sample_price;
		if (fabs(next_return) < 0.0000009)
			next_return = 0.0;
		g_data_set[now].sample_group_return = next_return;
		
		for (sample = now + 1; sample <= last_sample_in_the_next_sample_group; sample++)
		{
			next_return = (g_data_set[sample].sample_price - g_data_set[now].sample_price) / g_data_set[now].sample_price;
			if (next_return > STOP_GAIN)
			{
				g_data_set[now].sample_group_return = STOP_GAIN;
				break;
			}
			else if (next_return < STOP_LOSS)
			{
				g_data_set[now].sample_group_return = STOP_LOSS;
				break;
			}
		}
	}
	else	// Se for o ultimo grupo de amostras, nao tem sample_group_return...
		g_data_set[now].sample_group_return = 0.0;
}



void
compute_sample_group_return(int now, int last_sample_in_the_next_sample_group)//
{
	int next_sample;
	double next_return;
	
	if (last_sample_in_the_next_sample_group < (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE)
	{	// Nao ee o ultimo grupo de amostras
		next_sample = now + SAMPLE_GROUP_SIZE; 
		next_return = (g_data_set[next_sample].sample_price - g_data_set[now].sample_price) / g_data_set[now].sample_price;
		if (fabs(next_return) < 0.0000009)
			next_return = 0.0;

		g_data_set[now].sample_group_return = next_return;
	}
	else	// Se for o ultimo grupo de amostras, nao tem sample_group_return...
		g_data_set[now].sample_group_return = 0.0;
}


void
compute_returns_from_prices()
{
	int sample_group;
	int now, last_sample_in_the_next_sample_group;
	int reference;
	int N;
	
	
	N = LAST_SAMPLE/SAMPLE_GROUP_SIZE;

	for (sample_group = 0; sample_group < N; sample_group++)
	{
#ifdef	VERBOSE
		printf ("sample g. %04d(%s) = ", sample_group * SAMPLE_GROUP_SIZE, g_data_set[sample_group * SAMPLE_GROUP_SIZE].minute);
#endif
		now = LAST_SAMPLE_IN_THE_GROUP(sample_group * SAMPLE_GROUP_SIZE);
		reference = now - SAMPLE_GROUP_SIZE;
		
		compute_samples_returns(now, reference);

		last_sample_in_the_next_sample_group = LAST_SAMPLE_IN_THE_GROUP((sample_group + 1) * SAMPLE_GROUP_SIZE);
		compute_sample_group_return(now, last_sample_in_the_next_sample_group);
		now++;
	}
}

void
write_error_file(char *message, float current_return, int i, int neuron_vector_i_output, 
		 int last_sample_in_the_group_neuron_output_number, int current_sample, 
		 int last_sample_in_the_group_current_sample, int last_sample)
{
	FILE *error_file;
	char error_file_name[1000];
	
	strcpy(error_file_name, "ERROR_DIR/");
	strcat(error_file_name, g_current_data_set_file_name);
	error_file = fopen(error_file_name, "a");
	fprintf(error_file, "%s - current_return = %f, i = %d, neuron_vector_i_output = %d\n", message, current_return, i, neuron_vector_i_output);
	fprintf(error_file, "     last_sample_in_the_group_neuron_output_number = %d, current_sample = %d\n", last_sample_in_the_group_neuron_output_number, current_sample);
	fprintf(error_file, "     last_sample_in_the_group_current_sample = %d, last_sample = %d\n", last_sample_in_the_group_current_sample, last_sample);
	fclose(error_file);
}


void
check_error(float current_return, int i, int neuron_vector_i_output, int last_sample_in_the_group_neuron_output_number,
	    int current_sample, int last_sample_in_the_group_current_sample, int last_sample)
{
	if (fabs(current_return) > 1.0)
		write_error_file("modulo current_return > 1.0", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (neuron_vector_i_output >= last_sample)
		write_error_file("neuron_vector_i_output >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (last_sample_in_the_group_neuron_output_number >= last_sample)
		write_error_file("last_sample_in_the_group_neuron_output_number >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (current_sample >= last_sample)
		write_error_file("current_sample >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
	if (last_sample_in_the_group_current_sample >= last_sample)
		write_error_file("last_sample_in_the_group_current_sample >= last_sample", current_return, i, neuron_vector_i_output, 
				 last_sample_in_the_group_neuron_output_number, current_sample, 
				 last_sample_in_the_group_current_sample, last_sample);
}



float
get_sample_group_return(current_sample, now)
{
	float buy_price, current_return;
	int i, last_sample_evaluated;
	
	buy_price = g_data_set[now].sample_price;
	last_sample_evaluated = LAST_SAMPLE_IN_THE_GROUP(current_sample+SAMPLE_GROUP_SIZE);
	for (i = now + 1; i <= last_sample_evaluated; i++)
	{
		current_return = (g_data_set[i].sample_price - buy_price) / buy_price;
		if (current_return <= START_BUY)
		{
			buy_price = g_data_set[i].sample_price;
			break;
		}
	}
	if (i > last_sample_evaluated)
		return (0.0); // não conseguiu comprar no intervalo
	
	for (  ; i <= last_sample_evaluated; i++)
	{
		current_return = (g_data_set[i].sample_price - buy_price) / buy_price;
		if (current_return >= STOP_GAIN)
			return (current_return);
		else if (current_return <= STOP_LOSS)
			return (current_return);
	}
	return ((g_data_set[last_sample_evaluated].sample_price - buy_price) / buy_price);
}

void
contingency_table(int class_id, int wnn_predicted_max_min)
{

		if(g_data_set[g_current_sample + POINTS_BEFORE].max_min == class_id)
		{
			if(wnn_predicted_max_min == class_id)
			{
				g_tp[class_id]++;
			}
			if(wnn_predicted_max_min != class_id)
			{
				g_fn[class_id]++;
			}
		}
		if(g_data_set[g_current_sample + POINTS_BEFORE].max_min != class_id)
		{
			if(wnn_predicted_max_min == class_id)
			{
				g_fp[class_id]++;
			}
			if(wnn_predicted_max_min != class_id)
			{
				g_tn[class_id]++;
			}
		}
}

void
buy ()
{
	float cost_buy;
	cost_buy = 5.0; // Custo de compra (corretagem) MyCAP
	cost_buy += cost_buy * (2.0 / 100.0);// ISS sobre a corretagem
	cost_buy += g_money * (0.019 / 100.0); // Emolumentos Bovespa
	cost_buy += g_money * (0.006 / 100.0); // Liquidacao
	g_money = g_money - cost_buy;
	g_amount_stocks = (g_money / g_data_set[g_current_sample + SAMPLE_GROUP_SIZE].sample_price);
	g_money = 0;
	g_current_price = g_data_set[g_current_sample + SAMPLE_GROUP_SIZE].sample_price;
	printf ("->buy amount_stocks = %.2f current+SAMPLE_GROUP = %i\n",g_amount_stocks,(g_current_sample + SAMPLE_GROUP_SIZE));
}

void
sell ()
{
	float cost_sell;
	g_money = (g_amount_stocks * g_data_set[g_current_sample + SAMPLE_GROUP_SIZE].sample_price);
	g_amount_stocks = 0;
	cost_sell = 5.0; // Custo de venda (corretagem) MyCAP
	cost_sell += cost_sell * (2.0 / 100.0);// ISS sobre a corretagem
	cost_sell += g_money * (0.019 / 100.0); // Emolumentos Bovespa
	cost_sell += g_money * (0.006 / 100.0); // Liquidacao
	g_money = g_money - cost_sell;
	//g_current_price = g_data_set[g_current_sample + SAMPLE_GROUP_SIZE].sample_price;
	printf("->sell money = %.2f current+SAMPLE_GROUP = %i\n",g_money,(g_current_sample + SAMPLE_GROUP_SIZE));
}


void
buy_or_sell (int wnn_predicted_max_min)
{
	FILE *buy_sell_file;
	buy_sell_file = fopen("buy_sell.txt","a");
	if (wnn_predicted_max_min == 1 && g_money > 0 ){
		printf("predictor= %i current_sample= %i\n", wnn_predicted_max_min, g_current_sample);
		buy ();
		fprintf(buy_sell_file,"%d\n",g_current_sample);
	}
	else if (wnn_predicted_max_min == 2 && g_amount_stocks > 0 && g_current_price <= g_data_set[g_current_sample + SAMPLE_GROUP_SIZE].sample_price){
		printf("predictor= %i current_sample= %i\n", wnn_predicted_max_min, g_current_sample);
		sell ();
		fprintf(buy_sell_file,"%d\n",g_current_sample);
	}
	fclose (buy_sell_file);
}

void
base_output_handler_contingency_table (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))//
{
	int wnn_predicted_max_min;
	int num_neurons;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_max_min = (*EvaluateOutput) (output, &num_neurons);
		printf("wnn_predicted_max_min = %i sample_price = %0.2f current_price = %0.2f\n",wnn_predicted_max_min,g_data_set[g_current_sample+POINTS_BEFORE].sample_price,g_current_price);
		
		g_data_set[g_current_sample+POINTS_BEFORE].wnn_predicted_max_min = wnn_predicted_max_min;
		g_data_set[g_current_sample].test = 1;
		
		buy_or_sell (wnn_predicted_max_min);
		
		//Tabela de contingencia para classe neutro
		contingency_table(0, wnn_predicted_max_min);
		//Tabela de contingencia para classe minimo
		contingency_table(1, wnn_predicted_max_min);
		//Tabela de contingencia para classe maximo
		contingency_table(2, wnn_predicted_max_min);						
		
	}
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}


void
base_output_handler_average_stop_gain_stop_loss (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))//
{
	//int wnn_predicted_week;
	int wnn_predicted_return;
	int num_neurons;
	int now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_return = (*EvaluateOutput) (output, &num_neurons);
		
		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);
		g_data_set[now].sample_group_return = get_sample_group_return(g_current_sample, now);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		//Fprintf("wnn_predicted_return = %i\n",wnn_predicted_return);
	}
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
base_output_handler_average (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	//int wnn_predicted_week;
	double wnn_predicted_return;
	int num_neurons;
	int now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		wnn_predicted_return = (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		
		if (now != LAST_SAMPLE_IN_THE_GROUP((LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE))
			printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].sample_group_return,
				 g_data_set[now].wnn_predicted_return,
				 g_data_set[now].wnn_prediction_error);
		else
			printf ("%s ;            ; % .6f ;            ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].wnn_predicted_return);
	}
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
base_output_handler_stop_gain_stop_loss (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	int predicted_sample;
	double wnn_predicted_return;
	int num_neurons;
	int now, predicted_now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		predicted_sample = (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		predicted_now = LAST_SAMPLE_IN_THE_GROUP(predicted_sample);
		wnn_predicted_return = get_sample_group_return(predicted_sample, predicted_now);
#ifdef	VERBOSE
		printf("->  %04d(%s):  ", predicted_now, g_data_set[predicted_now].minute);
#endif
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);
		g_data_set[now].sample_group_return = get_sample_group_return(g_current_sample, now);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		
		if (now != LAST_SAMPLE_IN_THE_GROUP((LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE))
			printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].sample_group_return,
				 g_data_set[now].wnn_predicted_return,
				 g_data_set[now].wnn_prediction_error);
		else
			printf ("%s ;            ; % .6f ;            ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].wnn_predicted_return);
	}

#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
base_output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state, int (*EvaluateOutput) (OUTPUT_DESC *, int *))
{
	int predicted_sample;
	double wnn_predicted_return;
	int num_neurons;
	int now, predicted_now;
	
	if (g_nStatus == RECALL_PHASE)
	{
		// Obtem a saida da rede
		predicted_sample = (*EvaluateOutput) (output, &num_neurons);

		if (g_nTotalTested == 0)
			g_first_sample_tested = g_current_sample;

		g_nTotalTested++;
		
		predicted_now = LAST_SAMPLE_IN_THE_GROUP(predicted_sample);
#ifdef	VERBOSE
		printf("->  %04d(%s):  ", predicted_now, g_data_set[predicted_now].minute);
#endif
		wnn_predicted_return = g_data_set[predicted_now].sample_group_return;
		
		now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);

		g_data_set[now].wnn_predicted_return = wnn_predicted_return;
		g_data_set[now].wnn_prediction_error = g_data_set[now].sample_group_return - wnn_predicted_return;
		
		if (now != LAST_SAMPLE_IN_THE_GROUP((LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE - SAMPLE_GROUP_SIZE))
			printf ("%s ; % .6f  ; % .6f ; % .6f  ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].sample_group_return,
				 g_data_set[now].wnn_predicted_return,
				 g_data_set[now].wnn_prediction_error);
		else
			printf ("%s ;            ; % .6f ;            ;\n",
				 g_data_set[now].minute,
				 g_data_set[now].wnn_predicted_return);
	}

#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	output_display (output);
#endif
}



void
init_data_sets ()
{
	char *first_data_set_name;
	
	if ((g_data_set_dir = opendir (DATA_PATH)) == NULL)
	{
		show_message ("Could not open data set directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_data_set ()) == NULL)
	{
		show_message ("Could not initialize first data set from directory:", DATA_PATH, "");
		exit(1);
	}

}





/*
***********************************************************
* Function: make_input_image_wnn_pred
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void make_input_image_wnn_pred (INPUT_DESC *input, int w, int h)
{
	char message[256];
	int i;

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
	{
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
		for (i = 0; i < input->tfw * input->tfh * 3; i++)
			input->image[i] = 0;
	}
}


void
initialize_data_set(INPUT_DESC *input)
{

	setlocale (LC_ALL, "C");	
	init_data_sets ();
	g_nTotalTested = 0;
	g_nStatus = TRAINING_PHASE;	

	GetNextSample(NO_DIRECTION); 
	ReadSampleInput(input, g_current_sample);
}



/*
***********************************************************
* Function: init_wnn_pred
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_wnn_pred(INPUT_DESC *input)//
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	make_input_image_wnn_pred (input, 4*IN_WIDTH, 4*IN_HEIGHT);	
	
	input->up2date = 0;

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);
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

void init_wnn_prediction(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	
	make_input_image_wnn_pred (input, 2*IF_WIDTH, 2*IF_HEIGHT);	
		
	input->up2date = 0;

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);
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

void init_inflection_points(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	
	make_input_image_wnn_pred (input, 2*IF_WIDTH, 2*IF_HEIGHT);	
		
	input->up2date = 0;

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);
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

int init_user_functions (void)
{
	
	char strCommand[128];
	
	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);
	
	init_contingency_table ();
	
	//g_file_train = fopen ("samples_train.txt","r");
	
	srand(5);
	return (0);
}



/*
***********************************************************
* Function: GetNewSample
* Description:
* Inputs: input -
*	  nDirection -
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

void 
GetNewSample(INPUT_DESC *input, int nDirection)
{

	GetNextSample(nDirection);
	ReadSampleInput(input, g_current_sample);
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

void input_generator (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_wnn_pred(input);
		initialize_data_set(input);
		
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
	}
	else
	{
		
		if (status == MOVE)
		{
			if (input->wxd < 0)
				GetNewSample (input, DIRECTION_REWIND);
			else if (input->wxd >= IN_WIDTH)
				GetNewSample (input, DIRECTION_FORWARD);
			
			all_filters_update ();
			output_update(&sample_gaussian_out);
			output_update(&nl_wnn_pred_out);
#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}
}

/*
***********************************************************
* Function: wnn_prediction_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/
void wnn_predicted_generator (INPUT_DESC *input, int status)
{
	NEURON *neuron;
	int first_column;
	int x, y, x_w, y_h, last_point=-1, j, color=255, k; // color = red
	float max = -1, min = 1000;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_wnn_prediction(input);
#ifdef NO_INTERFACE
		input->win = 2;	
#endif
	}
	else
	{
		first_column = (LAST_SAMPLE_IN_THE_GROUP(g_current_sample) - (SAMPLE_GROUP_SIZE - 1)) % 420;
		for (x = 0; x < IF_WIDTH; x++)
		{
			for (y = 0; y < (IF_HEIGHT+1); y++)
			{
				if ((x >= first_column) && (x < first_column + SAMPLE_GROUP_SIZE))
					neuron[y * IF_WIDTH + x].output.ival = 0xffffff;
				else
					neuron[y * IF_WIDTH + x].output.ival = 0;
			}
		}
			
		for (x = (g_day_wnn_predicted * IF_WIDTH) ; x < (g_day_wnn_predicted  + 1)*IF_WIDTH ; x++)
		{
			if (g_data_set[x].sample_price > max)
				max = g_data_set[x].sample_price;
			if (g_data_set[x].sample_price < min)
				min = g_data_set[x].sample_price;
		}
		for (x = (g_day_wnn_predicted * IF_WIDTH) ; x < (g_day_wnn_predicted + 1)*IF_WIDTH ; x++)
		{
			if(g_data_set[x].wnn_predicted_max_min == 0 ||g_data_set[x].wnn_predicted_max_min == 1 ||g_data_set[x].wnn_predicted_max_min == 2)
			{
				
				y = (float) (IF_HEIGHT) * (g_data_set[x].sample_price - min) / (max - min);
				y = y - g_day_wnn_predicted;
				if (g_data_set[x].wnn_predicted_max_min == 0) // if not a maximum or minumum point
				{
					neuron[y * x_w + x].output.ival = color; // color = red
				}
				else if (g_data_set[x].wnn_predicted_max_min == 1) // if mininum point
				{
					neuron[y * x_w + x].output.ival = color << 16; // color = blue
					for(k = 0- g_day_wnn_predicted; k < IF_HEIGHT; k++)
				{
					neuron[k * x_w + x].output.ival = color << 16;
				}
				}
				else if (g_data_set[x].wnn_predicted_max_min == 2) // if maximum point
				{
					neuron[y * x_w + x].output.ival = color << 8; // color = green
					for(k = 0- g_day_wnn_predicted; k < IF_HEIGHT; k++)
				{
					neuron[k * x_w + x].output.ival = color << 8;
				}
				}
				if(x > 0) // if not first sample
				{
					if(last_point < y){
						for(j = last_point + 1; j < y; j++){
							neuron[j *x_w + x].output.ival = color;
						}
					}
					else{
						for(j = last_point-1; j > y; j--){
							neuron[j *x_w + x].output.ival = color;
						}
					}
				}
				last_point = y;			
			}
		}
		update_input_image(input);
		if (status == MOVE)
		{
#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}
}


void trading_window_paint (NEURON *neuron)
{
	int indice, x, y, i;
	FILE *buy_sell_file;
	if ((buy_sell_file = fopen ("buy_sell.txt","r")))
	{
		while (!feof(buy_sell_file))
		{
 			fscanf (buy_sell_file,"%d\n",&indice);
			if (indice >= (g_day_inflection_point * IF_WIDTH) && indice <=((g_day_inflection_point+1) * IF_WIDTH))
			{
				x = indice % (g_day_inflection_point * IF_WIDTH);
				for (i=0; i<SAMPLE_GROUP_SIZE; i++){
					x +=i;
					for (y=0; y < IF_HEIGHT; y++){
						neuron[y * IF_WIDTH + x].output.ival = 0xffffff;
					}
				}
			}
		}	
	}
}

/*
***********************************************************
* Function: inflection_points_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/
void inflection_points_generator (INPUT_DESC *input, int status)
{
	NEURON *neuron;
	int first_column;
	int x, y, x_w, y_h, k, last_point=-1, j, color=255; // color = red
	float max = -1, min = 1000;
	

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_inflection_points(input);
#ifdef NO_INTERFACE
		input->win = 2;	
#endif
	}
	else
	{
		first_column = (LAST_SAMPLE_IN_THE_GROUP(g_current_sample) - (SAMPLE_GROUP_SIZE - 1)) % 420;
		for (x = 0; x < IF_WIDTH; x++)
		{
			for (y = 0; y < (IF_HEIGHT + 1); y++)
			{
				if ((x >= first_column) && (x < first_column + SAMPLE_GROUP_SIZE))
					neuron[y * IF_WIDTH + x].output.ival = 0xffffff;
				else
					neuron[y * IF_WIDTH + x].output.ival = 0;
			}
		}
		
		for (x = (g_day_inflection_point * IF_WIDTH) ; x < (g_day_inflection_point + 1)*IF_WIDTH ; x++)
		{
			if (g_data_set[x].sample_price > max)
				max = g_data_set[x].sample_price;
			if (g_data_set[x].sample_price < min)
				min = g_data_set[x].sample_price;
		}

		for (x = (g_day_inflection_point * IF_WIDTH) ; x < (g_day_inflection_point + 1)*IF_WIDTH ; x++)
		{
			y = (float) (IF_HEIGHT) * (g_data_set[x].sample_price - min) / (max - min);
			y = y - g_day_inflection_point;
			if (g_data_set[x].max_min==0) // if not a maximum or minumum point
			{
				neuron[y * x_w + x].output.ival = color; // color = red
			}
			else if (g_data_set[x].max_min==1) // if mininum point
			{
				neuron[y * x_w + x].output.ival = color << 16; // color = blue
				for(k = 0 - g_day_inflection_point; k < IF_HEIGHT; k++)
				{
					neuron[k * x_w + x].output.ival = color << 16;
				}
			}
			else if (g_data_set[x].max_min==2) // if maximum point
			{
				neuron[y * x_w + x].output.ival = color << 8; // color = green
				for(k = 0 - g_day_inflection_point; k < IF_HEIGHT; k++)
				{
					neuron[k * x_w + x].output.ival = color << 8;
				}
			}
			
			if(x>0) // if not first sample
			{
				if(last_point < y){
					for(j = last_point+1; j < y; j++){
						neuron[j *x_w + x].output.ival = color;
					}
				}
				else{
					for(j = last_point-1; j > y; j--){
						neuron[j *x_w + x].output.ival = color;
					}
				}
			}
			last_point = y;			
		}
		update_input_image(input);
		if (status == MOVE)
		{
#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}
}

/*
***********************************************************
* Function: wnn_prediction_controler
* Description: handles the mouse input window events 
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void wnn_predicted_controler (INPUT_DESC *input, int status)
{
	NEURON *neuron;
	int x_w, y_h, i;
	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		if(neuron[input->wyd * x_w + input->wxd].output.ival == 255 && input->wxd <= 420){	
			printf("minute = %s day = %i\n",g_data_set[input->wxd].minute,g_day_wnn_predicted);
		}	
		if(input->wxd > 420 && g_day_wnn_predicted < (g_data_set_num_samples/420) - 1){
			for (i = 0; i < (IF_WIDTH * (IF_HEIGHT+1));i++){
				neuron[i].output.ival=0;
			}
			g_day_wnn_predicted++;
			printf("Wnn_predicted dia: %i\n",g_day_wnn_predicted+1);
			wnn_predicted_generator (input, status);	
		}
		if(input->wxd < 0 && g_day_wnn_predicted > 0){
			for (i = 0; i < (IF_WIDTH * (IF_HEIGHT+1));i++){
				neuron[i].output.ival=0;
			}
			g_day_wnn_predicted--;
			printf("Wnn_predicted dia: %i\n",g_day_wnn_predicted+1);
			wnn_predicted_generator (input, status);	
		}		
	}
	input->mouse_button = -1;
}

void update_file_max_min(FILE *max_file, int position,int point_max_min){
	char *minute;
	int point, i=0, day;
	
	day = g_day_inflection_point;
	minute = (char *) alloc_mem (11 * sizeof (char));
	while(i < g_data_set_num_samples){
		fscanf(max_file,"%s ",minute);
		if(strcmp(minute,g_data_set[position].minute) == 0){
			if(day == 0){
				fprintf(max_file,"%d",point_max_min);
				break;
			}
			day--;
		}
		fscanf(max_file,"%d\n",&point);	
		i++;
	}	
}

/*
***********************************************************
* Function: inflection_points_controler
* Description: handles the mouse input window events 
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void inflection_points_controler (INPUT_DESC *input, int status)
{
	NEURON *neuron;
	int x_w, y_h, i;
	static int color = 255;
	FILE *max_file;
	max_file=fopen(g_max_file_name,"r+");
	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		if(neuron[input->wyd * x_w + input->wxd].output.ival == color && input->wxd <= 420)
		{	
			color = color << 8;
			neuron[input->wyd * x_w + input->wxd].output.ival = color;
			update_file_max_min(max_file, input->wxd, 2);
		}	
		else if(neuron[input->wyd * x_w + input->wxd].output.ival == color << 8 && input->wxd <= 420)
		{
			color = color << 16;	
			neuron[input->wyd * x_w + input->wxd].output.ival = color;
			update_file_max_min(max_file, input->wxd, 1);
		}				
		else if(neuron[input->wyd * x_w + input->wxd].output.ival == color << 16 && input->wxd <= 420)
		{
			color = 255;
			neuron[input->wyd * x_w + input->wxd].output.ival = color;
			update_file_max_min(max_file, input->wxd, 0);
		}
		color=255;	
		update_input_image(input);
		if (input->wxd > 420 && g_day_inflection_point < (g_data_set_num_samples/420) - 1)
		{
			for (i = 0; i < (IF_WIDTH * (IF_HEIGHT+1));i++)
			{
				neuron[i].output.ival = 0;
			}
			g_day_inflection_point++;
			printf("Inflection_point dia: %i\n",g_day_inflection_point+1);
			inflection_points_generator (input, status);	
		}
		if (input->wxd < 0 && g_day_inflection_point > 0)
		{
			for (i = 0; i < (IF_WIDTH * (IF_HEIGHT+1));i++)
			{
				neuron[i].output.ival = 0;
			}
			g_day_inflection_point--;
			printf("Inflection_point dia: %i\n",g_day_inflection_point+1);
			inflection_points_generator (input, status);	
		}		
		if (max_file)
		{
			fclose(max_file);
		}
	}
	input->mouse_button = -1;
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

void input_generator_next (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_wnn_pred(input);
#ifdef NO_INTERFACE
		input->win = 2;	
#endif
	}
}



/*
***********************************************************
* Function: draw_output
* Description:Draws the output layer
* Inputs: strOutputName -
*	  strInputName -g_current_sample
* Output: None
***********************************************************
*/

void draw_output (char *strOutputName, char *strInputName)//
{
	OUTPUT_DESC *output;
	NEURON *neuron;
	int i;
	
	output = get_output_by_name (strOutputName);
	neuron = output->neuron_layer->neuron_vector;
	for (i = 0; i < output->wh * output->ww; i++)
	{
		neuron[i].output.ival = g_data_set[g_current_sample + POINTS_BEFORE].max_min;	
	}
	
	if (g_data_set[g_current_sample].train == 1)
	{
		if(g_data_set[g_current_sample + POINTS_BEFORE].max_min == 0){
			g_num_neutral_train++;
		}
		if(g_data_set[g_current_sample + POINTS_BEFORE].max_min == 1){
			g_num_min_train++;
		}
		if(g_data_set[g_current_sample + POINTS_BEFORE].max_min == 2){
			g_num_max_train++;
		}
	}

#ifdef	VERBOSE
	int now;
	now = LAST_SAMPLE_IN_THE_GROUP(g_current_sample);
	printf ("now %04d(%s) O<- %04d(%s)\n", now, g_data_set[now].minute, g_current_sample, g_data_set[g_current_sample].minute);
#endif

	update_output_image (output);
#ifndef NO_INTERFACE			
	glutSetWindow (output->win);
	glutPostWindowRedisplay (output->win);
#endif


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

void input_controler (INPUT_DESC *input, int status)
{
	NEURON *neuron;
	int x_w, y_h;
	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;	
	char strCommand[128];
	
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (strCommand, "draw nl_wnn_pred_out based on wnn_pred move;");
		interpreter (strCommand);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
		if (input->wxd > IN_WIDTH )
		{
			input_generator (input, status);
			g_day_inflection_point = g_current_sample / 420;
			g_day_wnn_predicted = g_current_sample / 420;
			inflection_points_generator (&inflection_points, MOVE);
			wnn_predicted_generator (&wnn_predicted, MOVE);
		}
		if (input->wxd < 0)
		{
			input_generator (input, status);	
			g_day_inflection_point = g_current_sample / 420;
			g_day_wnn_predicted = g_current_sample / 420;
			inflection_points_generator (&inflection_points, MOVE);
			wnn_predicted_generator (&wnn_predicted, MOVE);
		}

	}
	update_input_image(input);
	
	input->mouse_button = -1;
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: person ID
***********************************************************
*/

double 
EvaluateOutputAverage (OUTPUT_DESC *output, int *num_neurons)
{
	int i;
	double current_return, average_return = 0.0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_return = g_data_set[LAST_SAMPLE_IN_THE_GROUP(neuron_vector[i].output.ival)].sample_group_return;
		average_return += current_return;
		check_error(current_return, i, neuron_vector[i].output.ival, LAST_SAMPLE_IN_THE_GROUP(neuron_vector[i].output.ival), 
			    g_current_sample, LAST_SAMPLE_IN_THE_GROUP(g_current_sample), LAST_SAMPLE);
	}
	
	*num_neurons = output->wh * output->ww; /* it is not important in this type of output evaluation */
	
	return (average_return / (double) (output->wh * output->ww));
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: person ID
***********************************************************
*/

double 
EvaluateOutputAverageStopGainStopLoss (OUTPUT_DESC *output, int *num_neurons)//
{
	int i, predicted_sample, predicted_now;
	double current_return, average_return = 0.0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;

	for (i = 0; i < (output->wh * output->ww); i++)
	{
		predicted_sample = neuron_vector[i].output.ival;
		
		predicted_now = LAST_SAMPLE_IN_THE_GROUP(predicted_sample);
		current_return = get_sample_group_return(predicted_sample, predicted_now);
		average_return += current_return;
		check_error(current_return, i, neuron_vector[i].output.ival, LAST_SAMPLE_IN_THE_GROUP(neuron_vector[i].output.ival), 
			   g_current_sample, LAST_SAMPLE_IN_THE_GROUP(g_current_sample), LAST_SAMPLE);
		
	}
	
	*num_neurons = output->wh * output->ww; /* it is not important in this type of output evaluation */
	return (average_return / (double) (output->wh * output->ww));
}



/*
***********************************************************
* Function: EvaluateOutput
* Description: evaluates the output value
* Inputs: output
* Output: sample ID, num_neurons
***********************************************************
*/

int 
EvaluateOutput(OUTPUT_DESC *output, int *num_neurons)
{
	int i, j;
	int current_sample, selected_sample = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_sample = neuron_vector[i].output.ival;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.ival == current_sample)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			selected_sample = current_sample;
		}
	}
	
	*num_neurons = nMax;
	return (selected_sample);
}



/*
***********************************************************
* Function: output_handler
* Description: handles the mouse output window events 
* Inputs: output -
*	  type_call -
*	  mouse_button -
*	  mouse_state -
* Output: None
***********************************************************
*/

void 
output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	base_output_handler (output, type_call, mouse_button, mouse_state, EvaluateOutput);

}



/*
***********************************************************
* Function: output_handler
* Description: handles the mouse output window events 
* Inputs: output -
*	  type_call -
*	  mouse_button -
*	  mouse_state -
* Output: None
***********************************************************
*/

void 
output_handler_stop_gain_stop_loss (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	base_output_handler_stop_gain_stop_loss (output, type_call, mouse_button, mouse_state, EvaluateOutput);
	
}



/*
***********************************************************
* Function: output_handler
* Description: handles the mouse output window events 
* Inputs: output -
*	  type_call -
*	  mouse_button -
*	  mouse_state -
* Output: None
***********************************************************
*/

/*void 
output_handler_average (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	base_output_handler_average (output, type_call, mouse_button, mouse_state, EvaluateOutputAverage);

}*/



/*
***********************************************************
* Function: output_handler
* Description: handles the mouse output window events 
* Inputs: output -
*	  type_call -
*	  mouse_button -
*	  mouse_state -
* Output: None
***********************************************************
*/

void 
output_handler_average_stop_gain_stop_loss (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)//
{
	base_output_handler_average_stop_gain_stop_loss (output, type_call, mouse_button, mouse_state, EvaluateOutput);
	
}


void 
output_handler_contingency_table (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)//
{
	base_output_handler_contingency_table (output, type_call, mouse_button, mouse_state, EvaluateOutput);
	
}




/*
***********************************************************
* Function: f_keyboard
* Description: Funcao chamada quando eh pressionada uma tecla 
* Inputs: key_value -
* Output: None
***********************************************************
*/

void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
		case 'N':
		case 'n':
			if (g_nStatus == TRAINING_PHASE)
			{
				g_nStatus = RECALL_PHASE;
			}
			else
			{
				g_nStatus = TRAINING_PHASE;
			}
			break;
		case 'P':
		case 'p':
			printf("Oi!\n");
			break;
	}
}



NEURON_OUTPUT SavePredictionWnn (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	FILE *predicted_file;

	predicted_file = fopen(g_predicted_file_name, "w");
	for (i = 0; i < g_data_set_num_samples; i++){
		fprintf(predicted_file,"%s %i\n",g_data_set[i].minute,g_data_set[i].wnn_predicted_max_min);
	}
	
	fclose(predicted_file);	
	
	output.ival = 0;
	return (output);
}

NEURON_OUTPUT SaveSampleTest (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	FILE *test_file;

	test_file = fopen("samples_test.txt", "w");
	for (i = 0; i < g_data_set_num_samples; i++){
		if (g_data_set[i].test == 1)
			fprintf(test_file,"%i\n",(i + POINTS_BEFORE) );
	}
	
	fclose(test_file);	
	
	output.ival = 0;
	return (output);
}


/*
***********************************************************
* Function: ShowStatistics
* Description:
* Inputs:
* Output:
***********************************************************
*/

NEURON_OUTPUT ShowStatistics (PARAM_LIST *pParamList)//
{
	NEURON_OUTPUT output;
	int N, sample_group, i;
	double average_wnn_prediction_error = 0.0;
	double standard_deviation_wnn_error = 0.0;
	double average_target_return = 0.0;
	double standard_deviation_target_return = 0.0;
	int wnn_same_up = 0, target_up = 0, wnn_same_down = 0, target_down = 0;
	int now;
	double precision, recall,false_positive_rate,false_negative_rate;
	double macro_precision, macro_recall, macro_false_positive_rate, macro_false_negative_rate;
	double micro_precision, micro_recall, micro_false_positive_rate, micro_false_negative_rate;
	N = g_nTotalTested;

	PWB_ANALYSE_SERIES_TS *tgt;
	PWB_ANALYSE_SERIES_TS *out;

	macro_precision = 0;
	macro_recall = 0;
	macro_false_positive_rate = 0;
	macro_false_negative_rate = 0;
	micro_precision = 0;
	micro_recall = 0;
	micro_false_positive_rate = 0;
	micro_false_negative_rate = 0;

	tgt = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	tgt->v = alloc_mem (sizeof(double) * N);
	out = alloc_mem (sizeof(PWB_ANALYSE_SERIES_TS));
	out->v = alloc_mem (sizeof(double) * N);
	
	tgt->n_v = N;
	out->n_v = N;
	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);
		
		tgt->v[sample_group] = g_data_set[now].sample_group_return;
		out->v[sample_group] = g_data_set[now].wnn_predicted_return;
	}
	
	printf ("#  ****  me: %lf, rmse: %lf, mape: %lf, utheil: %lf, R2: %lf\n", 
			  ME(tgt, out), RMSE(tgt, out), MAPE(tgt, out), UTheil(tgt, out), R2(tgt, out));

	printf ("#  ****  hr_up: %.2f, hr_dn: %.2f, hr: %.2f\n", 
			  100.0 * HR_UP(tgt, out),
			  100.0 * HR_DN(tgt, out), 
			  100.0 * HR(tgt, out));
			 

	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);
		
		average_wnn_prediction_error += g_data_set[now].wnn_prediction_error;
		average_target_return += g_data_set[now].sample_group_return;
		
		if (g_data_set[now].sample_group_return > 0.0)
		{
			target_up++;
			if (g_data_set[now].wnn_predicted_return > 0.0)
				wnn_same_up++;
		}
		else if (g_data_set[now].sample_group_return < 0.0)
		{
			target_down++;
			if (g_data_set[now].wnn_predicted_return < 0.0)
				wnn_same_down++;
		}
	}
	average_wnn_prediction_error /= (double) N;
	average_target_return /= (double) N;

	for (sample_group = 0; sample_group < N; sample_group++)
	{
		now = LAST_SAMPLE_IN_THE_GROUP(g_first_sample_tested + SAMPLE_GROUP_SIZE * sample_group);

		standard_deviation_wnn_error += (g_data_set[now].wnn_prediction_error - average_wnn_prediction_error) *
					        (g_data_set[now].wnn_prediction_error - average_wnn_prediction_error);
		standard_deviation_target_return += (g_data_set[now].sample_group_return - average_target_return) *
					            (g_data_set[now].sample_group_return - average_target_return);
	}
	standard_deviation_wnn_error = sqrt (standard_deviation_wnn_error / (double) (N-1));
	standard_deviation_target_return = sqrt (standard_deviation_target_return / (double) (N-1)); 
	printf("\n\n\n\n");
	printf("g_money = %f g_amount_stocks = %f\n\n\n",g_money,g_amount_stocks);
	printf("\nN. amostras colecao: neutral = %i, min = %i, max = %i\n", g_num_neutral,g_num_min,g_num_max);	
	printf("\nN. amostras treino: neutral = %i, min = %i, max = %i\n", g_num_neutral_train,g_num_min_train,g_num_max_train);	
	for(i=0;i<3;i++){
		precision = ((double)(g_tp[i])/(double) (g_tp[i] + g_fp[i]));
		macro_precision = macro_precision + precision;
		recall = ((double)(g_tp[i])/(double) (g_tp[i] + g_fn[i]));
		macro_recall = macro_recall + recall;
		false_positive_rate = ((double)(g_fp[i])/(double) (g_fp[i] + g_tn[i]));
		macro_false_positive_rate = macro_false_positive_rate + false_positive_rate;
		false_negative_rate = ((double)(g_fn[i])/(double) (g_fn[i] + g_tp[i]));
		macro_false_negative_rate = macro_false_negative_rate + false_negative_rate;
		
		printf ("Classe %i\n tp = %i fn = %i fp = %i tn = %i\nprecision = %f recall = %f false_positive_rate = %f false_negative_rate = %f\n",i, g_tp[i], g_fn[i], g_fp[i], g_tn[i], precision, recall, false_positive_rate,false_negative_rate);
	}
	printf("\n\n%d %d %d",NL_WIDTH,NL_HEIGHT,SYNAPSES);
	for(i=0;i<3;i++){
		precision = ((double)(g_tp[i])/(double) (g_tp[i] + g_fp[i]));
		recall = ((double)(g_tp[i])/(double) (g_tp[i] + g_fn[i]));
		false_positive_rate = ((double)(g_fp[i])/(double) (g_fp[i] + g_tn[i]));
		false_negative_rate = ((double)(g_fn[i])/(double) (g_fn[i] + g_tp[i]));
		printf (" %f %f %f %f",precision, recall, false_positive_rate,false_negative_rate);
	}
	
	macro_precision = macro_precision / 3;
	macro_recall = macro_recall / 3;
	macro_false_positive_rate = macro_false_positive_rate / 3;
	macro_false_negative_rate = macro_false_negative_rate / 3;
	printf (" %f %f %f %f",macro_precision, macro_recall, macro_false_positive_rate, macro_false_negative_rate);

	micro_precision = ((double)(g_tp[0] + g_tp[1] + g_tp[2])) / ((double)((g_tp[0] + g_tp[1] + g_tp[2]) + (g_fp[0] + g_fp[1] + g_fp[2])));
	micro_recall = ((double)(g_tp[0] + g_tp[1] + g_tp[2])) / ((double)((g_tp[0] + g_tp[1] + g_tp[2]) + g_fn[0] + g_fn[1] + g_fn[2]));
	micro_false_positive_rate = ((double)(g_fp[0] + g_fp[1] + g_fp[2])) / ((double)((g_fp[0] + g_fp[1] + g_fp[2]) + g_tn[0] + g_tn[1] + g_tn[2]));
	micro_false_negative_rate = ((double)(g_fn[0] + g_fn[1] + g_fn[2])) / ((double)((g_fn[0] + g_fn[1] + g_fn[2]) + g_tp[0] + g_tp[1] + g_tp[2]));
	printf (" %f %f %f %f\n",micro_precision, micro_recall, micro_false_positive_rate, micro_false_negative_rate);
	
	
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

NEURON_OUTPUT ResetStatistics (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nTotalTested = 0;

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

NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;

	output.ival = 0;
	return (output);
}



/*
***********************************************************
* Function: GetSample
* Description: Get a sample
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSample (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_current_sample = pParamList->next->param.ival;	
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}


NEURON_OUTPUT OpenFileTrain (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	g_file_train = fopen ("samples_train.txt","r");

	output.ival = 0;
	
	return (output);
}

/*
***********************************************************
* Function: GetSampleForFile
* Description: Get a sample for vector
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSampleFromFile (PARAM_LIST *pParamList)//
{
	int sample_train;
	NEURON_OUTPUT output;
	//printf("sample_train = %d\n",sample_train);
	fscanf(g_file_train, "%d\n", &sample_train);
	//if (g_data_set[sample_train].train == 1)
		//printf("%d  %d\n",sample_train, g_data_set[sample_train+2].max_min);
	GetNextSample (NO_DIRECTION);
	g_current_sample = sample_train - POINTS_BEFORE;
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}

NEURON_OUTPUT GetSampleFromFile_old (PARAM_LIST *pParamList)//
{
	int i;
	NEURON_OUTPUT output;
	
	for (i = g_sample_train; i < g_data_set_num_samples; i++)
	{
		if (g_data_set[i].train == 1)
		{
			g_current_sample = i; 
			g_sample_train = i + 1;
			break;
		}
	}
	if (i == g_data_set_num_samples)
	{
		g_sample_train = 0;
	}
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}


/*
***********************************************************
* Function: GetSampleGroup
* Description: Get a sample group
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSampleGroup (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_current_sample = WINDOWS_OFFSET * pParamList->next->param.ival;
	
	GetNextSample (NO_DIRECTION);
	ReadSampleInput(&sample, g_current_sample);

	output.ival = 0;
	
	return (output);
}

NEURON_OUTPUT RewindFileTrain (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	
	rewind (g_file_train);
	
	output.ival = 0;
	
	return (output);
}

/*
***********************************************************
* Function: GetNumSamples
* Description: Get the naumber of samples
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumSamples (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = (LAST_SAMPLE/SAMPLE_GROUP_SIZE)*SAMPLE_GROUP_SIZE;
	
	return (output);
}



/*
***********************************************************
* Function: GetNumSampleGroups
* Description: Get the number of sample groups
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNumSampleGroups (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = LAST_SAMPLE / WINDOWS_OFFSET;
	
	return (output);
}


NEURON_OUTPUT ChangePhase (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	g_phase = 1;
	output.ival = 0;
	
	return (output);
}




/*
***********************************************************
* Function: GetSampleGroupSize
* Description: Get the sample group size
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetSampleGroupSize (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = SAMPLE_GROUP_SIZE;
	
	printf("SAMPLE_GROUP_SIZE=%i\n",SAMPLE_GROUP_SIZE);
	
	return (output);
}



/*
***********************************************************
* Function: GetNetworkInputSize
* Description: Get the network input size in sample groups
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNetworkInputSize (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = NETWORK_INPUT_SIZE;
	
	return (output);
}




/*
***********************************************************
* Function: GetNetworkInputSize
* Description: Get the network input size in sample groups
* Inputs:
* Output: returns 0 if OK, -1 otherwise
***********************************************************
*/

NEURON_OUTPUT GetNetworkInputStride (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = NETWORK_INPUT_STRIDE;
	
	return (output);
}
