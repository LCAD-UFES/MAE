#include "robot_user_functions.h"

// ***** Variaveis globais ******

int g_nColorImage = COLOR_MONO_256;
int g_Status;
int g_nMoveLeft;
int g_nVerticalGap = VERT_GAP;

float *g_fltDispAux;
float *g_fltConfAux;
char  *g_neg_slope;

DISP_DATA_LIST *g_DispData_list;
DISP_DATA_LIST *g_AllDispData_list;
DISP_DATA_LIST *g_AllDispDataProp_list;

INPUT_DESC *g_input = NULL;
INPUT_DESC *g_linked_inp = NULL;
INPUT_DESC *g_seq_inp = NULL;
int g_isRight = 0;
int g_isNext = 0;
int g_Arrows = 0;
int g_Arrow_Grid = ARROW_GRID;
float g_Threshold = THRESHOLD;
int g_pursuit = 0, g_next_x1, g_next_y1, g_next_x2, g_next_y2;
int g_Area = 0;

RECEPTIVE_FIELD_DESCRIPTION *g_ReceptiveField;

// ***** Variaveis para Mapping ******
TMap g_map;
GLint g_wndMap = 0;
GLint g_wndMapTop = 0;
float g_fCenterX, g_fCenterY, g_fCenterZ;	// Centro de rotacao

int g_nMouseX, g_nMouseY;
GLfloat rot, w1, w2, w3;			// Variables for rotation calculation
GLint prevx, prevy;				// Remember previous x and y positions

GLfloat w = MAP_WINDOW_WIDTH;
GLfloat h = MAP_WINDOW_HEIGHT;

GLfloat	g_fltFovW = (INPUT_WIDTH / 2.0) * MAX_Z_VIEW / LAMBDA;
GLfloat	g_fltFovH = (INPUT_HEIGHT / 2.0) * MAX_Z_VIEW / LAMBDA;

int verbose = VERBOSE;


// ----------------------------------------------------------------------------
// StrReplaceChar - Troca todas as ocorrencias de chr1 em strAux para chr2
//
// Entrada: strAux - String
//			chr1 - Caracter a ser procurado
//			chr2 - Caracter que substituira chr1
//
// Saida: Quantidade de trocas feitas em strAux;
// ----------------------------------------------------------------------------
int StrReplaceChar(char *strAux, char chr1, char chr2)
{
	int i, n;

	n = 0;
	for (i = 0; i < strlen(strAux); i++)
	{
		if (strAux[i] == chr1)
		{
			strAux[i] = chr2;
			n++;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// SalvaTMap - Salva num arquivo o estado da reconstrucao
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SalvaTMap()
{
	FILE *file;

	file = fopen(FILE_TMAP,"w");
	TMapSave(g_map, file);
	fclose(file);
	
	file = fopen(FILE_TMAP_ASCII,"w");
	TMapSavePoints(g_map, file);
	fclose(file);
}

// ----------------------------------------------------------------------------
// LeTMap - Le de um arquivo o estado da reconstucao
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void LeTMap()
{
	FILE *file;

	file = fopen(FILE_TMAP,"r");
	TMapLoad(&g_map, file);
	fclose(file);
}

// ----------------------------------------------------------------------------
// SalvaInputImage - Salva a imagem de uma input em formato RBT
//
// Entrada: input, filename
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SalvaInputImage(INPUT_DESC *input, char *filename)
{
	FILE *file;
	int x, y, i;
	GLubyte red, green, blue;

	file = fopen(filename,"w");
	if (!file)
		Erro ("Cannot open file ", filename, ". Error in SalvaInputImage.");
	for (x = 0; x < input->ww; x++)
	{
		for (y = 0; y < input->wh; y++)
		{
			i = 3 * (y * input->tfw + x);
			red   = input->image[i];
			green = input->image[i + 1];
			blue  = input->image[i + 2];
			fprintf(file,"%c", (GLubyte) (0.5 + (red + green + blue) / 3.0));
		}
	}
	fclose(file);
	printf ("File %s saved.\n", filename);
}

// ----------------------------------------------------------------------------
// SalvaImagens - Salva as imagens de todas as inputs em formato RBT
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SalvaImagens()
{
	char filename[100];
	INPUT_LIST *i_list;
	time_t now;
	struct tm *t;

	for (i_list = global_input_list; i_list != (INPUT_LIST *) NULL; i_list = i_list->next)
	{
		now = time(NULL);
		t = localtime(&now);
		sprintf(filename,"%s_%04d%02d%02d_%02d%02d%02d.rbt", i_list->input->name, 
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		SalvaInputImage(i_list->input, filename);
	}
	
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}


// ----------------------------------------------------------------------------
// init -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
int init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;

	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;

	return 0;
}

// ----------------------------------------------------------------------------
// DrawWorld - Desenha a estrutura TMap
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
GLvoid DrawWorld()
{
	int i, j;

	glBegin(GL_POINTS);
	{
		// Calcular as coord. do quadrado
		for (j = 0; j < g_map.height; j++)
		{
			for (i = 0; i < g_map.width; i++)
			{
				if (g_map.point[j][i].dist != DIST_INFINITY)
				{
					glColor3f(g_map.point[j][i].intensity, g_map.point[j][i].intensity,
						  g_map.point[j][i].intensity);
					glVertex3f(g_map.point[j][i].x, g_map.point[j][i].y, g_map.point[j][i].z);
				}
			}
		}
	}
	glEnd();
}

// ----------------------------------------------------------------------------
// DrawRobot - Desenha o robo
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
GLvoid DrawRobot()
{
	// Robot
	glColor3f(1.0, 0.0, 0.0);
	glutWireSphere(ROBOT_SIZE / 2.0, 10, 10);
}

// ----------------------------------------------------------------------------
// DrawRobotFOV - Desenha o campo de visao do robo
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
GLvoid DrawRobotFOV()
{
	// Field of view
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-g_fltFovW, g_fltFovH, -MAX_Z_VIEW);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(g_fltFovW, g_fltFovH, -MAX_Z_VIEW);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-g_fltFovW, -g_fltFovH, -MAX_Z_VIEW);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(g_fltFovW, -g_fltFovH, -MAX_Z_VIEW);
	}
	glEnd();	

	glBegin(GL_LINES);
	{
		glVertex3f(-g_fltFovW, g_fltFovH, -MAX_Z_VIEW);
		glVertex3f(-g_fltFovW, -g_fltFovH, -MAX_Z_VIEW);
		glVertex3f(g_fltFovW, -g_fltFovH, -MAX_Z_VIEW);
		glVertex3f(g_fltFovW, g_fltFovH, -MAX_Z_VIEW);
	}
	glEnd();	
}

// ----------------------------------------------------------------------------
// DrawGLScene - Desenha a cena
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void DrawGLScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glTranslatef(g_fCenterX, g_fCenterY, g_fCenterZ);	// Centre and away the viewer
	glRotatef(4*rot, w1, w2, w3);				// Do the transformation
	glTranslatef(-g_fCenterX, -g_fCenterY, -g_fCenterZ);	// Centre and away the viewer
	glPointSize(POINT_SIZE);
	DrawWorld();
	DrawRobotFOV();

	glutSwapBuffers();					// Swap buffers
	glFlush();
}

// ----------------------------------------------------------------------------
// DrawGLSceneTop - Desenha a cena
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void DrawGLSceneTop()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glPointSize(POINT_SIZE);
//	DrawWorld();
	glPushMatrix();
	{
		glRotatef(90.0, 1.0, 0.0, 0.0);

		DrawWorld();
		
		glPushMatrix();
		{
			glTranslatef(0.0, 0.0, (ROBOT_SIZE / 2.0) - 7.0);
			DrawRobot();
		}
		glPopMatrix();
		
		DrawRobotFOV();
	}
	glPopMatrix();
	
	glutSwapBuffers();					// Swap buffers
	glFlush();
}


// ----------------------------------------------------------------------------
// Transform - Realiza a transformacao para a visualizacao do WindowMap
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void Transform(GLfloat Width , GLfloat Height )
{
	glViewport(0,0, (GLfloat)Width, (GLfloat)Height);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0f, Width/Height, 0.1f, MAX_Z_VIEW * 1.2); 
	glTranslatef(0.0, 0.0, 0.0f);				// Centre and away the viewer
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// ----------------------------------------------------------------------------
// InitGL - Inicializacao de alguns parametros do OpenGL
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void InitGL(GLint Width, GLint Height)
{
	glClearColor(0.0f, 0.0f, 0.3f, 0.0f);			// This Will Clear The Background (r,g,b,alpha)
	glClearDepth(1.0);					// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);					// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
	glShadeModel(GL_SMOOTH);				// Enables Smooth Color Shading
	Transform(Width,Height);				// Do our viewing transform
}

// ----------------------------------------------------------------------------
// ResizeWindow - Funcao chamada quando a janela e redimensionada
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void ResizeWindow(GLint Width, GLint Height)
{
	if (Height == 0)					// Prevent A Divide By Zero If The Window Is Too Small
		Height = 1;

	Transform(Width,Height);				// Do our viewing transform
	w = glutGet((GLenum)GLUT_WINDOW_WIDTH);			// Make sure our window size is updated
	h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
}

// ----------------------------------------------------------------------------
// find_axis_of_rotation - Routine to compute the axis and angle of 
// rotation of a "crystal sphere" inverted on the canvas, under the
// action of the mouse.
// The mouse moves from the point (x,y) on canvas by the amount (dx, dy).
// This causes the glass sphere to rotate about an axis (w1,w2,w3) by an
// angle "rot".  The axis is computed with respect to the coordinates
// attached to the sphere, not the world frame.
//
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void find_axis_of_rotation(int x, int y, int dx, int dy, GLfloat *rot, GLfloat *w1, GLfloat *w2, GLfloat *w3)
{
	GLfloat m[16];						// To return the OpenGL transformation matrix values
	float z, dz;
	float R2, r2;
	float a1, a2, a3;
	
	w = glutGet((GLenum)GLUT_WINDOW_WIDTH);			// Make real sure we have the correct
	h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);		// window height and width
	
	R2 = (w*w+h*h)/4;					// sqrt(R2) is window's half-diagonal
	
	// z determines the mouse contact point (x,y,z) on the
	// "invisible glass" sphere of radius sqrt(R2).
	r2 = x*x + y*y;
	z = sqrt(R2-r2);
	
	// The motion of the mouse produces a vector (dx,dy,dz).
	// The first two components are read off the screen.  The
	// third component is computed assuming that the vector
	// (dx,dy,dz) is tangent to the sphere, thus orthogonal
	// to the vector (x,y,z).
	dz = -(x*dx+y*dy)/z;
	
	// the action of the displacement vector (dx,dy,dz) at the
	// contact point (x,y,z) rotates the virtual "glass sphere" about
	// an axis (a1,a2,a3) which is computed as the cross-product
	// of the vectors (x,y,z) and (dx,dy,dz).
	a1 = y*dz-z*dy;
	a2 = z*dx-x*dz;
	a3 = x*dy-y*dx;
	
	// the rotation angle equals displacement over the radius.
	*rot = (180/M_PI)*sqrt(dx*dx+dy*dy+dz*dz)/sqrt(R2);
	
	// The components (a1,a2,a3) of the axis of rotation w are
	// with respect to the coordiantes fixed to the screen.
	// We convert this to the model's local coordinates and
	// arrive at the rotation axis (w1, w2, w3) which
	// is computed as M^T a, where M is the ModelView matrix.
	glGetFloatv(GL_MODELVIEW_MATRIX,m);
	*w1 = m[0]*a1 + m[1]*a2 + m[2]*a3;
	*w2 = m[4]*a1 + m[5]*a2 + m[6]*a3;
	*w3 = m[8]*a1 + m[9]*a2 + m[10]*a3;
}

// ----------------------------------------------------------------------------
// Mouse - Click do mouse
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
GLvoid Mouse( int b , int s, int xx, int yy)
{
	prevx = xx - w/2;
	prevy = h/2 - yy;
}

// ----------------------------------------------------------------------------
// MouseMotion - Movimentacao do mouse
// 
// Entradas: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MouseMotion(int xx, int yy)
{
	int x, y, dx, dy;

	x = xx - w/2;
	y = h/2 - yy;
	dx = x - prevx;
	dy = y - prevy;
	prevx = x;		// Save x and y for dx and dy calcs next time
	prevy = y;
	find_axis_of_rotation(x, y, dx, dy, &rot, &w1, &w2, &w3);
	glutPostRedisplay();
}

// ----------------------------------------------------------------------------
// InitWindowMap - Inicializa os recursos da janela do WindowMap
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void InitWindowMap()
{
	if (g_wndMap == 0)
	{
		// Inicializa o modo de exibicao
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

		// Posicionamento e tamanho da janela
		glutInitWindowSize(MAP_WINDOW_WIDTH, MAP_WINDOW_HEIGHT);
		glutInitWindowPosition(0, 0);

		g_wndMap = glutCreateWindow("Mapping");

		glutDisplayFunc(DrawGLScene);
		glutMouseFunc(Mouse);
		glutMotionFunc(MouseMotion);
		glutReshapeFunc(ResizeWindow);
		glutKeyboardFunc (keyboard);

		// Seleciona o range de visualizacao
		InitGL(MAP_WINDOW_WIDTH, MAP_WINDOW_HEIGHT);

		// Inicializando o TMap
		TMapInitialize(&g_map, INPUT_WIDTH, INPUT_HEIGHT, FOCAL_DISTANCE);
	
		g_fCenterX = 0.0;
		g_fCenterY = 0.0;
		g_fCenterZ = -80.0;
	}
}

