#include "face_detect_api.h"
#include "face_recog_api.h"

//TODO: read basePath from enviromnent
char* basePath = "/home/mberger";
IplImage* originalImg;
CascadeClassifier cascade;

/*
 * getPersonName: receives the personId and try to find this person on the image
 * if the person is on the image, returns the person name
 * if not, randomically returns the name of a person that is on the image
 */
char* getPersonName(int personId)
{
	Mat imgMat, imgMatCopy;

	// loads image
	char* imagePath = (char*)malloc(256*sizeof(char));
	sprintf(imagePath, "%s/MAE/examples/nc/visual_memory/images/image01.jpg", basePath);
	//sprintf(imagePath, "%s/MAE/examples/nc/visual_memory/images/vitor02.bmp", basePath);

	originalImg = ReadImage(imagePath);

	free(imagePath);

	imgMat = originalImg;
	imgMat.copyTo(imgMatCopy);

	double percentage;
	int result = detectAndDraw(imgMatCopy, cascade, &percentage);

	printf("ID=%d, Percentage=%.2f\n", result, 100 * percentage);

	char* name = (char*)malloc(256*sizeof(char));
	sprintf(name, "%d", result);

	return name;
}

void LoadDetectionDevice()
{
	cascade.load("haarcascade_frontalface_alt.xml");
}

int detectAndDraw(Mat& img, CascadeClassifier& cascade, double* percentage)
{
	Mat resizedFaceMat;
	IplImage* resizedImageFace = cvCreateImage(cvSize(384, 288), IPL_DEPTH_8U, 3);
    vector<Rect> faces = detectFaces(img, cascade);
    CvPoint* eyes;
    // iterate over found faces

    for(int i = 0; i < faces.size(); i++)
    {
		//Rect face = faces.at(i);
    	Rect face = faces.at(0);

    	cvSetImageROI(originalImg, face);
    	cvResize(originalImg, resizedImageFace, CV_INTER_CUBIC);
    	cvResetImageROI(originalImg);

		resizedFaceMat = resizedImageFace;

		Rect face_rect = cvRect(0, 0, 384, 288);
		cvSetImageROI(resizedImageFace, face_rect);
		eyes = detectEyes(face_rect);

    	circle(resizedFaceMat, eyes[0], 5, green); //draw left eye
    	circle(resizedFaceMat, eyes[1], 5, green); // draw right eye
    }
    cv::imshow( "result", resizedFaceMat );
	cvWaitKey(1000);

	//MaeInitialize();
	MaeExecuteCommandScript("destroy network;");

	// Load training data
	char* loadNetworkCommand = (char*)malloc(256*sizeof(char));
	sprintf(loadNetworkCommand, "reload from \"%s/MAE/examples/nc/visual_memory/visual_memory.mem\";", basePath);

	MaeExecuteCommandScript(loadNetworkCommand);
	free(loadNetworkCommand);

	PutImageIntoNeuronLayer("face_recog_neuron_layer", resizedImageFace);
	int right_eye_x = eyes[1].x;
	int right_eye_y = resizedImageFace->height - 1 - eyes[1].y;
	int left_eye_x = eyes[0].x;
	int left_eye_y = resizedImageFace->height - 1 - eyes[0].y;

	// Set Eyes Position
	char* eyesPositionCommand = (char*)malloc(256*sizeof(char));
	sprintf(eyesPositionCommand, "SetEyesPositions(%d,%d,%d,%d);", right_eye_x, right_eye_y, left_eye_x, left_eye_y);
	MaeExecuteCommandScript(eyesPositionCommand);
	free(eyesPositionCommand);

	MaeExecuteCommandScript("forward filters;");
	MaeExecuteCommandScript("forward neural_layers;");

	int result = MajorityVote("nl_landmark", percentage);

	MaeQuit();

	return result;
}

