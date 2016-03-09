#include "calibration_user_functions.h"

#define INVERT_FRAME

// Global variables
int g_nInputType;

// Calibration variables
int g_nCameraSide, g_nCameraWindowSide;
int g_nCalibrationPointsCounter, g_nCalibrationPointsNumber;
BOOLEAN g_bCalibrating = FALSE;
double g_dblCalibrationPointX, g_dblCalibrationPointY;
double *g_p_dblImagePoints = NULL, *g_p_dblWorldPoints = NULL;

float g_fltZoom = 1.0;
int g_nCenterX, g_nCenterY;

int g_nTestingEpipolarGeometry = 0;
double *g_p_dblLeftEpipolarLine = NULL;
double g_p_dblLeftEpipole[2], g_p_dblRightEpipole[2];
double g_p_dblLeftPoint[2], g_p_dblRightPoint[2];
double *g_p_dblWorldPoint = NULL;
double g_dblNearVergenceCutPlane = NEAREST_DISTANCE;
double g_dblFarVergenceCutPlane = FARTEST_DISTANCE;
double g_p_dblWorldLine[6];
int g_nLineVertexCounter = 0;

/************************************************************************/
/* Name: init_user_functions						*/
/* Description:	initializes the user functions				*/
/* Inputs: none								*/
/* Output: returns zero if OK						*/
/************************************************************************/

int init_user_functions (void)
{
	char strCommand[128];
    
    	sprintf (strCommand, "toggle move_active;");
    	interpreter (strCommand);

    	sprintf (strCommand, "toggle draw_active;");
    	interpreter (strCommand);

	// Initializes the stereo system module
	StereoInitialize ();
	StereoSetCameraParameters (LEFT_CAMERA, IMG_WIDTH, IMG_HEIGHT, CCD_FORMAT);
	StereoSetCameraParameters (RIGHT_CAMERA, IMG_WIDTH, IMG_HEIGHT, CCD_FORMAT);
	
	// Loads the cameras parameters
	StereoLoadCameraConstants (LEFT_CAMERA, CAMERA_LEFT_CONSTANTS_FILE_NAME);
	StereoLoadCameraConstants (RIGHT_CAMERA, CAMERA_RIGHT_CONSTANTS_FILE_NAME);
	StereoBuildEpipolarGeometry ();

	return (0);
}



/************************************************************************/
/* Name: init								*/
/* Description:	loads an input image					*/
/* Inputs: a pointer to the input					*/
/* Output: none								*/
/************************************************************************/

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



