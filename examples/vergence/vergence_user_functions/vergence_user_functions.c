#include <stdlib.h>
#include "calibrate.h"
#include "filter.h"

#define	VERGENCE_DISP_ANGLE1	0.2
#define	VERGENCE_DISP_ANGLE2	1.0
#define	VERGENCE_DISP_ANGLE3	5.0

#define	MAX_VISUAL_ANGLE	60.0

#define	PERCENT_DIFFERENCE0	1
#define	PERCENT_DIFFERENCE1	10
#define	PERCENT_DIFFERENCE2	20
#define	PERCENT_DIFFERENCE3	40

#define	DISP_ANGLE(x) ((int) ((x * (double) i_left_eye->ww) / MAX_VISUAL_ANGLE))

#define	MINUS_FLAG 1
#define	TE_FLAG 2

#define TEST

int left_eye, right_eye;
 
int correction     = 0;
int current_kernel = 0;
int side_move      = 0;

int current_output;
int current_input;

enum _plot_type
{
	PLOT,
	SPLOT
};

typedef enum _plot_type PLOT_TYPE;

const char * const name_plot_type[] = {"plot", "splot"};

enum _plot_style
{
	LINES,
	POINTS,
	LINESPOINTS,
	IMPULSES,
 	DOTS 
};

typedef enum _plot_style PLOT_STYLE;

const char * const name_plot_style[] = {"lines", "points", "linespoints", "impulses", "dots"};

struct _gnuplot_parameters_desc
{
    PLOT_TYPE typePlot;
    char *data_filename;
    int x_field_position;
    int y_field_position;
    int z_field_position;
    char *title;
    PLOT_STYLE plot_style;
}; 
 
typedef struct _gnuplot_parameters_desc GNUPLOT_PARAMETERS_DESC; 

struct _gnuplot_parameters_list 
    { 
	GNUPLOT_PARAMETERS_DESC	*parameter; 
	struct _gnuplot_parameters_list *next; 
    }; 
 
typedef struct _gnuplot_parameters_list GNUPLOT_PARAMETERS_LIST; 

GNUPLOT_PARAMETERS_LIST *gnuplot_parameters_list;

void
free_gnuplot_parameters_list ()
{
	GNUPLOT_PARAMETERS_LIST *gp_list;

	for (;gnuplot_parameters_list != (GNUPLOT_PARAMETERS_LIST *) NULL;)
	{
		gp_list = gnuplot_parameters_list;
		gnuplot_parameters_list = gnuplot_parameters_list->next;
		free(gp_list->parameter->data_filename);
		free(gp_list->parameter->title);
		free(gp_list->parameter);
		free(gp_list);
	}
}



void
add_gnuplot_parameters_to_gnuplot_parameters_list (GNUPLOT_PARAMETERS_DESC *gnuplot_parameters)
{
	GNUPLOT_PARAMETERS_LIST *gp_list;

	if (gnuplot_parameters_list == ((GNUPLOT_PARAMETERS_LIST *) NULL))
	{
		gp_list = (GNUPLOT_PARAMETERS_LIST *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_LIST));
		gp_list->next = (GNUPLOT_PARAMETERS_LIST *) NULL;
		gp_list->parameter = gnuplot_parameters;
		gnuplot_parameters_list = gp_list;
	}
	else
	{
		for (gp_list = gnuplot_parameters_list; gp_list->next != (GNUPLOT_PARAMETERS_LIST *) NULL;
			gp_list = gp_list->next)
		{
			if (gp_list->parameter == gnuplot_parameters)
				Erro ("Two instances of the same gnuplot_parameters in gnuplot_parameters_list.", "", "");
		}
		if (gp_list->parameter == gnuplot_parameters)
				Erro ("Two instances of the same gnuplot_parameters in gnuplot_parameters_list.", "", "");
		gp_list->next = (GNUPLOT_PARAMETERS_LIST *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_LIST));
		gp_list->next->next = (GNUPLOT_PARAMETERS_LIST *) NULL;
		gp_list->next->parameter = gnuplot_parameters;
	}
}



GNUPLOT_PARAMETERS_DESC *
make_gnuplot_parameters (PLOT_TYPE typePlot, char *data_filename, int x_field_position, int y_field_position, int z_field_position, char *title, PLOT_STYLE plot_style)
{
	GNUPLOT_PARAMETERS_DESC *gnuplot_parameter;
	
	gnuplot_parameter = (GNUPLOT_PARAMETERS_DESC *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_DESC));
	gnuplot_parameter->typePlot = typePlot;
	gnuplot_parameter->data_filename = (char *) alloc_mem ((size_t) strlen(data_filename) + 1);
	sprintf(gnuplot_parameter->data_filename,"%s",data_filename);
	gnuplot_parameter->x_field_position = x_field_position;
	gnuplot_parameter->y_field_position = y_field_position;
	gnuplot_parameter->z_field_position = z_field_position;
	gnuplot_parameter->title = (char *) alloc_mem ((size_t) strlen(title) + 1);
	sprintf(gnuplot_parameter->title,"%s",title);
	gnuplot_parameter->plot_style = plot_style;
	
	return gnuplot_parameter;
}



