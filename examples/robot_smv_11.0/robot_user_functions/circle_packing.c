#include "circle_packing.h"
#include <stdlib.h>

#include <gsl/gsl_rng.h>
#include "robot.h"

#define POW2(a) ((a)*(a))

#define X_LEFT ((double)circles.neuron_layer->dimentions.x / 10.0)
#define X_RIGHT ((double)circles.neuron_layer->dimentions.x - (double)circles.neuron_layer->dimentions.x / 10.0)
#define AREA_REFERENCE ((double)circles.neuron_layer->dimentions.y / 10.0)
#define AREA_HIGHT ((double)circles.neuron_layer->dimentions.y - (double)circles.neuron_layer->dimentions.y / 10.0)

#define CIRCLE_INITIAL_SPEED_MAGNITUDE (-0.1)
#define COR (0.15) /* Coefficient Of Restitution */
#define MIN_SPEED (0.0005)
#define G (-0.0002) /* must be smaller than MIN_SPEED */
#define TEMP_FACTOR (0.999)

#define DRAWSQUARE(x, y, w, h) \
    glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd ()

#define	CROSS_UP_SIZE(x, w) (((x + w/10) < w)? x + w/10: w-1)
#define	CROSS_DOWN_SIZE(x, w) (((x - w/10) >= 0)? x - w/10: 0)

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); \
    		       glVertex2i ((x+w/50), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); \
    		       glVertex2i ((x), (y+h/50)); \
    glEnd (); \
}

#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); \
    		       glVertex2i ((x), (h+h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); \
    		       glVertex2i ((x), (-h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); \
    		       glVertex2i ((w+h/20), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); \
    		       glVertex2i ((-h/20), (y)); \
    glEnd (); \
}

struct _circle_list
{
	double x, y;
	double diameter;
	double speed_x;
	double speed_y;
	CYLINDER *associated_cylinder;
	struct _circle_list *next;
};

typedef struct _circle_list CIRCLE_LIST;


struct _boundaries
{
	/* Each boundary is a segment of line defined by two points */
	double x1, y1, x2, y2;
	struct _boundaries *next;
};

typedef struct _boundaries BOUNDARIES;


BOUNDARIES *global_boundaries_list = NULL;
CIRCLE_LIST *global_circle_list = NULL;
int global_initial_num_cylinders = 0;
int global_num_new_cylinders = 0;
int global_tries = 0;

CYLINDER_LIST *global_cylinder_list;
double cylinder_delta_x = -288.0;
double cylinder_delta_y = 432.0;
double theta = -0.4084;

/* GSL's random number generator. */
static gsl_rng* random_generator;
/*
Function Section
*/

/*
***********************************************************
* Function: make_input_image_circle_packing
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void
make_input_image_circle_packing (INPUT_DESC *input, int w, int h)
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



void
draw_circle (CIRCLE_LIST *circle)
{
	GLdouble vectorY1, vectorX1, vectorY, vectorX;
	double angle, radius;

	/* draw a circle from a bunch of short lines */
	radius = circle->diameter / 2.0;
	vectorY1 = circle->y + radius;
	vectorX1 = circle->x;
	glBegin(GL_LINE_STRIP);
	for(angle = 0.0f; angle <= (2.0*pi); angle += 2.0*pi/50.0)
	{
		vectorY = circle->y + (radius * cos (angle));
		vectorX = circle->x + (radius * sin (angle));
		glVertex2d (vectorX1, vectorY1);
		vectorY1 = vectorY;
		vectorX1 = vectorX;
	}
	glVertex2d (vectorX1, vectorY1);
	glEnd();
}



void
draw_circles ()
{
	CIRCLE_LIST *circle;

	if (global_circle_list == NULL)
		return;

	for (circle = global_circle_list; circle != NULL; circle = circle->next)
		draw_circle (circle);
}



void
draw_boundary (BOUNDARIES *boundary)
{
	glBegin(GL_LINE_STRIP);
		glVertex2d (boundary->x1, boundary->y1);
		glVertex2d (boundary->x2, boundary->y2);
	glEnd();
}



void
draw_boundaries ()
{
	BOUNDARIES *boundary;

	if (global_boundaries_list == NULL)
		return;

	for (boundary = global_boundaries_list; boundary != NULL; boundary = boundary->next)
	{
		draw_boundary (boundary);
	}
}



