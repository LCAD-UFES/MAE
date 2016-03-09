/*
This file is part of SMV.

History:

* 2006.02.06 - Created by Helio Perroni Filho
*/

#ifndef __ROBOT_SMV_HPP
#define __ROBOT_SMV_HPP

#include <vector>
#include <string>
#include <GL/glut.h>

#include "model_3d.hpp"
#include "point_cloud.hpp"
#include "robot_neuron_network.hpp"
#include "working_area.hpp"

using std::string;
using std::vector;

using mae::smv::PointCloud;
using mae::smv::WorkingArea;

/*
Datatype Section
*/

/** An image buffer. */
typedef vector<int> ImageBuffer;

/**
A pair of stereo images.
*/
class ImagePair
{
	public: ImageBuffer left;

	public: ImageBuffer right;

	private: void setImageBuffer(ImageBuffer &imageBuffer, int *buffer);

	public: void setLeft(int *buffer);

	public: void setRight(int *buffer);
};

/** A list of stereo image pairs. */
typedef vector<ImagePair> ImagePairList;

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
	
	/** The last selected camera pair. */
	private: int currentSide;
	
	/** Whether the log pile is divided in one or two halves. */
	private: int halves;

	/** Selected image areas for each half of the log pile. */
	private: WorkingArea workingAreas[2];

	/** The lists of stereo image pairs for the front and back halfs of the log pile. */
	private: ImagePairList imagePairs[2];

	/** The 3D model of a log pile. */
	private: MODEL_3D model3d;

	/** Index to the front half of a log pile. */
	public: static int HALF_FRONT;

	/** Index to the back half of a log pile. */
	public: static int HALF_BACK;

	/** Index to the left side of a log pile. */
	public: static int SIDE_LEFT;

	/** Index to the top side of a log pile. */
	public: static int SIDE_TOP;

	/** Index to the right side of a log pile. */
	public: static int SIDE_RIGHT;

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
	private: ~RobotSMV();

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
	
	private: MODEL_3D& computeModel3DFace3M(int side);
	
	private: MODEL_3D& computeModel3DFace6M(int side);

	public: static int inputSize();

	/**
	Computes the 3D model of a log pile out of the current image pair list.
	
	@return A 3D model of the log pile.
	*/
	public: MODEL_3D& computeModel3D();
	
	/**
	Computes the 3D model of a side of a log pile out of the current image pair list.
	
	@param side Side of the log pile to be modelled.

	@return A 3D model of the log pile.
	*/
	public: MODEL_3D& computeModel3DFace(int side);
	
	/**
	Computes the 3D model of the currently selected side of a log pile out of the current image pair list.
	
	@return A 3D model of the log pile.
	*/
	public: MODEL_3D& computeModel3DFace();
	
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
	Rectifies an image taken by the right camera. The input image is assumed to be of the size determined by the SMV constants <code>INPUT_HEIGHT</code> and <code>INPUT_WIDTH</code>. 

	@param image The original image input buffer.

	@param rectified The rectified image output buffer.
	*/
	public: void rectifyImageRight(int* image, ImageBuffer& rectified);

	/**
	Rectifies an image taken by the right camera. Fills the output buffer with only the stereo region of the rectified image. The input image is assumed to be of the size determined by the SMV constants <code>INPUT_HEIGHT</code> and <code>INPUT_WIDTH</code>.

	@param image The original image input buffer.

	@param rectified The rectified image output buffer.
	*/
	public: void stereoRegionRight(int* image, int* rectified);
	
	/*
	Input / Output Section
	*/
	
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
	public: void configWorkingArea(int half, int x0, int y0, int xn, int yn);

	/*
	Property Section
	*/
	
	public: int getCurrentSide();

	public: void setCurrentSide(int side);

	/**
	Gets the 3D model of the log pile enclosed within this object.
	
	@return 3D model of a log pile.
	*/
	public: MODEL_3D& getModel3D();

	/**
	Sets an image pair from a region of a log pile.

	@param half For 6m-long logs, whether the pair refers to the pile's front half (<code>RobotSMV::HALF_FRONT</code>) or back half (<code>RobotSMV::HALF_BACK</code>).

	@param side Whether the pair to the pile's left side (<code>RobotSMV::SIDE_LEFT</code>), top side (<code>RobotSMV::SIDE_TOP</code>) or right side (<code>RobotSMV::SIDE_RIGHT</code>).
	*/
	public: void setImagePair(int half, int side, int* left, int* right);
	
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

class RobotSMVException
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
