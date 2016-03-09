#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include <stdlib.h>
#include "forms.h"
#include "mae_control.h"
#include "calibrate.h"
#include "filter.h"

#include "mapping.h"

#include "robot_user_functions.h"

//#define NL_MINUS			"nl_minus_085"
#define NL_MINUS			"nl_gaussian_minus_085"
#define NL_GABOR_LEFT		"nl_gabor_085_left"
#define NL_GABOR_RIGHT		"nl_gabor_085_right"
#define NL_MT				"nl_mt"
#define NL_GAUSSIAN_MT		"nl_gaussian_mt"
#define NL_DISPARITY_MAP	"nl_disparity_map"

// Variaveis globais
int g_nLetfEye, g_nRightEye;
SOCKET g_SocketLeft, g_SocketRight;
char chrBuffer[MAX_PACKAGE];	// O tamanho maximo de um pacote TCP �1500 bytes. Foi colocado 1600 apenas para dar uma folga
byte pImage[IMAGE_WIDTH * IMAGE_HEIGHT];
int nTamImageLeft, nTamImageRight;
int g_nColorImage = COLOR_MONO_256;

int g_nDeslocamento = 0;

INPUT_DESC *g_pInputLeft;
INPUT_DESC *g_pInputRight;

double dblTempo1, dblTempo2;
int nFrame = 0;
int nTotalFrame = 0;

int g_Status;

int g_nMoveLeft;

int g_nVerticalGap = VERT_GAP;

RECEPTIVE_FIELD_DESCRIPTION *g_ReceptiveField;

// ***** Variaveis para Mapping *****
TMap g_map;
GLint g_wndMap = 0;
GLint g_wndMapTop = 0;
float g_fCenterX, g_fCenterY, g_fCenterZ;	// Centro de rotacao

int g_nMouseX, g_nMouseY;
GLfloat rot, w1, w2, w3;					// Variables for rotation calculation
GLint prevx, prevy;							// Remember previous x and y positions

GLfloat w = MAP_WINDOW_WIDTH;
GLfloat h = MAP_WINDOW_HEIGHT;

GLfloat	g_fltFovW = (IMAGE_WIDTH / 2.0) * MAX_Z_VIEW / LAMBDA;
GLfloat	g_fltFovH = (IMAGE_HEIGHT / 2.0) * MAX_Z_VIEW / LAMBDA;
// **********************************

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
// SalvaImagens - Salva as imagens
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SalvaImagens()
{
	FILE *file;
	int x, y;

	if (GET_IMAGE_FROM_SOCKET)
	{
		file = fopen(FILE_IMAGE_LEFT,"w");
	
		for (x = 0; x < IMAGE_WIDTH; x++)
		{
			for (y = 0; y < IMAGE_HEIGHT; y++)
			{
				fprintf(file,"%c", (char)g_pInputLeft->image[3 * (x + y*g_pInputLeft->tfw)]);
			}
		}
		
		fclose(file);
		
		file = fopen(FILE_IMAGE_RIGHT,"w");
	
		for (x = 0; x < IMAGE_WIDTH; x++)
		{
			for (y = 0; y < IMAGE_HEIGHT; y++)
			{
				fprintf(file,"%c", (char)g_pInputRight->image[3 * (x + y*g_pInputRight->tfw)]);
			}
		}
		fclose(file);
	}

	glutIdleFunc ((void (* ) (void))fl_check_forms);
}

