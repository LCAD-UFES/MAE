/*
*********************************************************************************
* Module : Input and output functions						*
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "mae.h"



/*
*********************************************************************************
* Add output to the output list							*
*********************************************************************************
*/

void
add_output_to_output_list (OUTPUT_DESC *output)
{
	OUTPUT_LIST *n_list;

	n_outputs++;
	
	if (global_output_list == (OUTPUT_LIST *) NULL)
	{
		n_list = (OUTPUT_LIST *) alloc_mem ((size_t) sizeof (OUTPUT_LIST));
		n_list->next = (OUTPUT_LIST *) NULL;
		n_list->output = output;
		global_output_list = n_list;
	}
	else
	{
		for (n_list = global_output_list; n_list->next != (OUTPUT_LIST *) NULL;
			n_list = n_list->next)
		{
			if (n_list->output == output)
				Erro ("two instances of the same output in global_output_list.", "", "");
		}
		if (n_list->output == output)
			Erro ("two instances of the same output in global_output_list.", "", "");
		n_list->next = (OUTPUT_LIST *) alloc_mem ((size_t) sizeof (OUTPUT_LIST));
		n_list->next->next = (OUTPUT_LIST *) NULL;
		n_list->next->output = output;
	}
}



/*
*********************************************************************************
* Add input to the input list							*
*********************************************************************************
*/

void
add_input_to_input_list (INPUT_DESC *input)
{
	INPUT_LIST *n_list;

	n_inputs++;

	if (global_input_list == (INPUT_LIST *) NULL)
	{
		n_list = (INPUT_LIST *) alloc_mem ((size_t) sizeof (INPUT_LIST));
		n_list->next = (INPUT_LIST *) NULL;
		n_list->input = input;
		global_input_list = n_list;
	}
	else
	{
		for (n_list = global_input_list; n_list->next != (INPUT_LIST *) NULL;
			n_list = n_list->next)
		{
			if (n_list->input == input)
				Erro ("two instances of the same input in global_input_list.", "", "");
		}
		if (n_list->input == input)
			Erro ("two instances of the same input in global_input_list.", "", "");
		n_list->next = (INPUT_LIST *) alloc_mem ((size_t) sizeof (INPUT_LIST));
		n_list->next->next = (INPUT_LIST *) NULL;
		n_list->next->input = input;
	}
}
 


/*! 
*********************************************************************************
* \brief Create input.
* \param[in] input A pointer to the input.
* \param[in] x_size The neuron layer width.
* \param[in] y_size The neuron layer height.
* \param[in] output_type The neuron output type (COLOR, GREYSCALE, GREYSCALE_FLOAT, BLACK_WHITE).
* \param[in] pyramid_height The number of image pyramid copies.
* \param[in] pyramid_type The image pyramid type (REGULAR_PYRAMID, GAUSSIAN_PYRAMID, LAPLACIAN_PYRAMID).
* \param[in] input_generator The input generator function.
* \param[in] input_controler The input controler function.
* \pre None.
* \post The input structure created and initialized.
* \return Nothing.
*********************************************************************************
*/

void create_input (INPUT_DESC *input, int x_size, int y_size, int output_type, int pyramid_height, int pyramid_type,
	      void (*input_generator)(INPUT_DESC *, int input_generator_status), 
	      void (*input_controler)(INPUT_DESC *, int input_controler_status), ...)
{ 
	va_list argument;
	char *parameters_input_generator, *parameters_input_controler;
/*	int i, level_area;
*/	
	input->neuron_layer = (NEURON_LAYER *) alloc_mem ((size_t) sizeof (NEURON_LAYER));
	memset((void *) (input->neuron_layer), 0, sizeof(NEURON_LAYER));
	input->neuron_layer->name = (char *) alloc_mem (strlen (input->name) + strlen ("_neuron_layer") + 1);
	
	strcpy (input->neuron_layer->name, input->name);
	strcat (input->neuron_layer->name, "_neuron_layer");
	
	input->pyramid_type = pyramid_type;
	input->pyramid_height = pyramid_height;
	
	input->neuron_layer->dimentions.x = x_size;
	input->neuron_layer->dimentions.y = y_size;
	
/*	for (i = 0, input->neuron_layer->num_neurons = 0, level_area = x_size * y_size; i <= pyramid_height; i++, level_area >> 2)
	       input->neuron_layer->num_neurons += level_area;
*/
	input->neuron_layer->num_neurons = x_size * y_size;
	       
	input->neuron_layer->neuron_type = NULL;

	initialise_neuron_vector (input->neuron_layer, 0);
        input->neuron_layer->output_type = output_type;
	
	input->input_generator = input_generator;
	input->input_controler = input_controler;
	
	// Sets the up to date input neuron layer flag to zero
	input->up2date = 0;
	
	input->cross_list = NULL;
	input->cross_list_size = 0;
	input->cross_list_colored = 0;
	
	input->rectangle_list = NULL;
	input->rectangle_list_size = 0;

	if ((input_generator != NULL) || (input_controler != NULL))
	{		
		va_start (argument, input_controler);
		
		parameters_input_generator = va_arg (argument, char *);

		if (input_generator != NULL)
			input->input_generator_params = get_param_list (parameters_input_generator, &(argument));
		
		parameters_input_controler = va_arg (argument, char *);
	
		if (input_controler != NULL)
			input->input_controler_params = get_param_list (parameters_input_controler, &(argument));
			
		va_end (argument);
	}
	
	add_input_to_input_list (input);
}  



