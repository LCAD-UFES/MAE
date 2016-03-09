#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include "drawShape.h"
#include "pointList.h"
#include "smv_user_functions.h"

#define CALIBRATE_IMAGE_WIDTH	2048
#define	VERGENCE_DISP_ANGLE1	0.2
#define	VERGENCE_DISP_ANGLE2	1.0
#define	VERGENCE_DISP_ANGLE3	5.0

#define	MAX_VISUAL_ANGLE	60.0

#define	PERCENT_DIFFERENCE0	1
#define	PERCENT_DIFFERENCE1	10
#define	PERCENT_DIFFERENCE2	20
#define	PERCENT_DIFFERENCE3	40

#define	DISP_ANGLE(x) ((int) ((x * (double) i_nonDominant_eye->ww) / MAX_VISUAL_ANGLE))

#define	MINUS_FLAG 1

#define LARGE 	1
#define THIN	0

#define SHAPE_WINDOW_WIDTH 			320
#define computeShapeWindowHeight(iw, ih, sww)	(GLint) (((double) ih / (double) iw) * (double) sww + 0.5)

#define IK_WB11A_HORIZONTAL_ANGLE_OF_VIEW	56.0f
#define IK_WB11A_VERTICAL_ANGLE_OF_VIEW		44.0f
#define IK_WB11A_DIAGONAL_ANGLE_OF_VIEW		67.0f

#define IMAGE_PART				12
#define MESH_SOFTWARE_COMMAND			"gmsh -0 -3 %s"

#define TOTAL_WINDOW_POINTS			17
#define WINDOW_SEARCH_SIZE			(1.0 / 6.0)

#define ACCURACY_VERGENCE			0
#define FAST_VERGENCE				1

#define MOVE_POINT				0
#define DELETE_POINT				1
#define INSERT_POINT				-1

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

struct _window_search_point
{ 
	XY_PAIR	point;
	int visitAgain;
	NEURON_OUTPUT stimulusValue; 
}; 
 
typedef struct _window_search_point WINDOW_SEARCH_POINT; 

/* Input Handle*/
int nonDominant_eye, dominant_eye;
 
int correction = 0;
int side_move = 0;

int current_output;
int current_input;
int pointsSelected = 0;
int pointOperation = INSERT_POINT;

GLint shapeWidth, shapeHeight;

GNUPLOT_PARAMETERS_LIST *gnuplot_parameters_list;

KEY currentElement;
int volumeWinID = 0;


void free_gnuplot_parameters_list ()
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
generate_gnuplot_file (char *command_file)
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
}

void
plot_gnuplot_file (char *command_file)
{
	char message[256];

	generate_gnuplot_file (command_file);
	sprintf(message,"gnuplot %s",command_file);
	system(message);
}



int
init_user_functions ()
{
	char strCommand[128];
      
        sprintf (strCommand, "toggle move_active;");
        interpreter (strCommand);

        sprintf (strCommand, "toggle draw_active;");
        interpreter (strCommand);

	return (0);
}



int
output_handler_get_current_output (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	if ((mouse_button == GLUT_LEFT_BUTTON) && (mouse_state == GLUT_DOWN))
		current_output = output->win;
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
	sprintf (filename, "%s.gnuplot.out",output->name);

	output_file = fopen(filename,"w");
	
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	for(i=0;i<w;i++)
		for(j=0;j<h;j++)
			fprintf(output_file, "%d %d %d\n",i, j, output->neuron_layer->neuron_vector[i*h+j].output.ival);
	
	fclose(output_file);
}



void
update_filters_output (int status)
{
	all_filters_update ();
	if (status == MOVE)
		all_outputs_update ();
}



void
load_input_image (INPUT_DESC *input)
{
	glutSetWindow(input->win);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, input->ww, input->wh, GL_RGB, GL_UNSIGNED_BYTE, input->image); 
	glDisable(GL_READ_BUFFER);
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
}



/* ----------------------------------------------------------------------
// Transform - Realiza a transformacao
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
*/
void Transform(GLfloat Width , GLfloat Height )
{
	glViewport(0,0, (GLfloat)Width, (GLfloat)Height);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(IK_WB11A_VERTICAL_ANGLE_OF_VIEW, Width/Height,1.0,200.0f); 
	glTranslatef(0.0, 0.0, 0.0f);     /* Centre and away the viewer */
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}



/* ------------------------------------------------------------------------------
// ResizeWindowShape - Called by glutReshapeFunc when the window will be resized
//
// Input - 	Width - window width
//		Height - window height
//
// Saida: Nenhuma
//-------------------------------------------------------------------------------
*/
void ResizeWindowShape(GLint Width, GLint Height)
{
	if (Height==0)    				/* Prevent A Divide By Zero If The Window Is Too Small*/
		Height=1;
	if (Width==0)
		Width=1;

	Transform(Width,Height);    				/* Do our viewing transform */
	shapeWidth  = glutGet((GLenum)GLUT_WINDOW_WIDTH);	/* Make sure our window size is updated */
	shapeHeight = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
}



/*
// ----------------------------------------------------------------------
// InitGL - Inicializacao de alguns parametros do OpenGL
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
*/
 void InitGL(GLint Width, GLint Height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);			/* This Will Clear The Background Color To Black */
	glPointSize(4.0);                             		/* Add point size, to make it clear */
	glLineWidth(2.0);                             		/* Add line width */
	glClearDepth(1.0);					/* Enables Clearing Of The Depth Buffer */
	glDepthFunc(GL_LESS);					/* The Type Of Depth Test To Do */
	glEnable(GL_DEPTH_TEST);				/* Enables Depth Testing */
	glShadeModel(GL_SMOOTH);				/* Enables Smooth Color Shading */
	Transform(Width,Height);				/* Do our viewing transform */
}



