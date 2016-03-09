#include "robot_user_functions.h"
#include "viewer.hpp"
#include "cylinder.h"
#include "estimate_volume.h"

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


// ----------------------------------------------------------------------------
// init -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;

	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;

	g_nCenterX = input->vpw / 2.0;
	g_nCenterY = input->vph / 2.0;

	return;
}


// ----------------------------------------------------------------------------
// confiability - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void distance_between_2_points (float x1, float y1, float x2, float y2)
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

void robot_input_mouse (int button, int state, int x, int y)
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
	
	return;
}

void draw_point(float x, float y){
	int i;
	glBegin(GL_POINTS);
	for (i = 0; i <= 5; i++){
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

void robot_input_display (void)
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

 
        /*glColor3f (1.0, 1.0, 1.0);
	if (g_nTestingEpipolarGeometry && (strcmp (input->name, image_left.name) == 0))
		DRAWLINE((float) input->ww - 1, -g_p_dblLeftEpipolarLine[2] / g_p_dblLeftEpipolarLine[1], .0f, (-g_p_dblLeftEpipolarLine[0] * (float) (input->ww - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1]);
*/
        
        
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
          if (g_pDistancePointsClicked[0]){
            draw_point(g_pDistancePoints[0], g_pDistancePoints[1]);
          }
          if (g_pDistancePointsClicked[1]){
            draw_point(g_pDistancePoints[2], g_pDistancePoints[3]);
          }
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
	
	return;
}


/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void robot_check_input_bounds (INPUT_DESC *input, int wx, int wy)
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
	
	return;
}



// ----------------------------------------------------------------------------
// robot_make_input_image - Inicializa a estrutura image
// da input
//
// Entrada: input - Descritor da input
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void robot_make_input_image (INPUT_DESC *input)
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
		
	return;
}



void robot_input_motion (int x, int y)
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

	return;
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

void InitWindow(INPUT_DESC *input)
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
	
	return;
}



// ----------------------------------------------------------------------------
// init_user_functions - initializes the user functions
//
// Inputs: none
//
// Output: returns zero if OK
// ----------------------------------------------------------------------------

int init_user_functions (void)
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
	ViewerInitialize ();
	
	return (0);
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void update_simple_mono_cells_filters ()
{
	FILTER_LIST *f_list;

	//Move to center of the image
	image_left.wxd = ((double)image_left.ww)/2.0;
	image_left.wyd = ((double)image_left.wh)/2.0;

	//Move to center of the image
	image_right.wxd = ((double)image_right.ww)/2.0;
	image_right.wyd = ((double)image_right.wh)/2.0;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next){
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_filter") == 0){
			filter_update (f_list->filter_descriptor);
		}
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_q_filter") == 0){
			filter_update (f_list->filter_descriptor);
		}
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_filter") == 0){
			filter_update (f_list->filter_descriptor);
		}
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_q_filter") == 0){
			filter_update (f_list->filter_descriptor);
		}
	}
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void update_mt_cells_filters ()
{
	FILTER_LIST *f_list;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next){
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_filter") == 0)
		{
			filter_update (f_list->filter_descriptor);
		}
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_gaussian_filter") == 0)
		{
			filter_update (f_list->filter_descriptor);
		}
	}
}

void generate_scan_range(int x, int y)
{
	int nInfinity;
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3];

	// Define the scaning range
	p_dblRightPoint[0] = (double) x;
	p_dblRightPoint[1] = (double) y;

	// Calculates the infinity distance position
	GetWorldPointAtDistance(p_dblWorldPoint, p_dblRightPoint, INFINITY_DISTANCE, RIGHT_CAMERA);
	CameraProjection(1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nInfinity = (int) (p_dblLeftPoint[0] + 0.5);
	g_pInfinityPoint[0] = nInfinity;
	g_pInfinityPoint[1] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5) - 1;

	// Gets the fartest left point
	GetWorldPointAtDistance(p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjection(1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	g_pRangeLine[0] = (int) (p_dblLeftPoint[0] + 0.5);
	g_pRangeLine[0] = (g_pRangeLine[0] < 0) ? 0 : g_pRangeLine[0];
	g_pRangeLine[1] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5) - 1;

	// Gets the nearest left point
	GetWorldPointAtDistance (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjection (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	g_pRangeLine[2] = (int) (p_dblLeftPoint[0] + 0.5);
	g_pRangeLine[2] = (g_pRangeLine[2] >= image_left.ww) ? image_left.ww - 1: g_pRangeLine[2];
	g_pRangeLine[3] = (int) image_left.wh - (p_dblLeftPoint[1] + 0.5) - 1;
}

// ----------------------------------------------------------------------------
// input_generator -
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------

void input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		InitWindow(input);
		update_input_neurons (input);
	}

	if (((input->win == image_left.win) || (input->win == image_right.win)) && (input->win != 0) && (status == MOVE))
	{
		// Draws the Infinity point and the line that marks the scan range.
		if (g_nShowRangeLine) generate_scan_range(image_right.wxd, image_right.wh - image_right.wyd - 1);

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
			}else if (!g_pDistancePointsClicked[1])
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
	return;
}