void
circle_packing_input_display (void)
{
	INPUT_DESC *input = &circles;
	GLdouble w, h, d;
	char info[256];
	int i;

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0,
		   0.0, 0.0, -d,
		   0.0, 1.0, 0.0);
	glTranslatef (-w/2.0, -h/2.0, -d);

	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (0.0, 0.0, 0.0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, 0.0); glVertex3i (0, 0, 0);
	glTexCoord2f (0.0, 1.0); glVertex3i (0, input->tfh, 0);
	glTexCoord2f (1.0, 1.0); glVertex3i (input->tfw, input->tfh, 0);
	glTexCoord2f (1.0, 0.0); glVertex3i (input->tfw, 0, 0);
	glEnd ();
	glDisable (GL_TEXTURE_2D);

	glColor3f (1.0, 0.0, 0.0);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	DRAWSQUARE (input->wx, input->wy, input->neuron_layer->dimentions.x, input->neuron_layer->dimentions.y);
	glDisable (GL_LINE_STIPPLE);

	if (move_active == 1)
		DRAW_XY_MARK(input->wxd, input->wyd, input->vpw, input->vph);

	if ((move_active == 1) && (draw_active == 1))
		DRAWCROSS(input->wxd, input->wyd, input->ww, input->wh);

	glColor3f (0.0, 1.0, 0.0);
	if (input->green_cross)
		DRAWCROSS(input->green_cross_x, input->green_cross_y, input->ww, input->wh);
	glColor3f (1.0, 0.0, 0.0);

	d = (GLdouble) input->vph / 64.0;
	if (d > ((GLdouble) input->vpw / 64.0))
		d = (GLdouble) input->vpw / 64.0;
	if (input->vpxo > 0)
		glBegin(GL_LINES); glVertex2f (-d, 0.0); glVertex2f (0.0, 0.0); glEnd ();
	if (input->vpxo + input->vpw < input->ww)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw+d, 0.0); glEnd ();
	if (input->vpyo + input->vph < input->wh)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw, -d); glEnd ();
	if (input->vpyo > 0)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph+d); glEnd ();

	switch (input->neuron_layer->output_type)
	{
		case COLOR:
			sprintf (info, "Color Input\r");
			break;
		case GREYSCALE:
			sprintf (info, "Greyscale Input\r");
			break;
		case BLACK_WHITE:
			sprintf (info, "Black & White Input\r");
			break;
	}

	draw_circles ();
	draw_boundaries ();

	glRasterPos2f (0.0, h + h / 20.0);

	for (i = 0; i < strlen (info); i++)
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, info[i]);

	glutSwapBuffers ();
	input->waiting_redisplay = 0;

	return;
}