void
plot_gnuplot_file (char *command_file)
{
	GNUPLOT_PARAMETERS_LIST *gp_list;
	FILE *dataToPlot, *gnuplot_file;
	PLOT_TYPE typePlot_ant = -1;
	
	char message[256];
	
	if ((gnuplot_file = fopen (command_file, "w")) == NULL)
	{
		sprintf(message,"Could not open file %s",command_file);
		Erro (message, "", "");
	}

	fprintf(gnuplot_file,"set mouse labels\n");

	for (gp_list = gnuplot_parameters_list; gp_list != (GNUPLOT_PARAMETERS_LIST *) NULL;)
	{
		if ((dataToPlot = fopen (gp_list->parameter->data_filename, "r")) == NULL)
		{
			sprintf(message,"Could not open file %s",gp_list->parameter->data_filename);
			Erro (message, "", "");
		}
		fclose(dataToPlot);
		if (typePlot_ant == gp_list->parameter->typePlot)
			if (gp_list->next == (GNUPLOT_PARAMETERS_LIST *) NULL || gp_list->parameter->typePlot != gp_list->next->parameter->typePlot)
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
				else
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d:%d title \'%s\' with %s\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
			else
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
		else
			if (gp_list->next == (GNUPLOT_PARAMETERS_LIST *) NULL || gp_list->parameter->typePlot != gp_list->next->parameter->typePlot)
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
			else
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
		typePlot_ant = gp_list->parameter->typePlot;
		gp_list = gp_list->next;
	}
	free_gnuplot_parameters_list();
	fprintf(gnuplot_file,"pause -1  \'\"press return\"\'\n"); 
	fclose(gnuplot_file);
	sprintf(message,"gnuplot %s",command_file);
	/*system(message);*/
}



init_user_functions ()
{
	return (0);
}



int
output_handler_get_current_output (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	if ((mouse_button == GLUT_LEFT_BUTTON) && (mouse_state == GLUT_DOWN))
		current_output = output->win;
		
	return (0);
}



void
print_output (int output_handle)
{
	OUTPUT_DESC *output;
	FILE *output_file;
	char filename[256];
	int i,j;
	int w,h;
	
	output = get_output_by_win (output_handle);
	sprintf (filename, "%s.gnuplot.dat",output->name);

	output_file = fopen(filename,"w");
	
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	for(i=0;i<w;i++)
		for(j=0;j<h;j++)
			fprintf(output_file, "%d %d %d\n",i, j, output->neuron_layer->neuron_vector[i*h+j].output.ival);
	
	fclose(output_file);
}



void
load_input_image (INPUT_DESC *input)
{
	glutSetWindow(input->win);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, input->ww, input->wh, GL_RGB, GL_UNSIGNED_BYTE, input->image); 
	glDisable(GL_READ_BUFFER);
}



void
update_input_neurons_on_eye_tracking (INPUT_DESC *input)
{
	int i, j, w, h, x, y, tfw, vpw, vph, r, g, b;
	GLubyte *image;
	
	h = input->wh;
	w = input->ww;
	x = input->wxd - w/2;
	y = h/2 - input->wyd;
	vph = input->vph;
	vpw = input->vpw;
	tfw = input->tfw;
	image = input->image;
	if (input->neuron_layer->output_type == COLOR)
	{
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				if ((x + j) < 0 || (x + j) >=  vpw || (y + i) < 0 || (y + i) >= vph)
					r = g = b = 0;
				else
				{
					r = image[3*((i+y)*tfw + j + x) + 0];
					g = image[3*((i+y)*tfw + j + x) + 1];
					b = image[3*((i+y)*tfw + j + x) + 2];
				}
				input->neuron_layer->neuron_vector[i*w + j].output.ival = 
					((r&128) >> 7)|((g&128) >> 6)|((b&128) >> 5); 
			}
		}
	}
	else if (input->neuron_layer->output_type == GREYSCALE)
	{
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				if ((x + j) < 0 || (x + j) >=  vpw || (y + i) < 0 || (y + i) >= vph)
					r = g = b = 0;
				else
					r = image[3*((i+y)*tfw + j + x) + 0];
				input->neuron_layer->neuron_vector[i*w + j].output.ival = 
					(int) (((float) r / (255.0/7.0)) + 0.5); 
			}
		}
	}
	else
	{
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				if ((x + j) < 0 || (x + j) >=  vpw || (y + i) < 0 || (y + i) >= vph)
					r = g = b = 0;
				else
					r = image[3*((i+y)*tfw + j + x) + 0];
				input->neuron_layer->neuron_vector[i*w + j].output.ival = (r / 128) * (NUM_COLORS-1); 
			}
		}
	}
}



/* preenche-se esta estrutura na mão. O código normal trata de uma imagem estática e
usa-se o LoadBMP do Xview. Por enquanto, estou preenchendo apenas alguns 
valores, depois deve-se pensar melhor como fazer.*/
int 
init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;
	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;
	
	return (0);
}