// ----------------------------------------------------------------------------
// draw_output -
//
// Inputs:
//
// Output: Nenhuma.
// ----------------------------------------------------------------------------

void draw_output (char *output_name, char *input_name)
{
        return;
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

void ReconstructByDisparityMapPlan()
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
			p_dblRightPoint[num_points+1] = (double) (image_right.wh - yi - 1);

			fltDisparity = nl_disparity_map.neuron_vector[w * yo + xo].output.fval;
	
			p_dblLeftPoint[num_points] = (double) xi + fltDisparity;
			p_dblLeftPoint[num_points+1] = (double) (image_left.wh - yi - 1);
			
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
			ViewerUpdateMap(p_dblRightWorldPoint[num_points+2], -p_dblRightWorldPoint[num_points+0], -p_dblRightWorldPoint[num_points+1], cRed, cGreen, cBlue);
			num_points += 3;
		}
	}
	free (p_dblLeftPoint);
	free (p_dblRightPoint);
	free (p_dblLeftWorldPoint);
	free (p_dblRightWorldPoint);
	
	return;
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

void ReconstructByDisparityMapPlan_old()
{
//int u, v;
	int w, h, wi, hi, xi, yi, yo, xo;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;
	double p_dblLeftPoint[2], p_dblRightPoint[2], p_dblLeftWorldPoint[3], p_dblRightWorldPoint[3];
	GLubyte cRed, cGreen, cBlue;
	
	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	// NAO SEI PORQUE.
	// PRECISA DISTO PARA FUNCIONAR DEVIDO A ALTERACAO FEITA POR DIJALMA
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = image_right.wxd - image_right.wx;
		y_center = image_right.wyd - image_right.wy;
		x_center_left = image_left.wxd - image_left.wx;
		y_center_left = image_left.wyd - image_left.wy;
	}
	else
	{
		x_center = wi/2;
		y_center = hi/2;
		x_center_left = wi/2;
		y_center_left = hi/2;
	}

	fltRangeCols = 1.0;
	nStartCol = w * ((1.0 - fltRangeCols) / 2.0);
	nEndCol = w - nStartCol;
/*
	for (v = 0; v < h; v++)
	{
		for (u = nStartCol; u < nEndCol; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h - 1), LOG_FACTOR);
*/
	for (xo = 0; xo < w; xo++)
	{
		xi = (int) ((float) (xo * wi) / (float) w + 0.5);

		for (yo = 0; yo < h; yo++)
		{
			yi = (int) ((float) (yo * hi) / (float) h + 0.5);
	
			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;
			if (outside_working_area (g_pWorkingArea, xi, yi))
			        continue;
			        
			pixel = image_right.neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			cRed = RED(pixel);
			cGreen = GREEN(pixel);
			cBlue = BLUE(pixel);

			p_dblRightPoint[0] = (double) xi;
			p_dblRightPoint[1] = (double) (image_right.wh - yi - 1);

			fltDisparity = nl_disparity_map.neuron_vector[w * yo + xo].output.fval;

			// Achar a coordenada relativa na imagem esquerda
//			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, (double) h / (double) (h - 1), LOG_FACTOR);
//			xi = (int) ((float) (xo * wi) / (float) w + 0.5);
//			yi = (int) ((float) (yo * hi) / (float) h + 0.5);
 
			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;
	
			p_dblLeftPoint[0] = (double) xi + fltDisparity;
			p_dblLeftPoint[1] = (double) (image_left.wh - yi - 1);

			StereoTriangulation (1, p_dblLeftWorldPoint, p_dblRightWorldPoint, p_dblLeftPoint, p_dblRightPoint);
			
			// Updates the map and rotates the frame coordinates			       			
			ViewerUpdateMap(p_dblRightWorldPoint[2], -p_dblRightWorldPoint[0], -p_dblRightWorldPoint[1], cRed, cGreen, cBlue);
		}
	}
	
	return;
}

