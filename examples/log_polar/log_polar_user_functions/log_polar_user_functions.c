#include "mae.h"



int init_user_functions ()
{
	char strCommand[128];
	
    	sprintf (strCommand, "toggle move_active;");
    	interpreter (strCommand);

    	sprintf (strCommand, "toggle draw_active;");
    	interpreter (strCommand);

	return (0);
}



void load_input_image (INPUT_DESC *input)
{
	glutSetWindow(input->win);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, input->ww, input->wh, GL_RGB, GL_UNSIGNED_BYTE, input->image); 
	glDisable(GL_READ_BUFFER);
}



void init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;
	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;
}



void input_generator (INPUT_DESC *input, int status)
{
	if ((input->win != 0) && (status == MOVE))
	{
		update_input_neurons (input);
		check_input_bounds (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		input_display ();
		all_filters_update ();
		all_outputs_update ();
  	}
	
	if (input->win == 0)
	{
		int x, y;

		make_input_image (input);
			
		init (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);

		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (input_display); 
		glutKeyboardFunc (keyboard);
		glutPassiveMotionFunc (input_passive_motion);
		glutMouseFunc (input_mouse);		
	}
}



void draw_output (char *output_name, char *input_name)
{
    return;
}



void input_controler (INPUT_DESC *input, int status)
{
	char command[256];
					
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		sprintf (command, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (command);
		
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		
	}
	
	input->mouse_button = -1;
}



void f_keyboard (char *key_value)
{
	switch (key_value[0]) 
	{
		case 'A':
		case 'a':
			break;
	}
	
	return;
}
