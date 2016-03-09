#include <opencv/cv.h>
#include "traffic_sign_filters.h"
#include "traffic_sign_user_functions.h"
#include "traffic_sign_utils.h"
#include "../traffic_sign_clahe/clahe.h"

void equalize_image(IplImage *img)
{
	IplImage *b = NULL;
	IplImage *g = NULL;
	IplImage *r = NULL;

	if (img->nChannels == 1)
	{
		cvEqualizeHist(img, img);
	}
	else if (img->nChannels == 3)
	{
		b = cvCreateImage(cvGetSize(img), 8, 1);
		g = cvCreateImage(cvGetSize(img), 8, 1);
		r = cvCreateImage(cvGetSize(img), 8, 1);

		cvSplit(img, b, g, r, NULL);

		cvEqualizeHist(b,b);
		cvEqualizeHist(g,g);
		cvEqualizeHist(r,r);

		cvMerge(b,g,r,NULL,img);

		cvReleaseImage(&b);
		cvReleaseImage(&g);
		cvReleaseImage(&r);
	}
}

void equalize_clahe_image(IplImage *img)
{
	IplImage *b = NULL;
	IplImage *g = NULL;
	IplImage *r = NULL;

	if (img->nChannels == 1)
	{
		cvCLAdaptEqualize(img, img, 2, 2, 256, 1.4, CV_CLAHE_RANGE_FULL);
	}
	else if (img->nChannels == 3)
	{
		b = cvCreateImage(cvGetSize(img), 8, 1);
		g = cvCreateImage(cvGetSize(img), 8, 1);
		r = cvCreateImage(cvGetSize(img), 8, 1);

		cvSplit(img, b, g, r, NULL);

		cvCLAdaptEqualize(b, b, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);
		cvCLAdaptEqualize(g, g, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);
		cvCLAdaptEqualize(r, r, 2, 2, 256, 1.4f, CV_CLAHE_RANGE_FULL);

		cvMerge(b,g,r,NULL,img);

		cvReleaseImage(&b);
		cvReleaseImage(&g);
		cvReleaseImage(&r);
	}
}

void
rotate_image(IplImage *img, double angle, double scale)
{
	CvSize img_size = cvGetSize(img);
	IplImage *tmp = cvCreateImage(img_size,img->depth, img->nChannels);
	CvMat *rotate = cvCreateMat(2,3,CV_32F);
	CvPoint2D32f center = cvPoint2D32f(
			((double)img_size.width)/2.0,
			((double)img_size.height)/2.0);
	cv2DRotationMatrix(center, angle, scale, rotate);
	cvWarpAffine(img, tmp, rotate, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0) );
	cvCopyImage(tmp, img);
	cvReleaseImage(&tmp);
}

void
translate_image(IplImage *img, int dx, int dy)
{
	CvRect roi = cvGetImageROI(img);
	roi.x += dx;
	roi.y += dy;
	cvResetImageROI(img);
	cvSetImageROI(img, roi);
}


void traffic_sign_reshape_opencv (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL;
	NEURON_LAYER *nl_input = NULL;
	IplImage *image_input = NULL;
	IplImage *image_output = NULL;
	CvRect image_roi, nl_roi;
	int wi, hi, wo, ho;

	//printf ("Rotation angle = %f\n", reshape_filter_rotation_angle);

	//printf ("Scale factor = %f\n", reshape_filter_scale_factor);

	image_roi = cvRect (g_roi_x1, g_roi_y1, g_roi_x2-g_roi_x1, g_roi_y2-g_roi_y1);
	nl_roi = cvRect(g_img_x1, g_img_y1, g_img_w, g_img_h);

	if (image_roi.height <= 0 || image_roi.width <= 0)
		return;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = g_img_w;
	hi = g_img_h;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	image_input = cvCreateImage(cvSize(wi, hi), 8, nl_input->output_type == COLOR ? 3 : 1);
	image_output = cvCreateImage(cvSize(wo, ho), 8, nl_input->output_type == COLOR ? 3 : 1);

	copy_neuron_layer_to_image(image_input, &nl_roi, nl_input);

	equalize_clahe_image(image_input);

	rotate_image(image_input,	reshape_filter_rotation_angle, reshape_filter_scale_factor);

	cvSetImageROI(image_input, image_roi);

	translate_image(image_input, reshape_filter_offset_x, reshape_filter_offset_y);

	cvResize (image_input, image_output, CV_INTER_LINEAR);

	copy_image_to_neuron_layer(nl_output, NULL, image_output);

	cvReleaseImage(&image_input);
	cvReleaseImage(&image_output);
}
