/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#include "robot_neuron_network.hpp"

#include <stdexcept>

#include "config_default.hpp"
#include "file.hpp"
#include "primitives.hpp"
#include "robot_con.h"
#include "stereo_api.h"

using std::runtime_error;

using mae::smv::RobotNeuronNetwork;
using mae::io::File;
using mae::util::Primitives;

/*
Definition Section
*/

#define ZOOM(value, fromScale, toScale) (int) ((float) (value * toScale) / (float) fromScale + 0.5)

/*
Constructor Section
*/

RobotNeuronNetwork::RobotNeuronNetwork(): network(NeuronNetwork::getInstance())
{
}

RobotNeuronNetwork& RobotNeuronNetwork::getInstance()
{
	static RobotNeuronNetwork* singleton = new RobotNeuronNetwork();

	return *singleton;
}

/*
Method Section
*/

void RobotNeuronNetwork::copyStereoPoints(
	WorkingArea& workingArea, vector<double>& left, vector<double>& right, vector<int>& colors)
{
	// Dimensoes do cortex (disparity map)
	NeuronLayer& disparityMap = network.getNeuronLayer("nl_disparity_map");
	int ho = disparityMap.getHeight();
	int wo = disparityMap.getWidth();

	// Dimensoes da imagem de entrada
	InputLayer& imageRight = network.getInputLayer("image_right");
	int hi = imageRight.getHeight();
	int wi = imageRight.getWidth();

	for (int xi = workingArea[0], num_points = 0; xi < workingArea[2]; xi++)
	{
		int xo = ZOOM(xi, wi, wo);
		for (int yi = workingArea[1]; yi < workingArea[3]; yi++)
		{
			int yo = ZOOM(yi, hi, ho);
			if ((0 <= xo && xo < wo) && (0 <= yo && yo < ho))
			{
				float disparity = disparityMap.floatOutput(wo * yo + xo);

				left [num_points + 0] = (double) xi + disparity;
				left [num_points + 1] = (double) yi;
				right[num_points + 0] = (double) xi;
				right[num_points + 1] = (double) yi;

				colors[num_points / 2] = imageRight.intOutput(yi * wi + xi);

				num_points += 2;
			}
		}
	}
}

void RobotNeuronNetwork::fillMergeMap(int w0, int offset)
{
	NeuronLayer& disparityMap = network.getNeuronLayer("nl_disparity_map");
	NeuronLayer& mergeMap = network.getNeuronLayer("match_merge_map");
	InputLayer& image = network.getInputLayer("image_right");

	int disparity_width = disparityMap.getWidth();
	int height = disparityMap.getHeight();
	int merge_width = mergeMap.getWidth();
	int image_width = image.getWidth();

	int neuron_w0 = ZOOM(w0, image_width, disparity_width);
	int neuron_offset = ZOOM(offset, image_width, disparity_width);

	int i = 0;
	for (i = 0; i < height; i++)
	{
		int j = 0, n = disparity_width - neuron_w0;
		for (j = 0; j < n; j++)
		{
			int merge_index = i*merge_width + (j + neuron_offset);
			int disparity_index = i*disparity_width + (j + neuron_w0);
			mergeMap.setOutput(merge_index, disparityMap.floatOutput(disparity_index));
		}

		for (j = neuron_offset + n; j < merge_width; j++)
			mergeMap.setOutput(i*merge_width + j, 0.0f);
	}

	network.updateAllOutputs();
}

void RobotNeuronNetwork::rectifyInput(RectifyFunction rectify, InputLayer& input)
{
	TextureFrameBuffer& frameBuffer = input.getTextureFrameBuffer();
	int frameWidth = frameBuffer.getWidth();
	int width  = input.getWidth();
	int height = input.getHeight();
	int size = width * height;
	int bytes = 3 * size;
	vector<GLubyte> original(bytes);
	vector<GLubyte> rectified(bytes);

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			int imageOffset = (height - j - 1) + i * height;
			int frameOffset = 3 * (i + j * frameWidth);
			original[imageOffset + 0 * size] = frameBuffer[frameOffset + 0];
			original[imageOffset + 1 * size] = frameBuffer[frameOffset + 1];
			original[imageOffset + 2 * size] = frameBuffer[frameOffset + 2];
		}
	}

	rectify(&(rectified[0]), &(original[0]), width, height);

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			int imageOffset = (height - j - 1) + i * height;
			int frameOffset = 3 * (i + j * frameWidth);
			frameBuffer[frameOffset + 0] = rectified[imageOffset + 0 * size];
			frameBuffer[frameOffset + 1] = rectified[imageOffset + 1 * size];
			frameBuffer[frameOffset + 2] = rectified[imageOffset + 2 * size];
		}
	}
	
	input.update();
	network.updateAllOutputs();
}

void RobotNeuronNetwork::rectifyInputLeft(string name)
{
	InputLayer& input = network.getInputLayer(name);

	rectifyInput(&RectifyLeftImage, input);
}

void RobotNeuronNetwork::rectifyInputRight(string name)
{
	InputLayer& input = network.getInputLayer(name);

	rectifyInput(&RectifyRightImage, input);
}

