#include <locale.h>
#include "robot_user_functions.h"



// Variaveis globais
int g_nLeftEye, g_nRightEye;
SOCKET g_SocketLeft, g_SocketRight;
char chrBuffer[MAX_PACKAGE];	// O tamanho maximo de um pacote TCP �1500 bytes. Foi colocado 1600 apenas para dar uma folga
byte pImage[INPUT_WIDTH * INPUT_HEIGHT];
int nTamImageLeft, nTamImageRight;
int g_nColorImage = COLOR_MONO_256;

double dblTempo1, dblTempo2;
int nFrame = 0;
int nTotalFrame = 0;

int g_Status;

int g_nMoveLeft;

int g_nVerticalGap = VERT_GAP;

float *g_fltDispAux;
float *g_fltConfAux;
DISP_DATA *g_DispData;
DISP_DATA *g_AllDispData;
int nAllSamples;

RECEPTIVE_FIELD_DESCRIPTION *g_ReceptiveField;

// ***** Variaveis para Mapping ******
TMap g_map;
GLint g_wndMap = 0;
GLint g_wndMapTop = 0;
float g_fCenterX, g_fCenterY, g_fCenterZ;	// Centro de rotacao

int g_nMouseX, g_nMouseY;
GLfloat rot, w1, w2, w3;					// Variables for rotation calculation
GLint prevx, prevy;							// Remember previous x and y positions

GLfloat w = MAP_WINDOW_WIDTH;
GLfloat h = MAP_WINDOW_HEIGHT;

GLfloat	g_fltFovW = (INPUT_WIDTH / 2.0) * MAX_Z_VIEW / LAMBDA;
GLfloat	g_fltFovH = (INPUT_HEIGHT / 2.0) * MAX_Z_VIEW / LAMBDA;

// ** Variaveis para o test_stimulus **
int g_nStimulusPosLeft;
int g_nStimulusPosRight;

// ***********************************
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
	
	return (0);
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
					glColor3f(g_map.point[j][i].intensity, g_map.point[j][i].intensity, g_map.point[j][i].intensity);
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
// DrawRobotFOV - Desenha o campo de fisao do robo
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glTranslatef(g_fCenterX, g_fCenterY, g_fCenterZ);		// Centre and away the viewer
	glRotatef(4*rot, w1, w2, w3);							// Do the transformation
	glTranslatef(-g_fCenterX, -g_fCenterY, -g_fCenterZ);	// Centre and away the viewer
	glPointSize(POINT_SIZE);
	DrawWorld();
	DrawRobotFOV();

	glutSwapBuffers();		// Swap buffers
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
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
	
	glutSwapBuffers();		// Swap buffers
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
	glTranslatef(0.0, 0.0, 0.0f);     /* Centre and away the viewer */
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
	glClearColor(0.0f, 0.0f, 0.3f, 0.0f);	// This Will Clear The Background (r,g,b,alpha)
	glClearDepth(1.0);						// Enables Clearing Of The Depth Buffer
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
	if (Height==0)		// Prevent A Divide By Zero If The Window Is Too Small
		Height=1;

	Transform(Width,Height);	// Do our viewing transform
	w = glutGet((GLenum)GLUT_WINDOW_WIDTH);		//Make sure our window size is updated
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
	GLfloat m[16];		// To return the OpenGL transformation matrix values
	float z, dz;
	float R2, r2;
	float a1, a2, a3;
	
	w = glutGet((GLenum)GLUT_WINDOW_WIDTH);		// Make real sure we have the correct
	h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);	// window height and width
	
	R2 = (w*w+h*h)/4;	// sqrt(R2) is window's half-diagonal
	
	// z determines the mouse contact point (x,y,z) on the
	// "invisible glass" sphere (of radius sqrt(R2).
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
	prevx = x; /* Save x and y for dx and dy calcs next time */
	prevy = y;
	find_axis_of_rotation(x, y, dx, dy, &rot, &w1, &w2, &w3);
	glutPostRedisplay();
}

