#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#define NIL -1

/* Arquivo que contem a lista de arquivos de dados de entrada do monks */
FILE *input_file;

int label = NIL;
int cont_data = 0;
short class_value;
 
/* - Dados lidos do arquivo Monks
   1 - Apaga os dois bits (0,0)
   2 - Acende o bit de cima (1,0)
   3 - Acende o bit de baixo (0,1)
   4 - Acende os dois bits (1,1)
*/
void
f_keyboard(char *key_value)
{
}

int 
init_user_functions()
{
	return (0);
}

unsigned
l_b (unsigned data)
{
	unsigned long data_out;
	
	data_out = (unsigned long) (data & 0xff) << 24;
	data_out |= (unsigned long) (data & 0xff00) << 8;
	data_out |= (unsigned long) (data & 0xff0000) >> 8;
	data_out |= (unsigned long) (data & 0xff000000) >> 24;
	return (data_out);
}

short
read_monks_data (FILE *monks_file)
{
	int data_value;
	char val[2];
	
	fread (val, 2, 1, monks_file);
	data_value = val[0] - '0';
       
/*
	fscanf(monks_file, "%d",&data_value);
*/	return ((short)data_value);

}

void 
read_monks_input_file (INPUT_DESC *input, FILE *monks_input_file)
{
	int i;
	int val;
	
	for (i = 0; i < input->ww; i++)
	{
		val = read_monks_data (monks_input_file); 
		input->neuron_layer->neuron_vector[i].output.ival = (NUM_GREYLEVELS - 1) * ((val - 1) & 1);
		input->neuron_layer->neuron_vector[i].output.ival = (NUM_GREYLEVELS - 1) * (((val - 1) >> 1) & 1);
	}
}


void
read_monks_input (INPUT_DESC *input, FILE *monks_input_file, int direction)
{
	if (direction == -1)
	{
		fseek (monks_input_file, -2 * (input->ww + 1), SEEK_CUR);
		if (cont_data)
			cont_data--;
	}
	else 
	{
		cont_data++;
	}
	
	// Read the 'output' class value
	class_value = read_monks_data (monks_input_file);
	// Read the 'input' features
	read_monks_input_file (input, monks_input_file);
}


void
get_new_data (INPUT_DESC *input, int direction)
{
	read_monks_input (input, input_file, direction);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	update_input_image (input);
}


void
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (command, "draw monks_out based on caracteristicas move;");
		interpreter (command);
	}
	if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (move_active == 1))
	{
		sprintf (command, "move %s to %d, %d;", input->name, input->wxme, input->wyme);
		interpreter (command);
	}
	input->mouse_state = -1;
}

void
draw_output (char *output_name, char *input_name)
{
	OUTPUT_DESC *output;
	INPUT_DESC *input;
	
	output = get_output_by_name (output_name);
	input = get_input_by_name (input_name);
	
	set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, class_value*(NUM_COLORS-1));
		
	update_output_image (output);
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
}


void 
init_monks_input (INPUT_DESC *input)
{
	int x, y;
	float f;

	make_input_image (input);

	f = 1.0;
	while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
		f += 1.0;
	while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
		f *= 0.9;
	glutInitWindowSize ((int) ((float)input->ww * f), (int) ((float)input->wh * f));
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);
	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutMouseFunc (input_mouse);
	glutPassiveMotionFunc (input_passive_motion);
	glutKeyboardFunc (keyboard);
}


void
input_generator (INPUT_DESC *input, int status)
{
	static int flag = 0;
		
	if (flag == 0)
	{
		init_monks_input (input);
		flag = 1;
	}
	
	if (status == MOVE)
	{
		if (input->wx >= 0)
			get_new_data (input, 1);
		else
			get_new_data (input, -1);
		all_filters_update ();
	}
}


int
evaluate_output (OUTPUT_DESC *output)
{
	int num_neurons;
	short value;
	
	num_neurons = output->ww * output->wh;
	
	value = (short) number_neurons_on (output->neuron_layer->neuron_vector, 0, num_neurons);

	return (value);
}


void
output_handler (OUTPUT_DESC *output, int status)
{
	static int flag = 0;
	int number;
	static int correct = 0, incorrect = 0;
	static int ii = 0, jj = 0;
	char simbolo;
	
	if (flag == 0)
	{
		flag = 1;
		return;
	}
	if (flag == 1)
	{
		number = evaluate_output (output);
		if (number == class_value)
		{
			correct++;
			simbolo = '=';
		}
		else
		{
			incorrect++;
			simbolo = '!';
		}
		flag = 1;

		if (ii > 400)
		{
			printf ("i=%d,j=%d -> ", ii+1, jj+1);
			printf ("(y=%d) %c= (class=%d) : acerto acum.=(%0.4f)\n", number,
				simbolo, class_value, (float) correct / (float) (correct+incorrect));
		}

		jj++;
		if (jj == 10)
		{
			jj = 0;
			ii++;
		}
	}
	else
		flag++;
}
