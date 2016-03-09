	#include <stdio.h> 
	#include <stdlib.h>
	#include "mae.h"

	#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	#include "forms.h"
	#include "mae_control.h"
	#include <GL/glx.h> 
	#endif

	#include <unistd.h>
	#include <math.h> 


	NEURON_LAYER *copied_data = NULL;
	int num_steps = 5;

	const char *command_string;

	char test_output_name[SYMBOL_SIZE];
	long test_neuron = 0;

	//Global variables for test_neuron_v
	long column_counter = 0;
	long vertical_neuron_counter = 0;
	long neuron_counter = 0;


	/*
	*********************************************************************************
	* Function: get_object_by_name							*
	* Description: get a pointer to an object (neuron_layer, output, input, etc.) 	*
	* using its name								*
	* Inputs: object name								*
	*********************************************************************************
	*/

	int
	get_object_by_name (char *name, void **object)
	{
		if ((*object = (void *) get_neuron_layer_by_name (name)) != NULL)
			return (NEURON_LAYER_OBJECT);
		if ((*object = (void *) get_input_by_name (name)) != NULL)
			return (INPUT_OBJECT);
		if ((*object = (void *) get_output_by_name (name)) != NULL)
			return (OUTPUT_OBJECT);
		if ((*object = (void *) get_filter_by_name (name)) != NULL)
			return (FILTER_OBJECT);
		return (-1);
	}



	/*
	*********************************************************************************
	* Function: clear_command_list							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	clear_command_list ()
	{
		COMMAND_LIST *aux, *aux2;
		
		aux = command_list;
		while (aux != NULL)
		{
			aux2 = aux->next;
			free (aux->command);
			free (aux);
			aux = aux2;
		}
		command_list_tail = command_list = NULL;
		clear_browser ();
	}



	/*
	*********************************************************************************
	* Function: reinit_inputs							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	reinit_inputs (void)
	{
		INPUT_LIST *p_input;

		for (p_input = global_input_list; p_input != (INPUT_LIST *) NULL; p_input = p_input->next)
		{
			p_input->input->wx = 0;
			p_input->input->wy = 0;
			p_input->input->x = p_input->input->x_old = p_input->input->ww / 2;
			p_input->input->y = p_input->input->y_old = p_input->input->wh / 2;
			p_input->input->valid = 0;
			p_input->input->vpxo = 0;
			p_input->input->vpyo = p_input->input->wh - p_input->input->vph;
			if (p_input->input->input_generator != NULL)
				(*(p_input->input->input_generator)) (p_input->input, 0);
			else
				update_input_neurons (p_input->input);
			update_input_image (p_input->input);
	#ifndef NO_INTERFACE
			glBindTexture (GL_TEXTURE_2D, (GLuint) p_input->input->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p_input->input->tfw, 
				p_input->input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, p_input->input->image);
			glutPostWindowRedisplay (p_input->input->win);
	#endif
		}
	}



	/*
	*********************************************************************************
	* Function: reinit_ouputs							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	reinit_ouputs (void)
	{
		OUTPUT_LIST *p_output;

		for (p_output = global_output_list; p_output != (OUTPUT_LIST *) NULL; p_output = p_output->next)
		{
			rand_output_image (p_output->output);
	#ifndef NO_INTERFACE
			glutPostWindowRedisplay (p_output->output->win);
	#endif
		}
	}



	/*
	*********************************************************************************
	* Function: rebuilt_network							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	rebuilt_network (void)
	{
		clear_network_memory ();
		reinit_inputs ();
		reinit_ouputs ();
		clear_command_list ();
		set_move (DISABLE);
		set_draw (DISABLE);
	}



	/*
	*********************************************************************************
	* Function: copy_neuron_outputs							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	copy_neuron_outputs (NEURON_LAYER *dest, NEURON_LAYER *source)
	{
		long num_neurons;
		long i;
		NEURON_OUTPUT value;
		
		num_neurons = get_num_neurons (dest->dimentions);
		for (i = 0; i < num_neurons; i++)
		{
			value = source->neuron_vector[i].output;
			if (source->output_type == dest->output_type)
				dest->neuron_vector[i].output = value;
			else
			{
				switch (dest->output_type)
				{
					case COLOR:
						switch (source->output_type)
						{
							case GREYSCALE:
							case BLACK_WHITE:
								dest->neuron_vector[i].output.ival = PIXEL (value.ival, value.ival, value.ival);

								break;
							case GREYSCALE_FLOAT:
								dest->neuron_vector[i].output.ival = PIXEL ((int) value.fval, (int) value.fval, (int) value.fval);
								break;
							default:
								dest->neuron_vector[i].output = value;
						}
						break;
					case GREYSCALE:
						switch (source->output_type)
						{
							case COLOR:
								dest->neuron_vector[i].output.ival = (int) ((float) (RED (value.ival) + GREEN (value.ival) + BLUE (value.ival)) / 3.0);
								break;
							case BLACK_WHITE:
								dest->neuron_vector[i].output = value;
								break;
							case GREYSCALE_FLOAT:
								dest->neuron_vector[i].output.ival = (int) value.fval;
								break;
							default:
								dest->neuron_vector[i].output = value;
						}
						break;
					case BLACK_WHITE:
						switch (source->output_type)
						{
							case COLOR:
								dest->neuron_vector[i].output.ival = (int) ((float) (RED (value.ival) + GREEN (value.ival) + BLUE (value.ival)) / 3.0);
								break;
							case GREYSCALE:
								dest->neuron_vector[i].output = value;
								break;
							case GREYSCALE_FLOAT:
								dest->neuron_vector[i].output.ival = (int) value.fval;
							default:
								dest->neuron_vector[i].output = value;
						}
						break;
					case GREYSCALE_FLOAT:
						switch (source->output_type)
						{
							case COLOR:
								dest->neuron_vector[i].output.fval = (float) (RED (value.ival) + GREEN (value.ival) + BLUE (value.ival)) / 3.0;
								break;
							case GREYSCALE:
							case BLACK_WHITE:
								dest->neuron_vector[i].output.fval = (float) value.ival;
								break;
							default:
								dest->neuron_vector[i].output = value;
						}
						break;
				}
			}
		}
	}


	/*
	*********************************************************************************
	* Function: same_dimentions							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	BOOLEAN
	same_dimentions (XY_PAIR dim1, XY_PAIR dim2)
	{
		if ((dim1.x == dim2.x) && (dim1.y == dim2.y))
			return (TRUE);
		else
			return (FALSE);
	}



	/*
	*********************************************************************************
	* Function: copy_input_or_output						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	copy_input_or_output (char *name)
	{
		INPUT_DESC *copied_input;
		OUTPUT_DESC *copied_output;

		if ((copied_input = get_input_by_name (name)) != NULL)
			copied_data = copied_input->neuron_layer;
		else if ((copied_output = get_output_by_name (name)) != NULL)
			copied_data = copied_output->neuron_layer;
		else
			Erro ("Could not find valid input or output in copy_input_or_output ()", "", "");
	}



	/*
	*********************************************************************************
	* Function: paste_to_output							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/


	void 
	paste_to_output (char *name)
	{
		OUTPUT_DESC *output;
		NEURON_LAYER *dest;

		if (copied_data == NULL)
		{
			show_message ("There is no data to be pasted.", "Please select source", "");
			return;
		}
		if (get_input_by_name (name) != NULL)
		{
			show_message ("Cannot paste to input.", "", "");
			return;
		}
		if ((output = get_output_by_name (name)) != NULL)
			dest = output->neuron_layer;
		else
		{
			show_message ("Could not find valid output in paste_to_output () with the name: ", name, "");
			return;
		}
		if (!same_dimentions (copied_data->dimentions, dest->dimentions))
		{
			show_message ("Selected source and target have different dimentions.", "", "");
			return;
		}
		copy_neuron_outputs (dest, copied_data);
		update_output_image (output);
	#ifndef NO_INTERFACE
		glutPostWindowRedisplay (output->win);
	#endif
	}


	void 
	set_neuron_layer_max_min (NEURON_LAYER *neuron_layer)
	{
		int i, w, h;
		NEURON *nv;
		NEURON_OUTPUT max, min, value;
		
		h = neuron_layer->dimentions.y;
		w = neuron_layer->dimentions.x;
		nv = neuron_layer->neuron_vector;
		
		switch (neuron_layer->output_type)
		{
			case BLACK_WHITE:
			case GREYSCALE:
				max = min = nv[0].output;
		
				for (i = 0; i < w * h; i++)
				{
					value = nv[i].output;
			
					if (value.ival > max.ival)
						max = value;
					else if (value.ival < min.ival)
						min = value;
				}
		
				neuron_layer->max_neuron_output = max;
				neuron_layer->min_neuron_output = min;
				break;
			case GREYSCALE_FLOAT:
				max.fval = FLT_MIN;
				min.fval = FLT_MAX;
		
				for (i = 0; i < w * h; i++)
				{
					value = nv[i].output;
			
					if (value.fval > max.fval)
						max = value;
					else if (value.fval < min.fval)
						min = value;
				}
		
				neuron_layer->max_neuron_output = max;
				neuron_layer->min_neuron_output = min;
				break;
		}
	}


	/*
	*********************************************************************************
	* Function: save_object_image							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	save_object_image (char *file_name, char *object_name)
	{
		INPUT_DESC *saved_input;
		OUTPUT_DESC *saved_output;
		FILE *file_dest = (FILE *) NULL;
		NEURON_LAYER *neuron_layer_saved = (NEURON_LAYER *) NULL, *saved_neuron_layer;
		NEURON *neuron_vector;	

		int w, h;
		int i, j;
		float max_fval;
		float min_fval;
			
		if ((saved_input = get_input_by_name (object_name)) != NULL)
			neuron_layer_saved = saved_input->neuron_layer;
		else if ((saved_output = get_output_by_name (object_name)) != NULL)
			neuron_layer_saved = saved_output->neuron_layer;
		else if ((saved_neuron_layer = get_neuron_layer_by_name (object_name)) != NULL)
			neuron_layer_saved = saved_neuron_layer;
		else
		{
			show_message ("Could not find valid input, output or neuron_layer \nin save_object_image()", "", "");
			return;
		}	
		
		if ((file_dest = fopen (file_name, "w")) == NULL)
		{
			show_message ("Could not open file ", file_name, " to write");
			return;
		}	

		w = neuron_layer_saved->dimentions.x;
		h = neuron_layer_saved->dimentions.y;
		neuron_vector = neuron_layer_saved->neuron_vector;

		switch (neuron_layer_saved->output_type)
		{
			case COLOR:
				fprintf (file_dest, "P3\n#MAE Image\n");
				fprintf (file_dest, "%d %d\n", w, h); 
				fprintf (file_dest, "255\n");
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{
						fprintf (file_dest, "%d %d %d\t", (int) RED(neuron_vector[i + j * w].output.ival), (int) GREEN(neuron_vector[i + j * w].output.ival),(int) BLUE(neuron_vector[i + j * w].output.ival));
					}
					fprintf (file_dest, "\n"); 
				}
				break;
			case GREYSCALE:
				set_neuron_layer_max_min (neuron_layer_saved);
				min_fval = (float) neuron_layer_saved->min_neuron_output.ival;
				max_fval = (float) neuron_layer_saved->max_neuron_output.ival;
				fprintf (file_dest, "P2\n#MAE Image max = %f, min = %f\n", max_fval, min_fval);
				fprintf (file_dest, "%d %d\n", w, h); 
				fprintf (file_dest, "255\n");
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{
						fprintf (file_dest, "%d ",4*neuron_vector[i + j * w].output.ival);
						//fprintf (file_dest, "%f ", 255.0 * ((float) neuron_vector[i + j * w].output.ival - min_fval) / (max_fval - min_fval));
					} 
					fprintf (file_dest, "\n"); 
				}
				break;
			case GREYSCALE_FLOAT:
				set_neuron_layer_max_min (neuron_layer_saved);
				min_fval = neuron_layer_saved->min_neuron_output.fval;
				max_fval = neuron_layer_saved->max_neuron_output.fval;
				fprintf (file_dest, "P2\n#MAE Image max = %f, min = %f\n", max_fval, min_fval);
				fprintf (file_dest, "%d %d\n", w, h); 
				fprintf (file_dest, "255\n");
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{	
						fprintf (file_dest, "%f ", 255.0 * (neuron_vector[i + j * w].output.fval - min_fval) / (max_fval - min_fval));
					}
					fprintf (file_dest, "\n"); 
				}
				break;
			default:
				set_neuron_layer_max_min (neuron_layer_saved);
				min_fval = (float) neuron_layer_saved->min_neuron_output.ival;
				max_fval = (float) neuron_layer_saved->max_neuron_output.ival;
				fprintf (file_dest, "P2\n#MAE Image max = %f, min = %f\n", max_fval, min_fval);
				fprintf (file_dest, "%d %d\n", w, h); 
				fprintf (file_dest, "255\n");
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)	
					{
						fprintf (file_dest, "%f ", 255.0 * ((float) neuron_vector[i + j * w].output.ival - min_fval) / (max_fval - min_fval)); 
					}
					fprintf (file_dest, "\n"); 
				}
				break;
		}
		fclose (file_dest);
	}



	int
	get_pnm_header (int *w, int *h, int *max_range, FILE *image_file)
	{
		int i;
		char header[4];
		char character = 0;
		int image_type;

		// Gets the image format	
		for (i = 0; i < 4; i++)
			header[i] = '\0';
		MAE_FREAD_WRAPPER(fread(header, 2, 1, image_file),"get_pnm_header");

		if (strcmp (header, "P3") == 0)
			image_type = P3;
		else if (strcmp (header, "P6") == 0)
			image_type = P6;
		else
			image_type = -1;

		while (character != '\n')
			character = fgetc (image_file);

		if ((character = fgetc (image_file)) == '#')
		{
			while (character != '\n')
				character = fgetc (image_file);
		}
		else
			ungetc (character, image_file);

		// Ignores the image dimentions
		MAE_FSCANF_WRAPPER(fscanf (image_file, "%d %d\n", w, h),"get_pnm_header");

		// Ignores the pixels value range
		MAE_FSCANF_WRAPPER(fscanf (image_file, "%d\n", max_range),"get_pnm_header");
		
		return (image_type);
	}


	/*
	*********************************************************************************
	* Function: load_neuron_layer_from_file						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	load_neuron_layer_from_file (NEURON_LAYER *neuron_layer, char *file_name)
	{
		FILE *file_origin;
		NEURON *neuron_vector;
		int i, j, w, h, r, g, b;
		int w_not_used, h_not_used, max_range_not_used;
		char buffer[256];
		float max_fval;
		float min_fval;
		float fval;
		int image_type;
		
		neuron_vector = neuron_layer->neuron_vector;
		w = neuron_layer->dimentions.x;
		h = neuron_layer->dimentions.y;
		file_origin = fopen (file_name, "r");

		switch (neuron_layer->output_type)
		{
			case COLOR:
				image_type = get_pnm_header (&w_not_used, &h_not_used, &max_range_not_used, file_origin);
				if (image_type == P3)
				{
					for (j = h - 1; j >= 0; j--)
					{
						for (i = 0; i < w; i++)
						{
							MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d %d %d", &(r), &(g), &(b)), "load_neuron_layer_from_file");
							neuron_vector[i + j * w].output.ival = PIXEL(r, g, b);
						}
					}
				} 
				else if (image_type == P6)
				{
					for (j = h - 1; j >= 0; j--)
					{
						for (i = 0; i < w; i++)
						{
							r = fgetc (file_origin);
							g = fgetc (file_origin);
							b = fgetc (file_origin);
							
							neuron_vector[i + j * w].output.ival = PIXEL(r, g, b);
						}
					}
				}
				else
					show_message (file_name, " must be of type P3 or P6.", "");	
				break;
			case GREYSCALE:
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%s\n", buffer),"load_neuron_layer_from_file");
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "#MAE Image max = %f, min = %f\n", &max_fval, &min_fval), "load_neuron_layer_from_file");

				// Ignores the image dimentions
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d %d\n", &w_not_used, &h_not_used),"load_neuron_layer_from_file");

				// Ignores the pixels value range
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d\n", &max_range_not_used),"load_neuron_layer_from_file");
		
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{
						MAE_FSCANF_WRAPPER(fscanf (file_origin, "%f", &fval),"load_neuron_layer_from_file");
						neuron_vector[i + j * w].output.ival = (int) ((fval / 255.0) * (max_fval - min_fval) + min_fval);
					}
				}
				break;
			case GREYSCALE_FLOAT:
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%s\n", buffer),"load_neuron_layer_from_file");
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "#MAE Image max = %f, min = %f\n", &max_fval, &min_fval),"load_neuron_layer_from_file");

				// Ignores the image dimentions
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d %d\n", &w_not_used, &h_not_used),"load_neuron_layer_from_file");

				// Ignores the pixels value range
				MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d\n", &max_range_not_used),"load_neuron_layer_from_file");
		
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{
						MAE_FSCANF_WRAPPER(fscanf (file_origin, "%f", &fval),"load_neuron_layer_from_file");
						neuron_vector[i + j * w].output.fval = (fval / 255.0) * (max_fval - min_fval) + min_fval;
					}
				}
				break;
			default:
				get_pnm_header (&w_not_used, &h_not_used, &max_range_not_used, file_origin);
				for (j = h - 1; j >= 0; j--)
				{
					for (i = 0; i < w; i++)
					{
						MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d", &(neuron_vector[i + j * w].output.ival)),"load_neuron_layer_from_file");
					}
				}
				break;
		}
		fclose (file_origin);
	}



	int
	image_size_ok (NEURON_LAYER *neuron_layer, FILE *file_origin, char *name)
	{
		int w, h;
		char header[256];
		char character = 0;
		int max_range;
		
		MAE_FSCANF_WRAPPER(fscanf (file_origin, "%s", header),"image_size_ok");

		// Discards any comments in the begin of the image file
		while (character != '\n')
			character = fgetc (file_origin);

		if ((character = fgetc (file_origin)) == '#')
		{
			while (character != '\n')
				character = fgetc (file_origin);
		}
		else
			ungetc (character, file_origin);

		MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d %d\n", &(w), &(h)),"image_size_ok");

		// Ignores the pixels value range
		MAE_FSCANF_WRAPPER(fscanf (file_origin, "%d\n", &(max_range)),"image_size_ok");
		
		if ((neuron_layer->dimentions.x != w) || (neuron_layer->dimentions.y != h))
		{
			show_message (name, " dimentions aren't good to load image.", "");
			return (0);
		}
		return (1);
	}



	/*
	*********************************************************************************
	* Function: load_output_from_image						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	load_output_from_image (OUTPUT_DESC *loaded_output, char *file_name)
	{
		FILE *file_origin = (FILE *) NULL;

		if ((file_origin = fopen (file_name, "r")) == NULL)
		{
			show_message ("Could not open file ", file_name, " to read");
			return;
		}
		if (!image_size_ok (loaded_output->neuron_layer, file_origin, loaded_output->name))
		{
			show_message ("Could not open file ", file_name, " - image size does not match.");
			return;
		}
		fclose (file_origin);
			
		load_neuron_layer_from_file (loaded_output->neuron_layer, file_name);
		
		update_output_image (loaded_output);
	}



	/*
	*********************************************************************************
	* Function: load_input_from_image						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	load_input_from_image (INPUT_DESC *loaded_input, char *file_name)
	{
		FILE *file_origin = (FILE *) NULL;

		if ((file_origin = fopen (file_name, "r")) == NULL)
		{
			show_message ("Could not open file ", file_name, " to read");
			return;
		}
		if (!image_size_ok (loaded_input->neuron_layer, file_origin, loaded_input->name))
			return;
			
		fclose (file_origin);

		load_input_image (loaded_input, file_name);
		
		update_input_image (loaded_input);
	}



	/*
	*********************************************************************************
	* Function: load_image_to_object						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void 
	load_image_to_object (char *object_name, char *file_name)
	{
		INPUT_DESC *loaded_input;
		OUTPUT_DESC *loaded_output;
		NEURON_LAYER *loaded_neuron_layer;	
		
		if ((loaded_input = get_input_by_name (object_name)) != NULL)
			load_input_from_image (loaded_input, file_name);
		else if ((loaded_output = get_output_by_name (object_name)) != NULL)
			load_output_from_image (loaded_output, file_name);
		else if ((loaded_neuron_layer = get_neuron_layer_by_name (object_name)) != NULL)
			load_neuron_layer_from_file (loaded_neuron_layer, file_name);
		else
		{
			show_message ("Could not find valid input, output or neuron_layer \nin load_image_to_object()", "", "");
			return;
		}
	}



	/*
	*********************************************************************************
	* Function: init_interpreter_pipe						*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	init_interpreter_pipe ()
	{
		if (pipe (interpreter_pipe) != 0)
			Erro ("Could not create pipe in init_interpreter_pipe ().", "", "");
		if ((interpreter_pipe_in = fdopen (interpreter_pipe[0], "r")) == NULL)
			Erro ("Could not open input i/o stream in init_interpreter_pipe ().", "", "");
		yyin = interpreter_pipe_in;
		if ((interpreter_pipe_out = fdopen (interpreter_pipe[1], "w")) == NULL)
			Erro ("Could not open output i/o stream in init_interpreter_pipe ().", "", "");
	}



	/*
	*********************************************************************************
	* Function: add_to_command_list							*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	char *
	add_to_command_list (char *command)
	{
		if (command_list == NULL)
		{
			command_list = (COMMAND_LIST *) alloc_mem (sizeof (COMMAND_LIST));
			command_list->command = (char *) alloc_mem (strlen (command) + 1);
			strcpy (command_list->command, command);
			command_list->next = NULL;
			command_list_tail = command_list;
		}
		else
		{
			command_list_tail->next = (COMMAND_LIST *) alloc_mem (sizeof (COMMAND_LIST));
			command_list_tail->next->command = (char *) alloc_mem (strlen (command) + 1);
			strcpy (command_list_tail->next->command, command);
			command_list_tail->next->next = NULL;
			command_list_tail = command_list_tail->next;
		}
		return (command_list_tail->command);
	}



	/*
	*********************************************************************************
	* Function: interprete_many_commands_in_a_file					*
	* Description: 									*
	* Inputs: 									*
	*********************************************************************************
	*/

	void
	interprete_many_commands_in_a_file (char *commands, char *file_name)
	{
		char *cmd;
		FILE *command_file_read;
		FILE *aux;

		cmd = add_to_command_list (commands);
		addto_browser (cmd);	
		

		if ((command_file_read = fopen (file_name, "r")) == NULL)
		{
			show_message ("Could not reopen temporary file to interpret command", "", "");
			return;
		}

	    aux = yyin;
		yyin = command_file_read;
		
		yyparse ();

	    yyin = aux;
		fclose (command_file_read);
	}



	/*
	*********************************************************************************
	* Function: interpreter								*
	* Description: interpret the language of interaction with the user		*
	* Inputs: command								*
	*********************************************************************************
	*/

	void
	interpreter (char *command)
	{	char *cmd;
		
		cmd = add_to_command_list (command);
		addto_browser (cmd);
		
		init_interpreter_pipe ();

		fprintf (interpreter_pipe_out, "%s", cmd);

		fclose (interpreter_pipe_out);
		yyparse ();
		fclose (interpreter_pipe_in);
	}



	/*
	*********************************************************************************
	* Function: callback_timer							*
	* Description: callback function that is invoked when an time event occurs	*
	* Inputs: irrelevant								*
	*********************************************************************************
	*/

	#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	void callback_timer (FL_OBJECT *ob, long value)
	#else
	void callback_timer (long value)
	#endif
	{
		running = 1;
		all_inputs_update ();
		all_filters_update ();
		all_dendrites_update (); 
		all_neurons_update ();
		all_outputs_update ();
		running = 0;

		if (remaining_steps-- <= 0)
			remaining_steps = 0;
	#ifndef NO_INTERFACE
		else
			glutTimerFunc (delay,(void (* ) (int)) callback_timer, 0);
	#endif
	}



	/*
	*********************************************************************************
	* Function: evaluate_output_rate						*
	* Description: evaluate the output rate of the running network by changing the  *
	* delay time									*
	* Inputs: none									*
	*********************************************************************************
	*/
		
	void
	evaluate_output_rate (void)
	{
	#if !(defined(NO_INTERFACE) || defined(WINDOWS))
		if (hide_interface == FALSE)
			delay = (unsigned int) 2000.0 * (1.005 - fl_get_slider_value (speed_slider));
	#endif
	}



	/*
	*********************************************************************************
	* Function: run_network								*
	* Description: run the network during a fixed steps number			*
	* Inputs: the steps number							*
	*********************************************************************************
	*/

	void
	run_network (int run_steps)
	{
		remaining_steps += run_steps;
		evaluate_output_rate ();
	#ifndef NO_INTERFACE
		glutTimerFunc (delay,(void (* ) (int)) callback_timer, 0);
	#endif
	}


	/*
	*********************************************************************************
	* Function: run_network_locked							*
	* Description: run the network during a fixed steps number			*
	* Inputs: the steps number							*
	*********************************************************************************
	*/

	void
	run_network_locked (int run_steps)
	{
		running = 1;
		for (remaining_steps += run_steps; remaining_steps > 0; remaining_steps--)
		{
			all_inputs_update ();
			all_filters_update ();
			all_dendrites_update (); 
			all_neurons_update ();
			all_outputs_update ();
		}
		running = 0;
	}


	/*
	*********************************************************************************
	* Function: move_input_window							*
	* Description: move the gaze position in the input window			*
	* Inputs: input name, new gaze coordinates (wx,wy) in the input image 		*
	*********************************************************************************
	*/

	void 
	move_input_window (char *name, int wx, int wy)
	{
		INPUT_DESC *input;
		char message[256];
		
		if ((input = get_input_by_name (name)) == NULL)
		{
			show_message ("Input ", name, "not found.");
			return;
		}	

		input->wxd = wx;
		input->wyd = wy;

		switch(TYPE_MOVING_FRAME)
		{
			case MOVING_CENTER:
				input->wx = wx - (input->neuron_layer->dimentions.x / 2);
				input->wy = wy - (input->neuron_layer->dimentions.y / 2); 
				break;
			case STOP:
				if (((wx >= (input->wx + input->neuron_layer->dimentions.x)) ||  (wx < input->wx))
				&&  (wx >= 0) && (wx < input->vpw))
					input->wx = wx - (input->neuron_layer->dimentions.x / 2);
				if (((wy >= (input->wy + input->neuron_layer->dimentions.y)) ||  (wy < input->wy))
				&&  (wy >= 0) && (wx < input->vph))
					input->wy = wy - (input->neuron_layer->dimentions.y / 2);
				break;
			default:
				sprintf(message,"%d. It can be MOVE_CENTER or STOP.",TYPE_MOVING_FRAME);
				Erro ("Invalid Type Moving Frame: ", message, " Error in move_input_window.");
				return;
				
		}

		if (input->input_generator != NULL)
			(*(input->input_generator)) (input, MOVE);
		else
		{
			check_input_bounds (input, wx, wy);
			update_input_neurons (input);
	#ifndef NO_INTERFACE
			glutPostWindowRedisplay (input->win);
	#endif
		}
	}



	/*
	*********************************************************************************
	* Function: rand_output								*
	* Description: 									*
	* Inputs:  									*
	*********************************************************************************
	*/

	void 
	rand_output (char *name)
	{
		OUTPUT_DESC *output;
			
		if ((output = get_output_by_name (name)) != NULL)
		{
			rand_output_image (output);
	#ifndef NO_INTERFACE
			glutPostWindowRedisplay (output->win);
	#endif
		}
		else
		{
			show_message ("Could not find valid output in rand_output () with the name: ", name, "");
			return;
		}
	}



	/*
	*********************************************************************************
	* Function: push_input_position							*
	* Description: 									*
	* Inputs:  									*
	*********************************************************************************
	*/

	void
	push_input_position (char *input_name)
	{
		INPUT_DESC *input;
		
		input = get_input_by_name (input_name);
		wx_saved = input->wx;
		wy_saved = input->wy;
		x_saved = input->x;
		y_saved = input->y;
	}



	/*
	*********************************************************************************
	* Function: pop_input_position							*
	* Description: 									*
	* Inputs:  									*
	*********************************************************************************
	*/

	void
	pop_input_position (char *input_name)
	{
		INPUT_DESC *input;
		
		input = get_input_by_name (input_name);
		input->wx = wx_saved;
		input->wy = wy_saved;
		input->x = x_saved;
		input->y = y_saved;
		if (input->input_generator != NULL)
			(*(input->input_generator)) (input, 0);
		else
			update_input_neurons (input);
	#ifndef NO_INTERFACE
		glutPostWindowRedisplay (input->win);
	#endif
	}



	/*
	*********************************************************************************
	* Function: project_synapses							*
	* Description: 									*
	* Inputs:  									*
	*********************************************************************************
	*/

	void
	project_synapses (NEURON_LAYER *neuron_layer_dst, long neuron, SENSITIVITY_TYPE neuron_layer_sensitivity)
	{
		SYNAPSE_LIST *s_list;
		
		switch(neuron_layer_dst->output_type)
		{
			case GREYSCALE:
				neuron_layer_dst->neuron_vector[neuron].output.ival = NUM_GREYLEVELS - 1;
				break;
			case BLACK_WHITE:
				neuron_layer_dst->neuron_vector[neuron].output.ival = NUM_GREYLEVELS - 1;
				break;
			case GREYSCALE_FLOAT:
				neuron_layer_dst->neuron_vector[neuron].output.fval = (float) (NUM_GREYLEVELS - 1);
				break;
			case COLOR:
				neuron_layer_dst->neuron_vector[neuron].output.ival = NUM_COLORS - 1;
				break;
			case COLOR_SELECTIVE:
				neuron_layer_dst->neuron_vector[neuron].output.ival = NUM_COLORS - 1;	//Higlight the active neuron as in the color neurons
				break;
		}
				
		for (s_list = ((SYNAPSE_LIST *) (neuron_layer_dst->neuron_vector[neuron].synapses)); 
		     s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
		{

			switch(neuron_layer_sensitivity)
			{
				case GREYSCALE:
					s_list->synapse->source->output.ival = s_list->synapse->sensitivity.ival;
					break;
				case BLACK_WHITE:
					s_list->synapse->source->output.ival = (s_list->synapse->sensitivity.ival) ? NUM_GREYLEVELS - 1: 0;
					break;
				case GREYSCALE_FLOAT:
					s_list->synapse->source->output.fval = s_list->synapse->sensitivity.fval;
					break;
				case COLOR:
					s_list->synapse->source->output.ival = s_list->synapse->sensitivity.ival;
	//				s_list->synapse->source->output.ival = NUM_GREYLEVELS - 1;
					break;
				case COLOR_SELECTIVE:
					s_list->synapse->source->output.ival = s_list->synapse->sensitivity.ival;
					#if 0
					switch(s_list->synapse->sensitivity.ival)
					{
						#if 0
						case(RED_SENSITIVE):
							s_list->synapse->source->output.ival = 0x000000ffL; //highest RED
							break;
						case(GREEN_SENSITIVE):
							s_list->synapse->source->output.ival = 0x0000ff00L; //highest GREEN
							break;
						case(BLUE_SENSITIVE):
							s_list->synapse->source->output.ival = 0x00ff0000L; //highest BLUE
							break;
						default:
							s_list->synapse->source->output.ival = 0x0; //Unknown sensitivity
							break;
						#endif
						
						case 0:	// RED
							s_list->synapse->source->output.ival = 0x000000ffL;
							break;
						case 1:	// GREEN
							s_list->synapse->source->output.ival = 0x0000ff00L;
							break;
						case 2:	// BLUE
							s_list->synapse->source->output.ival = 0x00ff0000L;
							break;
						case 3:	// RED + GREEN (YELLOW)
							s_list->synapse->source->output.ival = 0x000000ffL | 0x0000ff00L;
							break;
						case 4:	// RED + BLUE ()
							s_list->synapse->source->output.ival =  0x000000ffL | 0x00ff0000L;
							break;
						case 5:	// BLUE + GREEN ()
							s_list->synapse->source->output.ival = 0x00ff0000L | 0x0000ff00L;
							break;
						case 6:	// RED + GREEN + BLUE (GREYSCALE)
							s_list->synapse->source->output.ival = 0x00ffffffL;
							break;
						default:// RED + GREEN + BLUE (GREYSCALE)
							s_list->synapse->source->output.ival  = 0x00ffffffL;
							break;
						
					}
					#endif
					break;
			}
		}
	}


	/*
	*********************************************************************************
	* Function: test_layer								*
	* Description: test connections of the neuron layer 				*
	* Inputs: none 									*
	*********************************************************************************
	*/

	void 
	test_layer ()
	{
		long num_neurons;
		OUTPUT_DESC *output;
		OUTPUT_LIST *out_l;

		output = get_output_by_name (test_output_name);
		if (output == NULL)
		{
			show_message ("Could not find output named ", test_output_name, "");
	#ifndef NO_INTERFACE
			glutIdleFunc ((void (* ) (void)) check_forms);
	#endif
			return;
		}
		
		num_neurons = get_num_neurons (output->neuron_layer->dimentions);
	/*	if (test_neuron == num_neurons) // / 2 - output->neuron_layer->dimentions.x / 2)
		{
			glutIdleFunc ((void (* ) (void)) check_forms);
			return;
		}
	*/
		for (out_l = global_output_list; out_l != (OUTPUT_LIST *) NULL; out_l = out_l->next)
			clear_output_by_name (out_l->output->name);

		if (test_neuron < num_neurons)
		{
			project_synapses (output->neuron_layer, 
					  test_neuron, output->neuron_layer->sensitivity);
			for (out_l = global_output_list; out_l != (OUTPUT_LIST *) NULL; out_l = out_l->next)
			{
				update_output_image (out_l->output);
	#ifndef NO_INTERFACE
				glutSetWindow (out_l->output->win);
				glutPostRedisplay ();
	#endif
			}
			test_neuron++;
			check_forms ();
		}
	#ifndef NO_INTERFACE
		else
			glutIdleFunc ((void (* ) (void)) check_forms);
	#endif
	}



	/*
	*********************************************************************************
	* Function: test_layer_v							*
	* Description: test connections of the neuron layer in a upper vertical pattern	*
	* Inputs: none 									*
	*********************************************************************************
	*/

	void 
	test_layer_v ()
	{
		// Test neuron shouldn't be redefined here
		long horizontal_dimention, vertical_dimention, num_neurons;
		OUTPUT_DESC *output;
		OUTPUT_LIST *out_l;

		output = get_output_by_name (test_output_name);
		if (output == NULL)
		{
			show_message ("Could not find output named ", test_output_name, "");
	#ifndef NO_INTERFACE
			glutIdleFunc ((void (* ) (void)) check_forms);
	#endif
			return;
		}
		
		num_neurons = get_num_neurons (output->neuron_layer->dimentions);
		horizontal_dimention = output->neuron_layer->dimentions.x;
		vertical_dimention = output->neuron_layer->dimentions.y;

		for (out_l = global_output_list; out_l != (OUTPUT_LIST *) NULL; out_l = out_l->next)
			clear_output_by_name (out_l->output->name);

		if (neuron_counter < num_neurons)
		{
			project_synapses (output->neuron_layer,test_neuron, output->neuron_layer->sensitivity);

			for (out_l = global_output_list; out_l != (OUTPUT_LIST *) NULL; out_l = out_l->next)
			{

				update_output_image (out_l->output);
	#ifndef NO_INTERFACE
				glutSetWindow (out_l->output->win);
				glutPostRedisplay ();
	#endif
			}
			
			if(vertical_neuron_counter == vertical_dimention - 1)
			{
				column_counter++;
				test_neuron = column_counter;
				vertical_neuron_counter = 0;
			}
			else
			{
				test_neuron += horizontal_dimention;
				vertical_neuron_counter++;
			}

			neuron_counter++;
			check_forms ();
		}
	#ifndef NO_INTERFACE
		else
			glutIdleFunc ((void (* ) (void)) check_forms);
	#endif
	}




/*
*********************************************************************************
* Function: test_connections							*
* Description: test connections of the output's neuron layer			*
* Inputs: output name 								*
*********************************************************************************
*/

void
test_connections (char *output_name)
{
	strcpy (test_output_name, output_name);
	test_neuron = 0;
#ifndef NO_INTERFACE
	glutIdleFunc ((void (* ) (void)) test_layer);
#endif
}


/*
*********************************************************************************
* Function: test_connections_v							*
* Description: same as test_connections but in a upper vertical direction	*
* Inputs: output name 								*
*********************************************************************************
*/

void
test_connections_v (char *output_name)
{
	strcpy (test_output_name, output_name);
	column_counter = 0;
	vertical_neuron_counter = 0;
	neuron_counter = 0;
	test_neuron = 0;
#ifndef NO_INTERFACE
	glutIdleFunc ((void (* ) (void)) test_layer_v);
#endif
}



/*
*********************************************************************************
* Function: wait_display							*
* Description: wait until an event occur					*
* Inputs: none									*
*********************************************************************************
*/

void
wait_display (void)
{
#ifndef NO_INTERFACE	
	glFlush ();
#ifndef WINDOWS	

	glXWaitGL ();
	glXWaitX();
#endif
#endif
	check_forms ();
}



/*
*********************************************************************************
* Function: show_message							*
* Description: display a message box for the user				*
* Inputs: three string messages 						*
*********************************************************************************
*/

void
show_message (char *msg1, char *msg2, char *msg3)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_show_message (msg1, msg2, msg3);
#endif
}



