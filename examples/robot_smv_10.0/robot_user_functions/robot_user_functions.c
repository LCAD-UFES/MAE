#include "robot_user_functions.h"
#include "viewer.hpp"
#include "cylinder.h"
#include "estimate_volume.h"
#include "subpixel_disparity.h"
#include "gabor_guess_cylinder.h"
#include "stereo_volume.h" 

// Variaveis globais
int g_nCurrDisparity;

double g_dblNearVergenceCutPlane = NEAREST_DISTANCE;
double g_dblFarVergenceCutPlane = FAREST_DISTANCE;


//Varibles to draw things
int g_nCenterX, g_nCenterY;
float g_fltZoom = 1.0;
//Draw rectangle to reconstruction
int g_pRectangle[4] = {0,0,0,0};
int g_pWorkingArea[4] = {0,0,0,0};
//Draw Range Line
int g_pRangeLine[4];
int g_nShowRangeLine = 0;
//Draw Infinity Point
int g_pInfinityPoint[2];
int g_nInfinityDisp;
//Draw Distance Points
int g_pDistancePoints[4];
int g_pDistancePointsClicked[2] = {0 , 0};
int g_pDistanceDraw = 0;
int g_nDistanceClicking = 0;

int g_pLinePoints[4];
int g_pLineDraw = 0;
int g_pLineIndex = 2;

FILTER_DESC* nl_simple_mono_right_filter;


// ----------------------------------------------------------------------------
// init -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void 
init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;

	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;

	g_nCenterX = input->vpw / 2.0;
	g_nCenterY = input->vph / 2.0;
}


// ----------------------------------------------------------------------------
// confiability - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
distance_between_2_points (float x1, float y1, float x2, float y2)
{    
	double p_dblRightWorldPoint1[3], p_dblLeftWorldPoint1[3], p_dblLeftPoint1[2], p_dblRightPoint1[2];
	double p_dblRightWorldPoint2[3], p_dblLeftWorldPoint2[3], p_dblLeftPoint2[2], p_dblRightPoint2[2];
	int xo, yo;
	double disp1, disp2, distance;

	int wo = nl_disparity_map.dimentions.x; 
	int ho = nl_disparity_map.dimentions.y; 

	int wi = image_right.ww; 
	int hi = image_right.wh; 

	//Gets disparity from map
	yo = (int)((float)(y1*ho))/(float)hi + 0.5; 
	xo = (int)((float)(x1*wo))/(float)wi + 0.5;
	disp1 = nl_disparity_map.neuron_vector[(yo*wo) + xo].output.fval;
	printf ("Point 1: xo=%d yo=%d disp=%lf\n", 
	xo, 
	yo, 
	disp1);

	yo = (int) ((float)(y2*ho)) / (float) hi + 0.5; 
	xo = (int) ((float)(x2*wo)) / (float) wi + 0.5;
	disp2 = nl_disparity_map.neuron_vector[(yo*wo) + xo].output.fval;
	printf ("Point 2: xo=%d yo=%d disp=%lf\n", 
	xo, 
	yo, 
	disp2);

	p_dblRightPoint1[0] = (double) x1;
	p_dblRightPoint1[1] = (double) y1;

	p_dblRightPoint2[0] = (double) x2;
	p_dblRightPoint2[1] = (double) y2;

	p_dblLeftPoint1[0] = (double) (x1 + disp1);
	p_dblLeftPoint1[1] = (double) y1;

	p_dblLeftPoint2[0] = (double) (x2 + disp2);
	p_dblLeftPoint2[1] = (double) y2;

	//First Point
	StereoTriangulation (1, p_dblLeftWorldPoint1, p_dblRightWorldPoint1, p_dblLeftPoint1, p_dblRightPoint1);

	printf ("Point 1: x=%lf y=%lf z=%lf\n", 
	p_dblRightWorldPoint1[0], 
	p_dblRightWorldPoint1[1], 
	p_dblRightWorldPoint1[2]);

	//Second Point
	StereoTriangulation (1, p_dblLeftWorldPoint2, p_dblRightWorldPoint2, p_dblLeftPoint2, p_dblRightPoint2);

	printf ("Point 2: x=%lf y=%lf z=%lf\n", 
	p_dblRightWorldPoint2[0], 
	p_dblRightWorldPoint2[1], 
	p_dblRightWorldPoint2[2]);

	//Calculate distance       
	distance = sqrt (
	      (p_dblRightWorldPoint1[0] - p_dblRightWorldPoint2[0]) * (p_dblRightWorldPoint1[0] - p_dblRightWorldPoint2[0]) + 
	      (p_dblRightWorldPoint1[1] - p_dblRightWorldPoint2[1]) * (p_dblRightWorldPoint1[1] - p_dblRightWorldPoint2[1]) + 
	      (p_dblRightWorldPoint1[2] - p_dblRightWorldPoint2[2]) * (p_dblRightWorldPoint1[2] - p_dblRightWorldPoint2[2])); 
	printf ("Distance = %3f\n", distance);
}   


void
compute_working_area (void)
{
	if (g_pRectangle[0] == -1)
	{
		g_pWorkingArea[0] = g_pWorkingArea[2] = g_pWorkingArea[1] = g_pWorkingArea[3] = 0;
		return;
	}

	if (g_pRectangle[0] < g_pRectangle[2])
	{
		g_pWorkingArea[0] = g_pRectangle[0];
		g_pWorkingArea[2] = g_pRectangle[2];
	}
	else
	{
		g_pWorkingArea[0] = g_pRectangle[2];
		g_pWorkingArea[2] = g_pRectangle[0];
	}

	if (g_pRectangle[1] < g_pRectangle[3])
	{
		g_pWorkingArea[1] = g_pRectangle[1];
		g_pWorkingArea[3] = g_pRectangle[3];
	}
	else
	{
		g_pWorkingArea[1] = g_pRectangle[3];
		g_pWorkingArea[3] = g_pRectangle[1];
	}
}


