#include <stdlib.h>
#include "calibrate.h"

int left_eye, right_eye;
 
int correction     = 0;
int current_kernel = 0;
int side_move      = 0;

void update_filters_output (int status);
int calculate_disparity (int size);
void set_vergence (INPUT_DESC *image_left, INPUT_DESC *image_right);
//void micro_saccade (INPUT_DESC *image_left, INPUT_DESC *image_right);
int init (INPUT_DESC *input);
void input_generator (INPUT_DESC *input, int status);
void input_controler (INPUT_DESC *input, int status);
void new_output_passive_motion (int x, int y);
void move_x ();
void set_initial_position ();
void f_keyboard (char *key_value);
NEURON_OUTPUT v1_to_image_mapping (PARAM_LIST *param_list);
NEURON_OUTPUT verge_left_to_right_image_point (PARAM_LIST *param_list);
NEURON_OUTPUT draw_disparity_map (PARAM_LIST *param_list);



int
init_user_functions ()
{
	INPUT_DESC *image_left = NULL;
	INPUT_DESC *image_right = NULL;
	char command[256];
	
	image_left  = get_input_by_name ("image_left");
	image_right = get_input_by_name ("image_right");
	
	image_right->wxd_old = image_right->wxd = image_right->ww / 2;
	image_right->wyd_old = image_right->wyd = image_right->wh / 2;
	
	sprintf (command, "move %s to %d, %d;", image_right->name, image_right->wxd, image_right->wyd);
	interpreter (command);

	image_left->wxd_old = image_left->wxd = image_left->ww / 2;
	image_left->wyd_old = image_left->wyd = image_left->wh / 2;
	
	sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
	interpreter (command);
	
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
calculate_disparity (int size)
{
	static NEURON_LAYER *disparity = NULL;
	static NEURON_LAYER *complex_cells_distribution = NULL;

	int i,j;
	int w, h;
	double sum = 0.0;
	int neurons_active = 0;
	
	if (disparity == (NEURON_LAYER *) NULL)
	{
		disparity = get_neuron_layer_by_name("disparity");
		complex_cells_distribution = get_neuron_layer_by_name("complex_cells_distribution");
	}
	
	w = disparity->dimentions.x;
	h = disparity->dimentions.y;
	
	for (i = (w / 4); i < (3 * (w / 4)); i++)
	{
		for (j = 0; j < h; j++)
		{
			if ((complex_cells_distribution->neuron_vector[j * w + i].output.ival == size) ||
			    (disparity->neuron_vector[j * w + i].output.fval != 0.0))
			{
				sum += disparity->neuron_vector[j * w + i].output.fval;
				neurons_active++;
			}
		}
	}
	
	sum /= (double) (neurons_active);
	
	if (sum > 0.0)
		sum += 0.5;
	else
		sum -= 0.5;
		
	return (int) (-sum);
}



void
set_vergence (INPUT_DESC *image_left, INPUT_DESC *image_right)
{
	char command[256];
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	int i = 0;
	int disparity;
			
	update_filters_output (MOVE);
	
	/* ---------------------First Step--------------------- */		
	image_left->wxd += calculate_disparity(BIG);
	
	sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
	interpreter (command);

	/* ---------------------Second Step---------------------*/		
	image_left->wxd += calculate_disparity(MEDIUM);
	
	sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
	interpreter (command);

	/* ---------------------Third Step---------------------*/
	while (((disparity = calculate_disparity(SMALL)) != 0) && (i < 10))
	{
		image_left->wxd += disparity;
	
		sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
		interpreter (command);
		i++;
	}
	
	right_point.x = (double) (image_right->wxd);
	right_point.y = (double) (image_right->wyd);
				
	left_point.x = (double) (image_left->wxd);
	left_point.y = (double) (image_left->wyd);
	
	world_point = calculate_world_point (left_point, right_point, image_right->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);

	printf ("%f %f %f\n", world_point.x, world_point.y, world_point.z);
}



/*void
micro_saccade (INPUT_DESC *image_left, INPUT_DESC *image_right)
{
	char command[256];
	static NEURON_LAYER *simple_cell = (NEURON_LAYER *) NULL;
	float val, max_val = -1.0;
	int max_u = -1 , max_v = -1;
	int u, v, w, h;
	int xi, yi, wi, hi;
	
	if (simple_cell == (NEURON_LAYER *) NULL)
		simple_cell = get_neuron_layer_by_name ("simple_cell_nl_2");
	
	w = simple_cell->dimentions.x;
	h = simple_cell->dimentions.y;
	
	wi = image_left->neuron_layer->dimentions.x;
	hi = image_left->neuron_layer->dimentions.y;
	
	for (v = 0; v < h; v++)
	{
		for (u = w/4; u < 3*w/4; u++)
		{
		
			val = abs (simple_cell->neuron_vector[u + w * v].output.fval);
		
			if (val > max_val)
			{
				max_val = val;
				max_u = u;
				max_v = v;
			}
		}
	
	}
	
	map_v1_to_image (&(xi), &(yi), wi, hi, max_u, max_v, w, h, image_right->wxd, image_right->wyd);
	
	update_filters_output (MOVE);
	
	sprintf (command, "move %s to %d, %d;", image_right->name, xi, yi);
	interpreter (command);
	
	sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, yi);
	interpreter (command);
}*/



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
		image_left  = get_input_by_name ("image_left");
		image_right = get_input_by_name ("image_right");
	}
	
	if (((input->win == left_eye) || (input->win == right_eye)) && (input->win != 0))
	{
		glutSetWindow (input->win);
		glutPostWindowRedisplay (input->win);
		
		//update_input_image (input);
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
		int x, y;

		//load_input (input);
		make_input_image (input);
		init (input);
		update_input_neurons (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		if (strcmp (input->name, "image_left") == 0)
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
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
					
	if (image_left == NULL)
	{
		image_left  = get_input_by_name ("image_left");
		image_right = get_input_by_name ("image_right");
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
		
		//micro_saccade (image_left, image_right);
		
		set_vergence(image_left, image_right);
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == left_eye))
	{
		image_left->wyd = image_right->wyd + correction;

		sprintf (command, "move %s to %d, %d;", image_left->name, image_left->wxd, image_left->wyd);
		interpreter (command);
		
		right_point.x = (double) (image_right->wxd);
		right_point.y = (double) (image_right->wyd);

		left_point.x = (double) (image_left->wxd);
		left_point.y = (double) (image_left->wyd);

		world_point = calculate_world_point (left_point, right_point, image_right->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);

		printf ("%f %f %f\n", world_point.x, world_point.y, world_point.z);
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
		image_left = get_input_by_name ("image_left");
	
	x_current = image_left->wxd + side_move;
	sprintf (command, "move %s to %d, %d;", image_left->name, x_current, image_left->wyd);
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
new_output_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	static INPUT_DESC *image_right = NULL;
	static OUTPUT_DESC *output = NULL;
	NEURON_OUTPUT pixel;
	int r, g, b;
	int w;
	
	if (image_right == NULL)	
	{
		image_right = get_input_by_name ("image_right");
		output = get_output_by_win (glutGetWindow ());
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
	
	w = output->neuron_layer->dimentions.x;
	
	pixel = output->neuron_layer->neuron_vector[w * (int) (wy) + (int) (wx)].output;
	
	switch (output->neuron_layer->output_type)
	{
		case COLOR:
			r = RED   (pixel.ival);
			g = GREEN (pixel.ival);
			b = BLUE  (pixel.ival);
			
			sprintf (color_val, "r = %03d, g = %03d, b = %03d (color)", r, g, b);
			break;
		case GREYSCALE:
			sprintf (color_val, "intensity = %d (greyscale)", pixel.ival);
			break;
		case BLACK_WHITE:
			sprintf (color_val, "value = %d (b&w)", pixel.ival/ 255);
			break;	
		case GREYSCALE_FLOAT:
			sprintf (color_val, "intensity = %f (greyscale_float)", pixel.fval);
			break;	
	}
	set_color_bar (color_val);
		
	map_v1_to_image (&(image_right->green_cross_x), &(image_right->green_cross_y), 
		         image_right->ww, image_right->wh, (int) wx, (int) wy, output->ww, output->wh, 
			 image_right->wxd, image_right->wyd, (float) output->wh / (float) (output->wh - 1), SERENO_MODEL);

	glutPostWindowRedisplay (image_right->win);		 
}



/*void
generate_shape (INPUT_DESC *i_left_eye, INPUT_DESC *i_right_eye, FILTER_DESC *simple_cells, OUTPUT_DESC *disparity)
{
	int u, v, w, h;
	int xi, yi, previous_xi, previous_yi, wi, hi;
	float correction;
	float d;
	float theta;
	float pixels_disparity;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	FILE *gnuplot_file;
	
	if ((gnuplot_file = fopen ("shape_gnuplot.dat", "w")) == NULL)
	{
		Erro ("Could not open file shape_gnuplot.dat", "", "");
		return;
	}
	
	w  = disparity->neuron_layer->dimentions.x;
	h  = disparity->neuron_layer->dimentions.y;
	wi = i_right_eye->neuron_layer->dimentions.x;
	hi = i_right_eye->neuron_layer->dimentions.y;

	correction = (float) h / (float) (h - 1);
	
	previous_xi = -1;
	previous_yi = -1;
	
	receptive_field_descriptor = (RECEPTIVE_FIELD_DESCRIPTION *) simple_cells->private_state;
	
	for (v = 0; v < w/2; v++) 
	{
		d = distance_from_image_center (wi, hi, w, h, w/2 - 1 - v);
		
		for (u = 0; u < h; u++) 
		{
			theta = pi * (((float) h * (3.0 / 2.0) - ((float) u * correction)) / (float) h);
			xi = (int) (d * cos(theta) + 0.5);
			yi = (int) (d * sin(theta) + 0.5);
			
			if ((xi == previous_xi) && (yi == previous_yi))
				continue;
			else
			{
				pixels_disparity = disparity->neuron_layer->neuron_vector[(u * w) + v].output.fval;
				
				right_point.x = (float) (i_right_eye->wxd + xi);
				right_point.y = (float) (i_right_eye->wyd + yi);
				
				left_point.x = (float) (i_left_eye->wxd + xi) - pixels_disparity;
				left_point.y = (float) (i_left_eye->wyd + yi);
				
				world_point = calculate_world_point (left_point, right_point, i_right_eye->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);
				
				fprintf (gnuplot_file, "%f %f %f\n", world_point.x, world_point.y, world_point.z);
			}
			
			previous_xi = xi;
			previous_yi = yi;
		}
	}
	
	for (v = w/2; v < w; v++) 
	{
		d = distance_from_image_center (wi, hi, w, h, v - w/2);
		
		for (u = 0; u < h; u++) 
		{
			theta = pi * (((float) h * (3.0 / 2.0) + ((float) u * correction)) / (float) h);
			xi = (int) (d * cos(theta) + 0.5);
			yi = (int) (d * sin(theta) + 0.5);
			
			if ((xi == previous_xi) && (yi == previous_yi))
				continue;
			else
			{				
				pixels_disparity = disparity->neuron_layer->neuron_vector[(u * w) + v].output.fval;
				
				right_point.x = (float) (i_right_eye->wxd + xi);
				right_point.y = (float) (i_right_eye->wyd + yi);
				
				left_point.x = (float) (i_left_eye->wxd + xi) - pixels_disparity;
				left_point.y = (float) (i_left_eye->wyd + yi);
				
				world_point = calculate_world_point (left_point, right_point, i_right_eye->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);
				

				fprintf (gnuplot_file, "%f %f %f\n", world_point.x, world_point.y, world_point.z);		
			}
			
			previous_xi = xi;
			previous_yi = yi;
			
		}
	}	

	if (fclose (gnuplot_file) != 0)
		Erro ("Could not close file shape_gnuplot.dat", "", "");
}*/



/*void	
output_handler_disparity (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char command[256];
	static INPUT_DESC *i_right_eye   = NULL;
	static INPUT_DESC *i_left_eye 	 = NULL;
	static FILTER_DESC *simple_cells = NULL;
	
	if (i_right_eye == NULL)
	{
		i_left_eye   = (INPUT_DESC *) output->output_handler_params->next->param.pval;
		i_right_eye  = (INPUT_DESC *) output->output_handler_params->next->next->param.pval;
		
		simple_cells = get_filter_by_name ("simple_cell_nl_1_filter");
	}
	
	output->mouse_button = NO_BUTTON;
	
	generate_shape (i_left_eye, i_right_eye, simple_cells, output);
}*/



void 
f_keyboard (char *key_value)
{
	INPUT_DESC *image_right = NULL;
	//INPUT_DESC *image_left  = NULL;
	OUTPUT_DESC *output	= NULL;  
	char command[256];
	char key;
	
	key = key_value[0];
	switch (key) 
	{
		case 'a':
			image_right = get_input_by_name ("image_right");
			image_right->green_cross = 1;
						
			if ((output = get_output_by_win (glutGetWindow ())) != NULL)
			{
				glutSetWindow (output->win);
			
				glutPassiveMotionFunc (new_output_passive_motion);

				glutPostWindowRedisplay (image_right->win);
			}
			break;

		case 'A':
			image_right = get_input_by_name ("image_right");
			image_right->green_cross = 0;
			
			if ((output = get_output_by_win (glutGetWindow ())) != NULL)
			{
				glutSetWindow (output->win);
			
				glutPassiveMotionFunc (output_passive_motion);
			
				glutPostWindowRedisplay (image_right->win);
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
		case 'q':
			CAMERA_DISTANCE += 0.01;
			printf("CAMERA_DISTANCE = %f\n", CAMERA_DISTANCE);
			break;
		case 'w':
			CAMERA_DISTANCE -= 0.01;
			printf("CAMERA_DISTANCE = %f\n", CAMERA_DISTANCE);
			break;
		case 'e':
			FOCAL_DISTANCE += 50.0;
			printf("FOCAL_DISTANCE = %f\n", FOCAL_DISTANCE);
			break;
		case 'r':
			FOCAL_DISTANCE -= 50.0;
			printf("FOCAL_DISTANCE = %f\n", FOCAL_DISTANCE);
			break;
		case 'v':
			sprintf (command, "vergence (0);");
			interpreter (command);
			break;
	}
					
	update_filters_output (MOVE);
}



NEURON_OUTPUT
verge_left_to_right_image_point (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	static INPUT_DESC *image_left  = NULL;
	static INPUT_DESC *image_right = NULL;
	int xi, yi;
	int i = 0;
	int disparity;

	if (image_right == (INPUT_DESC *) NULL)
	{
		image_left  = get_input_by_name ("image_left");
		image_right = get_input_by_name ("image_right");
	}
	
	/* -------Get the New Coordinates of Image Right------ */
	xi = param_list->next->param.ival;
	yi = param_list->next->next->param.ival;
	
	/* -------Set the New Coordinates of Image Right------ */
	image_right->wxd_old = image_right->wxd;
	image_right->wyd_old = image_right->wyd;
	image_right->wxd = xi;
	image_right->wyd = yi;
	
	/* -------Set the New Coordinates of Image Left------ */
	image_left->wxd += image_right->wxd - image_right->wxd_old;
	image_left->wyd = image_right->wyd + correction;
	
	/* -------Move the Right Image------ */
	move_input_window (image_right->name, image_right->wxd, image_right->wyd);
	
	/* -------Move the Right Image------ */
	move_input_window (image_left->name, image_left->wxd, image_left->wyd);
	
	/* ----------------First Step of Vergence------------- */		
	image_left->wxd += calculate_disparity(BIG);
	
	move_input_window (image_left->name, image_left->wxd, image_left->wyd);

	/* ----------------Second Step of Vergence-------------*/		
	image_left->wxd += calculate_disparity(MEDIUM);
	
	move_input_window (image_left->name, image_left->wxd, image_left->wyd);


	/* --------------Third Step of Vergence----------------*/
	image_left->wxd += calculate_disparity(SMALL);
	
	move_input_window (image_left->name, image_left->wxd, image_left->wyd);
	
	/* ---------------------Third Step---------------------*/
	while (((disparity = calculate_disparity(SMALL)) != 0) && (i < 10))
	{
		image_left->wxd += disparity;
		
		move_input_window (image_left->name, image_left->wxd, image_left->wyd);
		i++;
	}

	output.ival = (image_left->wxd - image_right->wxd);
	
	return (output);
}


