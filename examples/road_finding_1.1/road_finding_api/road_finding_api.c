#include "road_finding_api.h"
#include "road_finding_util.h"
#include "../road_finding.h"	//mae.h include conflicts with cv.h
#include <mae_cv.h>
#include <mae_api.h>
#include <stdio.h>
#include <stdio.h>

#ifdef DEBUG
#include <opencv/highgui.h>
#endif

int MAE_RoadFinding_GetImageWidth()
{
	return IMAGE_WIDTH;
}

int MAE_RoadFinding_GetImageHeight()
{
	return IMAGE_HEIGHT;}

void MAE_RoadFinding_Init()
{
	MAE_Init();
}

void MAE_RoadFinding_Quit()
{
	MAE_Quit();
}

void MAE_RoadFinding_LoadTraining(char* file_name)
{
	LoadTraining(file_name);
}

void MAE_RoadFinding_SaveTraining(char* file_name)
{
	SaveTraining(file_name);
}

void MAE_RoadFinding_Train(IplImage* input, IplImage* output)
{
	PutImageIntoNeuronLayer((char*)"in_text_detect_neuron_layer", input);

	PutImageIntoNeuronLayer((char*)"nl_text_detect", output);

	MaeExecuteCommandScript((char*)"forward outputs;train network;");
}

IplImage* MAE_RoadFinding_Test(IplImage* image)
{
	PutImageIntoNeuronLayer((char*)"in_text_detect_neuron_layer", image);

	MaeExecuteCommandScript((char*)"forward neural_layers;");

	return GetImageFromNeuronLayer((char*)"nl_text_detect", Color);
}

void MAE_RoadFinding_TrainImage(IplImage* input, IplImage* output, int steps)
{
	int radius = 12;

	if (CV_IS_IMAGE(input) && CV_IS_IMAGE(output))
	{
		IplImage* resized_input = MAE_RoadFinding_ResizeImage(input, 1.);///(float)MAE_TextDetect_GetImageScale());
		IplImage* resized_output = MAE_RoadFinding_ResizeImage(output, 1./(float)MAE_RoadFinding_GetImageScale());

		CvPoint* shift_points = (CvPoint*) malloc(steps * sizeof(CvPoint));

		if (MAE_RoadFinding_CalcEllipsePoints(radius*2, radius, 0, steps, shift_points) == 0)
		{
			int shift_step;
			for (shift_step = 0; shift_step < steps; shift_step++)
			{
				CvPoint shift_point = shift_points[shift_step];

				IplImage * translated_input = cvCloneImage(resized_input);
				IplImage * translated_output = cvCloneImage(resized_output);

				MAE_RoadFinding_ShiftImage(resized_input, translated_input,
						(float)shift_point.x*(float)MAE_RoadFinding_GetImageScale(), (float)shift_point.x*(float)MAE_RoadFinding_GetImageScale(),
						(float)shift_point.y*(float)MAE_RoadFinding_GetImageScale(), (float)shift_point.y*(float)MAE_RoadFinding_GetImageScale());

				MAE_RoadFinding_ShiftImage(resized_output, translated_output,
						shift_point.x, shift_point.x,
						shift_point.y, shift_point.y);

			#ifdef DEBUG
				cvShowImage("Training: input", translated_input);
				cvShowImage("Training: output", translated_output);
				cvWaitKey(1);
			#endif

				MAE_RoadFinding_Train(translated_input, translated_output);

				cvReleaseImage( &translated_input );
				cvReleaseImage( &translated_output );
			}

			MAE_RoadFinding_Train(resized_input, resized_output);

		}
		if (shift_points != NULL)
			free(shift_points);
	}
}

CvRect MAE_RoadFinding_GetTextROI(IplImage* image, int width, int height)
{
	CvRect roi = cvRect(0, 0, -1, -1);

	if ( !CV_IS_IMAGE(image) )
		return roi;

	IplImage* resized_image = MAE_RoadFinding_ResizeImage(image, 1.);///(float)MAE_TextDetect_GetImageScale());

#ifdef DEBUG
	IplImage* image_show = MAE_RoadFinding_ResizeImage(image, 1./(float)MAE_RoadFinding_GetImageScale());
	cvShowImage("Resized Input...", image_show);
	cvWaitKey(1);
	cvReleaseImage(&image_show);
#endif

	IplImage* network_output = MAE_RoadFinding_Test(resized_image);

	roi = MAE_RoadFinding_GetMaxActivationROI(network_output,
			width/MAE_RoadFinding_GetImageScale(),
			height/MAE_RoadFinding_GetImageScale());

	roi.x *= MAE_RoadFinding_GetImageScale();
	roi.y *= MAE_RoadFinding_GetImageScale();
	roi.width *= MAE_RoadFinding_GetImageScale();
	roi.height *= MAE_RoadFinding_GetImageScale();

	cvReleaseImage( &network_output );
	cvReleaseImage( &resized_image );

	return roi;
}

IplImage* MAE_RoadFinding_GetNetworkOutput(IplImage* image)
{
	IplImage* output_image = NULL;

	if ( CV_IS_IMAGE(image) )
	{
		IplImage* resized_image = MAE_RoadFinding_ResizeImage(image, 1.);///(float)MAE_TextDetect_GetImageScale());

		IplImage* network_output = MAE_RoadFinding_Test(resized_image);

		output_image = MAE_RoadFinding_ResizeImage(network_output, MAE_RoadFinding_GetImageScale());

		cvReleaseImage( &network_output );
		cvReleaseImage( &resized_image );
	}

	return output_image;
}
