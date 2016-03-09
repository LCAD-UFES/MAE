/**
 * @file
 * @author Filipe Mutz
 *
 * @section DESCRIPTION
 * This program read the data from the sign detection module and generate a lot of statistics
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ground_truth_manipulation.h"

#define NUM_BINS_X 10
#define NUM_BINS_Y 10
#define NUM_BINS_Z 12
#define NUM_BINS_C 10
#define NUM_DIVISIONS_C 1000

#define MIN_SCALE_FACTOR 0.27344
#define MAX_SCALE_FACTOR 2.03166
double zoom_levels[12] = {0.27344, 0.32812, 0.39375, 0.47250, 0.56700, 0.68040, 0.81648, 0.97978, 1.17573, 1.41088, 1.69305, 2.03166};
//double zoom_levels[5] = {0.31250, 0.48906, 0.76538, 1.19782, 1.87460};

double prob_x_bins_given_hit[NUM_BINS_X];
double prob_y_bins_given_hit[NUM_BINS_Y];
double prob_z_bins_given_hit[NUM_BINS_Z];
double prob_x_bins_given_miss[NUM_BINS_X];
double prob_y_bins_given_miss[NUM_BINS_Y];
double prob_z_bins_given_miss[NUM_BINS_Z];
double prob_c_bins_given_hit[NUM_DIVISIONS_C];
double prob_c_bins_given_miss[NUM_DIVISIONS_C];

Line *data;
int data_length;
int data_alloc;


void
initialize_data()
{
	data = (Line *) calloc (100, sizeof(Line));

	data_length = 0;
	data_alloc = 100;
}


void
check_alloc_data()
{
	if (data_length >= data_alloc)
	{
		data_alloc += 100;
		data = (Line *) realloc (data, data_alloc * sizeof(Line));
	}
}


void
read_data_file(char *filename)
{
	char miss_or_hit[16];
	FILE *fileptr = fopen(filename, "r");

	if (fileptr == NULL)
		exit(printf("File '%s' could not be open!\n", filename));

	int n_hits_alberto = 0;
	int n_hits_filipe = 0;

	while (!feof(fileptr))
	{
		check_alloc_data();

		fscanf(fileptr, "\nimage = %d; scale_factor = %lf; x =  %d; y = %d; confidence = %lf; T.S. Type = %d; x_g = %d; y_g = %d; width %d; hight %d; %s\n",
			&(data[data_length].image_id),
			&(data[data_length].scale_factor),
			&(data[data_length].x),
			&(data[data_length].y),
			&(data[data_length].confidence),
			&(data[data_length].TS_type),
			&(data[data_length].x_g),
			&(data[data_length].y_g),
			&(data[data_length].width),
			&(data[data_length].height),
			miss_or_hit
		);


		if (!strcmp("HIT", miss_or_hit) && is_prohibitory(data[data_length].TS_type))
			n_hits_alberto++;

		if (check_is_hit(&(data[data_length])))
		{
			data[data_length].hit = 1;
			n_hits_filipe++;
/*			printf("image = %d; scale_factor = %lf; x =  %d; y = %d; confidence = %lf; T.S. Type = %d; x_g = %d; y_g = %d; width %d; hight %d\n",
			      data[data_length].image_id,
			      data[data_length].scale_factor,
			      data[data_length].x,
			      data[data_length].y,
			      data[data_length].confidence,
			      data[data_length].TS_type,
			      data[data_length].x_g,
			      data[data_length].y_g,
			      data[data_length].width,
			      data[data_length].height
			);
*/		}
		else
			data[data_length].hit = 0;

		data_length++;
	}
	
	fprintf(stderr, "num_hits_filipe: %d\n", n_hits_filipe);
	fprintf(stderr, "num_hits_alberto: %d\n", n_hits_alberto);

	print_hit_information();
	
	fclose(fileptr);
}