void
input_generator (INPUT_DESC *input, int status)
{
	static INPUT_DESC *i_left_eye  = NULL;
	static INPUT_DESC *i_right_eye = NULL;
					
	if (i_left_eye == NULL)
	{
		i_left_eye  = get_input_by_name ("image_left");
		i_right_eye = get_input_by_name ("image_right");
	}
	
	if (((input->win == left_eye) || (input->win == right_eye)) && (input->win != 0))
	{
		update_input_neurons (input);
		
		if (status == MOVE)
		{
			input->wxd_old = input->wxd;
			input->wyd_old = input->wyd;
			input->wxd = input->wx + (input->ww / 2);
			input->wyd = input->wy + (input->wh / 2);
		}
		
		check_input_bounds (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		input_display ();
		all_filters_update ();
		all_outputs_update ();
	}

	if (input->win == 0)
	{
		//float f = 1.0;
		int x, y;

		make_input_image (input);
		init (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		if (right_eye == 0) 
			right_eye = input->win;
		else
			left_eye = input->win;
		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (input_display); 
		glutKeyboardFunc (keyboard);
		glutPassiveMotionFunc (input_passive_motion);
		glutMouseFunc (input_mouse);
	}
}



void
set_output_randomly (char *output_name, int percentage)
{
	int i, j;
	int rand_color;
	int rand_color0;
	int rand_color1;
	int rand_color2;
	OUTPUT_DESC *output;

	output = get_output_by_name (output_name);
	if (output == NULL)
	{
		Erro ("unkonwn output ", output_name, " in set_output_randomly ()");
		return;
	}
		
	for (i = 0; i < output->wh; i++) 
	{
		for (j = 0; j < output->ww; j++) 
		{
			if ((i < output->wh) && (j < output->ww))
			{
				rand_color = ((rand () % 100) < percentage) ? NUM_COLORS-1: 0;
				rand_color0 = rand_color1 = rand_color2 = (rand_color / (NUM_COLORS/2)) * 255;
				
				output->neuron_layer->neuron_vector[i*output->ww + j].output.ival = rand_color;
				
				output->image[3*(i*output->tfw + j) + 0] = (GLubyte) rand_color0;
				output->image[3*(i*output->tfw + j) + 1] = (GLubyte) rand_color1;
				output->image[3*(i*output->tfw + j) + 2] = (GLubyte) rand_color2;
			}
			else
			{
				output->image[3*(i*output->tfw + j) + 0] = 0;
				output->image[3*(i*output->tfw + j) + 1] = 0;
				output->image[3*(i*output->tfw + j) + 2] = 0;
			}
		}
	}
	glutPostWindowRedisplay (output->win);
}



void
set_move_left_and_right (int distance)
{	
		
	switch (distance)	
	{
		
		case 0 :
			set_output_randomly ("move_left_out", 50);
			set_output_randomly ("move_right_out", 50);
			break;		
		 			
		case 1 :
			set_output_randomly ("move_left_out", 50+PERCENT_DIFFERENCE1);
			set_output_randomly ("move_right_out", 50-PERCENT_DIFFERENCE1);
			break;		
			
		case 2 :
			set_output_randomly ("move_left_out", 50+PERCENT_DIFFERENCE2);
			set_output_randomly ("move_right_out", 50-PERCENT_DIFFERENCE2);
			break;		
					
		case 3 :
			set_output_randomly ("move_left_out", 50+PERCENT_DIFFERENCE3);
			set_output_randomly ("move_right_out", 50-PERCENT_DIFFERENCE3);
			break;		
				
		case -1 :
			set_output_randomly ("move_left_out", 50-PERCENT_DIFFERENCE1);
			set_output_randomly ("move_right_out", 50+PERCENT_DIFFERENCE1);
			break;		
			
		case -2 :
			set_output_randomly ("move_left_out", 50-PERCENT_DIFFERENCE2);
			set_output_randomly ("move_right_out", 50+PERCENT_DIFFERENCE2);
			break;		
					
		case -3 :
			set_output_randomly ("move_left_out", 50-PERCENT_DIFFERENCE3);
			set_output_randomly ("move_right_out", 50+PERCENT_DIFFERENCE3);
			break;
					
		default:
			Erro ("invalid value in set_move_left_and_right ()", "", "");
	}	

}	
		


void
add_percentage_neurons_on (OUTPUT_DESC *output, double percentage)
{
	int num_neurons;
	int num_neurons_to_add;
	int position;
	int tries = 0;
	
	num_neurons = output->ww * output->wh;
	if (percentage < 0.0)
		num_neurons_to_add = (int) (percentage * (double) num_neurons - 0.5);
	else
		num_neurons_to_add = (int) (percentage * (double) num_neurons + 0.5);

	if (num_neurons_to_add > 0)
	{
		while (num_neurons_to_add != 0)
		{
			if (tries > 1000)
			{
				show_message ("After 1000 tries could not find a neuron to set", 
						 "in add_percentage_neurons_on ()", "");
				return;
			}
			position = rand () % num_neurons;
			if (output->neuron_layer->neuron_vector[position].output.ival == 0)
			{
				output->neuron_layer->neuron_vector[position].output.ival = NUM_COLORS-1;
				num_neurons_to_add--;
				tries = 0;
			}
			tries++;
		}
	}
	else
	{
		while (num_neurons_to_add != 0)
		{
			if (tries > 1000)
			{
				show_message ("After 1000 tries could not find a neuron to set", 
						 "in add_percentage_neurons_on ()", "");
				return;
			}
			position = rand () % num_neurons;
			if (output->neuron_layer->neuron_vector[position].output.ival != 0)
			{
				output->neuron_layer->neuron_vector[position].output.ival = 0;
				num_neurons_to_add++;
				tries = 0;
			}
			tries++;
		}
	}
}



double
percentual_difference_neurons_on (OUTPUT_DESC *output_1, OUTPUT_DESC *output_2)
{
	int neurons_on_on_1;
	int neurons_on_on_2;
	double difference;
	int num_neurons;

	num_neurons = output_1->ww * output_1->wh;
	
	neurons_on_on_1 = number_neurons_on (output_1->neuron_layer->neuron_vector, 0, num_neurons);
	neurons_on_on_2 = number_neurons_on (output_2->neuron_layer->neuron_vector, 0, num_neurons);
	difference = (double) (neurons_on_on_1 - neurons_on_on_2) / (double) num_neurons;

	return (difference);
}	



double factor = 5.0;

void
draw_output (char *output_name, char *input_name)
{
	INPUT_DESC *input;
	int input_move;
	double percentage_move;
	double difference;
	OUTPUT_DESC *output_left;
	OUTPUT_DESC *output_right;
	
	input = get_input_by_name (input_name);
	input_move = input->wxd - input->wxd_old;
	percentage_move = (double) input_move / ((double) input->ww / factor);

	output_left  = get_output_by_name ("move_left_out");
	output_right = get_output_by_name ("move_right_out");

	difference = percentual_difference_neurons_on (output_right, output_left);
	add_percentage_neurons_on (output_left, (percentage_move + difference) / 2.0);
	add_percentage_neurons_on (output_right, -(percentage_move + difference) / 2.0);
}



void 
set_vergence ()
{
	double difference;
	static INPUT_DESC *i_left_eye = NULL;
					
	if (i_left_eye == NULL)
		i_left_eye = get_input_by_name ("image_left");
	
	difference = percentual_difference_neurons_on (get_output_by_name ("move_right_out"), 
						       get_output_by_name ("move_left_out"));
	
	i_left_eye->wxd += (int) ((difference / factor) * (double) i_left_eye->ww + 0.5);
}



void
output_handler_left (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	if ((mouse_button == GLUT_LEFT_BUTTON) && (mouse_state == GLUT_DOWN) && (type_call == -1) && (draw_active == 1))
		interpreter ("draw output move_left_out;");
		
	if (type_call == RUN)
	{
		set_vergence ();
	}	
}


void
output_handler_right (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	if ((mouse_button == GLUT_LEFT_BUTTON) && (mouse_state == GLUT_DOWN) && (type_call == -1) && (draw_active == 1))
		interpreter ("draw output move_right_out;");
}



void
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	static INPUT_DESC *i_left_eye = NULL;
	static INPUT_DESC *i_right_eye = NULL;
	/*static OUTPUT_DESC *out_left = NULL;
	static OUTPUT_DESC *out_right = NULL;
	static OUTPUT_DESC *out_vertical = NULL;
	static OUTPUT_DESC *out_arm = NULL;
	int previous_x;*/
	
					
	if (i_left_eye == NULL)
	{
		i_left_eye = get_input_by_name ("image_left");
		i_right_eye = get_input_by_name ("image_right");
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == right_eye))
	{
		i_left_eye->wxd += i_right_eye->wxd - i_right_eye->wxd_old;
		i_left_eye->wyd = i_right_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_right_eye->name, i_right_eye->wxd, i_right_eye->wyd);
		interpreter (command);

		sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == left_eye))
	{
		i_left_eye->wyd = i_right_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
	}
	
	if ((move_active == 1) && (draw_active == 1) &&
	    (input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == left_eye))
	{
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE1), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE1), i_left_eye->wyd);
		interpreter (command);
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE2), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE2), i_left_eye->wyd);
		interpreter (command);
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE3), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE3), i_left_eye->wyd);
		interpreter (command);
		

		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE1), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE1), i_left_eye->wyd);
		interpreter (command);
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE2), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE2), i_left_eye->wyd);
		interpreter (command);
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd - DISP_ANGLE(VERGENCE_DISP_ANGLE3), i_left_eye->wyd);
		interpreter (command);
		interpreter ("forward filters;");
		interpreter ("forward neural_layers;");
		interpreter ("forward outputs;");
		interpreter ("draw move_left_out based on image_left move;");
		interpreter ("train network;");
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, 
				  i_left_eye->wxd + DISP_ANGLE(VERGENCE_DISP_ANGLE3), i_left_eye->wyd);
		interpreter (command);
	}
	current_input = input->win;
	input->mouse_button = -1;
}



