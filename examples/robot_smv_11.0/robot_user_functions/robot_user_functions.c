#include "robot_io.h"
#include "robot_user_functions.h"
#include "viewer.hpp"
#include "circle_packing.h"
#include "cylinder.h"
#include "estimate_volume.h"
#include "subpixel_disparity.h"
#include "gabor_guess_cylinder.h"
#include "stereo_volume.h"
#include "model_3d.hpp"
#include "model_3d_io.hpp"
#include "cconfig.h"

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

MODEL_3D model_3d;

void setup_data ()
{
	static int nLock = 0;

	if (nLock++) return;

	rectify_image ();

	all_outputs_update ();

	printf ("Ready.\n");
}

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

//	rectify_images ();
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

	if (input->input_generator_params->next == NULL)
	{
		return;
	}

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

	setup_data ();

	model_3d_init(&model_3d);

	return (0);
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

#define ZOOM(value, fromScale, toScale) (int) ((float) (value * toScale) / (float) fromScale + 0.5)

void
ReconstructByDisparityMapPlan ()
{
	// Dimensoes do cortex (disparity map)
	int ho = nl_disparity_map.dimentions.y;
	int wo = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	int hi = image_right.neuron_layer->dimentions.y;
	int wi = image_right.neuron_layer->dimentions.x;

	int size = (g_pWorkingArea[2] - g_pWorkingArea[0]) * (g_pWorkingArea[3] - g_pWorkingArea[1]);
	double *leftPoints  = (double*) alloc_mem (2 * size * sizeof (double));
	double *rightPoints = (double*) alloc_mem (2 * size * sizeof (double));
	int    *colors      =    (int*) alloc_mem (    size * sizeof (int));

	int xi = 0, yi = 0, num_points = 0;
	for (xi = g_pWorkingArea[0]; xi < g_pWorkingArea[2]; xi++)
	{
		int xo = ZOOM(xi, wi, wo);
		for (yi = g_pWorkingArea[1]; yi < g_pWorkingArea[3]; yi++)
		{
			int yo = ZOOM(yi, hi, ho);
			if ((0 <= xo && xo < wo) && (0 <= yo && yo < ho))
			{
				float disparity = nl_disparity_map.neuron_vector[wo * yo + xo].output.fval;

				leftPoints [num_points + 0] = (double) xi + disparity;
				leftPoints [num_points + 1] = (double) yi;
				rightPoints[num_points + 0] = (double) xi;
				rightPoints[num_points + 1] = (double) yi;

				colors[num_points / 2] = image_right.neuron_layer->neuron_vector[yi * wi + xi].output.ival;

				num_points += 2;
			}
		}
	}

	model_3d_fill_face(&model_3d, get_current_camera_pair(), num_points / 2, leftPoints, rightPoints, colors);
	model_3d_view(&model_3d);

	free(leftPoints);
	free(rightPoints);
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

void compute_model_3d_face(int face)
{
	CYLINDER_LIST *cylinders;

	load_camera_pair("fomento12", face, "ppm");

	cylinders = fit_cylinder_list(g_pWorkingArea, &nl_trunk_segmentation_map, &nl_disparity_map, &nl_simple_mono_right);

	model_3d_set_cylinders(&model_3d, face, cylinders);

	Reconstruct();
}

void compute_model_3d(int faces)
{
	CYLINDER_LIST *list = NULL;

	if (!(g_pWorkingArea[0] == 0 && g_pWorkingArea[1] == 0 &&
	    g_pWorkingArea[2] == 0 && g_pWorkingArea[3] == 0))
	{
		config_set_int("workspace.x0", g_pWorkingArea[0]);
		config_set_int("workspace.y0", g_pWorkingArea[1]);
		config_set_int("workspace.xn", g_pWorkingArea[2]);
		config_set_int("workspace.yn", g_pWorkingArea[3]);
		config_save();
	}
	else if (config_load())
	{
		g_pWorkingArea[0] = config_get_int("workspace.x0");
		g_pWorkingArea[1] = config_get_int("workspace.y0");
		g_pWorkingArea[2] = config_get_int("workspace.xn");
		g_pWorkingArea[3] = config_get_int("workspace.yn");
	}
	else
	{
		fprintf(stderr, "Please select working area.\n");
		return;
	}
	
	float dummy;
	FILE *coordinates = fopen("coordinates.dat", "r");
	fscanf(coordinates, "%f %f %f\n", &dummy, &dummy, &dummy);
	fscanf(coordinates, "%f %f %f\n", &dummy, &dummy, &dummy);
	model_3d_load(&model_3d, coordinates);
	fclose(coordinates);

	int i = 0;
	for (i = 0; i < faces; i++)
		compute_model_3d_face(i);

	list = model_3d_all_cylinders(&model_3d);
	circle_packing_set_cylinder_list(list);
		
	printf("Volume estereo: %f m3\n", model_3d_stereo_volume(&model_3d));
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
	time_t start, end;

	static double p_dblRightWorldPoints[6];
	static int nCurrentPoint = 0;
	double p_dblLeftWorldPoint[3], p_dblLeftPoint[2], p_dblRightPoint[2];
	char key = key_value[0];
	
	switch (key)
	{
/*		case 'c':
		{
			if (g_pWorkingArea[0] == 0 && g_pWorkingArea[1] == 0 &&
			    g_pWorkingArea[2] == 0 && g_pWorkingArea[3] == 0)
			{
				printf("Set working area!");
				break;
			}

			global_cylinder_list = fit_cylinder_list (g_pWorkingArea, &nl_trunk_segmentation_map,
								  &nl_disparity_map, &nl_simple_mono_right);

			cylinder_list_view  (global_cylinder_list, ARCS, CIRCLES);
			cylinder_list_print (global_cylinder_list);

			printf ("Volume estereo: %f m3\n", compute_stereo_volume (global_cylinder_list));

			glutIdleFunc ((void (* ) (void)) check_forms);
		}
		break; */
		case 'e':
			ViewerEraseMap ();
		break;
		case 'V':
			compute_model_3d(NUM_CAMERA_PAIRS);
		break;
		case 'v':
		{
			CYLINDER_LIST *list = model_3d_all_cylinders(&model_3d);
			
			if (list == NULL)
			{
				show_message ("cylinder list points to NULL.", "", "");
				return;
			}

			circle_packing_set_cylinder_list(list);
		}
		break;
		case 'R':
			ViewerEraseMap ();
			Reconstruct ();
		break;
		case 'r':
			Reconstruct ();
		break;
		case 'm':
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
		// Reads and processes image pair 0
		case '0':
			compute_model_3d_face(0);
		break;
		case '1':
			compute_model_3d_face(1);
		break;
		case '2':
			compute_model_3d_face(2);
		break;

		// Exits application
		case 'Q':
		case 'q':
			StereoTerminate ();
			exit (0);
		default:
			show_message ("Unknown key: ", key_value, "");
			break;
	}
}


void
update_map ()
{
	printf("Updating outputs\n");
	all_outputs_update();
	printf("Saindo do update map\n");
}

