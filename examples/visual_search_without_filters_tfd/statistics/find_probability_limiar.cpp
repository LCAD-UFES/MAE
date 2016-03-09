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

#include <map>
#include <vector>
using namespace std;

typedef struct
{
	int x;
	int y;
	int type;
	double probability;

}Sign;

typedef struct
{
	double probability;
	double confidence;
	int image_id;
	double scale_factor;
	int x;
	int y;

}DataProbLine;

Line *data;
DataProbLine *data_probabilities;
int data_length;
int data_alloc;
map<int, vector<Sign> > signs_per_image;
double min_acceptable_cutoff = 0;

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

		data[data_length].hit = 0;
		data_length++;
	}

	fclose(fileptr);
}


void
read_probabilities_file(char *filename)
{
	FILE *fileptr = fopen(filename, "r");

	data_probabilities = (DataProbLine *) calloc (data_length, sizeof(DataProbLine));

	for(int i = 0; i < data_length; i++)
	{
		fscanf(fileptr,
			"\np = %lf; confidence = %lf; image = %d; scale_factor = %lf; x = %d; y = %d\n",
			&(data_probabilities[i].probability),
			&(data_probabilities[i].confidence),
			&(data_probabilities[i].image_id),
			&(data_probabilities[i].scale_factor),
			&(data_probabilities[i].x),
			&(data_probabilities[i].y)
		);

		// teste de concistencia
		if ((data[i].x != data_probabilities[i].x) || (data[i].y != data_probabilities[i].y) || (data[i].confidence != data_probabilities[i].confidence) || (data[i].scale_factor != data_probabilities[i].scale_factor))
			exit(printf("Problem: Line %d:\n", i));
	}

	fclose(fileptr);
}


int
parse_arguments(int argc, char **argv)
{
	if (argc < 5)
		return 0;

	min_acceptable_cutoff = atof(argv[4]);

	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <data-file> <data-probabilities-file> <gt.txt> <min-cutoff>\n", program_name);
	exit(-1);
}


int
check_sign_is_already_in_the_table(Line *l)
{
	if (signs_per_image.count(l->image_id) == 0)
		return 0;
	else
	{
		for(unsigned int i = 0; i < signs_per_image[l->image_id].size(); i++)
			if ((l->x_g == signs_per_image[l->image_id][i].x) && (l->y_g == signs_per_image[l->image_id][i].y))
				return 1;

		return 0;
	}
}


void
update_probability_value_if_necessary(Line *l, Sign s)
{
	for(unsigned int i = 0; i < signs_per_image[l->image_id].size(); i++)
		if ((l->x_g == signs_per_image[l->image_id][i].x) && (l->y_g == signs_per_image[l->image_id][i].y))
			if (signs_per_image[l->image_id][i].probability < s.probability)
				signs_per_image[l->image_id][i] = s;
}


void
add_sign_to_table(Line *l, double probability)
{
	Sign s;

	s.x = l->x_g;
	s.y = l->y_g;
	s.type = l->TS_type;
	s.probability = probability;

	if (check_sign_is_already_in_the_table(l))
		update_probability_value_if_necessary(l, s);
	else
	{
		if (signs_per_image.count(l->image_id) == 0)
			signs_per_image[l->image_id] = vector<Sign>();

		signs_per_image[l->image_id].push_back(s);
	}
}


void
calculate_and_show_probability_limiar()
{
	int first_iteraction = 1;
	double min_probability = 0;
	map<int, vector<Sign> >::iterator it;

	// seleciona a menor probabilidade capaz de detectar uma placa
	for(it = signs_per_image.begin(); it != signs_per_image.end(); it++)
	{
		for(unsigned int i = 0; i < it->second.size(); i++)
		{
			if ((it->second[i].probability < min_probability) || (first_iteraction))
			{
				min_probability = it->second[i].probability;
				first_iteraction = 0;
			}
		}
	}

	printf("Cut off Probability: %lf\n", min_probability);

	printf("Placas analisadas:\n");
	for(it = signs_per_image.begin(); it != signs_per_image.end(); it++)
	{
		for(unsigned int i = 0; i < it->second.size(); i++)
		{
			printf("image = %d; x_g = %d; y_g = %d; probability = %lf\n", it->first, it->second[i].x, it->second[i].y, it->second[i].probability);
		}
	}
}


int
data_is_hit_on_ground_truth_line(IMAGE_INFO *ground_truth_line, Line *data_sample)
{
	return check_is_hit_based_on_gt(data_sample, ground_truth_line);
}


double
find_best_probability(IMAGE_INFO *ground_truth_line)
{
	int i, find_at_least_one_hit;
	double probability, max_probability;

	find_at_least_one_hit = 0;
	max_probability = 0;

	for(i = 0; i < data_length; i++)
	{
		if (ground_truth_line->ImgNo == data[i].image_id)
		{
			if (data_is_hit_on_ground_truth_line(ground_truth_line, &(data[i])))
			{
				find_at_least_one_hit = 1;
				probability = data_probabilities[i].probability;

				if (probability > max_probability)
					max_probability = probability;
			}
		}
	}

	if (find_at_least_one_hit)
	{
		int x = (ground_truth_line->leftCol + ground_truth_line->rightCol) / 2;
		int y = (ground_truth_line->topRow + ground_truth_line->bottomRow) / 2;
		printf("img = %05d x = %d y = %d probability = %lf\n", ground_truth_line->ImgNo, x, y, max_probability);

		return max_probability;
	}
	else
		return -1;
}


void
find_probability_limiar()
{
	int num_ground_truth_signs_not_detected, num_ground_truth_signs_detected;
	IMAGE_INFO *ground_truth_line;
	double best_probability, cutoff_probability;

	num_ground_truth_signs_not_detected = 0;
	num_ground_truth_signs_detected = 0;

	int first_iteraction = 1;
	cutoff_probability = 0;

	int total = 0;

	while ((ground_truth_line = get_next_ground_truth_sign()) != NULL)
	{
		if (is_prohibitory(ground_truth_line->ClassID) && (ground_truth_line->ImgNo >= 301))
		{
			best_probability = find_best_probability(ground_truth_line);

			if (best_probability >= min_acceptable_cutoff)
			{
				if ((best_probability < cutoff_probability) || (first_iteraction))
				{
					cutoff_probability = best_probability;
					first_iteraction = 0;
				}
				num_ground_truth_signs_detected++;
			}
			else
			{
				fprintf(stderr, "%05d.ppm;%d;%d;%d;%d;%d\n", 
					ground_truth_line->ImgNo, 
					ground_truth_line->leftCol, ground_truth_line->topRow, ground_truth_line->rightCol, ground_truth_line->bottomRow, 
					ground_truth_line->ClassID);
				num_ground_truth_signs_not_detected++;
			}

			total++;
		}
	}

	fprintf(stderr, "%% not detected: %lf %% detected: %lf total: %d\n", 100 * ((double) num_ground_truth_signs_not_detected / (double) total), 100 * ((double) num_ground_truth_signs_detected / (double) total), total);
	printf("cutoff_probability: %lf\n", cutoff_probability);
}


int
main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	read_data_file(argv[1]);
	load_ground_truth(argv[3]);
	read_probabilities_file(argv[2]);
	find_probability_limiar();

	return 0;
}