// ----------------------------------------------------------------------------
// InitWindowMap - Inicializa os recursos da janela do 
//				   WindowMap
//
// Entrada: Nenhuma
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void InitWindowMap()
{
	/*int i, j;*/

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

		// Seleciona o range de visualização
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
	/*int i, j;*/

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
void InitWindow(INPUT_DESC *input, int nInputImage)
{
	int x, y;
	int nImageSource;
	
	printf("%s\n",input->name);

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

	if (strcmp(input->name,image_left.name) == 0)
		g_nLeftEye = input->win;
	else
		g_nRightEye = input->win;

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);

	// Cria a janela do mapeamento
	InitWindowMap();
	InitWindowMapTop();
}

// ----------------------------------------------------------------------------
// GetImage - Busca uma imagem do servidor
//                      de imagens
//
// Entrada: input - INPUT_DESC
//          s - Socket no qual ser�lida a imagem
//          nTamImage - Tamanho da imagem a ser lida
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void GetImage(INPUT_DESC *input, SOCKET *s)
{
	int nTamImage;
	int i, x, y;
	int pPonto;
	FILE *file;
	int nImageSource;
	
	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;
	
	switch (nImageSource)
	{
		case GET_IMAGE_FROM_RBT:
		{
			nTamImage = INPUT_WIDTH * INPUT_HEIGHT;
			if (input == (&image_left))
				file = fopen(FILE_RBT_IMAGE_LEFT,"r");
			else
				file = fopen(FILE_RBT_IMAGE_RIGHT,"r");

			fread ((void*)pImage, 1, nTamImage, file);
			fclose(file);
			
			// Coloca a imagem na estrutura da MAE
			for (x = 0, i = 0; x < INPUT_WIDTH; x++)
			{
				for (y = 0; y < INPUT_HEIGHT; y++)
				{
					pPonto = pImage[i++];
					input->image[3 * (x + y*input->tfw) + 0] = pPonto;
					input->image[3 * (x + y*input->tfw) + 1] = pPonto;
					input->image[3 * (x + y*input->tfw) + 2] = pPonto;
				}
			}
		}
		break;

		case GET_IMAGE_FROM_PNM:
		{
		
		}
		break;

		default:
			Erro ("Invalid image source.", "", "");
	}
}

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
	
	if ((data_fd = fopen (CAMERA_LEFT_PARAMETERS_FILE_NAME, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s'\n", CAMERA_LEFT_PARAMETERS_FILE_NAME);
		exit(-1);
	}
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
	char *locale_string;
	
	locale_string = setlocale (LC_ALL, "C");
	if (locale_string == NULL)
	{
		fprintf (stderr, "Could not set locale.\n");
		exit (1);
	}
	else
		printf ("Locale set to %s.\n", locale_string);

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	LoadCameraParams();

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
	/*float fFrameRate;
	int x, y;
	int i;*/
	int nInputImage, nImageSource;
	//struct timeval tv1;
	//struct timeval tv2;
	//static int bInitIR = 1;

	// Define qual a imagem (direita ou esquerda)
	if (strcmp(input->name,image_left.name)==0)
		nInputImage = INPUT_IMAGE_LEFT;
	else
		nInputImage = INPUT_IMAGE_RIGHT;

	// DEBUG
// 	printf("FUNCAO: input_generator - janela: %d\n", nInputImage);

	// Inicializacao executada apenas uma vez por janela de entrada
	if (input->win == 0)
	{
		InitWindow(input, nInputImage);
		g_Status = ST_GET_IMAGE;
	}

/*	if ((nFrame < remaining_steps) && (running))
	{
		gettimeofday(&tv1,NULL);
		nFrame = remaining_steps;
		nTotalFrame = remaining_steps;
		dblTempo1 = (double)tv1.tv_sec + (double)tv1.tv_usec*1e-6;

		g_Status = ST_GET_IMAGE;
	}*/

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	// Busca a imagem
	if (g_Status == ST_GET_IMAGE)
	{
		if (nInputImage == INPUT_IMAGE_LEFT)
		{
			// Envia a requisicao de imagem quando apenas na
			// leitura do olho esquerdo para poder diminuir
			// o gap entre a captura das imagens
	//        nTamImageLeft = SendSnap(&g_SocketLeft);
	//        nTamImageRight = SendSnap(&g_SocketRight);
	
			GetImage(input, &g_SocketLeft);
		}
		else
		{
			GetImage(input, &g_SocketRight);
			nFrame--;
		}
		check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_neurons (input);
//		robot_check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_image (input);
	}

	// Forca a atualizacao dos filtros quando e trocado o ponto de atencao,
	// mas nao esta em modo running. So eh necessario quando for a o olho
	// esquerdo pois, se mover o olho direito, o esquerdo move junto.
	if ((status == MOVE) && (!running) && (input->win == g_nLeftEye))
	{
		all_filters_update ();
		all_outputs_update ();
	}

	// Calcula o Frame Rate
/*	if ((nFrame == 0) && (running))
	{
		g_Status = ST_WAIT;
		gettimeofday(&tv2,NULL);
		dblTempo2 = (double)tv2.tv_sec + (double)tv2.tv_usec*1e-6;
		printf("Tempo: %2.2lfseg - FrameRate: %2.2lf\n",dblTempo2 - dblTempo1, (double)nTotalFrame / (dblTempo2 - dblTempo1));
	}*/
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
	distorted_to_undistorted_image_coord (distortedLeftPoint.x, distortedLeftPoint.y, &undistortedLeftPoint.x, &undistortedLeftPoint.y);
	
	// Calculates the undistorted image right coordinates  
	cc = cameraRightCalibrationConstants;
	cp = cameraRightCalibrationParameters;
	distorted_to_undistorted_image_coord (distortedRightPoint.x, distortedRightPoint.y, &undistortedRightPoint.x, &undistortedRightPoint.y);

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
	
	return TMapCalculateWorldPoint (undistortedLeftPoint,
				       				undistortedRightPoint,
				      				leftPrincipalPoint,
				       				rightPrincipalPoint,
				       				fltCameraLeftFocus,
				       				fltCameraRightFocus,
				       				fltCameraDistance);
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
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	int/* i, j,*/ u, v;
	int w, h, wi, hi, xi, yi;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity = .0f;
	float intensity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;

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
			intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;

			right_point.x = (double) (xi);
			right_point.y = (double) (yi);

			switch (nl_disparity_map.output_type)
			{
				case GREYSCALE:
				{
					fltDisparity = (float) nl_disparity_map.neuron_vector[w * v + u].output.ival;
				}
				break;
				
				case GREYSCALE_FLOAT:
				{
					fltDisparity = nl_disparity_map.neuron_vector[w * v + u].output.fval;
				}
				break;
			}

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			left_point.x = (double) (xi + fltDisparity);
			left_point.y = (double) (yi);

			world_point = calculateWorldPoint (left_point, right_point);
							       
			g_map.point[yi][xi].x = world_point.x;
			g_map.point[yi][xi].y = world_point.y;
			g_map.point[yi][xi].z = -world_point.z;

			g_map.point[yi][xi].intensity = intensity;
			g_map.point[yi][xi].dist = sqrt((world_point.x * world_point.x) + (world_point.y * world_point.x) + (world_point.z * world_point.z));
		
			ViewerUpdateMap(world_point.z, -world_point.x, world_point.y, RED(pixel), GREEN(pixel), BLUE(pixel));
		}
	}
}

