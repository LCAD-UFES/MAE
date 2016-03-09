#include "smv_user_functions.h"


float g_fltZoom = 1.0;
int g_nCenterX, g_nCenterY;


/************************************************************************/
/* Name: init_user_functions						*/
/* Description:	initializes the user functions				*/
/* Inputs: none								*/
/* Output: returns zero if OK						*/
/************************************************************************/

int init_user_functions (void)
{
	char strCommand[128];
    
    	g_nCorrection = 0;
	g_fltVergenceWindowSize = VERGENCE_WINDOW_SIZE;
	g_bRunningCalibrationProcess = FALSE;
	
    	sprintf (strCommand, "toggle move_active;");
    	interpreter (strCommand);

    	sprintf (strCommand, "toggle draw_active;");
    	interpreter (strCommand);

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
	//g_nCenterX = input->wx + input->neuron_layer->dimentions.x / 2.0;
	//g_nCenterY = input->wy + input->neuron_layer->dimentions.y / 2.0;
	//g_nCenterX = input->ww / 2.0;
	//g_nCenterY = input->wh / 2.0;

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
	int h;
	char message[256];

	input->image_type = g_nImageType;
	
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



void 
input_mouse_smv (int button, int state, int x, int y) 
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
				g_dblCalibrationPointX = wx;
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



void 
input_display_smv (void)
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
	
	glRasterPos2f (0.0, h + h / 20.0);
	
	for (i = 0; i < strlen (info); i++)
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, info[i]);
		
	glutSwapBuffers ();
	input->waiting_redisplay = 0;
	
	return;
}



/*void 
input_display_smv (void)
{
	INPUT_DESC *input = NULL;
	GLdouble w, h, d;
	char info[256];
	int i;
	int p_nViewport[4];
	int nWidth, nHeight;
	static int nPreviusWidth = 0, nPreviusHeight = 0;
	static GLubyte *pScreenPixels = NULL;

	input = get_input_by_win (glutGetWindow ());
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
        glGetIntegerv (GL_VIEWPORT, p_nViewport);
        nWidth = p_nViewport[2];
        nHeight = p_nViewport[3];

        if ((nWidth != nPreviusWidth) || (nHeight != nPreviusHeight))
        {
                free (pScreenPixels);
                if ((pScreenPixels = (GLubyte *) malloc (3 * nWidth * nHeight * sizeof (GLubyte))) == NULL)
			return;
                nPreviusWidth = nWidth;
                nPreviusHeight = nHeight;
        }
	
        glClear (GL_COLOR_BUFFER_BIT);
        gluScaleImage (GL_RGB, input->tfw, input->tfh, GL_UNSIGNED_BYTE, input->image, nWidth, nHeight, GL_UNSIGNED_BYTE, pScreenPixels);
        glDrawBuffer (GL_BACK);
        glEnable(GL_DRAW_BUFFER);
        glDrawPixels(nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *) pScreenPixels);
        glDisable(GL_DRAW_BUFFER);	

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
}*/

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
		g_nImageType = input->input_generator_params->next->next->param.ival;
		
		switch (g_nInputType)
		{
			case READ_FROM_IMAGE_FILE	: make_input_image (input); break;
			case PASS_BY_PARAMETER       	: make_input_image_smv (input); break;
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
	    (input->win == image_right.win))
	{
		if (!g_bRunningCalibrationProcess)
		{
			image_left.wxd += image_right.wxd - image_right.wxd_old;
			image_left.wyd = image_right.wyd + g_nCorrection;
		}
		
		sprintf (command, "move %s to %d, %d;", image_right.name, image_right.wxd, image_right.wyd);
		interpreter (command);

		sprintf (command, "move %s to %d, %d;", image_left.name, image_left.wxd, image_left.wyd);
		interpreter (command);
	}
	
	// Moves the left input
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_left.win))
	{
		if (!g_bRunningCalibrationProcess)
			image_left.wyd = image_right.wyd + g_nCorrection;

		sprintf (command, "move %s to %d, %d;", image_left.name, image_left.wxd, image_left.wyd);
		interpreter (command);
	}
	
	// Inserts an image calibration point
	if (g_bRunningCalibrationProcess && 
	    (input->win == g_nCameraWindowSide) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		g_pPixelVectorX[g_nCalibrationPointsCounter] = g_dblCalibrationPointX;
		g_pPixelVectorY[g_nCalibrationPointsCounter] = g_dblCalibrationPointY;
		printf ("%2d - (%.3lf, %.3lf)\n", g_nCalibrationPointsCounter, g_pPixelVectorX[g_nCalibrationPointsCounter], g_pPixelVectorY[g_nCalibrationPointsCounter]);

		g_nCalibrationPointsCounter++;
	}
	
	// Deletes an image calibration point
	if (g_bRunningCalibrationProcess && 
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
	
	
	input->mouse_button = -1;
	
	return;
}



