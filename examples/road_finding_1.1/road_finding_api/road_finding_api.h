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

DLLEXPORT void MAE_RoadFinding_Init();
DLLEXPORT void MAE_RoadFinding_Quit();

DLLEXPORT int MAE_RoadFinding_GetImageScale();
DLLEXPORT int MAE_RoadFinding_GetImageWidth();
DLLEXPORT int MAE_RoadFinding_GetImageHeight();

DLLEXPORT int MAE_RoadFinding_GetImageWidth();
DLLEXPORT int MAE_RoadFinding_GetImageHeight();

DLLEXPORT void MAE_RoadFinding_LoadTraining(char* file_name);
DLLEXPORT void MAE_RoadFinding_SaveTraining(char* file_name);

DLLEXPORT void MAE_RoadFinding_TrainImage(IplImage* input, IplImage* output, int steps);

DLLEXPORT CvRect MAE_RoadFinding_GetTextROI(IplImage* image, int width, int height);

DLLEXPORT IplImage* MAE_RoadFinding_GetNetworkOutput(IplImage* image);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_DETECT_API_H_ */
