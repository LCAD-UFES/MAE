#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "mae.h"
#define NIL -1

		
#ifndef PI
#define PI 3.1415926535
#endif



FILE *char_file;
int label = NIL;
int cont_char = 0;
int char_file_size;
 

void 
f_keyboard (char *key_value)
{
}



int 
init_user_functions()
{
	char strCommand[128];

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

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



unsigned int
read_data (FILE *arq, int data_size)
{
	unsigned int val;
	
	if ((fread (&val, 1, data_size, arq)) != data_size)
	{
		printf ("Error! Could not read input file\n");
		exit (-1);
	}
	return (val);
}



void
make_image (INPUT_DESC *input, int w, int h)
{
	char message[256];
	
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	input->ww = w;
	input->wh = h;

	switch(TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vph = h;
			input->vpw = w;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if(input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}



int 
open_char_input (INPUT_DESC *input, FILE **char_file)
{
	int i;
	struct stat f_stat;
	char file_name[256];
	FILE *input_file;

	strcpy (file_name, input->name);
	strcat (file_name, ".in");
	if ((input_file = fopen(file_name, "r")) == NULL)
		Erro ("Cannot open input file: ", file_name, "");
	fgets (file_name, 250, input_file);
	fclose(input_file);
	
	i = 0;
	while (isgraph (file_name[i]))
		i++;
	file_name[i] = '\0';
	printf("File_Name:%s\n", file_name);
	if (( *(char_file) = fopen (file_name, "r")) == NULL)
	{
		printf ("Error! Could not open %s\n", file_name);
		exit (-1);
	}

	fstat (fileno (*char_file), &f_stat);
	return ((int) f_stat.st_size);
}



void
read_char_input (INPUT_DESC *input, FILE *char_file, int direction)
{
	if ((direction == -1) && (cont_char > 1))
	{
		fseek (char_file, -2, SEEK_CUR);
		
		input->string[0] = read_data (char_file, 1);
		input->string[1] = '\0';
		
		cont_char--;
	}
	else if ((direction == 1) /*&& (cont_char < char_file_size)*/)
	{
		input->string[0] = read_data (char_file, 1);
		input->string[1] = '\0';
		
		cont_char++;
		if (cont_char == char_file_size)
		{
			fclose (char_file);
			char_file_size = open_char_input (input, &char_file);

			if (char_file_size == 0)
			{
				printf ("Empty input file\n");
				exit (-1);
			}
			cont_char = 0;
		}
	}
}



void
get_new_char (INPUT_DESC *input, int direction)
{
	read_char_input (input, char_file, direction);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	
	glutSetWindow (input->win);
	glutPostWindowRedisplay (input->win);
}



void 
char_input_init (INPUT_DESC *input)
{    
	glClearColor (0, 0, 0, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glShadeModel(GL_FLAT);
}



void 
char_input_reshape (int w, int h)
{
	INPUT_DESC *input;
	double winput, hinput, d, ang;

	input = get_input_by_win (glutGetWindow ());
	winput = (GLdouble) input->vpw;
	hinput = (GLdouble) input->vph;
	d = sqrt(winput*winput + hinput*hinput);
	ang = 2.5 * ((atan2 (hinput/2.0, d) * 180.0)/PI);
	
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (ang, (GLfloat)w/(GLfloat)h, 100.0*d, d);
}



void 
char_input_display (void)
{
	INPUT_DESC *input;
	GLdouble w, h, d;
	int i;
	
	input = get_input_by_win (glutGetWindow ());

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0, 
		   0.0, 0.0, -d,
		   0.0, 1.0, 0.0);

	glClear (GL_COLOR_BUFFER_BIT);

	glColor3f (1.0, 1.0, 1.0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(d/5.0);
	
	glPushMatrix();
	glScalef (w / ((104.76 / (119.05 + 25.0)) * ((GLdouble) strlen (input->string)) * h), 1.0, 1.0);
	glTranslatef(-((GLdouble) strlen (input->string)) * 104.76/2.0, -119.05/2.5, -8 * d);
	for (i = 0; i < strlen (input->string); i++)
		glutStrokeCharacter (GLUT_STROKE_MONO_ROMAN, input->string[i]);
	glPopMatrix();	
	
	glutSwapBuffers ();	
	
	input->waiting_redisplay = 0;
}



void 
init_character_input (INPUT_DESC *input)
{
	int x, y;
	float f;
		
	char_file_size = open_char_input (input, &char_file);
	if (char_file_size == 0)
	{
		printf ("Empty input file\n");
		exit (-1);
	}
	input->string = (char *) alloc_mem (1024 * sizeof (char));
	read_char_input (input, char_file, 1);
	strcpy(input->string, "Paca Tatu");
	
	make_image (input, input->neuron_layer->dimentions.x, input->neuron_layer->dimentions.y);
	
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
	char_input_init (input);
	glutReshapeFunc (char_input_reshape);
	glutDisplayFunc (char_input_display);
	glutMouseFunc (input_mouse);
	glutPassiveMotionFunc (NULL);
	glutKeyboardFunc (keyboard);
}



void
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	
	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (command, "draw char_discr_out based on character move;");
		interpreter (command);
	}

	if ((input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (move_active == 1))
	{
		sprintf (command, "move character to %d, %d;", input->wxd, input->wyd);
		interpreter (command);
	}
	input->mouse_state = -1;
}



void
update_input_neurons_on_character (INPUT_DESC *input)
{
	int i, j, w, h, index, r, g, b;
	GLubyte *image;
	
	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;
	image = input->image;

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			index = 3 * (j * w + i);
			r = image[index + 0];
			g = image[index + 1];
			b = image[index + 2];
			
			switch (input->neuron_layer->output_type)
			{
				case COLOR:
					input->neuron_layer->neuron_vector[j * w + i].output.ival = PIXEL(r, g, b); 
					break;
				case GREYSCALE:
					input->neuron_layer->neuron_vector[j * w + i].output.ival = (r + g + b) / 3; 
					break;  
				case BLACK_WHITE:
					input->neuron_layer->neuron_vector[j * w + i].output.ival = r > 50? NUM_COLORS - 1: 0;
					break;
				case GREYSCALE_FLOAT:
					input->neuron_layer->neuron_vector[j * w + i].output.fval = (float) (r + g + b) / 3.0; 
					break;
			}
		}
	}
}



void
input_generator (INPUT_DESC *input, int status)
{
	static int flag = 0;
		
	if (flag == 0)
	{
		init_character_input (input);		
		update_input_neurons_on_character (input);
		flag = 1;
	}
	
	if (status == MOVE)
	{
		if (input->wx >= 0)
			get_new_char (input, 1);
		else
			get_new_char (input, -1);
		update_input_neurons_on_character (input);
		all_filters_update ();
		all_outputs_update ();
	}
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
			n[(aux*output->ww) + xi].output.ival = value;
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
	
		set_neurons (output->neuron_layer->neuron_vector, 0, output->ww * output->wh, 0);
		
		user_set_neurons (output->neuron_layer->neuron_vector, output, label, NUM_COLORS-1);			
				
		update_output_image (output);
		glutSetWindow(output->win);
		glutPostWindowRedisplay (output->win);
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
				if (n[(aux*output->ww) + xi].output.ival > NUM_COLORS/2)
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
output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
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
	if (type_call == FORWARD)
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
}

void
top_handler (OUTPUT_DESC *output, int status)
{
}