/*
*********************************************************************************
* Function: set_status_bar							*
* Description: set the status bar content					*
* Inputs: status info in the string format 					*
*********************************************************************************
*/

void
set_status_bar (char *status_info)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_object_label (status_bar, status_info);
#endif
}



/*
*********************************************************************************
* Function: set_mouse_bar							*
* Description: set the mouse bar content					*
* Inputs: mouse position already in the string format 				*
*********************************************************************************
*/

void
set_mouse_bar (char *mouse_pos)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_object_label (mouse_bar, mouse_pos);
#endif
}



/*
*********************************************************************************
* Function: set_color_bar							*
* Description: set the color bar content					*
* Inputs: color value already in the string format 				*
*********************************************************************************
*/

void
set_color_bar (char *color_val)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_object_label (color_bar, color_val);
#endif
}



/*
*********************************************************************************
* Function: addto_browser							*
* Description: add a command to the command log browser				*
* Inputs: command's string 							*
*********************************************************************************
*/
	
void
addto_browser (char *cmd)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_addto_browser (log_browser, cmd);
#endif
}



/*
*********************************************************************************
* Function: clear_browser							*
* Description: clear the command log browser					*
* Inputs: none									*
*********************************************************************************
*/

void
clear_browser (void)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_clear_browser (log_browser);
#endif
}



