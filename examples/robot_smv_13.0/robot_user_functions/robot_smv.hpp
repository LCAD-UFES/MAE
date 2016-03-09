/*
This file is part of SMV.

History:

* 2006.02.06 - Created by Helio Perroni Filho
*/

#ifndef __ROBOT_SMV_HPP
#define __ROBOT_SMV_HPP

#include <exception>
using std::exception;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <GL/glut.h>

#include "composed_model_3d.hpp"
using mae::smv::ComposedModel3D;

#include "model_3d.hpp"
using mae::smv::Model3D;

#include "point_cloud.hpp"
using mae::smv::PointCloud;

#include "robot_neuron_network.hpp"
using mae::smv::RobotNeuronNetwork;

#include "working_area.hpp"
using mae::smv::WorkingArea;

/**
Programming interface to SMV. Provides an intuitive, unified control point to code that must drive the application.
*/
class RobotSMV
{
	/*
	Attribute Section
	*/

	/** Reference to the SMV neuron network. */
	private: mae::smv::RobotNeuronNetwork& network;

	/** The selected pile half. */
	private: int currentHalf;

	/** The last selected camera pair. */
	private: int currentFace;

	/** Whether the log pile is divided in one or two halves. */
	private: int halves;

	/** Selected image areas for each half of the log pile. */
	private: vector<WorkingArea> workingAreas;

	/** The 3D model of a log pile. */
	private: ComposedModel3D pileModel;

	/*
	Constructor Section
	*/

	/**
	Default Constructor.
	*/
	private: RobotSMV();

	/**
	Default Destructor.
	*/
	public: ~RobotSMV();

	/**
	Returns the class' unique instance.

	<b>Implementation Notes</b>

	This method implements the <a href="http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.13">construct on first use idiom</a> in order to avoid the <a href="http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12"><code>static</code> initialization order fiasco</a>, as described in the <a href="http://www.parashift.com/c++-faq-lite/index.html">C++ FAQ Lite</a>.

	@return The unique instance of this class.
	*/
	public: static RobotSMV& getInstance();

	/*
	Method Section
	*/

	/**
	Extracts a point cloud from a section of the visual field.
	
	@param workingArea Area from which to extract the point cloud.
	
	@param cloud Point cloud to be filled with the extracted data.
	*/
	private: void extractPointCloud(WorkingArea& workingArea, PointCloud& cloud);

	public: static int inputSize();

	/**
	Computes the 3D model of a log pile out of the current image pair list.

	@return A 3D model of the log pile.
	*/
	public: ComposedModel3D& computeModel3D();
	
	private: void computeModel3D(WorkingArea& workingArea, int section, Model3D& model3d);

	private: int computeHalvesOffset();

	private: Model3D& computeModel3DFace(WorkingArea& workingArea, int section, Model3D& model3d);

	/**
	Computes the 3D model of the currently selected side of a log pile out of the current image pair list.

	@return A 3D model of the log pile.
	*/
	public: Model3D& computeModel3DFace();

	/**
	Computes volume for an image set, using the parameter set taken from a file.

	@param path Path to the parameter file that will be loaded before the volume is computed.
	*/
	public: void execute(string path);
	
	/**
	Extracts the point cloud from the given face.
	
	@param face Index to the face.
	*/
	public: void extractPointCloud();

	private: void extractPointCloud(WorkingArea& workingArea, Model3D& model3d);

	/**
	Loads an image from the filesystem, rectifies it as if it was taken from a right camera, and then extracts its stereo region (that is, the region that is also visible from the left camera). The result is saved to the filesystem as <code>image_stereo_region.ppm</code> on the current directory.

	@param path Path to the original image.
	*/
	public: void extractStereoRegion(string path);

	/**
	Applies a rectification function to the contents of an input layer. The input is expected to contain a stereo image pair's left image.

	@param name The input layer's name.
	*/
	public: void rectifyImageLeft(string name);

	/**
	Applies a rectification function to the contents of an input layer. The input is expected to contain a stereo image pair's right image.

	@param name The input layer's name.
	*/
	public: void rectifyImageRight(string name);

	/**
	Toggles between modeling 3m- and 6m-long log piles.

	@return Whether the 6m-long modeling strategy is selected.
	*/
	public: bool toggleTwoHalves();
	
	/**
	Returns a string representation of this robot.
	
	@return A string representation of this robot.
	*/
	public: string toString();

	/*
	Input / Output Section
	*/
	
	/**
	Loads the images from a given half and side of the log pile.

	@param half Log pile half.
	@param side Log pile side.
	*/
	private: void loadImages(int half, int side);

	/**
	Loads the parameters needed by the rectification functions.

	@param side Index for the rectification file set.
	*/
	private: void loadRectificationParameters(int side);
	
	/**
	Loads SMV's input neuron layers with data and executes the computations needed to fill the upstream layers. If the config setting <code>smv.mode</code> is set to <code>maintenance</code>, loads the input data from files in the current filesystem directory, instead of the in-memory image pair list.

	@param half The index of the specific image pair, among those used to cover a single log pile side, that will be used in the following computations.

	@param side which side of the log pile the input data refers to.
	*/
	public: void configNeuronLayers(int half, int side);

	/**
	Configures the user-selected image area used in computations.
	*/
	public: void configWorkingArea();

	/**
	Configures the user-selected image area used in computations.
	*/
	public: void configWorkingArea(int x0, int y0, int xn, int yn);

	/*
	Property Section
	*/

	public: int getCurrentHalf();

	public: void setCurrentHalf(int half);

	public: int getCurrentFace();

	public: void setCurrentSide(int side);

	/**
	Gets the 3D model of the log pile enclosed within this object.

	@return 3D model of a log pile.
	*/
	public: ComposedModel3D& getModel3D();

	/**
	Tells whether the robot assumes the log pile is divided in two halves.

	@return <code>true</code> if the robot assumes that a pile is represented by two image sets; <code>false</code> otherwise.
	*/
	public: bool isTwoHalves();

	/**
	Sets whether the robot assumes the log pile is divided in two halves.

	@param twoHalves <code>true</code> if the robot assumes that a pile is represented by two image sets; <code>false</code> otherwise.
	*/
	public: void setTwoHalves(bool twoHalves);

	/**
	Gets the working area for a half of the pile.

	@param half Index to the half of the pile.

	@return Reference to an working area.

	@see mae::smv::WorkingArea
	*/
	public: WorkingArea& getWorkingArea(int half);

	/**
	Sets the limits of the user-selected image area.

	@param half Index to the half of the pile.

	@param x0 Horizontal coordinate for the working area's lower left corner.
	@param y0 Vertical coordinate for the working area's lower left corner.
	@param xn Horizontal coordinate for the working area's upper right corner.
	@param yn Vertical coordinate for the working area's upper right corner.
	*/
	public: void setWorkingArea(int half, int x0, int y0, int xn, int yn);
};

class RobotSMVException: public exception
{
	/*
	Attribute Section
	*/

	private: int code;

	private: string message;

	/*
	Constructor Section
	*/

	public: RobotSMVException(int code, string message)
	{
		this->code = code;
		this->message = message;
	}
	
	public: virtual ~RobotSMVException() throw()
	{
	}
	
	/*
	Method Section
	*/
	
	virtual const char* what() const throw()
	{
		return message.c_str();
	}

	/*
	Property Section
	*/

	public: int getCode()
	{
		return code;
	}

	public: string getMessage()
	{
		return message;
	}
};

#endif
