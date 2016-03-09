#ifndef MAE_WRAPPER_H_
#define MAE_WRAPPER_H_

#include <opencv/cv.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	Color,
	Grayscale,
	GrayscaleFloat
} NeuronLayerType;

void MAE_Init();
void MAE_Quit();
void LoadTraining(char* file_name);
void SaveTraining(char* file_name);
int PutImageIntoNeuronLayer(char* neuron_layer_name, IplImage* image);
IplImage* GetImageFromNeuronLayer(char* neuron_layer_name, NeuronLayerType neuron_layer_type);
IplImage* DetectTextRegion(IplImage* image);

#ifdef __cplusplus
}
#endif

#endif /* MAE_WRAPPER_H_ */
