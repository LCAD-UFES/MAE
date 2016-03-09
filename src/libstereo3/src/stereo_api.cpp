/*! \file
* \brief Stereo system API module.
*/

#include "parser.h"
#include "common.hpp"
#include "stereo_api.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define DIMENTION_SIZE 3
#define OPENCV_FORMAT 0
#define MAE_FORMAT 1

/* Global Variables */
int g_image_width, g_image_height; // Image size
CvMat *g_matrix_left, *g_matrix_right, *g_distortion_left, *g_distortion_right; // Individual camera parameters
CvMat *g_R, *g_T, *g_om; // Stereo parameters
CvMat *g_Rl, *g_Rr, *g_Pl, *g_Pr, *g_reprojection_matrix; // Stereo rectification parameters
CvMat *g_left_mapx, *g_left_mapy, *g_right_mapx, *g_right_mapy; // Stereo rectification maps
IplImage *g_img_left, *g_img_right; // Original images
IplImage *g_rect_img_left, *g_rect_img_right; // Rectified images resulting from the stereo rectifying

/*! 
*********************************************************************************
* \brief Convert from/to both MAE specific image format and OpenCV interleaved image format.
* \param[in] p_type The convertion type must be 0 (CONVERT_TO_OPENCV) or 1 (CONVERT_TO_MAE).
* \pre None.
* \post The image data is converted.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
int convert(unsigned char *p_raw_image_data, IplImage *image, int p_type)
{
	int i, j, aux;
	int h = g_image_height;
	int w = g_image_width;

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			aux = h - j - 1;

			switch (p_type)
			{
				case CONVERT_TO_OPENCV:
					if (image->nChannels == 3)
					{
						(image->imageData + image->widthStep*aux)[3*i + 2]  = p_raw_image_data[aux + i * h + 0 * w * h];
						(image->imageData + image->widthStep*aux)[3*i + 1]  = p_raw_image_data[aux + i * h + 1 * w * h];
						(image->imageData + image->widthStep*aux)[3*i + 0]  = p_raw_image_data[aux + i * h + 2 * w * h];
					}
					else if (image->nChannels == 1)
					{
						//Erro("One-channel Image convertion not implemented.", "", "");
					}
					break;
				case CONVERT_TO_MAE: 
					if (image->nChannels == 3)
					{
						p_raw_image_data[aux + i * h + 0 * w * h] = (image->imageData + image->widthStep*aux)[3*i + 2];
						p_raw_image_data[aux + i * h + 1 * w * h] = (image->imageData + image->widthStep*aux)[3*i + 1];
						p_raw_image_data[aux + i * h + 2 * w * h] = (image->imageData + image->widthStep*aux)[3*i + 0];
					}
					else if (image->nChannels == 1)
					{
						char grayScale = (image->imageData + image->widthStep*aux)[i];
						p_raw_image_data[aux + i * h + 0 * w * h] = grayScale;
						p_raw_image_data[aux + i * h + 1 * w * h] = grayScale;
						p_raw_image_data[aux + i * h + 2 * w * h] = grayScale;
					}
					
					break;
				default:
                    return -1;
			}
			
		}
	}

	return 0;
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the left image
* \param[in] p_img_left The left image.
* \param[out] p_rect_img_left The left image undistorted and rectified.
* \param[in] p_format The desired format, either OPENCV_FORMAT or MAE_FORMAT.
* \pre None.
* \post The left image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
int _RectifyLeftImage (void* p_img_left, void* p_rect_img_left, int p_format)
{
	if (p_format == OPENCV_FORMAT)
		cvCopy((IplImage*)p_img_left, (IplImage*)g_img_left);
	else if (p_format == MAE_FORMAT)
		convert((unsigned char*)p_img_left, (IplImage*)g_img_left, CONVERT_TO_OPENCV);

	g_rect_img_left = cvCloneImage(g_img_left);
	cvRemap(g_img_left, g_rect_img_left, g_left_mapx, g_left_mapy); // undistort and rectify

	if (p_format == OPENCV_FORMAT)
		cvCopy((IplImage*)g_rect_img_left, (IplImage*)p_rect_img_left);
	else if (p_format == MAE_FORMAT)
		convert((unsigned char*)p_rect_img_left, (IplImage*)g_rect_img_left, CONVERT_TO_MAE);

	return 0;
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the right image
* \param[in] p_img_right The right image.
* \param[out] p_rect_img_right The right image undistorted and rectified.
* \param[in] p_format The desired format, either OPENCV_FORMAT or MAE_FORMAT.
* \pre None.
* \post The right image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
int _RectifyRightImage (void* p_img_right, void* p_rect_img_right, int p_format)
{
	if (p_format == OPENCV_FORMAT)
		cvCopy((IplImage*)p_img_right, (IplImage*)g_img_right);
	else if (p_format == MAE_FORMAT)
		convert((unsigned char*)p_img_right, (IplImage*)g_img_right, CONVERT_TO_OPENCV);

	g_rect_img_right = cvCloneImage(g_img_right);
	cvRemap(g_img_right, g_rect_img_right, g_right_mapx, g_right_mapy); // undistort and rectify

	if (p_format == OPENCV_FORMAT)
		cvCopy((IplImage*)g_rect_img_right, (IplImage*)p_rect_img_right);
	else if (p_format == MAE_FORMAT)
		convert((unsigned char*)p_rect_img_right, (IplImage*)g_rect_img_right, CONVERT_TO_MAE);

	return 0;
}

/*! 
*********************************************************************************
* \brief Initializes the stereo system module.
* \pre None.
* \post The stereo system module initialized.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
 
DLLEXPORT int StereoInitialize (int image_width, int image_height)
{	
	DEBUG("StereoInitialize:\n","","");
	DEBUG("\t->Creating global variables...\n","","");
	g_image_width = image_width;
	g_image_height = image_height;

	g_T = cvCreateMat(3, 1, CV_64F);
	g_R = cvCreateMat(3, 3, CV_64F);
	g_om = cvCreateMat(3, 1, CV_64F);
	g_matrix_left = cvCreateMat(3, 3, CV_64F);
	g_matrix_right = cvCreateMat(3, 3, CV_64F);
	g_distortion_left = cvCreateMat(5, 1, CV_64F);
	g_distortion_right = cvCreateMat(5, 1, CV_64F);
	g_Rl = cvCreateMat(3, 3, CV_64F);
	g_Rr = cvCreateMat(3, 3, CV_64F);
	g_Pl = cvCreateMat(3, 4, CV_64F);
	g_Pr = cvCreateMat(3, 4, CV_64F);
	g_reprojection_matrix = cvCreateMat(4, 4, CV_64F);

	g_left_mapx = cvCreateMat(image_height, image_width, CV_32F);
	g_left_mapy = cvCreateMat(image_height, image_width, CV_32F);
	g_right_mapx = cvCreateMat(image_height, image_width, CV_32F);
	g_right_mapy = cvCreateMat(image_height, image_width, CV_32F);

	g_img_left = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 3);
//	g_img_left->imageData = (char*)malloc(3 * image_width * image_height * sizeof(char));
	g_img_right = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 3);
//	g_img_right->imageData = (char*)malloc(3 * image_width * image_height * sizeof(char));

	return 0;
}


/*! 
*********************************************************************************
* \brief Exits the stereo system module.	
* \pre None.
* \post The stereo system module finalized.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoTerminate (void)
{
	DEBUG("StereoTerminate:\n","","");

	DEBUG("\t->Destroying global variables...\n","","");
	
	// Release Matrices
	cvReleaseMat(&g_matrix_left); g_matrix_left = 0;
	cvReleaseMat(&g_matrix_right); g_matrix_right = 0;
	cvReleaseMat(&g_distortion_left); g_distortion_left = 0;
	cvReleaseMat(&g_distortion_right); g_distortion_right = 0;
	cvReleaseMat(&g_R); g_R = 0;
	cvReleaseMat(&g_T); g_T = 0;
	cvReleaseMat(&g_om); g_om = 0;
	cvReleaseMat(&g_Rl); g_Rl = 0;
	cvReleaseMat(&g_Rr), g_Rr = 0;
	cvReleaseMat(&g_Pl); g_Pl = 0;
	cvReleaseMat(&g_Pr); g_Pr = 0;
	cvReleaseMat(&g_reprojection_matrix); g_reprojection_matrix = 0;
	cvReleaseMat(&g_left_mapx); g_left_mapx = 0;
	cvReleaseMat(&g_left_mapy); g_left_mapy = 0;
	cvReleaseMat(&g_right_mapx); g_right_mapx = 0;
	cvReleaseMat(&g_right_mapy); g_right_mapy = 0;

	// Release Images
	cvReleaseImage(&g_rect_img_left);
	cvReleaseImage(&g_rect_img_right);
	cvReleaseImage(&g_img_left);
	cvReleaseImage(&g_img_right);
	
	return 0;
}



/*! 
*********************************************************************************
* \brief Load the individual and stereo camera parameters.	
* \pre None.
* \post The individual and stereo camera parameters are available to use.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int LoadStereoParameters (const char *strFileName)
{	
	DEBUG("LoadStereoParameters\n","","");

	GetConfig appConfig;
	appConfig.readConfigFile(strFileName);

	// Rotation Matrix
	double** rotation = appConfig.getCamera()->getRotation();
	for(int i=0; i<g_R->rows; i++)
		for(int j=0; j<g_R->cols; j++)
			cvSetReal2D(g_R, i, j, rotation[i][j]);

	/* TEST DEBUG
	*/
	for(int i=0; i<g_R->rows; i++)
		for(int j=0; j<g_R->cols; j++)
			printf("%17.10le\n", cvGetReal2D(g_R, i, j));

	// Rotation Vector
	cvRodrigues2(g_R, g_om, 0); 

	/* TEST DEBUG
	*/
	printf("%17.10le\n", cvGetReal1D(g_om, 0));
	printf("%17.10le\n", cvGetReal1D(g_om, 1));
	printf("%17.10le\n", cvGetReal1D(g_om, 2));

	/* TEST DEBUG
	cvSetReal1D(g_om, 0, 1.463556);
	cvSetReal1D(g_om, 1, -0.872299);
	cvSetReal1D(g_om, 2, 3.056505);

	cvRodrigues2(g_om, g_R, 0); 
	*/

	// Translation Vector
	double* translation = appConfig.getCamera()->getTranslation();
	for (int i=0; i<g_T->rows; i++)
		cvSetReal1D(g_T, i, translation[i]);
	
	/* TEST DEBUG
	*/
	printf("%17.10le\n", cvGetReal1D(g_T, 0));
	printf("%17.10le\n", cvGetReal1D(g_T, 1));
	printf("%17.10le\n", cvGetReal1D(g_T, 2));

	double** left_intrinsics = appConfig.getCamera()->getLeft()->getIntrinsics();
	double** right_intrinsics = appConfig.getCamera()->getRight()->getIntrinsics();
	double* left_distortion = appConfig.getCamera()->getLeft()->getDistortion();
	double* right_distortion = appConfig.getCamera()->getRight()->getDistortion();

	// Left and Right Intrinsics
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		{
			cvSetReal2D(g_matrix_left, i, j, left_intrinsics[i][j]);
			cvSetReal2D(g_matrix_right, i, j, right_intrinsics[i][j]);
		}
	}

	// Left and Right Distortion Coefs
	for(int i=0; i<5; i++)
	{
		cvSetReal1D(g_distortion_left, i, left_distortion[i]);
		cvSetReal1D(g_distortion_right, i, right_distortion[i]);
	}

	return 0;
}



