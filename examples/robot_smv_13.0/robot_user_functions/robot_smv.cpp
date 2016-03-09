/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#include "robot_smv.hpp"

#include <limits.h>

#include "gabor_guess_cylinder.h"
#include "robot_neuron_network_c.h"
#include "stereo_api.h"

#include <stdexcept>
using std::runtime_error;

#include "config_default.hpp"

#include "file.hpp"
using mae::io::File;

#include "garbage_collector.hpp"
using mae::manage;

#include "image_ppm.hpp"
using mae::io::ImagePPM;

#include "logging.hpp"
using mae::logging::log;
using mae::logging::trace;

#include "model_3d_io.hpp"

#include "primitives.hpp"
using mae::util::Primitives;

/*
Constructor Section
*/

RobotSMV::RobotSMV(): network(RobotNeuronNetwork::getInstance()), workingAreas(2)
{
	trace("RobotSMV::RobotSMV");
	manage<RobotSMV>(this);

	configWorkingArea();
	currentHalf = 0;
	currentFace = -1;
	halves = 1;

	int width  = network.getInputWidth();
	int height = network.getInputHeight();
	trace("StereoInitialize", string() + width + ", " + height);
	if (StereoInitialize(width, height))
		throw runtime_error("Stereo system module initialization failure (MaeInitialize).");
}

RobotSMV::~RobotSMV()
{
	trace("RobotSMV::~RobotSMV");
	
	StereoTerminate();
}

RobotSMV& RobotSMV::getInstance()
{
	trace("RobotSMV::getInstance");

	static RobotSMV* robot = new RobotSMV();

	return *robot;
}

/*
Method Section
*/

int RobotSMV::inputSize()
{
	RobotNeuronNetwork& network = RobotNeuronNetwork::getInstance();

	return network.getInputWidth() * network.getInputHeight();
}

ComposedModel3D& RobotSMV::computeModel3D()
{
	trace("RobotSMV::computeModel3D");

	pileModel.clear();
	for (int half = 0; half < halves; half++)
{
		currentHalf = half;
	int offset = computeHalvesOffset();
		Model3D& model3dBack = pileModel.getModel3D(currentHalf);
		computeModel3D(getWorkingArea(currentHalf), offset, model3dBack);
	}

	return pileModel;
}

int RobotSMV::computeHalvesOffset()
{
	trace("RobotSMV::computeHalvesOffset");

	if (currentHalf >= (halves - 1))
		return INT_MAX;

	ConfigDefault::load();

	configNeuronLayers(0, 0);
	network.selectMatchRegion("match_back");

	configNeuronLayers(1, 0);
	network.selectMatchRegion("match_front");

	int disparity = (int) robot_neuron_network_merge_offset();
	int offset = ConfigDefault::intValue("matching.match_back.offset.x") + disparity;

	trace("RobotSMV", "", string() + "halvesOffset(" + offset + ")");

	return offset;
}

void RobotSMV::computeModel3D(WorkingArea& workingArea, int section, Model3D& model3d)
{
	trace("RobotSMV::computeModel3D", string() + workingArea + ", " + section + ", model3d");

	for (int face = 0; face < NUM_CAMERA_PAIRS; face++)
	{
		currentFace = face;
		computeModel3DFace(workingArea, section, model3d);
	}
}

Model3D& RobotSMV::computeModel3DFace(WorkingArea& workingArea, int section, Model3D& model3d)
{
	trace("RobotSMV::computeModel3DFace",
		string() + workingArea + ", " + section + ", model3d",
		string() + "currentFace(" + currentFace + ")");

	if (!(0 <= currentFace && currentFace < NUM_CAMERA_PAIRS))
		throw runtime_error("No face selected");

	extractPointCloud(workingArea, model3d);

	CYLINDER_LIST* cylinders = fit_cylinder_list(&(workingArea[0]), section);
	model3d.setCylinders(currentFace, cylinders);

	return model3d;
}

Model3D& RobotSMV::computeModel3DFace()
{
	trace(string() + "RobotSMV::computeModel3DFace");

	Model3D& model3d = pileModel.getModel3D(currentHalf);
	return computeModel3DFace(getWorkingArea(currentHalf), INT_MAX, model3d);

}