/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void 
robot_input_mouse (int button, int state, int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	GLdouble wxu, wyu;
	INPUT_DESC *input;

	input = get_input_by_win (glutGetWindow ());

	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note: viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;
	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 
	input->wxme = (GLint) (wx + 0.5);
	input->wyme = (GLint) (wy + 0.5);

	switch (button) 
	{
		case GLUT_LEFT_BUTTON:
			input->mouse_button = GLUT_LEFT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->mouse_state = GLUT_DOWN;
				input->x_old = input->x;
				input->y_old = input->y;
				input->x = x;
				input->y = y;
				input->wxd_old = input->wxd;
				input->wyd_old = input->wyd;
				input->wxd = (GLint) (wx + 0.5);
				input->wyd = (GLint) (wy + 0.5);
				
				if (((input->wxd >= input->wx) && (input->wxd <= (input->wx+input->ww))) && 
				    ((input->wyd >= input->wy) && (input->wyd <= (input->wy+input->wh))))
					input->valid = 1;

				if (input->valid)
				{
					g_pRectangle[0] = g_pRectangle[2] = input->wxd;
					g_pRectangle[1] = g_pRectangle[3] = input->wyd;
				}
				else
				{
					g_pRectangle[0] = -1;
				}
				
				update_input_status (input);
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
				input->valid = 0;
				wxu = (GLint) (wx + 0.5);
				wyu = (GLint) (wy + 0.5);
				
				if (((wxu >= input->wx) && (wxu <= (input->wx+input->ww))) && 
				    ((wyu >= input->wy) && (wyu <= (input->wy+input->wh))))
				{
					g_pRectangle[2] = wxu;
					g_pRectangle[3] = wyu;
					compute_working_area ();
				}
				else
				{
					g_pRectangle[0] = -1;
				}
				glutPostWindowRedisplay (input->win);
			}
			break;
		case GLUT_RIGHT_BUTTON:
			input->mouse_button = GLUT_RIGHT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
				glutPostWindowRedisplay (input->win);

				//Para testar a geracao da linha
				if ((input->win == image_right.win))
				{
					g_pLineIndex = (g_pLineIndex + 1) % 3;
					printf("g_pLineIndex = %d\n", g_pLineIndex);

					if (g_pLineIndex < 2)
					{ 
						g_pLinePoints[g_pLineIndex * 2 + 0] = image_right.wxme;
						g_pLinePoints[g_pLineIndex * 2 + 1] = image_right.wyme;
						printf("image_right.wxme = %d image_right.wyme = %d\n", 
							image_right.wxme, image_right.wyme);

						fprintf(stderr, "(%d, %d), (%d, %d)\n",
						g_pLinePoints[0], g_pLinePoints[1], g_pLinePoints[2], g_pLinePoints[3]);
					}

					if (g_pLineIndex == 0 || g_pLineIndex == 2)
						g_pLineDraw = 0;
					else
						g_pLineDraw = 1;
				}
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			input->mouse_button = GLUT_MIDDLE_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{           
				input->mouse_state = GLUT_UP;
				glutPostWindowRedisplay (input->win);
			}
			break;
		default:
			break;
	}
	if (input->input_controler != NULL)
		(*(input->input_controler)) (input, SET_POSITION);
}

void 
draw_point(float x, float y)
{
	int i;

	glBegin(GL_POINTS);
	for (i = 0; i <= 5; i++)
	{
		glVertex2f (x + i, y);
		glVertex2f (x - i, y);
		glVertex2f (x, y - i);
		glVertex2f (x, y + i);
	}
	glEnd ();
}

/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void 
robot_input_display (void)
{
	INPUT_DESC *input;
	GLdouble w, h, d;
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
	glTranslatef (-g_fltZoom * w/2.0, -g_fltZoom * h/2.0, -d);
	
	glTranslatef (g_fltZoom * (w/2.0 - g_nCenterX), g_fltZoom * (h/2.0 - g_nCenterY), 0.0);
	glScalef (g_fltZoom, g_fltZoom, g_fltZoom);

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

 
        //Draw the range line and the infinity point
        if (strcmp(input->name, image_left.name) == 0 && g_nShowRangeLine)
        {
		glColor3f (0.0, 1.0, 0.0);
		glBegin(GL_LINES);
			glVertex2f (g_pRangeLine[0], g_pRangeLine[1]);
			glVertex2f (g_pRangeLine[2], g_pRangeLine[3]);
		glEnd ();
		glColor3f (0.0, 0.0, 1.0);
		draw_point(g_pInfinityPoint[0], g_pInfinityPoint[1]);
        }
        
        //Draw distance points
        if (strcmp(input->name, image_right.name) == 0 && g_pDistanceDraw)
        {
		glColor3f (1.0, 1.0, 0.0);
		if (g_pDistancePointsClicked[0])
			draw_point(g_pDistancePoints[0], g_pDistancePoints[1]);

		if (g_pDistancePointsClicked[1])
			draw_point(g_pDistancePoints[2], g_pDistancePoints[3]);
        }

	//Draw line points
        if (strcmp(input->name, image_right.name) == 0 && g_pLineDraw)
        {
		glColor3f (1.0, 1.0, 1.0);
		glBegin(GL_LINES);
			glVertex2f (g_pLinePoints[0], g_pLinePoints[1]);
			glVertex2f (g_pLinePoints[2], g_pLinePoints[3]);
		glEnd ();
        }

	glColor3f (1.0, 0.0, 0.0);
	if (move_active == 1)
		DRAW_XY_MARK(input->wxd, input->wyd, input->vpw, input->vph);
	
	if ((move_active == 1) && (draw_active == 1))
		DRAWCROSS(input->wxd, input->wyd, input->ww, input->wh);
	
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
	

	glEnable (GL_COLOR_LOGIC_OP);
	glLogicOp (GL_XOR);
	if (g_pRectangle[0] != -1)
	{
		glColor3f (1.0, 1.0, 1.0);
		glLineStipple (1, 0x0F0F);
		DRAWSQUARE (g_pRectangle[0], g_pRectangle[1], g_pRectangle[2] - g_pRectangle[0], g_pRectangle[3] - g_pRectangle[1]);
	}
	
	glLogicOp (GL_COPY);

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
		
	glutSwapBuffers ();
	input->waiting_redisplay = 0;
}


/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void 
robot_check_input_bounds (INPUT_DESC *input, int wx, int wy)
{
	int redraw = 0;
	
	if (input->wx < 0)
	{
		if (input->vpxo > 0)
		{
			input->vpxo += input->wx;
			if (input->vpxo < 0)
				input->vpxo = 0;
			redraw = 1;
		} 
		input->wx = 0;
	}
	else if ((input->wx+input->neuron_layer->dimentions.x) >= input->vpw)
	{
		if (input->vpxo+input->vpw < input->ww)
		{
			input->vpxo += (input->wx+input->neuron_layer->dimentions.x) - input->vpw;
			if (input->vpxo+input->vpw > input->ww)
				input->vpxo = input->ww - input->vpw;
			redraw = 1;
		} 
		input->wx = input->vpw - input->neuron_layer->dimentions.x;
	}


	if (input->wy < 0)
	{
		if (input->vpyo+input->vph < input->wh)
		{
			input->vpyo -= input->wy;
			if (input->vpyo+input->vph > input->wh)
				input->vpyo = input->wh - input->vph;
			redraw = 1;
		} 
		input->wy = 0;
	}
	else if ((input->wy+input->neuron_layer->dimentions.y) >= input->vph)
	{
		if (input->vpyo > 0)
		{
			input->vpyo -= (input->wy+input->neuron_layer->dimentions.y) - input->vph;
			if (input->vpyo < 0)
				input->vpyo = 0;
			redraw = 1;
		} 
		input->wy = input->vph - input->neuron_layer->dimentions.y;
	}

	if (redraw && (input->waiting_redisplay == 0))
	{
		glutSetWindow (input->win);
		glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, 
			input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
		input->waiting_redisplay = 1;
	}
}