/*! 
*********************************************************************************
* \brief Load the stereo rectification parameters.	
* \pre None.
* \post The rectification parameters for both left and right cameras are available to use.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int LoadStereoRectificationIndexes (void)
{	
	DEBUG("LoadStereoRectificationIndexes\n","","");
	
	cvStereoRectify(g_matrix_left, g_matrix_right, g_distortion_left, g_distortion_right, cvSize(g_image_width, g_image_height), g_R, g_T, g_Rl, g_Rr, g_Pl, g_Pr, g_reprojection_matrix, 0);

	cvInitUndistortRectifyMap(g_matrix_left, g_distortion_left, g_Rl, g_Pl, g_left_mapx, g_left_mapy);
	cvInitUndistortRectifyMap(g_matrix_right, g_distortion_right, g_Rr, g_Pr, g_right_mapx, g_right_mapy);

	return 0;
}



/*! 
*********************************************************************************
* \brief Makes up the stereo triangulation of the given points, outputing their XYZ coordinates.
* \param[in] num_points Number of points to be trianguled.
* \param[in] src_points Points (X,Y,D) to be trianguled.
* The (X,Y) are the pixel coordinates of the point at the right image.
* The D coordinate is the disparity between X coordinate of the same point on right and left images (Xright - Xleft)
* \param[out] points_3D (X,Y,Z) points (note: should be previously allocated).
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoTriangulationNew(int num_points, double **src_points, double **points_3D)
{
	CvPoint3D64f* _src = (CvPoint3D64f*) malloc (num_points*sizeof(CvPoint3D64f));
	CvPoint3D64f* _dst = (CvPoint3D64f*) malloc (num_points*sizeof(CvPoint3D64f));

	// Copy the XYD-points to the OpenCV input format
	for (int i=0; i<num_points; i++)
	{
		_src[i].x = src_points[i][0];
		_src[i].y = src_points[i][1];
		_src[i].z = src_points[i][2];
	}

	CvMat src = cvMat(num_points, 1, CV_64FC3, _src);
	CvMat dst = cvMat(num_points, 1, CV_64FC3, _dst);

    cvPerspectiveTransform(&src, &dst, g_reprojection_matrix);

	// Copy the result 3D-points to the output buffer
	for (int i=0; i<num_points; i++)
	{
		points_3D[i][0] = _dst[i].x;
		points_3D[i][1] = _dst[i].y;
		points_3D[i][2] = _dst[i].z;
	}

	// Release memory
	free(_src);
	free(_dst);
	
	return 0;
}

/*! 
*********************************************************************************
* \brief Makes up the stereo triangulation of the given point, outputing it's XYZ coordinates.
* \param[in] src_point Point (X,Y,D) to be trianguled.
* The (X,Y) are the pixel coordinates of the point at the right image.
* The D coordinate is the disparity between X coordinate of the same point on right and left images (Xright - Xleft)
* \param[out] point_3D (X,Y,Z) point (note: should be previously allocated).
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoTriangulationSinglePoint(double *src_point, double *point_3D)
{
	double **src_points = (double**)malloc(sizeof(double*));
	src_points[0] = (double*)malloc(3*sizeof(double));

	double **points_3D = (double**)malloc(sizeof(double*));
	points_3D[0] = (double*)malloc(3*sizeof(double));

	src_points[0][0] = src_point[0];
	src_points[0][1] = src_point[1];
	src_points[0][2] = src_point[2];

	int returnValue = StereoTriangulationNew(1, src_points, points_3D);

	point_3D[0] = points_3D[0][0];
	point_3D[1] = points_3D[0][1];
	point_3D[2] = points_3D[0][2];

	free(points_3D[0]);
	free(points_3D);
	free(src_points[0]);
	free(src_points);

	return returnValue;
}

/*! 
*********************************************************************************
* \brief Prepares for the stereo triangulation of the given points, outputing their XYZ coordinates.
* \param[in] num_points Number of points to be trianguled.
* \param[in] image_point_left Points (X,Y) from left image.
* \param[in] image_point_right Points (X,Y) from right image.
* \param[out] world_points (X,Y,Z) points (note: should be previously allocated).
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int StereoTriangulation (int num_points, 
								   double *world_point_left, double *world_point_right,
								   double *image_point_left, double *image_point_right)
{
	int result = 0;

	result = StereoTriangulationLeft(num_points, image_point_left, image_point_right, world_point_left);

	if(result != 0)	return result;

	result = StereoTriangulationRight(num_points, image_point_left, image_point_right, world_point_right);

	return result;
}

DLLEXPORT int StereoTriangulationLeft(int num_points, 
								   double *image_point_left, double *image_point_right,
								   double *world_point_left)
{
	int result = 0;
	double** matrix_image_points_left = NULL;
	double** matrix_world_points_left = NULL;

	matrix_image_points_left = (double**) malloc (num_points*sizeof(double*));
	matrix_world_points_left = (double**) malloc (num_points*sizeof(double*));

	for(int i = 0; i < num_points; i++)
	{
		matrix_world_points_left[i] = (double*) malloc (3*sizeof(double));

		matrix_image_points_left[i] = (double*) malloc (3*sizeof(double));

		matrix_image_points_left[i][0] = image_point_left[2 * i + 0];
		matrix_image_points_left[i][1] = image_point_left[2 * i + 1];
		matrix_image_points_left[i][2] = image_point_left[2 * i + 0] - image_point_right[2 * i + 0];
	}
	
	result = StereoTriangulationNew(num_points, matrix_image_points_left, matrix_world_points_left);

	for (int i=0; i<num_points; i++)
	{
		world_point_left[3 * i + 0] = matrix_world_points_left[i][0];
		world_point_left[3 * i + 1] = matrix_world_points_left[i][1];
		world_point_left[3 * i + 2] = matrix_world_points_left[i][2];
	}

	free(matrix_image_points_left);
	free(matrix_world_points_left);

	return result;
}

DLLEXPORT int StereoTriangulationRight(int num_points, 
								   double *image_point_left, double *image_point_right,
								   double *world_point_right)
{
	int result = 0;
	double** matrix_image_points_right = NULL;
	double** matrix_world_points_right = NULL;

	matrix_image_points_right = (double**) malloc (num_points*sizeof(double*));
	matrix_world_points_right = (double**) malloc (num_points*sizeof(double*));

	for(int i = 0; i < num_points; i++)
	{
		matrix_world_points_right[i] = (double*) malloc (3*sizeof(double));

		matrix_image_points_right[i] = (double*) malloc (3*sizeof(double));

		matrix_image_points_right[i][0] = image_point_right[2 * i + 0];
		matrix_image_points_right[i][1] = image_point_right[2 * i + 1];
		matrix_image_points_right[i][2] = image_point_right[2 * i + 0] - image_point_left[2 * i + 0];
	}
	
	result = StereoTriangulationNew(num_points, matrix_image_points_right, matrix_world_points_right);

	for (int i=0; i<num_points; i++)
	{
		world_point_right[3 * i + 0] = matrix_world_points_right[i][0];
		world_point_right[3 * i + 1] = matrix_world_points_right[i][1];
		world_point_right[3 * i + 2] = matrix_world_points_right[i][2];
	}

	free(matrix_image_points_right);
	free(matrix_world_points_right);

	return result;
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the left image.
* \pre None.
* \post The left image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int RectifyLeftImage (unsigned char *p_rect_img_left, unsigned char *p_img_left)
{
	return _RectifyLeftImage (p_img_left, p_rect_img_left, MAE_FORMAT);
}


/*! 
*********************************************************************************
* \brief Undistort and Rectify the left image (OpenCV IPLImage format)
* \pre None.
* \post The left image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
DLLEXPORT int RectifyLeftImageNew (void* p_img_left, void* p_rect_img_left)
{
	return _RectifyLeftImage (p_img_left, p_rect_img_left, OPENCV_FORMAT);
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the right image.
* \pre None.
* \post The right image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int RectifyRightImage (unsigned char *p_rect_img_right, unsigned char *p_img_right)
{
	return _RectifyRightImage(p_img_right, p_rect_img_right, MAE_FORMAT);
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the right image.
* \pre None.
* \post The right image is undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int RectifyRightImageNew (void* p_img_right, void* p_rect_img_right)
{
	return _RectifyRightImage(p_img_right, p_rect_img_right, OPENCV_FORMAT);
}

/*! 
*********************************************************************************
* \brief Undistort and Rectify the left and right images.	
* \pre None.
* \post The left and right images are undistorted and rectified.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int RectifyStereoPair (unsigned char *p_rect_img_left, unsigned char *p_rect_img_right, unsigned char *p_img_left, unsigned char *p_img_right)
{	
	int left = RectifyLeftImage(p_rect_img_left, p_img_left);
	int right = RectifyRightImage(p_rect_img_right, p_img_right);

	if (left == 0 && right == 0)
		return 0;

	return -1;
}

/*! 
*********************************************************************************
* \brief Save the stereo maps generated.
* \param[in] p_print_disparity Flag to save the disparity map.
* \param[in] p_print_normalized_disparity Flag to save the normalized disparity map.
* \param[in] p_print_depth Flag to save the depth map.
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
DLLEXPORT int SaveStereoMaps (int image_width, int image_height, int p_print_disparity, int p_print_normalized_disparity, int p_print_depth)
{
	CvSize imgSize = cvSize(image_width, image_height);
	IplImage* depth_img = cvCreateImage(imgSize, IPL_DEPTH_32F, 3);
	CvMat* normalized_disparity_map = cvCreateMat(image_height, image_width, CV_8U );
	CvMat* disparity_map = cvCreateMat(image_height, image_width, CV_16SC1 );
	
	IplImage* gray_rect_left_image = cvCreateImage(imgSize, g_rect_img_left->depth, 1);
	cvCvtColor(g_rect_img_left, gray_rect_left_image, CV_BGR2GRAY);

	IplImage* gray_rect_right_image = cvCreateImage(imgSize, g_rect_img_right->depth, 1);
	cvCvtColor(g_rect_img_right, gray_rect_right_image, CV_BGR2GRAY);

	CvStereoBMState *BMState = cvCreateStereoBMState(CV_STEREO_BM_BASIC, 0);
	BMState->preFilterSize=41;
	BMState->preFilterCap=31;
    BMState->SADWindowSize=41;
    BMState->minDisparity=-64;
    BMState->numberOfDisparities=128;
    BMState->textureThreshold=10;
    BMState->uniquenessRatio=15;

	cvFindStereoCorrespondenceBM(gray_rect_left_image, gray_rect_right_image, disparity_map, BMState);

	cvReleaseStereoBMState(&BMState);

	if (p_print_disparity)
	{
		cvSaveImage("disparity_map.bmp", disparity_map);
	}

	if (p_print_normalized_disparity)
	{
		cvNormalize(disparity_map, normalized_disparity_map, 0, 256, CV_MINMAX, NULL);
		cvSaveImage("normalized_disparity_map.bmp", normalized_disparity_map);
	}

	if (p_print_depth)
	{
		cvReprojectImageTo3D(disparity_map, depth_img, g_reprojection_matrix);
		cvSaveImage("depth.bmp", depth_img);
	}
	
	return 0;
}


/*! 
*********************************************************************************
* \brief Gets the point in one image.
* \param[in] nCameraSide The camera side must be 1 (RIGHT_CAMERA) or 0 (LEFT_CAMERA).
* \param[in] p_dblImagePoint The image point.
* \param[in] dblDistance The world point distance.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
// TODO: implements
DLLEXPORT int GetWorldPointAtDistance (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide)
{
	/*int nargout = 2;
	mxArray *ml_XR, *ml_XL, *ml_xR, *ml_D;
	
	ml_XR = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_XL = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, 1, mxREAL);
	ml_D = mxCreateDoubleMatrix (1, 1, mxREAL);

	memcpy(mxGetPr(ml_xR), p_dblImagePoint, 2 * sizeof (double));
	memcpy(mxGetPr(ml_D), &dblDistance, sizeof (double));
	
    mlfGetWorldPointAtDistance (nargout, &ml_XL, &ml_XR, ml_xR, g_KK_left, g_KK_right, g_R, g_T, ml_D);
    
    switch (nCameraSide)
    {
            case LEFT_CAMERA:
                    memcpy (p_dblWorldPoint, mxGetPr(ml_XL), 3 * sizeof (double));
                    break;
            case RIGHT_CAMERA:
                    memcpy (p_dblWorldPoint, mxGetPr(ml_XR), 3 * sizeof (double));
                    break;
            default:
                    ERROR ("Invalid camera side (GetWorldPointAtDistance)", "", "");
                    return -1;
    }
        
	mxDestroyArray(ml_XR); 
	mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_xR); 
	mxDestroyArray(ml_D);*/
        
	return 0;
}


