#include "robot_user_functions.h"
#include "viewer.hpp"


// Variaveis globais
int g_nMoveLeft;
extern FILE *g_pLogFile;
float *g_fltDispAux;
float *g_fltConfAux;
DISP_DATA *g_DispData;
DISP_DATA *g_AllDispData;
int g_nAllSamples, g_nAllSamplesOld;

RECEPTIVE_FIELD_DESCRIPTION *g_ReceptiveField;

int g_nInputType;

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

void robot_input_mouse (int button, int state, int x, int y)
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
    	double *p_dblCameraOrientation = NULL;

	interpreter ("toggle move_active;");
    	interpreter ("toggle draw_active;");

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
		robot_check_input_bounds (input, input->wxd, input->wyd);
		glutSetWindow(input->win);
		robot_input_display ();
		all_filters_update ();
		all_outputs_update ();
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



// ----------------------------------------------------------------------------
// ReconstructByDisparityMap - Reconstroi o mundo em
// torno do ponto de vergencia atraves do mapa de
// disparidade.
//
// Entrada: Nenhuma.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void ReconstructByDisparityMap()
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

	fltRangeCols = 0.85;
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
	
			p_dblLeftPoint[0] = (double) (image_left.ww - xi - 1 - fltDisparity);
			p_dblLeftPoint[1] = (double) yi;

			p_dblWorldPoint = StereoImage2WorldPoint (p_dblRightPoint, p_dblLeftPoint);
			
			// Updates the map and rotates the frame coordinates			       			
			ViewerUpdateMap(p_dblWorldPoint[2], p_dblWorldPoint[0], p_dblWorldPoint[1], nRed, nGreen, nBlue);
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

void InitDispDataState (int nNumNeurons)
{
	int i, sample;

	for (i = 0; i < nNumNeurons; i++)
	{
		g_DispData->neg_slope[i] = 1;
	}

	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < nNumNeurons; i++)
		{
			g_DispData->samples[sample][i].val = FLT_MAX;
			g_DispData->samples[sample][i].conf = 0.0;
			g_DispData->samples[sample][i].pos = 0;
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

void AddLocalMin(int i, double minus_out, double minus_conf, int pos)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
		if (minus_out <= g_DispData->samples[victim_sample][i].val)
			break;
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			g_DispData->samples[moving_sample][i] = g_DispData->samples[moving_sample - 1][i];
			moving_sample--;
		}
		g_DispData->samples[moving_sample][i].val = minus_out;
		g_DispData->samples[moving_sample][i].conf = minus_conf;
		g_DispData->samples[moving_sample][i].pos = pos;
	}
	
	return;
}



// ----------------------------------------------------------------------------
// set_vergence - 
//
// Inputs: none
//
// Output: none
// ----------------------------------------------------------------------------

