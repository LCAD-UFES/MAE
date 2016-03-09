#ifndef __MAPPING_H
#define __MAPPING_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Definitions
#define DIST_INFINITY	-1

// Macros

// Structs
typedef struct struct_point
{
	float	alfa;		// angulo horizontal
	float	beta;		// angulo vertical
	float	dist;		// distancia total
	float	x;
	float	y;
	float	z;
	float	intensity;	// intensidade [0.0;1.0]
}	TPoint;

typedef struct struct_map
{
	TPoint	**point;
	int		width;
	int		height;
} TMap;

struct _world_point
    { 
     	double 			x; 
     	double 			y;
	double			z;
	
    }; 

typedef struct _world_point WORLD_POINT;

struct _spherical_coordinate
    { 
     	double 			r; 
     	double 			theta;
	double			fi;
    }; 
typedef struct _spherical_coordinate SPHERICAL_COORDINATE;

struct _image_coordinate
    { 
     	double 			x; 
     	double 			y;
    }; 
typedef struct _image_coordinate IMAGE_COORDINATE;

// Global Variables

// Prototypes
// ----------------------------------------------------------------------
// TMapInitialize - Inicializa um TMap de acordo com os parametros
//					de entrada da funcao.
// 
// Entradas: map - TMap a ser inicializado
//			 img_width - Largura em pixels da imagem de entrada
//			 img_height - Altura em pixels da imagem de entrada
//			 lambda - Distancia focal em pixels
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapInitialize(TMap *map, int img_width, int img_height, float lambda);

// ----------------------------------------------------------------------
// TMapDispose - Libera a memoria associada a um TMap
// 
// Entradas: map - TMap a ser finalizado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapDispose(TMap *map);

// ----------------------------------------------------------------------
// TMapPointCalcXYZ - Calcula as coordenadas X, Y e Z, a partir das coor-
//					  denadas esfericas do ponto.
// 
// Entradas: point - Ponto a ser calculado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapPointCalcXYZ(TPoint *point);

// ----------------------------------------------------------------------
// TMapPointCalcDist - Calcula a distancia do ponto a partir das coorde-
//					   nadas X, Y e Z.
// 
// Entradas: point - Ponto a ser calculado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapPointCalcDist(TPoint *point);

// ----------------------------------------------------------------------
// TMapGetMassCenter - Retorna o centro de massa do TMap
// 
// Entradas: map - TMap a ser calculado o centro geometrico
//			 center_x - Retorno da coordenada x do centro geometrico
//			 center_y - Retorno da coordenada y do centro geometrico
//			 center_z - Retorno da coordenada z do centro geometrico
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapGetMassCenter(TMap map, float *center_x, float *center_y, float *center_z);

// ----------------------------------------------------------------------
// TMapGetGeoCenter - Retorna o centro geometrico do TMap
// 
// Entradas: map - TMap a ser calculado o centro geometrico
//			 center_x - Retorno da coordenada x do centro geometrico
//			 center_y - Retorno da coordenada y do centro geometrico
//			 center_z - Retorno da coordenada z do centro geometrico
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapGetGeoCenter(TMap map, float *center_x, float *center_y, float *center_z);

// ----------------------------------------------------------------------
// TMapSave - Salva o estado do TMap no arquivo apontado por arq.
// 
// Entradas: map - TMap a ser salvo
//			 arq - Ponteiro para FILE
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapSave(TMap map, FILE *arq);

// ----------------------------------------------------------------------
// TMapLoad - Le de um arquivo o estado de um TMap.
// 
// Entradas: map - Ponteiro para o TMap a ser lido
//			 file - Ponteiro para FILE
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapLoad(TMap *map, FILE *file);

// ----------------------------------------------------------------------
// TMapReset - Reinicializa um TMap
// 
// Entradas: map - Ponteiro para o TMap a ser reinicializado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapReset(TMap *map);

// ----------------------------------------------------------------------
// TMapSavePoints - Salva os pontos do TMap no arquivo apontado por file.
// 
// Entradas: map - TMap a ser salvo
//			 file - Ponteiro para FILE
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapSavePoints(TMap map, FILE *file);

// ----------------------------------------------------------------------
// TMapCalculateWorldPoint - 
//
// Entradas: 
//
// Saida:
// ----------------------------------------------------------------------
WORLD_POINT TMapCalculateWorldPoint (IMAGE_COORDINATE leftPoint, 
				     IMAGE_COORDINATE rightPoint,
		       		     IMAGE_COORDINATE leftPrincipalPoint,
		       		     IMAGE_COORDINATE rightPrincipalPoint,
		       		     double fltCameraLeftFocus,
		       		     double fltCameraRightFocus,
		       		     double fltCameraDistance);
#endif
