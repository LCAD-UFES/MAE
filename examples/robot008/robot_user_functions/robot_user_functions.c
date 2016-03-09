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

#define NL_MINUS			"nl_minus_085"
#define NL_GABOR_LEFT		"nl_gabor_085_left"
#define NL_GABOR_RIGHT		"nl_gabor_085_right"
#define NL_DISPARITY_MAP	"nl_disparity_map"
#define DISP_FILTER_NAME	"nl_disparity_map_filter"
#define OUT_DISPARITY_MAP	"out_disparity_map"

// Variaveis globais
int g_nLetfEye, g_nRightEye;
SOCKET g_SocketLeft, g_SocketRight;
char chrBuffer[MAX_PACKAGE];   // O tamanho maximo de um pacote TCP �1500 bytes. Foi colocado 1600 apenas para dar uma folga
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

float *g_fltDispAux;

RECEPTIVE_FIELD_DESCRIPTION receptive_field_descriptor;

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

int left_eye, right_eye;
 
int correction     = 0;
int current_kernel = 0;
int side_move      = 0;

int current_output;
int current_input;

int flagExecuteDispFilter = 0;

enum _plot_type
{
	PLOT,
	SPLOT
};

typedef enum _plot_type PLOT_TYPE;

const char * const name_plot_type[] = {"plot", "splot"};

enum _plot_style
{
	LINES,
	POINTS,
	LINESPOINTS,
	IMPULSES,
 	DOTS 
};

typedef enum _plot_style PLOT_STYLE;

const char * const name_plot_style[] = {"lines", "points", "linespoints", "impulses", "dots"};

struct _gnuplot_parameters_desc
{
    PLOT_TYPE typePlot;
    char *data_filename;
    int x_field_position;
    int y_field_position;
    int z_field_position;
    char *title;
    PLOT_STYLE plot_style;
}; 
 
typedef struct _gnuplot_parameters_desc GNUPLOT_PARAMETERS_DESC; 

struct _gnuplot_parameters_list 
    { 
	GNUPLOT_PARAMETERS_DESC	*parameter; 
	struct _gnuplot_parameters_list *next; 
    }; 
 
typedef struct _gnuplot_parameters_list GNUPLOT_PARAMETERS_LIST; 

GNUPLOT_PARAMETERS_LIST *gnuplot_parameters_list;

//------------------


void
free_gnuplot_parameters_list ()
{
	GNUPLOT_PARAMETERS_LIST *gp_list;

	for (;gnuplot_parameters_list != (GNUPLOT_PARAMETERS_LIST *) NULL;)
	{
		gp_list = gnuplot_parameters_list;
		gnuplot_parameters_list = gnuplot_parameters_list->next;
		free(gp_list->parameter->data_filename);
		free(gp_list->parameter->title);
		free(gp_list->parameter);
		free(gp_list);
	}
}



void
add_gnuplot_parameters_to_gnuplot_parameters_list (GNUPLOT_PARAMETERS_DESC *gnuplot_parameters)
{
	GNUPLOT_PARAMETERS_LIST *gp_list;

	if (gnuplot_parameters_list == ((GNUPLOT_PARAMETERS_LIST *) NULL))
	{
		gp_list = (GNUPLOT_PARAMETERS_LIST *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_LIST));
		gp_list->next = (GNUPLOT_PARAMETERS_LIST *) NULL;
		gp_list->parameter = gnuplot_parameters;
		gnuplot_parameters_list = gp_list;
	}
	else
	{
		for (gp_list = gnuplot_parameters_list; gp_list->next != (GNUPLOT_PARAMETERS_LIST *) NULL;
			gp_list = gp_list->next)
		{
			if (gp_list->parameter == gnuplot_parameters)
				Erro ("Two instances of the same gnuplot_parameters in gnuplot_parameters_list.", "", "");
		}
		if (gp_list->parameter == gnuplot_parameters)
				Erro ("Two instances of the same gnuplot_parameters in gnuplot_parameters_list.", "", "");
		gp_list->next = (GNUPLOT_PARAMETERS_LIST *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_LIST));
		gp_list->next->next = (GNUPLOT_PARAMETERS_LIST *) NULL;
		gp_list->next->parameter = gnuplot_parameters;
	}
}