void set_vergence (void)
{
	int nMaxScan, nMinScan;
	int i, j, x, y, nIndex;
	int wo, ho;
	float fltSumOutputCells;
	float fltMinSum, fltAux, fltSum;
	int nVergLeft = 0;
	double p_dblRightPoint[2], *p_dblLeftPoint = NULL, *p_dblWorldPoint = NULL, *p_dblLeftEpipolarLine = NULL;
       
	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	
	// Gets the right point
	p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
	p_dblRightPoint[1] = (double) image_right.wyd;

	// Gets the fartest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblFarVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMinScan = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMinScan = (nMinScan < 0) ? 0 : nMinScan;

	// Gets the nearest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblNearVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMaxScan = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMaxScan = (nMaxScan > image_left.ww) ? image_left.ww : nMaxScan;

	// Gets the left epipolar line
	p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);			

	g_nAllSamplesOld = g_nAllSamples;
	g_nAllSamples = (nMaxScan - nMinScan);
	
	if (g_fltDispAux == NULL)
	{
		g_fltDispAux = (float *) malloc (sizeof(float) * wo * ho);
		g_fltConfAux = (float *) malloc (sizeof(float) * wo * ho);
	}

	if (g_DispData == NULL)
	{
		// Aloca memoria para a estrutura DISP_DATA
		g_DispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_DispData->neg_slope = (char *) alloc_mem (wo * ho);
		g_DispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
		for (i = 0; i < NUM_SAMPLES; i++)
			g_DispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
		g_AllDispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_nAllSamplesOld = g_nAllSamples;
		g_AllDispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * g_nAllSamples);
		for (i = 0; i < g_nAllSamples; i++)
			g_AllDispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
	}

	if (g_nAllSamplesOld < g_nAllSamples)
	{
		SAMPLES **p_SamplesOld = g_AllDispData->samples;
		g_AllDispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * g_nAllSamples);

		for (i = 0; i < g_nAllSamplesOld; i++)
			g_AllDispData->samples[i] = p_SamplesOld[i];	
		
		for (i = g_nAllSamplesOld; i < g_nAllSamples; i++)
			g_AllDispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
		
		free (p_SamplesOld);
	}

	// Inicializar a estrutura DISP_DATA
	InitDispDataState (wo * ho);

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < (wo * ho); i++)
	{
		g_fltDispAux[i] = FLT_MAX;
		g_fltConfAux[i] = .0f;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = .0f;
		nl_confidence_map.neuron_vector[i].output.fval = .0f;
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	}

	for (image_left.wxd = nMinScan, fltMinSum = FLT_MAX; image_left.wxd < nMaxScan; image_left.wxd++)
	{
		image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		for (y = 0, fltSumOutputCells = .0f; y < ho; y++) 
		{
			for (x = 0; x < wo; x++)
			{
				nIndex = y * wo + x;
				fltSumOutputCells += nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				fltAux = nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				
                                if (fltAux >= g_fltDispAux[nIndex])
				{
					if (g_DispData->neg_slope[nIndex])
					{
						g_DispData->neg_slope[nIndex] = 0;
						AddLocalMin(nIndex, g_fltDispAux[nIndex], g_fltConfAux[nIndex], image_left.wxd - 1);
						nl_mt_gaussian_map.neuron_vector[nIndex].output.fval = g_fltDispAux[nIndex];
						nl_confidence_map.neuron_vector[nIndex].output.fval = g_fltConfAux[nIndex];
						nl_disparity_map.neuron_vector[nIndex].output.fval = image_left.wxd - 1;
					}
				}
				else
					g_DispData->neg_slope[nIndex] = 1;

				g_fltDispAux[nIndex] = fltAux;
				g_fltConfAux[nIndex] = nl_complex_mono_right.neuron_vector[nIndex].output.fval + nl_complex_mono_left.neuron_vector[nIndex].output.fval;
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].val = g_fltDispAux[nIndex];
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].conf = g_fltConfAux[nIndex];
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].pos = image_left.wxd;
			}
                }
                
		if (fltSumOutputCells < fltMinSum)
		{
			fltMinSum = fltSumOutputCells;
			nVergLeft = image_left.wxd;
		}
	}

	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].val;
		nl_confidence_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].conf;
		nl_disparity_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].pos;
	}

	// Correcao do mapa de disparidade de acordo com a vergencia escolhida
	for (i = 0; i < (wo * ho); i++)
	{
		nl_disparity_map.neuron_vector[i].output.fval -= nVergLeft;
		for (j = 0; j < g_nAllSamples; j++)
			g_AllDispData->samples[j][i].pos -= nVergLeft;
	}

	// Calcula as variacoes das saidas de MT gaussian
	for (i = 0; i < (wo * ho); i++)
	{
		fltSum = .0f;
		for (j = 0; j < g_nAllSamples; j++) 
			fltSum += g_AllDispData->samples[j][i].val;
		nl_mt_gaussian_var.neuron_vector[i].output.fval = (fltSum / (float) g_nAllSamples) - g_DispData->samples[0][i].val;
	}

	image_left.wxd_old = image_left.wxd;
	image_left.wxd = nVergLeft;
	image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

        return;
}