/************************************************************************/
/* Name: sum_neurons_output 						*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

NEURON_OUTPUT sum_neurons_output (NEURON *n, OUTPUT_TYPE output_type, int begin, int end)
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



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void make_input_image_smv (INPUT_DESC *input)
{
	char message[256];
	
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
	input->vpyo = input->ww - input->vph;
	
	if(input->image == (GLubyte *) NULL)
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void check_input_bounds_smv (INPUT_DESC *input, int wx, int wy)
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
		//update_input_image (input);
		glutSetWindow (input->win);
		glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, 
			input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
		input->waiting_redisplay = 1;
	}
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void input_mouse_smv (int button, int state, int x, int y)
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
				input->x_old = input->x;
				input->y_old = input->y;
				input->x = x;
				input->y = y;
				input->mouse_state = GLUT_DOWN;
				input->wxd_old = input->wxd;
				input->wyd_old = input->wyd;
				input->wxd = (GLint) (wx + 0.5);
				input->wyd = (GLint) (wy + 0.5);
#ifdef INVERT_FRAME
				g_dblCalibrationPointX = (double) input->ww - wx - 1.0;
#else
				g_dblCalibrationPointX = wx;
#endif
				g_dblCalibrationPointY = wy;
				
				if (((input->wxd >= input->wx) && (input->wxd <= (input->wx+input->ww))) && 
				    ((input->wyd >= input->wy) && (input->wyd <= (input->wy+input->wh))))
					input->valid = 1;

				update_input_status (input);
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
				input->valid = 0;
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
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			input->mouse_button = GLUT_MIDDLE_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->x = (GLint) (wx + 0.5);
				input->y = (GLint) (wy + 0.5);
				input->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
			}
			break;
		default:
			break;
	}
	if (input->input_controler != NULL)
		(*(input->input_controler)) (input, SET_POSITION);
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void input_display_smv (void)
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

	glColor3f (1.0, 1.0, 1.0);
	if (g_nTestingEpipolarGeometry && (strcmp (input->name, image_left.name) == 0))
		DRAWLINE((float) input->ww, -g_p_dblLeftEpipolarLine[2] / g_p_dblLeftEpipolarLine[1], .0f, (-g_p_dblLeftEpipolarLine[0] * (float) input->ww - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1]);

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

void input_generator (INPUT_DESC *input, int status)
{	
	if (((input->win == image_left.win) || (input->win == image_right.win)) && (input->win != 0) && (status == MOVE))
	{
		update_input_neurons (input);
		check_input_bounds_smv (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		input_display_smv ();
		all_filters_update ();
		all_outputs_update ();
  	}
	
	if (input->win == 0)
	{
		int x, y;

		g_nInputType = input->input_generator_params->next->param.ival;
		
		switch (g_nInputType)
		{
			case READ_FROM_IMAGE_FILE:
				make_input_image (input);
				break;
			case PASS_BY_PARAMETER:
				make_input_image_smv (input);
				break;
			case CAPTURED_FROM_CAMERA:
				break;
			case CAPTURED_FROM_IP_ADDRESS:
				break;
			default:
				make_input_image (input);
		}
		
		init (input);
		update_input_neurons (input);
		
		glutInitWindowSize (input->ww, input->wh);
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (input_display_smv); 
		glutKeyboardFunc (keyboard);
		glutPassiveMotionFunc (input_passive_motion);
		glutMouseFunc (input_mouse_smv);		
	}
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void draw_output (char *output_name, char *input_name)
{
    return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void input_controler (INPUT_DESC *input, int status)
{
	char command[256];	

	// Moves the right input
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_right.win) && !g_bCalibrating)
	{
		// Updates the left epipolar line
#ifdef INVERT_FRAME
		g_p_dblRightPoint[0] = (double) (input->ww - input->wxd - 1);
#else
		g_p_dblRightPoint[0] = (double) input->wxd;
#endif
		g_p_dblRightPoint[1] = (double) input->wyd;
		g_p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, g_p_dblRightPoint);

		image_left.wxd += image_right.wxd - image_right.wxd_old;
#ifdef INVERT_FRAME
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);
#else
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) image_left.wxd - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);
#endif
		sprintf (command, "move %s to %d, %d;", image_right.name, image_right.wxd, image_right.wyd);
		interpreter (command);

		sprintf (command, "move %s to %d, %d;", image_left.name, image_left.wxd, image_left.wyd);
		interpreter (command);
	}
	
	// Moves the left input
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_left.win) && !g_bCalibrating)
	{
		if (g_p_dblLeftEpipolarLine == NULL)
		{
			// Updates the left epipolar line
#ifdef INVERT_FRAME
			g_p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
#else
			g_p_dblRightPoint[0] = (double) image_right.wxd;
#endif
			g_p_dblRightPoint[1] = (double) image_right.wyd;
			g_p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, g_p_dblRightPoint);
		}
		
#ifdef INVERT_FRAME
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);
#else
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) image_left.wxd - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);
#endif
		sprintf (command, "move %s to %d, %d;", image_left.name, image_left.wxd, image_left.wyd);
		interpreter (command);
	}
	
	// Inserts an image calibration point
	if (g_bCalibrating && 
	    (input->win == g_nCameraWindowSide) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		g_p_dblImagePoints[2 * g_nCalibrationPointsCounter + 0] = g_dblCalibrationPointX;
		g_p_dblImagePoints[2 * g_nCalibrationPointsCounter + 1] = g_dblCalibrationPointY;
		printf ("%2d - (%.3lf, %.3lf)\n", g_nCalibrationPointsCounter, g_p_dblImagePoints[2 * g_nCalibrationPointsCounter + 0], g_p_dblImagePoints[2 * g_nCalibrationPointsCounter + 1]);

		g_nCalibrationPointsCounter++;
	}
	
	// Deletes an image calibration point
	if (g_bCalibrating && 
	    (input->win == g_nCameraWindowSide) &&
	    (input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{	
		if (g_nCalibrationPointsCounter > 0)
			g_nCalibrationPointsCounter--;		
		printf ("%2d\n", g_nCalibrationPointsCounter);
	}
	
	// Updates the current center point
	if ((input->mouse_button == GLUT_MIDDLE_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{	
		g_nCenterX = input->x;
		g_nCenterY = input->y;
		input_display_smv ();
	}
	
	// Selects a point 
	if (!g_bCalibrating &&
	    (input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) && (input->win == image_right.win))
	{	
		SetVergenceByMinusFilter ();

#ifdef INVERT_FRAME
		g_p_dblLeftPoint[0] = (double) (image_left.ww - image_left.wxd - 1);
#else
		g_p_dblLeftPoint[0] = (double) image_left.wxd;
#endif
		g_p_dblLeftPoint[1] = (double) image_left.wyd;

#ifdef INVERT_FRAME
		g_p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
#else
		g_p_dblRightPoint[0] = (double) image_right.wxd;
#endif
		g_p_dblRightPoint[1] = (double) image_right.wyd;

		g_p_dblWorldPoint = StereoImage2WorldPoint (g_p_dblRightPoint, g_p_dblLeftPoint);
		
		g_p_dblWorldLine[3 * g_nLineVertexCounter + 0] = g_p_dblWorldPoint[0];
		g_p_dblWorldLine[3 * g_nLineVertexCounter + 1] = g_p_dblWorldPoint[1];
		g_p_dblWorldLine[3 * g_nLineVertexCounter + 2] = g_p_dblWorldPoint[2];
		g_nLineVertexCounter = (g_nLineVertexCounter + 1) % 2;

		if (!g_nLineVertexCounter)
			printf ("Distance = %.3f\n", sqrt ((g_p_dblWorldLine[0] - g_p_dblWorldLine[3]) * (g_p_dblWorldLine[0] - g_p_dblWorldLine[3]) +
							   (g_p_dblWorldLine[1] - g_p_dblWorldLine[4]) * (g_p_dblWorldLine[1] - g_p_dblWorldLine[4]) +
							   (g_p_dblWorldLine[2] - g_p_dblWorldLine[5]) * (g_p_dblWorldLine[2] - g_p_dblWorldLine[5])));
	}

	input->mouse_button = -1;
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void SetVergenceByMinusFilter (void)
{
	int x_min, x_max, x_min_minus, num_neurons;
	NEURON_OUTPUT minus, min_minus;
	double p_dblRightPoint[2], *p_dblLeftPoint = NULL, *p_dblWorldPoint = NULL, *p_dblLeftEpipolarLine = NULL;

	num_neurons = right_minus_left.dimentions.x * right_minus_left.dimentions.y;

	// Gets the right point
#ifdef INVERT_FRAME
	p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
#else
	p_dblRightPoint[0] = (double) image_right.wxd;
#endif
	p_dblRightPoint[1] = (double) image_right.wyd;

	// Gets the fartest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, g_p_dblRightPoint, g_dblFarVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
#ifdef INVERT_FRAME
	x_min = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
#else
	x_min = (int) (p_dblLeftPoint[0] + 0.5);
#endif
	x_min = (x_min < 0) ? 0 : x_min;

	// Gets the nearest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, g_p_dblRightPoint, g_dblNearVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
#ifdef INVERT_FRAME
	x_max = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
#else
	x_max = (int) (p_dblLeftPoint[0] + 0.5);
#endif
	x_max = (x_max > image_left.ww) ? image_left.ww : x_max;
	
	// Gets the left epipolar line
	p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);

	for (image_left.wxd = x_min_minus = x_min, min_minus.fval = FLT_MAX; image_left.wxd < x_max; image_left.wxd++)
	{
#ifdef INVERT_FRAME
		image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
#else	
		image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) image_left.wxd - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
#endif
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		minus = sum_neurons_output (right_minus_left.neuron_vector, right_minus_left.output_type, 0, num_neurons);

		if (min_minus.fval > minus.fval)
		{
			min_minus.fval = minus.fval;
			x_min_minus = image_left.wxd;
		}
	}
    	
	image_left.wxd_old = image_left.wxd;
	image_left.wxd = x_min_minus;
#ifdef INVERT_FRAME
	image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
#else
	image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) image_left.wxd - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
#endif
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	
	return;
}



/************************************************************************/
/* Name: calculateVergencePoint						*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

XY_PAIR calculateVergencePoint (XY_PAIR g_pInputRightPoint)
{
	XY_PAIR g_pInputLeftPoint;	
	
	// Moves the input right
	move_input_window (image_right.name, g_pInputRightPoint.x, g_pInputRightPoint.y);
	
	SetVergenceByMinusFilter ();
	
	// Saves the current input left position
	g_pInputLeftPoint.x = image_left.wxd;
	g_pInputLeftPoint.y = image_left.wyd;
	
	return (g_pInputLeftPoint);
}



/************************************************************************/
/* Name: LoadTargetPoints						*/
/* Description:	loads the calibration target points			*/
/* Inputs: a pointer to the file and the calibration data struct	*/
/* Output: 0 if OK, -1 otherwise					*/
/************************************************************************/