NEURON_OUTPUT
sum_neurons_output (NEURON *n, OUTPUT_TYPE output_type, int begin, int end)
{
	int i;
	NEURON_OUTPUT sum;
	
	if (output_type == GREYSCALE_FLOAT)
	{
		sum.fval = 0.0;
		for (i = begin; i < end; i++)
			sum.fval += n[i].output.fval;
	}
	else
	{
		sum.ival = 0;
		for (i = begin; i < end; i++)
			sum.ival += n[i].output.ival;
	}
	return (sum);
}



int
sum_nl_response (NEURON_LAYER *neural_layer, int begin, int end)
{
	int x, y, h, w, sum;
	
	sum = 0;
	w = neural_layer->dimentions.x;
	h = neural_layer->dimentions.y;
	for (y = 0; y < h; y++)
		for (x = begin; x < end; x++)
			sum += neural_layer->neuron_vector[y * w + x].output.ival;
	
	return (sum);
}



void
move_x ()
{
	char command[256];
	static INPUT_DESC *i_left_eye = NULL;
	//int x_current;
	
	if (i_left_eye == NULL)
		i_left_eye = get_input_by_name ("image_left");
	
	i_left_eye->wxd_old = i_left_eye->wxd;
	i_left_eye->wxd += side_move;
	sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}



void
set_initial_position ()
{
	char command[256];
	
	sprintf (command, "move %s to %d, %d;", "image_right", 118, 93);
	interpreter (command);
	sprintf (command, "move %s to %d, %d;", "image_left", 118, 93);
	interpreter (command);

	glutIdleFunc ((void (* ) (void)) check_forms);
}



