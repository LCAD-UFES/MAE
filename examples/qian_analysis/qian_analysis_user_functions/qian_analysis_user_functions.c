#include <stdlib.h>
#include "mae.h"

int left_eye, right_eye;
 
int correction     = 0;
int current_kernel = 0;
int side_move      = 0;



int
init_user_functions ()
{
	return (0);
}



void
update_filters_output (int status)
{
	all_filters_update ();
	if (status == MOVE)
		all_outputs_update ();
}



int 
init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;
	
	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;
}



void
input_generator (INPUT_DESC *input, int status)
{
	static INPUT_DESC *image_left = NULL;
	static INPUT_DESC *image_right = NULL;
					
	if (image_left == NULL)
	{
		image_left  = get_input_by_name ("stimulus_left");
		image_right = get_input_by_name ("stimulus_right");
	}
	
	if (((input->win == left_eye) || (input->win == right_eye)) && (input->win != 0))
	{
		glutSetWindow (input->win);
		glutPostWindowRedisplay (input->win);
		
		update_input_neurons (input);
		
		if (status == MOVE)
		{
			input->wxd_old = input->wxd;
			input->wyd_old = input->wyd;
			input->wxd = input->wx + (input->ww / 2);
			input->wyd = input->wy + (input->wh / 2);
		}
		
		check_input_bounds (input, input->wxd, input->wyd);
		update_filters_output (status);
	}

	if (input->win == 0)
	{
		float f = 1.0;
		int x, y;

		make_input_image (input);
		init (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		if (strcmp (input->name, "stimulus_left") == 0)
			left_eye = input->win;
		else
			right_eye = input->win;
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
draw_output (char *output_name, char *input_name)
{		
	
}



void
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	static INPUT_DESC *image_left  = NULL;
	static INPUT_DESC *image_right = NULL;
	int previous_x;
	int disparity;
					
	if (image_left == NULL)
	{
		image_left  = get_input_by_name ("stimulus_left");
		image_right = get_input_by_name ("stimulus_right");
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == right_eye))
	{
		
		image_left->wxd += image_right->wxd - image_right->wxd_old;
		image_left->wyd = image_right->wyd + correction;
	
		sprintf (command, "move %s to %d, %d;", image_right->name, image_right->wxd, image_right->wyd);
		interpreter (command);

		sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
		interpreter (command);
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == left_eye))
	{
		image_left->wyd = image_right->wyd + correction;

		sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
		interpreter (command);
	}
	
	input->mouse_button = -1;
}



void
move_x ()
{
	char command[256];
	static INPUT_DESC *image_left = NULL;
	int x_current;
	
	if (image_left == NULL)
		image_left = get_input_by_name ("stimulus_left");
	
	x_current = image_left->wxd + side_move;
	sprintf (command, "move %s to %d, %d;", image_left->name, x_current, image_left->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}



void 
f_keyboard (char *key_value)
{
	char key;
	
	key = key_value[0];
	switch (key) 
	{
		case 'z':
			side_move = -1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
		case 'x':
			side_move = 1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
	}
					
	update_filters_output (MOVE);
}
