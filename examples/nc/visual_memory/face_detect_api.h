#ifndef _NC_FACE_DETECT_API_H
#define _NC_FACE_DETECT_API_H 1

// Includes
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>

using namespace std;
using namespace cv;

#ifdef __cplusplus
extern "C" {
#endif

const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;

const static Scalar green = CV_RGB(0, 255, 0);

void LoadDetectionDevice();
int detectAndDraw( Mat& img, CascadeClassifier& cascade, double* percentage);
vector<Rect> detectFaces(Mat &img, CascadeClassifier& cascade);
CvPoint* detectEyes(Rect face);
bool PutImageIntoNeuronLayer(char* neuron_layer_name, IplImage* image);
int MajorityVote(char* neuron_layer_name, double* percentage);
char* getPersonName(int personId);
IplImage* ReadRawImage(char *fileName, int imageWidth, int imageHeight);
IplImage* ReadImage(char *imageName);
void TrainNetwork(int imageWidth, int imageHeight);

#ifdef __cplusplus
}
#endif

#endif