/*
*********************************************************************************
* Function: check_forms								*
* Description: glut idlle function						*
* Inputs: none									*
*********************************************************************************
*/

void
check_forms (void)
{
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_check_forms ();
#endif
}



/*
*********************************************************************************
* Function: toggle_move_active							*
* Description: toggle the state of the global variable move_active		*
* Inputs: none									*
*********************************************************************************
*/

void
toggle_move_active (void)
{
	move_active = !move_active;
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_button (move_button, move_active);
#endif
}



/*
*********************************************************************************
* Function: toggle_draw_active							*
* Description: toggle the state of the global variable draw_active		*
* Inputs: none									*
*********************************************************************************
*/

void
toggle_draw_active (void)
{
	draw_active = !draw_active;
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_button (draw_button, draw_active);
#endif	
}



/*
*********************************************************************************
* Function: set_move								*
* Description: set the state of the global variable move_active			*
* Inputs: button status can be ENABLE or DISABLE				*
*********************************************************************************
*/

void
set_move (int status)
{
	move_active = status;
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_button (move_button, move_active);
#endif
}



/*
*********************************************************************************
* Function: set_draw								*
* Description: set the state of the global variable draw_active			*
* Inputs: button status can be ENABLE or DISABLE				*
*********************************************************************************
*/