vector<Rect> detectFaces(Mat &img, CascadeClassifier& cascade)
{
	int height = cvRound(img.rows);
	int width = cvRound(img.cols);
	Mat gray, imgCpy(height, width, CV_8UC1);

    cvtColor(img, gray, CV_BGR2GRAY); // convert image to grayscale
    resize(gray, imgCpy, imgCpy.size(), 0, 0, INTER_LINEAR);
    equalizeHist(imgCpy, imgCpy);

	// detect faces
	vector<Rect> faces;
	cascade.detectMultiScale(img, faces,
		1.1, 1, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		|CV_HAAR_SCALE_IMAGE
		,
		Size(30, 30) );

	return faces;
}

CvPoint* detectEyes(Rect face)
{
	CvPoint* eyes = (CvPoint*)malloc(2*sizeof(CvPoint));

	double dW = (double)face.width / 3.5;
	double dH = (double)face.height / 2.6;
	eyes[0].x = cvRound(face.x + dW);
	eyes[0].y = cvRound(face.y + dH);
	eyes[1].x = cvRound(face.x + face.width - dW);
	eyes[1].y = cvRound(face.y + dH);

	return eyes;
}

bool PutImageIntoNeuronLayer(char* neuron_layer_name, IplImage* image)
{
	IplImage* out_image;

	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);

	if ((w != image->width) || (h != image->height))
	{
		out_image = cvCreateImage(cvSize(w, h), image->depth, image->nChannels);
		cvResize(image, out_image);
	}
	else
	{
		out_image = cvCloneImage(image);
	}

	int* neural_layer_output = (int*)malloc(w * h * sizeof(int));

	for (int y = 0; y < h; y++)
	{
		int yo = h - 1 - y;
		for (int x = 0; x < w; x++)
		{
			CvScalar pixel = cvGet2D(out_image, y, x);
			int blue = (int)pixel.val[0];
			int green = (int)pixel.val[1];
			int red = (int)pixel.val[2];

			int color = red;
			color |= (green & 0xff) << 8;
			color |= (blue & 0xff) << 16;

			neural_layer_output[yo * w + x] = color;
		}
	}

	if (PutNeuronLayerOutputInt(neuron_layer_name, neural_layer_output) == 1)
		return true;
	else
		return false;
}