GNUPLOT_PARAMETERS_DESC *
make_gnuplot_parameters (PLOT_TYPE typePlot, char *data_filename, int x_field_position, int y_field_position, int z_field_position, char *title, PLOT_STYLE plot_style)
{
	GNUPLOT_PARAMETERS_DESC *gnuplot_parameter;
	
	gnuplot_parameter = (GNUPLOT_PARAMETERS_DESC *) alloc_mem ((size_t) sizeof (GNUPLOT_PARAMETERS_DESC));
	gnuplot_parameter->typePlot = typePlot;
	gnuplot_parameter->data_filename = (char *) alloc_mem ((size_t) strlen(data_filename) + 1);
	sprintf(gnuplot_parameter->data_filename,"%s",data_filename);
	gnuplot_parameter->x_field_position = x_field_position;
	gnuplot_parameter->y_field_position = y_field_position;
	gnuplot_parameter->z_field_position = z_field_position;
	gnuplot_parameter->title = (char *) alloc_mem ((size_t) strlen(title) + 1);
	sprintf(gnuplot_parameter->title,"%s",title);
	gnuplot_parameter->plot_style = plot_style;
	
	return gnuplot_parameter;
}



void
plot_gnuplot_file (char *command_file)
{
	GNUPLOT_PARAMETERS_LIST *gp_list;
	FILE *dataToPlot, *gnuplot_file;
	PLOT_TYPE typePlot_ant = -1;
	
	char message[256];
	
	if ((gnuplot_file = fopen (command_file, "w")) == NULL)
	{
		sprintf(message,"Could not open file %s",command_file);
		Erro (message, "", "");
	}

	fprintf(gnuplot_file,"set mouse labels\n");

	for (gp_list = gnuplot_parameters_list; gp_list != (GNUPLOT_PARAMETERS_LIST *) NULL;)
	{
		if ((dataToPlot = fopen (gp_list->parameter->data_filename, "r")) == NULL)
		{
			sprintf(message,"Could not open file %s",gp_list->parameter->data_filename);
			Erro (message, "", "");
		}
		fclose(dataToPlot);
		if (typePlot_ant == gp_list->parameter->typePlot)
			if (gp_list->next == (GNUPLOT_PARAMETERS_LIST *) NULL || gp_list->parameter->typePlot != gp_list->next->parameter->typePlot)
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
				else
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d:%d title \'%s\' with %s\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
			else
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", 
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
		else
			if (gp_list->next == (GNUPLOT_PARAMETERS_LIST *) NULL || gp_list->parameter->typePlot != gp_list->next->parameter->typePlot)
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
			else
				if (gp_list->parameter->z_field_position <= 0)
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style] );
				else
					fprintf(gnuplot_file,"%s\t\'%s\' using %d:%d title \'%s\' with %s,\\\n", name_plot_type[gp_list->parameter->typePlot],
						gp_list->parameter->data_filename, gp_list->parameter->x_field_position, gp_list->parameter->y_field_position,
						gp_list->parameter->z_field_position, gp_list->parameter->title, name_plot_style[gp_list->parameter->plot_style]);
		typePlot_ant = gp_list->parameter->typePlot;
		gp_list = gp_list->next;
	}
	free_gnuplot_parameters_list();
	fprintf(gnuplot_file,"pause -1  \'\"press return\"\'\n"); 
	fclose(gnuplot_file);
	sprintf(message,"gnuplot %s",command_file);
	/*system(message);*/
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
	
