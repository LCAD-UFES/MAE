#ifndef __MAPPING_H
#define __MAPPING_H

#include <stdio.h>

#define DIST_INFINITY	-1

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

// ----------------------------------------------------------------------
// TMapInitialize - Inicializa um TMap de acordo com os parametros
//					de entrada da funcao.
// 
// Entradas: map - TMap a ser inicializado
//			 img_width - Largura em pixels da imagem de entrada
//			 img_height - Altura em pixels da imagem de entrada
//			 lambida - Distancia focal em pixels
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapInitialize(TMap *map, int img_width, int img_height, float lambida);

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

#endif