/* --------------------------------------------------------------------------
// setShapeWindowFeatures - Initializes the Shape Window Features
//
// Entrada: 	image_width	- width of the Shape Window
//		image_height	- height of the Shape Window
//
// Saida: Nenhuma.
// --------------------------------------------------------------------------
*/
void setShapeWindowFeatures (INPUT_DESC *input)
{   
	int x,y;
	
	/* Inicializa o modo de exibicao */
	glutInitDisplayMode(GLUT_RGBA |      /* RGB and Alpha */
        	      	    GLUT_DOUBLE|     /* Double buffer */
        	      	    GLUT_DEPTH);     /* Z buffer (depth) */

	

	shapeWidth = SHAPE_WINDOW_WIDTH;
	shapeHeight = computeShapeWindowHeight(input->ww, input->wh, SHAPE_WINDOW_WIDTH);
	
	/* Set Window Size */
	glutInitWindowSize (shapeWidth, shapeHeight);

	/* Set Window Position */
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);

	/* Open a window with a title. */ 
	volumeWinID = glutCreateWindow("Shape Window");
}



/* --------------------------------------------------------------------------
// MouseShape - Trata os eventos de click de mouse
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// --------------------------------------------------------------------------
*/
void MouseShape (int b , int s, int xx, int yy)
{
}

/* --------------------------------------------------------------------------
// draw - Desenha o shape para calcular a área.
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// --------------------------------------------------------------------------
*/
void draw ()
{
	drawShape (shape_map, volumeWinID);
}


/* --------------------------------------------------------------------------
// InitVolumeWindow - Inicializa os recursos da Janela para cálculo do volume
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// --------------------------------------------------------------------------
*/
void InitVolumeWindow (INPUT_DESC *input)
{
	INPUT_DESC *i_dominant_eye = NULL;
	int x,y;
	float g_fCenterX, g_fCenterY, g_fCenterZ;


	if (volumeWinID == 0)
	{
		setShapeWindowFeatures (input);
		InitGL (shapeWidth, shapeHeight);

		g_fCenterX = 0.0;
		g_fCenterY = 0.0;
		g_fCenterZ = -50.0;

		glutDisplayFunc (draw);
		glutMouseFunc (MouseShape);
//		glutPassiveMotionFunc (input_passive_motion);
//		glutMotionFunc (MouseMotionShape);
//		glutReshapeFunc (ResizeWindowShape);
	}
}


void 
new_input_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	INPUT_DESC *input;
	int r, g, b;
	
	input = get_input_by_win (glutGetWindow ());
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	sprintf (mouse_pos, "mouse: (%d, %d)", (int) (wx), (int) (wy));
	set_mouse_bar (mouse_pos);
	
	if ((wx >= 0) && (wx < input->tfw) && (wy >= 0) && (wy < input->tfh))
	{
		r = (int) input->image[3 * ((int) (wy) * input->tfw + (int) (wx)) + 0];
		g = (int) input->image[3 * ((int) (wy) * input->tfw + (int) (wx)) + 1];
		b = (int) input->image[3 * ((int) (wy) * input->tfw + (int) (wx)) + 2];
	}
	else
		r = g = b = 0;
	
	switch (input->neuron_layer->output_type)
	{
		case COLOR:
			sprintf (color_val, "r = %03d, g = %03d, b = %03d (color)", r, g, b);
			break;
		case GREYSCALE:
			sprintf (color_val, "intensity = %d (greyscale)", (r + g + b) / 3);
			break;
		case BLACK_WHITE:
			sprintf (color_val, "value = %d (b&w)", (r + g + b) / ( 3 * 255));
			break;	
		case GREYSCALE_FLOAT:
			sprintf (color_val, "intensity = %f (greyscale_float)", (float) (r + g + b) / 3.0);
			break;	
	}
	set_color_bar (color_val);
	input->wxme = wx;
	input->wyme = wy;
}


void
new_input_display ()
{
	NO_POINT_LIST  *current;
	XY_PAIR currentPoint;
	INPUT_DESC *input;
	GLdouble w, h, d, aux;
	char info[256];
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
		
	aux = (GLdouble) input->vph / 64.0;
	if (aux > ((GLdouble) input->vpw / 64.0))
		aux = (GLdouble) input->vpw / 64.0;
	if (input->vpxo > 0)
		glBegin(GL_LINES); glVertex2f (-aux, 0.0); glVertex2f (0.0, 0.0); glEnd (); 
	if (input->vpxo + input->vpw < input->ww)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw+aux, 0.0); glEnd (); 
	if (input->vpyo + input->vph < input->wh)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw, -aux); glEnd (); 
	if (input->vpyo > 0)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph+aux); glEnd (); 
	
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
	
	glRasterPos2f (0.0, h + h / 20.0);
	
	for (i = 0; i < strlen (info); i++)
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, info[i]);
 		
	if (processStatus > POINTS_NOT_SELECTED)
	{
		glColor3f (1.0, 1.0, 1.0);
		glBegin(GL_LINE_LOOP);
		for (current = point_list.head; current != NULL; current = current->next)
		{
			if (input->win == dominant_eye)
			{
				currentPoint.x = current->element.dominantEyePoint.x;
				currentPoint.y = current->element.dominantEyePoint.y;
			}
			else
			{
				currentPoint.x = current->element.nonDominantEyePoint.x;
				currentPoint.y = current->element.nonDominantEyePoint.y;
			}
			if (currentPoint.x != INT_MIN)
			{
				glVertex2i(currentPoint.x, currentPoint.y);
			}
		}
		glEnd();
	}

	glutSwapBuffers ();
	input->waiting_redisplay = 0;

}



void
removePoint (XY_PAIR dominantEyePoint, XY_PAIR nonDominantEyePoint, WORLD_POINT world_point, int handle)
{
	KEY elementToChange, element;

	if (handle == dominant_eye)
	{
		element.dominantEyePoint.x = dominantEyePoint.x;
		element.dominantEyePoint.y = dominantEyePoint.y;
		element.nonDominantEyePoint.x = INT_MIN;
		element.nonDominantEyePoint.y = INT_MIN;
		element.world_point.x = DBL_MAX;
		element.world_point.y = DBL_MAX;
		element.world_point.z = DBL_MAX;
	}
	else
	{
		element.nonDominantEyePoint.x = nonDominantEyePoint.x;
		element.nonDominantEyePoint.y = nonDominantEyePoint.y;
		element.dominantEyePoint.x = INT_MIN;
		element.dominantEyePoint.y = INT_MIN;
		element.world_point.x = DBL_MAX;
		element.world_point.y = DBL_MAX;
		element.world_point.z = DBL_MAX;
	}
	removePointFromList (&point_list, element);
	if (point_list.totalNo == 0)
		processStatus = POINTS_NOT_SELECTED;
	
}