/*
***********************************************************
* Function: input_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void
circles_input_generator (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_circle_packing(input);
	}
	else
	{
		if (status == MOVE)
		{
			// output_update(&nl_circle_packing_out);

			glutSetWindow (input->win);
			circle_packing_input_display ();
		}
	}
}



/*
***********************************************************
* Function: input_controler
* Description: handles the mouse input window events
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void
circles_input_controler (INPUT_DESC *input, int status)
{
	// char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		// sprintf (strCommand, "draw nl_circle_packing_out based on circle_packing move;");
		// interpreter (strCommand);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		// sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		// interpreter (strCommand);
	}

	input->mouse_button = -1;
}



double
random_val_within_range (double range_init, double range_end)
{
	return gsl_rng_uniform(random_generator) * (range_end - range_init) + range_init;


}



CIRCLE_LIST *
generate_initial_circle_list (CYLINDER_LIST *list)
{
	int i = 0, n = cylinder_list_size (list);
	CIRCLE_LIST *circle_list = NULL, *circle_list_head = NULL;

	global_initial_num_cylinders = 0;
	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get (list, i);
		if (circle_list_head == NULL)
		{
			circle_list_head = circle_list = (CIRCLE_LIST *) alloc_mem (sizeof (CIRCLE_LIST));
		}
		else
		{
			circle_list->next = (CIRCLE_LIST *) alloc_mem (sizeof (CIRCLE_LIST));
			circle_list = circle_list->next;
		}

		cylinder->z /= 10.0;
		cylinder->y /= 10.0;
		cylinder->diameter /= 10.0;
		cylinder->size /= 10.0;

		cylinder->z += cylinder_delta_x;
		cylinder->y += cylinder_delta_y;
		cylinder->z = cos (theta) * cylinder->z - sin (theta) * cylinder->y;
		cylinder->y = sin (theta) * cylinder->z + cos (theta) * cylinder->y;

		circle_list->associated_cylinder = cylinder;
		circle_list->x = cylinder->z;
		circle_list->y = cylinder->y;

		circle_list->diameter = cylinder->diameter;
		circle_list->speed_x = 0.0;
		circle_list->speed_y = 0.0;
		circle_list->next = NULL;

		global_initial_num_cylinders++;
	}

	return (circle_list_head);
}



void
free_circle_list (CIRCLE_LIST *circle_list_head)
{
	CIRCLE_LIST *current_circle, *aux;
	current_circle = circle_list_head;
	while (current_circle != NULL)
	{
		aux = current_circle->next;
		free (current_circle);
		current_circle = aux;
	}
}
BOUNDARIES *
generate_boundaries (CIRCLE_LIST *circle_list)
{
	BOUNDARIES *boundaries = NULL, *boundaries_head = NULL;
	CIRCLE_LIST *current_circle;
	double x1, y1, x2, y2;

	for (current_circle = circle_list; current_circle != NULL; current_circle = current_circle->next)
	{
		if (boundaries_head == NULL)
		{
			boundaries_head = boundaries = (BOUNDARIES *) alloc_mem (sizeof (BOUNDARIES));
		}
		else
		{
			boundaries->next = (BOUNDARIES *) alloc_mem (sizeof (BOUNDARIES));
			boundaries = boundaries->next;
		}
		/* Each boundary is a line segment defined by two points (x1, y1) and (x2, y2) */
		boundaries->y1 = y1 = current_circle->y;
		boundaries->x1 = x1 = current_circle->x;
		if (current_circle->next != NULL)
		{
			boundaries->x2 = x2 = current_circle->next->x;
			boundaries->y2 = y2 = current_circle->next->y;
		}
		else /* this closes a polygon */
		{
			boundaries->x2 = x2 = circle_list->x;
			boundaries->y2 = y2 = circle_list->y;
		}

		boundaries->next = NULL;
	}

	return (boundaries_head);
}
void
free_boundaries (BOUNDARIES *boudaries_head)
{
	BOUNDARIES *current_boundary, *aux;
	current_boundary = boudaries_head;
	while (current_boundary != NULL)
	{
		aux = current_boundary->next;
		free (current_boundary);
		current_boundary = aux;
	}
}



void
set_random_circle (CIRCLE_LIST *circle, BOUNDARIES *boundaries, CYLINDER_LIST *list)
{
	int chosen_cylinder = (int) random_val_within_range (0.0, (double) global_initial_num_cylinders);
	CYLINDER *first_cylinder;
	CYLINDER *last_cylinder;
	int i, n = cylinder_list_size (list);
	double angle_initial_speed;

	last_cylinder  = cylinder_list_get_last  (list);
	first_cylinder = cylinder_list_get_first (list);
	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get (list, i);
		if (i == chosen_cylinder)
		{
			circle->diameter = cylinder->diameter;
			circle->associated_cylinder = cylinder;
		}

		last_cylinder = cylinder;
	}

	/* cylinder_list points to the first cylinder */
	circle->x = random_val_within_range (first_cylinder->z + first_cylinder->diameter / 2.0 + circle->diameter / 2.0,
					     last_cylinder->z - last_cylinder->diameter / 2.0 - circle->diameter / 2.0);

	/* The first and the last cylinders define a line whose equation is */
	/* y = y1 + [(y2 - y1) / (x2 - x1)]�(x - x1) {http://www.cut-the-knot.org/Curriculum/Calculus/StraightLine.shtml} */
	/* where the center of the first cylinder is at (x1, y1), and of the last at (x2, y2) */
	/* The y coordinate of the circle is in this line */
	circle->y = first_cylinder->y + ((last_cylinder->y - first_cylinder->y) / (last_cylinder->z - first_cylinder->z)) * (circle->x - first_cylinder->z);

	angle_initial_speed = random_val_within_range (pi+pi/4.0, pi+pi/2.0+pi/4.0);
	circle->speed_x = CIRCLE_INITIAL_SPEED_MAGNITUDE * cos (angle_initial_speed);
	circle->speed_y = CIRCLE_INITIAL_SPEED_MAGNITUDE * sin (angle_initial_speed);
	circle->next = NULL;
}