/* 
********************************************************************************* 
* Create output 								* 
********************************************************************************* 
*/ 
 
void 
create_output (OUTPUT_DESC *output, int x_size, int y_size, void (*output_handler)(OUTPUT_DESC *,
	       int type_call, int mouse_button, int mouse_state), char *parameters, ...) 
{ 
	va_list argument;
	
/*	output->neuron_layer = (NEURON_LAYER *) alloc_mem ((size_t) sizeof (NEURON_LAYER));
	output->neuron_layer->name = (char *) alloc_mem (strlen (output->name) + strlen ("_neuron_layer") + 1);
	strcpy (output->neuron_layer->name, output->name);
	strcat (output->neuron_layer->name, "_neuron_layer");

	output->neuron_layer->dimentions.x = x_size;
	output->neuron_layer->dimentions.y = y_size;
	
	output->neuron_layer->neuron_vector = (NEURON *) NULL;
	output->neuron_layer->neuron_type = NULL;
*/
	output->neuron_layer = NULL;
	output->ww = x_size;
	output->wh = y_size;
	output->output_handler = output_handler;
	
	output->cross_list = NULL;
	output->cross_list_size = 0;
	output->cross_list_colored = 0;
	
	output->rectangle_list = NULL;
	output->rectangle_list_size = 0;

	if (output_handler != NULL)
	{
		va_start (argument, parameters);
		
		output->output_handler_params = get_param_list (parameters, &(argument));
		
		va_end (argument);
	}	
		
	add_output_to_output_list (output);
} 



/*
*********************************************************************************
* Output update									*
*********************************************************************************
*/

void 
output_update (OUTPUT_DESC *output)
{
	update_output_image (output);
	if (output->output_handler != NULL)
	{
		if (running)
			(*(output->output_handler)) (output, RUN, NO_BUTTON, NO_STATE);
		else
			(*(output->output_handler)) (output, SET_POSITION, NO_BUTTON, NO_STATE);
	}
#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display ();
#endif
}



/*
*********************************************************************************
* Output forward								*
*********************************************************************************
*/

void 
output_forward (OUTPUT_DESC *output)
{
	update_output_image (output);
	if (output->output_handler != NULL)
		(*(output->output_handler)) (output, FORWARD, NO_BUTTON, NO_STATE);
#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display ();
#endif
}



/*
*********************************************************************************
* Input update									*
*********************************************************************************
*/

void 
input_update (INPUT_DESC *input)
{
	if (input->input_controler != NULL)
	{
		if (running)
			(*(input->input_controler)) (input, RUN);
		else
			(*(input->input_controler)) (input, SET_POSITION);
	}
	if (input->input_generator != NULL)
	{
		if (running)
			(*(input->input_generator)) (input, RUN);
		else
			(*(input->input_generator)) (input, SET_POSITION);
	}
#ifndef NO_INTERFACE
	glutSetWindow (input->win);
	glutPostWindowRedisplay (input->win);
#endif
}



/*
*********************************************************************************
* Input forward									*
*********************************************************************************
*/

void 
input_forward (INPUT_DESC *input)
{
	if (input->input_controler != NULL)
		(*(input->input_controler)) (input, FORWARD);
	if (input->input_generator != NULL)
		(*(input->input_generator)) (input, FORWARD);
#ifndef NO_INTERFACE
	glutSetWindow (input->win);
	glutPostWindowRedisplay (input->win);
#endif
}



/*
*********************************************************************************
* All outputs update								*
*********************************************************************************
*/

void
all_outputs_update (void)
{
	OUTPUT_LIST *output;

	for (output = global_output_list; output != (OUTPUT_LIST *) NULL; output = output->next)
		output_update (output->output);
}



/*
*********************************************************************************
* All outputs forward								*
*********************************************************************************
*/

void
all_outputs_forward (void)
{
	OUTPUT_LIST *output;

	for (output = global_output_list; output != (OUTPUT_LIST *) NULL; output = output->next)
		output_forward (output->output);
}



/*
*********************************************************************************
* All inputs update								*
*********************************************************************************
*/

void
all_inputs_update (void)
{
	INPUT_LIST *input;

	for (input = global_input_list; input != (INPUT_LIST *) NULL; input = input->next)
		input_update (input->input);
}



/*
*********************************************************************************
* All inputs forward								*
*********************************************************************************
*/

void
all_inputs_forward (void)
{
	INPUT_LIST *input;

	for (input = global_input_list; input != (INPUT_LIST *) NULL; input = input->next)
		input_forward (input->input);
}



/*
*********************************************************************************
* Create I/O windows								*
*********************************************************************************
*/

