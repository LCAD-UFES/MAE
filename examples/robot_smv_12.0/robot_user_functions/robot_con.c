/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#include "robot_con.h"

#include <sys/stat.h>

#include "../util/cconfig.h"
#include "circle_packing.h"
#include "mae.h"
#include "../robot.h"
#include "robot_nl_functions.h"

/*
Definition Section
*/

#define ZOOM(value, fromScale, toScale) (int) ((float) (value * toScale) / (float) fromScale + 0.5)

/*
Function Section
*/

void robot_con_init()
{
	init_pattern_xor_table();
	init_measuraments();
	build_network();
}

int cache_file_exists(const char *name)
{
   struct stat buf;
   return !stat(name, &buf);
}

void robot_con_execute()
{
	time_t start, end;

	rectify_all_images();

	//Update Gabor layers
	start = time(NULL);
	update_simple_mono_cells_filters();
	end = time(NULL);
	printf("Time to update mono cells: %f s\n", difftime(end, start));

	//Generate disparity map
	start = time(NULL);
	generate_disparity_map_plan(&nl_mt_gaussian, &nl_mt_gaussian_map, &nl_disparity_map, "nl_mt");
	end = time(NULL);
	printf("Time to generate disparity map: %f s\n", difftime(end, start));
	
	//Disparity Map ready, then call segmentation filter
	filter_find_update("nl_segment_filter");

	all_outputs_update();
}

int robot_con_load_cache(const char* name, int side, const char* format)
{
	char rectified_left[256], rectified_right[256];
	char disparity_map[256],  segment_map[256], rotate_map[256];

	sprintf(rectified_left,  "./%s_%d_rtf_left.%s",      name, side, format);
	sprintf(rectified_right, "./%s_%d_rtf_right.%s",     name, side, format);
	sprintf(disparity_map,   "./%s_%d_disparity_map.%s", name, side, format);
	sprintf(segment_map,     "./%s_%d_segment_map.%s",   name, side, format);
	sprintf(rotate_map,      "./%s_%d_rotate_map.%s",    name, side, format);

	if (cache_file_exists(rectified_left) && cache_file_exists(rectified_right)
         && cache_file_exists(disparity_map)  && cache_file_exists(segment_map))
	{
		load_image_to_object("image_left",                rectified_left);
		load_image_to_object("image_right",               rectified_right);
		load_image_to_object("nl_disparity_map",          disparity_map);
		load_image_to_object("nl_trunk_segmentation_map", segment_map);
		load_image_to_object("nl_trunk_segmentation_map_vertical_rotate", rotate_map);
		
		all_outputs_update();

		return 1;
	}
	else
		return 0;
}

void robot_con_save_cache(const char* name, int side, const char* format)
{
	char rectified_left[256], rectified_right[256];
	char disparity_map[256],  segment_map[256], rotate_map[256];

	sprintf(rectified_left,  "./%s_%d_rtf_left.%s",      name, side, format);
	sprintf(rectified_right, "./%s_%d_rtf_right.%s",     name, side, format);
	sprintf(disparity_map,   "./%s_%d_disparity_map.%s", name, side, format);
	sprintf(segment_map,     "./%s_%d_segment_map.%s",   name, side, format);
	sprintf(rotate_map,      "./%s_%d_rotate_map.%s",    name, side, format);

	save_object_image(rectified_left,  "image_left");
	save_object_image(rectified_right, "image_right");
	save_object_image(disparity_map,   "nl_disparity_map");
	save_object_image(segment_map,     "nl_trunk_segmentation_map");
	save_object_image(rotate_map,      "nl_trunk_segmentation_map_vertical_rotate");
}

void robot_con_fill_network(const char* name, int side, int half, const char* format, int* left, int* right)
{
	robot_con_set_image_left(left);
	robot_con_set_image_right(right);

	robot_con_execute();

	robot_con_save_cache(name, side, format);
}

int robot_con_load_network(const char* name, int side, int half, const char* format)
{
	char original_left[256], original_right[256];
	sprintf(original_left,  "./%s_%d_left.%s",  name, side, format);
	sprintf(original_right, "./%s_%d_right.%s", name, side, format);

	if (robot_con_load_cache(name, side, format))
	{
		filter_find_update("nl_segment_filter");
		return 1;
	}

	else if (cache_file_exists(original_left) && cache_file_exists(original_right))
	{
		load_image_to_object("image_left",  original_left);
		load_image_to_object("image_right", original_right);

		robot_con_execute();

		robot_con_save_cache(name, side, format);

		return 1;
	}
	else
		return 0;
}