// ----------------------------------------------------------------------------
// robot_make_input_image - Inicializa a estrutura image
// da input
//
// Entrada: input - Descritor da input
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
robot_make_input_image (INPUT_DESC *input)
{
	int w, h;
	char message[256];

	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;

	// Compute the input image dimentions
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	// Save the image dimentions
	input->ww = w;
	input->wh = h;
	
	switch (TYPE_SHOW)
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
	
	if (input->image == (GLubyte *) NULL)
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
}



void 
robot_input_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	INPUT_DESC *input;
	
	input = get_input_by_win (glutGetWindow ());
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	g_pRectangle[2] = (GLint) (wx + 0.5);
	g_pRectangle[3] = (GLint) (wy + 0.5);

	glutPostWindowRedisplay (input->win);
}



// ----------------------------------------------------------------------------
// InitWindow - Inicializa os recursos de cada janela
//				de entrada
//
// Entrada: input - Janela de entrada
//			nInputImage - Indica qual e a janela.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void 
InitWindow(INPUT_DESC *input)
{
	int x, y;
	int nImageSource;

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	switch (nImageSource)
	{
		case GET_IMAGE_FROM_RBT:
		{
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_FROM_PNM:
		{
			make_input_image (input);
		}
		break;
		
		case GET_IMAGE_FROM_SMV:
		{
			robot_make_input_image (input);
		}
		break;
		
		default:
			Erro ("Invalid image source.", "", "");
	}

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
	glutDisplayFunc (robot_input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMotionFunc (robot_input_motion);
	glutMouseFunc (robot_input_mouse);
}



// ----------------------------------------------------------------------------
// init_user_functions - initializes the user functions
//
// Inputs: none
//
// Output: returns zero if OK
// ----------------------------------------------------------------------------

int 
init_user_functions (void)
{    
	interpreter ("toggle move_active;");
    	interpreter ("toggle draw_active;");

	// Initializes the stereo system module
	if (StereoInitialize (INPUT_WIDTH, INPUT_HEIGHT))
	{
		ERROR ("While initializing the MAE framework, stereo system module initialization failure (MaeInitialize).", "", "");
		return (-1);
	}
	
	LoadStereoParameters ("Calib_Results_stereo_rectified.mat");
	LoadStereoRectificationIndexes ("Calib_Results_rectification_indexes.mat");
	
	// Initializes the Viewer module	
	ViewerSetParameterf (VIEWER_FOVY, FOVY);
	ViewerSetParameterf (VIEWER_FAR, FAR);
	ViewerSetParameterf (VIEWER_NEAR, NEAR);
	ViewerSetParameterf (VIEWER_FOCUS, FOCUS);		
	ViewerSetParameteri (VIEWER_IMAGE_WIDTH, INPUT_WIDTH);
	ViewerSetParameteri (VIEWER_IMAGE_HEIGHT, INPUT_HEIGHT);
	ViewerInitialize (2);
	
	return (0);
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void 
update_simple_mono_cells_filters ()
{
	FILTER_LIST *f_list;

	//Move to center of the image
	image_left.wxd = ((double)image_left.ww)/2.0;
	image_left.wyd = ((double)image_left.wh)/2.0;

	//Move to center of the image
	image_right.wxd = ((double)image_right.ww)/2.0;
	image_right.wyd = ((double)image_right.wh)/2.0;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next)
	{
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_filter") == 0)
		{
			nl_simple_mono_right_filter = f_list->filter_descriptor;
			filter_update (nl_simple_mono_right_filter);
		}

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_q_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_q_filter") == 0)
			filter_update (f_list->filter_descriptor);
			
/*		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_q_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_q_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_illuminance_right_filter") == 0)
		{
			int i;
			for(i = 0; i < NUMBER_ILLUMINANCE_ITERACTIONS; i++)
				filter_update (f_list->filter_descriptor);
		}
			
		if (strcmp(f_list->filter_descriptor->name, "nl_reflectance_right_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_trunk_segmentation_map_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_vertical_segmentation_map_filter") == 0)
			filter_update (f_list->filter_descriptor);
*/	}
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void 
update_mt_cells_filters ()
{
	FILTER_LIST *f_list;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next)
	{
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_filter") == 0)
			filter_update (f_list->filter_descriptor);
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_gaussian_filter") == 0)
			filter_update (f_list->filter_descriptor);
	}
}



void 
generate_scan_range (int x, int y)
{
	int nInfinity;
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3];

	// Define the scaning range
	p_dblRightPoint[0] = (double) x;
	p_dblRightPoint[1] = (double) y;

	// Calculates the infinity distance position
	GetWorldPointAtDistanceRight(p_dblWorldPoint, p_dblRightPoint, INFINITY_DISTANCE, RIGHT_CAMERA);
	CameraProjectionRight(1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nInfinity = (int) (p_dblLeftPoint[0] + 0.5);
	g_pInfinityPoint[0] = nInfinity;
	g_pInfinityPoint[1] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5);

	// Gets the fartest left point
	GetWorldPointAtDistanceRight(p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight(1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	g_pRangeLine[0] = (int) (p_dblLeftPoint[0] + 0.5);
	g_pRangeLine[0] = (g_pRangeLine[0] < 0) ? 0 : g_pRangeLine[0];
	g_pRangeLine[1] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5);

	// Gets the nearest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	g_pRangeLine[2] = (int) (p_dblLeftPoint[0] + 0.5);
	g_pRangeLine[2] = (g_pRangeLine[2] >= image_left.ww) ? image_left.ww - 1: g_pRangeLine[2];
	g_pRangeLine[3] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5);
}

// ----------------------------------------------------------------------------
// input_generator -
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------

void 
input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		InitWindow(input);
		update_input_neurons (input);
	}

	if (((input->win == image_left.win) || (input->win == image_right.win)) && (input->win != 0) && (status == MOVE))
	{
		// Draws the Infinity point and the line that marks the scan range.
		if (g_nShowRangeLine) 
			generate_scan_range (image_right.wxd, image_right.wh - image_right.wyd - 1);

		robot_check_input_bounds (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		robot_input_display ();
		all_outputs_update ();

	}

	if ((input->win == image_right.win) && (input->win != 0) && (status == MOVE))
	{
		if (g_nDistanceClicking)
		{
			if (!g_pDistancePointsClicked[0])
			{
				g_pDistancePoints[0] = image_right.wxd;
				g_pDistancePoints[1] = image_right.wyd;
				g_pDistancePointsClicked[0] = 1;
			}
			else if (!g_pDistancePointsClicked[1])
			{
				g_pDistancePoints[2] = image_right.wxd;
				g_pDistancePoints[3] = image_right.wyd;
				g_pDistancePointsClicked[1] = 1;
				g_nDistanceClicking = 0;

				//Print the distance
				distance_between_2_points (g_pDistancePoints[0], g_pDistancePoints[1], g_pDistancePoints[2], g_pDistancePoints[3]);
			}
		}
	}
}