/*
		// Setar um quadrado
		for (j = 0; j < IMAGE_HEIGHT; j++)
		{
			for (i = 0; i < IMAGE_WIDTH; i++)
			{
				if ((j > (.1 * IMAGE_HEIGHT / 3)) && (j < (2.9 * IMAGE_HEIGHT / 3)) && (i > (.1 *IMAGE_WIDTH / 3)) && (i < (2.9 * IMAGE_WIDTH / 3)))
				{
					g_map.point[j][i].intensity = 1.0;
					g_map.point[j][i].dist = 51.0;
				}
				else
				{
					g_map.point[j][i].intensity = .5;
					g_map.point[j][i].dist = 68.0;
				}InitWindowMap();
}
			}
		}
	
		// Calcular as coord. do quadrado
		for (j = 0; j < IMAGE_HEIGHT; j++)
		{
			for (i = 0; i < IMAGE_WIDTH; i++)
			{
				TMapPointCalcXYZ(&(g_map.point[j][i]));
	
// 				printf("x: %f - y: %f - z: %f\n", g_map.point[j][i].x, g_map.point[j][i].y, g_map.point[j][i].z);
			}
		}
*/
	
		// Calcula o centro geometrico do TMap
// 		TMapGetMassCenter(g_map, &g_fCenterX, &g_fCenterY, &g_fCenterZ);
// 		printf("Centro de Massa - x: %f - y: %f - z: %f\n", g_fCenterX, g_fCenterY, g_fCenterZ);
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
// VerifyVergence - Retorna a diferenca media entre as
// 					transformacoes logpolar do olho direito
//					e olho esquerdo.
//
// Entrada: Nenhuma
//
// Saida: float - Media da diferenca entre lp_right e lp_left
// ----------------------------------------------------------------------------
float VerifyVergence()
{
	NEURON_LAYER *nl;
	int i, j;
	int pixel;
	float fltAux;
	float fltRangeCols = .75;
	int nStartCol, nEndCol;

	nl = get_neuron_layer_by_name (NL_MINUS_LEFT_RIGHT);

	nStartCol = nl->dimentions.x * ((1.0 - fltRangeCols) / 2.0);
	nEndCol = nl->dimentions.x - nStartCol;

	// Calcula a media da diferenca entre a imagem do olho esquerdo e olho direito
	fltAux = 0.0;

	for (j = 0; j < nl->dimentions.y; j++)
	{
		for (i = nStartCol; i < nEndCol; i++)
		{
			pixel = nl->neuron_vector[j * nl->dimentions.x + i].output.ival;
			fltAux += (float)(RED(pixel) + GREEN(pixel) + BLUE(pixel)) / 3.0 / 255.0;
		}
	}

	fltAux /= ((nEndCol - nStartCol) * nl->dimentions.y);

	return fltAux;
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

NEURON_OUTPUT
sum_neurons_output (NEURON *n, OUTPUT_TYPE output_type, int begin, int end)
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

void
set_vergence_by_minus_filter ()
{
	char command[256];
	static INPUT_DESC *i_left_eye     = NULL;
	static INPUT_DESC *i_right_eye    = NULL;	
	static NEURON_LAYER *minus_filter = NULL;
	static int num_neurons;
	int x, x_inic, x_min, x_max, x_max_minus, x_min_minus;
	NEURON_OUTPUT minus, max_minus, min_minus;
	FILE *arq_points_mf, *arq_points_minus, *arq_points_ini, *arq_points_min;

	// Permite que o filtro DISP_FILTER seja executado
	flagExecuteDispFilter = 1;

	if (minus_filter == NULL)
	{
		minus_filter = get_neuron_layer_by_name (NL_MINUS);
		i_left_eye   = g_pInputLeft;
		i_right_eye  = g_pInputRight;
		
		num_neurons = minus_filter->dimentions.x * minus_filter->dimentions.y;
	}
	
	system ("rm *.gnuplot");
	arq_points_minus = fopen ("minus.gnuplot", "w");
	arq_points_ini = fopen ("ini.gnuplot", "w");
	arq_points_min = fopen ("verg.gnuplot", "w");
	
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "ini.gnuplot" , 1, 2, 0, "Start Point", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "verg.gnuplot" , 1, 2, 0, "Vergence", IMPULSES));
	add_gnuplot_parameters_to_gnuplot_parameters_list (make_gnuplot_parameters (PLOT, "minus.gnuplot" , 1, 2, 0, "Response on minus", LINES));

	x_inic = i_left_eye->wxd;
	x_min  = i_right_eye->wxd;
	x_max  = i_right_eye->wxd + i_right_eye->ww / 6;
	
	if (minus_filter->output_type == GREYSCALE_FLOAT)
	{
		max_minus.fval = 0.0;
		min_minus.fval = 256.0 * ((float) num_neurons);
	}
	else
	{
		max_minus.ival = 0;
		min_minus.ival = 256 * num_neurons;
	}
	
	for (x = x_min; x < x_max; x++)
	{
		i_left_eye->wxd = x;
		i_left_eye->wyd = i_right_eye->wyd + correction;

		sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
		interpreter (command);
		
/*		all_filters_update ();*/
		
		if (minus_filter->output_type == GREYSCALE_FLOAT)
		{
			minus.fval = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).fval;
			fprintf (arq_points_minus, "%d, %f\n", x, minus.fval);
			
			if (max_minus.fval < minus.fval)
			{
				max_minus.fval = minus.fval;
				x_max_minus = x;
			}
			if (min_minus.fval > minus.fval)
			{
				min_minus.fval = minus.fval;
				x_min_minus = x;
			}
		}
		else
		{
			minus.ival = (sum_neurons_output (minus_filter->neuron_vector, minus_filter->output_type, 0, num_neurons)).ival;
			fprintf (arq_points_minus, "%d, %d\n", x, minus.ival);
			
			if (max_minus.ival < minus.ival)
			{
				max_minus.ival = minus.ival;
				x_max_minus = x;
			}
			if (min_minus.ival > minus.ival)
			{
				min_minus.ival = minus.ival;
				x_min_minus = x;
			}
		}
	}

	// Evita que o filtro DISP_FILTER seja executado
	flagExecuteDispFilter = 0;


	for (x = x_min; x < x_max; x++)
	{
		if (x == x_inic)
			if (minus_filter->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_ini, "%d, %f\n", x, max_minus.fval * 1.2);
			else
				fprintf (arq_points_ini, "%d, %d\n", x, (int) ((float) max_minus.ival * 1.2));
		else
			fprintf (arq_points_ini, "%d, %d\n", x, 0);
		if (x == x_min_minus)
			if (minus_filter->output_type == GREYSCALE_FLOAT)
				fprintf (arq_points_min, "%d, %f\n", x, max_minus.fval * 1.2);
			else
				fprintf (arq_points_min, "%d, %d\n", x, (int) ((float) max_minus.ival * 1.2));
		else
			fprintf (arq_points_min, "%d, %d\n", x, 0);
	}

	fclose (arq_points_minus);
	fclose (arq_points_min);
	fclose (arq_points_ini);
	plot_gnuplot_file("minus.cmd");

	i_left_eye->wxd_old = i_left_eye->wxd;
	i_left_eye->wxd = x_min_minus;
	sprintf (command, "move %s to %d, %d;", i_left_eye->name, i_left_eye->wxd, i_left_eye->wyd);
	interpreter (command);
	glutIdleFunc ((void (* ) (void)) check_forms);
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
	NEURON_LAYER *nl_minus, *nl_disparity_map;
	FILE *file;
	NEURON_LAYER *nl_gabor_left, *nl_gabor_right;
	FILE *file_gabor_left, *file_gabor_right;
	int i, wo, ho, x, y;
	char strAux[64];
	float fltSumOutputCells;
	float fltMinSum, fltAux;
	int nVergLeft;

	file = fopen("verg.out","w");

	file_gabor_left = fopen("gabor_left.out","w");
	file_gabor_right = fopen("gabor_right.out","w");

	// Buscar a neuron layer para efetuar a vergencia
	nl_disparity_map = get_neuron_layer_by_name(NL_DISPARITY_MAP);
	nl_minus = get_neuron_layer_by_name (NL_MINUS);

	nl_gabor_left = get_neuron_layer_by_name(NL_GABOR_LEFT);
	nl_gabor_right = get_neuron_layer_by_name (NL_GABOR_RIGHT);

	wo = nl_disparity_map->dimentions.x;
	ho = nl_disparity_map->dimentions.y;

	fltAux = 2.0;
	if (g_fltDispAux == NULL)
	{
		g_fltDispAux = (float*)malloc(sizeof(float)*wo*ho);
		compute_gaussian_kernel(&receptive_field_descriptor, 5, fltAux);
	}

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < (wo * ho); i++)
	{
		g_fltDispAux[i] = FLT_MAX;
		nl_disparity_map->neuron_vector[i].output.fval = 0.0;
	}


	nMinScan = g_pInputRight->wxd;
	nMaxScan = g_pInputRight->wxd + (g_pInputRight->neuron_layer->dimentions.x / 5);
	if (nMaxScan > g_pInputRight->neuron_layer->dimentions.x)
		nMaxScan = g_pInputRight->neuron_layer->dimentions.x;