void RobotSMV::extractPointCloud(WorkingArea& workingArea, Model3D& model3d)
{
	trace("RobotSMV::extractPointCloud", string() + workingArea + ", model3d", string() + currentFace);

	configNeuronLayers(currentHalf, currentFace);

/*	PointCloud cloud;
	extractPointCloud(workingArea, cloud);
	model3d.setPointCloud(currentFace, cloud); */
}

void RobotSMV::extractPointCloud()
{
	trace("RobotSMV::extractPointCloud");

	Model3D& model3d = pileModel.getModel3D(currentHalf);
	extractPointCloud(getWorkingArea(currentHalf), model3d);
}

void RobotSMV::extractPointCloud(WorkingArea& workingArea, PointCloud& cloud)
{
	trace("RobotSMV::extractPointCloud", string() + workingArea + ", cloud");

	int width  = workingArea[2] - workingArea[0];
	int height = workingArea[3] - workingArea[1];
	cloud.resize(width, height);

	vector<double>& leftPoints  = cloud.getLeftPoints();
	vector<double>& rightPoints = cloud.getRightPoints();
	vector<int>&    colors      = cloud.getPointColors();

	network.copyStereoPoints(workingArea, leftPoints, rightPoints, colors);
}

void RobotSMV::execute(string path)
{
	trace("RobotSMV::execute", "\"" + path + "\"");

	ConfigDefault::load(path);
	ConfigDefault::save();
	halves = ConfigDefault::intValue("input.halves", 1);
	configWorkingArea();

	computeModel3D();

	Configuration results("response.txt");
	results.setSetting("model3D.volume.stereo", pileModel.getStereoVolume());
	results.setSetting("model3D.volume.solid", pileModel.getSolidVolume());
	results.setSetting("model3D.length", pileModel.getLength());
	results.setSetting("model3D.width", pileModel.getWidth());
	results.setSetting("model3D.height", pileModel.getHeight());
}

void RobotSMV::extractStereoRegion(string path)
{
	trace("RobotSMV::extractStereoRegion", "\"" + path + "\"");
	if (File::exists(path)) try
	{
		currentFace = -1;
		loadRectificationParameters(0);

		network.load("image_right", path);
		network.rectifyInputRight("image_right");
		network.save("image_right", "image_rectified.ppm");

		int x0 = ConfigDefault::intValue("image.stereo.limits.x0", 0);
		int y0 = ConfigDefault::intValue("image.stereo.limits.y0", 0);
		int xn = ConfigDefault::intValue("image.stereo.limits.xn", network.getInputWidth());
		int yn = ConfigDefault::intValue("image.stereo.limits.yn", network.getInputHeight());
		ImagePPM::cut("image_rectified.ppm", "image_stereo_region.ppm", x0, y0, xn, yn);
	}
	catch (...)
	{
		throw RobotSMVException(-1, "Rectification failed");
	}
}

void RobotSMV::rectifyImageLeft(string name)
{
	network.rectifyInputLeft(name);
}

void RobotSMV::rectifyImageRight(string name)
{
	network.rectifyInputRight(name);
}

bool RobotSMV::toggleTwoHalves()
{
	halves = (halves < 2 ? 2 : 1);
	return (halves == 2);
}

string RobotSMV::toString()
{
	return string() + "mae::smv::RobotSMV [" + ((int) this) + "]";
}

/*
Input/Output Section
*/

void RobotSMV::configNeuronLayers(int half, int side)
{
	trace("RobotSMV::configNeuronLayers", string() + half + ", " + side);

	string basename = ConfigDefault::getSetting("image.basename");
	string format = ConfigDefault::getSetting("image.format");
	string mode = ConfigDefault::getSetting("smv.mode");

	loadRectificationParameters(side);
	network.load(basename.c_str(), half, side, format.c_str());
}