int LoadTargetPoints (char *strFileName)
{
	FILE *data_fd = NULL;

	if ((data_fd = fopen (strFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s'.\n", strFileName);
		return (-1);
	}
	
	fscanf (data_fd, "%d", &g_nCalibrationPointsNumber);
    
    	g_p_dblImagePoints = (double *) malloc (2 * g_nCalibrationPointsNumber * sizeof (double));	
	g_p_dblWorldPoints = (double *) malloc (3 * g_nCalibrationPointsNumber * sizeof (double));
	
    	for (g_nCalibrationPointsCounter = 0; g_nCalibrationPointsCounter < g_nCalibrationPointsNumber; g_nCalibrationPointsCounter++)
    	{
    		fscanf (data_fd, "%lf %lf %lf",
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 0]),
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 1]),
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 2]));
    	}

	if (fclose (data_fd))
	{
		printf ("Error: cannot close file '%s'.\n", strFileName);
		return (-1);
	}
	
	return (0);
}


/************************************************************************/
/* Name: LoadTargetPoints						*/
/* Description:	loads the calibration target points			*/
/* Inputs: a pointer to the file and the calibration data struct	*/
/* Output: 0 if OK, -1 otherwise					*/
/************************************************************************/

/*int LoadCalibrationPoints (char *strFileName)
{
	FILE *data_fd = NULL;

	if ((data_fd = fopen (strFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s'.\n", strFileName);
		return (-1);
	}
	
	fscanf (data_fd, "%d", &g_nCalibrationPointsNumber);
    
    	g_p_dblImagePoints = (double *) malloc (2 * g_nCalibrationPointsNumber * sizeof (double));	
	g_p_dblWorldPoints = (double *) malloc (3 * g_nCalibrationPointsNumber * sizeof (double));
	
    	for (g_nCalibrationPointsCounter = 0; g_nCalibrationPointsCounter < g_nCalibrationPointsNumber; g_nCalibrationPointsCounter++)
    	{
    		fscanf (data_fd, "%d - (%lf, %lf, %lf)\n",
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 0]),
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 1]),
		   	&(g_p_dblWorldPoints[3 * g_nCalibrationPointsCounter + 2]));
    	}

	if (fclose (data_fd))
	{
		printf ("Error: cannot close file '%s'.\n", strFileName);
		return (-1);
	}
	
	return (0);
}*/

