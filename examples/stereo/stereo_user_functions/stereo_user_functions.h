#ifndef __STEREO_USER_FUNCTIONS_H
#define __STEREO_USER_FUNCTIONS_H

// Includes
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include "filter.h"
#include "../../../src/libstereo/include/stereo_api.hpp"
#include "../stereo.h"

// Definitions
#define STIMULUS_LEFT			0
#define STIMULUS_RIGHT			1

#define INVALID_SOCKET			-1

#define ST_WAIT				0
#define ST_GET_IMAGE			1

#define MSG_SNAP_MONO_256		"snap_mono_256"
#define MSG_SNAP_332			"snap_332"
#define MSG_SNAP_MONO_8			"snap_mono_8"

#define COLOR_MONO_256			0
#define COLOR_332			1
#define COLOR_MONO_8			2

#define INPUT_IMAGE_LEFT		0
#define INPUT_IMAGE_RIGHT		1

#define MAX_PACKAGE			100000

#define VERT_GAP			0

#define MAP_WINDOW_WIDTH		400
#define MAP_WINDOW_HEIGHT		300

// Igual a FOCAL_DISTANCE
#define LAMBDA				400.0

// Distancia maxima (para visualizacao apenas) no eixo Z
#define MAX_Z_VIEW			1000.0

#define FILE_TMAP			"tmap.dat"
#define FILE_TMAP_ASCII			"tmap_ascii.dat"

#define POINT_SIZE			2

//#define LOG_FACTOR			2.0

// Tamanho do robo em centímetros (diametro)
#define ROBOT_SIZE			35.0

#define NUM_SAMPLES			5

// Macros
#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}
#define closesocket(s)			close(s);

#define DRAWSQUARE(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd (); \
}

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); glVertex2i ((x+w/50), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); glVertex2i ((x), (y+h/50)); glEnd (); \
}

#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); glVertex2i ((x), (h+h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); glVertex2i ((x), (-h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); glVertex2i ((w+h/20), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); glVertex2i ((-h/20), (y)); glEnd (); \
}

#define DRAWLINE(x0, y0, x1, y1) \
{   glBegin(GL_LINES); glVertex2f (x0, y0); glVertex2f (x1, y1); glEnd (); \
}

// Structs

// Prototypes
void set_vergence (void);

// Global Variables

// Exportable Variables
extern double g_dblNearVergenceCutPlane;
extern double g_dblFarVergenceCutPlane;

#endif