void
memorizePoint (XY_PAIR dominantEyePoint, XY_PAIR nonDominantEyePoint, WORLD_POINT world_point, int handle, int pointOperation)
{
	KEY elementToChange, element;

	element.dominantEyePoint.x = dominantEyePoint.x;
	element.dominantEyePoint.y = dominantEyePoint.y;
	element.nonDominantEyePoint.x = nonDominantEyePoint.x;
	element.nonDominantEyePoint.y = nonDominantEyePoint.y;
	element.world_point.x = world_point.x;
	element.world_point.y = world_point.y;
	element.world_point.z = world_point.z;
	switch (processStatus) 
	{
		case POINTS_NOT_SELECTED:
			if (handle == dominant_eye && pointOperation == INSERT_POINT)
			{
				element.nonDominantEyePoint.x = INT_MIN;
				element.nonDominantEyePoint.y = INT_MIN;
				element.world_point.x = DBL_MAX;
				element.world_point.y = DBL_MAX;
				element.world_point.z = DBL_MAX;

				if (!addPointToPointList (&point_list, element))
					Erro ("There's no memory to add point to list.","","");

				processStatus = POINTS_IN_SELECTION;
			}
			break;
		case POINTS_IN_SELECTION:
			if (handle == dominant_eye)
			{
				element.nonDominantEyePoint.x = INT_MIN;
				element.nonDominantEyePoint.y = INT_MIN;
				element.world_point.x = DBL_MAX;
				element.world_point.y = DBL_MAX;
				element.world_point.z = DBL_MAX;

				if (pointOperation == INSERT_POINT)
				{
					if (point_list.totalNo < 2)
					{
						if (!addPointToPointList (&point_list, element))
							Erro ("There's no memory to add point to list.", "", "");
					}
					else
						if (!insertOnList(&point_list, element))
							Erro ("There's no memory to insert this point on list.", "", "");
				}
				else
				{
					elementToChange.dominantEyePoint.x = dominantEyePoint.x;
					elementToChange.dominantEyePoint.y = dominantEyePoint.y;
					elementToChange.nonDominantEyePoint.x = INT_MIN;
					elementToChange.nonDominantEyePoint.y = INT_MIN;
					elementToChange.world_point.x = DBL_MAX;
					elementToChange.world_point.y = DBL_MAX;
					elementToChange.world_point.z = DBL_MAX;
					changePointOnList (&point_list, elementToChange, element);
				}
			}
			break;
		case VERGENCE_STARTED:
		case VERGENCE_COMPUTED:
		case AREA_CALCULATED:
		case VOLUME_CALCULATED:
			if (pointOperation == MOVE_POINT)
			{
				elementToChange.nonDominantEyePoint.x = INT_MIN;
				elementToChange.nonDominantEyePoint.y = INT_MIN;
				elementToChange.dominantEyePoint.x = INT_MIN;
				elementToChange.dominantEyePoint.y = INT_MIN;
				elementToChange.world_point.x = DBL_MAX;
				elementToChange.world_point.y = DBL_MAX;
				elementToChange.world_point.z = DBL_MAX;
				if (handle == nonDominant_eye)
				{
					elementToChange.nonDominantEyePoint.x = nonDominantEyePoint.x;
					elementToChange.nonDominantEyePoint.y = nonDominantEyePoint.y;
				}
				else
				{
					elementToChange.dominantEyePoint.x = dominantEyePoint.x;
					elementToChange.dominantEyePoint.y = dominantEyePoint.y;
				}
				changePointOnList (&point_list, elementToChange, element);
			}
			else
			{
				if (handle == dominant_eye)
				{
					element.nonDominantEyePoint.x = INT_MIN;
					element.nonDominantEyePoint.y = INT_MIN;
					element.world_point.x = DBL_MAX;
					element.world_point.y = DBL_MAX;
					element.world_point.z = DBL_MAX;
					if (!insertOnList(&point_list, element))
						Erro ("There's no memory to insert this point on list.", "", "");
				}
			}
	}
}



void
mainMenuPoint (int option)
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	XY_PAIR dominantEyePoint, nonDominantEyePoint;
	WORLD_POINT world_point;
	
					
	if (i_nonDominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
	}

	pointOperation = option;
	
	switch (pointOperation)
	{
		case INSERT_POINT:
		case MOVE_POINT:
			dominantEyePoint.x = i_dominant_eye->wxd;
			dominantEyePoint.y = i_dominant_eye->wyd;
			nonDominantEyePoint.x = i_nonDominant_eye->wxd;
			nonDominantEyePoint.y = i_nonDominant_eye->wyd;
			world_point.x = DBL_MAX;
			world_point.y = DBL_MAX;
			world_point.z = DBL_MAX;
			memorizePoint (dominantEyePoint, nonDominantEyePoint, world_point, glutGetWindow (), pointOperation);
			break;
		case DELETE_POINT:
			dominantEyePoint.x = i_dominant_eye->wxme;
			dominantEyePoint.y = i_dominant_eye->wyme;
			nonDominantEyePoint.x = i_nonDominant_eye->wxme;
			nonDominantEyePoint.y = i_nonDominant_eye->wyme;
			world_point.x = DBL_MAX;
			world_point.y = DBL_MAX;
			world_point.z = DBL_MAX;
			removePoint (dominantEyePoint, nonDominantEyePoint, world_point, glutGetWindow ());
			break;
	}
	glutPostRedisplay();
}