// ----------------------------------------------------------------------------
// draw_output -
//
// Inputs:
//
// Output: Nenhuma.
// ----------------------------------------------------------------------------

void 
draw_output (char *output_name, char *input_name)
{

}


int
outside_working_area (int *g_pWorkingArea, int x, int y)
{
	if ((x < g_pWorkingArea[0]) || (x >= g_pWorkingArea[2]) ||
	    (y < g_pWorkingArea[1]) || (y >= g_pWorkingArea[3]))
	    	return (1);
	else
		return (0);
}



// ----------------------------------------------------------------------------
// ReconstructByDisparityMapPlan - Reconstroi o mundo em
// torno do ponto de vergencia atraves do mapa de
// disparidade considerando uma amostragem PLANA.
//
// Entrada: Nenhuma.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void 
ReconstructByDisparityMapPlan ()
{
	int w, h, wi, hi, xi, yi, yo, xo, num_points;
	float fltDisparity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;
	double *p_dblLeftPoint, *p_dblRightPoint, *p_dblLeftWorldPoint, *p_dblRightWorldPoint;
	GLubyte cRed, cGreen, cBlue;
	
	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	fltRangeCols = 1.0;
	nStartCol = w * ((1.0 - fltRangeCols) / 2.0);
	nEndCol = w - nStartCol;

	p_dblLeftPoint = (double *) alloc_mem (NL_WIDTH*NL_HEIGHT * 2 * sizeof (double));
	p_dblRightPoint = (double *) alloc_mem (NL_WIDTH*NL_HEIGHT * 2 * sizeof (double));
	p_dblLeftWorldPoint = (double *) alloc_mem (NL_WIDTH*NL_HEIGHT * 3 * sizeof (double));
	p_dblRightWorldPoint = (double *) alloc_mem (NL_WIDTH*NL_HEIGHT * 3 * sizeof (double));

	num_points = 0;
	for (xo = 0; xo < w; xo++)
	{
		xi = (int) ((float) (xo * wi) / (float) w + 0.5);

		for (yo = 0; yo < h; yo++)
		{
			yi = (int) ((float) (yo * hi) / (float) h + 0.5);
	
			if (outside_working_area (g_pWorkingArea, xi, yi))
			        continue;
			        
			p_dblRightPoint[num_points] = (double) xi;
			p_dblRightPoint[num_points+1] = (double) yi;

			fltDisparity = nl_disparity_map.neuron_vector[w * yo + xo].output.fval;
	
			p_dblLeftPoint[num_points] = (double) xi + fltDisparity;
			p_dblLeftPoint[num_points+1] = (double) yi;
			
			num_points += 2;
		}
	}
	StereoTriangulation (num_points / 2, p_dblLeftWorldPoint, p_dblRightWorldPoint, p_dblLeftPoint, p_dblRightPoint);
//	project_points (num_points / 2, p_dblLeftWorldPoint, p_dblLeftPoint, p_dblRightPoint);
//	display_points (num_points / 2, p_dblLeftPoint, p_dblRightPoint);
	num_points = 0;
	for (xo = 0; xo < w; xo++)
	{
		xi = (int) ((float) (xo * wi) / (float) w + 0.5);

		for (yo = 0; yo < h; yo++)
		{
			yi = (int) ((float) (yo * hi) / (float) h + 0.5);
	
			if (outside_working_area (g_pWorkingArea, xi, yi))
			        continue;
			        
			pixel = image_right.neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			cRed = RED(pixel);
			cGreen = GREEN(pixel);
			cBlue = BLUE(pixel);

			// Updates the map and rotates the frame coordinates			       			
			ViewerUpdateMap(0, p_dblRightWorldPoint[num_points+2], -p_dblRightWorldPoint[num_points+0], p_dblRightWorldPoint[num_points+1], cRed, cGreen, cBlue);
			num_points += 3;
		}
	}
	free (p_dblLeftPoint);
	free (p_dblRightPoint);
	free (p_dblLeftWorldPoint);
	free (p_dblRightWorldPoint);
}



// ----------------------------------------------------------------------------
// AddLocalMin - 
//
// Inputs: 
//
// Output: none
// ----------------------------------------------------------------------------

void 
AddLocalMin (DISP_DATA* dispData, int i, double minus_out, int pos)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
	{
		if (minus_out <= dispData->samples[victim_sample][i].val)
			break;
	}
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			dispData->samples[moving_sample][i] = dispData->samples[moving_sample - 1][i];
			moving_sample--;
		}
		dispData->samples[moving_sample][i].val = minus_out;
		dispData->samples[moving_sample][i].pos = pos;
	}
}


// ----------------------------------------------------------------------------
// distance_neig - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

int 
distance_neig (SAMPLES *sample, int pos, int x, int y, int w, int h)
{
	int dist, count;
	
	dist = count = 0;
	if (x-1 >= 0)
	{
		dist += abs (sample[y*w + x-1].pos - pos);
		count++;
	}
	if (x+1 < w)
	{
		dist += abs (sample[y*w + x+1].pos - pos);
		count++;
	}
	if (y-1 >= 0)
	{
		dist += abs (sample[(y-1)*w + x].pos - pos);
		count++;
	}
	if (y+1 < h)
	{
		dist += abs (sample[(y+1)*w + x].pos - pos);
		count++;
	}

	if ((x-1 >= 0) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x-1].pos - pos);
		count++;
	}
	if ((x-1 >= 0) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x-1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x+1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x+1].pos - pos);
		count++;
	}
	
	return ((int) ((double) dist / (double) count + 0.5));
}