/*
	nMinScan = g_pInputLeft->wxd - 5;
	nMaxScan = g_pInputLeft->wxd + 5;
	if (nMaxScan > g_pInputRight->neuron_layer->dimentions.x)
		nMaxScan = g_pInputRight->neuron_layer->dimentions.x;
*/
	fltMinSum = FLT_MAX;
//	for (g_pInputLeft->wxd = g_pInputRight->wxd; g_pInputLeft->wxd < nMaxScan; g_pInputLeft->wxd++)
//	for (g_pInputLeft->wxd = nMaxScan; g_pInputLeft->wxd >= nMinScan; g_pInputLeft->wxd--)
	for (g_pInputLeft->wxd = nMinScan; g_pInputLeft->wxd < nMaxScan; g_pInputLeft->wxd++)
	{
		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

		fltSumOutputCells = 0.0;
		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				fltSumOutputCells += nl_minus->neuron_vector[y * wo + x].output.fval;
//				fltAux = nl_minus->neuron_vector[y * wo + x].output.fval;
				fltAux = apply_gaussian_kernel(&receptive_field_descriptor, nl_minus, x, y);
				if (fltAux < g_fltDispAux[y * wo + x])
				{
					g_fltDispAux[y * wo + x] = fltAux;
					nl_disparity_map->neuron_vector[y * wo + x].output.fval = g_pInputLeft->wxd;
				}
			}
		}

		if (fltSumOutputCells < fltMinSum)
		{
			fltMinSum = fltSumOutputCells;
			nVergLeft = g_pInputLeft->wxd;
		}