// ----------------------------------------------------------------------------
// InitDispDataState - Inicializa o estado da estrutura DISP_DATA
//
// Entrada: nNumNeurons: Quantidade de neuronios da neuron_layer de disparidade
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void InitDispDataState (DISP_DATA* dispData, int nNumNeurons)
{
	int i, sample;

	for (i = 0; i < nNumNeurons; i++)
	{
		dispData->neg_slope[i] = 1;
	}

	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < nNumNeurons; i++)
		{
			dispData->samples[sample][i].val = FLT_MAX;
			dispData->samples[sample][i].conf = 0.0;
			dispData->samples[sample][i].pos = 0;
		}
	}
	
	return;
}



// ----------------------------------------------------------------------------
// AddLocalMin - 
//
// Inputs: 
//
// Output: none
// ----------------------------------------------------------------------------

void AddLocalMin(DISP_DATA* dispData, int i, double minus_out, double minus_conf, int pos)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
		if (minus_out <= dispData->samples[victim_sample][i].val)
			break;
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			dispData->samples[moving_sample][i] = dispData->samples[moving_sample - 1][i];
			moving_sample--;
		}
		dispData->samples[moving_sample][i].val = minus_out;
		dispData->samples[moving_sample][i].conf = minus_conf;
		dispData->samples[moving_sample][i].pos = pos;
	}
	
	return;
}


// ----------------------------------------------------------------------------
// distance_neig - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

int distance_neig (SAMPLES *sample, int pos, int x, int y, int w, int h)
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

void winner_takes_it_all(DISP_DATA* dispData, NEURON_LAYER *nlDisparityMap, int wxd)
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
        
        return;
}

// ----------------------------------------------------------------------------
// generate_disparity_map_plan - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void generate_disparity_map_plan (void)
{
	int nMaxScan, nMinScan, leftPoint;
	int i, x, y, nIndex;
	float fltMinSum, fltAux;
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3]; 

	int wo = nl_disparity_map.dimentions.x;
	int ho = nl_disparity_map.dimentions.y;

	float* dispAux = (float*) malloc(sizeof(float)*wo*ho);
	float* confAux = (float*) malloc(sizeof(float)*wo*ho);
	DISP_DATA* dispData = NULL;

	// Define the scaning range
	p_dblRightPoint[0] = ((double)image_right.ww)/2.0;
	p_dblRightPoint[1] = (double) (((double)image_right.wh)/2.0 - 1);

	// Gets the fartest left point
	GetWorldPointAtDistance (p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjection (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nMinScan = (int) (p_dblLeftPoint[0] + 0.5);
	nMinScan = (nMinScan < 0) ? 0 : nMinScan;

	// Gets the nearest left point
	GetWorldPointAtDistance (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjection (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	nMaxScan = (int) (p_dblLeftPoint[0] + 0.5);
	nMaxScan = (nMaxScan >= image_left.ww) ? image_left.ww - 1: nMaxScan;

	// Aloca memoria para a estrutura DISP_DATA
	dispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
	dispData->neg_slope = (char *) alloc_mem (wo * ho);
	dispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
	for (i = 0; i < NUM_SAMPLES; i++)
		dispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));

	// Inicializar a estrutura DISP_DATA
	InitDispDataState(dispData, wo * ho);

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < (wo * ho); i++)
	{
		dispAux[i] = FLT_MAX;
		confAux[i] = .0f;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = .0f;
		nl_confidence_map.neuron_vector[i].output.fval = .0f;
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	}

	fltMinSum = FLT_MAX;
	for (leftPoint = nMinScan; leftPoint <= nMaxScan; leftPoint++)
	{
		//Set the current disparity
		g_nCurrDisparity = leftPoint - p_dblRightPoint[0];

		//Update MT layer
		update_mt_cells_filters();

		for (y = 0; y < ho; y++) 
		{
			for (x = 0; x < wo; x++)
			{
				nIndex = y * wo + x;
				fltAux = nl_mt_gaussian.neuron_vector[nIndex].output.fval;

				if (fltAux >= dispAux[nIndex])
				{
					if (dispData->neg_slope[nIndex])
					{
						dispData->neg_slope[nIndex] = 0;
						AddLocalMin(dispData, nIndex, dispAux[nIndex], confAux[nIndex], g_nCurrDisparity);//image_left.wxd - 1);
						nl_mt_gaussian_map.neuron_vector[nIndex].output.fval = dispAux[nIndex];
						nl_confidence_map.neuron_vector[nIndex].output.fval = confAux[nIndex];
						nl_disparity_map.neuron_vector[nIndex].output.fval = g_nCurrDisparity;//image_left.wxd - 1;
					}
				}
				else
					dispData->neg_slope[nIndex] = 1;

				dispAux[nIndex] = fltAux;
			}
			
		}

	}

	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = dispData->samples[0][i].val;
		nl_confidence_map.neuron_vector[i].output.fval = dispData->samples[0][i].conf;
		nl_disparity_map.neuron_vector[i].output.fval = dispData->samples[0][i].pos;
	}

	// Averages the disparity map by applying the WINNER TAKES IT ALL algorithm. 
	for (i = 0; i < WINNER_TAKES_IT_ALL_STEPS; i++)
		winner_takes_it_all(dispData, &nl_disparity_map, image_left.wxd);

	free(dispAux);
	free(confAux);
	free(dispData);

  return;
}