// ----------------------------------------------------------------------------
// set_vergence - 
//
// Inputs: 
//
// Output: none
// ----------------------------------------------------------------------------

void set_vergence_coarse2fine (void)
{
       	int nMaxScan, nMinScan;
	int i, j, x, y, nIndex;
	int wo, ho;
	float fltSumOutputCells;
	float fltMinSum, fltAux, fltSum;
	int nVergLeft;
        double p_dblRightPoint[2], *p_dblLeftPoint = NULL, *p_dblWorldPoint = NULL, *p_dblLeftEpipolarLine = NULL;
       
	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	
	// Gets the right point
	p_dblRightPoint[0] = (double) (image_right.ww - image_right.wxd - 1);
	p_dblRightPoint[1] = (double) image_right.wyd;

	// Gets the fartest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblFarVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMinScan = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMinScan = (nMinScan < 0) ? 0 : nMinScan;

	// Gets the nearest left point
	p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, g_dblNearVergenceCutPlane);
	p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
	nMaxScan = image_left.ww - (int) (p_dblLeftPoint[0] + 0.5) - 1;
	nMaxScan = (nMaxScan > image_left.ww) ? image_left.ww : nMaxScan;
	
	// Gets the left epipolar line
	p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);			
	
	g_nAllSamplesOld = g_nAllSamples;
	g_nAllSamples = (nMaxScan - nMinScan);
	
	if (g_fltDispAux == NULL)
	{
		g_fltDispAux = (float *) malloc (sizeof(float) * wo * ho);
		g_fltConfAux = (float *) malloc (sizeof(float) * wo * ho);
	}

	if (g_DispData == NULL)
	{
		// Aloca memoria para a estrutura DISP_DATA
		g_DispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_DispData->neg_slope = (char *) alloc_mem (wo * ho);
		g_DispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
		for (i = 0; i < NUM_SAMPLES; i++)
			g_DispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));		
		g_AllDispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_nAllSamplesOld = g_nAllSamples;
		g_AllDispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * g_nAllSamples);
		for (i = 0; i < g_nAllSamples; i++)
			g_AllDispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));

	}
	
	if (g_nAllSamplesOld < g_nAllSamples)
	{
		SAMPLES **p_SamplesOld = g_AllDispData->samples;
		g_AllDispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * g_nAllSamples);

		for (i = 0; i < g_nAllSamplesOld; i++)
			g_AllDispData->samples[i] = p_SamplesOld[i];	
		
		for (i = g_nAllSamplesOld; i < g_nAllSamples; i++)
			g_AllDispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
		
		free (p_SamplesOld);
	}
	
	// Inicializar a estrutura DISP_DATA
	InitDispDataState(wo * ho);

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < (wo * ho); i++)
	{
		g_fltDispAux[i] = FLT_MAX;
		g_fltConfAux[i] = .0f;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = .0f;
		nl_confidence_map.neuron_vector[i].output.fval = .0f;
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	}

        for (PYRAMID_LEVEL = image_left.pyramid_height, nVergLeft = nMinScan + (g_nAllSamples >> 1); PYRAMID_LEVEL >= 0; PYRAMID_LEVEL--)
        {
        	fltMinSum = FLT_MAX;
        	nMinScan = nVergLeft - (g_nAllSamples >> (image_left.pyramid_height - PYRAMID_LEVEL + 1));
        	nMaxScan = nVergLeft + (g_nAllSamples >> (image_left.pyramid_height - PYRAMID_LEVEL + 1));
        	
        	for (image_left.wxd = nMinScan; image_left.wxd < nMaxScan; image_left.wxd += (int) pow (2.0f, (float) PYRAMID_LEVEL))
        	{
        		image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
			move_input_window (image_left.name, image_left.wxd, image_left.wyd);
        
        		fltSumOutputCells = .0f;
        		for (y = 0; y < ho; y++) 
        		{
        			for (x = 0; x < wo; x++)
        			{
        				nIndex = y * wo + x;
        				fltSumOutputCells += nl_mt_gaussian.neuron_vector[nIndex].output.fval;
        				fltAux = nl_mt_gaussian.neuron_vector[nIndex].output.fval;
        				
                                        if (fltAux >= g_fltDispAux[nIndex])
        				{
        					if (g_DispData->neg_slope[nIndex])
        					{
        						g_DispData->neg_slope[nIndex] = 0;
        						AddLocalMin(nIndex, g_fltDispAux[nIndex], g_fltConfAux[nIndex], image_left.wxd - 1);
        						nl_mt_gaussian_map.neuron_vector[nIndex].output.fval = g_fltDispAux[nIndex];
        						nl_confidence_map.neuron_vector[nIndex].output.fval = g_fltConfAux[nIndex];
        						nl_disparity_map.neuron_vector[nIndex].output.fval = image_left.wxd - 1;
        					}
        				}
        				else
        					g_DispData->neg_slope[nIndex] = 1;
        
        				g_fltDispAux[nIndex] = fltAux;
        				g_fltConfAux[nIndex] = nl_complex_mono_right.neuron_vector[nIndex].output.fval + nl_complex_mono_left.neuron_vector[nIndex].output.fval;
        				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].val = g_fltDispAux[nIndex];
        				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].conf = g_fltConfAux[nIndex];
        				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].pos = image_left.wxd;
        			}
                        }
                        
        		if (fltSumOutputCells < fltMinSum)
        		{
        			fltMinSum = fltSumOutputCells;
        			nVergLeft = image_left.wxd;
        		}
        	}
        	//nMinScan = nVergLeft;
        	//nMaxScan = nVergLeft + (int) pow (2.0f, (float) PYRAMID_LEVEL);
        }
        PYRAMID_LEVEL = 0;
        
	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].val;
		nl_confidence_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].conf;
		nl_disparity_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].pos;
	}

	// Correcao do mapa de disparidade de acordo com a vergencia escolhida
	for (i = 0; i < (wo * ho); i++)
	{
		nl_disparity_map.neuron_vector[i].output.fval -= nVergLeft;
		for (j = 0; j < g_nAllSamples; j++)
			g_AllDispData->samples[j][i].pos -= nVergLeft;
	}

	// Calcula as variacoes das saidas de MT gaussian
	for (i = 0; i < (wo * ho); i++)
	{
		fltSum = 0.0;
		for (j = 0; j < g_nAllSamples; j++) 
			fltSum += g_AllDispData->samples[j][i].val;
		nl_mt_gaussian_var.neuron_vector[i].output.fval = (fltSum / (float) g_nAllSamples) - g_DispData->samples[0][i].val;
	}

	image_left.wxd_old = image_left.wxd;
	image_left.wxd = nVergLeft;
	image_left.wyd = (int) ((-p_dblLeftEpipolarLine[0] * (double) (image_left.ww - image_left.wxd - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

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
		robot_input_display ();
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
// LowPass - Passa uma gaussiana no mapa de disparidade
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void LowPass (void)
{
	int x, y, wo, ho;

	if (g_fltDispAux == NULL)
	{
		return;
	}

	if (g_ReceptiveField == NULL)
	{
		g_ReceptiveField = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(g_ReceptiveField, 5, 1.0);
	}

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			g_fltDispAux[y * wo + x] = apply_gaussian_kernel(g_ReceptiveField, &nl_disparity_map, x, y);
		}
	}

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			nl_disparity_map.neuron_vector[y * wo + x].output.fval = g_fltDispAux[y * wo + x];
		}
	}

	glutIdleFunc ((void (* ) (void)) check_forms);
	
	return;
}