void
createMenuPoint ()
{
	int menu;

	menu = glutCreateMenu(mainMenuPoint);
	if (processStatus == POINTS_NOT_SELECTED)
		glutAddMenuEntry("Insert",INSERT_POINT);
	else
	{
		glutAddMenuEntry("Insert",INSERT_POINT);
		glutAddMenuEntry("Move",MOVE_POINT);
		glutAddMenuEntry("Delete",DELETE_POINT);
	}
	glutAttachMenu(GLUT_RIGHT_BUTTON);

}



void
new_make_input_image (INPUT_DESC *input)
{
	int i;
	int w, h;
	char file_name[256];
	char header[256];
	char message[256];
	char character = 0;
	FILE *path_file = (FILE *) NULL;
	FILE *image_file = (FILE *) NULL;

	input->image_type = image_type;
	input->file = NULL;
	
	// Compute the input image dimentions
	input->tfw = nearest_power_of_2 (input->neuron_layer->dimentions.x);
	input->tfh = nearest_power_of_2 (input->neuron_layer->dimentions.y);

	// Save the image dimentions
	input->ww = input->neuron_layer->dimentions.x;
	input->wh = input->neuron_layer->dimentions.y;
	
	switch(TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vpw = input->ww;
			input->vph = input->wh;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;
	
	if(input->image == (GLubyte *) NULL)
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
	
	
//	update_input_image (input);

}



void
input_generator (INPUT_DESC *input, int status)
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	static createdInputs = 1;
					
	if (i_nonDominant_eye == NULL)
	{
		dominant_eye = (get_input_by_name ("image_right"))->win;
		nonDominant_eye = (get_input_by_name ("image_left"))->win;
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
		stackWidth = INITIAL_STACKWIDTH;
		ShapeMapInitialize (&shape_map, 100);
		breakLineFactor = INITIAL_BREAKLINEFACTOR;
		inputType = input->input_generator_params->next->param.ival;
		image_type = input->input_generator_params->next->next->param.ival;
	}
	
	if (((input->win == nonDominant_eye) || (input->win == dominant_eye)) && (input->win != 0) && (status == MOVE))
	{
		update_input_neurons (input);
		
		if (inputType != PASS_BY_PARAMETER)
		{
			check_input_bounds (input, input->wxd, input->wyd);
			glutSetWindow(input->win);
			new_input_display ();
		}
		
		all_filters_update ();
		all_outputs_update ();
  	}
	
	if (input->win == 0)
	{
		float f = 1.0;
		int x, y;

		switch (inputType)
		{
			case READ_FROM_IMAGE_FILE	: make_input_image (input); break;
			case PASS_BY_PARAMETER       	: new_make_input_image (input); break;
			case CAPTURED_FROM_CAMERA    	: break;
			case CAPTURED_FROM_IP_ADDRESS	: break;
		}
		init (input);
		update_input_neurons (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		if (dominant_eye == 0) 
			dominant_eye = input->win;
		else
			nonDominant_eye = input->win;
		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (new_input_display); 
		glutKeyboardFunc (keyboard);
		glutPassiveMotionFunc (new_input_passive_motion);
		glutMouseFunc (input_mouse);
		createdInputs++;
 		createMenuPoint ();

		//Initializes the Volume Window
		if (createdInputs == NUM_INPUTS)
		{
			nodeListInitialize (&node_list);
			elementListInitialize (&element_list);
			InitVolumeWindow (input);
			processStatus = POINTS_NOT_SELECTED;
		}
		
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
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	XY_PAIR dominantEyePoint, nonDominantEyePoint;
	WORLD_POINT world_point;
	
					
	if (i_nonDominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == dominant_eye))
	{
		i_nonDominant_eye->wxd += i_dominant_eye->wxd - i_dominant_eye->wxd_old;
		i_nonDominant_eye->wyd = i_dominant_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_dominant_eye->name, i_dominant_eye->wxd, i_dominant_eye->wyd);
		interpreter (command);

		sprintf (command, "move %s to %d, %d;", i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
		interpreter (command);
		
	}
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == nonDominant_eye))
	{
		i_nonDominant_eye->wyd = i_dominant_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
		interpreter (command);

	}
	
 	createMenuPoint();
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
	static INPUT_DESC *i_Dominant_eye = NULL;
	int x_current;
	
	if (i_Dominant_eye == NULL)
		i_Dominant_eye = get_input_by_win (dominant_eye);
	
	i_Dominant_eye->wxd_old = i_Dominant_eye->wxd;
	i_Dominant_eye->wxd += side_move;
	sprintf (command, "move %s to %d, %d;", i_Dominant_eye->name, i_Dominant_eye->wxd, i_Dominant_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}


void
move_y ()
{
	char command[256];
	static INPUT_DESC *i_Dominant_eye = NULL;
	int x_current;
	
	if (i_Dominant_eye == NULL)
		i_Dominant_eye = get_input_by_win (dominant_eye);
	
	i_Dominant_eye->wyd_old = i_Dominant_eye->wyd;
	i_Dominant_eye->wyd += side_move;
	sprintf (command, "move %s to %d, %d;", i_Dominant_eye->name, i_Dominant_eye->wxd, i_Dominant_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
}



void
setWindowPoint (int index, XY_PAIR point, WINDOW_SEARCH_POINT *windowPoints, NEURON_OUTPUT stimulusValue, int output_type, int visitAgain)
{
	windowPoints[index].point.x = point.x;
	windowPoints[index].point.y = point.y;
	windowPoints[index].visitAgain = visitAgain;
	if (output_type == GREYSCALE_FLOAT)
		windowPoints[index].stimulusValue.fval = stimulusValue.fval;
	else
		windowPoints[index].stimulusValue.fval = stimulusValue.ival;
}



void
computeStimulusVaue (XY_PAIR currentPoint, NEURON_OUTPUT *currentStimulus, int *output_type)
{

	static INPUT_DESC *i_nonDominant_eye = NULL;
	static NEURON_LAYER *minus_filter = NULL;
	static int num_neurons;

	if (i_nonDominant_eye == NULL)
	{
		minus_filter = get_neuron_layer_by_name ("right_minus_left");
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		num_neurons = minus_filter->dimentions.x * minus_filter->dimentions.y;
	}

	i_nonDominant_eye->wxd = currentPoint.x;
	i_nonDominant_eye->wyd = currentPoint.y;
	move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
	all_filters_update ();
	*output_type = minus_filter->output_type;

	if (minus_filter->output_type == GREYSCALE_FLOAT)
		(*currentStimulus).fval = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).fval;
	else
		(*currentStimulus).ival = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).ival;
	
}