/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void setVergenceByMinusFilter ()
{
	int x, x_min, x_max, x_min_minus, num_neurons;
	NEURON_OUTPUT minus, min_minus;
	
	num_neurons = right_minus_left.dimentions.x * right_minus_left.dimentions.y;

	x_min = image_right.wxd;
	x_max = image_right.wxd + (int) ((float) image_right.ww * g_fltVergenceWindowSize + 0.5);
	x_max = (x_max > image_left.neuron_layer->dimentions.x) ? image_left.neuron_layer->dimentions.x : x_max;
	
	if (right_minus_left.output_type == GREYSCALE_FLOAT)
		min_minus.fval = FLT_MAX;
	else
		min_minus.ival = INT_MAX;
	
	for (x = x_min, image_left.wyd = image_right.wyd + g_nCorrection; x < x_max; x++)
	{
		image_left.wxd = x;
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
		
		minus = sum_neurons_output (right_minus_left.neuron_vector, right_minus_left.output_type, 0, num_neurons);

		if (right_minus_left.output_type == GREYSCALE_FLOAT)
		{
			if (min_minus.fval > minus.fval)
			{
				min_minus.fval = minus.fval;
				x_min_minus = x;
			}
		}
		else
		{
			if (min_minus.ival > minus.ival)
			{
				min_minus.ival = minus.ival;
				x_min_minus = x;
			}
		}
	}
    	
	image_left.wxd_old = image_left.wxd;
	image_left.wxd = x_min_minus;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	
	return;
}



/************************************************************************/
/* Name: calculateWorldPoint						*/
/* Description:	calculates the world point				*/
/* Inputs: none								*/
/* Output: the world point 						*/
/************************************************************************/

WORLD_POINT calculateWorldPoint (IMAGE_COORDINATE undistortedLeftPoint, IMAGE_COORDINATE undistortedRightPoint)
{
	IMAGE_COORDINATE leftPrincipalPoint;
	IMAGE_COORDINATE rightPrincipalPoint;
	double fltCameraLeftFocus;
	double fltCameraRightFocus;
	double fltCameraDistance;
	
	// Gets the camera left principal point
	leftPrincipalPoint.x = cameraLeftCalibrationParameters.Cx;
	leftPrincipalPoint.y = cameraLeftCalibrationParameters.Cy;
	
	// Gets the camera right principal point
	rightPrincipalPoint.x = cameraRightCalibrationParameters.Cx;
	rightPrincipalPoint.y = cameraRightCalibrationParameters.Cy;
	
	// Gets the camera left focal distance
	fltCameraLeftFocus = cameraLeftCalibrationConstants.f;
	
	// Gets the camera right focal distance
	fltCameraRightFocus = cameraRightCalibrationConstants.f;
	
	// Gets the displacement between camera left and camera right
	fltCameraDistance = CAMERA_DISTANCE;
	
	return mapCalculateWorldPoint (undistortedLeftPoint,
				      undistortedRightPoint,
				      leftPrincipalPoint,
				      rightPrincipalPoint,
				      fltCameraLeftFocus,
				      fltCameraRightFocus,
				      fltCameraDistance);
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
	
	// Moves the input right and left with the same displacement  
	move_input_window (image_right.name, g_pInputRightPoint.x, g_pInputRightPoint.y);
	image_left.wxd += image_right.wxd - image_right.wxd_old;
	image_left.wyd = image_right.wyd + g_nCorrection;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	
	setVergenceByMinusFilter ();
	
	// Saves the current input left position
	g_pInputLeftPoint.x = image_left.wxd;
	g_pInputLeftPoint.y = image_left.wyd;
	
	return (g_pInputLeftPoint);
}