// ----------------------------------------------------------------------------
// GetShiftDisparity - Retorna o valor da output do
//					   filtro shift_disparity_filter
//
// Entrada: strNLName - Nome da neuron layer associada
//						ao filtro
//
// Saida: int - Valor da output do filtro
// ----------------------------------------------------------------------------
int GetShiftDisparity (char *strNLName)
{
	NEURON_LAYER *nl;

	nl = get_neuron_layer_by_name(strNLName);
	return nl->neuron_vector[0].output.ival;
}

// ----------------------------------------------------------------------------
// SumOutputCells - Soma o valor da saida de todas as celulas de uma neuron
//					layer
//
// Entrada: nl - Neuron Layer
//
// Saida: float - Somatorio da saida das celulas de nl
// ----------------------------------------------------------------------------
float SumOutputCells(NEURON_LAYER *nl)
{
	int i;
	float fltSum;

	fltSum = 0.0;
	for (i = 0; i < (nl->dimentions.x * nl->dimentions.y); i++)
		fltSum += nl->neuron_vector[i].output.fval;

	return fltSum;
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
}

// ----------------------------------------------------------------------------
// AddLocalMin - 
//
// Entrada: 
//
// Saida: Nenhuma
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
}



// ----------------------------------------------------------------------------
// set_neuronsf - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void SetNeuronsOutputs (NEURON *n, int n0, int num_neurons, float value)
{
	int i;
	
	for (i = 0; i < num_neurons; i++)
	{
		n[i+n0].output.fval = value;
	}
}



