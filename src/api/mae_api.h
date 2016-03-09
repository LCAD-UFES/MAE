#ifndef _MAE_API_H
#define _MAE_API_H 1

// Includes
#include <mae.h>

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
DLLEXPORT int MaeUpdateImage (char *p_InputName, int *p_nNewImage, int nImageWidth, int nImageHeight);

// ----------------------------------------------------------------------
// Neuron Layers manipulation API
// ----------------------------------------------------------------------
DLLEXPORT float *GetNeuronLayerOutputFloat (char *neuron_layer_name);
DLLEXPORT int *GetNeuronLayerOutputInt (char *neuron_layer_name);
DLLEXPORT int PutNeuronLayerOutputInt (char *neuron_layer_name, int *neural_layer_output_int);
DLLEXPORT int GetNeuronLayerHight (char *neuron_layer_name);
DLLEXPORT int GetNeuronLayerWidth (char *neuron_layer_name);

// ----------------------------------------------------------------------
// Other Functions
// ----------------------------------------------------------------------
DLLEXPORT XY_PAIR get_xy_pair_from_input_layer (char *input_name);

// Global Variables

#ifdef __cplusplus
}
#endif

#endif