void
set_draw (int status)
{
	draw_active = status;
#if !(defined(NO_INTERFACE) || defined(WINDOWS))
	if (hide_interface == FALSE)
		fl_set_button (draw_button, draw_active);
#endif	
}



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_build_button (FL_OBJECT *ob, long data)
{
  	if (fl_show_question ("Are you shure that you want to rebuilt the network?", 0))
		rebuilt_network ();
}
#endif



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_run_button (FL_OBJECT *ob, long data)
{
	char command[256];
	const char *char_steps;
	
	char_steps = fl_get_input (steps_input);
	strcpy (command, "run ");
	strcat (command, char_steps);
	strcat (command, ";");
	interpreter (command);
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_speed_slider (FL_OBJECT *ob, long data)
{
  	evaluate_output_rate ();
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_steps_input (FL_OBJECT *ob, long data)
{
	const char *steps_string;
	
	steps_string = fl_get_input (steps_input);
	remaining_steps = num_steps = atoi (steps_string);
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_command_input (FL_OBJECT *ob, long data)
{
	command_string = fl_get_input (command_input);
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_exec_button (FL_OBJECT *ob, long data)
{
	command_string = fl_get_input (command_input);
	interpreter ((char *) command_string);
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_move_button (FL_OBJECT *ob, long data)
{
	interpreter ("toggle move_active;");
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_draw_button (FL_OBJECT *ob, long data)
{
	interpreter ("toggle draw_active;");
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_undo_button (FL_OBJECT *ob, long data)
{
	COMMAND_LIST *cmd_list;
	COMMAND_LIST *old_cmd_list;
	COMMAND_LIST *aux;
	
	if (command_list == NULL)
	{
		return;
	}

	clear_network_memory ();
	reinit_inputs ();
	reinit_ouputs ();
	fl_clear_browser (log_browser);
	fl_check_forms ();

	if (command_list == command_list_tail) /* there is a single command in the list */
	{
		free (command_list->command);
		free (command_list);
		command_list = command_list_tail = NULL;
	}
	else
	{
		old_cmd_list = command_list;
		command_list = NULL;
		command_list_tail = NULL;
		for (cmd_list = old_cmd_list; cmd_list->next != NULL; cmd_list = cmd_list->next)
		{
			glFinish ();
			interpreter (cmd_list->command);
		}

		for (cmd_list = old_cmd_list; cmd_list != NULL; cmd_list = aux)
		{
			aux = cmd_list->next;
			free (cmd_list->command);
			free (cmd_list);
		}
	}
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_clear_button (FL_OBJECT *ob, long data)
{
	char command[256];
	
	if (selected_input != NULL)
	{
		show_message ("Cannot clear input.", "", "");
		return;
	}
	if (selected_output != NULL)
	{
		strcpy (command, "clear ");
		strcat (command, selected_output->name);
		strcat (command, ";");
		interpreter (command);
	}
	else
	{
		show_message ("There is no target selected.", "Please select target", "");
		return;
	}
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_rand_button (FL_OBJECT *ob, long data)
{
	char command[256];
	
	if (selected_input != NULL)
	{
		show_message ("Cannot rand input.", "", "");
		return;
	}
	if (selected_output != NULL)
	{
		strcpy (command, "rand ");
		strcat (command, selected_output->name);
		strcat (command, ";");
		interpreter (command);
	}
	else
	{
		show_message ("There is no target selected.", "Please select target", "");
		return;
	}
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_copy_button (FL_OBJECT *ob, long data)
{
	char command[256];

	if (selected_input != NULL)
	{
		strcpy (command, "copy ");
		strcat (command, selected_input->name);
		strcat (command, ";");
		interpreter (command);
	}
	else if (selected_output != NULL)
	{
		strcpy (command, "copy ");
		strcat (command, selected_output->name);
		strcat (command, ";");
		interpreter (command);
	}
	else
		show_message ("Select input or output you want to copy.", "", "");
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_paste_button (FL_OBJECT *ob, long data)
{
	char command[256];
	
	if (selected_input != NULL)
	{
		show_message ("Cannot paste to input.", "", "");
		return;
	}
	if (selected_output != NULL)
	{
		strcpy (command, "paste ");
		strcat (command, selected_output->name);
		strcat (command, ";");
		interpreter (command);
	}
	else
	{
		show_message ("There is no target selected.", "Please select target", "");
		return;
	}
}
#endif	



#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void 
callback_train_button (FL_OBJECT *ob, long data)
{
	interpreter ("train network;");
}
#endif		