// ----------------------------------------------------------------------------
// MoveXImageLeft - Move o foco da ImageLeft
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void MoveXImageLeft (void)
{
	image_left.wxd += g_nMoveLeft;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	glutPostWindowRedisplay (image_left.win);

	glutIdleFunc ((void (* ) (void)) check_forms);
	
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

void calculate_disparity (NEURON_LAYER *nlDisparityMap, int wxd)
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
				dist = distance_neig (g_DispData->samples[0], g_DispData->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = g_DispData->samples[0][y * w + x];
			g_DispData->samples[0][y * w + x] = g_DispData->samples[best_sample][y * w + x];
			g_DispData->samples[best_sample][y * w + x] = temp;
		}
	}

	for (i = 0; i < w*h; i++)
		nlDisparityMap->neuron_vector[i].output.fval = (float) (g_DispData->samples[0][i].pos - wxd);
        
        return;
}



/************************************************************************/
/* Name: 					*/
/* Description:							*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void GenerateErrorSurface (void)
{
	double dblWorkingDistance,
	       p_dblRightPoint[2], 
	       *p_dblLeftPoint = NULL, 
	       *p_dblWorldPoint = NULL, 
	       *p_dblLeftEpipolarLine = NULL,
	       p_dblMeanPrecision[3],
	       p_dblMinPrecision[] = {DBL_MAX, DBL_MAX , DBL_MAX},
	       p_dblMaxPrecision[] = {DBL_MIN, DBL_MIN , DBL_MIN},
	       dblBaselineLength;
	static double *p_dblPrecision = NULL;
	int xr, yr, xl, yl, w, h, i, j, k;
	
	FILE *pFile = NULL;
	pFile = fopen ("SpatialResolution.dat", "w");

	w = image_right.ww;
	h = image_right.wh;

	if (p_dblPrecision == NULL)
		p_dblPrecision = (double *) malloc (3 * w * h * sizeof (double));
	
	for (j = 0, dblBaselineLength = StereoGetBaselineLength (); j <= 5; j++, dblBaselineLength = (j == 1) ? 125.0 : 2.0 * dblBaselineLength, StereoSetBaselineLength (dblBaselineLength))
	{
		for (i = 0, dblWorkingDistance = 125.0; i < 10; i++, dblWorkingDistance *= 2.0)
		{
			k = 0;
			p_dblMeanPrecision[0] = p_dblMeanPrecision[1] = p_dblMeanPrecision[2] = 0.0;
			for (yr = 0; yr < h; yr += 8)
			{
				for (xr = 0; xr < w; xr += 8)
				{
					p_dblRightPoint[0] = (double) (w - xr - 1);
					p_dblRightPoint[1] = (double) yr;
					p_dblLeftEpipolarLine = StereoGetEpipolarLine (RIGHT_CAMERA, p_dblRightPoint);
					p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, dblWorkingDistance);
					p_dblLeftPoint = StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);
					xl = w - (int) (p_dblLeftPoint[0] + 0.5) - 1;
					if ((xl < 0) || (xl >= w)) continue;				
					yl = (int) ((-p_dblLeftEpipolarLine[0] * (double) (w - xl - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
					p_dblLeftPoint[0] = (double) (w - xl - 1);
					p_dblLeftPoint[1] = (double) yl;
					p_dblWorldPoint = StereoImage2WorldPoint (p_dblRightPoint, p_dblLeftPoint);
					p_dblPrecision[3 * (xr + yr * w) + 0] = p_dblWorldPoint[0];
					p_dblPrecision[3 * (xr + yr * w) + 1] = p_dblWorldPoint[1];
					p_dblPrecision[3 * (xr + yr * w) + 2] = p_dblWorldPoint[2];
		
					xl--;
					xl = (xl < 0) ? 0 : xl;
					yl = (int) ((-p_dblLeftEpipolarLine[0] * (double) (w - xl - 1) - p_dblLeftEpipolarLine[2]) / p_dblLeftEpipolarLine[1] + 0.5);
					p_dblLeftPoint[0] = (double) (w - xl - 1);
					p_dblLeftPoint[1] = (double) yl;
					p_dblWorldPoint = StereoImage2WorldPoint (p_dblRightPoint, p_dblLeftPoint);
					p_dblPrecision[3 * (xr + yr * w) + 0] -= p_dblWorldPoint[0];
					p_dblPrecision[3 * (xr + yr * w) + 1] -= p_dblWorldPoint[1];
					p_dblPrecision[3 * (xr + yr * w) + 2] -= p_dblWorldPoint[2];
					p_dblMeanPrecision[0] += p_dblPrecision[3 * (xr + yr * w) + 0] = sqrt (p_dblPrecision[3 * (xr + yr * w) + 0] * p_dblPrecision[3 * (xr + yr * w) + 0]);
					p_dblMeanPrecision[1] += p_dblPrecision[3 * (xr + yr * w) + 1] = sqrt (p_dblPrecision[3 * (xr + yr * w) + 1] * p_dblPrecision[3 * (xr + yr * w) + 1]);
					p_dblMeanPrecision[2] += p_dblPrecision[3 * (xr + yr * w) + 2] = sqrt (p_dblPrecision[3 * (xr + yr * w) + 2] * p_dblPrecision[3 * (xr + yr * w) + 2]);				
				
					if (p_dblMinPrecision[2] > p_dblPrecision[3 * (xr + yr * w) + 2])
						p_dblMinPrecision[2] = p_dblPrecision[3 * (xr + yr * w) + 2];
					
					if (p_dblMaxPrecision[2] < p_dblPrecision[3 * (xr + yr * w) + 2])
						p_dblMaxPrecision[2] = p_dblPrecision[3 * (xr + yr * w) + 2];
					
					k++;
				}
			}

			if (k != 0)
			{
				p_dblMeanPrecision[0] /= (double) k;
				p_dblMeanPrecision[1] /= (double) k;
				p_dblMeanPrecision[2] /= (double) k;
			}
			else
			{
				p_dblMeanPrecision[0] = -1.0;
				p_dblMeanPrecision[1] = -1.0;
				p_dblMeanPrecision[2] = -1.0;
			}

			fprintf (pFile, "%f %f %f %f %f\n", dblWorkingDistance, dblBaselineLength, p_dblMeanPrecision[0], p_dblMeanPrecision[1], p_dblMeanPrecision[2]);
		}
		fprintf (pFile, "\n");
	}
	fclose (pFile);
	return;
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
		//--------------------------------------------------------------
		// Stereo system interface
		//--------------------------------------------------------------
		case 'R':
		case 'r':
			Reconstruct ();
			break;
		case 'E':
		case 'e':
			ViewerEraseMap ();
			break;
		case 'F':
		case 'f':
			LowPass ();
			break;
		case 'D':
		case 'd':
			calculate_disparity (&nl_disparity_map, image_left.wxd);
			glutPostWindowRedisplay (out_disparity_map.win);
			break;
		// Starts the vergence process
		case 'V':
			set_vergence_coarse2fine ();
			break;
		case 'v':	
			set_vergence ();
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
			robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
			update_input_image (&(image_left));
			
			// Updates the right input image
			load_input_image (&(image_right), NULL);
			robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
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
			
			update_input_neurons (&(image_left));
			robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
			update_input_image (&(image_left));

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

			update_input_neurons (&(image_right));
			robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
			update_input_image (&(image_right));
			
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
		// Toggles the epipolar geometry testing mode
		case 'T': 
		case 't':
			g_nTestingEpipolarGeometry = !g_nTestingEpipolarGeometry;
			glutPostWindowRedisplay (image_left.win);
			break;
		case 'G': 
		case 'g':
			GenerateErrorSurface ();
			break;
		// Exits application
		case 'Q':
		case 'q':
			StereoQuit ();
			exit (0);
	}
		
	return;
}