/************************************************************************/
/* Name: loadTargetPoints						*/
/* Description:	loads the calibration target points			*/
/* Inputs: a pointer to the file and the calibration data struct	*/
/* Output: none								*/
/************************************************************************/

void loadTargetPoints (FILE *fp, struct calibration_data *cd)
{
    	fscanf (fp, "%d", &g_nCalibrationPointsNumber);
    
    	g_pPixelVectorX = (double *) malloc (g_nCalibrationPointsNumber * sizeof (double));
    	g_pPixelVectorY = (double *) malloc (g_nCalibrationPointsNumber * sizeof (double));
	
	g_pPointsVectorX = (double *) malloc (g_nCalibrationPointsNumber * sizeof (double));
	g_pPointsVectorY = (double *) malloc (g_nCalibrationPointsNumber * sizeof (double));
	g_pPointsVectorZ = (double *) malloc (g_nCalibrationPointsNumber * sizeof (double));
	
    	for (g_nCalibrationPointsCounter = 0; g_nCalibrationPointsCounter < g_nCalibrationPointsNumber; g_nCalibrationPointsCounter++)
    	{
    		fscanf (fp, "%lf %lf %lf",
		   	&(g_pPointsVectorX[g_nCalibrationPointsCounter]),
		   	&(g_pPointsVectorY[g_nCalibrationPointsCounter]),
		   	&(g_pPointsVectorZ[g_nCalibrationPointsCounter]));
    	}
	
	return;
}