// ----------------------------------------------------------------------------
// Connect - Cria o socket e conecta com o servidor
//           de imagens.
//
// Entrada: strAddr - Endereco do servidor
//
// Saida: Socket. Retorna 0 caso falhe.
// ----------------------------------------------------------------------------
int Connect(char *strAddr)
{
	struct sockaddr_in addr;
	struct hostent *he;
	SOCKET r = 0;

	// Cria o socket
	r = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == r)
	{
		printf("Could not create socket.\n");
		return 0;
	}

	// resolve localhost to an IP
	if ((he = gethostbyname(strAddr)) == NULL)
	{
		puts("Error resolving hostname...\n");
		return 0;
	}

	memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOCKET_PORT);

	if(connect(r, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		printf("Cannot connect to server\n");
		closesocket(r);
		return 0;
	}
	printf("Connected.\n");

	return r;
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
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
	
	// The components (a1,a2,a3) of the axis of5 rotation w are
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
	int i, j;

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
		TMapInitialize(&g_map, IMAGE_WIDTH, IMAGE_HEIGHT, FOCAL_DISTANCE);
	
		// Calcula o centro geometrico do TMap
//		TMapGetMassCenter(g_map, &g_fCenterX, &g_fCenterY, &g_fCenterZ);
//		printf("Centro de Massa - x: %f - y: %f - z: %f\n", g_fCenterX, g_fCenterY, g_fCenterZ);
//		TMapGetGeoCenter(g_map, &g_fCenterX, &g_fCenterY, &g_fCenterZ);
		g_fCenterX = 0.0;
		g_fCenterY = 0.0;
		g_fCenterZ = -80.0;
//		printf("Centro Geometrico - x: %f - y: %f - z: %f\n", g_fCenterX, g_fCenterY, g_fCenterZ);
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
	int i, j;

	if (g_wndMapTop == 0)
	{
		// Inicializa o modo de exibicao
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

		// Posicionamento e tamanho da janela
		glutInitWindowSize(230, 400);
		glutInitWindowPosition(0, 300);

		g_wndMapTop = glutCreateWindow("Mapping Top");
		
		glutKeyboardFunc (keyboard);

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

	printf("%s\n",input->name);

	// Conecta com o servidor
	if (GET_IMAGE_FROM_SOCKET)
	{
		if (VISION_STERO)
		{
			if (nInputImage == INPUT_IMAGE_LEFT)
			{
				g_pInputLeft = input;
				if ((g_SocketLeft = Connect(HOST_ADDR_LEFT)) == 0)
					ERRO_CONEXAO
			}
			else
			{
				g_pInputRight = input;
				if ((g_SocketRight = Connect(HOST_ADDR_RIGHT)) == 0)
					ERRO_CONEXAO
			}
		}
		else
		{
			if (nInputImage == INPUT_IMAGE_LEFT)
			{
				g_pInputLeft = input;
				if ((g_SocketLeft = Connect(HOST_ADDR_MONO)) == 0)
					ERRO_CONEXAO
			}
			else
			{
				g_pInputRight = input;
				g_SocketRight = g_SocketLeft;
			}
		}
	}
	else
	{
		if (nInputImage == INPUT_IMAGE_LEFT)
			g_pInputLeft = input;
		else
			g_pInputRight = input;
	}

//	make_pinfo(input, IMAGE_WIDTH, IMAGE_HEIGHT);
	robot_make_input_image (input);
	init (input);

	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);

	if (strcmp(input->name,INPUT_NAME_LEFT) == 0)
		g_nLetfEye = input->win;
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
	int nBytesPacote, nBytesImagem;
	int nTamImage;
	int *n_pAux;
	int i, x, y, j;
	int pPonto;
	FILE *file;
	int nDisparity;
	int xi, yi, xf, yf;
	int nFaixa, nCol;

	// Recebe o tamanho da Imagem
	switch(g_nColorImage)
	{
		case COLOR_MONO_256:
			send(*s, MSG_SNAP_MONO_256, strlen(MSG_SNAP_MONO_256), 0);
		break;

		case COLOR_332:
			send(*s, MSG_SNAP_332, strlen(MSG_SNAP_332), 0);
		break;

		case COLOR_MONO_8:
			send(*s, MSG_SNAP_MONO_8, strlen(MSG_SNAP_MONO_8), 0);
		break;
	}

	if (GET_IMAGE_FROM_SOCKET)
	{
		nBytesPacote = recv(*s, chrBuffer, 4, 0);
		n_pAux = (int*)chrBuffer;
		nTamImage = *n_pAux;
	
		// Recebe os bits da imagem
		nBytesImagem = 0;
		while (nBytesImagem < nTamImage)
		{
			//nBytesPacote = recv(*s, chrBuffer, nTamImage, 0);
			nBytesPacote = read(*s, &pImage[nBytesImagem], nTamImage);
			nBytesImagem += nBytesPacote;
		}
	}
	else
	{
		nTamImage = IMAGE_WIDTH * IMAGE_HEIGHT;
		if (input == g_pInputLeft)
			file = fopen(FILE_IMAGE_LEFT,"r");
		else
			file = fopen(FILE_IMAGE_RIGHT,"r");

		fread ((void*)pImage, 1, nTamImage, file);
		fclose(file);
	}

	// Coloca a imagem na estrutura da MAE
	if (GET_IMAGE_FROM_SOCKET)
	{
		switch (g_nColorImage)
		{
			case COLOR_MONO_256:
			{
				for (x = 0; x < IMAGE_WIDTH; x++)
				{
					for (y = 0; y < IMAGE_HEIGHT; y++)
					{
						pPonto = pImage[x + y * IMAGE_WIDTH];
						input->image[3 * (x + y*input->tfw) + 0] = pPonto;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto;
					}
				}
			}
			break;
	
			case COLOR_332:
			{
				for (x = 0; x < IMAGE_WIDTH; x++)
				{
					for (y = 0; y < IMAGE_HEIGHT; y++)
					{
						pPonto = pImage[x + y * IMAGE_WIDTH];
						input->image[3 * (x + y*input->tfw) + 0] = pPonto & 0x00E0;
						input->image[3 * (x + y*input->tfw) + 1] = (pPonto & 0x001C) << 3;
						input->image[3 * (x + y*input->tfw) + 2] = (pPonto & 0x0003) << 6;
					}
				}
			}
			break;
	
			case COLOR_MONO_8:
			{
				for (x = 0; x < IMAGE_WIDTH; x++)
				{
					for (y = 0; y < IMAGE_HEIGHT; y++)
					{
						pPonto = pImage[x + y * IMAGE_WIDTH];
						input->image[3 * (x + y*input->tfw) + 0] = pPonto * 32;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto * 32;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto * 32;
					}
				}
			}
		}
	}
	else
	{
		// Coloca a imagem na estrutura da MAE
		if (!ARTIFICIAL_IMAGE)
		{
			i = 0;
			for (x = 0; x < IMAGE_WIDTH; x++)
			{
				for (y = 0; y < IMAGE_HEIGHT; y++)
				{
					pPonto = pImage[i++];
					input->image[3 * (x + y*input->tfw) + 0] = pPonto;
					input->image[3 * (x + y*input->tfw) + 1] = pPonto;
					input->image[3 * (x + y*input->tfw) + 2] = pPonto;
				}
			}
		}
		else
		{
			// Limpa a imagem
			for (x = 0; x < IMAGE_WIDTH; x++)
			{
				for (y = 0; y < IMAGE_HEIGHT; y++)
				{
					input->image[3 * (x + y*input->tfw) + 0] = 96;
					input->image[3 * (x + y*input->tfw) + 1] = 96;
					input->image[3 * (x + y*input->tfw) + 2] = 96;
				}
			}

			nFaixa = 40;

			// Primeiro Quadrado
			nDisparity = 32;
			xi = 80;
			yi = 100;
			xf = xi + 30;
			yf = yi + 40;
			nCol = 0;
			pPonto = 0;
			if (input == g_pInputLeft)
			{
				for (x = (xi + nDisparity); x < (xf + nDisparity); x++)
				{
					if ((nCol % nFaixa) == 0)
						pPonto = (pPonto == 0) ? 1 : 0;

					for (y = yi; y < yf; y++)
					{
						input->image[3 * (x + y*input->tfw) + 0] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto * 255;
					}
					nCol++;
				}
			}
			else
			{
				for (x = xi; x < xf; x++)
				{
					if ((nCol % nFaixa) == 0)
						pPonto = (pPonto == 0) ? 1 : 0;

					for (y = yi; y < yf; y++)
					{
						input->image[3 * (x + y*input->tfw) + 0] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto * 255;
					}
					nCol++;
				}
			}
	
			// Segundo Quadrado
			nDisparity = 16;
			xi = 215;
			yi = 100;
			xf = xi + 30;
			yf = yi + 40;
			nCol = 0;
			pPonto = 0;
			if (input == g_pInputLeft)
			{
				for (x = (xi + nDisparity); x < (xf + nDisparity); x++)
				{
					if ((nCol % nFaixa) == 0)
						pPonto = (pPonto == 0) ? 1 : 0;

					for (y = yi; y < yf; y++)
					{
						input->image[3 * (x + y*input->tfw) + 0] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto * 255;
					}
					nCol++;
				}
			}
			else
			{
				for (x = xi; x < xf; x++)
				{
					if ((nCol % nFaixa) == 0)
						pPonto = (pPonto == 0) ? 1 : 0;

					for (y = yi; y < yf; y++)
					{
						input->image[3 * (x + y*input->tfw) + 0] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 1] = pPonto * 255;
						input->image[3 * (x + y*input->tfw) + 2] = pPonto * 255;
					}
					nCol++;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// SendSnap - Envia uma mensage para capturar a imagem
//
// Entrada: s - Socket no qual ser�lida a imagem
//
// Saida: int - Tamanho da imagem a ser capturada
// ----------------------------------------------------------------------------
int SendSnap(SOCKET *s)
{
	int *nTamImagem;

	send(*s, MSG_SNAP_MONO_256, strlen(MSG_SNAP_MONO_256), 0);
	recv(*s, chrBuffer, 4, 0);
	nTamImagem = (int*)chrBuffer;
	return (*nTamImagem);
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
	float fFrameRate;
	int x, y;
	int i;
	int nInputImage;
	struct timeval tv1;
	struct timeval tv2;
	static int bInitIR = 1;
//	OUTPUT_DESC *output;
//	FILTER_DESC *filter;

	// DEBUG
//	printf("FUNCAO: input_generator\n");

	// Define qual a imagem (direita ou esquerda)
	if (strcmp(input->name,INPUT_NAME_LEFT)==0)
		nInputImage = INPUT_IMAGE_LEFT;
	else
		nInputImage = INPUT_IMAGE_RIGHT;

	// Inicializacao executada apenas uma vez por janela de entrada
	if (input->win == 0)
	{
		InitWindow(input, nInputImage);
		g_Status = ST_GET_IMAGE;
	}

	if ((nFrame < remaining_steps) && (running))
	{
		gettimeofday(&tv1,NULL);
		nFrame = remaining_steps;
		nTotalFrame = remaining_steps;
		dblTempo1 = (double)tv1.tv_sec + (double)tv1.tv_usec*1e-6;

		g_Status = ST_GET_IMAGE;
	}

	// Busca a imagem
	if (g_Status == ST_GET_IMAGE)
	{
		if (nInputImage == INPUT_IMAGE_LEFT)
		{
			// Envia a erquisi�o de imagem quando apenas na
			// leitura do olho esquerdo para poder diminuir
			// o gap entre a captura das imagens
	//        nTamImageLeft = SendSnap(&g_SocketLeft);
	//        nTamImageRight = SendSnap(&g_SocketRight);
	
			GetImage(input, &g_SocketLeft);
	//        GetImage(input, &g_SocketLeft, nTamImageLeft);
		}
		else
		{
			GetImage(input, &g_SocketRight);
	//        GetImage(input, &g_SocketRight, nTamImageRight);
			nFrame--;
		}
		check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_neurons (input);
		update_input_image (input);
	}

	// Atualiza os filtros
	if (((input->win == g_nLetfEye) || (input->win == g_nRightEye)) && (input->win != 0))
	{
		if ((status == MOVE) && (!running) && (input->win != g_nRightEye))
		{
			all_filters_update ();
			all_outputs_update ();
		}
	}

	// Calcula o Frame Rate
	if ((nFrame == 0) && (running))
	{
		g_Status = ST_WAIT;
		gettimeofday(&tv2,NULL);
		dblTempo2 = (double)tv2.tv_sec + (double)tv2.tv_usec*1e-6;
		printf("Tempo: %2.2lfseg - FrameRate: %2.2lf\n",dblTempo2 - dblTempo1, (double)nTotalFrame / (dblTempo2 - dblTempo1));
	}
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
	NEURON_LAYER *nl_DisparityMap;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	int i, j, u, v;
	int w, h, wi, hi, xi, yi;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity;
	float intensity;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;

	nl_DisparityMap = get_neuron_layer_by_name(NL_DISPARITY_MAP);

	// Dimensoes do cortex (disparity map)
	h = nl_DisparityMap->dimentions.y;
	w = nl_DisparityMap->dimentions.x;

	// Dimensoes da imagem de entrada
	hi = g_pInputRight->neuron_layer->dimentions.y;
	wi = g_pInputRight->neuron_layer->dimentions.x;

	// NAO SEI PORQUE.
	// PRECISA DISTO PARA FUNCIONAR DEVIDO A ALTERACAO FEITA POR DIJALMA
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = g_pInputRight->wxd - g_pInputRight->wx;
		y_center = g_pInputRight->wyd - g_pInputRight->wy;
		x_center_left = g_pInputLeft->wxd - g_pInputLeft->wx;
		y_center_left = g_pInputLeft->wyd - g_pInputLeft->wy;
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

			pixel = g_pInputRight->neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;

			right_point.x = (double) (xi);
			right_point.y = (double) (yi);

			switch (nl_DisparityMap->output_type)
			{
				case GREYSCALE:
				{
					fltDisparity = (float) nl_DisparityMap->neuron_vector[w * v + u].output.ival;
				}
				break;
				
				case GREYSCALE_FLOAT:
				{
					fltDisparity = nl_DisparityMap->neuron_vector[w * v + u].output.fval;
				}
				break;
			}

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			left_point.x = (double) (xi + (fltDisparity * DISPARITY_FACTOR));
			left_point.y = (double) (yi);

			world_point = calculate_world_point (left_point, right_point, g_pInputRight->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);

			g_map.point[yi][xi].x = world_point.x;
			g_map.point[yi][xi].y = world_point.y;
			g_map.point[yi][xi].z = -world_point.z;

			pixel = g_pInputLeft->neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			intensity += (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
			intensity /= 2;

			g_map.point[yi][xi].intensity = intensity;
			g_map.point[yi][xi].dist = sqrt((world_point.x * world_point.x) + (world_point.y * world_point.x) + (world_point.z * world_point.z));
//			printf ("[%03d,%03d] - x:%3.2f y:%3.2f z:%3.2f - intensity: %f\n", xi, yi, world_point.x, world_point.y, world_point.z, intensity);
// 			printf ("Left [%03.0lf,%03.0lf] - Right [%03.0lf,%03.0lf]", left_point.x, left_point.y, right_point.x, right_point.y);
// 			if ((g_pInputRight->wxd == (int)(left_point.x - nDispXVergOffset)) && (g_pInputRight->wyd == left_point.y))
// 				printf(" *****");
// 			printf("\n");
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

/*!
*********************************************************************************
* Function: compute_gaussian_kernel							*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

void compute_dif_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int nKernelSize, float fltSigma1, float fltSigma2)
{
	int i, x, y, num_points = 0;
	double k1, k2;
	POINT *points;
	int nRadius;
	FILE *file;
	char strAux[64];

	points = (POINT *) alloc_mem (sizeof (POINT) * nKernelSize * nKernelSize);

	file = fopen("dif_gaussian.out","w");

	k1 = 1.4;
	k2 = 0.6;
	nRadius = nKernelSize / 2;
	for (y = -nRadius; y <= nRadius; y++)
	{
		for (x = -nRadius; x <= nRadius; x++)
		{
			points[num_points].x = x;
			points[num_points].y = y;
			points[num_points].gaussian = k1 * exp (-(x * x + y * y) / (2.0 * fltSigma1 * fltSigma1)) - (k2 * exp (-(x * x + y * y) / (2.0 * fltSigma2 * fltSigma2)));

			sprintf(strAux, "%d %d %.3f\n", x, y, points[num_points].gaussian);
			for (i = 0; strAux[i] != '\n'; i++)
				if (strAux[i] == ',') strAux[i] = '.';
			fprintf (file, "%s", strAux);
			printf("%1.03f ", points[num_points].gaussian);
			
			num_points++;
		}
		printf("\n");
	}

	fclose(file);

	receptive_field_descriptor->num_points = num_points;
	receptive_field_descriptor->points = points;
}

// ----------------------------------------------------------------------------
// set_vergence - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void set_vergence (TPoint *point)
{
	int nMaxScan, nMinScan;
	NEURON_LAYER *nl_minus, *nl_mt, *nl_gaussian_mt, *nl_disparity_map;
//	OUTPUT_DESC	*out_disparity_map;
	FILE *file;
	int i, x, y, x_mt, y_mt, nIndex;
	int wo, ho, wo_mt, ho_mt;
	char strAux[64];
	float fltSumOutputCells;
	float fltMin, fltMax, fltAux;
	int nVergLeft;
	int nAux, nTotalScan;

	file = fopen("verg.out","w");

	// Buscar a neuron layer para efetuar a vergencia
	nl_minus = get_neuron_layer_by_name (NL_MINUS);
	nl_mt = get_neuron_layer_by_name (NL_MT);
	nl_gaussian_mt = get_neuron_layer_by_name (NL_GAUSSIAN_MT);
	nl_disparity_map = get_neuron_layer_by_name(NL_DISPARITY_MAP);

	wo = nl_minus->dimentions.x;
	ho = nl_minus->dimentions.y;
	wo_mt = nl_mt->dimentions.x;
	ho_mt = nl_mt->dimentions.y;

	nMinScan = g_pInputRight->wxd;
	nMaxScan = g_pInputRight->wxd + (g_pInputRight->neuron_layer->dimentions.x / 5);
	if (nMaxScan > g_pInputRight->neuron_layer->dimentions.x)
		nMaxScan = g_pInputRight->neuron_layer->dimentions.x;
	nTotalScan = nMaxScan-nMinScan;
	printf("nMinScan: %d - nMaxScan: %d - nTotalScan: %d\n", nMinScan, nMaxScan, nTotalScan);


	// Faz a vergencia acumulando os dados na MT
	fltMin = FLT_MAX;
	fltMax = FLT_MIN;
	nAux = 0;
	for (g_pInputLeft->wxd = nMinScan; g_pInputLeft->wxd < nMaxScan; g_pInputLeft->wxd++)
	{
		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

//		printf("----- %d -----\n", g_pInputLeft->wxd);
		fltSumOutputCells = 0.0;
		for (y = 0; y < ho; y++)
		{
			if ((y % 2) == 0)
				y_mt = (nTotalScan * y) + nAux;
			else
				y_mt = ((nTotalScan * (y + 1)) - nAux) - 1;

			for (x = 0; x < wo; x++)
			{
				nIndex = y * wo + x;
				x_mt = x;
				fltSumOutputCells += nl_minus->neuron_vector[nIndex].output.fval;
//				printf("nl(%d,%d)-->MT(%d,%d)\n", x, y, x_mt, y_mt);
				nl_mt->neuron_vector[y_mt * wo_mt + x_mt].output.fval = nl_minus->neuron_vector[nIndex].output.fval;
			}
		}
		nAux++;

//		if (fltSumOutputCells < fltMin)
		if (fltSumOutputCells > fltMax)
		{
//			fltMin = fltSumOutputCells;
			fltMax = fltSumOutputCells;
			nVergLeft = g_pInputLeft->wxd;
		}

		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, fltSumOutputCells);
		for (i = 0; strAux[i] != '\n'; i++)
			if (strAux[i] == ',') strAux[i] = '.';
		fprintf (file, "%s", strAux);
	}

	g_pInputLeft->wxd = nVergLeft;
	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	printf("Vergence Point: Left (%d,%d) - Right (%d, %d)\n", g_pInputLeft->wxd, g_pInputLeft->wyd, g_pInputRight->wxd, g_pInputRight->wyd);

	fclose(file);

	// Passa uma gaussiana sobre a camada MT gerando uma nova camada
	if (g_ReceptiveField == NULL)
	{
		g_ReceptiveField = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
//		compute_gaussian_kernel(g_ReceptiveField, 9, 2.0);
		compute_dif_gaussian_kernel(g_ReceptiveField, 15, 1.3, 3.0);
	}
	for (y = 0; y < ho_mt; y++)
	{
		for (x = 0; x < wo_mt; x++)
		{
//			nl_gaussian_mt->neuron_vector[y * wo + x].output.fval = nl_mt->neuron_vector[y * wo + x].output.fval;
			nl_gaussian_mt->neuron_vector[y * wo + x].output.fval = apply_gaussian_kernel(g_ReceptiveField, nl_mt, x, y);
		}
	}

	// Passa os dados da MT_GAUSSIAN para o disparity_map
	for (y = 0; y < ho; y++)
	{
		y_mt = (nTotalScan * y);

		for (x = 0; x < wo; x++)
		{
			nIndex = y * wo + x;
			x_mt = x;
			fltMin = FLT_MAX;
			fltMax = FLT_MIN;

//			printf("\nnl_disp(%d,%d) --> mt (%d,[", x, y, x_mt);

			if ((y % 2) == 0)
			{
				nAux = nMinScan;
				for (i = y_mt; i < (y_mt + nTotalScan); i++)
				{
					fltAux = nl_gaussian_mt->neuron_vector[i * wo_mt + x_mt].output.fval;
//					if (fltAux < fltMin)
					if (fltAux > fltMax)
					{
//						fltMin = fltAux;
						fltMax = fltAux;
						nl_disparity_map->neuron_vector[nIndex].output.fval = nAux - nVergLeft;
					}
					nAux++;
//					printf("%d, ", i);
				}
			}
			else
			{
				nAux = nMinScan;
				for (i = (y_mt + nTotalScan - 1); i >= y_mt; i--)
				{
					fltAux = nl_gaussian_mt->neuron_vector[i * wo_mt + x_mt].output.fval;
//					if (fltAux < fltMin)
					if (fltAux > fltMax)
					{
//						fltMin = fltAux;
						fltMax = fltAux;
						nl_disparity_map->neuron_vector[nIndex].output.fval = nAux - nVergLeft;
					}
					nAux++;
//					printf("%d, ", i);
				}
			}
//			printf("])\n");
		}
	}

	update_output_image(get_output_by_neural_layer(nl_disparity_map));
	
	glutPostWindowRedisplay (g_pInputLeft->win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
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

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nRightEye))
	{
		g_pInputLeft->wxd += g_pInputRight->wxd - g_pInputRight->wxd_old;
		g_pInputLeft->wyd = g_pInputRight->wyd + g_nVerticalGap;

		move_input_window (g_pInputRight->name, g_pInputRight->wxd, g_pInputRight->wyd);
		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

/*
		set_vergence(NULL);
		set_vergence(&(g_map.point[g_pInputRight->wyd][g_pInputRight->wxd]));

		pixel = g_pInputRight->neuron_layer->neuron_vector[(g_pInputRight->wyd * g_pInputRight->neuron_layer->dimentions.x) + g_pInputRight->wxd].output.ival;
		intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
		g_map.point[g_pInputRight->wyd][g_pInputRight->wxd].intensity = intensity;

		// Vai vei
		ReconstructByDisparityMap();

*/
//		MapImageToV1(g_pInputRight->wxd, g_pInputRight->wyd, IMAGE_WIDTH, IMAGE_HEIGHT, NL_WIDTH, NL_HEIGHT, LOG_FACTOR);

		glutPostWindowRedisplay (g_pInputRight->win);
		glutPostWindowRedisplay (g_pInputLeft->win);
//		glutPostWindowRedisplay (g_wndMap);
//		glutPostWindowRedisplay (g_wndMapTop);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nLetfEye))
	{
		g_pInputLeft->wyd = g_pInputRight->wyd + g_nVerticalGap;

		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

		glutPostWindowRedisplay (g_pInputLeft->win);
	}

	input->mouse_button = -1;
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
	float x, y, z, d;
	float intensity;
	int pixel;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	TPoint *point;

	printf("Reconstruct\n");
	
	point = &(g_map.point[g_pInputRight->wyd][g_pInputRight->wxd]);

	right_point.x = (double) (g_pInputRight->wxd);
	right_point.y = (double) (g_pInputRight->wyd);

	left_point.x = (double) (g_pInputLeft->wxd);
	left_point.y = (double) (g_pInputLeft->wyd);
	
	world_point = calculate_world_point (left_point, right_point, g_pInputRight->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);

	point->x = world_point.x;
	point->y = world_point.y;
	point->z = -world_point.z;
	point->dist = sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z);
	printf ("Vergence - x:%3.2f y:%3.2f z:%3.2f - distance: %3.2f\n", world_point.x, world_point.y, world_point.z, sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z));

	pixel = g_pInputRight->neuron_layer->neuron_vector[(g_pInputRight->wyd * g_pInputRight->neuron_layer->dimentions.x) + g_pInputRight->wxd].output.ival;
	intensity = (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
	g_map.point[g_pInputRight->wyd][g_pInputRight->wxd].intensity = intensity;

	// Vai vei
	ReconstructByDisparityMap();

	glutPostWindowRedisplay (g_pInputRight->win);
	glutPostWindowRedisplay (g_pInputLeft->win);
	glutPostWindowRedisplay (g_wndMap);
	glutPostWindowRedisplay (g_wndMapTop);

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
	char command[256];
	float fltAvgVerg;

	g_pInputLeft->wxd += g_nMoveLeft;
	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	glutPostWindowRedisplay (g_pInputLeft->win);

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
	char command[256];
	float fltAvgVerg;

	g_pInputLeft->wyd = g_pInputRight->wyd + g_nVerticalGap;
	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	glutPostWindowRedisplay (g_pInputLeft->win);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);
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
	OUTPUT_DESC *output;

	key = key_value[0];
	switch (key)
	{
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

		case 'M':
		case 'm':
		{
			g_nColorImage = COLOR_MONO_256;
		}
		break;

		case 'R':
		case 'r':
		{
			glutIdleFunc ((void (* ) (void))Reconstruct);
		}
		break;

		case 'V':
		case 'v':
		{
			glutIdleFunc ((void (* ) (void))set_vergence);
		}
		break;

		case 'G':
		case 'g':
		{
			FOCAL_DISTANCE += 10.0;
			printf("FOCAL_DISTANCE = %f\n", FOCAL_DISTANCE);
		}
		break;

		case 'H':
		case 'h':
		{
			FOCAL_DISTANCE -= 10.0;
			printf("FOCAL_DISTANCE = %f\n", FOCAL_DISTANCE);
		}
		break;

		case 'J':
		case 'j':
		{
			CAMERA_DISTANCE += 0.05;
			printf("CAMERA_DISTANCE = %f\n", CAMERA_DISTANCE);
		}
		break;

		case 'K':
		case 'k':
		{
			CAMERA_DISTANCE -= 0.05;
			printf("CAMERA_DISTANCE = %f\n", CAMERA_DISTANCE);
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

		case 'Y':
		{
			g_nDeslocamento += 5;
			printf("nDeslocamento = %d\n", g_nDeslocamento);
		}
		break;

		case 'y':
		{
			g_nDeslocamento += 1;
			printf("nDeslocamento = %d\n", g_nDeslocamento);
		}
		break;

		case 'T':
		{
			g_nDeslocamento -= 5;
			printf("nDeslocamento = %d\n", g_nDeslocamento);
		}
		break;

		case 't':
		{
			g_nDeslocamento -= 1;
			printf("nDeslocamento = %d\n", g_nDeslocamento);
		}
		break;
	}
}