int
boundary_will_be_crossed_by_circle (CIRCLE_LIST *circle, BOUNDARIES *boundary)
{
	double xp1 = boundary->x1;
	double yp1 = boundary->y1;
	double xp2 = boundary->x2;
	double yp2 = boundary->y2;

	double xp3 = circle->x;
	double yp3 = circle->y;
	double xp4 = circle->x + circle->speed_x;
	double yp4 = circle->y + circle->speed_y;

	double S1x = xp2 - xp1;
	double S1y = yp2 - yp1;

	double S2x = xp4 - xp3;
	double S2y = yp4 - yp3;

	double s = (-S1y * (xp1 - xp3) + S1x * (yp1 - yp3)) / (-S2x*S1y + S1x*S2y);
	double t = (S2x * (yp1 - yp3) - S2y * (xp1 - xp3)) / (-S2x*S1y + S1x*S2y);

	if ((0.0 <= s) && (s <= 1.0) && (0.0 <= t) && (t <= 1.0))
		return (1);
	else
		return (0);
}



double
circle_distance (CIRCLE_LIST *circle1, CIRCLE_LIST *circle2)
{
	return (sqrt ((circle1->x - circle2->x) * (circle1->x - circle2->x) +
		      (circle1->y - circle2->y) * (circle1->y - circle2->y)));
}



BOUNDARIES *
boundary_colision (CIRCLE_LIST *circle, BOUNDARIES *boundaries)
{
	BOUNDARIES *boundary_aux;

	for (boundary_aux = boundaries; boundary_aux->next != NULL; boundary_aux = boundary_aux->next)
	{
		if (boundary_will_be_crossed_by_circle (circle, boundary_aux))
		    	return (boundary_aux);
	}

	return (NULL);
}


CIRCLE_LIST *
circle_colision (CIRCLE_LIST *circle, CIRCLE_LIST *circle_list)
{
	CIRCLE_LIST *circle_aux;

	for (circle_aux = circle_list; circle_aux != NULL; circle_aux = circle_aux->next)
	{
		if ((circle != circle_aux) &&
		    (circle_distance (circle, circle_aux) < (circle->diameter / 2.0 + circle_aux->diameter / 2.0)))
			return (circle_aux);
	}

	return (NULL);
}



int
add_new_circle (CIRCLE_LIST *circle_list, BOUNDARIES *boundaries, CYLINDER_LIST *list)
{
	CIRCLE_LIST *circle, *aux;

	circle = (CIRCLE_LIST *) alloc_mem (sizeof (CIRCLE_LIST));
	set_random_circle (circle, boundaries, list);
	if (!circle_colision (circle, circle_list) && !boundary_colision (circle, boundaries))
	{
		for (aux = circle_list; aux->next != NULL; aux = aux->next)
			;
		aux->next = circle;
		global_num_new_cylinders++;
//		fprintf(stderr, "Cylinders: %d\n", global_num_new_cylinders + global_initial_num_cylinders);
		return (1);
	}
	else
	{
		if (global_tries < 100)
		{
			global_tries++;
			free (circle);
			return (1);
		}
		else
		{
			free (circle);
			return (0);
		}
	}
}



void
update_display (CIRCLE_LIST *circle_list, BOUNDARIES *boundaries_list)
{
	global_circle_list = circle_list;
	global_boundaries_list = boundaries_list;
	circle_packing_input_display ();
}



void
change_circle_position (CIRCLE_LIST *circle)
{
	circle->x += circle->speed_x;
	circle->y += circle->speed_y;
}



void
update_positions (CIRCLE_LIST *circle_list)
{
	CIRCLE_LIST *circle;

	for (circle = circle_list; circle != NULL; circle = circle->next)
		change_circle_position (circle);
}



