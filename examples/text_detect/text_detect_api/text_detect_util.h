#ifndef TEXT_DETECT_UTIL_H_
#define TEXT_DETECT_UTIL_H_

#include <opencv/cv.h>

#ifdef __cplusplus
extern "C" {
#endif

int MAE_TextDetect_ShiftImage(IplImage* src, IplImage* dst, int bottom_offset_x, int top_offset_x, int bottom_offset_y, int top_offset_y);

int MAE_TextDetect_CalcCirclePoints(int radius, int steps, CvPoint * points);

int MAE_TextDetect_CalcEllipsePoints(int major_axis, int minor_axis, int angle_degrees, int steps, CvPoint * points);

CvRect MAE_TextDetect_GetMaxActivationROI(IplImage* image, int width, int height);

IplImage* MAE_TextDetect_ResizeImage(const IplImage *origImg, float scaleFactor);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_DETECT_UTIL_H_ */