void
set_vergence_by_te_filter ()
{
	char command[256];
	static INPUT_DESC *i_left_eye = NULL;
	static INPUT_DESC *i_right_eye = NULL;	
	static NEURON_LAYER *te = NULL;
	static NEURON_LAYER *te1 = NULL;
	static NEURON_LAYER *te2 = NULL;
	static int num_neurons;
	int x, x_inic, x_min, x_max, x_max_te = 0, x_min_te;
	NEURON_OUTPUT current_te, current_te1, current_te2, max_te, min_te;
	FILE /* *arq_points_mf,*/ *arq_points_te, *arq_points_ini, *arq_points_min;

	if (te == NULL)
	{
		te  = get_neuron_layer_by_name ("te");
		te1 = get_neuron_layer_by_name ("te1");
		te2 = get_neuron_layer_by_name ("te2");
		
		i_left_eye  = get_input_by_name ("image_left");
		i_right_eye = get_input_by_name ("image_right");
		
		num_neurons = te->dimentions.x * te->dimentions.y;

		if (te->output_type != te1->output_type 
		||  te->output_type != te2->output_type)
		{
			Erro ("Neuron layers has differents output type.", 
			"set_vergence_by_te_filter function must be applied with te outputs of the same output_type.", "");
			return;
		}
	}
	system ("rm *.out");
	arq_points_te = fopen ("te_out.out", "w");
	arq_points_ini = fopen ("ini.out", "w");
	arq_points_min = fopen ("verg.out", "w");

	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "ini.out", 1, 2, 0, "Start Point", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "verg.out", 1, 2, 0, "Vergence", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "te_out.out", 1, 2, 0, "Response on te_out", LINES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "te_out.out", 1, 3, 0, "Response on te1_out", LINES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "te_out.out", 1, 4, 0, "Response on te2_out", LINES));

	x_inic = i_left_eye->wxd;
	x_min  = i_right_eye->wxd;
	x_max  = i_right_eye->wxd + i_right_eye->ww / 6;

	if (te->output_type == GREYSCALE_FLOAT)
	{
		max_te.fval = te->min_neuron_output.fval * ((float) num_neurons);
		min_te.fval = te->max_neuron_output.fval * ((float) num_neurons);
	}
	else
	{
		max_te.ival = te->min_neuron_output.ival * num_neurons;
		min_te.ival = te->max_neuron_output.ival * num_neurons;
	}

	for (x = x_min; x < x_max; x++)
	{
		i_left_eye->wxd = x;
		
		sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
		
		if (te->output_type == GREYSCALE_FLOAT)
		{
			current_te.fval  = (sum_neurons_output (te->neuron_vector, te->output_type, 0, num_neurons)).fval;
			current_te1.fval = (sum_neurons_output (te1->neuron_vector, te1->output_type, 0, num_neurons)).fval;
			current_te2.fval = (sum_neurons_output (te2->neuron_vector, te2->output_type, 0, num_neurons)).fval;
			
			fprintf (arq_points_te, "%d, %f, %f, %f\n", x, current_te.fval, current_te1.fval, current_te2.fval);
			
			if (current_te.fval >= max_te.fval)
			{
				max_te.fval = current_te.fval;
				x_max_te = x;
			}
			if (min_te.fval >= current_te.fval)
			{
				min_te.fval = current_te.fval;
				x_min_te = x;
			}
		}
		else
		{
			current_te.ival  = (sum_neurons_output (te->neuron_vector, te->output_type, 0, num_neurons)).ival;
			current_te1.ival = (sum_neurons_output (te1->neuron_vector, te1->output_type, 0, num_neurons)).ival;
			current_te2.ival = (sum_neurons_output (te2->neuron_vector, te2->output_type, 0, num_neurons)).ival;
			
			fprintf (arq_points_te, "%d, %d, %d, %d\n", x, current_te.ival, current_te1.ival, current_te2.ival);
			
			if (max_te.ival < current_te.ival)
			{
				max_te.ival = current_te.ival;
				x_max_te = x;
			}
			if (min_te.ival > current_te.ival)
			{
				min_te.ival = current_te.ival;
				x_min_te = x;
			}
		}
	}
	for (x = x_min; x < x_max; x++)
	{
		if (x == x_inic)
			if (te->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_ini, "%d, %f\n", x, max_te.fval * 1.2);
			else
				fprintf (arq_points_ini, "%d, %d\n", x, (int) ((float) max_te.ival * 1.2));
		else
			fprintf (arq_points_ini, "%d, %d\n", x, 0);
		if (x == x_max_te)
			if (te->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_min, "%d, %f\n", x, max_te.fval * 1.2);
			else
				fprintf (arq_points_min, "%d, %d\n", x, (int) ((float) max_te.ival * 1.2));
		else
			fprintf (arq_points_min, "%d, %d\n", x, 0);
	}
	fclose (arq_points_te);
	fclose (arq_points_min);
	fclose (arq_points_ini);

	i_left_eye->wxd_old = i_left_eye->wxd;
	i_left_eye->wxd = x_max_te;
	sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
	plot_gnuplot_file("te.cmd");
}



void
set_vergence_by_minus_filter ()
{
	char command[256];
	static INPUT_DESC *i_left_eye     = NULL;
	static INPUT_DESC *i_right_eye    = NULL;	
	static NEURON_LAYER *minus_filter = NULL;
	static int num_neurons;
	int x, x_inic, x_min, x_max, x_max_minus, x_min_minus = 0;
	NEURON_OUTPUT minus, max_minus, min_minus;
	FILE /* *arq_points_mf,*/ *arq_points_minus, *arq_points_ini, *arq_points_min;
	
	if (minus_filter == NULL)
	{
		minus_filter = get_neuron_layer_by_name ("right_minus_left");
		i_left_eye   = get_input_by_name ("image_left");
		i_right_eye  = get_input_by_name ("image_right");
		
		num_neurons = minus_filter->dimentions.x * minus_filter->dimentions.y;
	}
	
	system ("rm *.out");
	arq_points_minus = fopen ("minus.out", "w");
	arq_points_ini = fopen ("ini.out", "w");
	arq_points_min = fopen ("verg.out", "w");
	
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "ini.out" , 1, 2, 0, "Start Point", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "verg.out" , 1, 2, 0, "Vergence", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "minus.out" , 1, 2, 0, "Response on minus", LINES));

	x_inic = i_left_eye->wxd;
	x_min  = i_right_eye->wxd;
	x_max  = i_right_eye->wxd + i_right_eye->ww / 6;
	
	if (minus_filter->output_type == GREYSCALE_FLOAT)
	{
		max_minus.fval = 0.0;
		min_minus.fval = 256.0 * ((float) num_neurons);
	}
	else
	{
		max_minus.ival = 0;
		min_minus.ival = 256 * num_neurons;
	}
	
	for (x = x_min; x < x_max; x++)
	{
		i_left_eye->wxd = x;
		i_left_eye->wyd = i_right_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
		
		all_filters_update ();
		
		if (minus_filter->output_type == GREYSCALE_FLOAT)
		{
			minus.fval = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).fval;
			fprintf (arq_points_minus, "%d, %f\n", x, minus.fval);
			
			if (max_minus.fval < minus.fval)
			{
				max_minus.fval = minus.fval;
				x_max_minus = x;
			}
			if (min_minus.fval > minus.fval)
			{
				min_minus.fval = minus.fval;
				x_min_minus = x;
			}
		}
		else
		{
			minus.ival = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).ival;
			fprintf (arq_points_minus, "%d, %d\n", x, minus.ival);
			
			if (max_minus.ival < minus.ival)
			{
				max_minus.ival = minus.ival;
				x_max_minus = x;
			}
			if (min_minus.ival > minus.ival)
			{
				min_minus.ival = minus.ival;
				x_min_minus = x;
			}
		}
	}
	for (x = x_min; x < x_max; x++)
	{
		if (x == x_inic)
			if (minus_filter->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_ini, "%d, %f\n", x, max_minus.fval * 1.2);
			else
				fprintf (arq_points_ini, "%d, %d\n", x, (int) ((float) max_minus.ival * 1.2));
		else
			fprintf (arq_points_ini, "%d, %d\n", x, 0);
		if (x == x_min_minus)
			if (minus_filter->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_min, "%d, %f\n", x, min_minus.fval * 1.2);
			else
				fprintf (arq_points_min, "%d, %d\n", x, (int) ((float) min_minus.ival * 1.2));
		else
			fprintf (arq_points_min, "%d, %d\n", x, 0);
	}
	
	fclose (arq_points_minus);
	fclose (arq_points_min);
	fclose (arq_points_ini);
	plot_gnuplot_file("minus.cmd");

	i_left_eye->wxd_old = i_left_eye->wxd;
	i_left_eye->wxd = x_min_minus;
	sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}



