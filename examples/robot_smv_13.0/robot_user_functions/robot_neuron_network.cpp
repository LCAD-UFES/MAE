/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#include "robot_neuron_network.hpp"
using mae::smv::RobotNeuronNetwork;

#include "robot_neuron_network_c.h"
#include "stereo_api.h"

#include <stdexcept>
using std::runtime_error;

#include "config_default.hpp"

#include "file.hpp"
using mae::io::File;

#include "garbage_collector.hpp"
using mae::manage;

#include "logging.hpp"
using mae::logging::log;
using mae::logging::trace;

#include "primitives.hpp"
using mae::util::Primitives;

/*
Definition Section
*/

#define ZOOM(value, fromScale, toScale) (int) ((float) (value * toScale) / (float) fromScale + 0.5)

/*
Constructor Section
*/

RobotNeuronNetwork::RobotNeuronNetwork(): network(NeuronNetwork::getInstance()), cached()
{
	trace("RobotNeuronNetwork::RobotNeuronNetwork");
	manage<RobotNeuronNetwork>(this);
	
	cached.push_back("image_left");
	cached.push_back("image_right");
	cached.push_back("nl_disparity_map");
	cached.push_back("nl_trunk_segmentation_map_vertical");
	cached.push_back("nl_trunk_segmentation_map_vertical_rotate");
	cached.push_back("nl_trunk_segmentation_map");
	cached.push_back("nl_segment");
	
	log(string() + "cached(" + cached.size() + ")");
}

RobotNeuronNetwork::~RobotNeuronNetwork()
{
	trace("RobotNeuronNetwork::~RobotNeuronNetwork");
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

string RobotNeuronNetwork::toString()
{
	return string() + "mae::smv::RobotNeuronNetwork [" + ((int) this) + "]";
}

/*
Input / Output Section
*/

bool RobotNeuronNetwork::isCached(const string basename, int half, int side, const string format)
{
	trace("RobotNeuronNetwork::isCached", "\"" + basename + "\", " + half + ", " + side + ", \"" + format + "\"");

	string setname = basename + "_half" + half + "_side" + side + "_";
	for (vector<string>::iterator i = cached.begin(), n = cached.end(); i != n; i++)
	{
		string layer = (*i);
		string path = setname + layer + "." + format;
		if(!File::exists(path))
			return false;
	}
	
	return true;
}

void RobotNeuronNetwork::loadCache(const string basename, int half, int side, const string format)
{
	trace("RobotNeuronNetwork::loadCache", "\"" + basename + "\", " + half + ", " + side + ", \"" + format + "\"");

	string setname = basename + "_half" + half + "_side" + side + "_";
	for (vector<string>::iterator i = cached.begin(), n = cached.end(); i != n; i++)
	{
		string layer = (*i);
		string path = setname + layer + "." + format;
		network.load(layer, path);
	}

	network.updateAllOutputs();
}

void RobotNeuronNetwork::saveCache(const string basename, int half, int side, const string format)
{
	trace("RobotNeuronNetwork::saveCache", "\"" + basename + "\", " + half + ", " + side + ", \"" + format + "\"");

	string setname = basename + "_half" + half + "_side" + side + "_";
	for (vector<string>::iterator i = cached.begin(), n = cached.end(); i != n; i++)
	{
		string layer = (*i);
		string path = setname + layer + "." + format;
		network.save(layer, path);
	}

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
	trace("RobotNeuronNetwork::load", "\"" + basename + "\", " + half + ", " + side + ", \"" + format + "\"");

	bool useCache = ConfigDefault::boolValue("input.cached", false);

	if (useCache && isCached(basename, half, side, format))
		loadCache(basename, half, side, format);
	else
	{
		loadInputs(basename, half, side, format);

		trace("robot_neuron_network_execute");
		robot_neuron_network_execute();

		saveCache(basename, half, side, format);

		return true;
	}
	
	return true;
}

void RobotNeuronNetwork::loadInputs(const string basename, int half, int side, const string format)
{
	trace("RobotNeuronNetwork::loadInputs", "\"" + basename + "\", " + half + ", " + side + ", \"" + format + "\"");

	string setname = basename + "_half" + half + "_side" + side;
	string pathLeft  = setname + "_left."  + format;
	string pathRight = setname + "_right." + format;

	if (File::exists(pathLeft) && File::exists(pathRight))
	{
		network.load("image_left",  pathLeft);
		network.load("image_right", pathRight);
	}
	else
		throw runtime_error("Input file set (" + pathLeft + ", " + pathRight + ") not found");
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