// ----------------------------------------------------------------------------
// calculate_disparity - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
winner_takes_it_all (DISP_DATA* dispData, NEURON_LAYER *nlDisparityMap, int wxd)
{
	SAMPLES temp;
	int x, y, w, h, i, sample, best_sample, cur_dist, dist;
	
	w = nlDisparityMap->dimentions.x;
	h = nlDisparityMap->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (best_sample = sample = 0; sample < NUM_SAMPLES; sample++)
			{
				dist = distance_neig (dispData->samples[0], dispData->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = dispData->samples[0][y * w + x];
			dispData->samples[0][y * w + x] = dispData->samples[best_sample][y * w + x];
			dispData->samples[best_sample][y * w + x] = temp;
		}
	}

	for (i = 0; i < w*h; i++)
	{
		nlDisparityMap->neuron_vector[i].output.fval = (float) dispData->samples[0][i].pos;
	}
}



// ----------------------------------------------------------------------------
// get_scan_range - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
get_scan_range (int *nMinScan, int *nMaxScan)
{
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3]; 

	// Define the scaning range
	p_dblRightPoint[0] = ((double) image_right.ww) / 2.0;
	p_dblRightPoint[1] = ((double) image_right.wh) / 2.0 - 1.0;

	// Gets the fartest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	*nMinScan = (int) (p_dblLeftPoint[0] + 0.5);
	*nMinScan = (*nMinScan < 0) ? 0 : *nMinScan;

	// Gets the nearest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	*nMaxScan = (int) (p_dblLeftPoint[0] + 0.5);
	*nMaxScan = (*nMaxScan >= image_left.ww) ? image_left.ww - 1: *nMaxScan;
}



void
alloc_and_init_generate_disparity_map_plan_variables (DISP_DATA **p_dispData, float **p_dispAux)
{
	int num_neurons, i, sample;
	float *dispAux;
	DISP_DATA *dispData;

	num_neurons = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;

	dispAux = (float *) malloc (sizeof (float) * num_neurons);

	// Aloca memoria para a estrutura DISP_DATA
	dispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
	dispData->neg_slope = (char *) alloc_mem (sizeof (char) * num_neurons);
	dispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
	for (i = 0; i < NUM_SAMPLES; i++)
		dispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * num_neurons);

	// Inicializar a estrutura DISP_DATA
	for (i = 0; i < num_neurons; i++)
		dispData->neg_slope[i] = 1;
	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < num_neurons; i++)
		{
			dispData->samples[sample][i].val = FLT_MAX;
			dispData->samples[sample][i].pos = 0;
		}
	}

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < num_neurons; i++)
	{
		dispAux[i] = FLT_MAX;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = .0f;
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	}
	
	*p_dispAux = dispAux;
	*p_dispData = dispData;
}


void
free_generate_disparity_map_plan_variables (DISP_DATA *dispData, float *dispAux)
{
	int i;

	for (i = 0; i < NUM_SAMPLES; i++)
		free (dispData->samples[i]);

	free (dispData->samples);
	free (dispData->neg_slope);
	free (dispData);
	free (dispAux);
}


void
add_subpixel_disparity_to_disparity_map_plan (void)
{
	int i, n = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;
	for (i = 0; i < n; i++)
		nl_disparity_map.neuron_vector[i].output.fval +=
			nl_subpixel_disparity_map.neuron_vector[i].output.fval;
}

/*void
generate_subpixel_disparity_map_plan (void)
{
	double phaseL1 = PHASE_SIMPLE_LEFT_1;
	double phaseR1 = PHASE_SIMPLE_RIGHT_2;
	double phaseL2 = PHASE_SIMPLE_LEFT_2;
	double phaseR2 = PHASE_SIMPLE_RIGHT_1 + pi;

	RECEPTIVE_FIELD_DESCRIPTION *receptive_field = nl_simple_mono_right_filter->private_state;
	double frequency = receptive_field->frequency;
	int kernel_type = nl_simple_mono_right_filter->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;
	int kernel_num_points = receptive_field->num_points;

	double deltaPhase1 = phaseL1 - phaseR1;
	double deltaPhase2 = phaseL2 - phaseR2;
	int kernel_size = kernel_num_points / (kernel_type + 1);
			
	robot_mt_cell_subpixel_disparity (&nl_subpixel_disparity_map, &nl_disparity_map,
					  &nl_simple_mono_left, &nl_simple_mono_right_subpixel,
					  &nl_simple_mono_left_q, &nl_simple_mono_right_q_subpixel,
					  &nl_simple_mono_left_subpixel, &nl_simple_mono_right,
					  &nl_simple_mono_left_q_subpixel, &nl_simple_mono_right_q,
					  deltaPhase1, deltaPhase2, kernel_size);
}*/

// ----------------------------------------------------------------------------
// generate_disparity_map_plan - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
generate_disparity_map_plan (void)
{
	int nMaxScan, nMinScan, leftPoint;
	int i, x, y, nIndex;
	float fltMinSum, current_mt_gaussian;
	float *mt_gaussian_with_previous_disparity;
	DISP_DATA *dispData;
	int wo, ho;
	
	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	alloc_and_init_generate_disparity_map_plan_variables (&dispData, &mt_gaussian_with_previous_disparity);
	
	get_scan_range (&nMinScan, &nMaxScan);	

	fltMinSum = FLT_MAX;
	for (leftPoint = nMinScan; leftPoint <= nMaxScan; leftPoint++)
	{
		g_nCurrDisparity = leftPoint - (int) (((double) image_right.ww) / 2.0);
		update_mt_cells_filters ();
		for (y = 0; y < ho; y++) 
		{
			for (x = 0; x < wo; x++)
			{
				nIndex = y * wo + x;
				current_mt_gaussian = nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				if (current_mt_gaussian >= mt_gaussian_with_previous_disparity[nIndex])
				{
					if (dispData->neg_slope[nIndex])
					{
						dispData->neg_slope[nIndex] = 0;
						AddLocalMin (dispData, nIndex, mt_gaussian_with_previous_disparity[nIndex], g_nCurrDisparity);
					}
				}
				else
				{
					dispData->neg_slope[nIndex] = 1;
				}
				mt_gaussian_with_previous_disparity[nIndex] = current_mt_gaussian;
			}
		}
	}

	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = dispData->samples[0][i].val;
		nl_disparity_map.neuron_vector[i].output.fval = dispData->samples[0][i].pos;
	}

	// Improve the disparity map by applying the WINNER TAKES IT ALL algorithm. 
	for (i = 0; i < WINNER_TAKES_IT_ALL_STEPS; i++)
		winner_takes_it_all (dispData, &nl_disparity_map, image_left.wxd);

	free_generate_disparity_map_plan_variables (dispData, mt_gaussian_with_previous_disparity);
}