//		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, fltSumOutputCells);
		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, nl_minus->neuron_vector[(nl_minus->dimentions.y / 2) * nl_minus->dimentions.x + (nl_minus->dimentions.x / 2) + g_nDeslocamento].output.fval);
		for (i = 0; strAux[i] != '\n'; i++)
			if (strAux[i] == ',') strAux[i] = '.';
		fprintf (file, "%s", strAux);

		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, nl_gabor_left->neuron_vector[(nl_gabor_left->dimentions.y / 2) * nl_gabor_left->dimentions.x + (nl_gabor_left->dimentions.x / 2) + g_nDeslocamento].output.fval);
		for (i = 0; strAux[i] != '\n'; i++)
			if (strAux[i] == ',') strAux[i] = '.';
		fprintf (file_gabor_left, "%s", strAux);

		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, nl_gabor_right->neuron_vector[(nl_gabor_right->dimentions.y / 2) * nl_gabor_right->dimentions.x + (nl_gabor_right->dimentions.x / 2) + g_nDeslocamento].output.fval);
		for (i = 0; strAux[i] != '\n'; i++)
			if (strAux[i] == ',') strAux[i] = '.';
		fprintf (file_gabor_right, "%s", strAux);

//		printf("posMin: %d\n", (int)nl_disparity_map->neuron_vector[(nl_gabor_right->dimentions.y / 2) * nl_gabor_right->dimentions.x + (nl_gabor_right->dimentions.x / 2) + g_nDeslocamento].output.fval);
	}

	fclose(file);

	fclose(file_gabor_left);
	fclose(file_gabor_right);

	// Correcao do mapa de disparidade de acordo com a vergencia escolhida
	for (i = 0; i < (wo * ho); i++)
		nl_disparity_map->neuron_vector[i].output.fval -= nVergLeft;

	g_pInputLeft->wxd = nVergLeft;
	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	printf("nVergLeft: %d\n", nVergLeft);

	glutPostWindowRedisplay (g_pInputLeft->win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}