// ----------------------------------------------------------------------------
// NeuronOutputsAverage - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

float NeuronOutputsAverage(NEURON *n, int w, int h, float band)
{
	int i, j;
	float fltSum = .0f;

	for (j = 0; j < h; j++)
		for (i = .5f * (1.0f - band) * w; i < .5f * (1.0f + band) * w; i++)
			fltSum += n[i].output.fval;

	return fltSum / (band * w * h);
}


// ----------------------------------------------------------------------------
// set_vergence - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

/*void set_vergence (void)
{
	int nMaxScan, nMinScan, nStepsNumber = 0;
	int wo, ho, num_neurons;
	float fltSumOutputCells = FLT_MAX, fltDisparity;

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	num_neurons = wo*ho;
	
	nMinScan = image_right.wxd - (DISPARITY_RANGE >> 1);
	if (nMinScan < 0)
		nMinScan = 0;
	nMaxScan = image_right.wxd + (DISPARITY_RANGE >> 1);
	if (nMaxScan >= image_right.neuron_layer->dimentions.x)
		nMaxScan = image_right.neuron_layer->dimentions.x - 1;
	
	fltDisparity = (float) (DISPARITY_RANGE >> 1);
	for (image_right.wxd = nMinScan; image_right.wxd < nMaxScan; image_right.wxd++)
	{
		g_nNetworkStatus = TRAINNING;
		SetNeuronsOutputs (nl_disparity_map.neuron_vector, 0, num_neurons, fltDisparity);
		move_input_window (image_right.name, image_right.wxd, image_right.wyd);
		//all_filters_update ();
		//all_outputs_update ();
		g_nNetworkStatus = RUNNING;
		fltDisparity -= 1.0f;
	}
	image_right.wxd -= (DISPARITY_RANGE >> 1);
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	all_filters_update ();
	all_outputs_update ();
	
	while (fltSumOutputCells > .0f && nStepsNumber < 5)
	{	
		fltSumOutputCells = NeuronOutputsAverage(nl_disparity_map.neuron_vector, wo, ho, BAND);
		
		image_left.wxd += (int) (fltSumOutputCells + .5f);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
		nStepsNumber++;
	}

	glutPostWindowRedisplay (image_left.win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}*/

