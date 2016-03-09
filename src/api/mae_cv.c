#include "mae_cv.h"
#include "mae_api.h"
#include <stdio.h>
#include <stdio.h>

void MAE_Init()
{
	MaeInitialize();
}

void MAE_Quit()
{
	MaeQuit();
}

void LoadTraining(char* file_name)
{
	char commands[1024];
	sprintf(commands, "destroy network; reload from \"%s\";", file_name);
	MaeExecuteCommandScript(commands);
}

void SaveTraining(char* file_name)
{
	char commands[1024];
	sprintf(commands, "unload to \"%s\";", file_name);
	MaeExecuteCommandScript(commands);
}

int MajorityVote(char* neuron_layer_name, double percentage)
{
	int i, j;
	int nClassAux, nClassID = 0;
	int nAux, nMax = 0;
	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);

	int* neural_layer_output = (int*) calloc(w * h, sizeof(int));
	neural_layer_output = GetNeuronLayerOutputInt(neuron_layer_name);

	if (neural_layer_output == NULL)
	{
		percentage = 0.0;
		return 0;
	}

	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de classes. A classe que tiver o maior numero de neuronios
	// setados sera considerada como o valor da camada de saida.
	for (i = 0; i < (h * w); i++)
	{

		nClassAux = neural_layer_output[i];
		nAux = 1;

		for (j = i + 1; j < (h * w); j++)
		{
			if (neural_layer_output[j] == nClassAux)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			nClassID = nClassAux;
		}
	}

	percentage = (float) nMax / (float) (h * w);
	return nClassID;
}

int PutImageIntoNeuronLayer(char* neuron_layer_name, IplImage* image)
{
	int x, y;
	int color;
	IplImage* out_image = NULL;

	if (image == NULL)
		return 0;

	CvSize img_size = cvGetSize(image);

	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);

	if ((w != img_size.width) || (h != img_size.height))
	{
		out_image = cvCreateImage(img_size, image->depth, image->nChannels);
		cvResize(image, out_image, CV_INTER_LINEAR);
	}
	else
		out_image = cvCloneImage(image);

	int* neural_layer_output = (int*) calloc(w * h, sizeof(int));

	for (y = 0; y < h; y++)
	{
		int yo = h - 1 - y;
		for (x = 0; x < w; x++)
		{
			uchar blue;
			uchar green;
			uchar red;

			if (out_image->nChannels == 3)
			{
				blue  = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3];
				green = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3+1];
				red   = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3+2];
			}
			else
			{
				blue  = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x];
				green = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x];
				red   = ((uchar*)(out_image->imageData + out_image->widthStep*y))[x];
			}

			color =  (  (int)red  );
			color |= (( (int)green) & 0xff) << 8;
			color |= (( (int)blue ) & 0xff) << 16;

			neural_layer_output[yo * w + x] = color;
		}
	}

	cvReleaseImage( &out_image );

	if (PutNeuronLayerOutputInt(neuron_layer_name, neural_layer_output) == 1)
		return 1;
	else
		return 0;
}

IplImage* GetImageFromNeuronLayerGrayscaleFloat(char* neuron_layer_name)
{
	return NULL;
}

IplImage* GetImageFromNeuronLayerGrayscale(char* neuron_layer_name)
{
	int max_level, min_level;
	int level;
	int x, y;

	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);

	int* neuron_layer_output = (int*) calloc(w * h, sizeof(int));
	neuron_layer_output = GetNeuronLayerOutputInt(neuron_layer_name);

	if (neuron_layer_output == NULL)
		return NULL;

	max_level = INT_MIN;
	min_level = INT_MAX;

	for (x = 0; x < (w * h); x++)
	{
		if (neuron_layer_output[x] > max_level)
			max_level = neuron_layer_output[x];
		if (neuron_layer_output[x] < min_level)
			min_level = neuron_layer_output[x];
	}

	IplImage* out_image = cvCreateImage(cvSize(w, h), 8, 1);
	for (y = 0; y < h; y++)
	{
		int yi = h - 1 - y;
		for (x = 0; x < w; x++)
		{
			level = (int) (((double) (neuron_layer_output[yi * w + x] - min_level) / (double) (max_level - min_level)) * 255);
			((uchar*)(out_image->imageData + out_image->widthStep*y))[x] = level;
		}
	}
	return out_image;
}

IplImage* GetImageFromNeuronLayerColor(char* neuron_layer_name)
{
	int x, y;
	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);

	int* neuron_layer_output = (int*) calloc(w * h, sizeof(int));
	neuron_layer_output = GetNeuronLayerOutputInt(neuron_layer_name);

	if (neuron_layer_output == NULL)
		return NULL;

	IplImage* out_image = cvCreateImage(cvSize(w, h), 8, 3);
	for (y = 0; y < h; y++)
	{
		int yi = h - 1 - y;
		for (x = 0; x < w; x++)
		{
			int red   = neuron_layer_output[yi * w + x] & 0xff;
			int green = (neuron_layer_output[yi * w + x] >> 8) & 0xff;
			int blue  = (neuron_layer_output[yi * w + x] >> 16) & 0xff;
			((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3]   = blue;
			((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3+1] = green;
			((uchar*)(out_image->imageData + out_image->widthStep*y))[x*3+2] = red;
		}
	}
	return out_image;
}

IplImage* GetImageFromNeuronLayer(char* neuron_layer_name, NeuronLayerType neuron_layer_type)
{
	switch (neuron_layer_type)
	{
	case Color:
		return GetImageFromNeuronLayerColor(neuron_layer_name);
	case Grayscale:
		return GetImageFromNeuronLayerGrayscale(neuron_layer_name);
	case GrayscaleFloat:
		return GetImageFromNeuronLayerGrayscaleFloat(neuron_layer_name);
	default:
		return GetImageFromNeuronLayerColor(neuron_layer_name);
	}
}

int SetNeuronsOfNeuronLayer(char* neuron_layer_name, int int_val)
{
	int i;
	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);
	int* neural_layer_output = (int*) calloc(w * h, sizeof(int));

	for (i = 0; i < h * w; i++)
		neural_layer_output[i] = int_val;

	if (PutNeuronLayerOutputInt(neuron_layer_name, neural_layer_output) == 1)
		return 1;
	else
		return 0;
}