// ----------------------------------------------------------------------------
// set_vergence_n - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void set_vergence_n (TPoint *point)
{
	int nMax;
	NEURON_LAYER *nl;
	FILTER_DESC *filter;
	NEURON_LAYER *filter_output;
	TStructMinAssocV1Filter *aux;
	FILE *file;
	int i, wo, ho;
	char strAux[64];
	float fltSumOutputCells;
	float fltMinSum;
	int nVergLeft;

/*
	// Verge	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

	while((nShiftDisparity = GetShiftDisparity(NL_SMALL_SHIFT_VERGENCE)) != 0)
	{
		g_pInputLeft->wxd += nShiftDisparity;
		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	}
*/
	file = fopen("verg.out","w");

	// *** Zerar o filtro de disparidade ***
	// Buscar o filtro atraves da neuron layer de output
	filter = get_filter_by_output(get_neuron_layer_by_name(NL_DISPARITY_MAP));
	filter_output = filter->output;
	aux = (TStructMinAssocV1Filter*)filter->private_state;

	wo = filter->output->dimentions.x;
	ho = filter->output->dimentions.y;

	// Zerar o filtro
	for (i = 0; i < wo * ho; i++)
	{
		aux[i].value = FLT_MAX;
		aux[i].x = -1;
		filter_output->neuron_vector[i].output.fval = 0.0;
	}

	nl = get_neuron_layer_by_name (NL_MINUS_V1FILTER);

	nMax = g_pInputRight->wxd + (g_pInputRight->neuron_layer->dimentions.x / 5);
	if (nMax > g_pInputRight->neuron_layer->dimentions.x)
		nMax = g_pInputRight->neuron_layer->dimentions.x;

	fltMinSum = FLT_MAX;
	for (g_pInputLeft->wxd = g_pInputRight->wxd; g_pInputLeft->wxd < nMax; g_pInputLeft->wxd++)
	{
		move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);

		fltSumOutputCells = SumOutputCells(nl);

		if (fltSumOutputCells < fltMinSum)
		{
			fltMinSum = fltSumOutputCells;
			nVergLeft = g_pInputLeft->wxd;
		}

//		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, fltSumOutputCells);
		sprintf(strAux, "%d %.3f\n", g_pInputLeft->wxd, nl->neuron_vector[(nl->dimentions.y / 2) * nl->dimentions.x + (nl->dimentions.x / 2)].output.fval);
		for (i = 0; strAux[i] != '\n'; i++)
			if (strAux[i] == ',') strAux[i] = '.';
		fprintf (file, "%s", strAux);
	}

	fclose(file);

	g_pInputLeft->wxd = nVergLeft;
	move_input_window (g_pInputLeft->name, g_pInputLeft->wxd, g_pInputLeft->wyd);
	printf("nVergLeft: %d\n", nVergLeft);

/*
	if (point != NULL)
	{
		right_point.x = (double) (g_pInputRight->wxd);
		right_point.y = (double) (g_pInputRight->wyd);
	
		left_point.x = (double) (g_pInputLeft->wxd);
		left_point.y = (double) (g_pInputLeft->wyd);
		
		world_point = calculate_world_point (left_point, right_point, g_pInputRight->neuron_layer->dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);
	
		point->x = world_point.x;
		point->y = world_point.y;
		point->z = -world_point.z;
		point->dist = sqrt(world_point.x*world_point.x + world_point.y*world_point.y + world_point.z*world_point.z);
		printf ("Vergence - x:%3.2f y:%3.2f z:%3.2f - distance: %3.2f\n", world_point.x, world_point.y, world_point.z, point->dist);
	}
*/
//	fltAvgVerg = VerifyVergence();
//	printf("fltAvgVerg: %f\n", fltAvgVerg);
	glutPostWindowRedisplay (g_pInputLeft->win);
	glutIdleFunc ((void (* ) (void))fl_check_forms);
}

