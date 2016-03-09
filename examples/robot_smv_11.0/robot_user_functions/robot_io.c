#include <time.h>
#include <sys/stat.h>
#include "robot_nl_functions.h"

int current_camera_pair = 0;

int
file_exists(char *name)
{
   struct stat buf;
   return !stat(name, &buf);
}

void
process_camera_pair()
{
	time_t start, end;

	rectify_image();

	//Update Gabor layers
	start = time(NULL);
	update_simple_mono_cells_filters();
	end = time(NULL);
	printf("Time to update mono cells: %f s\n", difftime(end, start));

	//Generate disparity map
	start = time(NULL);
	generate_disparity_map_plan();
	end = time(NULL);
	printf("Time to generate disparity map: %f s\n", difftime(end, start));
}

int
load_current_camera_pair(char *name, char *format)
{
	char original_left[256],  rectified_left[256];
	char original_right[256], rectified_right[256];
	char disparity_map[256],  segment_map[256];

	sprintf(original_left,   "./%s_%d_left.%s",          name, current_camera_pair, format);
	sprintf(original_right,  "./%s_%d_right.%s",         name, current_camera_pair, format);
	sprintf(rectified_left,  "./%s_%d_rtf_left.%s",      name, current_camera_pair, format);
	sprintf(rectified_right, "./%s_%d_rtf_right.%s",     name, current_camera_pair, format);
	sprintf(disparity_map,   "./%s_%d_disparity_map.%s", name, current_camera_pair, format);
	sprintf(segment_map,     "./%s_%d_segment_map.%s",   name, current_camera_pair, format);

	if (file_exists(rectified_left) && file_exists(rectified_right)
         && file_exists(disparity_map)  && file_exists(segment_map))
	{
		load_image_to_object("image_left",                rectified_left);
		load_image_to_object("image_right",               rectified_right);
		load_image_to_object("nl_disparity_map",          disparity_map);
		load_image_to_object("nl_trunk_segmentation_map", segment_map);
	}
	else if (file_exists(original_left) && file_exists(original_right))
	{
		load_image_to_object("image_left", original_left);
		load_image_to_object("image_right", original_right);

		process_camera_pair();

		save_object_image(rectified_left,  "image_left");
		save_object_image(rectified_right, "image_right");
		save_object_image(disparity_map,   "nl_disparity_map");
		save_object_image(segment_map,     "nl_trunk_segmentation_map");
	}
	else
		return 0;

	all_outputs_update();

	return 1;
}

int
load_camera_pair(char *name, int camera_pair, char *format)
{
	current_camera_pair = camera_pair;
	return load_current_camera_pair(name, format);
}

int
get_current_camera_pair (void)
{
	return current_camera_pair;
}