void set_vergence (void)
{
	int i, nMaxScan, nMinScan, nVergDisparity;
	int wo, ho, num_neurons;
	float fltDisparity = .0f;

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	num_neurons = wo*ho;
	
	nMinScan = image_right.wxd;
	if (nMinScan < 0)
		nMinScan = 0;
	nMaxScan = image_right.wxd + DISPARITY_RANGE;
	if (nMaxScan >= image_right.neuron_layer->dimentions.x)
		nMaxScan = image_right.neuron_layer->dimentions.x - 1;
	
	for (image_left.wxd = nMinScan; image_left.wxd < nMaxScan; image_left.wxd++)
	{
		g_nNetworkStatus = TRAINNING;
		SetNeuronsOutputs (nl_disparity_map.neuron_vector, 0, num_neurons, fltDisparity);
		//copy_neuron_outputs (&nl_disparity_map, &nl_lp_image_left);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
		g_nNetworkStatus = RUNNING;
		fltDisparity += 1.0f;
	}
	
	//image_left.wxd = (nMinScan + nMaxScan) >> 1;
	image_left.wxd = nMinScan;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

	nVergDisparity = (int) (NeuronOutputsAverage(nl_disparity_map.neuron_vector, wo, ho, BAND) + .5f);
	
	image_left.wxd = image_right.wxd + nVergDisparity;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

	for (i = 0; i < num_neurons; i++)
		nl_disparity_map.neuron_vector[i].output.fval -= nVergDisparity;
	
	all_outputs_update ();

	glutPostWindowRedisplay (image_left.win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}

/*void set_vergence (void)
{
	int i, nMaxScan, nMinScan, nVergDisparity;
	int u, v, wo, ho, num_neurons;
	float fltDisparity = .0f;
	static float *p_fltDisparityMap = NULL;
	static in *p_nFrequencyMap = NULL;

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;
	num_neurons = wo*ho;
	
	if (p_fltDisparityMap == NULL)
	{
		p_fltDisparityMap = (float *) alloc_mem (wo * ho * sizeof (float));
		p_nFrequencyMap = (int *) alloc_mem (wo * ho * sizeof (int));
	}
	
	nMinScan = image_right.wxd;
	if (nMinScan < 0)
		nMinScan = 0;
	nMaxScan = image_right.wxd + DISPARITY_RANGE;
	if (nMaxScan >= image_right.neuron_layer->dimentions.x)
		nMaxScan = image_right.neuron_layer->dimentions.x - 1;
	
	for (image_left.wxd = nMinScan; image_left.wxd < nMaxScan; image_left.wxd++)
	{
		g_nNetworkStatus = TRAINNING;
		SetNeuronsOutputs (nl_disparity_map.neuron_vector, 0, num_neurons, fltDisparity);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
		g_nNetworkStatus = RUNNING;
		fltDisparity += 1.0f;
	}

	image_left.wxd = nMinScan;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	
	nMinScan = image_right.wxd - DISPARITY_RANGE + 1;
	if (nMinScan < 0)
		nMinScan = 0;
	nMaxScan = image_right.wxd;
	
	for (i = 0; i < num_neurons; i++)
	{
		p_fltDisparityMap[i] = .0f;
		p_nFrequencyMap[i] = 0;
	}

	for (image_right.wxd = nMinScan; image_right.wxd <= nMaxScan; image_right.wxd++)
	{
		move_input_window (image_right.name, image_right.wxd, image_right.wyd);
		
		for (v = 0; v < ho; v++)
		{
			for (u = 0; u < wo; u++)
			{
				d = nMaxScan - image_right.wxd;
				
				if (u + d < wo && u + d >= 0)
				{
					p_fltDisparityMap[u - d + v * wo] = nl_disparity_map.neuron_vector[u + v * wo].output.fval + d;
					p_nFrequencyMap[u - d + v * wo]++;
				}
			}
		}
	}

	for (i = 0; i < num_neurons; i++)
		nl_disparity_map.neuron_vector[i].output.fval = p_fltDisparityMap[i] / (float) p_nFrequencyMap[i];

	nVergDisparity = (int) (NeuronOutputsAverage(nl_disparity_map.neuron_vector, wo, ho, BAND) + .5f);

	image_left.wxd = image_right.wxd + nVergDisparity;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);

	for (i = 0; i < num_neurons; i++)
		nl_disparity_map.neuron_vector[i].output.fval -= nVergDisparity;
	
	all_outputs_update ();

	glutPostWindowRedisplay (image_left.win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}*/

// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void input_controler (INPUT_DESC *input, int status)
{
//	char command[256];

	// DEBUG
// 	printf("Funcão: input_controler\n");
	
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nRightEye))
	{
		image_left.wxd += image_right.wxd - image_right.wxd_old;
		image_left.wyd = image_right.wyd + g_nVerticalGap;

		move_input_window (image_right.name, image_right.wxd, image_right.wyd);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		glutPostWindowRedisplay (image_right.win);
		glutPostWindowRedisplay (image_left.win);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nLeftEye))
	{
		image_left.wyd = image_right.wyd + g_nVerticalGap;

		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		glutPostWindowRedisplay (image_left.win);
	}

	input->mouse_button = -1;
}

// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void output_handler(OUTPUT_DESC *output, int status)
{
	printf("output_handler\n");
}