/************************************************************************/
/* Name: endCalibrationProcess						*/
/* Description:	ends the calibration process and saves the results	*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void endCalibrationProcess (void)
{
	if (g_nCalibrationPointsCounter >= g_nCalibrationPointsNumber)
	{
		maeCameraCalibration (g_nCameraSide, NONCOPLANAR_WITH_FULL_OPTIMIZATION, g_pPixelVectorX, g_pPixelVectorY, g_pPointsVectorX, g_pPointsVectorY, g_pPointsVectorZ, g_nCalibrationPointsNumber);
		//maeCameraCalibration (g_nCameraSide, COPLANAR_WITH_FULL_OPTIMIZATION, g_pPixelVectorX, g_pPixelVectorY, g_pPointsVectorX, g_pPointsVectorY, g_pPointsVectorZ, g_nCalibrationPointsNumber);
	
		g_bRunningCalibrationProcess = FALSE;
		
		free (g_pPixelVectorX);
		free (g_pPixelVectorY);
		free (g_pPointsVectorX);
		free (g_pPointsVectorY);
		free (g_pPointsVectorZ);

		glutIdleFunc (NULL);
		
		g_nCenterX = image_right.vpw/2;
		g_nCenterY = image_right.vph/2; 
		
		printf ("End calibration process\n");
	}

	return;
}



/************************************************************************/
/* Name: startCalibrationProcess					*/
/* Description:	starts the calibration process and waits for input 	*/
/*		points							*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void startCalibrationProcess (void)
{
	FILE *data_fd;
	
	g_nCameraWindowSide = glutGetWindow ();
			
	if (g_nCameraWindowSide == image_right.win)
	{
		g_nCameraSide = RIGHT_CAMERA;
		printf ("Right camera\n");
	}
	else if (g_nCameraWindowSide == image_left.win)
	{
		g_nCameraSide = LEFT_CAMERA;
		printf ("Left camera\n");
	}
	else
	{
		printf ("Error: invalid window\n");
		return;
	}
	
	data_fd = fopen (TARGET_DATA_FILE_NAME, "r");
	loadTargetPoints (data_fd, &cd);
	fclose (data_fd);

	g_bRunningCalibrationProcess = TRUE;
	g_nCalibrationPointsCounter = 0;
	
	glutIdleFunc (endCalibrationProcess);
	
	printf ("Start calibration process\n");
	
	return;
}



/*
*********************************************************************************
* Function: Distorted2UndistortedImageMapping	  				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

/*void Distorted2UndistortedImageMapping (INPUT_DESC *input, int nCameraSide)
{
	int xi, yi, wi, hi, xo, yo, ho, wo, pixel, red, green, blue;
	double Xfd, Yfd, Xfu, Yfu;
	
	// Gets the Input Image Dimentions
	wi = input->neuron_layer->dimentions.x;
	hi = input->neuron_layer->dimentions.y;
	
	// Gets the Output Image Dimentions
	wo = input->ww;
	ho = input->wh;
		
	// Gets the Camera Parameters
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cc = cameraLeftCalibrationConstants;
			cp = cameraLeftCalibrationParameters;
			break;
		case RIGHT_CAMERA:
			cc = cameraRightCalibrationConstants;
			cp = cameraRightCalibrationParameters;
			break;
	}
	
	// Makes the dirt work
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			Xfu = (double) xo;
			Yfu = (double) yo;
			
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			
			xi = (int) (Xfd + 0.5);
			yi = (int) (Yfd + 0.5);
								
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				pixel = input->neuron_layer->neuron_vector[xi + yi * wi].output.ival;
			else
				pixel = 0;
				
			red   = RED(pixel);
			green = GREEN(pixel);
			blue  = BLUE(pixel);
			
			input->image[3 * (xo + yo * input->tfw) + 0] =  red ;
			input->image[3 * (xo + yo * input->tfw) + 1] =  green;
			input->image[3 * (xo + yo * input->tfw) + 2] =  blue ;
		}		
	}
	
	return;
}*/


void Distorted2UndistortedImageMapping (INPUT_DESC *input, int nCameraSide)
{
	int xi, yi, wi, hi, xo, yo, ho, wo, pixel, red, green, blue;
	double Xfd, Yfd, Xfu, Yfu;
	static GLubyte *pVirtualImage = NULL;
	static int wo_old = 0, ho_old = 0;

	// Gets the Input Image Dimentions
	wi = input->tfw;
	hi = input->tfh;
	
	// Gets the Output Image Dimentions
	wo = (SCALE_FACTOR) * input->neuron_layer->dimentions.x;
	ho = (SCALE_FACTOR) * input->neuron_layer->dimentions.y;
	
	// Resizes the virtual image
	if ((wo_old != wo) || (ho_old != ho))
	{
		free (pVirtualImage);
		pVirtualImage = (GLubyte *) alloc_mem (3 * wo * ho *sizeof (GLubyte));
		wo_old = wo;
		ho_old = ho;
	}
	
	// Gets the Camera Parameters
	switch (nCameraSide)
	{
		case LEFT_CAMERA:
			cc = cameraLeftCalibrationConstants;
			cp = cameraLeftCalibrationParameters;
			break;
		case RIGHT_CAMERA:
			cc = cameraRightCalibrationConstants;
			cp = cameraRightCalibrationParameters;
			break;
	}
	
	// Maps the distorted input image to the undistorted virtual image
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			// Calculates the associated position at the distorted input image
			Xfu = (double) xo;
			Yfu = (double) yo;
			undistorted_to_distorted_image_coord (Xfu, Yfu, &Xfd, &Yfd);
			xi = (int) (Xfd + 0.5) / (double) (SCALE_FACTOR);
			yi = (int) (Yfd + 0.5) / (double) (SCALE_FACTOR);
			
			if ((xi < 0) || (xi >= wi) || (yi < 0) || (yi >= hi))
				continue;
                	
			// Gets the input image pixel at the previus calculated position
			pVirtualImage[3 * (xo + yo * wo) + 0] =  input->image[3 * (xi + yi * input->tfw) + 0];
			pVirtualImage[3 * (xo + yo * wo) + 1] =  input->image[3 * (xi + yi * input->tfw) + 1];
			pVirtualImage[3 * (xo + yo * wo) + 2] =  input->image[3 * (xi + yi * input->tfw) + 2];
		}
	}
	
	// Scales the undistorted virtual image to the undistorted output image
	gluScaleImage (GL_RGB, wo, ho, GL_UNSIGNED_BYTE, (GLubyte *) pVirtualImage, wi, hi, GL_UNSIGNED_BYTE, (GLubyte *) input->image);
	
	return;
}