void
windowSearchMinusStimulus (XY_PAIR minPoint, XY_PAIR maxPoint, WINDOW_SEARCH_POINT *windowPoints, XY_PAIR *vergencePoint, int windowSize)
{
	float step;
	int i;
	XY_PAIR centerPoint;
	XY_PAIR currentPoint;
	NEURON_OUTPUT minStimulus, currentStimulus;
	int indexMinusPoint;
	int output_type;
/* 	FILE *arq_points_minus, *arq_points_ini, *arq_points_min, *arq_points_win, *arq_old_minus;
 */      

	step = ((maxPoint.x - minPoint.x + 1) * 1.0) / ((windowSize - 1) * 1.0);
	if (step < 1.0)
	{
		step = 1.0;
		maxPoint.x = minPoint.x + windowSize;
	}	
	
/*  	system ("rm *.out");
	arq_points_minus = fopen ("minus.out", "w");
	arq_old_minus = fopen ("minus.dat", "r");
	arq_points_ini = fopen ("ini.out", "w");
	arq_points_min = fopen ("verg.out", "w");
	arq_points_win = fopen ("win.out", "w");
	
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "ini.out" , 1, 2, 0, "Start Point", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "verg.out" , 1, 2, 0, "Vergence", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "minus.out" , 1, 2, 0, "Response on windowSearchMinusStimulus", POINTS));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "win.out" , 1, 2, 0, "Response on Window Points", POINTS));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "minus.dat" , 1, 2, 0, "Response on minusVergence", LINES));
 */
	for (i = 0; i < windowSize; i++)
	{
		currentPoint.x = minPoint.x + ((int) ((i * step) + 0.5));
		currentPoint.x = currentPoint.x > maxPoint.x ? maxPoint.x : currentPoint.x;
		currentPoint.y = minPoint.y;
		if (windowPoints[i].visitAgain == 1)
		{
			computeStimulusVaue (currentPoint, &currentStimulus, &output_type);
			setWindowPoint (i, currentPoint, windowPoints, currentStimulus, output_type, 1);
		}
		
		if (output_type == GREYSCALE_FLOAT)
		{
/*  			fprintf (arq_points_minus, "%d, %f\n", currentPoint.x, currentStimulus.fval);
 */ 			if (i == 0 || windowPoints[i].stimulusValue.fval < minStimulus.fval)
			{
				indexMinusPoint = i;
				minStimulus.fval = windowPoints[i].stimulusValue.fval;
			}
		}
		else
			if (i == 0 || windowPoints[i].stimulusValue.ival < minStimulus.ival)
			{
				indexMinusPoint = i;
				minStimulus.ival = windowPoints[i].stimulusValue.ival;
			}
	}
	
	vergencePoint->x = windowPoints[indexMinusPoint].point.x;
	vergencePoint->y = windowPoints[indexMinusPoint].point.y;
	
/* 	for (i = 0; i < windowSize; i++)
	{
		if (output_type == GREYSCALE_FLOAT)
			fprintf (arq_points_win, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.fval);
		else
			fprintf (arq_points_win, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.ival);

		if (i == 0)
			if (output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_ini, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.fval * 1.2);
			else
				fprintf (arq_points_ini, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.ival * 1.2);
		else
			fprintf (arq_points_ini, "%d, %d\n", windowPoints[i].point.x, 0);
		if (i == indexMinusPoint)
			if (output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_min, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.fval * 1.2);
			else
				fprintf (arq_points_min, "%d, %f\n", windowPoints[i].point.x, windowPoints[i].stimulusValue.ival * 1.2);
		else
			fprintf (arq_points_min, "%d, %d\n", windowPoints[i].point.x, 0);
	}
	
	fclose (arq_old_minus);
	fclose (arq_points_minus);
	fclose (arq_points_min);
	fclose (arq_points_ini);
	fclose (arq_points_win);
	plot_gnuplot_file("minus.cmd");
 */
	if (step == 1.0)
		return;
	
	switch (indexMinusPoint) 
	{
		case 0:
		case 1:
			setWindowPoint (0, windowPoints[0].point, windowPoints, windowPoints[0].stimulusValue, output_type, 0);
			setWindowPoint (windowSize - 1, windowPoints[2].point, windowPoints, windowPoints[2].stimulusValue, output_type, 0);
			setWindowPoint ((windowSize - 1)/2, windowPoints[indexMinusPoint].point, windowPoints, windowPoints[indexMinusPoint].stimulusValue, output_type, 0);
 			break;
		case TOTAL_WINDOW_POINTS - 1:
		case TOTAL_WINDOW_POINTS - 2:
			setWindowPoint (0, windowPoints[windowSize - 3].point, windowPoints, windowPoints[windowSize - 3].stimulusValue, output_type, 0);
			setWindowPoint (windowSize - 1, windowPoints[windowSize - 1].point, windowPoints, windowPoints[windowSize - 1].stimulusValue, output_type, 0);
			setWindowPoint ((windowSize - 1)/2, windowPoints[indexMinusPoint].point, windowPoints, windowPoints[indexMinusPoint].stimulusValue, output_type, 0);
 			break;
		default:		
			setWindowPoint (0, windowPoints[indexMinusPoint - 1].point, windowPoints, windowPoints[indexMinusPoint - 1].stimulusValue, output_type, 0);
			setWindowPoint (windowSize - 1, windowPoints[indexMinusPoint + 1].point, windowPoints, windowPoints[indexMinusPoint + 1].stimulusValue, output_type, 0);
			setWindowPoint ((windowSize - 1)/2, windowPoints[indexMinusPoint].point, windowPoints, windowPoints[indexMinusPoint].stimulusValue, output_type, 0);
	}

	windowSearchMinusStimulus (windowPoints[0].point, windowPoints[windowSize - 1].point, windowPoints, vergencePoint, windowSize);
}