// ----------------------------------------------------------------------------
// MapImageToV1 - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void MapImageToV1(int x, int y, int nImageWidth, int nImageHeight, int nV1Width, int nV1Height, float fltLogFactor)
{
//	printf("x: %d - y: %d - nImageWidth: %d - nImageHeight: %d - nV1Width: %d - nV1Height: %d - fltLogFactor: %2.2f\n", x, y, nImageWidth, nImageHeight, nV1Width, nV1Height, fltLogFactor);
	printf("x: %d - y: %d\n", x, y);
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

//	fltAvgVerg = VerifyVergence();
//	printf("fltAvgVerg: %f\n", fltAvgVerg);

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

//	fltAvgVerg = VerifyVergence();
//	printf("fltAvgVerg: %f\n", fltAvgVerg);

	glutIdleFunc ((void (* ) (void)) fl_check_forms);
}

// ----------------------------------------------------------------------------
// DisplayDisparityMap - Mostra o mapa de disparidade de v1_filter
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void DisplayDisparityMap()
{
	FILTER_DESC *filter;
	NEURON_LAYER *filter_output;
	OUTPUT_DESC *output;
	int i, wo, ho;
	int x_verg_left;
	TStructMinAssocV1Filter *aux;

	// Buscar o filtro atraves da neuron layer de output
	filter = get_filter_by_output(get_neuron_layer_by_name(NL_DISPARITY_MAP));
	filter_output = filter->output;
	aux = (TStructMinAssocV1Filter*)filter->private_state;

	wo = filter->output->dimentions.x;
	ho = filter->output->dimentions.y;

	// Buscar a coordenada x de vergencia da imagem esquerda
	x_verg_left = g_pInputLeft->wxd;

	// Mostrar a saida do filtro
	for (i = 0; i < wo * ho; i++)
		filter_output->neuron_vector[i].output.fval = (float) (aux[i].x - x_verg_left);

	output = get_output_by_neural_layer(filter_output);

	glutPostWindowRedisplay (output->win);
	glutIdleFunc ((void (* ) (void)) fl_check_forms);
}

typedef struct
    {
	double val;
	int pos;
    } SAMPLES;
	

typedef struct
    {
    	char *neg_slope;
	SAMPLES **samples;

    } DISP_DATA;

#define NUM_SAMPLES 5
    
    
   
/*
*********************************************************************************
*********************************************************************************
*/

void
add_local_min (INPUT_DESC *image_left, DISP_DATA *disp_data, int i, double minus_out)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
		if (minus_out <= disp_data->samples[victim_sample][i].val)
			break;
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			disp_data->samples[moving_sample][i] = disp_data->samples[moving_sample - 1][i];
			moving_sample--;
		}
		disp_data->samples[moving_sample][i].val = minus_out;
		disp_data->samples[moving_sample][i].pos = image_left->wxd;
	}
}



void
init_disp_filter_state (FILTER_DESC *filter_desc)
{
	DISP_DATA *disp_data;
	int num_neurons, i, sample;
	
	disp_data = (DISP_DATA *) filter_desc->private_state;

	num_neurons = get_num_neurons (filter_desc->output->dimentions);
	for (i = 0; i < num_neurons; i++)
	{
		disp_data->neg_slope[i] = 0;
		filter_desc->output->neuron_vector[i].output.fval = 1000000000.0;
	}
	
	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < num_neurons; i++)
		{
			disp_data->samples[sample][i].val = 100000000.0;
			disp_data->samples[sample][i].pos = 0;
		}
	}
}



/*
*********************************************************************************
*********************************************************************************
*/

void
init_disp_filter (FILTER_DESC *filter_desc)
{
	DISP_DATA *disp_data;
	int num_neurons, i, sample;
	
	disp_data = (DISP_DATA *) filter_desc->private_state;

	num_neurons = get_num_neurons (filter_desc->output->dimentions);
	disp_data->neg_slope = (char *) alloc_mem (num_neurons);
	
	disp_data->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
	for (sample = 0; sample < NUM_SAMPLES; sample++)
		disp_data->samples[sample] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * num_neurons);
	init_disp_filter_state (filter_desc);
}



