#include <math.h>
#include "mapping.h"

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
void TMapInitialize(TMap *map, int img_width, int img_height, float lambida)
{
	int i, j;
	float alfa, beta;
	
	map->width = img_width;
	map->height = img_height;
	map->point = (TPoint**)malloc(sizeof(TPoint*)*img_height);

	for (j = 0; j < img_height; j++)
	{
		map->point[j] = (TPoint*)malloc(sizeof(TPoint)*img_width);

		// Calcula beta
		beta = atanf((float)((img_height / 2) - j) / lambida);

		// Inicializa os pontos
		for (i = 0; i < img_width; i++)
		{
			map->point[j][i].beta = beta;
			alfa = atanf((float)((img_width / 2) - i) / lambida);	// Calcula o alfa
			map->point[j][i].alfa = alfa;
			map->point[j][i].dist = DIST_INFINITY;
			map->point[j][i].intensity = 0.0;
		}
	}
}

// ----------------------------------------------------------------------
// TMapDispose - Libera a memoria associada a um TMap
// 
// Entradas: map - TMap a ser finalizado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapDispose(TMap *map)
{
	int j;
	
	for (j = 0; j < map->height; j++)
		free(map->point[j]);
	free(map->point);
}

// ----------------------------------------------------------------------
// TMapPointCalcXYZ - Calcula as coordenadas X, Y e Z, a partir das coor-
//					  denadas esfericas do ponto.
// 
// Entradas: point - TMap a ser finalizado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapPointCalcXYZ(TPoint *point)
{
	float fDistXZ;

	point->y = point->dist * sin(point->beta);
	fDistXZ = point->dist * cos(point->beta);
	point->x = fDistXZ * sin(point->alfa);
	point->z = -fDistXZ * cos(point->alfa);
}

// ----------------------------------------------------------------------
// TMapPointCalcDist - Calcula a distancia do ponto a partir das coorde-
//					   nadas X, Y e Z.
// 
// Entradas: point - Ponto a ser calculado
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapPointCalcDist(TPoint *point)
{
	point->dist = sqrt((point->x * point->x) + (point->y * point->y) + (point->z * point->z));
}

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
void TMapGetMassCenter(TMap map, float *center_x, float *center_y, float *center_z)
{
	int i, j;

	*center_x = 0.0;
	*center_y = 0.0;
	*center_z = 0.0;

	for (j = 0; j < map.height; j++)
	{
		for (i = 0; i < map.width; i++)
		{
			*center_x += map.point[j][i].x;
			*center_y += map.point[j][i].y;
			*center_z += map.point[j][i].z;
		}
	}

	*center_x /= (map.width * map.height);
	*center_y /= (map.width * map.height);
	*center_z /= (map.width * map.height);
}

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
void TMapGetGeoCenter(TMap map, float *center_x, float *center_y, float *center_z)
{
	int i, j;
	float fMinX, fMaxX;
	float fMinY, fMaxY;
	float fMinZ, fMaxZ;
	int flagPrimeiroPontoValido;

	*center_x = 0.0;
	*center_y = 0.0;
	*center_z = 0.0;
	flagPrimeiroPontoValido = 1;
	
	for (j = 0; j < map.height; j++)
	{
		for (i = 0; i < map.width; i++)
		{
			if (map.point[j][i].dist != DIST_INFINITY)
			{
				if (flagPrimeiroPontoValido)
				{
					flagPrimeiroPontoValido = 0;
					fMinX = map.point[0][0].x;
					fMaxX = map.point[0][0].x;
					fMinY = map.point[0][0].y;
					fMaxY = map.point[0][0].y;
					fMinZ = map.point[0][0].z;
					fMaxZ = map.point[0][0].z;
				}
				else
				{
					if (map.point[j][i].x < fMinX)
						fMinX = map.point[j][i].x;
		
					if (map.point[j][i].x > fMaxX)
						fMaxX = map.point[j][i].x;
		
					if (map.point[j][i].y < fMinY)
						fMinY = map.point[j][i].y;
		
					if (map.point[j][i].y > fMaxY)
						fMaxY = map.point[j][i].y;
		
					if (map.point[j][i].z < fMinZ)
						fMinZ = map.point[j][i].z;
		
					if (map.point[j][i].z > fMaxZ)
						fMaxZ = map.point[j][i].z;
				}
			}
		}
	}

	*center_x = (fMaxX + fMinX) / 2.0;
	*center_y = (fMaxY + fMinY) / 2.0;
	*center_z = (fMaxZ + fMinZ) / 2.0;
}

// ----------------------------------------------------------------------
// TMapSave - Salva o estado do TMap no arquivo apontado por file.
// 
// Entradas: map - TMap a ser salvo
//			 file - Ponteiro para FILE
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapSave(TMap map, FILE *file)
{
	int i, j;

	// Salva a estrutura TMap
	fwrite((void*)&(map.width), sizeof(int), 1, file);
	fwrite((void*)&(map.height), sizeof(int), 1, file);

	// Percorre os pontos salvando cada um deles
	for (j = 0; j < map.height; j++)
	{
		for (i = 0; i < map.width; i++)
		{			
			fwrite((void*)&(map.point[j][i]), sizeof(TPoint), 1, file);
		}
	}
}

// ----------------------------------------------------------------------
// TMapLoad - Le de um arquivo o estado de um TMap.
// 
// Entradas: map - Ponteiro para o TMap a ser lido
//			 file - Ponteiro para FILE
//
// Saida: Nenhuma
// ----------------------------------------------------------------------
void TMapLoad(TMap *map, FILE *file)
{
	int i, j;

	// Salva a estrutura TMap
	fread((void*)&(map->width), sizeof(int), 1, file);
	fread((void*)&(map->height), sizeof(int), 1, file);

	// Percorre os pontos salvando cada um deles
	for (j = 0; j < map->height; j++)
	{
		for (i = 0; i < map->width; i++)
		{			
			fread((void*)&(map->point[j][i]), sizeof(TPoint), 1, file);
		}
	}
}
