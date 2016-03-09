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
#include "ground_truth_manipulation.h"

typedef struct
{
	double probability;
	double confidence;
	int image_id;
	double scale_factor;
	int x;
	int y;

}DataProbLine;

DataProbLine *data_probabilities;
int data_length;
int data_alloc;
double probability_cutoff = 0.0;

void
initialize_data()
{
	data_probabilities = (DataProbLine *) calloc (100, sizeof(DataProbLine));
	data_length = 0;
	data_alloc = 100;
}


void
check_alloc_data()
{
	if (data_length >= data_alloc)
	{
		data_alloc += 100;
		data_probabilities = (DataProbLine *) realloc (data_probabilities, data_alloc * sizeof(DataProbLine));
	}
}


void
read_data_file(char *filename)
{
	FILE *fileptr = fopen(filename, "r");

	if (fileptr == NULL)
		exit(printf("File '%s' could not be open!\n", filename));

	while (!feof(fileptr))
	{
		check_alloc_data();

		fscanf(fileptr,
			"\np = %lf; confidence = %lf; image = %d; scale_factor = %lf; x = %d; y = %d\n",
			&(data_probabilities[data_length].probability),
			&(data_probabilities[data_length].confidence),
			&(data_probabilities[data_length].image_id),
			&(data_probabilities[data_length].scale_factor),
			&(data_probabilities[data_length].x),
			&(data_probabilities[data_length].y)
		);

		data_length++;
	}

	fclose(fileptr);
}


double
calculate_halph_width(double scale_factor)
{
	double stimated_halph_width = (18.0 / scale_factor + 0.5);
	return stimated_halph_width;
}


void
perform_probability_cutoff()
{
	int i;
	int halph_width;

	for(i = 0; i < data_length; i++)
	{
		if (data_probabilities[i].probability >= probability_cutoff)
		{
			halph_width = (int) calculate_halph_width(data_probabilities[i].scale_factor);

			printf("%05d.ppm;%d;%d;%d;%d\n",
				data_probabilities[i].image_id,
				data_probabilities[i].x - halph_width,
				IMG_HEIGHT - (data_probabilities[i].y + halph_width),
				data_probabilities[i].x + halph_width,
				IMG_HEIGHT - (data_probabilities[i].y - halph_width)
			);
		}
	}
}


int
parse_arguments(int argc, char **argv)
{
	if (argc < 3)
		return 0;

	probability_cutoff = atof(argv[2]);

	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <data-probabilities-file> <probability-cutoff>\n", program_name);
	exit(-1);
}


int
main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	read_data_file(argv[1]);
	perform_probability_cutoff();

	return 0;
}


