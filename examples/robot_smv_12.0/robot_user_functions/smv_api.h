/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Refitting file for new dynamic library API. 
*/

#ifndef __SMV_API_H
#define __SMV_API_H

/*
Definitions Section
*/

#ifdef	WINDOWS /* Windows demands dynamically-linked functions to be properly tagged. */
	#define DLLEXPORT __declspec (dllexport)
#else /* Linux has no such issues. */
	#define DLLEXPORT
#endif

/* Function response codes. */
#define MAE_UNKNOWN_ERROR          -1
#define MAE_OK                      0
#define MAE_SIZE_MISMATCH           1
#define MAE_WRONG_WORKING_AREA      2
#define MAE_MERGE_ERROR             3
#define MAE_DISPARITY_MAP_ERROR     4

/* Indexes for the three faces (picture pairs) of a log pile. */
#define MAE_FACE_LEFT  0
#define MAE_FACE_TOP   1
#define MAE_FACE_RIGHT 2

/* Type of a log pile. */
#define MAE_TYPE_3M 0
#define MAE_TYPE_6M 1

/*
Structures Section
*/

/**
Um par de imagens est�reo.
*/
struct _MaeImagePair
{
       // Imagem da c�mera esquerda.
       int *left;

       // Imagem da c�mera direita.
       int *right;
};

typedef struct _MaeImagePair MaeImagePair;

/**
Regi�o delimitada pelo usu�rio.
*/
struct _MaeWorkingSpace
{
       // Coordenadas do canto inferior esquerdo da regi�o.
       int x0, y0;

       // Coordenadas do canto superior direito da regi�o.
       int xn, yn;
};

typedef struct _MaeWorkingSpace MaeWorkingSpace;

/**
Estrutura para comunica��o com o SMV.
*/
struct _MaeVolumeData
{
       // Tipo do implemento (3 ou 6 metros).
       int type; 

       // Faces da metade da frente do implemento.
       MaeImagePair front[3];

       // Faces da metade de tr�s do implemento
       // (apenas para implemento de 6 metros).
       MaeImagePair back[3];
       
       // Regi�o delimitada pelo usu�rio.
       MaeWorkingSpace workingSpace;
       
       // Volume est�reo.
       double stereoVolume;
       
       // Volume s�lido.
       double solidVolume;
       
       // Comprimento, altura e largura m�dias da pilha de toras.
       double length, height, width;
};

typedef struct _MaeVolumeData MaeVolumeData;

#ifdef __cplusplus
extern "C" {
#endif

/*
Initialization Section
*/

/**
Initializes the MAE framework. 

@return 0 if OK, -1 otherwise.
*/
DLLEXPORT int MaeInitialize();

/** 
Terminates the MAE framework. This method should only be called after the MAE framework has been initialized with <code>MaeInitialize()</code>.

@return 0 if OK, -1 otherwise.
*/
DLLEXPORT int MaeQuit();

/*
Function Section
*/

/**
Recebe uma imagem "crua" e retorna a imagem retificada, com a regi�o n�o-est�reo
removida.

C�digos de erro:

MAE_OK - Opera��o bem-sucedida.

MAE_SIZE_MISMATCH - Se as dimens�es da imagem de entrada n�o casarem com
as configura��es em "robot.con".

MAE_UNKNOWN_ERROR - Se um erro imprevisto ocorrer.
*/
DLLEXPORT int MaeRectify(int* image, int size, int* rectified);

/**
Calcula o volume e as dimens�es m�dias de uma pilha de madeira a partir dos
dados de entrada.

C�digos de erro:
        
MAE_OK - Opera��o bem-sucedida.

MAE_WRONG_WORKING_AREA - Se o usu�rio n�o selecionou uma regi�o.

MAE_MERGE_ERROR - Para implementos de seis metros, se n�o for poss�vel casar
as metades de uma pilha.

MAE_DISPARITY_MAP_ERROR - Se n�o for poss�vel gerar um mapa de disparidades
coerente a partir das imagens de entrada.

MAE_UNKNOWN_ERROR - Se um erro imprevisto ocorrer.
*/
DLLEXPORT int MaeComputeVolume(MaeVolumeData* data);

#ifdef __cplusplus
}
#endif

#endif