// ----------------------------------------------------------------------------
// input_controler -
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------

void 
input_controler (INPUT_DESC *input, int status)
{
        if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_right.win))
	{
		//PESQUISA:Teste de disparidade subpixel
/*		int wi = nl_simple_mono_left.dimentions.x;
		int yi = image_right.wyd;
		int xir = image_right.wxd;
		int xil = xir + (int)nl_disparity_map.neuron_vector[(int)(
			(( yi * nl_disparity_map.dimentions.y) / (double) nl_simple_mono_left.dimentions.y *
			nl_subpixel_disparity_map.dimentions.x) + 
			( xir * nl_disparity_map.dimentions.x) / (double) nl_simple_mono_left.dimentions.x)].output.fval;

		xil = image_left.wxd;
		
		if (xil < 0 || xil >= wi)
		{ 
			printf("Fora dos limites\n");
		}
		else
		{
			int kernel_type = nl_simple_mono_right_filter->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;
			RECEPTIVE_FIELD_DESCRIPTION *receptive_field = nl_simple_mono_right_filter->private_state;
			int kernel_num_points = receptive_field->num_points;
			int kernel_size = kernel_num_points / (kernel_type + 1);
			double deltaPhase1 = acos(-1) * (45 - 0)/180.0;
			double deltaPhase2 = acos(-1) * (0 - 45)/180.0;
			double 	sc_l1_45, sc_r1_0, sc_q_l1_135, sc_q_r1_90, sc_l2_0, sc_r2_45, sc_q_l2_90, 
				sc_q_r2_135;

			sc_l1_45   = nl_simple_mono_left.neuron_vector[(yi*wi) + xil].output.fval;
			sc_r1_0    = nl_simple_mono_right_subpixel.neuron_vector[(yi*wi) + xir].output.fval;
			sc_q_l1_135= nl_simple_mono_left_q.neuron_vector[(yi*wi) + xil].output.fval;
			sc_q_r1_90 = nl_simple_mono_right_q_subpixel.neuron_vector[(yi*wi) + xir].output.fval;

			sc_l2_0    = nl_simple_mono_right_subpixel.neuron_vector[(yi*wi) + xil].output.fval;
			sc_r2_45   = -nl_simple_mono_right.neuron_vector[(yi*wi) + xir].output.fval;
			sc_q_l2_90 = nl_simple_mono_left_q_subpixel.neuron_vector[(yi*wi) + xil].output.fval;
			sc_q_r2_135= -nl_simple_mono_right_q.neuron_vector[(yi*wi) + xir].output.fval;

			printf("Disparidade subpixel = %f\n", 
				     calculate_subpixel_disparity (sc_l1_45, sc_r1_0, sc_q_l1_135, 
				     sc_q_r1_90, sc_l2_0, sc_r2_45, 
				     sc_q_l2_90, sc_q_r2_135,
				     deltaPhase1, deltaPhase2, kernel_size));		
		}
*/		//PESQUISA:Fim

		image_left.wxd += image_right.wxd - image_right.wxd_old;
		image_left.wyd = image_right.wyd;

		move_input_window (image_right.name, image_right.wxd, image_right.wyd);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		glutPostWindowRedisplay (image_right.win);
		glutPostWindowRedisplay (image_left.win);
		
	}

        if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_left.win))
	{
		image_left.wyd = image_right.wyd;

		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		glutPostWindowRedisplay (image_left.win);
	}

	
	input->mouse_button = -1;
}



// ----------------------------------------------------------------------------
// output_handler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void 
output_handler (OUTPUT_DESC *output, int status)
{

}



// ----------------------------------------------------------------------------
// Reconstruct -
//
// Entrada:
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
Reconstruct ()
{
	printf("Reconstruct\n");

	ReconstructByDisparityMapPlan ();
	ViewerRedisplay ();
	glutIdleFunc ((void (* ) (void)) check_forms);
}



// ----------------------------------------------------------------------------
// LowPass - Passa uma gaussiana no mapa de disparidade
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
LowPass ()
{
	RECEPTIVE_FIELD_DESCRIPTION *receptiveField;
	int x, y, wo, ho;
	float * dispAux;

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	dispAux = (float*)malloc(sizeof(float)*wo*ho);

	receptiveField = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
	compute_gaussian_kernel(receptiveField, 5, 1.0);


	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
			dispAux[y * wo + x] = apply_gaussian_kernel(receptiveField, &nl_disparity_map, x, y);
	}

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
			nl_disparity_map.neuron_vector[y * wo + x].output.fval = dispAux[y * wo + x];
	}

	free(receptiveField);
	free(dispAux);
}

// ----------------------------------------------------------------------------
// confiability - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
confiability (double limit)
{    
	int i; 
	int maxIndex;
	
	maxIndex = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;
	for (i = 0; i < maxIndex; i++)
	{
		//    printf("val = %f, limit = %f\n", nl_mt_gaussian_map.neuron_vector[i].output.fval, limit);
		if (nl_mt_gaussian_map.neuron_vector[i].output.fval > limit)
		{
			nl_disparity_map.neuron_vector[i].output.fval = g_nInfinityDisp;
			nl_mt_gaussian_map.neuron_vector[i].output.fval = 1;
		}
	}
}   

// ----------------------------------------------------------------------------
// rectify_image - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
rectify_image ()
{
	int i, j, w, h;
	static unsigned char *p_cUndistortedImage = NULL;
	static unsigned char *p_cDistortedImage = NULL;

	w = image_left.ww;
	h = image_left.wh;

	p_cDistortedImage = (unsigned char*) malloc (3 * w * h * sizeof (unsigned char));
	p_cUndistortedImage = (unsigned char*) malloc (3 * w * h * sizeof (unsigned char));

	//Rectify right image
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			p_cDistortedImage[(h - j - 1) + i * h + 0 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 0];
			p_cDistortedImage[(h - j - 1) + i * h + 1 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 1];
			p_cDistortedImage[(h - j - 1) + i * h + 2 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 2];			
		}	
	}

	RectifyLeftImage (p_cUndistortedImage, p_cDistortedImage, w, h);

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			image_left.image[3 * (i + j * image_left.tfw) + 0] = p_cUndistortedImage[(h - j - 1) + i * h + 0 * w * h];
			image_left.image[3 * (i + j * image_left.tfw) + 1] = p_cUndistortedImage[(h - j - 1) + i * h + 1 * w * h];
			image_left.image[3 * (i + j * image_left.tfw) + 2] = p_cUndistortedImage[(h - j - 1) + i * h + 2 * w * h];
		}
	}

	//Rectify right image
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			p_cDistortedImage[(h - j - 1) + i * h + 0 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 0];
			p_cDistortedImage[(h - j - 1) + i * h + 1 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 1];
			p_cDistortedImage[(h - j - 1) + i * h + 2 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 2];			
		}	
	}

	RectifyRightImage (p_cUndistortedImage, p_cDistortedImage, w, h);

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			image_right.image[3 * (i + j * image_right.tfw) + 0] = p_cUndistortedImage[(h - j - 1) + i * h + 0 * w * h];
			image_right.image[3 * (i + j * image_right.tfw) + 1] = p_cUndistortedImage[(h - j - 1) + i * h + 1 * w * h];
			image_right.image[3 * (i + j * image_right.tfw) + 2] = p_cUndistortedImage[(h - j - 1) + i * h + 2 * w * h];
        	}
	}
      
	free(p_cDistortedImage);
	free(p_cUndistortedImage);
}

