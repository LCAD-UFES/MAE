#include <stdlib.h>
#include "mae.h"
#include "../qian.h"

int left_eye, right_eye;
int correction = 0;
int side_move = 0;



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



/* preenche-se esta estrutura na mão. O código normal trata de uma imagem estática e
usa-se o LoadBMP do Xview. Por enquanto, estou preenchendo apenas alguns 
valores, depois deve-se pensar melhor como fazer.*/

int 
init (INPUT_DESC *input)
{
	return (0);
}



void
input_generator (INPUT_DESC *input, int status)
{
	static INPUT_DESC *i_left_eye = NULL;
	static INPUT_DESC *i_right_eye = NULL;
					
	if (i_left_eye == NULL)
	{
		i_left_eye = get_input_by_name ("image_left");
		i_right_eye = get_input_by_name ("image_right");
	}
	
	if (((input->win == left_eye) || (input->win == right_eye)) && (input->win != 0))
	{
		glutSetWindow(input->win);
		glutPostWindowRedisplay(input->win);
		update_input_neurons (input);
		
		if (status == MOVE)
		{
			input->wxd_old = input->wxd;
			input->wyd_old = input->wyd;
			input->wxd = input->wx + (input->ww / 2);
			input->wyd = input->wy + (input->wh / 2);
		}
		check_input_bounds (input, input->wx, input->wy);
		update_filters_output (status);
	}

	if (input->win == 0)
	{
		//float f = 1.0;
		int x, y;

		make_input_image (input);
		
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
input_controler (INPUT_DESC *input, int status)
{
	char command[256];
	static INPUT_DESC *i_left_eye = NULL;
	static INPUT_DESC *i_right_eye = NULL;
					
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
		i_left_eye->wyd = i_right_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_right_eye->name, i_right_eye->wxd, i_right_eye->wyd);
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
	
	input->mouse_button = -1;
}



void
draw_output (char *output_name, char *input_name)
{

	
}



void
move_x ()
{
	char command[256];
	static INPUT_DESC *i_left_eye = NULL;
	int x_current;
	
	if (i_left_eye == NULL)
		i_left_eye = get_input_by_name ("image_left");
	
	x_current = i_left_eye->wxd + side_move;
	sprintf (command, "move %s to %d, %d;", i_left_eye->name, x_current, i_left_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}


void 
filter_group_update (char *prefix)
{
	FILTER_LIST *f_list;
	int length = strlen(prefix);
	for (f_list = global_filter_list; f_list != NULL; f_list = f_list->next)
	{
		if (strncmp(f_list->filter_descriptor->name, prefix, length) == 0)
			filter_update (f_list->filter_descriptor);
	}	
}


void
compute_disparity_map (char *mt_filter, char *mt_gaussian_filter, char *disparity_map_filter)
{
//	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_INIT;
//	filter_group_update (disparity_map_filter);
	
	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_SAMPLE;
	for (DISPARITY = MIN_DISP; DISPARITY < MAX_DISP; DISPARITY++)
	{
		filter_group_update (mt_filter);
		filter_group_update (mt_gaussian_filter);
		filter_group_update (disparity_map_filter);
	}
	DISPARITY_MAP_FILTER_STATE = DISPARITY_MAP_FILTER_OPTIMIZE;
	filter_group_update (disparity_map_filter);
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
		case 'd':
			compute_disparity_map ("nl_mt_filter", "nl_mt_gaussian_filter", "nl_disparity_map_filter");
		break;
	}
					
	update_filters_output (MOVE);
}