void RobotSMV::configWorkingArea()
{
	trace("RobotSMV::configWorkingArea");

	ConfigDefault::load();

	for (int i = 0, n = workingAreas.size(); i < n; i++)
	{
		WorkingArea& workingArea = getWorkingArea(i);
		string basename = string() + "input.half" + i;

		workingArea[0] = ConfigDefault::intValue(basename + ".workingArea.x0", 0);
		workingArea[1] = ConfigDefault::intValue(basename + ".workingArea.y0", 0);
		workingArea[2] = ConfigDefault::intValue(basename + ".workingArea.xn", network.getInputWidth());
		workingArea[3] = ConfigDefault::intValue(basename + ".workingArea.yn", network.getInputHeight());
	}
}

void RobotSMV::configWorkingArea(int x0, int y0, int xn, int yn)
{
	trace("RobotSMV::configWorkingArea",
		string() + x0 + ", " + y0 + ", " + xn + ", " + yn,
		string() + "currentHalf(" + currentHalf + "), currentFace(" + currentFace + ")");

	string basename = string() + "input.half" + currentHalf;
	WorkingArea& workingArea = getWorkingArea(currentHalf);
	workingArea[0] = x0;
	workingArea[1] = y0;
	workingArea[2] = xn;
	workingArea[3] = yn;

	ConfigDefault::setSetting(basename + ".workingArea.x0", x0);
	ConfigDefault::setSetting(basename + ".workingArea.y0", y0);
	ConfigDefault::setSetting(basename + ".workingArea.xn", xn);
	ConfigDefault::setSetting(basename + ".workingArea.yn", yn);

	ConfigDefault::save();
}

void RobotSMV::loadImages(int half, int side)
{
	trace("RobotSMV::loadImages", string() + half + ", " + side);

	if (!(0 <= half && half < halves))
		throw runtime_error(string() + "Half [" + half + "] is not valid");

	if (!(0 <= side && side < NUM_CAMERA_PAIRS))
		throw runtime_error(string() + "Side [" + side + "] is not valid");

	ConfigDefault::load();

	loadRectificationParameters(side);

	string basename = ConfigDefault::getSetting("image.basename");
	string format = ConfigDefault::getSetting("image.format");
	string mode = ConfigDefault::getSetting("smv.mode");

	network.loadInputs(basename.c_str(), half, side, format.c_str());
	network.rectifyInputLeft("image_left");
	network.rectifyInputRight("image_right");
}

void RobotSMV::loadRectificationParameters(int side)
{
	trace("RobotSMV::loadRectificationParameters", string() + side);

	string pathIndexes = string() + "camera_pair_" + side + "_rectification_indexes.mat";
	string pathStereo  = string() + "camera_pair_" + side + "_rectification_stereo.mat";

	if (File::exists(pathIndexes) && File::exists(pathStereo))
	{
		trace("LoadStereoParameters(\"" + pathStereo + "\")");
		LoadStereoParameters(pathStereo.c_str());

		trace("LoadStereoRectificationIndexes", "\"" + pathIndexes + "\"");
		LoadStereoRectificationIndexes(pathIndexes.c_str());
	}
	else
		throw runtime_error("Rectification file set (\"" + pathIndexes + "\", \"" + pathStereo + "\") not found");
}

/*
Property Section
*/

int RobotSMV::getCurrentHalf()
{
	return currentHalf;
}

void RobotSMV::setCurrentHalf(int half)
{
	trace("RobotSMV::setCurrentHalf", string() + half);

	if (half != currentHalf)
	{
		loadImages(half, currentFace);
		currentHalf = half;
	}
}

int RobotSMV::getCurrentFace()
{
	return currentFace;
}

void RobotSMV::setCurrentSide(int side)
{
	trace("RobotSMV::setCurrentSide", string() + side);
//	if (side != currentFace)
//	{
		loadImages(currentHalf, side);
		currentFace = side;
//	}
}

ComposedModel3D& RobotSMV::getModel3D()
{
	return pileModel;
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
	trace("RobotSMV::getWorkingArea", string() + half);

	return workingAreas.at(half);
}

void RobotSMV::setWorkingArea(int half, int x0, int y0, int xn, int yn)
{
	WorkingArea& workingArea = getWorkingArea(half);

	workingArea[0] = x0;
	workingArea[1] = y0;
	workingArea[2] = xn;
	workingArea[3] = yn;
}