void
windowSearchVergence ()
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;	
	XY_PAIR minPoint, maxPoint, vergencePoint;
	WINDOW_SEARCH_POINT *vergenceFunctionPoints;
	int i;

	if (i_nonDominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);

	}


	vergenceFunctionPoints = (WINDOW_SEARCH_POINT *) alloc_mem ((size_t) sizeof (WINDOW_SEARCH_POINT) * TOTAL_WINDOW_POINTS);

	for (i = 0;i < TOTAL_WINDOW_POINTS; i++)
		vergenceFunctionPoints[i].visitAgain = 1;
		
	minPoint.x = i_dominant_eye->wxd;
	minPoint.y = i_dominant_eye->wyd;
	maxPoint.x = i_dominant_eye->wxd + i_dominant_eye->ww / IMAGE_PART;
	maxPoint.y = i_dominant_eye->wyd;
	maxPoint.x = (maxPoint.x > i_nonDominant_eye->neuron_layer->dimentions.x) ? i_nonDominant_eye->neuron_layer->dimentions.x : maxPoint.x;

	windowSearchMinusStimulus (minPoint, maxPoint, vergenceFunctionPoints, &vergencePoint, TOTAL_WINDOW_POINTS);

	i_nonDominant_eye->wxd_old = i_nonDominant_eye->wxd;
	i_nonDominant_eye->wyd_old = i_nonDominant_eye->wyd;
	i_nonDominant_eye->wxd = vergencePoint.x;
	i_nonDominant_eye->wyd = vergencePoint.y;
	move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
	free(vergenceFunctionPoints);
	
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
set_vergence_by_minus_filter ()
{
	char command[256];
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;	
	static NEURON_LAYER *minus_filter = NULL;
	static int num_neurons;
	int x, x_inic, x_min, x_max, x_max_minus, x_min_minus;
	NEURON_OUTPUT minus, max_minus, min_minus;
	FILE *arq_points_minus, *arq_points_ini, *arq_points_min;
	
	if (minus_filter == NULL)
	{
		minus_filter = get_neuron_layer_by_name ("right_minus_left");
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
		num_neurons = minus_filter->dimentions.x * minus_filter->dimentions.y;
	}
	
/* 	system ("rm *.out");
	arq_points_minus = fopen ("minus.out", "w");
	arq_points_ini = fopen ("ini.out", "w");
	arq_points_min = fopen ("verg.out", "w");
	processStatus = VERGENCE_STARTED;
	
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "ini.out" , 1, 2, 0, "Start Point", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "verg.out" , 1, 2, 0, "Vergence", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "minus.out" , 1, 2, 0, "Response on minus", LINES));
 */
	x_inic = i_nonDominant_eye->wxd;
	x_min = i_dominant_eye->wxd;
	x_max = i_dominant_eye->wxd + i_dominant_eye->ww / IMAGE_PART;
	x_max = (x_max > i_nonDominant_eye->neuron_layer->dimentions.x) ? i_nonDominant_eye->neuron_layer->dimentions.x : x_max;
	
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
		i_nonDominant_eye->wxd = x;
		move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
		all_filters_update ();
		if (minus_filter->output_type == GREYSCALE_FLOAT)
		{
			minus.fval = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).fval;
/* 			fprintf (arq_points_minus, "%d, %f\n", x, minus.fval);
 */			if (max_minus.fval < minus.fval)
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
/* 			fprintf (arq_points_minus, "%d, %d\n", x, minus.ival);
 */			if (max_minus.ival < minus.ival)
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

/* 	for (x = x_min; x < x_max; x++)
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
	//plot_gnuplot_file("minus.cmd");

 */	i_nonDominant_eye->wxd_old = i_nonDominant_eye->wxd;
	i_nonDominant_eye->wxd = x_min_minus;
	move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);

	glutIdleFunc ((void (* ) (void)) check_forms);
}



/*
------------------------------------------------------------------------------------------
calculateWorldPoint - calculate the World Point.
Input	- No inputs.
Output	- WORLD_POINT relative to image right point and vergence point.
------------------------------------------------------------------------------------------
*/

WORLD_POINT
calculateWorldPoint ()
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_Dominant_eye = NULL;
	IMAGE_COORDINATE left_point;
	IMAGE_COORDINATE right_point;
	float image_focal_distance;
	int image_width;
	XY_PAIR image_dimentions;
	
	if (i_Dominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_Dominant_eye = get_input_by_win (dominant_eye);
	}
	
	image_dimentions.x = i_Dominant_eye->neuron_layer->dimentions.x;
	image_dimentions.y = i_Dominant_eye->neuron_layer->dimentions.y;
	image_focal_distance = get_image_focal_distance (FOCAL_DISTANCE, CALIBRATE_IMAGE_WIDTH, image_dimentions.x);

	left_point.x = (double) i_nonDominant_eye->wxd;
	left_point.y = (double) i_nonDominant_eye->wyd;
	right_point.x = (double) i_Dominant_eye->wxd;
	right_point.y = (double) i_Dominant_eye->wyd;

	return calculate_world_point (left_point, right_point, image_dimentions, image_focal_distance, CAMERA_DISTANCE);

}

/*
------------------------------------------------------------------------------------------
plotWorldPoint - plot a World Point in the Window Map.
Input	- No inputs.
Output	- No outputs.
------------------------------------------------------------------------------------------
*/

void
plotWorldPoint ()
{
	WORLD_POINT world_point;
	
	pointsSelected++;
	windowSearchVergence ();
	world_point = calculateWorldPoint ();
	memorizeWorldPoint (world_point, &shape_map);
	drawShape (shape_map, volumeWinID);
}



