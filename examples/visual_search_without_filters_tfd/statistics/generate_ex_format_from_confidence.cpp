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

Line *data;
int data_length;
int data_alloc;
double confidence_cutoff = 0.0;

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
perform_confidence_cutoff()
{
	int i;
	int halph_width;

	for(i = 0; i < data_length; i++)
	{
		if (data[i].confidence >= confidence_cutoff)
		{
			halph_width = (int) calculate_halph_width(data[i].scale_factor);

			printf("%05d.ppm;%d;%d;%d;%d\n",
				data[i].image_id,
				data[i].x - halph_width,
				IMG_HEIGHT - (data[i].y + halph_width),
				data[i].x + halph_width,
				IMG_HEIGHT - (data[i].y - halph_width)
			);
		}
	}
}


int
parse_arguments(int argc, char **argv)
{
	if (argc < 3)
		return 0;

	confidence_cutoff = atof(argv[2]);

	return 1;
}


void
show_usage_information_and_exit(char *program_name)
{
	printf("\nUse %s <data-file> <confidence-cutoff>\n", program_name);
	exit(-1);
}


int
main(int argc, char **argv)
{
	if (!parse_arguments(argc, argv))
		show_usage_information_and_exit(argv[0]);

	initialize_data();
	read_data_file(argv[1]);
	perform_confidence_cutoff();

	return 0;
}