// ----------------------------------------------------------------------------
// Reconstruct -
//
// Entrada:
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void Reconstruct()
{
	//float x, y, z, d;
	float intensity;
	int pixel;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	TPoint *point;

	printf("Reconstruct\n");
	
	point = &(g_map.point[image_right.wyd][image_right.wxd]);

	right_point.x = (double) (image_right.wxd);
	right_point.y = (double) (image_right.wyd);

	left_point.x = (double) (image_left.wxd);
	left_point.y = (double) (image_left.wyd);
	
	world_point = calculateWorldPoint (left_point, right_point);

	point->x = world_point.x;
	point->y = world_point.y;
	point->z = -world_point.z;
	point->dist = sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z);
	printf ("Vergence - x:%3.2f y:%3.2f z:%3.2f - distance: %3.2f\n", world_point.x, world_point.y, world_point.z, sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z));

	pixel = image_right.neuron_layer->neuron_vector[(image_right.wyd * image_right.neuron_layer->dimentions.x) + image_right.wxd].output.ival;
	intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
	g_map.point[image_right.wyd][image_right.wxd].intensity = intensity;

	// Vai vei
	ReconstructByDisparityMap();

	glutPostWindowRedisplay (image_right.win);
	glutPostWindowRedisplay (image_left.win);
	glutPostWindowRedisplay (g_wndMap);
	glutPostWindowRedisplay (g_wndMapTop);

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}

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

	if (g_ReceptiveField == NULL)
	{
		g_ReceptiveField = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(g_ReceptiveField, 5, 1.0);
	}

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	if (g_fltDispAux == NULL)
		g_fltDispAux = (float*)malloc(sizeof(float)*wo*ho);

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

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}


// ----------------------------------------------------------------------------
// MoveXImageLeft - Move o foco da ImageLeft
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MoveXImageLeft()
{
	//char command[256];
	//float fltAvgVerg;

	image_left.wxd += g_nMoveLeft;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	glutPostWindowRedisplay (image_left.win);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);
}

// ----------------------------------------------------------------------------
// MoveVerticalGap - Altera o Gap Vertical da imagem
//					 da esquerda.
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MoveVerticalGap()
{
	//char command[256];
	//float fltAvgVerg;

	image_left.wyd = image_right.wyd + g_nVerticalGap;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	glutPostWindowRedisplay (image_left.win);

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
	int x, y, w, h, i, sample, best_sample = 0, cur_dist, dist;
	
	w = nlDisparityMap->dimentions.x;
	h = nlDisparityMap->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (sample = 0; sample < NUM_SAMPLES; sample++)
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
}



// ----------------------------------------------------------------------------
// new_output_passive_motion - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void 
new_output_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	static INPUT_DESC *image_right = NULL;
	static OUTPUT_DESC *output = NULL;
	NEURON_OUTPUT pixel;
	int r, g, b;
	int w, h;
	int i;
	ASSOCIATION *memory = NULL;

	if (image_right == NULL)	
	{
		image_right = get_input_by_name ("image_right");
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
		
	map_v1_to_image (&(image_right->green_cross_x), &(image_right->green_cross_y), 
		         image_right->ww, image_right->wh, (int) wx, (int) wy, output->ww, output->wh, 
			 image_right->wxd, image_right->wyd, (float) output->wh / (float) (output->wh - 1), LOG_FACTOR);

	// Updates the memory viewer
	MemViewerClearMemory();
	if ((wx >= 0) && (wx < w) && (wy >= 0) && (wy < h))
	{
		memory = output->neuron_layer->neuron_vector[w * (int) (wy) + (int) (wx)].memory;		
		for (i = 0; (i < NEURON_MEMORY_SIZE) && (memory[i].pattern != NULL); i++)
			MemViewerUpdateMemory (memory[i].pattern, memory[i].associated_value.fval / (float) DISPARITY_RANGE);
	}
	MemViewerRefresh();

	glutPostWindowRedisplay (image_right->win);		 
}


