#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H

#define INVALID_SOCKET			-1
#define closesocket(s)			close(s);

#define GET_IMAGE_FROM_SOCKET	FALSE
#define VISION_STERO			FALSE
#define ARTIFICIAL_IMAGE		FALSE

#define ST_WAIT					0
#define ST_GET_IMAGE			1

#define MSG_SNAP_MONO_256		"snap_mono_256"
#define MSG_SNAP_332			"snap_332"
#define MSG_SNAP_MONO_8			"snap_mono_8"

#define COLOR_MONO_256			0
#define COLOR_332				1
#define COLOR_MONO_8			2

#define SOCKET_PORT				27015
#define HOST_ADDR_MONO			"192.16328.0.1"
#define HOST_ADDR_LEFT			"10.50.5.121"
#define HOST_ADDR_RIGHT			"10.50.5.122"
#define IMAGE_WIDTH				320
#define IMAGE_HEIGHT			240
#define INPUT_NAME_LEFT			"image_left"
#define INPUT_NAME_RIGHT		"image_right"
#define INPUT_IMAGE_LEFT		0
#define INPUT_IMAGE_RIGHT		1

#define NL_WIDTH				96
#define NL_HEIGHT				48

#define MAX_PACKAGE				100000

#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}

#define VERT_GAP				0

#define MAP_WINDOW_WIDTH		400
#define MAP_WINDOW_HEIGHT		300

// Igual a FOCAL_DISTANCE
#define LAMBDA					400.0

// Distancia maxima (para visualizacao apenas) no eixo Z
#define MAX_Z_VIEW				300.0

#define VERG_SCAN_WIDTH			60
#define VERG_SCAN_HEIGHT		45

#define FILE_IMAGE_LEFT			"img_left.rbt"
#define FILE_IMAGE_RIGHT		"img_right.rbt"
#define FILE_TMAP				"tmap.dat"

#define POINT_SIZE				2

#define LOG_FACTOR				2.0

// RTBC
#define DISPARITY_FACTOR		1.0

// Tamanho do robo em centímetros (diametro)
#define ROBOT_SIZE				35.0

typedef int SOCKET;

#endif
