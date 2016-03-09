#include "stereo_user_functions.h"
#include "viewer.hpp"

// Variaveis globais
float g_fltZoom = 1.0;
int g_nCenterX, g_nCenterY;
int g_nTestingEpipolarGeometry = 0;
double *g_p_dblLeftEpipolarLine = NULL;
double g_p_dblLeftEpipole[2], g_p_dblRightEpipole[2];
double g_p_dblLeftPoint[2], g_p_dblRightPoint[2];
double g_dblNearVergenceCutPlane = NEAREST_DISTANCE;
double g_dblFarVergenceCutPlane = FARTEST_DISTANCE;

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

/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void stereo_input_mouse (int button, int state, int x, int y)
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

void stereo_input_display (void)
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
		DRAWLINE((float) input->ww - 1, -g_p_dblLeftEpipolarLine[2] / g_p_dblLeftEpipolarLine[1], .0f, (-g_p_dblLeftEpipolarLine[0] * (float) (input->ww - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1]);

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

void stereo_check_input_bounds (INPUT_DESC *input, int wx, int wy)
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
// stereo_make_input_image - Inicializa a estrutura image
// da input
//
// Entrada: input - Descritor da input
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void stereo_make_input_image (INPUT_DESC *input)
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
			stereo_make_input_image (input);
			break;
		case GET_IMAGE_FROM_PNM:
			make_input_image (input);
			break;
		case GET_IMAGE_FROM_SMV:
			stereo_make_input_image (input);
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
	glutDisplayFunc (stereo_input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (stereo_input_mouse);
	
	return;
}




// ----------------------------------------------------------------------------
// init_user_functions -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

int init_user_functions()
{
	int i;
	double *p_dblCameraOrientation = NULL;

	interpreter ("toggle move_active;");
    	interpreter ("toggle draw_active;");
	
	// Emptys the disparity map
	for (i = 0; i < nl_disparity_map.num_neurons; i++)
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	
	// Initializes the stereo system module
	StereoInitialize ();
	StereoSetCameraParameters (LEFT_CAMERA, INPUT_WIDTH, INPUT_HEIGHT, CCD_FORMAT);
	StereoSetCameraParameters (RIGHT_CAMERA, INPUT_WIDTH, INPUT_HEIGHT, CCD_FORMAT);
	
	// Loads the cameras parameters
	StereoLoadCameraConstants (LEFT_CAMERA, CAMERA_LEFT_CONSTANTS_FILE_NAME);
	StereoLoadCameraConstants (RIGHT_CAMERA, CAMERA_RIGHT_CONSTANTS_FILE_NAME);
	StereoBuildEpipolarGeometry ();

	// Initializes the Viewer module	
	ViewerSetParameterf (VIEWER_FOVY, FOVY);
	ViewerSetParameterf (VIEWER_FAR, FAR);
	ViewerSetParameterf (VIEWER_NEAR, NEAR);
	ViewerSetParameterf (VIEWER_FOCUS, FOCUS);		
	ViewerSetParameteri (VIEWER_IMAGE_WIDTH, INPUT_WIDTH);
	ViewerSetParameteri (VIEWER_IMAGE_HEIGHT, INPUT_HEIGHT);
	p_dblCameraOrientation = StereoGetCameraOrientation (LEFT_CAMERA);
	ViewerSetParameterv (VIEWER_LEFT_CAMERA_ORIENTATION, p_dblCameraOrientation, 6);
	p_dblCameraOrientation = StereoGetCameraOrientation (RIGHT_CAMERA);
	ViewerSetParameterv (VIEWER_RIGHT_CAMERA_ORIENTATION, p_dblCameraOrientation, 6);
	ViewerInitialize ();

	return (0);
}



// ----------------------------------------------------------------------------
// input_generator -
//
// Entrada:
//
// Saida:
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
		update_input_neurons (input);
		stereo_check_input_bounds (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		stereo_input_display ();
		all_filters_update ();
		all_outputs_update ();
  	}

	return;
}