void
change_circles_speed (CIRCLE_LIST *circle, CIRCLE_LIST *circle_hit)
{
	double n_x, n_y, mn, un_x, un_y, ut_x, ut_y;
	double v1_x, v1_y, v1n, v1t, mv1, nv1_x, nv1_y, m1;
	double v2_x, v2_y, v2n, v2t, mv2, nv2_x, nv2_y, m2;
#define REPULSAO (mn / 25.0 + (exp (mn/5.0)-1.0)/8.0)

	n_x = circle_hit->x - circle->x;
	n_y = circle_hit->y - circle->y;
	mn = sqrt (n_x * n_x + n_y * n_y);
	un_x = n_x / mn;
	un_y = n_y / mn;
	ut_x = -un_y;
	ut_y = un_x;
	v1_x = circle->speed_x;
	v1_y = circle->speed_y;
	mv1 = sqrt (v1_x * v1_x + v1_y * v1_y);
	nv1_x = v1_x / mv1;
	nv1_y = v1_y / mv1;
	v2_x = circle_hit->speed_x;
	v2_y = circle_hit->speed_y;
	mv2 = sqrt (v2_x * v2_x + v2_y * v2_y);
	nv2_x = v2_x / mv2;
	nv2_y = v2_y / mv2;

	if (mv2 == 0.0)
	{
		v1n = v1_x * un_x + v1_y * un_y;
		if (v1n < 0.0) /* the circles are getting further appart */
			return;
		v1t = v1_x * ut_x + v1_y * ut_y;
		if (mn < (circle->diameter/2.0 + circle_hit->diameter/2.0))
			mn = circle->diameter/2.0 + circle_hit->diameter/2.0 - mn; /* repulsion when mixed */
		else
			mn = 0.0;
		v1n = -COR * v1n - REPULSAO;

		circle->speed_x = v1n * un_x + v1t * ut_x;
		circle->speed_y = v1n * un_y + v1t * ut_y;
	}
	else
	{
		v1n = v1_x * un_x + v1_y * un_y;
		v2n = v2_x * un_x + v2_y * un_y;
		if ((v1n - v2n) < 0.0) /* the circles are getting further appart */
			return;
		v1t = v1_x * ut_x + v1_y * ut_y;
		v2t = v2_x * ut_x + v2_y * ut_y;
		m1 = circle->diameter;
		m2 = circle_hit->diameter;
		/* After colision v1t and v2t do not change and v1n and v2n: */
		if (mn < (circle->diameter/2.0 + circle_hit->diameter/2.0))
			mn = circle->diameter/2.0 + circle_hit->diameter/2.0 - mn; /* repulsion when mixed */
		else
			mn = 0.0;
		v1n = (v1n * (m1 - COR * m2) + (1.0 + COR) * m2 * v2n) / (m1 + m2) - REPULSAO;
		v2n = (v2n * (m2 - COR * m1) + (1.0 + COR) * m1 * v1n) / (m1 + m2) + REPULSAO;

		circle->speed_x = v1n * un_x + v1t * ut_x;
		circle->speed_y = v1n * un_y + v1t * ut_y;
		circle_hit->speed_x = v2n * un_x + v2t * ut_x;
		circle_hit->speed_y = v2n * un_y + v2t * ut_y;
	}
}



void
change_circle_speed (CIRCLE_LIST *circle, BOUNDARIES *boundary_hit)
{
	double n_x, n_y, mn, un_x, un_y, ut_x, ut_y;
	double v1_x, v1_y, v1n, v1t, mv1, nv1_x, nv1_y;

	n_x = -(boundary_hit->y1 - boundary_hit->y2);
	n_y = boundary_hit->x1 - boundary_hit->x2;
	mn = sqrt (n_x * n_x + n_y * n_y);
	un_x = n_x / mn;
	un_y = n_y / mn;
	ut_x = -un_y;
	ut_y = un_x;
	v1_x = circle->speed_x;
	v1_y = circle->speed_y;
	mv1 = sqrt (v1_x * v1_x + v1_y * v1_y);
	nv1_x = v1_x / mv1;
	nv1_y = v1_y / mv1;

	v1n = v1_x * un_x + v1_y * un_y;
	if (v1n > 0.0) /* the circle is getting further appart from the boundary */
		return;
	v1t = v1_x * ut_x + v1_y * ut_y;
	v1n = -COR * v1n;

	circle->speed_x = v1n * un_x + v1t * ut_x;
	circle->speed_y = v1n * un_y + v1t * ut_y;
}



void
update_velocities (CIRCLE_LIST *circle_list, BOUNDARIES *boundaries)
{
	CIRCLE_LIST *circle, *circle_hit;
	BOUNDARIES *boundary_hit;
	double mv;

	for (circle = circle_list; circle != NULL; circle = circle->next)
	{
		mv = sqrt (circle->speed_x * circle->speed_x + circle->speed_y * circle->speed_y);
		if (mv == 0.0)
			continue;

		circle->speed_y -= G;
		if ((circle_hit = circle_colision (circle, circle_list)) != NULL)
			change_circles_speed (circle, circle_hit);
	}

	for (circle = circle_list; circle != NULL; circle = circle->next)
	{
		mv = sqrt (circle->speed_x * circle->speed_x + circle->speed_y * circle->speed_y);
		if (mv == 0.0)
			continue;
		if ((boundary_hit = boundary_colision (circle, boundaries)) != NULL)
			change_circle_speed (circle, boundary_hit);
	}
}