extern float g_fltZoom;

/************************************************************************/
/* Name: f_keyboard							*/
/* Description:	waits for keyboard events				*/
/* Inputs: the key pressed						*/
/* Output: none								*/
/************************************************************************/

void f_keyboard (char *key_value)
{	
	IMAGE_COORDINATE leftPoint, rightPoint;
	WORLD_POINT worldPoint;
	static WORLD_POINT worldPointPair[2];
	FILE *data_fd;
	static int nLock = 0;
	static int index = 0;
	int i;
	
	switch (key_value[0]) 
	{
		case 'V':
		case 'v':	
			// Starts the vergence process
			setVergenceByMinusFilter ();
			break;
		case 'C':
			data_fd = fopen ("CalibrationData.dat", "r");
			load_cd_data (data_fd, &cd);
			fclose (data_fd);
			
			for (i = 0; i < cd.point_count; i++)
			{
				cd.xw[i] += 750.0;
		   		cd.yw[i] += 300.0;
				cd.zw[i] += 1000.0;
			}
			
			maeInitializeIKWB11AParameters ();
			noncoplanar_calibration_with_full_optimization ();
			//coplanar_calibration_with_full_optimization ();
			break;
		case 'c':
			// Starts the calibration process			
			startCalibrationProcess ();
			break;
		case 'E': 
		case 'e':
			// Ends the calibration process
			g_nCalibrationPointsCounter = g_nCalibrationPointsNumber;
			break;
		case 'L': 
		case 'l':
    			// Loads up the cameras parameters and calibration constants from the data file 
    			data_fd = fopen (CAMERA_LEFT_PARAMETERS_FILE_NAME, "r");
    			load_cp_cc_data (data_fd, &cameraLeftCalibrationParameters, &cameraLeftCalibrationConstants);
    			fclose (data_fd);
			
			data_fd = fopen (CAMERA_RIGHT_PARAMETERS_FILE_NAME, "r");
    			load_cp_cc_data (data_fd, &cameraRightCalibrationParameters, &cameraRightCalibrationConstants);
    			fclose (data_fd);
			
			g_nCorrection = (int) (cameraLeftCalibrationParameters.Cy - cameraRightCalibrationParameters.Cy + 0.5);
			break;
		case 'D':
		case 'd':
    			// Dumps the cameras parameters and calibration constants to the data file 
    			data_fd = fopen (CAMERA_LEFT_PARAMETERS_FILE_NAME, "w");
    			dump_cp_cc_data (data_fd, &cameraLeftCalibrationParameters, &cameraLeftCalibrationConstants);
    			fclose (data_fd);
			
			data_fd = fopen (CAMERA_RIGHT_PARAMETERS_FILE_NAME, "w");
    			dump_cp_cc_data (data_fd, &cameraRightCalibrationParameters, &cameraRightCalibrationConstants);
    			fclose (data_fd);
			break;
		case 'M':
		case 'm':
			// Maps the image left and right current points to world point
			leftPoint.x = (double) (image_left.wxd);
			leftPoint.y = (double) (image_left.wyd);
			
			rightPoint.x = (double) (image_right.wxd);
			rightPoint.y = (double) (image_right.wyd);

			worldPointPair[index%2] = worldPoint = calculateWorldPoint (leftPoint, rightPoint);
			index++;
			printf ("%lf %lf %lf\n", worldPoint.x, worldPoint.y, worldPoint.z);   
			break;
		case 'Y':
			image_right.wyd++;
			break;
		case 'y':
			image_right.wyd--;
			break;
		case 'X':
			image_right.wxd++;
			break;
		case 'x':
			image_right.wxd--;
			break;
		case 'S':
		case 's':
			// Prints the calibration report
    			print_cp_cc_data (stderr, &cp, &cc);
			print_error_stats (stderr);
    			dump_cp_cc_data (stdout, &cp, &cc);  
			break;
		case 'U':
		case 'u':
			// Gets the input images
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
		case 'P':
		case 'p':
			// Gets the camera left principal point
			image_right.wxd = (int) cameraRightCalibrationParameters.Cx;
			image_right.wyd = (int) cameraRightCalibrationParameters.Cy;
	
			// Gets the camera right principal point
			image_left.wxd = (int) cameraLeftCalibrationParameters.Cx;
			image_left.wyd = (int) cameraLeftCalibrationParameters.Cy;
			
			move_input_window (image_right.name, image_right.wxd, image_right.wyd);

			move_input_window (image_left.name, image_left.wxd, image_left.wyd);
			break;
		case 'R':
		case 'r':
			// Resets the scene
			g_nCenterX = image_right.vpw/2;
			g_nCenterY = image_right.vph/2;
			
			g_fltZoom = 1.0;
			
			move_input_window (image_right.name, image_right.wxd, image_right.wyd);

			move_input_window (image_left.name, image_left.wxd, image_left.wyd);
			break;
		case 'A':
		case 'a':
			if (nLock)
				break;
			// Undistortes the left input image
			glutSetWindow(image_left.win);
			Distorted2UndistortedImageMapping (&(image_left), LEFT_CAMERA);
			input_init (&(image_left));
			update_input_neurons (&(image_left));
			check_input_bounds_smv (&(image_left), image_left.wxd, image_left.wyd);
			input_display_smv ();
			
			// Undistortes the right input image
			glutSetWindow(image_right.win);
			Distorted2UndistortedImageMapping (&(image_right), RIGHT_CAMERA);
			input_init (&(image_right));
			update_input_neurons (&(image_right));
			check_input_bounds_smv (&(image_right), image_right.wxd, image_right.wyd);
			input_display_smv ();
			
			nLock = 1;
			break;
		case 'J':
		case 'j':
			// Calculates the Euclidean distance
			printf ("%lf %lf %lf\n", worldPointPair[0].x, worldPointPair[0].y, worldPointPair[0].z);   
			printf ("%lf %lf %lf\n", worldPointPair[1].x, worldPointPair[1].y, worldPointPair[1].z);   

			printf ("Distance = %f\n", sqrt ((worldPointPair[0].x - worldPointPair[1].x) * (worldPointPair[0].x - worldPointPair[1].x) +
							 (worldPointPair[0].y - worldPointPair[1].y) * (worldPointPair[0].y - worldPointPair[1].y) +
							 (worldPointPair[0].z - worldPointPair[1].z) * (worldPointPair[0].z - worldPointPair[1].z)));
			break;
		case 'z':
			g_fltZoom *= 0.5;			
			glutSetWindow(image_left.win);
			input_display_smv ();
			glutSetWindow(image_right.win);
			input_display_smv ();
			break;
		case 'Z':
			g_fltZoom *= 2.0;
			glutSetWindow(image_left.win);
			input_display_smv ();
			glutSetWindow(image_right.win);
			input_display_smv ();
			break;
		case 'Q':
		case 'q':
			exit (0);
	}
		
	return;
}