// ----------------------------------------------------------------------------
// input_controler -
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------

void input_controler (INPUT_DESC *input, int status)
{
        if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_right.win))
	{
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
	
	return;
}



// ----------------------------------------------------------------------------
// output_handler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void output_handler(OUTPUT_DESC *output, int status)
{
	printf("output_handler\n");
	
	return;
}



// ----------------------------------------------------------------------------
// Reconstruct -
//
// Entrada:
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void Reconstruct (void)
{
	printf("Reconstruct\n");

	ReconstructByDisparityMapPlan();
	
	ViewerRedisplay ();

	glutIdleFunc ((void (* ) (void)) check_forms);
	
	return;
}



// ----------------------------------------------------------------------------
// LowPass - Passa uma gaussiana no mapa de disparidade
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void LowPass (void)
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
		{
			dispAux[y * wo + x] = apply_gaussian_kernel(receptiveField, &nl_disparity_map, x, y);
		}
	}

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			nl_disparity_map.neuron_vector[y * wo + x].output.fval = dispAux[y * wo + x];
		}
	}

	free(receptiveField);
	free(dispAux);
  
	return;
}

// ----------------------------------------------------------------------------
// confiability - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void confiability (double limit)
{    
	int i; 
	int maxIndex = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;
	for (i = 0; i < maxIndex; i++)
	{
		//    printf("val = %f, limit = %f\n", nl_mt_gaussian_map.neuron_vector[i].output.fval, limit);
		if (nl_mt_gaussian_map.neuron_vector[i].output.fval > limit){
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

void rectify_image ()
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

void f_keyboard (char *key_value)
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
			estimate_volume(g_pWorkingArea, &nl_disparity_map, image_right.neuron_layer, &nl_cylinder_map);
			
//			glutSetWindow(out_disparity_map.win);
//			update_output_image (&(out_disparity_map));
//			glutPostWindowRedisplay (out_disparity_map.win);
		break;
		}
		case 'C':
			// void cylinder (double *origin, double size, double diameter, double alpha, double betha, int arcs, int circles);
			// test_StereoTriangulation_CameraProjection ();
			// cylinder (origin, 400.0, 100.0, 0.0, 0.0, 50, 50);
			// break;
			cylinder (100.0, -75.0, 1000.0, 400.0, 100.0, 40.0, 30.0, 150, 150);
			exchange_to_disparity_map();
			
			glutSetWindow(out_disparity_map.win);
			update_output_image (&(out_disparity_map));
			glutPostWindowRedisplay (out_disparity_map.win);
		break;
		case 'c':
			confiability (0.3);
			glutSetWindow(out_disparity_map.win);
			update_output_image (&(out_disparity_map));
			glutPostWindowRedisplay (out_disparity_map.win);
		break;
		case 'R':
		case 'r':
			Reconstruct ();
		break;
		case 'E':
			exchange_to_disparity_map();
			update_output_image (&out_disparity_map);
			update_output_image (&out_cylinder_map);
			
		break;
		case 'e':
			ViewerEraseMap ();
		break;
		case 'F':
		case 'f':
			LowPass ();

			glutIdleFunc ((void (* ) (void)) check_forms);

			glutSetWindow(out_disparity_map.win);
			update_output_image (&(out_disparity_map));
			glutPostWindowRedisplay (out_disparity_map.win);
			break;
		case 'D':
		case 'd':

			if (g_pDistanceDraw){
				g_pDistanceDraw = 0;
				g_nDistanceClicking = 0;      
			}else{
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
		case 'm':	

			//Rectify Image   
			if (!nLock++){ 
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

			glutSetWindow(out_disparity_map.win);
			update_output_image (&(out_disparity_map));
			glutPostWindowRedisplay (out_disparity_map.win);

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
		// Exits application
		case 'Q':
		case 'q':
			StereoTerminate ();
			exit (0);
	}
		
	return;
}


void update_map ()
{	
	printf("Updating outputs\n");
	all_outputs_update();
	printf("Saindo do update map\n");
}

