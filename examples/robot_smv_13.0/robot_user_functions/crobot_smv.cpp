/*
This file is part of SMV.

History:

[2006.02.07 - Helio Perroni Filho] Created.
*/

#include "crobot_smv.h"

#include <stdexcept>
using std::exception;
using std::runtime_error;

#include "cylinder_list.h"

#include "logging.hpp"
using mae::logging::log;
using mae::logging::report;
using mae::logging::trace;

#include "model_3d.hpp"
#include "model_3d_io.hpp"
using mae::smv::Model3D;

#include "primitives.hpp"
using mae::util::Primitives;

#include "robot_smv.hpp"

/*
Function Section
*/

void robot_smv_compute_current()
{
	try
	{
		log("Processing current log side...");
		RobotSMV& robot = RobotSMV::getInstance();
		robot.computeModel3DFace();
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_compute_point_cloud_current()
{
	try
	{
		log("Processing current log side...");
		RobotSMV& robot = RobotSMV::getInstance();
		robot.extractPointCloud();
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_compute_all()
{
	try
	{
		log("Computing volumes...\n");
		RobotSMV& robot = RobotSMV::getInstance();
		ComposedModel3D& model3d = robot.computeModel3D();
		double stereoVolume = model3d.getStereoVolume();
		double solidVolume  = model3d.getSolidVolume();
		
		log(string() + "Mean length: " + model3d.getLength());
		log(string() + "Mean width: "  + model3d.getWidth());
		log(string() + "Mean height: " + model3d.getHeight());
		log(string() + "Stereo volume: " + stereoVolume);
		log(string() + "Solid volume: " + solidVolume);
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_compute_model_3d_volume()
{
	try
	{
		log("Computing volumes...");
		RobotSMV& robot = RobotSMV::getInstance();
		ComposedModel3D& model3d = robot.getModel3D();
		model3d.getStereoVolume();
		model3d.getSolidVolume();
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_config_neuron_layers(int half, int side)
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.configNeuronLayers(half, side);
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_save_working_area(int half, int* workingArea)
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.configWorkingArea(workingArea[0], workingArea[1], workingArea[2], workingArea[3]);
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_select_half(int half)
{
	trace("robot_smv_select_half", string() + half);
	try
	{
		log(string() + "Selecting " + (half == 0 ? "front" : "back") + " half...");
		RobotSMV& robot = RobotSMV::getInstance();
		robot.setCurrentHalf(half);
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
	catch (...)
	{
		report("Unknown error");
	}
}

void robot_smv_select_side(int side)
{
	trace("robot_smv_select_side", string() + side);
	try
	{
		log(string() + "Loading images from pile side [" + side + "]...");
		RobotSMV& robot = RobotSMV::getInstance();
		robot.setCurrentSide(side);
		log("Done.");
	}
	catch (exception& e)
	{
		report(e);
	}
	catch (...)
	{
		report("Unknown error");
	}
}

int robot_smv_toggle_two_halves()
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.toggleTwoHalves();
		return (robot.isTwoHalves() ? 2 : 1);
	}
	catch (exception& e)
	{
		report(e);
	}
	
	return 1;
}

int get_current_camera_pair()
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		return robot.getCurrentFace();
	}
	catch (exception& e)
	{
		report(e);
	}
	
	return -1;
}

/*
Graphics Section
*/

void robot_smv_display()
{
	try
	{
	RobotSMV& robot = RobotSMV::getInstance();
	ComposedModel3D& model3d = robot.getModel3D();
		int currentHalf = robot.getCurrentHalf();
		model_3d_view(model3d.getModel3D(currentHalf));
	}
	catch (exception& e)
	{
		report(e);
	}
}

void robot_smv_rectify_left(char* name)
{
	RobotSMV& robot = RobotSMV::getInstance();
	robot.rectifyImageLeft(name);
}

void robot_smv_rectify_right(char* name)
{
	RobotSMV& robot = RobotSMV::getInstance();
	robot.rectifyImageRight(name);
}

void robot_smv_rotate_x(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.rotate(get_current_camera_pair(), MODEL_3D_X_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_rotate_y(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.rotate(get_current_camera_pair(), MODEL_3D_Y_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_rotate_z(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.rotate(get_current_camera_pair(), MODEL_3D_Z_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_straight()
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.straight(get_current_camera_pair());
	model_3d_view(model3d);
}

void robot_smv_translate_x(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.translate(get_current_camera_pair(), MODEL_3D_X_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_translate_y(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.translate(get_current_camera_pair(), MODEL_3D_Y_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_translate_z(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.translate(get_current_camera_pair(), MODEL_3D_Z_AXIS, value);
	model_3d_view(model3d);
}

void robot_smv_center()
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model3d.center(get_current_camera_pair());
	model_3d_view(model3d);
}

/*
Input / Output Section
*/

void robot_smv_load_config()
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model_3d_load_config(model3d);
}

void robot_smv_save_config()
{
	RobotSMV& robot = RobotSMV::getInstance();

	int currentHalf = robot.getCurrentHalf();
	ComposedModel3D& composed = robot.getModel3D();
	Model3D& model3d = composed.getModel3D(currentHalf);
	model_3d_save_config(model3d);
}

/*
Property Section
*/

void robot_smv_set_two_halves(int halves)
{
	RobotSMV& robot = RobotSMV::getInstance();
	robot.setTwoHalves(halves != 0);
}