// ----------------------------------------------------------------------------
// InitWindowMapTop - 
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void InitWindowMapTop()
{
	if (g_wndMapTop == 0)
	{
		// Inicializa o modo de exibicao
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

		// Posicionamento e tamanho da janela
		glutInitWindowSize(230, 400);
		glutInitWindowPosition(0, 300);

		g_wndMapTop = glutCreateWindow("Mapping Top");
		
		glutKeyboardFunc (keyboard);

		glClearColor(0.0f, 0.0f, 0.25f, 0.0f);	// This Will Clear The Background (r,g,b,alpha)

		// Seleciona o range de visualização
		glOrtho(-g_fltFovW * 1.1, g_fltFovW * 1.1, -25, MAX_Z_VIEW * 1.2, -25, MAX_Z_VIEW * 1.2);

		glutDisplayFunc(DrawGLSceneTop);
	}
}


// ----------------------------------------------------------------------------
// robot_make_input_image - Inicializa a estrutura image da input
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
	
	switch (TYPE_SHOW)
	{
		case SHOW_FRAME:
		{
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
		}
		break;
		
		case SHOW_WINDOW:
		{
			input->vpw = input->ww;
			input->vph = input->wh;
		}
		break;
			
		default:
		{
			sprintf(message,"%d. It should be SHOW_FRAME or SHOW_WINDOW.", TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in robot_make_input_image.");
		}
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;
	
	if(input->image == (GLubyte *) NULL)
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
}

// ----------------------------------------------------------------------------
// InitWindow - Inicializa os recursos de cada janela de entrada
//
// Entrada: input - Janela de entrada
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void InitWindow(INPUT_DESC *input)
{
	int x, y;
	int nImageSource;
	
	if (verbose)
		printf("Input: %-20s  InitWindow\n", input->name);

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	switch (nImageSource)
	{
		case GET_IMAGE_FROM_SOCKET:
		{
			// Code suppressed from this software version.
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_FROM_PNM:
		{
			// Code suppressed from this software version.
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_TEST_STIMULUS:
		{
			// Code suppressed from this software version.
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_FROM_RBT:
		{
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_FROM_JPEG:
		{
			robot_make_input_image (input);
		}
		break;

		case GET_IMAGE_FROM_MPEG:
		{
			robot_make_input_image (input);
		}
		break;

		default:
		{
			Erro ("Invalid image source.", "", "");
		}
	}

	init (input);

	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);
	if (!input->win)
		Erro ("Cannot create window ", input->name, ". Error in InitWindow.");

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);

	if (WINDOW_MAP)
	{
		// Cria a janela do mapeamento
		InitWindowMap();
		InitWindowMapTop();
	}
}


/*!
**********************************************************************************
* Function:	GetInputGeneratorParam
* Description:	Returns a input generator parameter
* Inputs:	input descriptor , parameter index
* Output:	result = required parameter
*		pointer to parameter list or NULL
**********************************************************************************
*/
PARAM_LIST *GetInputGeneratorParam (INPUT_DESC *input, int param_index, PARAM *result)
{
	PARAM_LIST *pl = input->input_generator_params;
	int i;

	for (i = 0; (pl != NULL) && (i < param_index); pl = pl->next, i++);
	
	if (pl != NULL)
		*result = pl->param;

	return pl;

} /* end of GetInputGeneratorParam */


// ----------------------------------------------------------------------------
// GetImage - Busca uma nova imagem 
//
// Entrada: input - INPUT_DESC
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void GetImage(INPUT_DESC *input)
{
	int x, y, i, j;
	PARAM p;
	PARAM_LIST *pl;
	int nImageSource, isRightEye, isNextImage, nFrameSkip;
	char *linked_name, *seq_name , *filename;
	INPUT_DESC *linked_inp = NULL, *seq_inp = NULL;
	FILE *file;
	GLubyte pixel;
	mpeg2_rgb_frame_t *frame_struc;
	unsigned int ms;
	GLubyte *buffer;
	int width, height, components;
	int index_img, index_buf;
	char *c1, *c2, new_filename[256];

	nImageSource = (GetInputGeneratorParam (input, 1, &p) ? p.ival : -1);		// 1st parameter: image source
	isRightEye   = (GetInputGeneratorParam (input, 2, &p) ? p.ival :  0);		// 2nd parameter: image flag
	linked_name  = (GetInputGeneratorParam (input, 3, &p) ? p.sval :  NULL);	// 3rd parameter: linked input name
	if (strlen(linked_name))
	{
		linked_inp = get_input_by_name (linked_name);
		if (!linked_inp)
			Erro ("Missing linked stereo image: ", linked_name, ". Error in GetImage.");
	}
	
	switch (nImageSource)
	{
		case GET_IMAGE_FROM_SOCKET:
		{
			// Code suppressed from this software version.
		}
		break;

		case GET_IMAGE_FROM_PNM:
		{
			// Code suppressed from this software version.
		}
		break;

		case GET_IMAGE_TEST_STIMULUS:
		{
			// Code suppressed from this software version.
		}
		break;

		case GET_IMAGE_FROM_RBT:
		{
			filename = (GetInputGeneratorParam (input, 4, &p) ? p.sval : NULL);	// 4th parameter: source file name
			file = fopen(filename,"r");
			if (!file)
				Erro ("Cannot open RBT file ", filename, ". Error in GetImage.");

			// Store the new image into the MAE structure
			for (x = 0, i = 0; x < input->ww; x++)
			{
				for (y = 0; y < input->wh; y++)
				{
					if (!fread (&pixel, sizeof (GLubyte), 1, file))
						pixel = 0;
					input->image[3 * (x + y * input->tfw) + 0] = pixel;
					input->image[3 * (x + y * input->tfw) + 1] = pixel;
					input->image[3 * (x + y * input->tfw) + 2] = pixel;
				}
			}
			fclose(file);
		}
		break;

		case GET_IMAGE_FROM_JPEG:
		{
			filename = (GetInputGeneratorParam (input, 4, &p) ? p.sval : NULL);	// 4th parameter: source file name
			if (g_pursuit)
			{
				c1 = strrchr (input->image_file_name, '_');
				c2 = strrchr (input->image_file_name, '.');
				if (c1 != NULL && c2 != NULL && c2 > c1)
				{
					sprintf(new_filename, "%.*s_%d%s", strlen(input->image_file_name) - strlen(c1),
									   input->image_file_name, atoi(c1+1)+1, c2);
					strcpy (input->image_file_name, new_filename);
				}
				else
					fprintf(stderr, "JPEG filename is not in format XXX_999.jpg\n");
			}
			else
				strcpy (input->image_file_name, filename);
			printf("Opening file %s ...\n", input->image_file_name);
			file = fopen(input->image_file_name,"rb");
			if (!file)
				Erro ("Cannot open JPEG file ", input->image_file_name, ". Error in GetImage.");
			
			buffer = (GLubyte *) read_jpeg_file(file, &width, &height, &components);
			if (!buffer)
				Erro ("Cannot read JPEG file ", input->image_file_name, ". Error in GetImage.");

			for (y = 0; (y < input->wh) && (y < height); y++)
			{
				for (x = 0; (x < input->ww) && (x < width); x++)
				{
					index_img = 3 * (x + y * input->tfw);
					if (components == 3)
					{
						index_buf = 3 * (x + (height - 1 - y) * width);
						input->image[index_img + 0] = buffer[index_buf + 0];
						input->image[index_img + 1] = buffer[index_buf + 1];
						input->image[index_img + 2] = buffer[index_buf + 2];
					}
					else
					{
						index_buf = (x + (height - 1 - y) * width);
						input->image[index_img + 0] = buffer[index_buf];
						input->image[index_img + 1] = buffer[index_buf];
						input->image[index_img + 2] = buffer[index_buf];
					}
				}
			}
			free (buffer);
			fclose(file);
		}
		break;

		case GET_IMAGE_FROM_MPEG:
		{
			filename    = (GetInputGeneratorParam (input, 4, &p) ? p.sval : NULL);	// 4th parameter: source file name
			isNextImage = (GetInputGeneratorParam (input, 5, &p) ? p.ival : 0);	// 5th parameter: sequence flag
			seq_name    = (GetInputGeneratorParam (input, 6, &p) ? p.sval : NULL);	// 6th parameter: sequence input name
			if (strlen(seq_name))
			{
				seq_inp = get_input_by_name (seq_name);
				if (!seq_inp)
					Erro ("Missing linked motion image: ", seq_name, ". Error in GetImage.");
			}
			nFrameSkip  = (GetInputGeneratorParam (input, 7, &p) ? p.ival : 0);	// 7th parameter: frame skip
			frame_struc = (GetInputGeneratorParam (input, 8, &p) ? p.pval : NULL); 	// 8th parameter: MPEG frame structure

			if (!isNextImage)
			{
				if (!frame_struc)
				{
					// This is the first time GetImage is called, so we must create the MPEG frame structure
					pl = GetInputGeneratorParam (input, 8, &p);
					frame_struc = pl->param.pval = alloc_mem (sizeof (mpeg2_rgb_frame_t));
					strcpy (frame_struc->mpeg2filename, filename);
					open_mpeg_file(frame_struc);
					nFrameSkip++;
				}
				else
				{
					// This is not the first read, so just have to copy the image already stored in the next input
					for (y = 0; y < input->wh; y++)
						for (x = 0, i = (y * 3 * input->tfw); x < (3 * input->ww); x++)
							input->image[i + x] = seq_inp->image[i + x];
					if (verbose)
						printf("Input: %-20s  Frame: %d\n", input->name, frame_struc->num);
					break;
				}
			}
			else // isNextImage
			{
				// Get the 8th parameter from the previous input
				frame_struc = (GetInputGeneratorParam (seq_inp, 8, &p) ? p.pval : NULL);
				if (!frame_struc)
					Erro ("The NEXT image must come after the first: ", input->name, ". Error in GetImage.");
			}

			// Skip and read next MPEG frame
			for (i = 0; i < nFrameSkip; i++)
			{
				if (!read_mpeg_frame(frame_struc))
				{
					printf("MPEG EOF reached beyond frame # %d.\n", frame_struc->num);
					return;
				}
			}
			ms = (frame_struc->period / 27000.0 * frame_struc->num) + 0.5;
			printf ("Input: %-20s  Frame: %d [%02d:%02d:%02d.%03d]\n", input->name, frame_struc->num,
				(ms/3600000),((ms%3600000)/60000), ((ms%60000)/1000), (ms%1000));

			// Store the new image into the MAE structure
			for (y = 0; y < input->wh; y++)
				for (x = 0, i = (y * 3 * input->tfw), j = ((input->wh - 1 - y) * 3 * input->ww);
					x < (3 * input->ww); x++)
					input->image[i + x] = (GLubyte) frame_struc->buf[j + x];
		}
		break;

		default:
		{
			Erro ("Invalid image source", "", ". Error in GetImage.");
		}
	}
	
} /* end of GetImage */


// ----------------------------------------------------------------------------
// input_generator -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void input_generator (INPUT_DESC *input, int status)
{
	PARAM p;
	int isRight;
	char *linked_name, *seq_name;

	if (verbose > 1)
		printf("Input: %-20s  input_generator  status: %d  running: %d  remaining_steps: %d\n", 
			input->name, status, running, remaining_steps);

	// Inicializacao executada apenas uma vez por janela de entrada
	if (input->win == 0)
	{
		InitWindow (input);
		g_Status = ST_GET_IMAGE;
	}
	else
		g_Status = (remaining_steps ? ST_GET_IMAGE : ST_WAIT);

	// Busca a imagem
	if (g_Status == ST_GET_IMAGE)
	{
		GetImage(input);

		check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_neurons (input);
		glutSetWindow (input->win);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
		glutPostWindowRedisplay (input->win);

		if (!g_input)
		{
			g_input	     = input;
			g_isRight    = (GetInputGeneratorParam (input, 2, &p) ? p.ival : 0);	// 2nd parameter: image flag
			linked_name  = (GetInputGeneratorParam (input, 3, &p) ? p.sval : NULL);	// 3rd parameter: linked input name
			g_linked_inp = get_input_by_name (linked_name);
			g_isNext     = (GetInputGeneratorParam (input, 5, &p) ? p.ival : 0);	// 5th parameter: sequence flag
			seq_name     = (GetInputGeneratorParam (input, 6, &p) ? p.sval : NULL);	// 6th parameter: sequence input name
			g_seq_inp    = get_input_by_name (seq_name);
			g_Arrows = 0;
			printf("%s position: (%d,%d)\n", input->name, input->wxd, input->wyd);
			if (g_linked_inp)
				printf("%s position:  (%d,%d)\n", g_linked_inp->name, g_linked_inp->wxd, g_linked_inp->wyd);
			if (g_seq_inp)
				printf("%s position:  (%d,%d)\n", g_seq_inp->name, g_seq_inp->wxd, g_seq_inp->wyd);
		}
	}

	// Forca a atualizacao dos filtros quando eh trocado o ponto de atencao,
	// mas nao estah em modo running. So eh necessario quando for o olho
	// esquerdo pois, se mover o olho direito, o esquerdo move junto.
	isRight = (GetInputGeneratorParam (input, 2, &p) ? p.ival : 0);		// 2nd parameter: image flag
	if ((status == MOVE) && (!running) && (!isRight))
	{
		all_filters_update ();
		all_outputs_update ();
	}

} /* end of input_generator */


// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void input_controler (INPUT_DESC *input, int status)
{
	PARAM p;
	char *linked_name, *seq_name;

	if (verbose)
		printf("Input: %-20s  input_controler  status: %d\n", input->name, status);

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		g_input	     = input;
		g_isRight    = (GetInputGeneratorParam (input, 2, &p) ? p.ival : 0);		// 2nd parameter: image flag
		linked_name  = (GetInputGeneratorParam (input, 3, &p) ? p.sval : NULL);		// 3rd parameter: linked input name
		g_linked_inp = get_input_by_name (linked_name);
		g_isNext     = (GetInputGeneratorParam (input, 5, &p) ? p.ival : 0);		// 5th parameter: sequence flag
		seq_name     = (GetInputGeneratorParam (input, 6, &p) ? p.sval : NULL);		// 6th parameter: sequence input name
		g_seq_inp    = get_input_by_name (seq_name);
		g_Arrows = 0;

		if (g_isRight)
		{
			if (g_linked_inp)
			{
				g_linked_inp->wxd += input->wxd - input->wxd_old;
				g_linked_inp->wyd  = input->wyd + g_nVerticalGap;
				printf("%s position:  (%d,%d)\n", g_linked_inp->name, g_linked_inp->wxd, g_linked_inp->wyd);
				move_input_window (g_linked_inp->name, g_linked_inp->wxd, g_linked_inp->wyd);
				glutPostWindowRedisplay (g_linked_inp->win);
			}
		}
		else /* is Left */
		{
			if (g_linked_inp)
			{
				glutPostWindowRedisplay (g_linked_inp->win);
				input->wyd = g_linked_inp->wyd + g_nVerticalGap;
			}
		}

		printf("%s position: (%d,%d)\n", input->name, input->wxd, input->wyd);
		move_input_window (input->name, input->wxd, input->wyd);
		glutPostWindowRedisplay (input->win);

		if (!g_isNext)
		{
			if (g_seq_inp)
			{
				g_seq_inp->wxd  = input->wxd;
				g_seq_inp->wyd  = input->wyd;
				printf("%s position:  (%d,%d)\n", g_seq_inp->name, g_seq_inp->wxd, g_seq_inp->wyd);
				move_input_window (g_seq_inp->name, g_seq_inp->wxd, g_seq_inp->wyd);
				glutPostWindowRedisplay (g_seq_inp->win);
			}
		}
	}

	input->mouse_button = -1;
	
} /* end of input_controler */


// ----------------------------------------------------------------------------
// output_handler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	FILTER_DESC *filter_desc;

	if (verbose > 1)
		printf("Output: %-20s  output_controler  type_call: %d\n", output->name, type_call);

	if ((move_active == 1) &&
	    (output->mouse_button == GLUT_LEFT_BUTTON) &&
	    (output->mouse_state == GLUT_DOWN))
	{
		filter_desc = get_filter_by_output (output->neuron_layer);
		if ((filter_desc) && (filter_desc->filter == &robot_motion_area_filter))
		{
			filter_update (filter_desc);
			glutPostWindowRedisplay (output->win);
		}
	}

	output->mouse_button = -1;

} /* end of output_handler */


// ----------------------------------------------------------------------------
// draw_output -
//
// Entrada:
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void draw_output (char *output_name, char *input_name)
{
	if (verbose)
		printf("Output: %s - Input: %s  draw_output\n", output_name, input_name);

} /* end of draw_output */


// ----------------------------------------------------------------------------
// LoadCameraParams - Loads up the cameras parameters and calibration constants from the data file
//
// Entrada: 
//
// Saida: 
// ----------------------------------------------------------------------------
void LoadCameraParams()
{
	FILE *data_fd;
	
	data_fd = fopen (CAMERA_LEFT_PARAMETERS_FILE_NAME, "r");
	load_cp_cc_data (data_fd, &cameraLeftCalibrationParameters, &cameraLeftCalibrationConstants);
	fclose (data_fd);
			
	data_fd = fopen (CAMERA_RIGHT_PARAMETERS_FILE_NAME, "r");
	load_cp_cc_data (data_fd, &cameraRightCalibrationParameters, &cameraRightCalibrationConstants);
	fclose (data_fd);	
	printf("Left and right camera parameters and calibration constants loaded up.\n");
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
	char strCommand[128];

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	g_DispData_list = (DISP_DATA_LIST *) alloc_mem (sizeof (DISP_DATA_LIST));
	g_DispData_list->DispData = NULL;
	g_DispData_list->next = NULL;

	g_AllDispData_list = (DISP_DATA_LIST *) alloc_mem (sizeof (DISP_DATA_LIST));
	g_AllDispData_list->DispData = NULL;
	g_AllDispData_list->next = NULL;

	g_AllDispDataProp_list = (DISP_DATA_LIST *) alloc_mem (sizeof (DISP_DATA_LIST));
	g_AllDispDataProp_list->DispData = NULL;
	g_AllDispDataProp_list->next = NULL;

	LoadCameraParams();
	printf ("\nType 'h' for help\n");
	return (0);
}


/*!
**********************************************************************************************
* Function:	GetComplexMonoNeuronLayerByImage
* Description:	Find a complex mono neuron layer corresponding to the given input neuron layer
* Inputs:	nl_input : given input neuron layer
* Output:	pointer to the desired neuron layer or NULL
**********************************************************************************************
*/
NEURON_LAYER *GetComplexMonoNeuronLayerByImage (NEURON_LAYER *nl_input)
{
	FILTER_LIST *fl;
	NEURON_LAYER *nl_simple = NULL;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &biological_gabor_filter) 
			if (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_input)
			{
				nl_simple = fl->filter_descriptor->output;
				break;
			}
	}
	if (nl_simple == NULL)
		return NULL;
	
	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_complex_cell) 
			if ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_simple) ||
			     (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_simple) )
			{
				return fl->filter_descriptor->output;
			}
	}
	return NULL;

} /* end of GetComplexMonoNeuronLayerByImage */