void robot_con_select_match_region(char* match_layer)
{
	char x_offset_name[256],  y_offset_name[256];
	sprintf(x_offset_name, "matching.%s.offset.x", match_layer);
	sprintf(y_offset_name, "matching.%s.offset.y", match_layer);

	config_load();
	int x_offset = config_get_int(x_offset_name);
	int y_offset = config_get_int(y_offset_name);

	INPUT_DESC* input = get_input_by_name(match_layer);
	GLubyte* region = input->image;
	int regionWidth  = input->tfw;

	GLubyte* image = image_right.image;
	int imageWidth = image_right.tfw;

	int i = 0, j = 0, m = 0, n = 0;
	for (i = 0, m = input->vph; i < m; i++)
	{
		for (j = 0, n = input->vpw; j < n; j++)
		{
			int regionOffset = 3*(i*regionWidth + j);
			int imageOffset  = 3*((i + y_offset)*imageWidth + (j + x_offset));

			region[regionOffset + 0] = image[imageOffset + 0];
			region[regionOffset + 1] = image[imageOffset + 1];
			region[regionOffset + 2] = image[imageOffset + 2];
		}
	}

	update_input_neurons(input);
	update_input_image(input);
	all_outputs_update();
}

long robot_con_merge_offset()
{
	return compute_horizontal_offset();
}

void robot_con_copy_to_merge_map(int w0, int offset)
{
	NEURON* disparity_map = nl_disparity_map.neuron_vector;
	NEURON* merge_map = match_merge_map.neuron_vector;

	int image_width = image_right.neuron_layer->dimentions.x;
	int disparity_width = nl_disparity_map.dimentions.x;
	int merge_width = match_merge_map.dimentions.x;
	int height = nl_disparity_map.dimentions.y;

	int neuron_w0 = ZOOM(w0, image_width, disparity_width);
	int neuron_offset = ZOOM(offset, image_width, disparity_width);

	int i = 0;
	for (i = 0; i < height; i++)
	{
		int j = 0, n = disparity_width - neuron_w0;
		for (j = 0; j < n; j++)
		{
			int merge_index = i*merge_width + (j + neuron_offset);
			int disparity_index = i*disparity_width + (j + neuron_w0);
			merge_map[merge_index].output.fval = disparity_map[disparity_index].output.fval;
		}
		
		for (j = neuron_offset + n; j < merge_width; j++)
			merge_map[i*merge_width + j].output.fval = 0.0;
	}

	all_outputs_update();
}


void robot_con_get_points(int* workingArea, double* leftPoints, double* rightPoints, int* colors)
{
	// Dimensoes do cortex (disparity map)
	int ho = nl_disparity_map.dimentions.y;
	int wo = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	int hi = image_right.neuron_layer->dimentions.y;
	int wi = image_right.neuron_layer->dimentions.x;

	int xi = 0, yi = 0, num_points = 0;
	for (xi = workingArea[0]; xi < workingArea[2]; xi++)
	{
		int xo = ZOOM(xi, wi, wo);
		for (yi = workingArea[1]; yi < workingArea[3]; yi++)
		{
			int yo = ZOOM(yi, hi, ho);
			if ((0 <= xo && xo < wo) && (0 <= yo && yo < ho))
			{
				float disparity = nl_disparity_map.neuron_vector[wo * yo + xo].output.fval;

				leftPoints [num_points + 0] = (double) xi + disparity;
				leftPoints [num_points + 1] = (double) yi;
				rightPoints[num_points + 0] = (double) xi;
				rightPoints[num_points + 1] = (double) yi;

				colors[num_points / 2] = image_right.neuron_layer->neuron_vector[yi * wi + xi].output.ival;

				num_points += 2;
			}
		}
	}
}

void robot_con_setup_circle_packing(CYLINDER_LIST* list)
{
	circle_packing_set_cylinder_list(list);
}

/*
Property Section
*/

int robot_con_get_input_height()
{
	return INPUT_HEIGHT;
}

int robot_con_get_input_width()
{
	return INPUT_WIDTH;
}

void robot_con_set_input(INPUT_DESC* input, int* buffer)
{
	int i, j;
	int w = input->ww;
	int h = input->wh;

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			input->image[3 * (i + j * image_left.tfw) + 0] =   RED(buffer[(h - j - 1) + i * h]);
			input->image[3 * (i + j * image_left.tfw) + 1] = GREEN(buffer[(h - j - 1) + i * h]);
			input->image[3 * (i + j * image_left.tfw) + 2] =  BLUE(buffer[(h - j - 1) + i * h]);
		}
	}
}

void robot_con_set_image_left(int* left)
{
	robot_con_set_input(&image_left, left);
}

void robot_con_set_image_right(int* right)
{
	robot_con_set_input(&image_right, right);
}