/*! 
*********************************************************************************
* \brief GetWorldPointAtDistanceRight3.
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
// TODO: implements
DLLEXPORT int GetWorldPointAtDistanceRight (double *p_dblWorldPoint, double *p_dblImagePoint, double dblDistance, int nCameraSide)
{
	/*int nargout = 2;
	mxArray *ml_XR, *ml_XL, *ml_xR, *ml_D;
	
	ml_XR = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_XL = mxCreateDoubleMatrix (3, 1, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, 1, mxREAL);
	ml_D = mxCreateDoubleMatrix (1, 1, mxREAL);

	memcpy(mxGetPr(ml_xR), p_dblImagePoint, 2 * sizeof (double));
	memcpy(mxGetPr(ml_D), &dblDistance, sizeof (double));
	
        mlfGetWorldPointAtDistanceRight (nargout, &ml_XL, &ml_XR, ml_xR, g_KK_left, g_KK_right, g_R, g_T, ml_D);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XL), 3 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblWorldPoint, mxGetPr(ml_XR), 3 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (GetWorldPointAtDistance)", "", "");
                        return -1;
        }
        
	mxDestroyArray(ml_XR); 
	mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_xR); 
	mxDestroyArray(ml_D);*/
        
	return 0;
}


/*! 
*********************************************************************************
* \brief CameraProjection3.
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
// TODO: implements
DLLEXPORT int CameraProjection (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide)
{
	/*int nargout = 2;
	mxArray *ml_XL, *ml_xL, *ml_xR;
	
	ml_XL = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_xL = mxCreateDoubleMatrix (2, num_points, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, num_points, mxREAL);

	memcpy(mxGetPr(ml_XL), p_dblWorldPoint, num_points * 3 * sizeof (double));
	
        mlfCameraProjection (nargout, &ml_xL, &ml_xR, ml_XL, g_KK_left, g_KK_right, g_R, g_T);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xL), num_points * 2 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xR), num_points * 2 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (CameraProjection)", "", "");
                        return -1;
        }        
         
	mxDestroyArray(ml_XL); 
	mxDestroyArray(ml_xL); 
	mxDestroyArray(ml_xR);*/
        
	return 0;
}