/*!
*****************************************************************************************
* Function:	GetMTNeuronLayerByImages
* Description:	Find a MT neuron layer corresponding to the given pair of images
* Inputs:	nl_1 , nl_2 : image neuron layers
* Output:	pointer to the desired neuron layer or NULL
*****************************************************************************************
*/
NEURON_LAYER *GetMTNeuronLayerByImages (NEURON_LAYER *nl_1, NEURON_LAYER *nl_2)
{
	FILTER_LIST *fl;
	NEURON_LAYER *nl_complex_1, *nl_complex_2;

	nl_complex_1 = GetComplexMonoNeuronLayerByImage (nl_1);
	nl_complex_2 = GetComplexMonoNeuronLayerByImage (nl_2);
	if ((nl_complex_1 == NULL) || (nl_complex_2 == NULL))
		return NULL;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_mt_cell)
		{
			if ( ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_complex_1) ||
			       (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_complex_1) )
			     &&
			     ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_complex_2) ||
			       (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_complex_2) ) )
			{
				return fl->filter_descriptor->output;
			}
		}
	}
	return NULL;
	
} /* end of GetMTNeuronLayerByImages */


/*!
*****************************************************************************************
* Function:	GetMTGaussianNeuronLayerByImages
* Description:	Find a MT Gaussian neuron layer corresponding to the given pair of images
* Inputs:	nl_1 , nl_2 : image neuron layers
* Output:	pointer to the desired neuron layer or NULL
*****************************************************************************************
*/
NEURON_LAYER *GetMTGaussianNeuronLayerByImages (NEURON_LAYER *nl_1, NEURON_LAYER *nl_2)
{
	FILTER_LIST *fl;
	NEURON_LAYER *nl_mt;

	nl_mt = GetMTNeuronLayerByImages (nl_1, nl_2);
	if (nl_mt == NULL)
		return NULL;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_gaussian_filter)
			if (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_mt)
			{
				return fl->filter_descriptor->output;
			}
	}
	return NULL;

} /* end of GetMTGaussianNeuronLayerByImages */


/*!
*****************************************************************************************
* Function:	GetConfidenceNeuronLayerByImages
* Description:	Find a Confidence neuron layer corresponding to the given pair of images
* Inputs:	nl_1 , nl_2 : image neuron layers
* Output:	pointer to the desired neuron layer or NULL
*****************************************************************************************
*/
NEURON_LAYER *GetConfidenceNeuronLayerByImages (NEURON_LAYER *nl_1, NEURON_LAYER *nl_2)
{
	FILTER_LIST *fl;
	NEURON_LAYER *nl_complex_1, *nl_complex_2;

	nl_complex_1 = GetComplexMonoNeuronLayerByImage (nl_1);
	nl_complex_2 = GetComplexMonoNeuronLayerByImage (nl_2);
	if ((nl_complex_1 == NULL) || (nl_complex_2 == NULL))
		return NULL;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_sum_filter)
		{
			if ( ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_complex_1) ||
			       (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_complex_1) )
			     &&
			     ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_complex_2) ||
			       (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_complex_2) ) )
			{
				return fl->filter_descriptor->output;
			}
		}
	}
	return NULL;

} /* end of GetConfidenceNeuronLayerByImages */


/*!
*****************************************************************************************
* Function:	GetMapNeuronLayer
* Description:	Find a Map neuron layer corresponding to the given neuron layer and type
* Inputs:	nl_input : preceding neuron layer , map_type 
* Output:	pointer to the desired neuron layer or NULL
*****************************************************************************************
*/
NEURON_LAYER *GetMapNeuronLayer (NEURON_LAYER *nl_input, int map_type)
{
	FILTER_LIST *fl;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_map_filter)
		{
			if ( (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_input) &&
			     (fl->filter_descriptor->filter_params->next->param.ival == map_type) )
			{
				return fl->filter_descriptor->output;
			}
		}
	}
	return NULL;

} /* end of GetMapNeuronLayer */


/*!
****************************************************************************************************
* Function:	GetMTGaussianNeuronLayerByMap
* Description:	Find a MT Gaussian neuron layer corresponding to the given map neuron layer and type
* Inputs:	nl_input : preceding neuron layer , map_type 
* Output:	pointer to the desired neuron layer or NULL
****************************************************************************************************
*/
NEURON_LAYER *GetMTGaussianNeuronLayerByMap (NEURON_LAYER *nl_map, int map_type)
{
	FILTER_LIST *fl;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_map_filter)
		{
			if ( (fl->filter_descriptor->output == nl_map) &&
			     (fl->filter_descriptor->filter_params->next->param.ival == map_type) )
			{
				return fl->filter_descriptor->neuron_layer_list->neuron_layer;
			}
		}
	}
	return NULL;

} /* end of GetMTGaussianNeuronLayerByMap */


/*!
******************************************************************************************
* Function:	GetIlluminanceFilterByNeuronLayer
* Description:	Find the illuminance filter corresponding to the given neuron layer 
* Inputs:	nl_output : output neuron layer  
* Output:	pointer to the illuminance filter or NULL
******************************************************************************************
*/
FILTER_DESC *GetIlluminanceFilterByNeuronLayer (NEURON_LAYER *nl_output)
{
	FILTER_LIST *fl;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_illuminance_filter)
		{
			if (fl->filter_descriptor->output == nl_output)
			{
				return fl->filter_descriptor;
			}
		}
	}
	return NULL;

} /* end of GetIlluminanceFilterByNeuronLayer */


/*!
******************************************************************************************
* Function:	GetReflectanceFilterByNeuronLayer
* Description:	Find the reflectance filter corresponding to the given neuron layer 
* Inputs:	nl_output : output neuron layer  
* Output:	pointer to the reflectance filter or NULL
******************************************************************************************
*/
FILTER_DESC *GetReflectanceFilterByNeuronLayer (NEURON_LAYER *nl_output)
{
	FILTER_LIST *fl;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_reflectance_filter)
		{
			if (fl->filter_descriptor->output == nl_output)
			{
				return fl->filter_descriptor;
			}
		}
	}
	return NULL;

} /* end of GetReflectanceFilterByNeuronLayer */


/*!
******************************************************************************************
* Function:	GetReflectanceFilterByIlluminance
* Description:	Find the reflectance filter corresponding to the given illuminance filter 
* Inputs:	illum : pointer to the illuminance filter  
* Output:	pointer to the reflectance filter or NULL
******************************************************************************************
*/
FILTER_DESC *GetReflectanceFilterByIlluminance (FILTER_DESC *illum)
{
	FILTER_LIST *fl;
	NEURON_LAYER *nl_input, *nl_illum;

	nl_input = illum->neuron_layer_list->neuron_layer;
	nl_illum = illum->output;

	for (fl = global_filter_list; fl != NULL; fl = fl->next)
	{
		if (fl->filter_descriptor->filter == &robot_reflectance_filter)
		{
			if (  (fl->filter_descriptor->neuron_layer_list->neuron_layer == nl_input)
			   && (fl->filter_descriptor->neuron_layer_list->next->neuron_layer == nl_illum) )
			{
				return fl->filter_descriptor;
			}
		}
	}
	return NULL;

} /* end of GetReflectanceFilterByIlluminance */