// ----------------------------------------------------------------------------------
// f_keyboard -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------------

void 
f_keyboard (char *key_value)
{	
	static int nLock = 0;
	static double p_dblRightWorldPoints[6];
	static int nCurrentPoint = 0;
	double p_dblLeftWorldPoint[3], p_dblLeftPoint[2], p_dblRightPoint[2];
	time_t start, end;
	
	switch (key_value[0]) 
	{
		//--------------------------------------------------------------
		// Stereo system interface
		//--------------------------------------------------------------

		case 'A':
		{
			CYLINDER_LIST *guessed_cylinder_list;
			
			if (g_pWorkingArea[0] == 0 && g_pWorkingArea[1] == 0 && 
			    g_pWorkingArea[2] == 0 && g_pWorkingArea[3] == 0)
			{
				printf("Set working area!");
				break;   
			}
			
			guessed_cylinder_list = guess_cylinder_list (g_pWorkingArea, &nl_trunk_segmentation_map, 
					     			   &nl_disparity_map);
			cylinder_list_view (guessed_cylinder_list, ARCS, CIRCLES);
					     
			cylinder_list_print (guessed_cylinder_list);
			
			cylinder_list_free (guessed_cylinder_list);
			
			glutIdleFunc ((void (* ) (void)) check_forms);
		}
		break; 
		case 'a':
		{
			CYLINDER_LIST *cylinder_list;
			CYLINDER *nearest, *highest;
			
			if (g_pWorkingArea[0] == 0 && g_pWorkingArea[1] == 0 && 
			    g_pWorkingArea[2] == 0 && g_pWorkingArea[3] == 0)
			{
				printf("Set working area!");
				break;   
			}
			
			cylinder_list = fit_cylinder_list (g_pWorkingArea, &nl_trunk_segmentation_map, 
							  &nl_disparity_map, &nl_simple_mono_right);
						
			cylinder_list_view (cylinder_list, ARCS, CIRCLES);
			nearest = cylinder_list_get_nearest (cylinder_list);
			highest = cylinder_list_get_highest (cylinder_list);
			cylinder_3D_view_color (nearest, ARCS, CIRCLES, 0, 0, 0);
			cylinder_3D_view_color (highest, ARCS, CIRCLES, 0, 0, 0);
					     
			cylinder_list_print (cylinder_list);
			
			printf ("Volume estereo: %f m3\n", compute_stereo_volume (cylinder_list));
			
			cylinder_list_free (cylinder_list);
						
			glutIdleFunc ((void (* ) (void)) check_forms);
		}
		break; 
		case 'S':
		{
/*			int x_limit, y_limit;
			
			FILE *lines = fopen("lines.txt", "r");
			
			for (;;)
			{
				int read = fscanf(lines, "(%d, %d), (%d, %d)\n",
					g_pLinePoints, g_pLinePoints + 1, g_pLinePoints + 2, g_pLinePoints + 3);
				if (read < 4) break;
					
				segment_find_right_limit (&nl_segment, image_right.neuron_layer, g_pLinePoints, &x_limit, &y_limit);
			} */
			
//			int x_limit, y_limit;
//			segment_find_right_limit (&nl_segment, image_right.neuron_layer, g_pLinePoints, &x_limit, &y_limit);
			
			find_segmentation_borders (&nl_segment, image_right.neuron_layer, g_pLinePoints[0], g_pLinePoints[1], g_pWorkingArea[1], g_pWorkingArea[3]);
		}
		break;
		case 's':
		{
//			generate_subpixel_disparity_map_plan ();

			add_subpixel_disparity_to_disparity_map_plan();
		
			all_outputs_update();
		break;
		}
		case 'V':
		case 'v':
		{
			int w, h, wi, hi;


			// Dimensoes do cortex (disparity map)
			h = nl_disparity_map.dimentions.y;
			w = nl_disparity_map.dimentions.x;

			// Dimensoes da imagem de entrada
			hi = image_right.neuron_layer->dimentions.y;
			wi = image_right.neuron_layer->dimentions.x;

			//Passar retangulo convertido para o tamanho do mapa de disparidades
			estimate_volume(g_pWorkingArea, &nl_disparity_map, image_right.neuron_layer);

			all_outputs_update();
		break;
		}
		case 'C':
			// void cylinder (double *origin, double size, double diameter, double alpha, double betha, int arcs, int circles);
			test_StereoTriangulation_CameraProjection ();
			// cylinder (origin, 400.0, 100.0, 0.0, 0.0, 50, 50);
			// break;
			cylinder (100.0, -75.0, 1000.0, 400.0, 100.0, 40.0, 30.0, 150, 150);
			exchange_to_disparity_map();
			
			all_outputs_update();
		break;
		case 'c':
			confiability (0.3);
			all_outputs_update();
		break;
		case 'R':
		case 'r':
			Reconstruct ();
		break;
		case 'E':
			exchange_to_disparity_map();
			all_outputs_update();
		break;
		case 'e':
			ViewerEraseMap ();
		break;
		case 'F':
		case 'f':
			LowPass ();

			glutIdleFunc ((void (* ) (void)) check_forms);

			all_outputs_update();
			break;
		case 'D':
		case 'd':

			if (g_pDistanceDraw)
			{
				g_pDistanceDraw = 0;
				g_nDistanceClicking = 0;      
			}
			else
			{
				g_pDistanceDraw = 1;
				g_nDistanceClicking = 1;      
			}
			g_pDistancePointsClicked[0] = 0;
			g_pDistancePointsClicked[1] = 0;

			glutSetWindow(image_right.win);
			input_display ();
			glutIdleFunc ((void (* ) (void)) check_forms);
			break;
		case 'L':
		case 'l':
			g_nShowRangeLine = !g_nShowRangeLine;
		break;
		// Starts the vergence process
		case 'M':
			//Rectify Image   
			if (!nLock++)
			{ 
				start = time (NULL);
				rectify_image ();
				end = time (NULL);
				printf ("Time to rectify image: %f s\n", difftime (end, start));

				//Precisa fazer o update dos neuronios de entrada antes de aplicar as gabors,
				//pois os dados de entrada devem estar atualizados
				update_input_neurons (&(image_left));
				robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
				update_input_image (&(image_left));

				update_input_neurons (&(image_right));
				robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
				update_input_image (&(image_right));
			}
		break;
		case 'm':			
			//Rectify Image   
			if (!nLock++)
			{ 
				start = time (NULL);
				rectify_image ();
				end = time (NULL);
				printf ("Time to rectify image: %f s\n", difftime (end, start));

				//Precisa fazer o update dos neuronios de entrada antes de aplicar as gabors,
				//pois os dados de entrada devem estar atualizados
				update_input_neurons (&(image_left));
				robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
				update_input_image (&(image_left));

				update_input_neurons (&(image_right));
				robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
				update_input_image (&(image_right));
			}

			//Update Gabor layers
			start = time (NULL);
			update_simple_mono_cells_filters ();
			end = time (NULL);
			printf ("Time to update mono cells: %f s\n", difftime (end, start));

			//Generate disparity map
			start = time (NULL);
			generate_disparity_map_plan ();  
			end = time (NULL);
			printf ("Time to generate disparity map: %f s\n", difftime (end, start));
			
//			generate_subpixel_disparity_map_plan ();

//			add_subpixel_disparity_to_disparity_map_plan();

			all_outputs_update();
			
			glutIdleFunc ((void (* ) (void)) check_forms);
		break;
		// Maps the image left and right current points to world point
		case 'W':
		case 'w':
			p_dblLeftPoint[0] = (double) image_left.wxd;
			p_dblLeftPoint[1] = (double) (image_left.wh - image_left.wyd - 1);

			p_dblRightPoint[0] = (double) image_right.wxd;
			p_dblRightPoint[1] = (double) (image_right.wh - image_right.wyd - 1);

			//printf ("x' = (%.2f,%.2f), x = (%.2f,%.2f)\n", p_dblLeftPoint[0], p_dblLeftPoint[1], p_dblRightPoint[0], p_dblRightPoint[1]);

			StereoTriangulation (1, p_dblLeftWorldPoint, &(p_dblRightWorldPoints[3 * nCurrentPoint]), p_dblLeftPoint, p_dblRightPoint);
					
			nCurrentPoint = (nCurrentPoint + 1) % 2;
			
			printf ("%lf %lf %lf\n", p_dblRightWorldPoints[3 * nCurrentPoint + 0], 
						 p_dblRightWorldPoints[3 * nCurrentPoint + 1], 
						 p_dblRightWorldPoints[3 * nCurrentPoint + 2]);
			
			if (!nCurrentPoint)
				printf ("Distance = %3f\n", sqrt ((p_dblRightWorldPoints[0] - p_dblRightWorldPoints[3]) * (p_dblRightWorldPoints[0] - p_dblRightWorldPoints[3]) + 
								  (p_dblRightWorldPoints[1] - p_dblRightWorldPoints[4]) * (p_dblRightWorldPoints[1] - p_dblRightWorldPoints[4]) + 
								  (p_dblRightWorldPoints[2] - p_dblRightWorldPoints[5]) * (p_dblRightWorldPoints[2] - p_dblRightWorldPoints[5]))); 
			break;
		// Updates the input images from network cameras
		case 'U':
		case 'u':
    			system (CAPTURE_SCRIPT);
			
			// Updates the left input image
			load_input_image (&(image_left), NULL);
			robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
			update_input_image (&(image_left));
			
			// Updates the right input image
			load_input_image (&(image_right), NULL);
			robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
			update_input_image (&(image_right));
			
			nLock = 0;
			break;
		// Zoom out
		case 'z':
			g_fltZoom *= 0.5;			
			glutSetWindow(image_left.win);
			robot_input_display ();
			glutSetWindow(image_right.win);
			robot_input_display ();
			break;
		// Zoom in
		case 'Z':
			g_fltZoom *= 2.0;
			glutSetWindow(image_left.win);
			robot_input_display ();
			glutSetWindow(image_right.win);
			robot_input_display ();
			break;
		// Set first point on right and left images
		case 't':
			p_dblLeftPoint[0] = (double) image_left.wxd;
			p_dblLeftPoint[1] = (double) image_left.wyd;

			p_dblRightPoint[0] = (double) image_right.wxd;
			p_dblRightPoint[1] = (double) image_right.wyd;
			StereoTriangulation (1, p_dblLeftWorldPoint, &(p_dblRightWorldPoints[0]), p_dblLeftPoint, p_dblRightPoint);
			break;
		// Set second point and measure distance between first and second point
		case 'y':
			p_dblLeftPoint[0] = (double) image_left.wxd;
			p_dblLeftPoint[1] = (double) image_left.wyd;

			p_dblRightPoint[0] = (double) image_right.wxd;
			p_dblRightPoint[1] = (double) image_right.wyd;
			StereoTriangulation (1, p_dblLeftWorldPoint, &(p_dblRightWorldPoints[3]), p_dblLeftPoint, p_dblRightPoint);
					
			printf ("%lf %lf %lf\n", p_dblRightWorldPoints[0], 
						 p_dblRightWorldPoints[1], 
						 p_dblRightWorldPoints[2]);
			
			printf ("Distance = %3f\n", sqrt ((p_dblRightWorldPoints[0] - p_dblRightWorldPoints[3]) * (p_dblRightWorldPoints[0] - p_dblRightWorldPoints[3]) + 
							  (p_dblRightWorldPoints[1] - p_dblRightWorldPoints[4]) * (p_dblRightWorldPoints[1] - p_dblRightWorldPoints[4]) + 
							  (p_dblRightWorldPoints[2] - p_dblRightWorldPoints[5]) * (p_dblRightWorldPoints[2] - p_dblRightWorldPoints[5]))); 
			break;
		// Exits application
		case 'Q':
		case 'q':
			StereoTerminate ();
			exit (0);
	}
}


void 
update_map ()
{	
	printf("Updating outputs\n");
	all_outputs_update();
	printf("Saindo do update map\n");
}