/*! 
*********************************************************************************
* \brief CameraProjectionRight3.
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
// TODO: implements
DLLEXPORT int CameraProjectionRight (int num_points, double *p_dblImagePoint, double *p_dblWorldPoint, int nCameraSide)
{
	/*int nargout = 2;
	mxArray *ml_XR, *ml_xL, *ml_xR;
	
	ml_XR = mxCreateDoubleMatrix (3, num_points, mxREAL);
	ml_xL = mxCreateDoubleMatrix (2, num_points, mxREAL);
	ml_xR = mxCreateDoubleMatrix (2, num_points, mxREAL);

	memcpy(mxGetPr(ml_XR), p_dblWorldPoint, num_points * 3 * sizeof (double));
	
        mlfCameraProjectionRight (nargout, &ml_xL, &ml_xR, ml_XR, g_KK_left, g_KK_right, g_R, g_T);
        
        switch (nCameraSide)
        {
                case LEFT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xL), num_points * 2 * sizeof (double));
                        break;
                case RIGHT_CAMERA:
                        memcpy (p_dblImagePoint, mxGetPr(ml_xR), num_points * 2 * sizeof (double));
                        break;
                default:
                        ERROR ("Invalid camera side (CameraProjection)", "", "");
                        return -1;
        }        
         
	mxDestroyArray(ml_XR); 
	mxDestroyArray(ml_xL); 
	mxDestroyArray(ml_xR); */
        
	return 0;
}





