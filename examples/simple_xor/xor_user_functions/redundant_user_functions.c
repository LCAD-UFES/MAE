#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "mae.h"
#define NIL -1

FILE *char_image_file, *char_label_file;
int label = NIL;
int cont_char = 0;
int char_image_file_size;
int char_label_file_size;
 

int 
init_user_functions()
{
	return (0);
}

unsigned int
read_data (FILE *arq)
{
	int val, n_itens;
	
	n_itens = fscanf (arq, "%d ", &val);
	if (n_itens != 1)
	{
		printf ("Error! Could not read input file\n");
		exit (-1);
	}
	return (val);
}

void
print_header_image (FILE *arq, int *row, int *column, int *sz)
{
	int val;
	int big_endian;
	
	val = read_data (arq);
	*sz = val;
	printf ("Number of images:  %d\n", val);
	val = read_data (arq);
	*row = val;
	printf ("Number of rows:    %d\n", val);
	val = read_data (arq);
	*column = val;
	printf ("Number of columns: %d\n", val);
}

void
print_header_label (FILE *arq, int *sz)
{
	int val;

	val = read_data (arq);
	*sz = val;
	printf ("Number of images:  %d\n", val);
}

void
make_pinfo (INPUT_DESC *input, int w, int h)
{
	int i;
	
	input->pinfo = (PICINFO *) alloc_mem ((size_t) sizeof (PICINFO));
	input->pinfo->pic = (byte *)  alloc_mem ((size_t) w * h);
	input->pinfo->w = input->pinfo->normw = w;
	input->pinfo->h = input->pinfo->normh = h;
	input->pinfo->type = PIC8;
	for (i = 0; i < 256; i++)
		input->pinfo->r[i] = input->pinfo->g[i] = input->pinfo->b[i] = i;
}

int 
open_char_input_image (INPUT_DESC *input, FILE **char_image_file)
{
	char file_name[256];
	int i;
	int sz, row, column;
	static int flag = 0;
	
	fgets (file_name, 250, input->file);
	i = 0;
	while (isgraph (file_name[i]))
		i++;
	file_name[i] = '\0';
	printf("File_Name:%s\n",file_name);
	if (( *(char_image_file) = fopen (file_name, "r")) == NULL)
	{
		printf ("Error! Could not open %s\n", file_name);
		exit (-1);
	}
	print_header_image (*char_image_file, &row, &column, &sz);
	
	if (flag == 0)
	{
		make_pinfo (input, row, column);
		flag ++;
	}
		
	return (sz);
}

int 
open_char_input_label (INPUT_DESC *input, FILE **char_label_file)
{
	char file_name[256];
	int i;
	int sz, row, column;
		
	fgets (file_name, 250, input->file);
	i = 0;
	while (isgraph (file_name[i]))
		i++;
	file_name[i] = '\0';
	printf("File_Name:%s\n",file_name);
	if ((*(char_label_file) = fopen (file_name, "r")) == NULL)
	{
		printf ("Error! Could not open %s\n", file_name);
		exit (-1);
	}
	print_header_label (*char_label_file, &sz);
	
	return (sz);
}

void 
read_char_input_image (INPUT_DESC *input, FILE *char_image_file)
{
	int i;
	
	for (i = 0; i < (input->wh * input->ww); i++)
		input->pinfo->pic[i] = 255 * read_data (char_image_file);
}

unsigned char
read_char_input_label (FILE *char_label_file)
{
	
	return (read_data (char_label_file));
}

int
read_char_input (INPUT_DESC *input, FILE *char_label_file, FILE *char_image_file, int direction)
{
	int new_label;
	
	if ((direction == -1) && (cont_char > 1))
	{
		fseek (char_image_file, -2 * (input->wh * input->ww * 2), SEEK_CUR);
		fseek (char_label_file, -2 * 2, SEEK_CUR);
		read_char_input_image (input, char_image_file);
		new_label = read_char_input_label (char_label_file);
		cont_char--;
	}
	else if ((direction == 1) && (cont_char < char_image_file_size))
	{
		read_char_input_image (input, char_image_file);
		new_label = read_char_input_label (char_label_file);
		cont_char++;
	}
	else
		new_label = label;
		
	return (new_label);
}