/*
------------------------------------------------------------------------------------------
generateMesh - generate the Mesh that will calculate area and the Volume
Input	- No inputs.
Output	- No outputs.
------------------------------------------------------------------------------------------
*/
void
generateMesh ()
{
	readMSHFormatFile (shape_map);
	shapeArea = computeArea ();
	printf ("shapeArea -> %.5f\n", shapeArea);
	stockPileVolume = calculateVolume(shapeArea, stackWidth);
	printf ("stockPileVolume -> %.5f stackWidth -> %.5f\n", stockPileVolume, stackWidth);
	drawShape (shape_map, volumeWinID);
}



/*
------------------------------------------------------------------------------------------
generateShape - generate the Shape that will calculate area and the Volume
Input	- No inputs.
Output	- No outputs.
------------------------------------------------------------------------------------------
*/

void
generateShape ()
{
	char gmsh_command[255];

	generateGeoFile (&shape_map, breakLineFactor);
	sprintf(gmsh_command, MESH_SOFTWARE_COMMAND, GMSH_GEO_FILENAME);
	system(gmsh_command);
	generateMesh();
	
}



void
move_right_eye ()
{
	char command[256];
	static INPUT_DESC *i_dominant_eye = NULL;
	
	if (i_dominant_eye == NULL)
		i_dominant_eye = get_input_by_win (dominant_eye);
	
	sprintf (command, "move %s to %d, %d;", i_dominant_eye->name, 
			  i_dominant_eye->wxd, i_dominant_eye->wyd);
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

	map_v1_to_image (&(input->green_cross_x), &(input->green_cross_y), input->neuron_layer->dimentions.x, input->neuron_layer->dimentions.y, 
		 	(int) wx, (int) wy, output->neuron_layer->dimentions.x, output->neuron_layer->dimentions.y, input->wxd, input->wyd);

	glutPostWindowRedisplay (input->win);		 
}


void
output_handler_logpolar_righteye (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	char command[256];
	static INPUT_DESC *i_dominant_eye = NULL;
	
	if (i_dominant_eye == NULL)
		i_dominant_eye = get_input_by_win (dominant_eye);
	
	if ((mouse_button == GLUT_LEFT_BUTTON) &&
	    (mouse_state == GLUT_DOWN))
	{
		map_v1_to_image (&(i_dominant_eye->wxd), &(i_dominant_eye->wyd), i_dominant_eye->neuron_layer->dimentions.x, i_dominant_eye->neuron_layer->dimentions.y, 
				 output->wxd, output->wyd, output->neuron_layer->dimentions.x, output->neuron_layer->dimentions.y, i_dominant_eye->wxd, i_dominant_eye->wyd);

		glutIdleFunc ((void (* ) (void))move_right_eye);		 
	}
	output->mouse_button = -1;
}


XY_PAIR 
calculateVergencePoint (int vergence_type, XY_PAIR dominantEyePoint)
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	XY_PAIR vergencePoint;	
	
	if (i_dominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
	}
	
	move_input_window (i_dominant_eye->name, dominantEyePoint.x, dominantEyePoint.y);
	i_nonDominant_eye->wxd += i_dominant_eye->wxd - i_dominant_eye->wxd_old;
	i_nonDominant_eye->wyd = i_dominant_eye->wyd + correction;
	move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
	
	if (vergence_type == FAST_VERGENCE)
		windowSearchVergence ();
	else
		set_vergence_by_minus_filter ();
	
	vergencePoint.x = i_nonDominant_eye->wxd;
	vergencePoint.y = i_nonDominant_eye->wyd;
	
	return vergencePoint;
}


void 
startGenerateVolumeProcess (int vergence_type)
{
	NO_POINT_LIST  *current;
	
	current = point_list.head;
	
	ShapeMapDispose(&shape_map);
	ShapeMapInitialize (&shape_map, point_list.totalNo);
	
	for(;current != NULL;)
	{
		if (current->element.nonDominantEyePoint.x == INT_MIN)
		{
			current->element.nonDominantEyePoint = calculateVergencePoint (vergence_type, current->element.dominantEyePoint);
			current->element.world_point.x = DBL_MAX;
		}
			
		if (current->element.world_point.x == DBL_MAX)
			current->element.world_point = calculateWorldPoint ();
		
		memorizeWorldPoint (current->element.world_point, &shape_map);
		drawShape (shape_map, volumeWinID);
		current = current->next;
	}
	generateShape();
}


void 
cleanAllPoints ()
{
	disposeList(&point_list);
}


void 
f_keyboard (char *key_value)
{
	int i;
	char key;
	INPUT_DESC *input;	
	OUTPUT_DESC *output;	
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	XY_PAIR dominantEyePoint, nonDominantEyePoint;
	WORLD_POINT world_point;
	int x[4] = {487, 489, 625, 625};
	int y[4] = {558, 800, 798, 558};
	int n = 4;
	float pWorldPointsVectorX[4], pWorldPointsVectorY[4], pWorldPointsVectorZ[4];
					
	if (i_nonDominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
	}
	
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
		case 'M':
			glutIdleFunc ((void (* ) (void))set_vergence_by_minus_filter);
			break;
		case 'c':
			cleanAllPoints();
			break;
		case 'f':
			glutIdleFunc ((void (* ) (void))set_initial_position);
			break;
		case 'g':
			startGenerateVolumeProcess(FAST_VERGENCE);
			break;
		case 'G':
			startGenerateVolumeProcess(ACCURACY_VERGENCE);
			break;
		case 'i':
			dominantEyePoint.x = i_dominant_eye->wxd;
			dominantEyePoint.y = i_dominant_eye->wyd;
			nonDominantEyePoint.x = i_nonDominant_eye->wxd;
			nonDominantEyePoint.y = i_nonDominant_eye->wyd;
			world_point.x = DBL_MAX;
			world_point.y = DBL_MAX;
			world_point.z = DBL_MAX;
			memorizePoint (dominantEyePoint, nonDominantEyePoint, world_point, glutGetWindow (), pointOperation);
			break;
		case 'l':
			side_move = -1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
		case 'r':
			side_move = 1;
			glutIdleFunc ((void (* ) (void))move_x);
			break;
		case 'u':
			side_move = 1;
			glutIdleFunc ((void (* ) (void))move_y);
			break;
		case 'd':
			side_move = -1;
			glutIdleFunc ((void (* ) (void))move_y);
			break;
		case 'P':
			if (output != NULL)
				print_output(current_output);
			break;
		case 'p':
			plotWorldPoint ();
			break;
		case 's':
			generateShape();
			break;
		case 'v':
//			generateMesh ();
			printf("Volume -> %.5f\n", maeCalculateVolume(x, y, n));
			break;
		case 'w':
			windowSearchVergence ();
			break;
		case '+':
			breakLineFactor/=2;
			break;
		case '-':
			breakLineFactor*=2;
			break;
		case 'z':
			maeGetWorldPoints (pWorldPointsVectorX, pWorldPointsVectorY, pWorldPointsVectorZ);
			for (i = 0; i < 4; i++)
				printf ("%.5f,\t%.5f,\t%.5f\n", pWorldPointsVectorX[i], pWorldPointsVectorY[i], pWorldPointsVectorZ[i]);
			break;
	}
					
	update_filters_output (MOVE);
}