void RobotNeuronNetwork::selectMatchRegion(string name)
{
	string xOffsetName = "matching." + name + ".offset.x";
	string yOffsetName = "matching." + name + ".offset.y";

	ConfigDefault::load();
	int xOffset = ConfigDefault::intValue(xOffsetName);
	int yOffset = ConfigDefault::intValue(yOffsetName);

	InputLayer& regionLayer = network.getInputLayer(name);
	TextureFrameBuffer& region = regionLayer.getTextureFrameBuffer();
	int regionWidth  = region.getWidth();

	InputLayer& imageLayer = network.getInputLayer("image_right");
	TextureFrameBuffer& image = imageLayer.getTextureFrameBuffer();
	int imageWidth = image.getWidth();

	int i = 0, j = 0, m = 0, n = 0;
	for (i = 0, m = regionLayer.getVisibleHeight(); i < m; i++)
	{
		for (j = 0, n = regionLayer.getVisibleWidth(); j < n; j++)
		{
			int regionOffset = 3*(i*regionWidth + j);
			int imageOffset  = 3*((i + yOffset)*imageWidth + (j + xOffset));

			region[regionOffset + 0] = image[imageOffset + 0];
			region[regionOffset + 1] = image[imageOffset + 1];
			region[regionOffset + 2] = image[imageOffset + 2];
		}
	}

	regionLayer.update();
	network.updateAllOutputs();
}

/*
Input / Output Section
*/

bool RobotNeuronNetwork::loadCache(const string basename, int half, int side, const string format)
{
	string setname = basename + "_half" + Primitives::toString(half) + "_side" + Primitives::toString(side);
	string pathLeftRectified  = setname + "_rtf_left."      + format;
	string pathRightRectified = setname + "_rtf_right."     + format;
	string pathDisparityMap   = setname + "_disparity_map." + format;
	string pathRotateMap      = setname + "_rotate_map."    + format;
	string pathSegmentMap     = setname + "_segment_map."   + format;

	bool cached = (File::exists(pathLeftRectified)
	            && File::exists(pathRightRectified)
		    && File::exists(pathDisparityMap)
		    && File::exists(pathRotateMap)
		    && File::exists(pathSegmentMap));

	if (cached)
	{
		network.load("image_left",                pathLeftRectified);
		network.load("image_right",               pathRightRectified);
		network.load("nl_disparity_map",          pathDisparityMap);
		network.load("nl_trunk_segmentation_map_vertical_rotate", pathRotateMap);
		network.load("nl_trunk_segmentation_map", pathSegmentMap);

		network.updateAllOutputs();
	}

	return cached;
}

void RobotNeuronNetwork::saveCache(const string basename, int half, int side, const string format)
{
	string setname = basename + "_half" + Primitives::toString(half) + "_side" + Primitives::toString(side);
	string pathLeftRectified  = setname + "_rtf_left."      + format;
	string pathRightRectified = setname + "_rtf_right."     + format;
	string pathDisparityMap   = setname + "_disparity_map." + format;
	string pathRotateMap      = setname + "_rotate_map."    + format;
	string pathSegmentMap     = setname + "_segment_map."   + format;

	network.save("image_left",                pathLeftRectified);
	network.save("image_right",               pathRightRectified);
	network.save("nl_disparity_map",          pathDisparityMap);
	network.save("nl_trunk_segmentation_map_vertical_rotate", pathRotateMap);
	network.save("nl_trunk_segmentation_map", pathSegmentMap);
}

void RobotNeuronNetwork::load(string name, string path)
{
	network.load(name, path);
}

void RobotNeuronNetwork::save(string name, string path)
{
	network.save(name, path);
}

bool RobotNeuronNetwork::load(const string basename, int half, int side, const string format)
{
	string setname = basename + "_half" + Primitives::toString(half) + "_side" + Primitives::toString(side);
	string pathLeft  = setname + "_left."  + format;
	string pathRight = setname + "_right." + format;

	if (loadCache(basename, half, side, format))
		return true;
	else if (loadInputs(basename, half, side, format))
	{
		robot_con_execute();

		saveCache(basename, half, side, format);

		return true;
	}
	else
		return false;
}

void RobotNeuronNetwork::loadInputs(const string basename, int half, int side, const string format)
{
	string setname = basename + "_half" + Primitives::toString(half) + "_side" + Primitives::toString(side);
	string pathLeft  = setname + "_left."  + format;
	string pathRight = setname + "_right." + format;

	if (File::exists(pathLeft) && File::exists(pathRight))
	{
		network.load("image_left",  pathLeft);
		network.load("image_right", pathRight);
	}
	else
		throw runtime_error("Input file set (" pathLeft + ", " + pathRight + ") not found");

	return inputsExist;
}

/*
Property Section
*/

int RobotNeuronNetwork::getInputWidth()
{
	InputLayer& input = network.getInputLayer("image_right");
	return input.getWidth();
}

int RobotNeuronNetwork::getInputHeight()
{
	InputLayer& input = network.getInputLayer("image_right");
	return input.getHeight();
}
