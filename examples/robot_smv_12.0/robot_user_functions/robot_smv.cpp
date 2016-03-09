/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#include "robot_smv.hpp"

#include <stdexcept>

#include "config_default.hpp"
#include "image_ppm.hpp"
#include "gabor_guess_cylinder.h"
#include "model_3d_io.hpp"
#include "primitives.hpp"
#include "robot_con.h"
#include "stereo_api.h"

using std::runtime_error;

using mae::io::ImagePPM;
using mae::util::Primitives;

using mae::smv::RobotNeuronNetwork;

/*
Attribute Section
*/

int RobotSMV::HALF_FRONT = 0;

int RobotSMV::HALF_BACK  = 1;

int RobotSMV::SIDE_LEFT  = 0;

int RobotSMV::SIDE_TOP   = 1;

int RobotSMV::SIDE_RIGHT = 2;

/*
Constructor Section
*/

RobotSMV::RobotSMV(): network(RobotNeuronNetwork::getInstance())
{
	currentSide = -1;
	halves = 1;

	int width  = network.getInputWidth();
	int height = network.getInputHeight();
	if (StereoInitialize(width, height))
		throw runtime_error("Stereo system module initialization failure (MaeInitialize).");

	configWorkingArea();
	model_3d_init(&model3d);
	model_3d_load_config(&model3d);
}

RobotSMV::~RobotSMV()
{
	StereoTerminate();
}

RobotSMV& RobotSMV::getInstance()
{
	static RobotSMV* robot = new RobotSMV();

	return *robot;
}

/*
Method Section
*/

void ImagePair::setImageBuffer(ImageBuffer &imageBuffer, int *buffer)
{
	imageBuffer.clear();
	for (int i = 0, n = RobotSMV::inputSize(); i < n; i++)
		imageBuffer[i] = buffer[i];
}

void ImagePair::setLeft(int* buffer)
{
	setImageBuffer(left, buffer);
}

void ImagePair::setRight(int* buffer)
{
	setImageBuffer(right, buffer);
}

int RobotSMV::inputSize()
{
	RobotNeuronNetwork& network = RobotNeuronNetwork::getInstance();

	return network.getInputWidth() * network.getInputHeight();
}

MODEL_3D& RobotSMV::computeModel3D()
{
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
		computeModel3DFace(i);

//	model_3d_view(&model3d);
	model_3d_stereo_volume(&model3d);

	return model3d;
}

MODEL_3D& RobotSMV::computeModel3DFace(int side)
{
	setCurrentSide(side);
	
	if (halves <= 1)
		return computeModel3DFace3M(side);
	else
		return computeModel3DFace6M(side);
}

MODEL_3D& RobotSMV::computeModel3DFace()
{
	if (0 <= currentSide && currentSide < NUM_CAMERA_PAIRS)
		return computeModel3DFace(currentSide);
	else
		throw runtime_error("No side selected");
}

MODEL_3D& RobotSMV::computeModel3DFace3M(int side)
{
	int workingArea[4];
	workingArea[0] = getWorkingArea(0).x0();
	workingArea[1] = getWorkingArea(0).y0();
	workingArea[2] = getWorkingArea(0).xn();
	workingArea[3] = getWorkingArea(0).yn();

	int points = (workingArea[2] - workingArea[0]) * (workingArea[3] - workingArea[1]);
	vector<double> leftPoints(points*2);
	vector<double> rightPoints(points*2);
	vector<int> colors(points);

	configNeuronLayers(0, side);
	network.copyStereoPoints(getWorkingArea(0), leftPoints, rightPoints, colors);
	model_3d_fill_face(&model3d, side, points, &(leftPoints[0]), &(rightPoints[0]), &(colors[0]));
	CYLINDER_LIST* cylinders = fit_cylinder_list(workingArea, 1);
	model_3d_set_cylinders(&model3d, side, cylinders);

	return model3d;
}

MODEL_3D& RobotSMV::computeModel3DFace6M(int side)
{
/*	int disparity = computeHalvesDisparity(side);

	int points = cloud.getSize();
	vector<double>& leftPoints  = cloud.getLeftPoints();
	vector<double>& rightPoints = cloud.getRightPoints();
	vector<int>&    colors      = cloud.getPointColors();
	
	int workingArea[4];
	workingArea[0] = getWorkingArea(0).x0();
	workingArea[1] = getWorkingArea(0).y0();
	workingArea[2] = getWorkingArea(1).xn() - disparity;
	workingArea[3] = getWorkingArea(1).yn();

	model_3d_fill_face(&model3d, side, points, &(leftPoints[0]), &(rightPoints[0]), &(colors[0]));
	CYLINDER_LIST* cylinders = fit_cylinder_list(workingArea);
	model_3d_set_cylinders(&model3d, side, cylinders); */

	return model3d;
}
/*
int RobotSMV::computeHalvesDisparity(int side)
{
	ConfigDefault::load();

	configNeuronLayers(0, side);
	network.selectMatchRegion("match_back");
	network.fillMergeMap(0, 0);

	configNeuronLayers(1, side);
	network.selectMatchRegion("match_front");
	int x0      = ConfigDefault::intValue("matching.match_front.offset.x");
	int disparity = (int) robot_con_merge_offset();
	int offset = ConfigDefault::intValue("matching.match_back.offset.x") + disparity;
	network.fillMergeMap(x0, offset);

	return disparity;
}

void RobotSMV::extractPointCloud(PointCloud& cloud, int half, int w0, int offset)
{
	WorkingArea& workingArea = getWorkingArea(half);
	int points = (workingArea[2] - workingArea[0]) * (workingArea[3] - workingArea[1]);

	vector<double>& leftPoints  = cloud.getLeftPoints();
	vector<double>& rightPoints = cloud.getRightPoints();
	vector<int>&    colors      = cloud.getPointColors();
	
	leftPoints.resize(2*points);
	rightPoints.resize(2*points);
	colors.resize(points);

	network.copyStereoPoints(workingArea, leftPoints, rightPoints, colors);
} */