void
print_data()
{
	for(int i = 0; i < data_length; i++)
	{
		printf("image = %d; scale_factor = %lf; x =  %d; y = %d; confidence = %lf; T.S. Type = %d; x_g = %d; y_g = %d; width %d; hight %d; %s\n",
			data[i].image_id,
			data[i].scale_factor,
			data[i].x,
			data[i].y,
			data[i].confidence,
			data[i].TS_type,
			data[i].x_g,
			data[i].y_g,
			data[i].width,
			data[i].height,
			(data[i].hit == 1)? "HIT": "MISS"
		);
	}
}


int
parse_arguments(int argc, char **argv)
{
	if (argc < 3)
		return 0;

	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <data-file> <groundtruth-file>\n", program_name);
	
	exit(-1);
}


void
initialize_bin(double *bin, int num_bins)
{
	for (int i = 0; i < num_bins; i++)
		bin[i] = 0.0;
}


void
increment_frequency_in_the_bin(double *bins, int num_bins, double min_value, double max_value, double measurement)
{
	if (measurement > max_value)
		measurement = max_value;

	double bin_variation = ((max_value - min_value) / num_bins);
	int measurement_index = (int) ((measurement - min_value) / bin_variation);

	// do jeito que a conta esta, os limites superiores
	// sempre caem no proximo slot. o if abaixo garante
	// que as medidas iguais ao valor maximo nao tentem
	// acessar uma posicao fora do array
	if (measurement == max_value)
		measurement_index = (num_bins - 1);

	if (measurement_index >= num_bins || measurement_index < 0)
		exit(printf("measurement error: index out of bounds: %d in [0, %d]: measurement: %lf max_value: %lf\n", measurement_index, num_bins, measurement, max_value));

	bins[measurement_index]++;
}


void
normalize_bin_frequencies(double *bins, int num_bins, double num_hits)
{
	for (int i = 0; i < num_bins; i++)
		bins[i] /= num_hits;
}


void
show_bin_probabilities(double *bins, int num_bins, const char *label, const char *given_label)
{
	double summ = 0;
	
	for (int i = 0; i < num_bins; i++)
	{
		printf("%s %d %lf\n", label, i, bins[i]);
		summ += bins[i];
	}

	if (fabs(summ - 1) > 0.000001)
		exit(printf("ERRO: a soma das probabilidades no bin foi diferente de 1: label: '%s' soma: %lf\n", label, summ));
}


int
find_z_index(double scale_factor)
{
	for(int i = 0; i < NUM_BINS_Z; i++)
		if (scale_factor == zoom_levels[i])
			return i;

	exit(printf("Error: zoom level '%lf' not found\n", scale_factor));
}

void
initialize_bins()
{
	initialize_bin(prob_x_bins_given_hit, NUM_BINS_X);
	initialize_bin(prob_y_bins_given_hit, NUM_BINS_Y);
	initialize_bin(prob_z_bins_given_hit, NUM_BINS_Z);
	initialize_bin(prob_x_bins_given_miss, NUM_BINS_X);
	initialize_bin(prob_y_bins_given_miss, NUM_BINS_Y);
	initialize_bin(prob_z_bins_given_miss, NUM_BINS_Z);
	initialize_bin(prob_c_bins_given_hit, NUM_BINS_C);
	initialize_bin(prob_c_bins_given_miss, NUM_BINS_C);
}


void
normalize_probabilities(int num_hits, int num_misses)
{
	normalize_bin_frequencies(prob_x_bins_given_hit, NUM_BINS_X, num_hits);
	normalize_bin_frequencies(prob_x_bins_given_miss, NUM_BINS_X, num_misses);
	normalize_bin_frequencies(prob_y_bins_given_hit, NUM_BINS_Y, num_hits);
	normalize_bin_frequencies(prob_y_bins_given_miss, NUM_BINS_Y, num_misses);
	normalize_bin_frequencies(prob_z_bins_given_hit, NUM_BINS_Z, num_hits);
	normalize_bin_frequencies(prob_z_bins_given_miss, NUM_BINS_Z, num_misses);
	normalize_bin_frequencies(prob_c_bins_given_hit, NUM_BINS_C, num_hits);
	normalize_bin_frequencies(prob_c_bins_given_miss, NUM_BINS_C, num_misses);
}


