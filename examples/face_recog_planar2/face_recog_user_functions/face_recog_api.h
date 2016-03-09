#ifndef _FACE_RECOG_API_H
#define _FACE_RECOG_API_H 1

// Includes
#include "face_recog_user_functions.h"


#ifdef	WINDOWS
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT
#endif


#define ERROR_LOG_FILE_NAME		"ErrorLogFile.txt"
#define EXECUTION_LOG_FILE_NAME		"ExecutionLogFile.txt"

#ifdef __cplusplus
extern "C" {
#endif

// Types

struct _neuron_layer_output_int
{
	NEURON_LAYER *neuron_layer;
	int *output_int;
	struct _neuron_layer_output_int *next;
};

typedef struct _neuron_layer_output_int NEURON_LAYER_OUTPUT_INT;


struct _neuron_layer_output_float
{
	NEURON_LAYER *neuron_layer;
	float *output_float;
	struct _neuron_layer_output_float *next;
};

typedef struct _neuron_layer_output_float NEURON_LAYER_OUTPUT_FLOAT;


// Macros
#define ERROR(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (ERROR_LOG_FILE_NAME, "a"); \
						fprintf (pLogFile, "Error: %s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }

#define MESSAGE(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (EXECUTION_LOG_FILE_NAME, "a"); \
						fprintf (pLogFile, "%s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }

// Types

// Prototypes

// ----------------------------------------------------------------------
// MAE API
// ----------------------------------------------------------------------
DLLEXPORT int MaeInitialize (void);
DLLEXPORT int MaeQuit (void);
DLLEXPORT void MaeExecuteCommandScript (char *script);

// ----------------------------------------------------------------------
// Image manipulation API
// ----------------------------------------------------------------------
DLLEXPORT int MaeUpdateImage (int *p_nNewImage, int nImageWidth, int nImageHeight);

// ----------------------------------------------------------------------
// Neuron Layers manipulation API
// ----------------------------------------------------------------------
DLLEXPORT int *get_neuron_layer_output_int_pointer (NEURON_LAYER *neuron_layer);
DLLEXPORT float *get_neuron_layer_output_float_pointer (NEURON_LAYER *neuron_layer);
DLLEXPORT int *GetNeuronLayerOutputInt (char *neuron_layer_name);
DLLEXPORT int *GetNeuronLayerOutputFloat (char *neuron_layer_name);
DLLEXPORT int PutNeuronLayerOutputInt (char *neuron_layer_name, int *neural_layer_output_int);
DLLEXPORT int GetNeuronLayerHight (char *neuron_layer_name);
DLLEXPORT int GetNeuronLayerWidth (char *neuron_layer_name);


// Global Variables

#ifdef __cplusplus
}
#endif

#endif
