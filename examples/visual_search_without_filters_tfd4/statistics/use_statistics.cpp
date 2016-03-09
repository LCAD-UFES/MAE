/**
 * @file
 * @author Filipe Mutz
 *
 * @section DESCRIPTION
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ground_truth_manipulation.h"

typedef struct
{
	int img_width;
	int img_height;
	int num_bins_x;
	int num_bins_y;
	int num_bins_z;
	int num_bins_c;
	double phit;
	double pmiss;
	double *zoom_levels;
	double *c_limits_hit;
	double *c_limits_miss;
	double *px_hit;
	double *px_miss;
	double *py_hit;
	double *py_miss;
	double *pz_hit;
	double *pz_miss;
	double *pc_hit;
	double *pc_miss;
}ProbabilitiesData;


ProbabilitiesData prob_data;

Line *data;
int data_length;
int data_alloc;


enum DATA_USED_IN_STATISTICS
{
	ALL = 0,
	ONLY_CONFIDENCE = 1,
	CONFIDENCE_AND_SCALE = 2,
	CONFIDENCE_SCALE_AND_X = 3,
	CONFIDENCE_AND_X = 4
};

DATA_USED_IN_STATISTICS features_to_use;

void
allocate_prob_data_arrays()
{
	prob_data.px_hit = (double *) calloc (prob_data.num_bins_x, sizeof(double));
	prob_data.px_miss = (double *) calloc (prob_data.num_bins_x, sizeof(double));

	prob_data.py_hit = (double *) calloc (prob_data.num_bins_y, sizeof(double));
	prob_data.py_miss = (double *) calloc (prob_data.num_bins_y, sizeof(double));

	prob_data.pz_hit = (double *) calloc (prob_data.num_bins_z, sizeof(double));
	prob_data.pz_miss = (double *) calloc (prob_data.num_bins_z, sizeof(double));
	prob_data.zoom_levels = (double *) calloc (prob_data.num_bins_z, sizeof(double));

	prob_data.pc_hit = (double *) calloc (prob_data.num_bins_c, sizeof(double));
	prob_data.pc_miss = (double *) calloc (prob_data.num_bins_c, sizeof(double));
	prob_data.c_limits_hit = (double *) calloc (prob_data.num_bins_c, sizeof(double));
	prob_data.c_limits_miss = (double *) calloc (prob_data.num_bins_c, sizeof(double));
}


void
read_x_probs(FILE *fileptr)
{
	int i, index;

	for(i = 0; i < prob_data.num_bins_x; i++)
		fscanf(fileptr, "\nX %d %lf\n", &index, &(prob_data.px_hit[i]));

	for(i = 0; i < prob_data.num_bins_x; i++)
		fscanf(fileptr, "\nX %d %lf\n", &index, &(prob_data.px_miss[i]));
}


void
read_y_probs(FILE *fileptr)
{
	int i, index;

	for(i = 0; i < prob_data.num_bins_y; i++)
		fscanf(fileptr, "\nY %d %lf\n", &index, &(prob_data.py_hit[i]));

	for(i = 0; i < prob_data.num_bins_y; i++)
		fscanf(fileptr, "\nY %d %lf\n", &index, &(prob_data.py_miss[i]));
}


void
read_z_probs(FILE *fileptr)
{
	int i;
	double trash;

	for(i = 0; i < prob_data.num_bins_z; i++)
		fscanf(fileptr, "\nZ %lf %lf\n", &(prob_data.zoom_levels[i]), &(prob_data.pz_hit[i]));

	for(i = 0; i < prob_data.num_bins_z; i++)
		fscanf(fileptr, "\nZ %lf %lf\n", &trash, &(prob_data.pz_miss[i]));
}


void
read_c_probs(FILE *fileptr)
{
	int i, index;
//	double trash;

	for(i = 0; i < prob_data.num_bins_c; i++)
		fscanf(fileptr, "\nC %d %lf\n", &index, &(prob_data.pc_hit[i]));

	for(i = 0; i < prob_data.num_bins_c; i++)
		fscanf(fileptr, "\nC %d %lf\n", &index, &(prob_data.pc_miss[i]));
}


void
read_prob_file(char *filename)
{
	FILE *fileptr = fopen(filename, "r");

	if (fileptr == NULL)
		exit(printf("File '%s' could not be open!\n", filename));

	// read configuration
	fscanf(fileptr, "height %d width %d\n", &(prob_data.img_height), &(prob_data.img_width));
	fscanf(fileptr, "\nnbins_x %d nbins_y %d nbins_z %d nbins_c %d\n", &(prob_data.num_bins_x), &(prob_data.num_bins_y), &(prob_data.num_bins_z), &(prob_data.num_bins_c));

	allocate_prob_data_arrays();

	// read the probabilities
	fscanf(fileptr, "\np_hit %lf\n", &(prob_data.phit));
	fscanf(fileptr, "\np_miss %lf\n", &(prob_data.pmiss));

	read_x_probs(fileptr);
	read_y_probs(fileptr);
	read_z_probs(fileptr);
	read_c_probs(fileptr);

	fclose(fileptr);
}


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

	while(!feof(fileptr))
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
		
		data[data_length].hit = 0;	
		data_length++;
	}

	fclose(fileptr);
}


int
parse_arguments(int argc, char **argv)
{
	if (argc < 4)
		return 0;

	int features_to_use_int = atoi(argv[3]);

	switch(features_to_use_int)
	{
		case 0:
			features_to_use = ALL;
			break;
		case 1:
			features_to_use = ONLY_CONFIDENCE;
			break;
		case 2:
			features_to_use = CONFIDENCE_AND_SCALE;
			break;
		case 3:
			features_to_use = CONFIDENCE_SCALE_AND_X;
			break;
		case 4:
			features_to_use = CONFIDENCE_AND_X;
			break;
		default:
			printf("ERROR: INVALID ARGUMENTS\n");
			return 0;
	}

	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <probabilities-file> <data-file> [DATA_USED_IN_STATISTICS: ALL = 0, ONLY_CONFIDENCE = 1, CONFIDENCE_AND_SCALE = 2, CONFIDENCE_SCALE_AND_X = 3, CONFIDENCE_AND_X = 4]\n", program_name);
	exit(-1);
}


int
find_z_index(double scale_factor)
{
	for(int i = 0; i < prob_data.num_bins_z; i++)
		if (scale_factor == prob_data.zoom_levels[i])
			return i;

	exit(printf("Error: zoom level '%lf' not found\n", scale_factor));
}


int
find_c_hit_index(double confidence)
{
	double pc_bin_variation = 1.0 / (prob_data.num_bins_c);
	int pc_index = (int) (confidence / pc_bin_variation);
	return pc_index;
}


int
find_c_miss_index(double confidence)
{	
	double pc_bin_variation = 1.0 / (prob_data.num_bins_c);
	int pc_index = (int) (confidence / pc_bin_variation);
	return pc_index;
}


double
calculate_bayes_rule(double phit, double pmiss, double px_hit, double px_miss, double py_hit, double py_miss, double pz_hit, double pz_miss, double pc_hit, double pc_miss)
{
	double probability, bayes_rule_numerator, bayes_rule_denominator;

	bayes_rule_numerator = 0;
	bayes_rule_denominator = 0;

	//
	// DEBUG:
	// printf("bayes_rule_numerator = %lf * (%lf * %lf * %lf * %lf) = %.13lf\n", phit, pc_hit, pz_hit, px_hit, py_hit, phit * (pc_hit * pz_hit * px_hit * py_hit));
	// printf("bayes_rule_denominator = (num) + (%lf * (%lf * %lf * %lf * %lf)) = %lf\n", pmiss, pc_miss, pz_miss, px_miss, py_miss, (pmiss * (pc_miss * pz_miss * px_miss * py_miss)));
	//

	switch(features_to_use)
	{
		case ALL:
			bayes_rule_numerator = phit * (pc_hit * pz_hit * px_hit * py_hit);
			bayes_rule_denominator = (phit * (pc_hit * pz_hit * px_hit * py_hit)) + (pmiss * (pc_miss * pz_miss * px_miss * py_miss));
			break;
		case ONLY_CONFIDENCE:
			bayes_rule_numerator = phit * (pc_hit);
			bayes_rule_denominator = (phit * (pc_hit)) + (pmiss * (pc_miss));
			break;
		case CONFIDENCE_AND_SCALE:
			bayes_rule_numerator = phit * (pc_hit * pz_hit);
			bayes_rule_denominator = (phit * (pc_hit * pz_hit)) + (pmiss * (pc_miss * pz_miss));
			break;
		case CONFIDENCE_SCALE_AND_X:
			bayes_rule_numerator = phit * (pc_hit * pz_hit * px_hit);
			bayes_rule_denominator = (phit * (pc_hit * pz_hit * px_hit)) + (pmiss * (pc_miss * pz_miss * px_miss));
			break;
		case CONFIDENCE_AND_X:
			bayes_rule_numerator = phit * (pc_hit * px_hit);
			bayes_rule_denominator = (phit * (pc_hit * px_hit)) + (pmiss * (pc_miss * px_miss));
			break;
		default:
			exit(printf("ERROR: INVALID CHOOSEN FEATURE\n"));
			break;
	}

	if (bayes_rule_denominator == 0)
		probability = 0.0;
	else
		probability = bayes_rule_numerator / bayes_rule_denominator;

	return probability;
}


void
calculate_and_print_line_probability(Line *data, int line)
{
	double confidence, scale_factor, x, y;
	double phit, pmiss, px_hit, px_miss, py_hit, py_miss, pz_hit, pz_miss, pc_hit, pc_miss;
	double probability;

	confidence = data[line].confidence;
	scale_factor = data[line].scale_factor;
	x = data[line].x;
	y = data[line].y;
	
	phit = prob_data.phit;
	pmiss = prob_data.pmiss;

	double px_bin_variation = ((double) prob_data.img_width) / (prob_data.num_bins_x);
	int px_index = (int) (x / px_bin_variation);
	px_hit = prob_data.px_hit[px_index];
	px_miss = prob_data.px_miss[px_index];

	double py_bin_variation = ((double) prob_data.img_height) / (prob_data.num_bins_y);
	int py_index = (int) (y / py_bin_variation);
	py_hit = prob_data.py_hit[py_index];
	py_miss = prob_data.py_miss[py_index];

	int pz_index = find_z_index(scale_factor);
	pz_hit = prob_data.pz_hit[pz_index];
	pz_miss = prob_data.pz_miss[pz_index];

	int pc_hit_index = find_c_hit_index(confidence);
	int pc_miss_index = find_c_miss_index(confidence);
	pc_hit = prob_data.pc_hit[pc_hit_index];
	pc_miss= prob_data.pc_miss[pc_miss_index];

	if (pz_miss == 0)
		exit(printf("OPPA!\n"));

	probability = calculate_bayes_rule(phit, pmiss, px_hit, px_miss, py_hit, py_miss, pz_hit, pz_miss, pc_hit, pc_miss);

	printf("p = %.4lf; confidence = %.4lf; image = %05d; scale_factor = %.5lf; x = %4d; y = %3d\n",
		probability, data[line].confidence, data[line].image_id, data[line].scale_factor, data[line].x, data[line].y);

}


int main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	read_prob_file(argv[1]);
	read_data_file(argv[2]);

	for(int i = 0; i < data_length; i++)
		calculate_and_print_line_probability(data, i);

	//printf("Terminou!\n");
	return 0;
}