void RobotSMV::rectifyImageLeft(string name)
{
	network.rectifyInputLeft(name);
}

void RobotSMV::rectifyImageRight(string name)
{
	network.rectifyInputRight(name);
}

void RobotSMV::rectifyImageRight(int* image, vector<int>& rectified)
{
	int width  = network.getInputWidth();
	int height = network.getInputWidth();
	
	ImagePPM::save("image_right.ppm", width, height, image);
	network.load("image_right", "image_right.ppm"); 
	network.rectifyInputRight("image_right");

	network.save("image_right", "image_right_rectified.ppm"); 
	ImagePPM::load("image_right_rectified.ppm", rectified);
}

void RobotSMV::stereoRegionRight(int* image, int* rectified)
{
	setCurrentSide(0);

	int x0 = ConfigDefault::intValue("image.stereo.limits.x0", 0);
	int y0 = ConfigDefault::intValue("image.stereo.limits.y0", 0);
	int xn = ConfigDefault::intValue("image.stereo.limits.xn", network.getInputWidth());
	int yn = ConfigDefault::intValue("image.stereo.limits.yn", network.getInputHeight());
	int width = 1 + xn - x0;

	vector<int> buffer;
	rectifyImageRight(image, buffer);

	for (int i = x0, k = 0; i < xn; i++)
		for (int j = y0; j < yn; j++, k++)
			rectified[k] = buffer[i + j*width];
}

/*
Input/Output Section
*/

void RobotSMV::configNeuronLayers(int half, int side)
{
	string basename = ConfigDefault::getSetting("image.basename");
	string format = ConfigDefault::getSetting("image.format");
	string mode = ConfigDefault::getSetting("smv.mode");

	if (mode == "maintenance")
		network.load(basename.c_str(), half, side, format.c_str());
	else
	{
		ImageBuffer left  = imagePairs[0][side].left;
		ImageBuffer right = imagePairs[0][side].right;
		robot_con_fill_network(basename.c_str(), side, half, format.c_str(), &(left[0]), &(right[0]));
	}
}

void RobotSMV::configWorkingArea()
{
	ConfigDefault::load();

	WorkingArea& workingArea = getWorkingArea(0);
	workingArea[0] = ConfigDefault::intValue("workspace.x0", 0);
	workingArea[1] = ConfigDefault::intValue("workspace.y0", 0);
	workingArea[2] = ConfigDefault::intValue("workspace.xn", network.getInputWidth());
	workingArea[3] = ConfigDefault::intValue("workspace.yn", network.getInputHeight());
}

void RobotSMV::configWorkingArea(int half, int x0, int y0, int xn, int yn)
{
	WorkingArea& workingArea = getWorkingArea(half);
	workingArea[0] = x0;
	workingArea[1] = y0;
	workingArea[2] = xn;
	workingArea[3] = yn;

	ConfigDefault::setSetting("workspace.x0", x0);
	ConfigDefault::setSetting("workspace.y0", y0);
	ConfigDefault::setSetting("workspace.xn", xn);
	ConfigDefault::setSetting("workspace.yn", yn);

	ConfigDefault::save();
}

void RobotSMV::loadRectificationParameters(int side)
{
	string pathIndexes = "camera_pair_" + Primitives::toString(side) + "_rectification_indexes.mat";
	string pathStereo  = "camera_pair_" + Primitives::toString(side) + "_rectification_stereo.mat";
	
	if (File::exists(pathIndexes) && File::exists(pathStereo))
	{
		LoadStereoParameters(pathStereo.c_str());
		LoadStereoRectificationIndexes(pathIndexes.c_str());
	}
	else
		throw runtime_error("Rectification file set (" pathIndexes + ", " + pathStereo + ") not found");
}

/*
Property Section
*/

int RobotSMV::getCurrentSide()
{
	return currentSide;
}

void RobotSMV::setCurrentSide(int side)
{
	if (side == currentSide)
		return;
	
	if (0 <= side && side < NUM_CAMERA_PAIRS)
	{
		ConfigDefault::load();

		loadRectificationParameters(side);
	
		string basename = ConfigDefault::getSetting("image.basename");
		string format = ConfigDefault::getSetting("image.format");
		string mode = ConfigDefault::getSetting("smv.mode");

		network.loadInputs(basename.c_str(), 0, side, format.c_str());
		network.rectifyInputLeft("image_left");
		network.rectifyInputRight("image_right");
	}
	else
		throw runtime_error("Side " + Primitives::toString(side) + " is not valid");

	currentSide = side;
}

MODEL_3D& RobotSMV::getModel3D()
{
	return model3d;
}

void RobotSMV::setImagePair(int half, int side, int* left, int* right)
{
	ImagePair& pair = imagePairs[half][side];
	pair.setLeft(left);
	pair.setRight(right);
}

bool RobotSMV::isTwoHalves()
{
	return (halves > 1);
}
	
void RobotSMV::setTwoHalves(bool twoHalves)
{
	halves = (twoHalves ? 2 : 1);
}

WorkingArea& RobotSMV::getWorkingArea(int half)
{
	return workingAreas[half];
}

void RobotSMV::setWorkingArea(int half, int x0, int y0, int xn, int yn)
{
	WorkingArea& workingArea = getWorkingArea(half);
	
	workingArea[0] = x0;
	workingArea[1] = y0;
	workingArea[2] = xn;
	workingArea[3] = yn;
}