void
move_right_eye ()
{
	char command[256];
	static INPUT_DESC *i_right_eye = NULL;
	
	if (i_right_eye == NULL)
		i_right_eye = get_input_by_name ("image_right");
	
	sprintf (command, "move %s to %d, %d;", i_right_eye->name, 
			  i_right_eye->wxd, i_right_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}



void 
new_output_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static INPUT_DESC *input = NULL;
	static OUTPUT_DESC *output = NULL;
	
	if (input == NULL || output->win != current_output)
	{
		output = get_output_by_win (current_output);
		if (strcmp(output->name,"logpolar_righteye_out") == 0)
			input = get_input_by_name ("image_right");
		else
			input = get_input_by_name ("image_left");
	}
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	sprintf (mouse_pos, "mouse: (%d, %d)", (int) (wx), (int) (wy));
	set_mouse_bar (mouse_pos);

/*	if (input->green_cross)*/
		map_v1_to_image (&(input->green_cross_x), &(input->green_cross_y), input->neuron_layer->dimentions.x, input->neuron_layer->dimentions.y, 
			 	(int) wx, (int) wy, output->neuron_layer->dimentions.x, output->neuron_layer->dimentions.y, input->wxd, input->wyd, 1.0, SERENO_MODEL);

	glutPostWindowRedisplay (input->win);		 
}



void
output_handler_logpolar_righteye (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	//char command[256];
	static INPUT_DESC *i_right_eye = NULL;
	
	if (i_right_eye == NULL)
		i_right_eye = get_input_by_name ("image_right");
	
	if ((mouse_button == GLUT_LEFT_BUTTON) &&
	    (mouse_state == GLUT_DOWN))
	{
		map_v1_to_image (&(i_right_eye->wxd), &(i_right_eye->wyd), i_right_eye->neuron_layer->dimentions.x, i_right_eye->neuron_layer->dimentions.y, 
				 output->wxd, output->wyd, output->neuron_layer->dimentions.x, output->neuron_layer->dimentions.y, i_right_eye->wxd, i_right_eye->wyd, 1.0, SERENO_MODEL);

		glutIdleFunc ((void (* ) (void))move_right_eye);		 
	}
	output->mouse_button = -1;
}



extern double par;

void 
f_keyboard (char *key_value)
{
	char key;
	INPUT_DESC *input;	
	OUTPUT_DESC *output;	
	
	key = key_value[0];
	current_output = glutGetWindow ();
	output = get_output_by_win (current_output);
	switch (key) 
	{
		case 'a':

			if (strcmp(output->name,"logpolar_righteye_out") == 0)
				input = get_input_by_name ("image_right");
			else
				input = get_input_by_name ("image_left");
			input->green_cross = 1;
						
			if (output != NULL)
			{
				glutSetWindow (output->win);
				glutPassiveMotionFunc (new_output_passive_motion);
				glutPostWindowRedisplay (input->win);
			}
			break;

		case 'A':

			if (strcmp(output->name,"logpolar_righteye_out") == 0)
				input = get_input_by_name ("image_right");
			else
				input = get_input_by_name ("image_left");
			input->green_cross = 0;
						
			if (output != NULL)
			{
				glutSetWindow (output->win);
				glutPassiveMotionFunc (output_passive_motion);
				glutPostWindowRedisplay (input->win);
			}
			break;
		case 's':
			current_kernel += 1;
			printf ("current_kernel = %d\n", current_kernel);
			break;

		case 'S':
			current_kernel -= 1;
			printf ("current_kernel = %d\n", current_kernel);
			break;
		case 'm':
		case 'M':
			glutIdleFunc ((void (* ) (void))set_vergence_by_minus_filter);
			break;
		case 't':
		case 'T':
			glutIdleFunc ((void (* ) (void))set_vergence_by_te_filter);
			break;
		case 'f':
			glutIdleFunc ((void (* ) (void))set_initial_position);
			break;
		case 'z':
			side_move = -1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
		case 'x':
			side_move = 1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
		case 'c':
			par = par*2.0;
			printf ("par = %f\n", par);
			break;
		case 'p':
			if (output != NULL)
				print_output(current_output);
			break;
		case 'v':
			par = par/2.0;
			printf ("par = %f\n", par);
			break;
	}
					
	all_filters_update ();
}



NEURON_OUTPUT
v1_to_image_mapping (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int *xi, *yi;
	int wi, hi;
	int u, v;
	int w, h;
	
	/*------Get the Address of the Image Coordenates------*/
	xi = (int *) param_list->next->param.pval;
	yi = (int *) param_list->next->next->param.pval;
	
	/*--------------Get the Image Dimentions--------------*/
	wi = param_list->next->next->next->param.ival;
	hi = param_list->next->next->next->next->param.ival;
	
	/*-------------Get the V1 Coordenates-----------------*/
	u = param_list->next->next->next->next->next->param.ival;
	v = param_list->next->next->next->next->next->next->param.ival;
	
	/*--------------Get the Image Dimentions--------------*/
	w = param_list->next->next->next->next->next->next->next->param.ival;
	h = param_list->next->next->next->next->next->next->next->next->param.ival;
	
	map_v1_to_image (xi, yi, wi, hi, u, v, w, h, 238, 189, 1.0, SERENO_MODEL);
	 
/*	printf ("\nV1 point: (%d, %d)\n", u, v);
	printf ("Image point: (%d ,%d)\n", *xi, *yi);*/
	
	return (output);
}