void
show_configuration()
{
	printf("height %d width %d\n", IMG_HEIGHT, IMG_WIDTH);
	printf("nbins_x %d nbins_y %d nbins_z %d nbins_c  %d\n", NUM_BINS_X, NUM_BINS_Y, NUM_BINS_Z, NUM_BINS_C);
}


void
compute_variables_probabilities()
{
	double num_hits = 0, num_misses = 0;
	double prob_hit = 0, prob_miss = 0;
	double avg_confidence_on_hit = 0;

	for (int i = 0; i < data_length; i++)
	{
		int z_index = find_z_index(data[i].scale_factor);

		if (data[i].hit)
		{
			increment_frequency_in_the_bin(prob_x_bins_given_hit, NUM_BINS_X, 0.0, IMG_WIDTH, data[i].x);
			increment_frequency_in_the_bin(prob_y_bins_given_hit, NUM_BINS_Y, 0.0, IMG_HEIGHT, data[i].y);
			increment_frequency_in_the_bin(prob_c_bins_given_hit, NUM_BINS_C, 0.0, 1.0, data[i].confidence);

			prob_z_bins_given_hit[z_index]++;
			//increment_frequency_in_the_bin(, NUM_BINS_Z, MIN_SCALE_FACTOR, MAX_SCALE_FACTOR, data[i].scale_factor);

			avg_confidence_on_hit += data[i].confidence;
			prob_hit++;
			num_hits++;
		}
		else
		{
			increment_frequency_in_the_bin(prob_x_bins_given_miss, NUM_BINS_X, 0.0, IMG_WIDTH, data[i].x);
			increment_frequency_in_the_bin(prob_y_bins_given_miss, NUM_BINS_Y, 0.0, IMG_HEIGHT, data[i].y);
			increment_frequency_in_the_bin(prob_c_bins_given_miss, NUM_BINS_C, 0.0, 1.0, data[i].confidence);

			prob_z_bins_given_miss[z_index]++;
			//increment_frequency_in_the_bin(prob_z_bins_given_miss, NUM_BINS_Z, MIN_SCALE_FACTOR, MAX_SCALE_FACTOR, data[i].scale_factor);

			prob_miss++;
			num_misses++;
		}
	}


	if (num_hits == 0)
		exit(fprintf(stderr, "ERROR: NUM HITS: %lf\n", num_hits));

	if (num_hits == 0)
		exit(fprintf(stderr, "ERROR: NUM MISSES: %lf\n", num_misses));

	show_configuration();
	normalize_probabilities(num_hits, num_misses);
	
	fprintf(stderr, "avg_confidence_on_hit: %lf\n", avg_confidence_on_hit / num_hits);

	prob_hit /= data_length;
	prob_miss/= data_length;

	printf("p_hit %lf\n", prob_hit);
	printf("p_miss %lf\n", prob_miss);
}


void
print_probabilities()
{
	show_bin_probabilities(prob_x_bins_given_hit, NUM_BINS_X, "X", "D");
	show_bin_probabilities(prob_x_bins_given_miss, NUM_BINS_X, "X", "!D");
	show_bin_probabilities(prob_y_bins_given_hit, NUM_BINS_Y, "Y", "D");
	show_bin_probabilities(prob_y_bins_given_miss, NUM_BINS_Y, "Y", "!D");
	
	for (int i = 0; i < NUM_BINS_Z; i++)
		printf("Z %lf %lf\n", zoom_levels[i], prob_z_bins_given_hit[i]);

	for (int i = 0; i < NUM_BINS_Z; i++)
		printf("Z %lf %lf\n", zoom_levels[i], prob_z_bins_given_miss[i]);

	show_bin_probabilities(prob_c_bins_given_hit, NUM_BINS_C, "C", "D");
	show_bin_probabilities(prob_c_bins_given_miss, NUM_BINS_C, "C", "!D");
}


void
calculate_probabilities()
{	
	initialize_bins();
	compute_variables_probabilities();
	print_probabilities();	
}


int
main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	load_ground_truth(argv[2]);
	read_data_file(argv[1]);

	calculate_probabilities();
	
	return 0;
}