/*! 
*********************************************************************************
* \brief TransformPoints3.
* \pre The stereo module initialized.
* \post None.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/
// TODO: test
DLLEXPORT int TransformPoints (int num_points, double *output_points, double *input_points, double *euler_angles, double *displacement, double *mass_center)
{
	TMatrix PIN(DIMENTION_SIZE, num_points);
	TMatrix E(DIMENTION_SIZE, 1);
	TMatrix D(DIMENTION_SIZE, 1);
	TMatrix MC(DIMENTION_SIZE, 1);
	
	for(int j=0; j<num_points; j++)
		for(int i=0; i<DIMENTION_SIZE; i++)
			PIN(i,j) = input_points[i + j];

	for(int i=0; i<DIMENTION_SIZE; i++)
		E(i,0) = euler_angles[i];

	for(int i=0; i<DIMENTION_SIZE; i++)
		D(i,0) = displacement[i];

	for(int i=0; i<DIMENTION_SIZE; i++)
		MC(i,0) = mass_center[i];

	//Put the input points in homogeneous coordinates
	TMatrix P1(PIN);
	P1.SetSize(DIMENTION_SIZE+1, num_points);
	for(int j=0; j<num_points; j++)
		P1(DIMENTION_SIZE,j) = 1.0;

	//Put the origin at the mass center
	TMatrix T1(DIMENTION_SIZE+1, DIMENTION_SIZE+1);
	T1.Unit();
	for(int i=0; i<DIMENTION_SIZE; i++)
		T1(i,DIMENTION_SIZE) = -MC(i,0);

	TMatrix P2(DIMENTION_SIZE+1, num_points);
	P2=T1*P1;

	//Calculate the rotation matrix from the Euler angles
	CvMat *src = cvCreateMat(DIMENTION_SIZE, 1, CV_64F);
	CvMat *dst = cvCreateMat(DIMENTION_SIZE, DIMENTION_SIZE, CV_64F);

	for(int i=0; i<DIMENTION_SIZE; i++)
		cvmSet(src, i, 0, E(i,0));

	cvRodrigues2(src, dst);

	TMatrix R(DIMENTION_SIZE+1, DIMENTION_SIZE+1);
	R.Unit();
	for(int i=0; i<DIMENTION_SIZE; i++)
		for(int j=0; j<DIMENTION_SIZE; j++)
			R(i,j) = cvmGet(dst, i, j);

	//Rotate the points around the new origin
	TMatrix P3(DIMENTION_SIZE+1, num_points);
	P3=R*P2;

	//Back to the old origin
	TMatrix T2(DIMENTION_SIZE+1, DIMENTION_SIZE+1);
	T2.Unit();
	for(int i=0; i<DIMENTION_SIZE; i++)
		T2(i,DIMENTION_SIZE) = MC(i,0);

	TMatrix P4(DIMENTION_SIZE+1, num_points);
	P4=T2*P3;
	
	//Translate the points
	TMatrix T3(DIMENTION_SIZE+1, DIMENTION_SIZE+1);
	T3.Unit();
	for(int i=0; i<DIMENTION_SIZE; i++)
		T3(i,DIMENTION_SIZE) = D(i,0);

	TMatrix P5(DIMENTION_SIZE+1, num_points);
	P5=T3*P4;


	for(int j=0; j<num_points; j++)
	{
		for(int i=0; i<DIMENTION_SIZE; i++)
			output_points[i + j] = P5(i,j) / P5(DIMENTION_SIZE,j);
	}
        
	return 0;
}

DLLEXPORT int GetCameraParameters (ImagePoint* fov, ImagePoint* principal_point, double* focal_length, double* aspect_ratio, int nCameraSide)
{
	double fovx;
	double fovy;
	double focalLength;
	double aspectRatio;
	CvMat* camera_matrix;
	CvPoint2D64f principalPoint;

	if (nCameraSide == LEFT_CAMERA)
		camera_matrix = g_matrix_left;
	else
		camera_matrix = g_matrix_right;

	cvCalibrationMatrixValues(camera_matrix, cvSize(g_image_width, g_image_height), 0, 0, &fovx, &fovy, &focalLength, &principalPoint, &aspectRatio);

	(*fov).x = fovx;
	(*fov).y = fovy;

	(*principal_point).x = principalPoint.x;
	(*principal_point).y = principalPoint.y;

	(*focal_length) = focalLength;

	(*aspect_ratio) = aspectRatio;

	return 0;
}

DLLEXPORT double GetStereoBaseline (void)
{
	double invBaseline = cvGetReal2D(g_reprojection_matrix, 3, 2);

	if (abs(invBaseline) > 0.0)
		return -1.0 / invBaseline;
	else
		return 0.0;
}

DLLEXPORT int ConvertImage (unsigned char *p_raw_image_data, void *image, int p_type)
{
	return convert(p_raw_image_data, (IplImage*) image, p_type);
}