/*!
*********************************************************************************************
* Function:	GetDispData
* Description:	Find a DISP_DATA pointer corresponding to the given MT Gaussian neuron layer
*		in the given list
* Inputs:	nl_input : MT Gaussian neuron layer
*		DispData_list : list of DISP_DATA pointers
* Output:	pointer to the desired DISP_DATA or NULL
********************************************************************************************
*/
DISP_DATA *GetDispData (NEURON_LAYER *nl_input, DISP_DATA_LIST *DispData_list)
{
	DISP_DATA_LIST *ddl;

	for (ddl = DispData_list; ddl->DispData != NULL; ddl = ddl->next)
	{
		if (ddl->DispData->mt_gaussian == nl_input)
		{
			return ddl->DispData;
		}
	}
	return NULL;

} /* end of GetDispData */


/*!
********************************************************************************************
* Function:	CreateDispData
* Description:	Create a new instance of a DISP_DATA structure
* Inputs:	DispData_list : pointer to DISP_DATA list structure
*		num_samples , num_neurons : the two dimensions of DispData->samples
* Output:	pointer to the new DISP_DATA structure or NULL
********************************************************************************************
*/
DISP_DATA *CreateDispData (DISP_DATA_LIST *DispData_list, int num_samples, int num_neurons)
{
	DISP_DATA_LIST *ddl;
	int i;

	for (ddl = DispData_list; ddl->DispData != NULL; ddl = ddl->next);

	ddl->DispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
	ddl->DispData->num_samples = num_samples;
	ddl->DispData->num_neurons = num_neurons;
	ddl->DispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * num_samples);
	for (i = 0; i < num_samples; i++)
		ddl->DispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * num_neurons);

	ddl->next = (DISP_DATA_LIST *) alloc_mem (sizeof (DISP_DATA_LIST));
	ddl->next->DispData = NULL;
	ddl->next->next = NULL;

	return ddl->DispData;

} /* end of CreateDispData */


/*!
*********************************************************************************************
* Function:	InitDispDataState
* Description:	Initialise the state of a DISP_DATA structure that uses local mimimum samples
* Inputs:	DispData : pointer to DISP_DATA structure
* Output:	-
*********************************************************************************************
*/
void InitDispDataState (DISP_DATA *DispData)
{
	int num_samples, num_neurons;
	int s, i;

	num_samples = DispData->num_samples;
	num_neurons = DispData->num_neurons;

	if (g_fltDispAux == NULL)
	{
		g_fltDispAux = (float *) alloc_mem (sizeof (float) * num_neurons);
		g_fltConfAux = (float *) alloc_mem (sizeof (float) * num_neurons);
		g_neg_slope  = (char *)  alloc_mem (num_neurons);
	}

	for (i = 0; i < num_neurons; i++)
	{
		g_fltDispAux[i] = FLT_MAX;
		g_fltConfAux[i] = 0.0;
		g_neg_slope[i]  = 1;
	}

	for (s = 0; s < num_samples; s++)
	{
		for (i = 0; i < num_neurons; i++)
		{
			DispData->samples[s][i].val  = FLT_MAX;
			DispData->samples[s][i].conf = 0.0;
			DispData->samples[s][i].pos  = 0;
		}
	}

} /* end of InitDispDataState */


/*!
********************************************************************************************
* Function:	AddLocalMin
* Description:	Add a local minimum sample to a DISP_DATA structure
* Inputs:	DispData : pointer to DISP_DATA structure
*		i : neuron index , (val, conf, pos) : sample data to be added
* Output:	-
********************************************************************************************
*/
void AddLocalMin (DISP_DATA *DispData, int i, double val, double conf, int pos)
{
	int s = DispData->num_samples - 1;
	
	if (val >= DispData->samples[s][i].val)
		return;
		
	for (s = s; (s > 0) && (val < DispData->samples[s - 1][i].val); s--)
		DispData->samples[s][i] = DispData->samples[s - 1][i];

	DispData->samples[s][i].val  = val;
	DispData->samples[s][i].conf = conf;
	DispData->samples[s][i].pos  = pos;

} /* end of AddLocalMin */


/*!
******************************************************************************************
* Function:	set_stereo_vergence
* Description:	Determine the horizontal vergence of two stereo images (left & right eyes)
* Inputs:	DispData with the following pointers:
			img_l , img_r : left & right image inputs
*			nl_mtg : MT Gaussian neuron layer 
*			nl_conf : Confidence neuron layer
* Output:	horizontal vergence, i.e, the value of img_l->wxd that matches img_r->wxd
*		updated global structures DispData and AllDispData  
*		with the minimum MT Gaussian values and corresponding Confidence and wxd 
******************************************************************************************
*/
void set_stereo_vergence (DISP_DATA *DispData, DISP_DATA *AllDispData)
{
	NEURON_LAYER *nl_mtg, *nl_conf;
	INPUT_DESC *img_l, *img_r;
	FILE *file_verg = NULL, *file_mt_cell = NULL;
	char strAux[64];
	int i, x, y, nIndex;
	int wo, ho;
	int nMaxScan, nMinScan;
	float fltSumOutputCells, fltMinSum;
	float fltDispAux, fltConfAux;
	int nVergLeft = 0;
	
	nl_mtg  = DispData->mt_gaussian;
	nl_conf = DispData->confidence;
	img_l   = DispData->left;
	img_r   = DispData->right;

	printf ("set_stereo_vergence: %s - %s (%d, %d)\n", img_l->name, img_r->name, img_r->wxd, img_r->wyd);

	if (verbose)
	{
		file_verg = fopen(FILE_PLOT_VERGENCE,"w");
		file_mt_cell = fopen(FILE_PLOT_MT_CELL,"w");
		if (!file_verg || !file_mt_cell)
			Erro ("Could not open plot files", "", ". Error in set_stereo_vergence.");
	}

	wo = nl_mtg->dimentions.x;
	ho = nl_mtg->dimentions.y;
	nMinScan = img_r->wxd;
	nMaxScan = img_r->wxd + (img_r->neuron_layer->dimentions.x / 5);
	if (nMaxScan > img_r->neuron_layer->dimentions.x)
		nMaxScan = img_r->neuron_layer->dimentions.x;

	fltMinSum = FLT_MAX;
	
	for (img_l->wxd = nMinScan; img_l->wxd < nMaxScan; img_l->wxd++)
	{
		move_input_window (img_l->name, img_l->wxd, img_l->wyd);

		fltSumOutputCells = 0.0;
		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				nIndex = y * wo + x;
				fltDispAux = nl_mtg->neuron_vector[nIndex].output.fval;
				fltConfAux = (nl_conf ? nl_conf->neuron_vector[nIndex].output.fval : 0.0);
				
				/* Compare the current data with the previous xy_plane_disparity iteration */
				if (fltDispAux >= g_fltDispAux[nIndex])
				{
					if (g_neg_slope[nIndex])
					{
						/* The previous slope was negative but now it is positive, 
						   so the last xy_plane_disparity iteration was a local minimum */
						AddLocalMin (DispData, nIndex, 
							     g_fltDispAux[nIndex], g_fltConfAux[nIndex], img_l->wxd - 1);
					}
					g_neg_slope[nIndex] = 0;
				}
				else // fltDispAux < g_fltDispAux[nIndex]
				{
					if (img_l->wxd == nMaxScan - 1)
					{
						/* This is the last xy_plane_disparity iteration, so take it as a local minimum */
						AddLocalMin (DispData, nIndex, fltDispAux, fltConfAux, img_l->wxd);
					}
					g_neg_slope[nIndex] = 1;
				}

				/* Accumulate all MT output values in the xy_plane corresponding to disparity (left - right) wxd */
				fltSumOutputCells += fltDispAux;

				/* Save current data to compare with the next xy_plane_disparity iteration */
				g_fltDispAux[nIndex] = fltDispAux;
				g_fltConfAux[nIndex] = fltConfAux;
				
				/* Save data into a 3D cube with the dimensions x_y_disparity */
				AllDispData->samples[img_l->wxd - nMinScan][nIndex].val  = fltDispAux;
				AllDispData->samples[img_l->wxd - nMinScan][nIndex].conf = fltConfAux;
				AllDispData->samples[img_l->wxd - nMinScan][nIndex].pos  = img_l->wxd;
			} /* end of for (y < ho) */
		} /* end of for (x < wo) */

		/* The Vergence Disparity xy plane will be the one with the least total accumulated MT output value */
		if (fltSumOutputCells < fltMinSum)
		{
			fltMinSum = fltSumOutputCells;
			nVergLeft = img_l->wxd;
		}


		if (verbose)
		{
			// plot MT_CELL
			sprintf(strAux, "%d %.3f\n", img_l->wxd, nl_mtg->neuron_vector[INDEX_MT_CELL].output.fval);
			StrReplaceChar(strAux, ',', '.');
			fprintf (file_mt_cell, "%s", strAux);

			// plot VERGENCE
			sprintf(strAux, "%d %.3f\n", img_l->wxd, fltSumOutputCells);
			StrReplaceChar(strAux, ',', '.');
			fprintf (file_verg, "%s", strAux);
		}
	} /* end of for (img_l->wxd < nMaxScan) */

	if (verbose)
	{
		fclose(file_verg);
		fclose(file_mt_cell);

		// Imprime os minimos armazenados na MT CELL que foi plotada.
		for (i = 0; i < DispData->num_samples; i++)
			printf ("Min %d - mt_cell[%d] - pos:%d - val: %.3f - conf: %.3f\n", i, INDEX_MT_CELL,
				 DispData->samples[i][INDEX_MT_CELL].pos, DispData->samples[i][INDEX_MT_CELL].val,
				 DispData->samples[i][INDEX_MT_CELL].conf);
	}

	/* Update DispData and AllDispData */
	DispData->left_vergence = AllDispData->left_vergence = nVergLeft;

	/* Display result */
	printf("Left Vergence: %d\n", nVergLeft);
	img_l->wxd = nVergLeft;
	move_input_window (img_l->name, img_l->wxd, img_l->wyd);
	glutPostWindowRedisplay (img_l->win);

} /* end of set_stereo_vergence */