// ----------------------------------------------------------------------------
// PrintAllDispData - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void 
PrintAllDispData ()
{
	OUTPUT_DESC *output = NULL;  
	int w, h, x, y;
	int i, nIndex;

	if ((output = get_output_by_win (glutGetWindow ())) == NULL)
		return;

	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	x = output->wxd;
	y = output->wyd;
	
	if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
	{
		printf("MT gaussian output and confidence of point x = %d , y = %d:\n", x, y);
		nIndex = (w * y) + x;
		for (i = 0; i < nAllSamples; i++)
			printf("disp = %3d   output = %f   conf = %f\n", g_AllDispData->samples[i][nIndex].pos, g_AllDispData->samples[i][nIndex].val, g_AllDispData->samples[i][nIndex].conf);
	}
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
	char key;
	INPUT_DESC *image_right = NULL;
	OUTPUT_DESC *output = NULL;
	FILTER_DESC *illuminance_filter_desc, *reflectance_filter_desc;
	float avg_val, previous_avg_val, dif_val;
	int i;

	key = key_value[0];
	switch (key)
	{
		// Initializes the memory viewer module
		case 'T':	
		{
			MemViewerSetParameteri (MEM_VIEWER_NEURON_MEMORY_SIZE, NEURON_MEMORY_SIZE);
			MemViewerSetParameteri (MEM_VIEWER_MEMORY_PATTERN_SIZE, SYNAPSES);
			MemViewerInitialize ();
		}
		break;
		
		// Tests the disparity map neuron connections
		case 't':
		{
			test_connections ("out_disparity_map");
		}
		break;
		
		// Initializes the viewer module
		case 'K':	
		case 'k':
		{
			ViewerSetParameterf (VIEWER_FOVY, FOVY);
			ViewerSetParameterf (VIEWER_FAR, FAR);
			ViewerSetParameterf (VIEWER_NEAR, NEAR);
			ViewerSetParameterf (VIEWER_FOCUS, FOCUS);		
			ViewerSetParameteri (VIEWER_IMAGE_WIDTH, INPUT_WIDTH);
			ViewerSetParameteri (VIEWER_IMAGE_HEIGHT, INPUT_HEIGHT);
			ViewerInitialize ();
		}
		break;
		
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
			glutIdleFunc ((void (* ) (void))set_vergence);
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

		case 'd':
		{
			calculate_disparity (&nl_disparity_map, image_left.wxd);
			glutPostWindowRedisplay (out_disparity_map.win);
		}
		break;

		case 'A':
		case 'a':
		{
			SalvaTMap();
			printf("Reconstrucao salva.\n");
			glutPostWindowRedisplay (g_wndMap);
			glutPostWindowRedisplay (g_wndMapTop);
		}
		break;

		case 'Q':
		case 'q':
		{
			LeTMap();
			printf("Reconstrucao lida.\n");
			glutPostWindowRedisplay (g_wndMap);
			glutPostWindowRedisplay (g_wndMapTop);
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
			image_right = get_input_by_name ("image_right");
			image_right->green_cross = 1;
						
			if ((output = get_output_by_win (glutGetWindow ())) != NULL)
			{
				glutSetWindow (output->win);
			
				glutPassiveMotionFunc (new_output_passive_motion);

				glutPostWindowRedisplay (image_right->win);
			}
		}
		break;

		case 'Z':
		{
			image_right = get_input_by_name ("image_right");
			image_right->green_cross = 0;
			
			if ((output = get_output_by_win (glutGetWindow ())) != NULL)
			{
				glutSetWindow (output->win);
			
				glutPassiveMotionFunc (output_passive_motion);
			
				glutPostWindowRedisplay (image_right->win);
			}
		}
		break;
		
		case 'B':
		case 'b':
		{
			PrintAllDispData();
		}
		break;
		
		case 'I':
		case 'i':
		{
			illuminance_filter_desc = get_filter_by_name ("nl_illuminance_right_filter");
			if (illuminance_filter_desc == NULL)
			{
				printf ("No filter producing neuron layer nl_illuminance_right\n");
				break;
			}
			reflectance_filter_desc = get_filter_by_name ("nl_reflectance_right_filter");
			if (reflectance_filter_desc == NULL)
			{
				printf ("No filter producing neuron layer nl_reflectance_right\n");
				break;
			}

			for (i = 0; i < 100; i++)
			{
				previous_avg_val = * (float *) illuminance_filter_desc->private_state;
				filter_update (illuminance_filter_desc);
				filter_update (reflectance_filter_desc);
				avg_val = * (float *) illuminance_filter_desc->private_state;
				if ((avg_val == 0.0) || (previous_avg_val == 0.0))
				{
					dif_val = 0.0;
					break;
				}
				dif_val = (avg_val / previous_avg_val) - 1.0;
				dif_val = (dif_val > 0.0 ? dif_val : - dif_val);
				if (dif_val < 1.0e-6)
					break;
			}
			
			printf ("Average illuminance difference after %d iterations: %g%%\n", i, dif_val * 100.0); 
		}
		break;
	}
}