void
create_io_windows (void)
{
	INPUT_LIST *input;
	OUTPUT_LIST *output;

	for (input = global_input_list; input != (INPUT_LIST *) NULL; input = input->next)
		create_input_window (input->input);

	for (output = global_output_list; output != (OUTPUT_LIST *) NULL; output = output->next)
		create_output_window (output->output);
}



/*
*********************************************************************************
* Function : get_image_via_neuron_layer						*
* Description: Get image from global input list via neuron_layer.		*
* version: 1.0									*
*    date: 24/10/2002								*
*      By: Dijalma Fardin Junior						*
********************************************************************************* 
*/

GLubyte *
get_image_via_neuron_layer (NEURON_LAYER *neuron_layer)
{
	INPUT_LIST *n_list;

	if (global_input_list != (INPUT_LIST *) NULL)
	{
		for (n_list = global_input_list; n_list != (INPUT_LIST *) NULL;
			n_list = n_list->next)
		{
			if (n_list->input->neuron_layer == neuron_layer)
				return n_list->input->image;
		}
	}
	return NULL;
}



/*
*********************************************************************************
* Get input descriptor by win							*
*********************************************************************************
*/

INPUT_DESC *
get_input_by_win (int win)
{
	INPUT_LIST *n_list;
	
	if (global_input_list == NULL)
		return (NULL);
	else
		for (n_list = global_input_list; n_list != (INPUT_LIST *) NULL;
			n_list = n_list->next)
			if (n_list->input->win == win)
				return (n_list->input);
	
	return (NULL); 
}



/*
*********************************************************************************
* Get output descriptor by win							*
*********************************************************************************
*/

OUTPUT_DESC *
get_output_by_win (int win)
{
	OUTPUT_LIST *n_list;
	
	if (global_output_list == NULL)
		return (NULL);
	else
		for (n_list = global_output_list; n_list != (OUTPUT_LIST *) NULL;
			n_list = n_list->next)
			if (n_list->output->win == win)
				return (n_list->output);
	
	return (NULL); 
}



/*
*********************************************************************************
* Get output descriptor by name							*
*********************************************************************************
*/

OUTPUT_DESC *
get_output_by_name (char *output_name)
{
	OUTPUT_LIST *n_list;
	
	if (global_output_list == NULL)
		return (NULL);
	else
		for (n_list = global_output_list; n_list != (OUTPUT_LIST *) NULL;
			n_list = n_list->next)
			if (strcmp (n_list->output->name, output_name) == 0)
				return (n_list->output);
	
	return (NULL); 
}



/*
*********************************************************************************
* Get output descriptor by neuron layer						*
*********************************************************************************
*/

OUTPUT_DESC *
get_output_by_neural_layer (NEURON_LAYER *neuron_layer)
{
	OUTPUT_LIST *n_list;
	
	if (global_output_list == NULL)
		return (NULL);
	else
		for (n_list = global_output_list; n_list != (OUTPUT_LIST *) NULL;
			n_list = n_list->next)
			if (n_list->output->neuron_layer->neuron_vector == neuron_layer->neuron_vector)
				return (n_list->output);

	return (NULL); 
}



/*
*********************************************************************************
* Get input descriptor by name							*
*********************************************************************************
*/

INPUT_DESC *
get_input_by_name (char *input_name)
{
	INPUT_LIST *n_list;
	
	if (global_input_list == NULL)
		return (NULL);
	else
		for (n_list = global_input_list; n_list != (INPUT_LIST *) NULL;
			n_list = n_list->next)
			if (strcmp (n_list->input->name, input_name) == 0)
				return (n_list->input);
	
	return (NULL); 
}



/*
*********************************************************************************
* Get input descriptor by neuron layer						*
*********************************************************************************
*/

INPUT_DESC *
get_input_by_neural_layer (NEURON_LAYER *neuron_layer)
{
	INPUT_LIST *n_list;
	
	if (global_input_list == NULL)
		return (NULL);
	else
		for (n_list = global_input_list; n_list != (INPUT_LIST *) NULL;
			n_list = n_list->next)
			if (n_list->input->neuron_layer == neuron_layer)
				return (n_list->input);
	
	return (NULL); 
}



/*
*********************************************************************************
* Get filter descriptor by output (neuron layer)				*
*********************************************************************************
*/

FILTER_DESC *
get_filter_by_output (NEURON_LAYER *output)
{
	FILTER_LIST *filter_list;
	 
	if (global_filter_list == NULL)
		return (NULL);
	else
		for (filter_list = global_filter_list; filter_list != (FILTER_LIST *) NULL;filter_list = filter_list->next)
			if (filter_list->filter_descriptor->output->neuron_vector == output->neuron_vector)
				return (filter_list->filter_descriptor);

	return (NULL); 
}



/*
*********************************************************************************
* Clear output by name								*
*********************************************************************************
*/

void 
clear_output_by_name (char *name)
{
	OUTPUT_DESC *output;
		
	if ((output = get_output_by_name (name)) != NULL)
	{
		set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
		
		update_output_image (output);
#ifndef NO_INTERFACE
		glutPostWindowRedisplay (output->win);
#endif
	}
	else
	{
		show_message ("Could not find valid output in clear_output () with the name: ", name, "");
		return;
	}
}
