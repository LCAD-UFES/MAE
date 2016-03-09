#ifndef TEXT_DETECT_API_H_
#define TEXT_DETECT_API_H_

#include <opencv/cv.h>

#ifdef	WINDOWS
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void MAE_TextDetect_Init();
DLLEXPORT void MAE_TextDetect_Quit();

DLLEXPORT int MAE_TextDetect_GetImageScale();
DLLEXPORT int MAE_TextDetect_GetImageWidth();
DLLEXPORT int MAE_TextDetect_GetImageHeight();

DLLEXPORT void MAE_TextDetect_LoadTraining(char* file_name);
DLLEXPORT void MAE_TextDetect_SaveTraining(char* file_name);

DLLEXPORT void MAE_TextDetect_TrainImage(IplImage* input, IplImage* output, int steps);

DLLEXPORT CvRect MAE_TextDetect_GetTextROI(IplImage* image, int width, int height);

DLLEXPORT IplImage* MAE_TextDetect_GetNetworkOutput(IplImage* image);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_DETECT_API_H_ */