/************************************************************************/
/* Name: EndCalibrationProcess						*/
/* Description:	ends the calibration process and saves the results	*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void EndCalibrationProcess (void)
{
	if (g_nCalibrationPointsCounter >= g_nCalibrationPointsNumber)
	{	
		StereoCameraCalibration (g_nCameraSide, CALIBRATION_TYPE, g_p_dblImagePoints, g_p_dblWorldPoints, g_nCalibrationPointsNumber);
		g_bCalibrating = FALSE;
		
		free (g_p_dblImagePoints);
		free (g_p_dblWorldPoints);

		glutIdleFunc (NULL);
		
		g_nCenterX = image_right.vpw/2;
		g_nCenterY = image_right.vph/2; 
		
		printf ("End calibration process.\n");

		StereoShowCameraConstants (g_nCameraSide);
	}

	return;
}



/************************************************************************/
/* Name: StartCalibrationProcess					*/
/* Description:	starts the calibration process and waits for input 	*/
/*		points							*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void StartCalibrationProcess (void)
{
	g_nCameraWindowSide = glutGetWindow ();
			
	if (g_nCameraWindowSide == image_right.win)
	{
		g_nCameraSide = RIGHT_CAMERA;
		LoadTargetPoints (RIGHT_CALIBRATION_DATA_FILE_NAME);
		printf ("Right camera\n");
	}
	else if (g_nCameraWindowSide == image_left.win)
	{
		g_nCameraSide = LEFT_CAMERA;
		LoadTargetPoints (LEFT_CALIBRATION_DATA_FILE_NAME);
		printf ("Left camera\n");
	}
	else
	{
		printf ("Error: invalid window.\n");
		return;
	}

	g_bCalibrating = TRUE;
	g_nCalibrationPointsCounter = 0;
	
	glutIdleFunc (EndCalibrationProcess);
	
	printf ("Start calibration process.\n");

	return;
}



/************************************************************************/
/* Name: TestCalibration						*/
/* Description:	tests the calibration		 		 	*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void TestCalibration (void)
{
	int i;
	double *p_dblDistortedImagePoint = NULL, *p_dblUndistortedImagePoint = NULL;

	g_nCameraWindowSide = glutGetWindow ();
			
	if (g_nCameraWindowSide == image_right.win)
	{
		g_nCameraSide = RIGHT_CAMERA;
		LoadTargetPoints (RIGHT_CALIBRATION_DATA_FILE_NAME);
		printf ("Right camera\n");
	}
	else if (g_nCameraWindowSide == image_left.win)
	{
		g_nCameraSide = LEFT_CAMERA;
		LoadTargetPoints (LEFT_CALIBRATION_DATA_FILE_NAME);
		printf ("Left camera\n");
	}
	else
	{
		printf ("Error: invalid window.\n");
		return;
	}

	for (i = 0; i < g_nCalibrationPointsNumber; i++)
	{
		p_dblUndistortedImagePoint = StereoProjectWorldPoint (g_nCameraSide, &(g_p_dblWorldPoints[3*i]));
		p_dblDistortedImagePoint = StereoUndistorted2DistortedImageCoordinate (g_nCameraSide, p_dblUndistortedImagePoint);
		g_p_dblImagePoints[2 * i + 0] = p_dblDistortedImagePoint[0];
		g_p_dblImagePoints[2 * i + 1] = p_dblDistortedImagePoint[1];
		/*g_p_dblImagePoints[2 * i + 0] = p_dblUndistortedImagePoint[0];
		g_p_dblImagePoints[2 * i + 1] = p_dblUndistortedImagePoint[1];*/
	}

	StereoShowCameraConstants (g_nCameraSide);
	StereoCameraCalibration (g_nCameraSide, CALIBRATION_TYPE, g_p_dblImagePoints, g_p_dblWorldPoints, g_nCalibrationPointsNumber);
	StereoShowCameraConstants (g_nCameraSide);
	
	return;
}



