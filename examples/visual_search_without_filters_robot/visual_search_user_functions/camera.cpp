/* Camera manipulation module for visual_search_whithout_filters_robot application */

#include <locale.h>
#include "stdio.h"
#include "stdlib.h"
#include "camera.hpp"				// camera.cpp prototypes
#include "libbee.hpp"				// libbee include - must be included here due to C++ conflicts

//Triclops Image format (PNM5) copy into input layer 
void	copy_bumblebee2_image_into_input(INPUT_DESC *input,TriclopsColorImage *image)
{
	int i, j;
	unsigned char intensity;
	int im_size = input->vpw * input->vph ;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = 0; i < input->vph ; i++)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) image->red[im_size - i * input->vpw + j];
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) image->green[im_size - i * input->vpw + j];
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) image->blue[im_size - i * input->vpw + j];
				}
			}
			break;
		case BLACK_WHITE:
			for (i = 0; i < input->vph ; i++)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					intensity = image->red[im_size - i * input->vpw + j];
					intensity += image->green[im_size - i * input->vpw + j];
					intensity += image->blue[im_size - i * input->vpw + j];

					// Intensity as pixel averaged intensity.					
					intensity /= 3;
					intensity = (intensity > 128) ? 255: 0;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
			break;
		default:
			unsigned char *red,*green,*blue;
			red = image->red;
			green = image->green;
			blue = image->blue;
			for (i = 0; i < input->vph ; i++)
			{
				for (j = 0; j < input->vpw; j++) 
				{
					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) image->red[im_size - i * input->vpw + j];
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) image->green[im_size - i * input->vpw + j];
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) image->blue[im_size - i * input->vpw + j];
				}
			}
	}
	printf("%d",image->rowinc);
}

/* 
TODO: GetLeft code should be updated when GetRight is finninshed
*/
NEURON_OUTPUT GetLeft (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	INPUT_DESC *input_layer;
	char	in_pattern_name[] = "in_pattern";

	//GetRight has no parameters
	if(pParamList->next->param.ival)
	{
		printf ("Error: GetRight must have no parameters.\n");
		output.ival = -1;
		return (output);
	}

	TriclopsColorImage *image;
	image = libbee_get_rectified_images(LEFT);

	if(!image)
	{
		printf ("GetRight Error: Couldn't Capture Image\n");
		output.ival = -1;
		return (output);
	}

	input_layer = get_input_by_name(in_pattern_name);
	if(!image)
	{
		printf ("Error: Invalid Input Layer\n");
		output.ival = -1;
		return (output);
	}

	// Copy the acquired Bumblebee2 image into the
	copy_bumblebee2_image_into_input(input_layer,image);

	// Image must be destroyed on capture end;
	libbee_destroy_flycapture_image(image);

	// Update the input layer neurons and Image
	check_input_bounds (input_layer, input_layer->wx + input_layer->ww/2, input_layer->wy + input_layer->wh/2);
	input_layer->up2date = 0;
	update_input_neurons (input_layer);
	update_input_image (input_layer);

	// Zero return on default
	output.ival = 0;
	return (output);
}

NEURON_OUTPUT GetRight (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	INPUT_DESC *input_layer;
	char	in_pattern_name[] = "in_pattern";

	//GetRight has no parameters
	if(pParamList->next->param.ival)
	{
		printf ("Error: GetRight must have no parameters.\n");
		output.ival = -1;
		return (output);
	}

	TriclopsColorImage *image;
	image = libbee_get_rectified_images(RIGHT);

	if(!image)
	{
		printf ("GetRight Error: Couldn't Capture Image\n");
		output.ival = -1;
		return (output);
	}

	input_layer = get_input_by_name(in_pattern_name);
	if(!image)
	{
		printf ("Error: Invalid Input Layer\n");
		output.ival = -1;
		return (output);
	}

	// Copy the acquired Bumblebee2 image into the
	copy_bumblebee2_image_into_input(input_layer,image);

	// Image must be destroyed on capture end;
	libbee_destroy_flycapture_image(image);

	// Update the input layer neurons and Image
	check_input_bounds (input_layer, input_layer->wx + input_layer->ww/2, input_layer->wy + input_layer->wh/2);
	input_layer->up2date = 0;
	update_input_neurons (input_layer);
	update_input_image (input_layer);

	// Zero return on default
	output.ival = 0;
	return (output);
}

// A Wrapper function to libbee camera initialize
void	visual_search_initialize_camera(void)
{
	libbbee_initialize_camera_context();
}

// A Wrapper function to libbee camera termination
void	visual_search_terminate_camera(void)
{
	libbbee_terminate_camera_context();
}
