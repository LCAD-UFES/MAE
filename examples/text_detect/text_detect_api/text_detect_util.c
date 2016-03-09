#include "text_detect_util.h"
#include <mae_cv.h>
#include <mae_api.h>

#ifdef DEBUG
#include <opencv/highgui.h>
#endif

int MAE_TextDetect_ShiftImage(IplImage* src, IplImage* dst, int bottom_offset_x, int top_offset_x, int bottom_offset_y, int top_offset_y)
{
	CvPoint2D32f srcQuad[4], dstQuad[4];
	CvMat* warp_matrix = NULL;

	if ( !CV_IS_IMAGE(src) || !CV_IS_IMAGE(dst) )
		return -1;

	cvZero(dst);

	//src Top left
	srcQuad[0].x  = top_offset_x;
	srcQuad[0].y  = top_offset_y;
	//src Top right
	srcQuad[1].x  = src->width + top_offset_x;
	srcQuad[1].y =  top_offset_y;
	//src Bottom left
	srcQuad[2].x  = bottom_offset_x;
	srcQuad[2].y =  src->height + bottom_offset_y;
	//src Bot right
	srcQuad[3].x  = src->width + bottom_offset_x;
	srcQuad[3].y =  src->height + bottom_offset_y;

	//dst  Top left
	dstQuad[0].x  = 0;
	dstQuad[0].y  = 0;
	//dst  Top right
	dstQuad[1].x  = src->width;
	dstQuad[1].y =  0;
	//dst  Bottom left
	dstQuad[2].x  = 0;
	dstQuad[2].y =  src->height;
	//dst  Bot right
	dstQuad[3].x  = src->width;
	dstQuad[3].y =  src->height;

	warp_matrix = cvCreateMat(3,3,CV_32FC1);
	cvZero(warp_matrix);

	cvGetPerspectiveTransform(
	   srcQuad,
	   dstQuad,
	   warp_matrix
	);

	cvWarpPerspective( src, dst, warp_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0) );

	cvReleaseMat(&warp_matrix);

	return 0;
}

int MAE_TextDetect_CalcCirclePoints(int radius, int steps, CvPoint * points)
{
	int step;
	double angle_radians;

	if (radius <= 0 || steps <= 0 || points == 0)
		return -1;

	for (angle_radians = 0, step=0; angle_radians < 2.0 * M_PI; angle_radians += (2.0 * M_PI) / (double) steps, step++)
	{
		int x = (int)( ((double)radius) * cos(angle_radians) );
		int y = (int)( ((double)radius) * sin(angle_radians) );

		if (step < steps)
		{
			points[step].x = x;
			points[step].y = y;
		}
	}
	return 0;
}

int MAE_TextDetect_CalcEllipsePoints(int major_axis, int minor_axis, int angle_degrees, int steps, CvPoint * points)
{
	int step = 0;
	int alpha_degrees;

	if (major_axis <= 0 || minor_axis <= 0 || steps <= 0 || points == 0)
		return -1;

	if (steps == 1)
	{
		points[step].x = 0;
		points[step].y = 0;
		return 0;
	}

  // Angle is given by Degree Value
  double angle_radians = -angle_degrees * (M_PI / 180);
  double sin_angle_radians = sin(angle_radians);
  double cos_angle_radians = cos(angle_radians);

  for (alpha_degrees = 0, step=0; alpha_degrees < 360; alpha_degrees += 360 / steps, step++)
  {
    double alpha_radians = alpha_degrees * (M_PI / 180) ;
    double sin_alpha_radians = sin(alpha_radians);
    double cos_alpha_radians = cos(alpha_radians);

    int x = (major_axis * cos_alpha_radians * cos_angle_radians - minor_axis * sin_alpha_radians * sin_angle_radians);
    int y = (major_axis * cos_alpha_radians * sin_angle_radians + minor_axis * sin_alpha_radians * cos_angle_radians);

		if (step < steps)
		{
			points[step].x = x;
			points[step].y = y;
		}
	}
	return 0;
}

#define CV_RED cvScalar(0,0,255,0)
void MAE_TextDetect_DrawRect(IplImage* img, CvRect* rect, CvScalar color)
{
	int thick=1;
	int type = 8;
	int shift = 0;

	cvRectangle(img,cvPoint(rect->x,rect->y),cvPoint(rect->x+rect->width,rect->y+rect->height),color,thick,type,shift);
}

CvRect MAE_TextDetect_GetMaxActivationROI(IplImage* image, int width, int height)
{
	CvRect roi = cvRect(0, 0, -1, -1);
	CvRect mask = cvRect(0, 0, -1, -1);

	if ( !CV_IS_IMAGE(image) )
		return roi;

	IplImage* image_copy = cvCreateImage(cvGetSize(image), image->depth, 1);
	if ( image->nChannels > 1 && cvGetImageCOI(image) <= 0)
		cvCvtColor(image, image_copy, CV_BGR2GRAY);
	else
		cvCopyImage(image, image_copy);

	int width_min = MIN(image_copy->width, width);
	int height_min = MIN(image_copy->height, height);
	int width_max = MAX(image_copy->width, width);
	int height_max = MAX(image_copy->height, height);

	mask = cvRect(0, 0, width_min, height_min);
	roi = cvRect(0, 0, width_max, height_max);

	int x, y;
	int max_activation = -1;
	for(x=0; x<(width_max - width_min); x++)
	{
		for(y=0; y<(height_max - height_min); y++)
		{
			mask.x = x;
			mask.y = y;
			cvSetImageROI(image_copy, mask);
			int count = cvCountNonZero(image_copy);
			if (count > max_activation)
			{
				max_activation = count;
				roi.x = x;
				roi.y = y;
				roi.width = width_min;
				roi.height= height_min;
			}
			cvResetImageROI(image_copy);

	#ifdef DEBUG
			IplImage* image_show = cvCreateImage(cvGetSize(image_copy), image_copy->depth, 3);
			cvCvtColor(image_copy, image_show, CV_GRAY2BGR);
			MAE_TextDetect_DrawRect(image_show, &roi, CV_RED);
			cvShowImage("Search ROI...", image_show);
			cvWaitKey(1);
			cvReleaseImage( &image_show );
	#endif
		}
	}
	cvReleaseImage( &image_copy );
	return roi;
}

IplImage* MAE_TextDetect_ResizeImage(const IplImage *origImg, float scaleFactor)
{
	IplImage *outImg = NULL;

	if (CV_IS_IMAGE(origImg))
	{
		CvSize origImgSize = cvGetSize(origImg);

		CvSize newImgSize = cvSize(origImgSize.width*scaleFactor,	origImgSize.height*scaleFactor);

		outImg = cvCreateImage(newImgSize, origImg->depth, origImg->nChannels);

		if (newImgSize.width > origImgSize.width && newImgSize.height > origImgSize.height)
		{
			// CV_INTER_LINEAR: good at enlarging.
			// CV_INTER_CUBIC: good at enlarging.
			cvResize(origImg, outImg, CV_INTER_LINEAR);
		}
		else
		{
			// CV_INTER_AREA: good at shrinking (decimation) only.
			cvResize(origImg, outImg, CV_INTER_AREA);
		}
	}

	return outImg;
}