/*
*********************************************************************************
* Minus Filter: if the difference between the two inputs is positive, the 	*
* output is the difference, otherwise is zero					*
*********************************************************************************
*/

void
disp_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l = NULL;
	INPUT_DESC *image_left;
	NEURON *minus_out, *disp_out;
	DISP_DATA *disp_data;
	int i, num_neurons;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2) 
	{
		Erro ("Wrong number of neuron layers. disp_filter must be applied on 1 input and 1 neuron layer.", "", "");
		return;
	}

	n_l = filter_desc->neuron_layer_list->neuron_layer;
	image_left = get_input_by_neural_layer (n_l);

	minus_out = filter_desc->neuron_layer_list->next->neuron_layer->neuron_vector;
	disp_out = filter_desc->output->neuron_vector;
	
	if (filter_desc->private_state == NULL)
	{
		disp_data = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		filter_desc->private_state = (void *) disp_data;
		init_disp_filter (filter_desc);
	}

	if (flagExecuteDispFilter)
	{
		disp_data = (DISP_DATA *) filter_desc->private_state;
	
		num_neurons = get_num_neurons (filter_desc->output->dimentions);
	
		for (i = 0; i < num_neurons; i++)
		{
			if (minus_out[i].output.fval >= disp_out[i].output.fval)
			{
				if (disp_data->neg_slope[i])
				{
					disp_data->neg_slope[i] = 0;
					add_local_min (image_left, disp_data, i, minus_out[i].output.fval);
				}
			}
			else
				disp_data->neg_slope[i] = 1;
	
			disp_out[i].output.fval = minus_out[i].output.fval;
		}
	}
}



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
	return ((int) ((double) dist / (double) count + 0.5));
}



void
calculate_disparity (FILTER_DESC *filter_desc, int wxd)
{
	DISP_DATA *disp_data;
	SAMPLES temp;
	int x, y, w, h, i, sample, best_sample, cur_dist, dist;
	
	disp_data = (DISP_DATA *) filter_desc->private_state;

	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (sample = 0; sample < NUM_SAMPLES; sample++)
			{
				dist = distance_neig (disp_data->samples[0], disp_data->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = disp_data->samples[0][y * w + x];
			disp_data->samples[0][y * w + x] = disp_data->samples[best_sample][y * w + x];
			disp_data->samples[best_sample][y * w + x] = temp;
		}
	}

	for (i = 0; i < w*h; i++)
		filter_desc->output->neuron_vector[i].output.fval = (double) (disp_data->samples[0][i].pos - wxd);
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
//			update_input_neurons (input);
		}
		break;

		case 'X':
		case 'x':
		{
			g_nColorImage = COLOR_MONO_8;
//			update_input_neurons (input);
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

		case 'B':
		case 'b':
		{
			rot = 0.0;
			w1 = 0.0;
			w2 = 0.0;
			w3 = 0.0;
			TMapGetGeoCenter(g_map, &g_fCenterX, &g_fCenterY, &g_fCenterZ);
			printf("Centro Geometrico - x: %f - y: %f - z: %f\n", g_fCenterX, g_fCenterY, g_fCenterZ);
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

/*
		case 'I':
		{
			g_nMoveLeft = -5;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;

		case 'i':
		{
			g_nMoveLeft = -1;
			glutIdleFunc ((void (* ) (void))MoveXImageLeft);
		}
		break;
*/

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

/*
		case 'i':
		{
			init_disp_filter_state (get_filter_by_name (DISP_FILTER_NAME));
		}
		break;

		case 'd':
		{
			calculate_disparity (get_filter_by_name (DISP_FILTER_NAME), g_pInputLeft->wxd);
			output = get_output_by_name (OUT_DISPARITY_MAP);
			glutPostWindowRedisplay (output->win);
		}
		break;

		case 'm':
		case 'M':
		{
			glutIdleFunc ((void (* ) (void))set_vergence_by_minus_filter);
		}
		break;
*/

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