NEURON_OUTPUT
verge_left_to_right_image_point (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	NEURON_OUTPUT minus, max_minus, min_minus;
	int xi, yi;
	static INPUT_DESC *i_left_eye     = NULL;
	static INPUT_DESC *i_right_eye    = NULL;	
	static NEURON_LAYER *minus_filter = NULL;
	static int num_neurons;
	int x, x_inic, x_min, x_max, x_max_minus, x_min_minus = 0	;
	
	if (minus_filter == NULL)
	{
		minus_filter = get_neuron_layer_by_name ("right_minus_left");
		
		i_left_eye   = get_input_by_name ("image_left");
		i_right_eye  = get_input_by_name ("image_right");
		
		num_neurons = minus_filter->dimentions.x * minus_filter->dimentions.y;
	}
	
	/* -------Get the New Coordinates of Image Right------ */
	xi = param_list->next->param.ival;
	yi = param_list->next->next->param.ival;
	
	if ((xi < 0) || (xi >= i_right_eye->ww) || (yi < 0) || (yi >= i_right_eye->wh))
	{	
		output.ival = 0;
		return (output);
	}
	
	/* -------Keep the Old Coordinates of Image Right------ */
	i_right_eye->wxd_old = i_right_eye->wxd;
	i_right_eye->wyd_old = i_right_eye->wyd;
	
	/* -------Set the New Coordinates of Image Right------ */
	i_right_eye->wxd = xi;
	i_right_eye->wyd = yi;
	
	/* -------Set the New Coordinates of Image Left------ */
	i_left_eye->wxd += i_right_eye->wxd - i_right_eye->wxd_old;
	i_left_eye->wyd = i_right_eye->wyd + correction;
	
	/* ---------------Move the Right Image--------------- */
	move_input_window (i_right_eye->name, i_right_eye->wxd, i_right_eye->wyd);	
		
	x_inic = i_left_eye->wxd;
	x_min  = i_right_eye->wxd;
	x_max  = i_right_eye->wxd + i_right_eye->ww / 6;
	
	if (minus_filter->output_type == GREYSCALE_FLOAT)
	{
		max_minus.fval = 0.0;
		min_minus.fval = 256.0 * ((float) num_neurons);
	}
	else
	{
		max_minus.ival = 0;
		min_minus.ival = 256 * num_neurons;
	}
	
	for (x = x_min; x < x_max; x++)
	{
		i_left_eye->wxd = x;
		i_left_eye->wyd = i_right_eye->wyd + correction;

		move_input_window (i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		
		all_filters_update ();
		all_outputs_update ();
		
		if (minus_filter->output_type == GREYSCALE_FLOAT)
		{
			minus.fval = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).fval;
			
			if (max_minus.fval < minus.fval)
			{
				max_minus.fval = minus.fval;
				x_max_minus = x;
			}
			if (min_minus.fval > minus.fval)
			{
				min_minus.fval = minus.fval;
				x_min_minus = x;
			}
		}
		else
		{
			minus.ival = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).ival;
			
			if (max_minus.ival < minus.ival)
			{
				max_minus.ival = minus.ival;
				x_max_minus = x;
			}
			if (min_minus.ival > minus.ival)
			{
				min_minus.ival = minus.ival;
				x_min_minus = x;
			}
		}
	}
	
	i_left_eye->wxd_old = i_left_eye->wxd;
	i_left_eye->wxd = x_min_minus;
	
	move_input_window (i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);

	output.ival = (i_left_eye->wxd - i_right_eye->wxd);
	
/*	printf ("\nImage left: (%d ,%d)\n", i_left_eye->wxd, i_left_eye->wyd);
	printf ("Image right: (%d ,%d)\n", i_right_eye->wxd, i_right_eye->wyd);*/

	return (output);
}



NEURON_OUTPUT
map_disparity (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	static NEURON_LAYER *disparity_map = (NEURON_LAYER *) NULL;
	int u, v;
	int w, h;
	int disparity;
	
	if (disparity_map == (NEURON_LAYER *) NULL)
		disparity_map = get_neuron_layer_by_name ("disparity_map");	

	w = disparity_map->dimentions.x;
	h = disparity_map->dimentions.y;
		
	/*-------------Get the V1 Coordenates-----------------*/
	u = param_list->next->param.ival;
	v = param_list->next->next->param.ival;
	
	/*--------------Get the Image Dimentions--------------*/
	disparity = param_list->next->next->next->param.ival;
	
	disparity_map->neuron_vector[u + v * w].output.fval = (float) disparity;
		 
	/*printf ("\nDisparity map point: (%d, %d, %d)\n", u, v, disparity);*/
	
	return (output);
}



NEURON_OUTPUT
save_point (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int xi, yi;
	int disparity;
	int color_rgb;
	float r, g, b;
	FILE *points_file = (FILE *) NULL;
	static int flag = 0;
	
	if (flag == 0)
	{
		points_file = fopen ("points.out", "w");
		flag++;
	}
	else
		points_file = fopen ("points.out", "a");
		
	if (points_file == (FILE *) NULL)
	{
		Erro ("Could not open file points.out.", "", "");
		output.ival = 0;
		return (output);
	}
	
	/*---Get the Image Coordinates-----*/
	xi = param_list->next->param.ival;
	yi = param_list->next->next->param.ival;
	
	/*---Get the Disparity-----*/
	disparity = param_list->next->next->next->param.ival;
	
	/*------Extract the RGB Values of the Pixel----*/
	color_rgb = param_list->next->next->next->next->param.ival;
	
	r = (float) RED   (color_rgb) / 255.0;
	g = (float) GREEN (color_rgb) / 255.0;
	b = (float) BLUE  (color_rgb) / 255.0;
	
	/*-----Save Point to File-----------*/
	fprintf (points_file, "%d %d %d %f %f %f\n", xi, yi, disparity, r ,g ,b);

	if (fclose (points_file) != 0)
		Erro ("Could not close points.out.", "", "");
	
	return (output);
}



