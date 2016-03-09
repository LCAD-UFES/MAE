/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_SMV_ROBOT_NEURON_NETWORK_HPP
#define __MAE_SMV_ROBOT_NEURON_NETWORK_HPP

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "neuron_network.hpp"
#include "working_area.hpp"


namespace mae
{
	namespace smv
	{
		class RobotNeuronNetwork;
	};
};

/*
Datatype Section
*/

/** Rectifier function type. */
typedef int (*RectifyFunction)(unsigned char *rectified, unsigned char *original, int width, int height);

/**
The unique entry point for interacting with SMV's underlying neuron network.
*/
class mae::smv::RobotNeuronNetwork
{
	/*
	Attribute Section
	*/

	/** Reference to the wrapped MAE neuron network. */
	private: NeuronNetwork& network;
	
	/** The names of the neuron layers that are to be cached into disk. */
	private: vector<string> cached;

	/*
	Constructor Section
	*/

	/**
	Default Constructor.
	*/
	private: RobotNeuronNetwork();

	/**
	Default destructor.
	*/
	public: ~RobotNeuronNetwork();

	/**
	Returns the class' unique instance.

	<b>Implementation Notes</b>

	This method implements the <a href="http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.13">construct on first use idiom</a> in order to avoid the <a href="http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12"><code>static</code> initialization order fiasco</a>, as described in the <a href="http://www.parashift.com/c++-faq-lite/index.html">C++ FAQ Lite</a>.

	@return The unique instance of this class.
	*/
	public: static RobotNeuronNetwork& getInstance();

	/*
	Method Section
	*/
	
	/**
	Applies a rectification function to the contents of an input layer, most likely data from an image.
	
	@param rectify The rectifier function that is to be applied to the data.
	
	@param input Input layer containing the data to be rectified.
	*/
	private: void rectifyInput(RectifyFunction rectify, InputLayer& input);
	
	/**
	Copies position and color data from the current stereo image pair, for the region outlined by the working area.

	@param workingArea Limits of the region from which the data is to be copied.

	@param left  Position data from the left image.

	@param right Position data from the right image.

	@param colors Color data from the images.
	*/
	public: void copyStereoPoints(WorkingArea& workingArea, vector<double>& left, vector<double>& right, vector<int>& colors);

	/**
	Copies data from the disparity map layer to the merged disparity map layer. Data is copied from a given <i>starting point</i> up to the width of the disparity map, and placed withing the merged map at a given <i>offset.</i> Both arguments are provided in <i>image scale</i> (that is, as valid range values within an input image), but are converted to <i>disparity map scale</i> during processing. So, as long as input image dimensions remain the same, a given argument set remains valid rergardless the actual size of the disparity and merged maps.

	@param w0 Starting width range from which to start copying data, in image pixels.

	@param offset Starting width range from which to start placing data, in image pixels.
	*/
	public: void fillMergeMap(int w0, int offset);

	/**
	Applies a rectification function to the contents of an input layer. The layer is expected to contain a stereo image pair's left image.
	
	@param name Name of the input layer to rectify.
	*/
	public: void rectifyInputLeft(string name);
	
	/**
	Applies a rectification function to the contents of an input layer. The layer is expected to contain a stereo image pair's right image.
	
	@param name Name of the input layer to rectify.
	*/
	public: void rectifyInputRight(string name);
	
	/**
	Copies a region of the right image input layer to a matching region input layer.

	@param name Name of the destination matching region input layer.
	*/
	public: void selectMatchRegion(string name);
	
	/**
	Returns a string representation of this neuron network.
	
	@return A string representation of this neuron network.
	*/
	public: string toString();

	/*
	Input / Output Section
	*/

	/**
	Returns whether a given cache file set exists.
	
	@param basename The path and prefix of the file set.

	@param half The half of the log pile to which the file set corresponds.

	@param side The side of the log pile to which the file set corresponds.

	@param format The format of the files in the set.
	
	@return Whether the file set exists.
	*/
	private: bool isCached(const string basename, int half, int side, const string format);

	/**
	Loads the contents of calculated layers on the network from cache files.

	@param basename The path and prefix of the file set.

	@param half The half of the log pile to which the file set corresponds.

	@param side The side of the log pile to which the file set corresponds.

	@param format The format of the files in the set.
	*/
	private: void loadCache(const string basename, int half, int side, const string format);

	/**
	Saves the contents of calculated layers on the network to cache files.

	@param basename The path and prefix of the file set.

	@param half The half of the log pile to which the file set corresponds.

	@param side The side of the log pile to which the file set corresponds.

	@param format The format of the files in the set.
	*/
	private: void saveCache(const string basename, int half, int side, const string format);

	/**
	Loads the contents of an input or neuron layer from a file.

	@param name Name of the layer.

	@param path Path to the contents file.
	*/
	public: void load(string name, string path);

	/**
	Saves the contents of an input or neuron layer to a file.

	@param name Name of the layer.

	@param path Path to the contents file.
	*/
	public: void save(string name, string path);

	/**
	Loads the contents of the neuron network from a file set, possibly loading the contents of some calculated layers from cache files.

	@param basename The path and prefix of the file set.

	@param side The side of the log pile to which the file set corresponds.

	@param half The half of the log pile to which the file set corresponds.

	@param format The format of the files in the set.

	@return Whether the operation was succesful.
	*/
	public: bool load(const string basename, int half, int side, const string format);
	
	/**
	Loads the contents of the input layers from a file set.

	@param basename The path and prefix of the file set.

	@param side The side of the log pile to which the file set corresponds.

	@param half The half of the log pile to which the file set corresponds.

	@param format The format of the files in the set.

	@throws runtime_error If the operation was not succesful.
	*/
	public: void loadInputs(const string basename, int half, int side, const string format);

	/*
	Property Section
	*/

	/**
	Returns the width for the pair of stereo image input layers.
	*/
	public: int getInputWidth();

	/**
	Returns the height for the pair of stereo image input layers.
	*/
	public: int getInputHeight();
};

#endif