void
get_new_char (INPUT_DESC *input, int direction)
{
	label = (int) read_char_input (input, char_label_file, char_image_file, direction);
	update_input_neurons (input);
	robot_check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
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
		sprintf (command, "draw result_out based on character move;");
		interpreter (command);
	}
	input->mouse_state = -1;
}

void 
user_set_neurons (NEURON *n, OUTPUT_DESC *output, int label, int value)
{
	int xi, yi, xf, yf, aux;
	
	if ((label >= 0) && (label <= 4))
	{
		xi = 0;
		xf = output->ww/2;
	}
	else if ((label >= 5)&&(label <= 9))
	{
		xi = output->ww/2;
		xf = output->ww;
	}
	
	yi = (label%5)*(output->wh/5);
	yf = ((label%5)+1)*(output->wh/5);
	
	for (; xi<xf; xi++)
	{
		for(aux = yi; aux < yf; aux++)
			n[(aux*output->ww) + xi].output = value;
	}
}

void
draw_output (char *output_name, char *input_name)
{
	OUTPUT_DESC *output;
	INPUT_DESC *input;
	
	if ((label >= 0) && (label <= 9))
	{
		output = get_output_by_name (output_name);
		input = get_input_by_name (input_name);
	
		set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, label * NUM_COLORS);
		
		update_output_image (output);
		glutSetWindow(output->win);
		glutPostWindowRedisplay (output->win);
	}
	
}


void 
init_character_input (INPUT_DESC *input)
{
	int x, y;
	float f;
	char file_name[256];

	strcpy (file_name, input->name);
	strcat (file_name, ".in");
		
	if ((input->file = fopen (file_name, "r")) == NULL)
		Erro ("cannot open input file: ", file_name, "");
		
	char_image_file_size = open_char_input_image (input, &char_image_file);
	char_label_file_size = open_char_input_label (input, &char_label_file);
	
	if (char_image_file_size != char_label_file_size)
	{
		printf ("Image and label files with diferent sizes\n");
		exit (-1);
	}
	
	if ((char_image_file_size == 0) || (char_label_file_size == 0))
	{
		printf ("Empty input file\n");
		exit (-1);
	}
	
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
		init_character_input (input);
		flag = 1;
	}
	
	if (status == MOVE)
	{
		if (input->wx >= 0)
			get_new_char (input, 1);
		else
			get_new_char (input, -1);
		all_filters_update ();
	}
	
}


int
evaluate_output (OUTPUT_DESC *output, int *result)
{
	NEURON *n;
	int xi, yi, xf, yf, aux;
	int number, selected_number;
	int greater = 0;
	
	n = output->neuron_layer->neuron_vector;
	for (number = 0; number < 10; number++)
	{
		result[number] = 0;
		
		if ((number >= 0) && (number <= 4))
		{
			xi = 0;
			xf = output->ww/2;
		}
		else if ((number >= 5)&&(number <= 9))
		{
			xi = output->ww/2;
			xf = output->ww;
		}

		yi = (number%5)*(output->wh/5);
		yf = ((number%5)+1)*(output->wh/5);

		for (; xi<xf; xi++)
		{
			for(aux = yi; aux < yf; aux++)
			{
				if (n[(aux*output->ww) + xi].output > NUM_COLORS/2)
					result[number] += 1;
				if (result[number] > greater)
				{
					greater = result[number];
					selected_number = number;
				}
			}
		}
	}
	return (selected_number);
}


void
output_handler (OUTPUT_DESC *output, int status)
{
	static int flag = 0;
	int result[10];
	int i, number;
	static int correct = 0, incorrect = 0;
	
	if (flag == 0)
	{
		flag = 1;
		return;
	}
	if (flag == 1)
	{
		number = evaluate_output (output, result);
		for (i = 0; i < 10; i++)
		{
			printf ("%d=%0.2f  ", i, (float) result[i] / ((float) (output->ww * output->wh)/10.0));
		}
		if (number == label)
		{
			correct++;
			printf ("-> %d == %d (%0.4f)\n", number, label, (float) correct / (float) (correct+incorrect));
		}
		else
		{
			incorrect++;
			printf ("-> %d != %d (%0.4f)\n", number, label, (float) correct / (float) (correct+incorrect));
		}
		flag = 1;
	}
	else
		flag++;
}