//------------------------------------------------------------------------------
// Name: f_keyboard
// Description:	waits for keyboard events
// Inputs: the key pressed
// Output: none
//------------------------------------------------------------------------------

void f_keyboard (char *key_value)
{	
	static int nLock = 0;
	int i, j, w, h, nPixel;
	GLubyte cRed, cGreen, cBlue;
	int *p_nUndistortedImage = NULL;
	static int *p_nDistortedImage = NULL;
	double *aux = NULL;
	static double dblWorldPoints[6];
	static int nCurrentPoint = 0;
	
	switch (key_value[0]) 
	{
		//--------------------------------------------------------------
		// Stereo system calibration interface:
		// 1) Calibrates each camera separately
		// 2) Build the epipolar geometry
		// 3) Save the cameras parameters to file
		//--------------------------------------------------------------

		// Starts the camera calibration process (one camera at time)
		case 'C':
		case 'c':
			StartCalibrationProcess ();
			break;
		// Ends the camera calibration process (optional)
		case 'E': 
		case 'e':
			g_nCalibrationPointsCounter = g_nCalibrationPointsNumber;
			break;
		// Loads up the cameras parameters and calibration constants from file 
		case 'L': 
		case 'l':
    			StereoLoadCameraConstants (LEFT_CAMERA, CAMERA_LEFT_CONSTANTS_FILE_NAME);
    			StereoLoadCameraConstants (RIGHT_CAMERA, CAMERA_RIGHT_CONSTANTS_FILE_NAME);
			break;
		// Dumps the cameras parameters and calibration constants to file 
		case 'D':
		case 'd':
    			StereoSaveCameraConstants (LEFT_CAMERA, CAMERA_LEFT_CONSTANTS_FILE_NAME);
    			StereoSaveCameraConstants (RIGHT_CAMERA, CAMERA_RIGHT_CONSTANTS_FILE_NAME);
			break;
		// Calculates the epipolar geometry from cameras parameters
		case 'S': 
		case 's':
			StereoBuildEpipolarGeometry ();
			break;
		// Toggles the epipolar geometry testing mode
		case 'T': 
		case 't':
			g_nTestingEpipolarGeometry = !g_nTestingEpipolarGeometry;
			glutPostWindowRedisplay (image_left.win);
			break;
		// Test the calibration
		case 'B': 
		case 'b':
			TestCalibration ();		
			break;

		//--------------------------------------------------------------
		// Stereo system interface
		//--------------------------------------------------------------

		// Starts the vergence process
		case 'V':
		case 'v':	
			SetVergenceByMinusFilter ();
			break;
		// Maps the image left and right current points to world point
		case 'M':
		case 'm':
#ifdef INVERT_FRAME
			g_p_dblLeftPoint[0] = (double) (image_left.ww - image_left.wxd - 1);
#else
			g_p_dblLeftPoint[0] = (double) image_left.wxd;
#endif
			g_p_dblLeftPoint[1] = (double) image_left.wyd;

#ifdef INVERT_FRAME
			g_p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
#else
			g_p_dblRightPoint[0] = (double) image_right.wxd;
#endif
			g_p_dblRightPoint[1] = (double) image_right.wyd;

			aux = StereoImage2WorldPoint (g_p_dblRightPoint, g_p_dblLeftPoint);
			
			dblWorldPoints[3 * nCurrentPoint + 0] = aux[0];
			dblWorldPoints[3 * nCurrentPoint + 1] = aux[1];
			dblWorldPoints[3 * nCurrentPoint + 2] = aux[2];
			
			nCurrentPoint = (nCurrentPoint + 1) % 2;
			
			printf ("%lf %lf %lf\n", aux[0], aux[1], aux[2]);
			
			if (!nCurrentPoint)
				printf ("Distance = %3f\n", sqrt ((dblWorldPoints[0] - dblWorldPoints[3]) * (dblWorldPoints[0] - dblWorldPoints[3]) + 
								  (dblWorldPoints[1] - dblWorldPoints[4]) * (dblWorldPoints[1] - dblWorldPoints[4]) + 
								  (dblWorldPoints[2] - dblWorldPoints[5]) * (dblWorldPoints[2] - dblWorldPoints[5]))); 
			break;
		// Updates the input images from network cameras
		case 'U':
		case 'u':
			
    			system (CAPTURE_SCRIPT);
			
			// Updates the left input image
			load_input_image (&(image_left), NULL);
			check_input_bounds_smv (&(image_left), image_left.wxd, image_left.wyd);
			input_display_smv ();
			
			// Updates the right input image
			load_input_image (&(image_right), NULL);
			check_input_bounds_smv (&(image_right), image_right.wxd, image_right.wyd);
			input_display_smv ();
			
			nLock = 0;
			break;
		// Updates the left input image from network cameras
		case 'K':
		case 'k':
			
    			system ("capture_left_image.bat");
			
			// Updates the left input image
			load_input_image (&(image_left), NULL);
			check_input_bounds_smv (&(image_left), image_left.wxd, image_left.wyd);
			input_display_smv ();
			break;
		// Updates the right input image from network cameras
		case 'J':
		case 'j':
			
    			system ("capture_right_image.bat");
			
			// Updates the right input image
			load_input_image (&(image_right));
			check_input_bounds_smv (&(image_right), image_right.wxd, image_right.wyd);
			input_display_smv ();
			break;
		// Resets the scene
		case 'R':
		case 'r':
			g_nCenterX = image_right.vpw/2;
			g_nCenterY = image_right.vph/2;
			
			g_fltZoom = 1.0;
			
			move_input_window (image_right.name, image_right.wxd, image_right.wyd);

			move_input_window (image_left.name, image_left.wxd, image_left.wyd);
			break;
		// Undistortes the left and right images
		case 'A':
		case 'a':
			//if (nLock++)
			//	break;
	
			w = image_left.ww;
			h = image_left.wh;

			if (p_nDistortedImage == NULL)
				p_nDistortedImage = (int *) malloc (w * h * sizeof (int));

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					cRed   = image_left.image[3 * (i + j * image_left.tfw) + 0];
					cGreen = image_left.image[3 * (i + j * image_left.tfw) + 1];
					cBlue  = image_left.image[3 * (i + j * image_left.tfw) + 2];

					p_nDistortedImage[i + j * w] = PIXEL(cRed, cGreen, cBlue);				
				}	
			}

			p_nUndistortedImage = StereoDistorted2UndistortedImage (LEFT_CAMERA, p_nDistortedImage, w, h);
			
			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					nPixel = p_nUndistortedImage[i + j * w];

					image_left.image[3 * (i + j * image_left.tfw) + 0] = RED(nPixel);
					image_left.image[3 * (i + j * image_left.tfw) + 1] = GREEN(nPixel);
					image_left.image[3 * (i + j * image_left.tfw) + 2] = BLUE(nPixel);
				}
			}
			
			glutSetWindow(image_left.win);
			input_init (&(image_left));
			update_input_neurons (&(image_left));
			check_input_bounds_smv (&(image_left), image_left.wxd, image_left.wyd);
			input_display_smv ();

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					cRed   = image_right.image[3 * (i + j * image_right.tfw) + 0];
					cGreen = image_right.image[3 * (i + j * image_right.tfw) + 1];
					cBlue  = image_right.image[3 * (i + j * image_right.tfw) + 2];

					p_nDistortedImage[i + j * w] = PIXEL(cRed, cGreen, cBlue);				
				}	
			}

			p_nUndistortedImage = StereoDistorted2UndistortedImage (RIGHT_CAMERA, p_nDistortedImage, w, h);
			
			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					nPixel = p_nUndistortedImage[i + j * w];

					image_right.image[3 * (i + j * image_right.tfw) + 0] = RED(nPixel);
					image_right.image[3 * (i + j * image_right.tfw) + 1] = GREEN(nPixel);
					image_right.image[3 * (i + j * image_right.tfw) + 2] = BLUE(nPixel);
				}
			}

			glutSetWindow(image_right.win);
			input_init (&(image_right));
			update_input_neurons (&(image_right));
			check_input_bounds_smv (&(image_right), image_right.wxd, image_right.wyd);
			input_display_smv ();
			
			break;
		// Undistortes the left and right images
		case 'O':
		case 'o':
			//if (nLock++)
			//	break;
	
			w = image_left.ww;
			h = image_left.wh;

			if (p_nDistortedImage == NULL)
				p_nDistortedImage = (int *) malloc (w * h * sizeof (int));

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					cRed   = image_left.image[3 * (i + j * image_left.tfw) + 0];
					cGreen = image_left.image[3 * (i + j * image_left.tfw) + 1];
					cBlue  = image_left.image[3 * (i + j * image_left.tfw) + 2];

					p_nDistortedImage[i + j * w] = PIXEL(cRed, cGreen, cBlue);				
				}	
			}

			p_nUndistortedImage = StereoUndistorted2DistortedImage (LEFT_CAMERA, p_nDistortedImage, w, h);
			
			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					nPixel = p_nUndistortedImage[i + j * w];

					image_left.image[3 * (i + j * image_left.tfw) + 0] = RED(nPixel);
					image_left.image[3 * (i + j * image_left.tfw) + 1] = GREEN(nPixel);
					image_left.image[3 * (i + j * image_left.tfw) + 2] = BLUE(nPixel);
				}
			}
			
			glutSetWindow(image_left.win);
			input_init (&(image_left));
			update_input_neurons (&(image_left));
			check_input_bounds_smv (&(image_left), image_left.wxd, image_left.wyd);
			input_display_smv ();

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					cRed   = image_right.image[3 * (i + j * image_right.tfw) + 0];
					cGreen = image_right.image[3 * (i + j * image_right.tfw) + 1];
					cBlue  = image_right.image[3 * (i + j * image_right.tfw) + 2];

					p_nDistortedImage[i + j * w] = PIXEL(cRed, cGreen, cBlue);				
				}	
			}

			p_nUndistortedImage = StereoUndistorted2DistortedImage (RIGHT_CAMERA, p_nDistortedImage, w, h);
			
			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					nPixel = p_nUndistortedImage[i + j * w];

					image_right.image[3 * (i + j * image_right.tfw) + 0] = RED(nPixel);
					image_right.image[3 * (i + j * image_right.tfw) + 1] = GREEN(nPixel);
					image_right.image[3 * (i + j * image_right.tfw) + 2] = BLUE(nPixel);
				}
			}

			glutSetWindow(image_right.win);
			input_init (&(image_right));
			update_input_neurons (&(image_right));
			check_input_bounds_smv (&(image_right), image_right.wxd, image_right.wyd);
			input_display_smv ();
			
			break;
		// Zoom out
		case 'z':
			g_fltZoom *= 0.5;			
			glutSetWindow(image_left.win);
			input_display_smv ();
			glutSetWindow(image_right.win);
			input_display_smv ();
			break;
		// Zoom in
		case 'Z':
			g_fltZoom *= 2.0;
			glutSetWindow(image_left.win);
			input_display_smv ();
			glutSetWindow(image_right.win);
			input_display_smv ();
			break;
		// Exits application
		case 'Q':
		case 'q':
			StereoQuit ();
			exit (0);
	}
		
	return;
}