// ----------------------------------------------------------------------------
// draw_output -
//
// Entrada:
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void draw_output (char *output_name, char *input_name)
{
        return;
}



// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void input_controler (INPUT_DESC *input, int status)
{
	char command[256];	

	// Moves the right input
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == image_right.win))
	{
		// Updates the left epipolar line
		g_p_dblRightPoint[0] = (double) (input->ww - input->wxd - 1);
		g_p_dblRightPoint[1] = (double) input->wyd;
		g_p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, g_p_dblRightPoint);

		image_left.wxd += image_right.wxd - image_right.wxd_old;
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);
		
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
		if (g_p_dblLeftEpipolarLine == NULL)
		{
			// Updates the left epipolar line
			g_p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
			g_p_dblRightPoint[1] = (double) image_right.wyd;
			g_p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, g_p_dblRightPoint);
		}
		
		image_left.wyd = (int) ((-g_p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - g_p_dblLeftEpipolarLine[2]) / g_p_dblLeftEpipolarLine[1] + 0.5);

		sprintf (command, "move %s to %d, %d;", image_left.name, image_left.wxd, image_left.wyd);
		interpreter (command);
	}
	
	// Updates the current center point
	if ((input->mouse_button == GLUT_MIDDLE_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{	
		g_nCenterX = input->x;
		g_nCenterY = input->y;
		stereo_input_display ();
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
	return;
}



// ----------------------------------------------------------------------------
// sum_neurons_output -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

float sum_neurons_output (NEURON_LAYER *neuron_layer, float band_with)
{
        int xi, yi, wi, hi, begin, end;
        float sum = .0f;
        
        wi = neuron_layer->dimentions.x;
        hi = neuron_layer->dimentions.y;
        
        begin = (int) ((float) wi * (.5f - .5f * band_with) + .5f);
        end   = (int) ((float) wi * (.5f + .5f * band_with) + .5f);

        for (yi = 0; yi < hi; yi++)
                for (xi = begin; xi < end; xi++)
                      sum += neuron_layer->neuron_vector[xi + yi * wi].output.fval;
        
        sum /= (float) ((end - begin) * hi);
        
	return (sum);
}



// ----------------------------------------------------------------------------
// generate_disparity_map -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void generate_disparity_map (void)
{
 	int i;
	float pyramid_level_frequency;	
	static RECEPTIVE_FIELD_DESCRIPTION *gaussian_kernels = NULL;
	
	if (gaussian_kernels == NULL)
		gaussian_kernels = (RECEPTIVE_FIELD_DESCRIPTION *) (get_filter_by_output (&(nl_pooled_phase_diff)))->private_state;

	// Emptys the disparity map
	for (i = 0; i < nl_disparity_map.num_neurons; i++)
		nl_disparity_map.neuron_vector[i].output.fval = .0f;

        for (PYRAMID_LEVEL = image_left.pyramid_height; PYRAMID_LEVEL >= 0; PYRAMID_LEVEL--)
        {
		all_filters_update ();

		pyramid_level_frequency = 2.0f * M_PI * gaussian_kernels[PYRAMID_LEVEL].frequency;
		for (i = 0; i < nl_disparity_map.num_neurons; i++)
			nl_disparity_map.neuron_vector[i].output.fval += nl_pooled_phase_diff.neuron_vector[i].output.fval / pyramid_level_frequency;
        	
		all_outputs_update ();
	}
	PYRAMID_LEVEL = 0;
}


// ----------------------------------------------------------------------------
// set_vergence - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void set_vergence (void)
{
	int i = 0, wo, ho, nMinX, nMaxX;
	double p_dblRightPoint[2], *p_dblLeftPoint = NULL, *p_dblWorldPoint = NULL, *p_dblLeftEpipolarLine = NULL;
  	float fltGlobalDisparity = FLT_MAX;	

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	
	// Gets the right point
	p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
	p_dblRightPoint[1] = (double) image_right.wyd;

	// Gets the fartest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblFarVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMinX = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMinX = (nMinX < 0) ? 0 : nMinX;

	// Gets the nearest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblNearVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMaxX = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMaxX = (nMaxX > image_left.ww) ? image_left.ww : nMaxX;

	// Gets the left epipolar line
	p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);			

	image_left.wxd = (nMinX + nMaxX) >> 1;
	image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
                    
	while ((fabs (fltGlobalDisparity) > 1.0f) && (i++ < 10))
	{
		generate_disparity_map ();
		
		fltGlobalDisparity = sum_neurons_output (&(nl_disparity_map), .25f);
		
		image_left.wxd -= (int) (fltGlobalDisparity + .5f);
		image_left.wxd = (image_left.wxd < nMinX) ? nMinX : image_left.wxd;
		image_left.wxd = (image_left.wxd > nMaxX) ? nMaxX : image_left.wxd;
		image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
        }
        
        return;
}



// ----------------------------------------------------------------------------
// ReconstructByDisparityMap - Reconstroi o mundo em
// torno do ponto de vergencia atraves do mapa de
// disparidade.
//
// Entrada: Nenhuma.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void ReconstructByDisparityMap (void)
{
        int u, v;
	int w, h, wi, hi, xi, yi;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;
	double p_dblLeftPoint[2], p_dblRightPoint[2], *p_dblWorldPoint = NULL;
	int nRed, nGreen, nBlue;
	
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

	for (v = 0; v < h; v++)
	{
		for (u = nStartCol; u < nEndCol; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			pixel = image_right.neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			nRed = RED(pixel);
			nGreen = GREEN(pixel);
			nBlue = BLUE(pixel);

			p_dblRightPoint[0] = (double) (image_right.ww - xi - 1);
			p_dblRightPoint[1] = (double) yi;

			fltDisparity = nl_disparity_map.neuron_vector[w * v + u].output.fval;

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;
	
			p_dblLeftPoint[0] = (double) (image_left.ww - xi - 1) - (double) fltDisparity;
			p_dblLeftPoint[1] = (double) yi;

			p_dblWorldPoint = StereoImage2WorldPoint (p_dblRightPoint, p_dblLeftPoint);
			
			// Updates the map and rotates the frame coordinates			       			
			ViewerUpdateMap(p_dblWorldPoint[2], p_dblWorldPoint[0], p_dblWorldPoint[1], nRed, nGreen, nBlue);
		}
	}
	
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
	double p_dblLeftPoint[2], p_dblRightPoint[2], *p_dblWorldPoint = NULL;

	printf("Reconstruct\n");

	p_dblLeftPoint[0] = (double) (image_left.ww - image_left.wxd - 1);
	p_dblLeftPoint[1] = (double) (image_left.wyd);

	p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
	p_dblRightPoint[1] = (double) (image_right.wyd);

	p_dblWorldPoint = StereoImage2WorldPoint (g_p_dblRightPoint, g_p_dblLeftPoint);

	// Vai vei
	ReconstructByDisparityMap();

	glutPostWindowRedisplay (image_right.win);
	glutPostWindowRedisplay (image_left.win);

	glutIdleFunc ((void (* ) (void)) check_forms);
	
	return;
}



// ----------------------------------------------------------------------------
// Reconstruct -
//
// Entrada:
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

int SavePyramidImage (void)
{
	int xi, yi, wi, hi, w, h, pyramid_level, pyramid_level_offset, w_offset;
	char file_name[256];
	FILE *image_file = (FILE *) NULL;
	INPUT_DESC *input;
	int nPixel;

	// Opens image file
	if ((input = get_input_by_win (glutGetWindow ())) != NULL)
	{
		strcpy (file_name, input->name);
		strcat (file_name, ".pnm");
        	if ((image_file = fopen (file_name, "w")) == (FILE *) NULL)
       			Erro ("Cannot create file: ", file_name, "");

		// Write the image format and comment
		fprintf (image_file, "P3\n# CREATOR: MAE save_image ()\n");	
		
		wi = input->neuron_layer->dimentions.x;
		hi = input->neuron_layer->dimentions.y;

		// Write the image dimentions and range
		fprintf (image_file, "%d %d\n%d\n", 2 * wi, hi, 255);
	}
	else
		return (-1);
		
	for (yi = hi - 1; yi >= 0; yi--)  
	{
		for (pyramid_level = 0, pyramid_level_offset = 0, w_offset = 0, w = wi, h = hi; pyramid_level <= input->pyramid_height; pyramid_level_offset += w * h, w_offset += w, w = w >> 1, h = h >> 1, pyramid_level++)
       		{
			if (yi >= h) break;
			
			for (xi = 0; xi < w; xi++) 
			{
				nPixel = input->neuron_layer->neuron_vector[pyramid_level_offset + yi * w + xi].output.ival;
				fprintf (image_file, "%d\n%d\n%d\n", (int) RED(nPixel), (int) GREEN(nPixel), (int) BLUE(nPixel));
			}
		}
		
		for (xi = w_offset; xi < 2 * wi; xi++)
			fprintf (image_file, "0\n0\n0\n");
	}
	
	// Closes image file
	fclose (image_file);

	return (0);
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
	int i, j, w, h, nPixel;
	GLubyte cRed, cGreen, cBlue;
	int *p_nUndistortedImage = NULL;
	static int *p_nDistortedImage = NULL;
	double *aux = NULL;
	static double dblWorldPoints[6];
	static int nCurrentPoint = 0;
	
	switch (key_value[0]) 
	{
		case 'P':
			PYRAMID_LEVEL = (PYRAMID_LEVEL + 1) % image_left.pyramid_height;
			break;
		case 'p':
			PYRAMID_LEVEL = (--PYRAMID_LEVEL < 0) ? 0 : PYRAMID_LEVEL;
			break;
		//--------------------------------------------------------------
		// Stereo system interface
		//--------------------------------------------------------------
		// Emptys the disparity map
		case 'R':
		case 'r':
			Reconstruct ();
			break;
		// Emptys the disparity map
		case 'D':
		case 'd':
			generate_disparity_map ();
			break;
		// Starts the vergence process
		case 'V':
		case 'v':	
			set_vergence ();
			break;
		// Emptys the disparity map
		case 'E':
		case 'e':	
			for (i = 0; i < nl_disparity_map.num_neurons; i++)
				nl_disparity_map.neuron_vector[i].output.fval = .0f;
			break;
		// Maps the image left and right current points to world point
		case 'M':
		case 'm':
			g_p_dblLeftPoint[0] = (double) (image_left.ww - image_left.wxd - 1);
			g_p_dblLeftPoint[1] = (double) (image_left.wyd);

			g_p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
			g_p_dblRightPoint[1] = (double) (image_right.wyd);

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
			stereo_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
			update_input_image (&(image_left));
			
			// Updates the right input image
			load_input_image (&(image_right), NULL);
			stereo_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
			update_input_image (&(image_right));
			
			nLock = 0;
			break;
		// Undistortes the left and right images
		case 'A':
		case 'a':
			if (nLock++)
				break;
	
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
			stereo_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
			stereo_input_display ();

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
			stereo_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
			stereo_input_display ();
			
			break;
		// Zoom out
		case 'z':
			g_fltZoom *= 0.5;			
			glutSetWindow(image_left.win);
			stereo_input_display ();
			glutSetWindow(image_right.win);
			stereo_input_display ();
			break;
		// Zoom in
		case 'Z':
			g_fltZoom *= 2.0;
			glutSetWindow(image_left.win);
			stereo_input_display ();
			glutSetWindow(image_right.win);
			stereo_input_display ();
			break;
		// Toggles the epipolar geometry testing mode
		case 'T': 
		case 't':
			g_nTestingEpipolarGeometry = !g_nTestingEpipolarGeometry;
			glutPostWindowRedisplay (image_left.win);
			break;
		// Saves the pyramid image
		case 'S':
		case 's':
			SavePyramidImage ();
			break;
		// Exits application
		case 'Q':
		case 'q':
			StereoQuit ();
			exit (0);
	}
		
	return;
}
