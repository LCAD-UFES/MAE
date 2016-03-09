/**
 * @file
 * @author Filipe Mutz
 *
 * @section DESCRIPTION
 *
 */

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
int data_length;
int data_alloc;
DataProbLine *data_probabilities;
double cutoff_frequency = 0;

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
	if (argc < 4)
		return 0;

	cutoff_frequency = atof(argv[3]);
	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <data-file> <data-probabilities-file> <cutoff-frequency>\n", program_name);
	exit(-1);
}


void
probability_filter()
{
	int i;

	for (i = 0; i < data_length; i++)
	{
		if (data_probabilities[i].probability > cutoff_frequency)
		{
			printf("p = %lf; confidence = %lf; image = %d; scale_factor = %lf; x = %d; y = %d\n",
				data_probabilities[i].probability,
				data_probabilities[i].confidence,
				data_probabilities[i].image_id,
				data_probabilities[i].scale_factor,
				data_probabilities[i].x,
				data_probabilities[i].y
			);

//			printf("image = %d; scale_factor = %lf; x =  %d; y = %d; confidence = %lf; T.S. Type = %d; x_g = %d; y_g = %d; width = %d; height = %d; probability = %lf\n",
//				data_probabilities[i].probability
//				data[i].image_id,
//				data[i].scale_factor,
//				data[i].x,
//				data[i].y,
//				data[i].confidence,
//				data[i].TS_type,
//				data[i].x_g,
//				data[i].y_g,
//				data[i].width,
//				data[i].height,
//			);

		}
	}
}


int
main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	read_data_file(argv[1]);
	read_probabilities_file(argv[2]);
	probability_filter();

	return 0;
}