void
update_temperatures (CIRCLE_LIST *circle_list)
{
	CIRCLE_LIST *circle;
	double mv;

	for (circle = circle_list; circle != NULL; circle = circle->next)
	{
		circle->speed_x *= TEMP_FACTOR;
		circle->speed_y *= TEMP_FACTOR;
		mv = sqrt (circle->speed_x * circle->speed_x + circle->speed_y * circle->speed_y);
		if (mv < MIN_SPEED / 3.0)
		{
			circle->speed_x = 0.0;
 			circle->speed_y = 0.0;
		}
	}
}



double
cylinder_volume (CYLINDER *cylinder)
{
//	printf ("pi = %f, r = %f, size = %f, volume = %f\n", pi, cylinder->diameter / 2.0, cylinder->size,
//	         pi * (cylinder->diameter / 2.0) * (cylinder->diameter / 2.0) * cylinder->size);
	return (pi * POW2(cylinder->diameter / 2.0) * cylinder->size);
}



double
compute_volume (CIRCLE_LIST *circle_list)
{
	CIRCLE_LIST *circle;
	double volume = 0.0;

	for (circle = circle_list; circle != NULL; circle = circle->next)
		volume += cylinder_volume (circle->associated_cylinder);

	return volume;
}



double
volume_solido (CYLINDER_LIST *source, int display)
{
	CYLINDER_LIST *cylinder_list = NULL;
	CIRCLE_LIST *circle_list;
	BOUNDARIES *boundaries;
	int i;
	double volume;
	cylinder_list = (CYLINDER_LIST *) alloc_mem (sizeof (CYLINDER_LIST));
	init_cylinder_list (cylinder_list);
	cylinder_list_copy (cylinder_list, source);
	circle_list = generate_initial_circle_list (cylinder_list);
	boundaries = generate_boundaries (circle_list);
	update_display (circle_list, boundaries);
	i = 1;
	while (i)
	{
		i--;
		if (i == 0)
		{
			i = 500 * add_new_circle (circle_list, boundaries, cylinder_list);
//			fprintf(stderr, "Tries: %d\n", global_tries);
		}
		update_positions (circle_list);
		update_velocities (circle_list, boundaries);
		update_temperatures (circle_list);
		if ((i%250 == 0) && display)
			update_display (circle_list, boundaries);
	}

	update_display (circle_list, boundaries);

	volume = compute_volume (circle_list);

	free_boundaries (boundaries);
	free_circle_list (circle_list);
	cylinder_list_free (cylinder_list);
	return (volume);
}



double circle_packing_execute()
{
	random_generator = gsl_rng_alloc(gsl_rng_rand);
	double volume = volume_solido(global_cylinder_list, 1)  / (1000.0*1000.0);
	global_num_new_cylinders = 0;
	global_tries = 0;
	gsl_rng_free(random_generator);
	return volume;
}




void
circles_f_keyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'q':
			cylinder_delta_x += 1.0;
			printf ("cylinder_delta_x = %f\n", cylinder_delta_x);
			circle_packing_execute ();
		break;
		case 'Q':
			cylinder_delta_x -= 1.0;
			printf ("cylinder_delta_x = %f\n", cylinder_delta_x);
			circle_packing_execute ();
		break;
		case 'w':
			cylinder_delta_y += 1.0;
			printf ("cylinder_delta_y = %f\n", cylinder_delta_y);
			circle_packing_execute ();
		break;
		case 'W':
			cylinder_delta_y -= 1.0;
			printf ("cylinder_delta_y = %f\n", cylinder_delta_y);
			circle_packing_execute ();
		break;
		case 'a':
			theta += pi/100.0;
			printf ("theta = %f\n", theta);
			circle_packing_execute ();
		break;
		case 'A':
			theta -= pi/100.0;
			printf ("theta = %f\n", theta);
			circle_packing_execute ();
		break;
		case 'v':
			circle_packing_execute();
		break;
	}
}



/*
***********************************************************
* Function: init_circle_packing
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void
init_circle_packing (INPUT_DESC *input)
{
	gsl_rng_env_setup();
	make_input_image_circle_packing (input, IN_WIDTH, IN_HEIGHT);
	input->up2date = 0;
	int x, y;



	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (circle_packing_input_display);
	glutKeyboardFunc (circles_f_keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
}


void circle_packing_set_cylinder_list(CYLINDER_LIST *list)
{
	global_cylinder_list = list;
}