NEURON_OUTPUT
image_to_word (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int xi, yi;
	int disparity;
	float *x_world, *y_world, *z_world;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	static INPUT_DESC *i_right_eye = (INPUT_DESC *) NULL;	
	
	if (i_right_eye == (INPUT_DESC *) NULL)
		i_right_eye  = get_input_by_name ("image_right");
		
	/* -------Get the New Coordinates of Image Right------ */
	xi = param_list->next->param.ival;
	yi = param_list->next->next->param.ival;
	
	/*-------------Get the Disparity Measured-------------*/
	disparity = param_list->next->next->next->param.ival;
	
	/* -------Get the Word Coordinates-------------------- */
	x_world = (float *) param_list->next->next->next->next->param.pval;
	y_world = (float *) param_list->next->next->next->next->next->param.pval;
	z_world = (float *) param_list->next->next->next->next->next->next->param.pval;
	
	right_point.x = (double) (xi);
	right_point.y = (double) (yi);
				
	left_point.x = (double) (xi + disparity);
	left_point.y = (double) (yi);
	
	world_point = calculate_world_point (left_point, right_point, i_right_eye->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);
	
	*x_world = (float) world_point.x;
	*y_world = (float) world_point.y;
 	*z_world = (float) world_point.z;
	
/*	printf ("\nImage point: (%d ,%d)\n", xi, yi);
	printf ("Disparity: %d\n", disparity);
	printf ("Word point: (%f ,%f ,%f)\n", world_point.x, world_point.y, world_point.z);*/
	
	printf ("%f %f %f\n", world_point.x, world_point.y, -world_point.z);
	
	return (output);
}



NEURON_OUTPUT
get_color_of_image_right (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int xi, yi;
	static INPUT_DESC *image_right = (INPUT_DESC *) NULL;
	int vpw, vph, tfw;
	int r, g, b;
	GLubyte *image;	
	
	/*------Get the Right Input Image------*/
	if (image_right == (INPUT_DESC *) NULL)
		image_right  = get_input_by_name ("image_right");
	
	/*------Get the Image Coordenates------*/
	xi = param_list->next->param.ival;
	yi = param_list->next->next->param.ival;
	
	/*------Get the Image Parameters------*/
	tfw = image_right->tfw;
	image = image_right->image;
	vph = image_right->vph;
	vpw = image_right->vpw;
		
	/*------Get the Pixel Value-----------*/
	if (xi < 0 || xi >=  vpw || yi < 0 || yi >= vph)
		r = g = b = 0;	
	else
	{
		r = image[3 * (yi * tfw + xi) + 0];
		g = image[3 * (yi * tfw + xi) + 1];
		b = image[3 * (yi * tfw + xi) + 2];	
	}
		
	output.ival = PIXEL(r, g, b);
	
/*	printf ("\nColor: (%d, %d ,%d)\n", r, g, b);*/
	
	return (output);
}



NEURON_OUTPUT
generate_iv_file_prolog (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	FILE *iv_file = (FILE *) NULL;
		
	if ((iv_file = fopen ("word.iv", "w")) == (FILE *) NULL)
	{
		Erro ("Could not open file word.iv.", "", "");
		output.ival = 0;
		return (output);
	}
	
	fprintf (iv_file, "#Inventor V2.0 ascii\n\n");
	fprintf (iv_file, "Separator \n{\n\tPerspectiveCamera {position 0 0 0}\n");
	
	if (fclose (iv_file) != 0)
		Erro ("Could not close file word.iv.", "", "");
	
	return (output);
}



NEURON_OUTPUT
generate_iv_file_body (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	float x_world, y_world, z_world;
	int color_rgb;
	float r, g, b;
	float pixel_size;
	FILE *iv_file = (FILE *) NULL;
		
	if ((iv_file = fopen ("word.iv", "a")) == (FILE *) NULL)
	{
		Erro ("Could not open file word.iv.", "", "");
		output.ival = 0;
		return (output);
	}
	
	/*---Get the Word Coordinates of the Point-----*/
	x_world =param_list->next->param.fval;
	y_world =param_list->next->next->param.fval;
	z_world =param_list->next->next->next->param.fval;
	
	/*------Extract the RGB Values of the Pixel----*/
	color_rgb = param_list->next->next->next->next->param.ival;
	
	r = (float) RED   (color_rgb) / 255.0;
	g = (float) GREEN (color_rgb) / 255.0;
	b = (float) BLUE  (color_rgb) / 255.0;
	
/*	printf ("\nColor: (%d, %d ,%d)\n", r, g, b);
	printf ("Word point: (%f ,%f ,%f)\n", x_world, y_world, z_world);*/
	
	/*------------Get the Pixel Size---------------*/
	pixel_size = 0.01;

	fprintf (iv_file, "\tTranslation {translation  %.8f %.8f %.8f}\n", x_world, y_world, -z_world);
    	fprintf (iv_file, "\tMaterial {diffuseColor %f %f %f}\n",  r, g, b);
	fprintf (iv_file, "\tCube {width %f height %f depth %f}\n", pixel_size, pixel_size, 0.1 * pixel_size);
	fprintf (iv_file, "\tTranslation {translation  %.8f %.8f %.8f}\n\n", -x_world, -y_world, z_world);

	if (fclose (iv_file) != 0)
		Erro ("Could not close file word.iv.", "", "");
	
	return (output);
}



NEURON_OUTPUT
generate_iv_file_epilog (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	FILE *iv_file = (FILE *) NULL;
		
	if ((iv_file = fopen ("word.iv", "a")) == (FILE *) NULL)
	{
		Erro ("Could not open file word.iv.", "", "");
		output.ival = 0;
		return (output);
	}

	fprintf (iv_file, "}\n");
	
	if (fclose (iv_file) != 0)
		Erro ("Could not close file word.iv.", "", "");

	return (output);
}