/*!
******************************************************************************************
* Function:	SetStereoVergence
* Description:	Determine the horizontal vergence of two stereo images (left & right eyes)
*		and determine corresponding maps (disparity, MT Gaussian and Confidence)
* Inputs:	last clicked input window
* Output:	updated input windows with horizontal vergence
*		updated global structures DispData[NUM_SAMPLES] and AllDispData[ww/5]  
*		updated map neuron layers (disparity, MT Gaussian and Confidence)
******************************************************************************************
*/
void SetStereoVergence ()
{
	int i, wo, ho;
	int nMaxScan, nMinScan;
	INPUT_DESC *img_l, *img_r;
	NEURON_LAYER *nl_mtg, *nl_conf, *nl_disp, *nl_mtmap, *nl_cmap;
	OUTPUT_DESC *out_disp, *out_mtmap, *out_cmap;
	DISP_DATA *DispData, *AllDispData;

	img_l = (g_isRight ? g_linked_inp : g_input);
	img_r = (g_isRight ? g_input : g_linked_inp);

	if (!img_l || !img_r)
	{
		if (!g_input)
			printf ("Click on an input to select it for stereo vergence.\n");
		else 		
			printf ("%s needs a stereo image for vergence. Check the input generator parameters.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_conf  = GetConfidenceNeuronLayerByImages (img_l->neuron_layer, img_r->neuron_layer);
	nl_mtg	 = GetMTGaussianNeuronLayerByImages (img_l->neuron_layer, img_r->neuron_layer);
	if (!nl_mtg)
	{
		printf ("%s needs a MT Gaussian neuron layer for stereo vergence.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	wo = nl_mtg->dimentions.x;
	ho = nl_mtg->dimentions.y;
	nMinScan = img_r->wxd;
	nMaxScan = img_r->wxd + (img_r->neuron_layer->dimentions.x / 5);
	if (nMaxScan > img_r->neuron_layer->dimentions.x)
		nMaxScan = img_r->neuron_layer->dimentions.x;

	DispData = GetDispData (nl_mtg, g_DispData_list);
	if (DispData == NULL)
		DispData = CreateDispData (g_DispData_list, NUM_SAMPLES, wo * ho);
	
	AllDispData = GetDispData (nl_mtg, g_AllDispData_list);
	if (AllDispData == NULL)
		AllDispData = CreateDispData (g_AllDispData_list, (img_r->neuron_layer->dimentions.x / 5), wo * ho);
	
	/* Initialise DispData and auxiliary structures */
	InitDispDataState (DispData);

	/* Update DispData and AllDispData */
	DispData->mt_gaussian	= AllDispData->mt_gaussian = nl_mtg;
	DispData->confidence	= AllDispData->confidence  = nl_conf;
	DispData->left  	= AllDispData->left	   = img_l;
	DispData->right		= AllDispData->right	   = img_r;
	DispData->wxd		= AllDispData->wxd	   = img_r->wxd;
	DispData->wyd		= AllDispData->wyd	   = img_r->wyd;
	DispData->max_sample	= DispData->num_samples;
	AllDispData->max_sample	= (nMaxScan - nMinScan);

	set_stereo_vergence (DispData, AllDispData);

	nl_disp  = GetMapNeuronLayer (nl_mtg, DISPARITY_MAP);
	nl_mtmap = GetMapNeuronLayer (nl_mtg, MT_GAUSSIAN_MAP);
	nl_cmap  = GetMapNeuronLayer (nl_mtg, CONFIDENCE_MAP);

	/* For each point set the disparity corresponding to the least total accumulated MT output value */
	for (i = 0; i < (wo * ho); i++)
	{
		if (nl_disp)	nl_disp->neuron_vector[i].output.fval  = DispData->samples[0][i].pos - DispData->left_vergence;
		if (nl_mtmap)	nl_mtmap->neuron_vector[i].output.fval = DispData->samples[0][i].val;
		if (nl_cmap)	nl_cmap->neuron_vector[i].output.fval  = DispData->samples[0][i].conf;
	}

	out_disp  = get_output_by_neural_layer (nl_disp);
	out_mtmap = get_output_by_neural_layer (nl_mtmap);
	out_cmap  = get_output_by_neural_layer (nl_cmap);
	if (out_disp)	glutPostWindowRedisplay (out_disp->win);
	if (out_mtmap)	glutPostWindowRedisplay (out_mtmap->win);
	if (out_cmap)	glutPostWindowRedisplay (out_cmap->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of SetStereoVergence */


int MinSample (DISP_DATA *AllDispData, int i, int max_sample)
{
	int s; 
	int min_s = 0;
	float min_val = AllDispData->samples[0][i].val;

	for (s = 1; s < max_sample; s++)
		if (AllDispData->samples[s][i].val < min_val)
		{
			min_val = AllDispData->samples[s][i].val;
			min_s = s;
		}

	return min_s;
}


/*!
***********************************************************************************************
* Function:	set_motion_vergence
* Description:	Determine the motion vergence of two sequenced images (current & next)
* Inputs:	DispData with the following pointers:
			img_r , img_l : current & next image inputs
*			nl_mtg : MT Gaussian neuron layer 
*			nl_conf : Confidence neuron layer
* Output:	motion vergence, i.e, the value of img_l->wxd,wyd that matches img_r->wxd,wyd
*		updated global structures DispData and AllDispData  
*		with the minimum MT Gaussian values and corresponding Confidence and wxd,wyd 
**********************************************************************************************
*/
void set_motion_vergence (DISP_DATA *DispData, DISP_DATA *AllDispData)
{
	NEURON_LAYER *nl_mtg, *nl_conf;
	INPUT_DESC *img_1, *img_2;
	FILE *file_verg = NULL, *file_mt_cell = NULL;
	char strAux[64];
	int x, y, nIndex, pos;
	int ind_directions, ind_radius;
	const int horiz[] = MOTION_HORIZ;
	const int vert[]  = MOTION_VERT;
	int wo, ho;
	float fltSumOutputCells, fltMinSum;
	float fltDispAux, fltConfAux;
	int nVergence = 0;
	
	nl_mtg  = DispData->mt_gaussian;
	nl_conf = DispData->confidence;
	img_1   = DispData->current;
	img_2   = DispData->next;

	printf ("set_motion_vergence: %s - %s (%d, %d)\n", img_1->name, img_2->name, img_1->wxd, img_1->wyd);

	if (verbose)
	{
		file_verg = fopen(FILE_PLOT_VERGENCE,"w");
		file_mt_cell = fopen(FILE_PLOT_MT_CELL,"w");
		if (!file_verg || !file_mt_cell)
			Erro ("Could not open plot files", "", ". Error in set_stereo_vergence.");
	}

	wo = nl_mtg->dimentions.x;
	ho = nl_mtg->dimentions.y;

	fltMinSum = FLT_MAX;
	pos = -1;
	
	for (ind_radius = 0; ind_radius <= MOTION_MAX_RADIUS; ind_radius += MOTION_STEP)
	{
		for (ind_directions = 0; ind_directions < MOTION_DIRECTIONS; ind_directions++)
		{
			img_2->wxd = img_1->wxd + horiz[ind_directions] * ind_radius;
			img_2->wyd = img_1->wyd + vert [ind_directions] * ind_radius;
			pos++;
			
			if ((img_2->wxd < 0) || (img_2->wxd >= img_2->neuron_layer->dimentions.x) ||
			    (img_2->wyd < 0) || (img_2->wyd >= img_2->neuron_layer->dimentions.y)  )
			{
				/* Out of range */
				for (y = 0; y < ho; y++)
				{
					for (x = 0; x < wo; x++)
					{
						nIndex = y * wo + x;
						AllDispData->samples[pos][nIndex].val  = FLT_MAX;
						AllDispData->samples[pos][nIndex].conf = 0.0;
						AllDispData->samples[pos][nIndex].pos  = img_2->wxd;
						AllDispData->samples[pos][nIndex].vpos = img_2->wyd;
					}
				}
				continue;
			}

			move_input_window (img_2->name, img_2->wxd, img_2->wyd);

			fltSumOutputCells = 0.0;
			for (y = 0; y < ho; y++)
			{
				for (x = 0; x < wo; x++)
				{
					nIndex = y * wo + x;
					fltDispAux = nl_mtg->neuron_vector[nIndex].output.fval;
					fltConfAux = (nl_conf ? nl_conf->neuron_vector[nIndex].output.fval : 0.0);

					/* Save data into a 3D cube with the dimensions x_y_disparity */
					AllDispData->samples[pos][nIndex].val  = fltDispAux;
					AllDispData->samples[pos][nIndex].conf = fltConfAux;
					AllDispData->samples[pos][nIndex].pos  = img_2->wxd;
					AllDispData->samples[pos][nIndex].vpos = img_2->wyd;

					/* Accumulate all MT output values in the xy_plane corresponding to disparity
					   only for those cells of the moving area  */
					if (AllDispData->samples[0][nIndex].val >= THRESHOLD)
						fltSumOutputCells += fltDispAux;

					if (verbose)
					{
						if ((img_2->wxd == img_1->wxd) && (img_2->wyd == img_1->wyd))
						{
							/* plot zero speed xy_plane */
							sprintf(strAux, "%d %d %.6f\n", x, y, fltDispAux);
							StrReplaceChar(strAux, ',', '.');
							fprintf (file_mt_cell, "%s", strAux);
							if (x == (wo - 1))
								fprintf (file_mt_cell, "\n");
						}
					}

				} /* end of for (x < wo) */
			} /* end of for (y < ho) */
			
			/* The Vergence Disparity xy plane will be the one with the least total accumulated MT output value */
			if (fltSumOutputCells < fltMinSum)
			{
				fltMinSum = fltSumOutputCells;
				nVergence = pos;
			}

			if (verbose)
			{
				printf("%2d [%3d, %3d]: (%d, %d) SumOutputCells = %f\n", 
					pos, horiz[ind_directions] * ind_radius, vert[ind_directions] * ind_radius, 
					img_2->wxd, img_2->wyd, fltSumOutputCells);

				/* plot VERGENCE */
				sprintf(strAux, "%d %d %.3f\n", img_2->wxd, img_2->wyd, fltSumOutputCells);
				StrReplaceChar(strAux, ',', '.');
				fprintf (file_verg, "%s", strAux);
				if ((ind_directions == (MOTION_DIRECTIONS - 1)) || (ind_radius == 0))
					fprintf (file_verg, "\n");
			}
			
			if (ind_radius == 0)	/* No need to check all directions when radius is zero */
				break;

		} /* end of for (ind_directions < MOTION_DIRECTIONS) */
	} /* end of for (ind_radius <= MOTION_MAX_RADIUS) */

	if (verbose)
	{
		fclose(file_verg);
		fclose(file_mt_cell);
	}

	/* Update DispData and AllDispData */
	DispData->motion_vergence = AllDispData->motion_vergence = nVergence;

	/* Display result */
	img_2->wxd = AllDispData->samples[nVergence][0].pos;
	img_2->wyd = AllDispData->samples[nVergence][0].vpos;
	printf("Moving area average speed: Vx=%d , Vy=%d\n", img_2->wxd - img_1->wxd, img_2->wyd - img_1->wyd);
	g_pursuit = 1;
	g_next_x1 = img_2->wxd;
	g_next_y1 = img_2->wyd;
	g_next_x2 = img_2->wxd + (img_2->wxd - img_1->wxd);
	g_next_y2 = img_2->wyd + (img_2->wyd - img_1->wyd);

} /* end of set_motion_vergence */


/*!
******************************************************************************************
* Function:	SetMotionVergence
* Description:	Determine the motion vergence of two sequenced images (current & next)
*		and determine corresponding maps (disparity, MT Gaussian and Confidence)
* Inputs:	last clicked input window
* Output:	updated input windows with motion vergence
*		updated global structures DispData[NUM_SAMPLES] and AllDispData[ww/5]  
*		updated map neuron layers (disparity, MT Gaussian and Confidence)
******************************************************************************************
*/
void SetMotionVergence ()
{
	int i, wo, ho, s;
	INPUT_DESC *img_1, *img_2;
	NEURON_LAYER *nl_mtg, *nl_conf, *nl_disp, *nl_vdisp, *nl_mtmap, *nl_cmap;
	OUTPUT_DESC *out_disp, *out_vdisp, *out_mtmap, *out_cmap;
	DISP_DATA *DispData, *AllDispData;

	img_1 = (g_isNext ? g_seq_inp : g_input);
	img_2 = (g_isNext ? g_input : g_seq_inp);

	if (!img_1 || !img_2)
	{
		if (!g_input)
			printf ("Click on an input to select it for motion vergence.\n");
		else 		
			printf ("%s needs a motion sequence image for vergence. "
				"Check the input generator parameters.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_conf  = GetConfidenceNeuronLayerByImages (img_1->neuron_layer, img_2->neuron_layer);
	nl_mtg	 = GetMTGaussianNeuronLayerByImages (img_1->neuron_layer, img_2->neuron_layer);
	if (!nl_mtg)
	{
		printf ("%s needs a MT Gaussian neuron layer for motion vergence.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	wo = nl_mtg->dimentions.x;
	ho = nl_mtg->dimentions.y;

	DispData = GetDispData (nl_mtg, g_DispData_list);
	if (DispData == NULL)
		DispData = CreateDispData (g_DispData_list, NUM_SAMPLES, wo * ho);
	
	AllDispData = GetDispData (nl_mtg, g_AllDispData_list);
	if (AllDispData == NULL)
		AllDispData = CreateDispData (g_AllDispData_list, MOTION_MAX_SAMPLES, wo * ho);
	
	/* Initialise DispData and auxiliary structures */
	InitDispDataState (DispData);

	/* Update DispData and AllDispData */
	DispData->mt_gaussian	= AllDispData->mt_gaussian = nl_mtg;
	DispData->confidence	= AllDispData->confidence  = nl_conf;
	DispData->current	= AllDispData->current	   = img_1;
	DispData->next  	= AllDispData->next	   = img_2;
	DispData->wxd		= AllDispData->wxd	   = img_1->wxd;
	DispData->wyd		= AllDispData->wyd	   = img_1->wyd;
	DispData->max_sample	= DispData->num_samples;
	AllDispData->max_sample	= AllDispData->num_samples;
	g_Arrows = 0;

	set_motion_vergence (DispData, AllDispData);

	nl_disp  = GetMapNeuronLayer (nl_mtg, DISPARITY_MAP);
	nl_vdisp = GetMapNeuronLayer (nl_mtg, VDISPARITY_MAP);
	nl_mtmap = GetMapNeuronLayer (nl_mtg, MT_GAUSSIAN_MAP);
	nl_cmap  = GetMapNeuronLayer (nl_mtg, CONFIDENCE_MAP);

	/* For each point set the disparity corresponding to the least total accumulated MT output value */
	for (i = 0; i < (wo * ho); i++)
	{
		/* Only for those cells of the moving area  */
		if (AllDispData->samples[0][i].val >= THRESHOLD)
		{
			s = MinSample (AllDispData, i, MOTION_MAX_SAMPLES);
			if (nl_disp)	nl_disp->neuron_vector[i].output.fval  = AllDispData->samples[s][i].pos  - img_1->wxd;
			if (nl_vdisp)	nl_vdisp->neuron_vector[i].output.fval = AllDispData->samples[s][i].vpos - img_1->wyd;
			if (nl_mtmap)	nl_mtmap->neuron_vector[i].output.fval = AllDispData->samples[s][i].val;
			if (nl_cmap)	nl_cmap->neuron_vector[i].output.fval  = AllDispData->samples[s][i].conf;
		}
		else
		{
			if (nl_disp)	nl_disp->neuron_vector[i].output.fval  = 0;
			if (nl_vdisp)	nl_vdisp->neuron_vector[i].output.fval = 0;
			if (nl_mtmap)	nl_mtmap->neuron_vector[i].output.fval = 0.0;
			if (nl_cmap)	nl_cmap->neuron_vector[i].output.fval  = 0.0;
		}
	}

	out_disp  = get_output_by_neural_layer (nl_disp);
	out_vdisp = get_output_by_neural_layer (nl_vdisp);
	out_mtmap = get_output_by_neural_layer (nl_mtmap);
	out_cmap  = get_output_by_neural_layer (nl_cmap);
	if (out_disp)	glutPostWindowRedisplay (out_disp->win);
	if (out_vdisp)	glutPostWindowRedisplay (out_vdisp->win);
	if (out_mtmap)	glutPostWindowRedisplay (out_mtmap->win);
	if (out_cmap)	glutPostWindowRedisplay (out_cmap->win);

	g_Arrows = 1;
	move_input_window (img_2->name, img_1->wxd, img_1->wyd);
	glutPostWindowRedisplay (img_2->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of SetMotionVergence */


/*!
*********************************************************************************
* Function: MoveSaccade
* Description: Move input window to a point of interest 
* Inputs:
* Output:
*********************************************************************************
*/

void MoveSaccade ( )
{
	int u, v, w, h, xi, yi, wi, hi, x_center, y_center;
	float x_mean, y_mean, weight, accum, hAux;
	INPUT_DESC *img_1, *img_2;
	NEURON_LAYER *nl_mtg = NULL;

	img_1 = (g_isNext ? g_seq_inp : g_input);
	img_2 = (g_isNext ? g_input : g_seq_inp);
	if (!img_1 || !img_2)
	{
		if (!g_input)
			printf ("Click on an input to select it for saccade.\n");
		else 		
			printf ("%s needs a motion sequence image for saccade. "
				"Check the input generator parameters.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_mtg	 = GetMTGaussianNeuronLayerByImages (img_1->neuron_layer, img_2->neuron_layer);
	if (!nl_mtg)
	{
		printf ("%s needs a MT Gaussian neuron layer for saccade.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	if ((img_1->wxd != img_2->wxd) || (img_1->wyd != img_2->wyd))
	{
		img_2->wxd = img_1->wxd;
		img_2->wyd = img_1->wyd;
		move_input_window (img_2->name, img_2->wxd, img_2->wyd);
	}

	w = nl_mtg->dimentions.x;
	h = nl_mtg->dimentions.y;
	wi = img_1->neuron_layer->dimentions.x;
	hi = img_1->neuron_layer->dimentions.y;
	hAux = (float) h / (float) (h - 1);
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = img_1->wxd - img_1->wx;
		y_center = img_1->wyd - img_1->wy;
	}
	else
	{
		x_center = wi/2;
		y_center = hi/2;
	}

	x_mean = y_mean = accum = 0.0;
	
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			weight = nl_mtg->neuron_vector[v * w + u].output.fval;
			if (weight < THRESHOLD)
				continue;
				
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, hAux, LOG_FACTOR);
			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			x_mean += (float) xi * weight;
			y_mean += (float) yi * weight;
			accum += weight;
		}
	}
	
	// Normalize the mean coordinates 
	img_1->wxd = img_2->wxd = (int) ((x_mean / accum) + 0.5);
	img_1->wyd = img_2->wyd = (int) ((y_mean / accum) + 0.5);
	printf ("Saccade : move to (%d, %d)\n", img_1->wxd, img_1->wyd);
	move_input_window (img_1->name, img_1->wxd, img_1->wyd);
	move_input_window (img_2->name, img_2->wxd, img_2->wyd);
	glutPostWindowRedisplay (img_1->win);
	glutPostWindowRedisplay (img_2->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}


/*!
*********************************************************************************
* Function: MotionPursuit
* Description: Get next image and perform a motion pursuit
* Inputs:
* Output:
*********************************************************************************
*/

void MotionPursuit ( )
{
	char strCommand[128];
	INPUT_DESC *img_1, *img_2;

	img_1 = (g_isNext ? g_seq_inp : g_input);
	img_2 = (g_isNext ? g_input : g_seq_inp);
	if (!img_1 || !img_2 || !g_pursuit)
	{
		printf ("Must set motion vergence before performing motion pursuit.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	printf("Motion pursuit : %s to (%d, %d)\n", img_1->name, g_next_x1, g_next_y1);
	sprintf (strCommand, "run 1;");
	interpreter (strCommand);
	img_1->wxd = img_2->wxd = g_next_x1;
	img_1->wyd = img_2->wyd = g_next_y1;

	g_Arrows = 0;
	move_input_window (img_1->name, img_1->wxd, img_1->wyd);
	move_input_window (img_2->name, img_2->wxd, img_2->wyd);
	glutPostWindowRedisplay (img_1->win);
	glutPostWindowRedisplay (img_2->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}


// ----------------------------------------------------------------------------------
// calculateWorldPoint -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------------
WORLD_POINT calculateWorldPoint (IMAGE_COORDINATE distortedLeftPoint, IMAGE_COORDINATE distortedRightPoint)
{
	IMAGE_COORDINATE undistortedLeftPoint;
	IMAGE_COORDINATE undistortedRightPoint;
	IMAGE_COORDINATE leftPrincipalPoint;
	IMAGE_COORDINATE rightPrincipalPoint;
	double fltCameraLeftFocus;
	double fltCameraRightFocus;
	double fltCameraDistance;
	
	// Calculates the undistorted image left coordinates  
	cc = cameraLeftCalibrationConstants;
	cp = cameraLeftCalibrationParameters;
	distorted_to_undistorted_image_coord (distortedLeftPoint.x, distortedLeftPoint.y, 
					      &undistortedLeftPoint.x, &undistortedLeftPoint.y);
	
	// Calculates the undistorted image right coordinates  
	cc = cameraRightCalibrationConstants;
	cp = cameraRightCalibrationParameters;
	distorted_to_undistorted_image_coord (distortedRightPoint.x, distortedRightPoint.y, 
					      &undistortedRightPoint.x, &undistortedRightPoint.y);

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
	
	return TMapCalculateWorldPoint (undistortedLeftPoint, undistortedRightPoint, leftPrincipalPoint, rightPrincipalPoint,
				       	fltCameraLeftFocus, fltCameraRightFocus, fltCameraDistance);
}


// -----------------------------------------------------------------------------------------
// ReconstructByDisparityMap - Reconstroi o mundo em torno do ponto de 
// vergencia atraves do mapa de disparidade.
//
// Entrada: Nenhuma.
//
// Saida: Nenhuma.
// -----------------------------------------------------------------------------------------
void ReconstructByDisparityMap (INPUT_DESC *img_l, INPUT_DESC *img_r, NEURON_LAYER *nl_disp)
{
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	int u, v;
	int w, h, wi, hi, xi, yi;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity = 0.0;
	float intensity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;
	
	// Dimensoes do cortex (disparity map)
	h = nl_disp->dimentions.y;
	w = nl_disp->dimentions.x;

	// Dimensoes da imagem de entrada
	hi = img_r->neuron_layer->dimentions.y;
	wi = img_r->neuron_layer->dimentions.x;

	// NAO SEI POR QUE. PRECISA DISTO PARA FUNCIONAR DEVIDO A ALTERACAO FEITA POR DIJALMA
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = img_r->wxd - img_r->wx;
		y_center = img_r->wyd - img_r->wy;
		x_center_left = img_l->wxd - img_l->wx;
		y_center_left = img_l->wyd - img_l->wy;
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

			pixel = img_r->neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;

			right_point.x = (double) (xi);
			right_point.y = (double) (yi);

			switch (nl_disp->output_type)
			{
				case GREYSCALE:
				{
					fltDisparity = (float) nl_disp->neuron_vector[w * v + u].output.ival;
				}
				break;
				
				case GREYSCALE_FLOAT:
				{
					fltDisparity = nl_disp->neuron_vector[w * v + u].output.fval;
				}
				break;
				
				default:
				{
					Erro ("Neuron layer ", nl_disp->name, " must have greyscale outputs. "
						" Error in ReconstructByDisparityMap.");
				}
			}

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, 
					 (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			left_point.x = (double) (xi + fltDisparity);
			left_point.y = (double) (yi);

			world_point = calculateWorldPoint (left_point, right_point);
							       
			g_map.point[yi][xi].x = world_point.x;
			g_map.point[yi][xi].y = world_point.y;
			g_map.point[yi][xi].z = -world_point.z;

			g_map.point[yi][xi].intensity = intensity;
			g_map.point[yi][xi].dist = sqrt((world_point.x * world_point.x) + 
							(world_point.y * world_point.x) + (world_point.z * world_point.z));
		} /* end of for (u < nEndCol) */
	} /* end of for (v < h) */
} /* end of ReconstructByDisparityMap */


// ----------------------------------------------------------------------------
// Reconstruct -
//
// Entrada:
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void Reconstruct()
{
	float intensity;
	int pixel;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	TPoint *point;
	INPUT_DESC *img_l, *img_r;
	NEURON_LAYER *nl_mtg, *nl_disp;


	img_l = (g_isRight ? g_linked_inp : g_input);
	img_r = (g_isRight ? g_input : g_linked_inp);

	if (!img_l || !img_r)
	{
		if (!g_input)
			printf ("Click on an input to select it for reconstruct.\n");
		else 		
			printf ("%s needs a stereo image for reconstruct. Check the input generator parameters.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_mtg = GetMTGaussianNeuronLayerByImages (img_l->neuron_layer, img_r->neuron_layer);
	if (!nl_mtg)
	{
		printf ("%s needs a MT Gaussian neuron layer for reconstruct.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_disp = GetMapNeuronLayer (nl_mtg, DISPARITY_MAP);
	if (!nl_disp)
	{
		printf ("%s needs a Disparity Map neuron layer for reconstruct.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	printf("Reconstruct\n");
	
	point = &(g_map.point[img_r->wyd][img_r->wxd]);

	right_point.x = (double) (img_r->wxd);
	right_point.y = (double) (img_r->wyd);

	left_point.x = (double) (img_l->wxd);
	left_point.y = (double) (img_l->wyd);
	
	world_point = calculateWorldPoint (left_point, right_point);

	point->x = world_point.x;
	point->y = world_point.y;
	point->z = -world_point.z;
	point->dist = sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z);
	printf ("Vergence - x:%3.2f y:%3.2f z:%3.2f - distance: %3.2f\n", world_point.x, world_point.y, world_point.z, point->dist);

	pixel = img_r->neuron_layer->neuron_vector[(img_r->wyd * img_r->neuron_layer->dimentions.x) + img_r->wxd].output.ival;
	intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
	g_map.point[img_r->wyd][img_r->wxd].intensity = intensity;

	ReconstructByDisparityMap (img_l, img_r, nl_disp);

	glutPostWindowRedisplay (img_l->win);
	glutPostWindowRedisplay (img_r->win);
	glutPostWindowRedisplay (g_wndMap);
	glutPostWindowRedisplay (g_wndMapTop);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of Reconstruct */


// ----------------------------------------------------------------------------
// EraseMapping - Reseta o mapa
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void EraseMapping()
{
	TMapReset(&g_map);

	glutPostWindowRedisplay (g_wndMap);
	glutPostWindowRedisplay (g_wndMapTop);

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}


// ----------------------------------------------------------------------------
// LowPass - Passa uma gaussiana no mapa de disparidade
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void LowPass()
{
	int x, y, wo, ho;
	OUTPUT_DESC *output;  
	NEURON_LAYER *nl, *nl_mtg;
	float *fltOutVal;


	if ((output = get_output_by_win (glutGetWindow ())) == NULL)
	{
		printf ("Click on a Disparity Map output to select it for low pass.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_mtg = GetMTGaussianNeuronLayerByMap (output->neuron_layer, DISPARITY_MAP);
	if (!nl_mtg)
	{
		printf ("%s is not a Disparity Map.\n", output->neuron_layer->name);
		printf ("Click on a Disparity Map output to select it for low pass.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	printf("%s - Low Pass\n", output->neuron_layer->name);

	nl = output->neuron_layer;
	wo = nl->dimentions.x;
	ho = nl->dimentions.y;
	
	fltOutVal = g_fltDispAux;
	if (fltOutVal == NULL)
		fltOutVal = (float *) alloc_mem (sizeof (float) * wo * ho);

	if (g_ReceptiveField == NULL)
	{
		g_ReceptiveField = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof (RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(g_ReceptiveField, KERNEL_SIZE_LOWPASS, SIGMA_LOWPASS);
	}

	/* We need an auxiliary structure fltOutVal because the neuron layer must stay unchanged during the apply */
	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			fltOutVal[y * wo + x] = apply_gaussian_kernel(g_ReceptiveField, nl, x, y);
		}
	}

	/* Now we are ready to update the neuron layer */
	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			nl->neuron_vector[y * wo + x].output.fval = fltOutVal[y * wo + x];
		}
	}

	if (fltOutVal != g_fltDispAux)
		free (fltOutVal);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of LowPass */


// ----------------------------------------------------------------------------
// MoveXImageLeft - Move o foco da ImageLeft
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MoveXImageLeft()
{
	INPUT_DESC *img_l;

	img_l = (g_isRight ? g_linked_inp : g_input);

	if (!img_l)
	{
		if (!g_input)
			printf ("Click on an input to move X coordinate.\n");
		else 		
			printf ("%s is not a left image. Cannot move X coordinate.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	img_l->wxd += g_nMoveLeft;
	move_input_window (img_l->name, img_l->wxd, img_l->wyd);
	glutPostWindowRedisplay (img_l->win);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);
}

// ----------------------------------------------------------------------------
// MoveVerticalGap - Altera o Gap Vertical da imagem da esquerda.
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MoveVerticalGap()
{
	INPUT_DESC *img_l, *img_r;

	img_l = (g_isRight ? g_linked_inp : g_input);
	img_r = (g_isRight ? g_input : g_linked_inp);

	if (!img_l || !img_r)
	{
		if (!g_input)
			printf ("Click on an input to select it for move vertical gap.\n");
		else 		
			printf ("%s needs a stereo image for move vertical gap. "
				"Check the input generator parameters.\n", g_input->name);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	img_l->wyd = img_r->wyd + g_nVerticalGap;
	move_input_window (img_l->name, img_l->wxd, img_l->wyd);
	glutPostWindowRedisplay (img_l->win);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);
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

} /* end of distance_neig */


// ----------------------------------------------------------------------------
// calculate_disparity - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void calculate_disparity (NEURON_LAYER *nl_disp, DISP_DATA *DispData)
{
	SAMPLES temp;
	int x, y, w, h, sample, best_sample = 0, cur_dist, dist;
	
	w = nl_disp->dimentions.x;
	h = nl_disp->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (sample = 0; sample < DispData->num_samples; sample++)
			{
				dist = distance_neig (DispData->samples[0], DispData->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = DispData->samples[0][y * w + x];
			DispData->samples[0][y * w + x] = DispData->samples[best_sample][y * w + x];
			DispData->samples[best_sample][y * w + x] = temp;
		}
	}
}


// ----------------------------------------------------------------------------
// CalculateDisparity - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void CalculateDisparity ()
{
	OUTPUT_DESC *out_disp, *out_mtmap, *out_cmap;  
	NEURON_LAYER *nl_mtg, *nl_disp, *nl_mtmap, *nl_cmap;
	DISP_DATA *DispData;
	int i, wo, ho; 


	if ((out_disp = get_output_by_win (glutGetWindow ())) == NULL)
	{
		printf ("Click on a Disparity Map output to select it for calculate disparity.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_disp = out_disp->neuron_layer;
	nl_mtg  = GetMTGaussianNeuronLayerByMap (nl_disp, DISPARITY_MAP);
	if (!nl_mtg)
	{
		printf ("%s is not a Disparity Map.\n", nl_disp->name);
		printf ("Click on a Disparity Map output to select it for calculate disparity.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	DispData = GetDispData (nl_mtg, g_DispData_list);
	if (DispData == NULL)
	{
		printf ("Must set vergence before calculating disparity.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	printf("%s - Calculate Disparity\n", nl_disp->name);

	calculate_disparity (nl_disp, DispData);

	nl_mtmap = GetMapNeuronLayer (nl_mtg, MT_GAUSSIAN_MAP);
	nl_cmap  = GetMapNeuronLayer (nl_mtg, CONFIDENCE_MAP);

	wo = nl_disp->dimentions.x;
	ho = nl_disp->dimentions.y;

	for (i = 0; i < (wo * ho); i++)
	{
		nl_disp->neuron_vector[i].output.fval  = DispData->samples[0][i].pos - DispData->left_vergence;
		if (nl_mtmap)	nl_mtmap->neuron_vector[i].output.fval = DispData->samples[0][i].val;
		if (nl_cmap)	nl_cmap->neuron_vector[i].output.fval  = DispData->samples[0][i].conf;
	}

	out_mtmap = get_output_by_neural_layer (nl_mtmap);
	out_cmap  = get_output_by_neural_layer (nl_cmap);
	glutPostWindowRedisplay (out_disp->win);
	if (out_mtmap)	glutPostWindowRedisplay (out_mtmap->win);
	if (out_cmap)	glutPostWindowRedisplay (out_cmap->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of CalculateDisparity */


/*!
*********************************************************************************
* Function: permeability_weight
* Description: Returns a weight used to represent the anisotropic nature of the 
*              smoothing constraint in the evidence propagation algorithm
* Inputs:
* Output:
*********************************************************************************
*/

float permeability_weight (float Pa, float Pb, float P_min)
{
	float weight, P, delta_P;

	P = (Pa < Pb ? Pa : Pb);
	P = (P > P_min ? P : P_min);
	 
	// To avoid division by zero, let's assume abs (Pa - Pb) >= P_min 
	delta_P = (Pa < Pb ? (Pb - Pa) : (Pa - Pb));
	delta_P = (delta_P > P_min ? delta_P : P_min);
	
	weight = P / delta_P;

	return weight;
}

	
/*!
*********************************************************************************
* Function: evidence_propagation
* Description: Returns an estimate of the evidence propagation.
*              The linear partial differential equation can be solved numerically
*              using the Gauss-Seidel iterative method. This function returns
*              the result of one iteration.
* Inputs:
* Output:
*********************************************************************************
*/

float evidence_propagation (int i, int j, int s, int w, int h, int d, float fltSmoothness, float fltPMin, 
			    DISP_DATA *AllDispData, DISP_DATA *AllDispDataProp)
{
	float I;
	float P, P_up, P_down, P_left, P_right, P_low, P_high;
	float L, L_up, L_down, L_left, L_right, L_low, L_high;
	float W_up, W_down, W_left, W_right, W_low, W_high;
	float weighted_sum, total_weight;

	/* I is the output value of MT gaussian point (i,j) at one specific disparity layer */
	I       = AllDispData->samples[s][j * w + i].val;

	/* P is the confidence value of point (i,j) at one specific disparity layer */
	P       = AllDispData->samples[s][j * w + i].conf;
	P_up    = (j ==  0      ? 0.0 : AllDispData->samples[s][(j - 1) * w + i].conf);
	P_down  = (j == (h - 1) ? 0.0 : AllDispData->samples[s][(j + 1) * w + i].conf);
	P_left  = (i ==  0      ? 0.0 : AllDispData->samples[s][j * w + (i - 1)].conf);
	P_right = (i == (w - 1) ? 0.0 : AllDispData->samples[s][j * w + (i + 1)].conf);
	P_low   = (s ==  0      ? 0.0 : AllDispData->samples[s - 1][j * w + i].conf);
	P_high  = (s == (d - 1) ? 0.0 : AllDispData->samples[s + 1][j * w + i].conf);

	/* L is the modified I value after the propagation of neighbors evidences */
	L_up    = (j ==  0      ? 0.0 : AllDispDataProp->samples[s][(j - 1) * w + i].val);
	L_down  = (j == (h - 1) ? 0.0 : AllDispDataProp->samples[s][(j + 1) * w + i].val);
	L_left  = (i ==  0      ? 0.0 : AllDispDataProp->samples[s][j * w + (i - 1)].val);
	L_right = (i == (w - 1) ? 0.0 : AllDispDataProp->samples[s][j * w + (i + 1)].val);
	L_low   = (s ==  0      ? 0.0 : AllDispDataProp->samples[s - 1][j * w + i].val);
	L_high  = (s == (d - 1) ? 0.0 : AllDispDataProp->samples[s + 1][j * w + i].val);

	// W is the permeability weight factor used for propagation of neighbor evidences based on confidence gradient
	W_up    = (j ==  0      ? 0.0 : permeability_weight (P_up,    P, fltPMin));
	W_down  = (j == (h - 1) ? 0.0 : permeability_weight (P_down,  P, fltPMin));
	W_left  = (i ==  0      ? 0.0 : permeability_weight (P_left,  P, fltPMin));
	W_right = (i == (w - 1) ? 0.0 : permeability_weight (P_right, P, fltPMin));
	W_low   = (s ==  0      ? 0.0 : permeability_weight (P_low,   P, fltPMin));
	W_high  = (s == (d - 1) ? 0.0 : permeability_weight (P_high,  P, fltPMin));

	weighted_sum = (I + fltSmoothness * (L_up * W_up + L_down * W_down + L_left * W_left + L_right * W_right + 
					     L_low * W_low + L_high * W_high));
	total_weight = (1.0 + fltSmoothness * (W_up + W_down + W_left + W_right + W_low + W_high)); 
	L = weighted_sum / total_weight; 

	return L;
}


/*!
*********************************************************************************
* Function: copy_AllDispData
* Description: 
* Inputs:
* Output:
*********************************************************************************
*/

void copy_AllDispData (DISP_DATA *dest, DISP_DATA *source)
{
	int s, i;

	for (s = 0; s < source->max_sample; s++)
	{
		for (i = 0; i < source->num_neurons; i++)
		{
			dest->samples[s][i] = source->samples[s][i];
		}
	}
}


/*!
*********************************************************************************
* Function: evidence_propagation_all
* Description: 
* Inputs:
* Output:
*********************************************************************************
*/

void evidence_propagation_all ( )
{
	int i, j, s, n, nIndex;
	int wo, ho, max_sample;
	float fltSmoothness, fltPMin, fltOutput; 
	float avg_val = FLT_MAX, previous_avg_val, dif_val;
	NEURON_LAYER *nl_mtg, *nl_disp, *nl_mtmap, *nl_cmap;
	OUTPUT_DESC *out_disp, *out_mtmap, *out_cmap;
	DISP_DATA *DispData, *AllDispData, *AllDispDataProp;


	if ((out_disp = get_output_by_win (glutGetWindow ())) == NULL)
	{
		printf ("Click on a Disparity Map output to select it for evidence propagation.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_disp = out_disp->neuron_layer;
	nl_mtg  = GetMTGaussianNeuronLayerByMap (nl_disp, DISPARITY_MAP);
	if (!nl_mtg)
	{
		printf ("%s is not a Disparity Map.\n", nl_disp->name);
		printf ("Click on a Disparity Map output to select it for evidence propagation.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	DispData = GetDispData (nl_mtg, g_DispData_list);
	AllDispData = GetDispData (nl_mtg, g_AllDispData_list);
	if ((DispData == NULL) || (AllDispData == NULL))
	{
		printf ("Must set vergence before evidence propagation.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	printf("%s - Evidence Propagation\n", nl_disp->name);

	AllDispDataProp = g_AllDispDataProp_list->DispData;
	if (AllDispDataProp == NULL)
		AllDispDataProp = CreateDispData (g_AllDispDataProp_list, AllDispData->num_samples, AllDispData->num_neurons);

	copy_AllDispData (AllDispDataProp, AllDispData);

	wo = nl_mtg->dimentions.x;
	ho = nl_mtg->dimentions.y;
	max_sample = AllDispData->max_sample;
	fltSmoothness = SMOOTHNESS;
	fltPMin       = PERMEABILITY_MIN;

	for (n = 0; n < MAX_ITERATIONS; n++)
	{
		previous_avg_val = avg_val;
		avg_val = 0.0;
		for (s = 0; s < max_sample; s++)
		{
			for (i = 0; i < wo; i++)
			{
				for (j = 0; j < ho; j++)
				{
					fltOutput = evidence_propagation (i, j, s, wo, ho, max_sample, fltSmoothness, fltPMin,
									  AllDispData, AllDispDataProp);
					AllDispDataProp->samples[s][j * wo + i].val = fltOutput;
					avg_val += fltOutput; 
				}
			}
		}
		avg_val /= (wo * ho * max_sample);

		dif_val = 0.0;
		if ((avg_val == 0.0) || (previous_avg_val == 0.0))
			break;

		dif_val = (avg_val / previous_avg_val) - 1.0;
		dif_val = (dif_val > 0.0 ? dif_val : - dif_val);
		if (dif_val < CONVERGENCE)
			break;
				
		if (n % (MAX_ITERATIONS / 50) == (MAX_ITERATIONS / 50 - 1))
			printf ("Average propagation difference after %d iterations: %f%%\n", n+1, dif_val * 100.0);
	}			
	printf ("Average propagation difference after %d iterations: %f%%\n", n, dif_val * 100.0);

	copy_AllDispData (AllDispData, AllDispDataProp);

	InitDispDataState (DispData);

	for (s = 1; s < max_sample; s++)
	{
		for (i = 0; i < wo; i++)
		{
			for (j = 0; j < ho; j++)
			{
				nIndex = j * wo + i;
				
				if (AllDispData->samples[s][nIndex].val >= AllDispData->samples[s - 1][nIndex].val)
				{
					if (g_neg_slope[nIndex])
					{
						AddLocalMin (DispData, nIndex, 
							     AllDispData->samples[s - 1][nIndex].val,
							     AllDispData->samples[s - 1][nIndex].conf,
							     AllDispData->samples[s - 1][nIndex].pos);
					}
					g_neg_slope[nIndex] = 0;
				}
				else
				{
					if (s == max_sample - 1)
					{
						AddLocalMin (DispData, nIndex,
							     AllDispData->samples[s][nIndex].val, 
							     AllDispData->samples[s][nIndex].conf, 
							     AllDispData->samples[s][nIndex].pos);
					}
					g_neg_slope[nIndex] = 1;
				}
			}
		}
	}

	nl_mtmap = GetMapNeuronLayer (nl_mtg, MT_GAUSSIAN_MAP);
	nl_cmap  = GetMapNeuronLayer (nl_mtg, CONFIDENCE_MAP);

	for (i = 0; i < DispData->num_neurons; i++)
	{
		if (nl_disp)	nl_disp->neuron_vector[i].output.fval  = DispData->samples[0][i].pos - DispData->left_vergence;
		if (nl_mtmap)	nl_mtmap->neuron_vector[i].output.fval = DispData->samples[0][i].val;
		if (nl_cmap)	nl_cmap->neuron_vector[i].output.fval  = DispData->samples[0][i].conf;
	}

	out_mtmap = get_output_by_neural_layer (nl_mtmap);
	out_cmap  = get_output_by_neural_layer (nl_cmap);
	if (out_disp)	glutPostWindowRedisplay (out_disp->win);
	if (out_mtmap)	glutPostWindowRedisplay (out_mtmap->win);
	if (out_cmap)	glutPostWindowRedisplay (out_cmap->win);

	glutIdleFunc ((void (* ) (void))fl_check_forms);

} /* end of evidence_propagation_all */


// ----------------------------------------------------------------------------
// new_output_passive_motion - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void new_output_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	static INPUT_DESC *current_frame = NULL;
	static OUTPUT_DESC *output = NULL;
	NEURON_OUTPUT pixel;
	int r, g, b;
	int w, h;
	
	if (current_frame == NULL)	
	{
		if (!g_input)
			return;
		current_frame = g_input;
	}
	output = get_output_by_win (glutGetWindow ());
			
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
	h = output->neuron_layer->dimentions.y;
	
	if ((wx >= 0) && (wx < w) && (wy >= 0) && (wy < h))
		pixel = output->neuron_layer->neuron_vector[w * (int) (wy) + (int) (wx)].output;
	else
		pixel.ival = 0;
		
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
		
	map_v1_to_image (&(current_frame->green_cross_x), &(current_frame->green_cross_y), 
		         current_frame->ww, current_frame->wh, (int) wx, (int) wy, output->ww, output->wh, 
			 current_frame->wxd, current_frame->wyd, (float) output->wh / (float) (output->wh - 1), LOG_FACTOR);

	glutPostWindowRedisplay (current_frame->win);		 

} /* end of new_output_passive_motion */


/*!
********************************************************************************************
* Function:	PrintAllDispData
* Description:	Prints all samples of g_AllDispData for point (wxd, wyd)
* Inputs:	-
* Output:	-
********************************************************************************************
*/
void PrintAllDispData ()
{
	NEURON_LAYER *nl_mtg, *nl_disp;
	OUTPUT_DESC *out_disp;  
	DISP_DATA *AllDispData;
	int w, h, x, y;
	int i, nIndex;


	if ((out_disp = get_output_by_win (glutGetWindow ())) == NULL)
	{
		printf ("Click on a Disparity Map output to select it for printing AllDispData samples.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	nl_disp = out_disp->neuron_layer;
	nl_mtg  = GetMTGaussianNeuronLayerByMap (nl_disp, DISPARITY_MAP);
	if (!nl_mtg)
	{
		printf ("%s is not a Disparity Map.\n", nl_disp->name);
		printf ("Click on a Disparity Map output to select it for printing AllDispData samples.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	AllDispData = GetDispData (nl_mtg, g_AllDispData_list);
	if (AllDispData == NULL)
	{
		printf ("Must set vergence before printing AllDispData samples.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	w = nl_disp->dimentions.x;
	h = nl_disp->dimentions.y;
	x = out_disp->wxd;
	y = out_disp->wyd;

	if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
	{
		nIndex = (w * y) + x;
		printf("MT gaussian output and confidence of point x = %d , y = %d, disp = %d:\n", x, y,
			(int) nl_disp->neuron_vector[nIndex].output.fval);
		for (i = 0; i < AllDispData->max_sample; i++)
			printf("disp = %3d   output = %f   conf = %f\n", 
				AllDispData->samples[i][nIndex].pos - AllDispData->left_vergence,
				AllDispData->samples[i][nIndex].val, 
				AllDispData->samples[i][nIndex].conf);
	}

	glutIdleFunc ((void (* ) (void)) fl_check_forms);

} /* end of PrintAllDispData */


/*!
********************************************************************************************
* Function:	calc_illuminance
* Description:	Apply the Illuminance and the Reflectance filters to ONE image
*		until convergence
* Inputs:	illuminance_filter , reflectance_filter 
* Output:	-
********************************************************************************************
*/
void calc_illuminance (FILTER_DESC *illum_filter, FILTER_DESC *reflec_filter)
{
	int i;
	float avg_val, previous_avg_val, dif_val;
	OUTPUT_DESC *out_illum, *out_reflec;

	printf ("calc_illuminance: %s - %s\n", illum_filter->output->name, reflec_filter->output->name);
	for (i = 0; i < MAX_ITERATIONS; i++)
	{
		previous_avg_val = * (float *) illum_filter->private_state;
		filter_update (illum_filter);
		filter_update (reflec_filter);
		avg_val = * (float *) illum_filter->private_state;
		dif_val = 0.0;
		if ((avg_val == 0.0) || (previous_avg_val == 0.0))
			break;

		dif_val = (avg_val / previous_avg_val) - 1.0;
		dif_val = (dif_val > 0.0 ? dif_val : - dif_val);
		if (dif_val < CONVERGENCE)
			break;
				
		if (i % (MAX_ITERATIONS / 10) == (MAX_ITERATIONS / 10 - 1))
			printf ("Average illuminance difference after %d iterations: %f%%\n", i+1, dif_val * 100.0);
	}
	printf ("Average illuminance difference after %d iterations: %f%%\n", i, dif_val * 100.0);

	out_illum  = get_output_by_neural_layer (illum_filter->output);
	out_reflec = get_output_by_neural_layer (reflec_filter->output);
	if (out_illum)	glutPostWindowRedisplay (out_illum->win);
	if (out_reflec)	glutPostWindowRedisplay (out_reflec->win);
	
} /* end of calc_illuminance */


/*!
*********************************************************************************
* Function:	CalculateIlluminance
* Description:	Apply the Illuminance and the Reflectance filters to ALL images
*		until convergence
* Inputs:	-
* Output:	-
*********************************************************************************
*/
void CalculateIlluminance()
{
	OUTPUT_DESC *output;
	FILTER_DESC *illum_filter, *reflec_filter;
	char msg[128];

	if ((output = get_output_by_win (glutGetWindow ())) == NULL)
	{
		printf ("Click on a Illuminance or Reflectance output to select it for calculation.\n");
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	illum_filter  = GetIlluminanceFilterByNeuronLayer (output->neuron_layer);
	if (!illum_filter)
	{
		reflec_filter = GetReflectanceFilterByNeuronLayer (output->neuron_layer);
		if (!reflec_filter)
		{
			printf ("%s is not generated by illuminance or reflectance filters.\n", output->name);
			printf ("Click on an Illuminance or Reflectance output to select it for calculation.\n");
			glutIdleFunc ((void (* ) (void))fl_check_forms);
			return;
		}
		illum_filter  = GetIlluminanceFilterByNeuronLayer (reflec_filter->neuron_layer_list->next->neuron_layer);
		sprintf (msg, "%s needs a Illuminance neuron layer for calculation.\n", output->name);
	}
	else
	{
		reflec_filter = GetReflectanceFilterByIlluminance (illum_filter);
		sprintf (msg, "%s needs a Reflectance neuron layer for calculation.\n", output->name);
	}

	if (!illum_filter || !reflec_filter)
	{
		printf ("%s", msg);
		glutIdleFunc ((void (* ) (void))fl_check_forms);
		return;
	}

	calc_illuminance (illum_filter, reflec_filter);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);

} /* end of CalculateIlluminance */


// ----------------------------------------------------------------------------------
// f_keyboard -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------------
void f_keyboard (char *key_value)
{
	char key;
	OUTPUT_DESC *output;

	key = key_value[0];
	switch (key)
	{
		case 'M':
		case 'm':
		{
			g_nColorImage = COLOR_MONO_256;
		}
		break;

		case 'C':
		case 'c':
		{
			g_nColorImage = COLOR_332;
		}
		break;

		case 'X':
		case 'x':
		{
			g_nColorImage = COLOR_MONO_8;
		}
		break;

		case 'R':
		case 'r':
		{
			glutIdleFunc ((void (* ) (void))Reconstruct);
		}
		break;

		case 'E':
		case 'e':
		{
			glutIdleFunc ((void (* ) (void))EraseMapping);
		}
		break;

		case 'F':
		case 'f':
		{
			glutIdleFunc ((void (* ) (void))LowPass);
		}
		break;

		case 'V':
		case 'v':
		{
			glutIdleFunc ((void (* ) (void))SetStereoVergence);
		}
		break;

		case 'T':
		case 't':
		{
			glutIdleFunc ((void (* ) (void))SetMotionVergence);
		}
		break;

		case 'K':
		case 'k':
		{
			glutIdleFunc ((void (* ) (void))MoveSaccade);
		}
		break;

		case 'Y':
		case 'y':
		{
			glutIdleFunc ((void (* ) (void))MotionPursuit);
		}
		break;

		case 'W':
		case 'w':
		{
			glutIdleFunc ((void (* ) (void))evidence_propagation_all);
		}
		break;

		case 'O':
		{
			g_nMoveLeft = -5;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;

		case 'o':
		{
			g_nMoveLeft = -1;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;

		case 'P':
		{
			g_nMoveLeft = 5;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;

		case 'p':
		{
			g_nMoveLeft = 1;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;

		case 'S':
		case 's':
		{
			glutIdleFunc ((void (* ) (void))SalvaImagens);
		}
		break;
		
		case 'U':
		{
			g_nVerticalGap += 1;
			printf("g_nVerticalGap = %d\n", g_nVerticalGap);
			glutIdleFunc ((void (* ) (void))MoveVerticalGap);
		}
		break;
		
		case 'u':
		{
			g_nVerticalGap -= 1;
			printf("g_nVerticalGap = %d\n", g_nVerticalGap);
			glutIdleFunc ((void (* ) (void))MoveVerticalGap);
		}
		break;

		case 'D':
		case 'd':
		{
			glutIdleFunc ((void (* ) (void))CalculateDisparity);
		}
		break;

		case 'G':
		case 'g':
		{
			if (g_input && g_seq_inp)
			{
				g_Arrows++;
				if (g_Arrows > 2)
					g_Arrows = 0;
				printf("g_Arrows state = %d\n", g_Arrows);
				if (g_isNext)
				{
					move_input_window (g_input->name, g_seq_inp->wxd, g_seq_inp->wyd);
					glutPostWindowRedisplay (g_input->win);
				}
				else
				{
					move_input_window (g_seq_inp->name, g_input->wxd, g_input->wyd);
					glutPostWindowRedisplay (g_seq_inp->win);
				}
			}
		}
		break;

		case 'J':
		{
			g_Threshold += 0.05;
			printf("g_Threshold = %.2f\n", g_Threshold);
		}
		break;

		case 'j':
		{
			if (g_Threshold > 0.0)
				g_Threshold -= 0.05;
			printf("g_Threshold = %.2f\n", g_Threshold);
		}
		break;

		case 'N':
		{
			g_Arrow_Grid++;
			printf("g_Arrow_Grid = %d\n", g_Arrow_Grid);
		}
		break;

		case 'n':
		{
			if (g_Arrow_Grid > 1)
				g_Arrow_Grid--;
			printf("g_Arrow_Grid = %d\n", g_Arrow_Grid);
		}
		break;

		case 'A':
		case 'a':
		{
			g_Area++;
			if (g_Area > 1)
				g_Area = 0;
			printf("g_Area state = %d\n", g_Area);
		}
		break;

		case 'L': 
		case 'l':
		{
			LoadCameraParams(); 
		}
		break;

		case 'z':
		{
			if (g_input)
			{
				g_input->green_cross = 1;
						
				if ((output = get_output_by_win (glutGetWindow ())) != NULL)
				{
					glutSetWindow (output->win);
					glutPassiveMotionFunc (new_output_passive_motion);
					glutPostWindowRedisplay (g_input->win);
				}
			}
		}
		break;

		case 'Z':
		{
			if (g_input)
			{
				g_input->green_cross = 0;
			
				if ((output = get_output_by_win (glutGetWindow ())) != NULL)
				{
					glutSetWindow (output->win);
					glutPassiveMotionFunc (output_passive_motion);
					glutPostWindowRedisplay (g_input->win);
				}
			}
		}
		break;
		
		case 'B':
		case 'b':
		{
			glutIdleFunc ((void (* ) (void))PrintAllDispData);
		}
		break;
		
		case 'I':
		case 'i':
		{
			glutIdleFunc ((void (* ) (void))CalculateIlluminance);
		}
		break;
		
		case 'H':
		case 'h':
		{
			printf ("\nAvailable key commands:\n");
			printf ("m : Set image color to COLOR_MONO_256\n");
			printf ("c : Set image color to COLOR_332\n");
			printf ("x : Set image color to COLOR_MONO_8\n");
			printf ("v : Set stereo vergence on left image based on right image focus and calculate maps:\n"
				"    disparity, confidence, MT gaussian and AllDispData\n");
			printf ("t : Set motion vergence based on two sequenced images and calculate maps:\n"
				"    disparity, confidence, MT gaussian and AllDispData\n");
			printf ("k : Move to Saccade\n");
			printf ("y : Perform Motion Pursuit\n");
			printf ("g : Display/erase arrows at the motion area\n");
			printf ("J : Increase moving threshold\n");
			printf ("j : Decrease moving threshold\n");
			printf ("N : Increase arrow grid\n");
			printf ("n : Decrease arrow grid\n");
			printf ("a : Toggle Area flag\n");
			printf ("w : Propagate evidences on AllDispData until convergence then recalculate maps\n");
			printf ("f : Apply gaussian kernel on disparity map\n");
			printf ("d : Recalculate best disparity map based on DispData minimum values\n");
			printf ("b : Print AllDispData samples for current (x,y)\n");
			printf ("r : Reconstruct world by disparity map\n");
			printf ("e : Erase world map\n");
			printf ("O : Move left image 5 pixels to the right hand\n");
			printf ("o : Move left image 1 pixel to the right hand\n");
			printf ("P : Move left image 5 pixels to the left hand\n");
			printf ("p : Move left image 1 pixel to the left hand\n");
			printf ("U : Move vertical gap 1 pixel up\n");
			printf ("u : Move vertical gap 1 pixel down\n");
			printf ("s : Save images\n");
			printf ("l : Load camera parameters\n");
			printf ("z : Set green cross on right image based on current output window\n");
			printf ("Z : Reset green cross on right image\n");
			printf ("i : Apply illuminance and reflectance filters until convergence\n");
			printf ("\n");
		}
		break;
		
	}
}