int MajorityVote(char* neuron_layer_name, double* percentage)
{
	int nClassAux, nClassID = 0;
	int nAux, nMax = 0;
	int w = GetNeuronLayerWidth(neuron_layer_name);
	int h = GetNeuronLayerHight(neuron_layer_name);


	int* neural_layer_output = (int*)malloc(w * h * sizeof(int));
	neural_layer_output = GetNeuronLayerOutputInt(neuron_layer_name);

	if (neural_layer_output == NULL)
	{
		*percentage = 0.0;
		return 0;
	}

	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de classes. A classe que tiver o maior numero de neuronios
	// setados sera considerada como o valor da camada de saida.
	for (int i = 0; i < (h * w); i++)
	{

		nClassAux = neural_layer_output[i];
		nAux = 1;

		for (int j = i + 1; j < (h * w); j++)
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

	*percentage = (float) nMax / (float) (h * w);
	return nClassID;
}

IplImage* ReadRawImage(char *fileName, int imageWidth, int imageHeight)
{
	int imageSize = imageWidth * imageHeight;
	int rawImage[imageSize * 3];

	// Read RAW Image
	FILE *rawImageFile = fopen (fileName, "rb");
	if (rawImageFile == NULL)
	{
		printf ("Error: cannot open file '%s' (ReadRawImage).\n", fileName);
		return NULL;
	}

	for (int i = 0; i < (imageSize * 3); i++)
	{
		unsigned char ch;
		fread (&ch, 1, 1, rawImageFile);
		rawImage[i] = (unsigned int) ch;
	}

	fclose(rawImageFile);

	// Convert RAW image to BGRImage (OpenCV's IplImage format)
	IplImage* bgrImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	int i = 0;
	for (int y = 0; y < imageHeight; y++)
	{
		for (int x = 0; x < imageWidth; x++)
		{
			bgrImage->imageData[3 * (x + y * imageWidth) + 0] = rawImage[i + imageSize * 2];
			bgrImage->imageData[3 * (x + y * imageWidth) + 1] = rawImage[i + imageSize];
			bgrImage->imageData[3 * (x + y * imageWidth) + 2] = rawImage[i];
			i++;
		}
	}

	//cv::imshow( "convert", bgrImage );
	//cvWaitKey(-1);

	return bgrImage;
}

IplImage* ReadImage(char *imageName)
{
	String imageNameStr = imageName;
	String fileExtension = imageNameStr.substr(imageNameStr.size() - 3, 3);

	//Load image based on file extension
	IplImage* image;
	if (fileExtension.compare("raw") == 0)
		image = ReadRawImage(imageName, 768, 576);
	else
		image = cvLoadImage(imageName, CV_LOAD_IMAGE_COLOR);

	IplImage* resizedImage = cvCreateImage(cvSize(640, 480), image->depth, image->nChannels);
	cvResize(image, resizedImage, CV_INTER_LINEAR);

	return resizedImage;
}

void TrainNetwork(int imageWidth, int imageHeight)
{
	MaeInitialize();

	char imageListFile[256];
	sprintf(imageListFile, "%s/MAE/examples/nc/visual_memory/imageList.txt", basePath);
	FILE* f = fopen(imageListFile, "r");

	int c_image = 1;
	while (!feof(f))
	{
		char imageName[256];
		fscanf(f, "%[^\n]\n", imageName);
		IplImage* image = ReadImage(imageName);

		//Resize image to the face_recog network size
		IplImage* resizedImageFace = cvCreateImage(cvSize(384, 288), IPL_DEPTH_8U, 3);
		Mat resizedFaceMat;
		Mat imgMat = image;

		vector<Rect> faces = detectFaces(imgMat, cascade);
		if (faces.size() < 1)
		{
			printf ("Error: cannot train with '%s'. Can't detect a face (TrainNetwork).\n", imageName);
			return;
		}

		cvSetImageROI(image, faces.at(0));
		cvResize(image, resizedImageFace, CV_INTER_CUBIC);
		cvResetImageROI(image);

		resizedFaceMat = resizedImageFace;

		Rect face_rect = cvRect(0, 0, 384, 288);
		cvSetImageROI(resizedImageFace, face_rect);
		CvPoint* eyes = detectEyes(face_rect);

		int right_eye_x = eyes[1].x;
		int right_eye_y = resizedImageFace->height - 1 - eyes[1].y;
		int left_eye_x = eyes[0].x;
		int left_eye_y = resizedImageFace->height - 1 - eyes[0].y;

		PutImageIntoNeuronLayer("face_recog_neuron_layer", resizedImageFace);
		char eyesPositionCommand[256];
		sprintf(eyesPositionCommand, "SetEyesPositions(%d,%d,%d,%d);", right_eye_x, right_eye_y, left_eye_x, left_eye_y);
		MaeExecuteCommandScript(eyesPositionCommand);

		MaeExecuteCommandScript("forward filters;");

		int w_nl_landmark = GetNeuronLayerWidth("nl_landmark");
		int h_nl_landmark = GetNeuronLayerHight("nl_landmark");
		int neuralLayerOutput[w_nl_landmark * w_nl_landmark];
		for (int i = 0; i < w_nl_landmark * h_nl_landmark; i++)
			neuralLayerOutput[i] = c_image;
		PutNeuronLayerOutputInt("nl_landmark", neuralLayerOutput);

		MaeExecuteCommandScript("train network;");

		c_image++;

		cvReleaseImage(&image);
		cvReleaseImage(&resizedImageFace);
	}

	fclose(f);

	// Load training data
	char trainingCommand[256];
	sprintf(trainingCommand, "unload to \"%s/MAE/examples/nc/visual_memory/visual_memory.mem\";", basePath);
	MaeExecuteCommandScript(trainingCommand);
}
