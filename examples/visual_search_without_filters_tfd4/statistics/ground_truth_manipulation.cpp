
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ground_truth_manipulation.h"

int prohibitory[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 15, 16}; 				// (circular, white ground with red border)
int prohibitory_size = 12;
int mandatory[] =   {33, 34, 35, 36, 37, 38, 39, 40};					// (circular, blue ground)
int mandatory_size = 8;
int danger[] = 	    {11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; 	// (triangular, white ground with red border)
int danger_size = 15;

#define CONFIDENCE_LEVEL 0.41

// Criterions
#define	NONE			-1
#define	PROHIBITORY_AND_SCALED	0
#define	PROHIBITORY		1
#define	FOUND			2

IMAGE_INFO *g_image_info = NULL;
char *g_input_path = NULL;
int g_num_image_info = 0;

int num_signs_inside_box = 0;
int num_valid_scales = 0;
int num_prohibitive = 0;

int ground_truth_line = 0;

void
load_ground_truth(char *file_name)
{
	FILE *image_list = NULL;
	char file_line[512];
	int num_lines = 0;

	if ((image_list = fopen(file_name, "r")) == NULL)
	{
		printf("Error: could not open file '%s' in read_images_info_file().\n", file_name);
		exit(1);
	}

	while (fgets(file_line, 256, image_list))
		num_lines++;

	rewind(image_list);

	if (g_image_info != NULL)
		free(g_image_info);

	g_image_info = (IMAGE_INFO *) calloc(num_lines, sizeof(IMAGE_INFO));
	if (!g_image_info)
	{
		printf("Error: could not allocate memory in read_images_info_file().\n");
		exit(1);
	}

	for (g_num_image_info = 0; g_num_image_info < num_lines; g_num_image_info++)
	{
		fgets(file_line, 256, image_list);
		sscanf(file_line, "%d.ppm;%d;%d;%d;%d;%d", &(g_image_info[g_num_image_info].ImgNo), &(g_image_info[g_num_image_info].leftCol),
							   &(g_image_info[g_num_image_info].topRow), &(g_image_info[g_num_image_info].rightCol),
							   &(g_image_info[g_num_image_info].bottomRow), &(g_image_info[g_num_image_info].ClassID));
	}

	fclose(image_list);
}


IMAGE_INFO*
get_next_ground_truth_sign()
{
	if (ground_truth_line >= g_num_image_info)
		return NULL;
	else
	{
		IMAGE_INFO *gt_line = &(g_image_info[ground_truth_line]);
		ground_truth_line++;

		return gt_line;
	}
}


int
is_prohibitory(int ClassID)
{
	int i;

	for (i = 0; i < prohibitory_size; i++)
		if (ClassID == prohibitory[i])
			return (1);

	return (0);
}


int
sign_is_inside_box(Line *l, IMAGE_INFO *info)
{
	
	// so botei o fabs pq nao tenho certeza que o top eh maior que o
	// bottom e que o right eh maior que o left
	double width = fabs((double) (info->rightCol - info->leftCol));
	double height = fabs((double) (info->topRow - info->bottomRow));

	double center_x = (double) (info->leftCol + info->rightCol) / 2.0;
	double center_y = (double) (IMG_HEIGHT - 1) - (double) (info->topRow + info->bottomRow) / 2.0;

	double guessed_x = ((double) l->x);
	double guessed_y = ((double) l->y);

	double delta_x = fabs(guessed_x - center_x);
	double delta_y = fabs(guessed_y - center_y);

	//
	// DEBUG:
	// printf("-> center_x: %lf center_y: %lf l->x: %lf l->y: %lf delta_x: %lf width: %lf delta_y: %lf height: %lf\n", center_x, center_y, guessed_x, guessed_y, delta_x, width, delta_y, height);
	//

	if ((delta_x <= (0.8 * (width / 2.0))) && (delta_y < (0.8 * (height / 2.0))))
	{
		num_signs_inside_box++;
		return 1;
	}
	else
		return 0;
}


double
estimate_detected_width(Line *data_sample)
{
	double stimated_width = 2.0 * (18.0 / data_sample->scale_factor);

	return stimated_width;
}


int
scale_is_valid(Line *l, IMAGE_INFO *info)
{
	double true_width = fabs((double) (info->rightCol - info->leftCol));
	double guessed_width = (double) (estimate_detected_width(l));

	double lower_accepted_width = 0.7 * true_width;
	double upper_accepted_width = 1.2 * true_width;

	//
	// DEBUG:
	// printf("->->true width: %lf guessed width: %lf\n", true_width, guessed_width);

	if ((lower_accepted_width < guessed_width) && (guessed_width < upper_accepted_width))
	{
		num_valid_scales++;
		return 1;
	}
	else
		return 0;
}


int
sign_type_is_prohibitive(Line *l, IMAGE_INFO *info)
{
	// DEBUG:
	// printf("->->->class id: %d\n", info->ClassID);

	if (is_prohibitory(info->ClassID))
	{
		num_prohibitive++;
		return 1;
	}
	else
		return 0;
}


int
sign_has_valid_confidence(Line *l)
{
	if (l->confidence > 0.25)
		return 1;
	else
		return 0;
}


int
sign_is_hit(Line *l, IMAGE_INFO *info)
{
	if (sign_is_inside_box(l, info) && scale_is_valid(l, info) && 
	    sign_type_is_prohibitive(l, info) && sign_has_valid_confidence(l))
		return 1;
	else
		return 0;
}


int
check_is_hit(Line *l)
{
	int i;

	for (i = 0; i < g_num_image_info; i++)
	{
		if (l->image_id == g_image_info[i].ImgNo)
		{
			if (sign_is_hit(l, &(g_image_info[i])))
				return 1;
		}
	}

	return 0;
}

int
check_is_hit_based_on_gt(Line *line, IMAGE_INFO *gt_line)
{
	return sign_is_hit(line, gt_line);
}

void
print_hit_information()
{
	fprintf(stderr, "num_signs_inside_box: %d\n", num_signs_inside_box);
	fprintf(stderr, "num_signs_width_valid_scale: %d\n", num_prohibitive);
	fprintf(stderr, "num_prohibitive_signs: %d\n", num_valid_scales);
}