NEURON_OUTPUT
setDominantEye (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	dominant_eye = (get_input_by_name ("image_right"))->win;
	
	return output;
}

NEURON_OUTPUT
setNonDominantEye (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	nonDominant_eye = (get_input_by_name ("image_left"))->win;
	return output;
}

NEURON_OUTPUT
moveDominantEye (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;

	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	int x, y;
	
	output.ival = 0;
	x = param_list->next->param.ival;
	y = param_list->next->next->param.ival;
	
					
	if (i_dominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
	}
	
	move_input_window (i_dominant_eye->name, x, y);

	i_nonDominant_eye->wxd += i_dominant_eye->wxd - i_dominant_eye->wxd_old;
	i_nonDominant_eye->wyd = i_dominant_eye->wyd + correction;

	move_input_window (i_nonDominant_eye->name, i_nonDominant_eye->wxd, i_nonDominant_eye->wyd);
	return output;
}

NEURON_OUTPUT
compute_point_error (PARAM_LIST *param_list)
{
	static INPUT_DESC *i_nonDominant_eye = NULL;
	static INPUT_DESC *i_dominant_eye = NULL;
	NEURON_LAYER *right_minus_left_nl = NULL;
	int error_x = 0;
	int error_y = 0;
	char point_name[10] = "teste";
	int x_real, y_real, id_point;
	static int w, h;
	NEURON_OUTPUT output;
	FILTER_DESC *logpolar_righteye_filter = NULL;
	static int cone_organization, concetrically_region, color_model, distribution_type;
	static double log_factor;
	
	output.ival = 0;
	id_point = param_list->next->param.ival;
	x_real = param_list->next->next->param.ival;
	y_real = param_list->next->next->next->param.ival;
					
	if (i_dominant_eye == NULL)
	{
		i_nonDominant_eye = get_input_by_win (nonDominant_eye);
		i_dominant_eye = get_input_by_win (dominant_eye);
		right_minus_left_nl = get_neuron_layer_by_name ("right_minus_left");
		w = right_minus_left_nl->dimentions.x;
		h = right_minus_left_nl->dimentions.y;
		logpolar_righteye_filter = get_filter_by_name ("logpolar_righteye_filter");
		cone_organization = logpolar_righteye_filter->filter_params->next->param.ival;
		concetrically_region = logpolar_righteye_filter->filter_params->next->next->param.ival;
		color_model = logpolar_righteye_filter->filter_params->next->next->next->param.ival;
		distribution_type = logpolar_righteye_filter->filter_params->next->next->next->next->param.ival;
		log_factor = logpolar_righteye_filter->filter_params->next->next->next->next->next->param.fval;
	}

	error_x = x_real - i_nonDominant_eye->wxd;
	error_y = y_real - i_nonDominant_eye->wyd;
	
	printf("%d\t%d\t%d\t%d\t%f\t%d\t%d\t%d\t%d\t%d\n", cone_organization, concetrically_region, color_model, distribution_type, log_factor, 
						w, h, id_point, error_x, error_y, KERNEL_SIZE);
	fflush(stdin);
	return output;
}

NEURON_OUTPUT
evaluate_error (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;

	output.ival = 0;
	return output;
}

NEURON_OUTPUT
make_vergence (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	set_vergence_by_minus_filter();
	return output;
}

NEURON_OUTPUT
drawWorldPoint (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	plotWorldPoint ();
	return output;
}

NEURON_OUTPUT
generateShapeMesh (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	generateShape ();
	return output;
}

NEURON_OUTPUT
viewShapeMesh (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	output.ival = 0;
	generateMesh ();
	return output;
}

NEURON_OUTPUT
setBreakLineFactor (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	breakLineFactor = param_list->next->param.fval;
	generateShape ();
	return output;
}

NEURON_OUTPUT
addBreakLineFactor (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	breakLineFactor += param_list->next->param.fval;
	generateShape ();
	return output;
}

NEURON_OUTPUT
subtractBreakLineFactor (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	breakLineFactor -= param_list->next->param.fval;
	generateShape ();
	return output;
}

NEURON_OUTPUT
multiplyBreakLineFactor (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	breakLineFactor *= param_list->next->param.fval;
	generateShape ();
	return output;
}

NEURON_OUTPUT
divideBreakLineFactor (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	
	breakLineFactor /= param_list->next->param.fval;
	generateShape ();
	return output;
}

NEURON_OUTPUT
setStackWidth (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;

	stackWidth = param_list->next->param.fval;
	return output;
}

NEURON_OUTPUT
setShapeMapSize (PARAM_LIST *param_list)
{
	NEURON_OUTPUT output;
	int newSize;
	
	newSize = param_list->next->param.fval;
	generateShape ();
	return output;
}

