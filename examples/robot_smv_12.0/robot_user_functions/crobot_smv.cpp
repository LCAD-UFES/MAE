/*
This file is part of SMV.

History:

[2006.02.07 - Helio Perroni Filho] Created.
*/

#include "crobot_smv.h"

#include <exception>

#include "model_3d.hpp"
#include "model_3d_io.hpp"
#include "robot_smv.hpp"

#include "cylinder_list.h"
#include "robot_con.h"

using std::exception;

/*
Function Section
*/

void robot_smv_compute_current()
{
	printf("Processing current log side...\n");
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.computeModel3DFace();
	}
	catch (exception& e)
	{
		printf(e.what());
		printf("\n");
	}
}

void robot_smv_compute_all()
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		MODEL_3D& model3d = robot.computeModel3D();	
		CYLINDER_LIST *list = model_3d_all_cylinders(&model3d);
	
		if (list == NULL)
			fprintf(stderr, "cylinder list points to NULL.");
		else
			robot_con_setup_circle_packing(list);
	}
	catch (exception& e)
	{
		printf(e.what());
		printf("\n");
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
		printf(e.what());
		printf("\n");
	}
}

void robot_smv_save_working_area(int half, int* workingArea)
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.configWorkingArea(half, workingArea[0], workingArea[1], workingArea[2], workingArea[3]);
	}
	catch (exception& e)
	{
		printf(e.what());
		printf("\n");
	}
}

void robot_smv_select_side(int side)
{
	printf("Loading images from log side [%d]...\n", side);
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		robot.setCurrentSide(side);
		printf("Done.\n");
	}
	catch (exception& e)
	{
		printf(e.what());
		printf("\n");
	}
}

int get_current_camera_pair()
{
	try
	{
		RobotSMV& robot = RobotSMV::getInstance();
		return robot.getCurrentSide();
	}
	catch (exception& e)
	{
		printf(e.what());
		printf("\n");
	}
	
	return -1;
}

/*
Graphics Section
*/

void robot_smv_display()
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_view(&model3d);
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
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_rotate_face(&model3d, get_current_camera_pair(), MODEL_3D_X_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_rotate_y(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_rotate_face(&model3d, get_current_camera_pair(), MODEL_3D_Y_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_rotate_z(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_rotate_face(&model3d, get_current_camera_pair(), MODEL_3D_Z_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_straight()
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_straight_face(&model3d, get_current_camera_pair());
	model_3d_view(&model3d);
}

void robot_smv_translate_x(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_translate_face(&model3d, get_current_camera_pair(), MODEL_3D_X_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_translate_y(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_translate_face(&model3d, get_current_camera_pair(), MODEL_3D_Y_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_translate_z(double value)
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_translate_face(&model3d, get_current_camera_pair(), MODEL_3D_Z_AXIS, value);
	model_3d_view(&model3d);
}

void robot_smv_center()
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_center_face(&model3d, get_current_camera_pair());
	model_3d_view(&model3d);
}

/*
Input / Output Section
*/

void robot_smv_load_config()
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_load_config(&model3d);
}

void robot_smv_save_config()
{
	RobotSMV& robot = RobotSMV::getInstance();
	MODEL_3D& model3d = robot.getModel3D();
	model_3d_save_config(&model3d);
}

/*
Property Section
*/

void robot_smv_set_two_halves(int halves)
{
	RobotSMV& robot = RobotSMV::getInstance();
	robot.setTwoHalves(halves != 0);
}
