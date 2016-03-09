// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY

#ifndef _ROBOT_H
#define _ROBOT_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"

// Definitions
#define JOSELITO_IMAGE_WIDTH (320)
#define JOSELITO_IMAGE_HEIGHT (240)
#define SIMULATOR_IMAGE_WIDTH (256)
#define SIMULATOR_IMAGE_HEIGHT (256)
#define NL_WIDTH (128)
#define NL_HEIGHT (64)
#define GET_IMAGE_FROM_SOCKET (0)
#define GET_IMAGE_FROM_RBT (1)
#define GET_IMAGE_FROM_PNM (2)
#define GET_IMAGE_FROM_SIMULATOR (3)
#define LOG_FACTOR (2.0)
#define CYCLES_PER_DEGREE (0.85)
#define MT_SENSIBILITY (0.1)
#define SMOOTHNESS (1.0)
#define CONTRAST_RATIO (10.0)
#define ROBOT_FOVY (47.05)
#define ROBOT_FAR (500.0)
#define ROBOT_NEAR (0.1)
#define ROBOT_CAMERA_SEPARATION (6.9)
#define ROBOT_RADIUS (15.0)
#define ROBOT_HEIGHT (6.5)
#define TERRAIN_SIZE (500.0)
#define JOSELITO_FOCUS (400.0)
#define SIMULATOR_ROBOT_FOCUS (294.0)
#define TURN_LEFT_STEP (5.0)
#define TURN_RIGHT_STEP (5.0)
#define MOVE_FORWARD_STEP (1.5)
#define FUZZY_INPUTS_NUMBER (3)
#define FUZZY_OUTPUTS_NUMBER (2)
#define FIS_FILE ("robot.fis")
#define CAMERA_LEFT_PARAMETERS_FILE_NAME ("CameraLeftParameters.dat")
#define CAMERA_RIGHT_PARAMETERS_FILE_NAME ("CameraRightParameters.dat")
#define FILE_RBT_IMAGE_LEFT ("img_left.rbt")
#define FILE_RBT_IMAGE_RIGHT ("img_right.rbt")
#define SOCKET_PORT (27015)
#define HOST_ADDR_MONO ("192.168.0.1")
#define HOST_ADDR_LEFT ("10.50.5.121")
#define HOST_ADDR_RIGHT ("10.50.5.122")
#define CHANNELS (2)
#define FEATURES_PER_CHANNEL (4)
#define BAND_WIDTH (0.125)
#define CUT_POINT (0.7)

// Macros
#define INPUT_WIDTH (SIMULATOR_IMAGE_WIDTH)
#define INPUT_HEIGHT (SIMULATOR_IMAGE_HEIGHT)
#define IMAGE_SOURCE (GET_IMAGE_FROM_SIMULATOR)
#define ROBOT_FLOOR (-16.0)
#define ROBOT_FOCUS (SIMULATOR_ROBOT_FOCUS)
#define VIRTUAL_FOV (ROBOT_FOVY)
#define INDEX_MT_CELL ((NL_WIDTH * NL_HEIGHT / 2) + (NL_WIDTH / 2))
#define VISION_STEREO (TRUE)
#define FEATURES_NUMBER (CHANNELS * FEATURES_PER_CHANNEL)
#define FEATURES_SAMPLE ((int) (BAND_WIDTH * NL_WIDTH))

// Structs

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void output_handler_mean_position (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); 
extern void biological_gabor_filter (FILTER_DESC *);
extern void robot_sum_filter (FILTER_DESC *);
extern void robot_complex_cell (FILTER_DESC *);
extern void robot_mt_cell (FILTER_DESC *);
extern void robot_gaussian_filter (FILTER_DESC *);
extern NEURON_TYPE minchinton;
extern NEURON_OUTPUT Verge (PARAM_LIST *);
extern NEURON_OUTPUT SetVergenceAtPoint (PARAM_LIST *);
extern NEURON_OUTPUT Reconstruct3DWorld (PARAM_LIST *);
extern NEURON_OUTPUT GetLandmark (PARAM_LIST *);
extern NEURON_OUTPUT MemorizeLandmark (PARAM_LIST *);
extern NEURON_OUTPUT SearchLandmark (PARAM_LIST *);
extern NEURON_OUTPUT GetFuzzyInputsFromMap (PARAM_LIST *);
extern NEURON_OUTPUT ForwardFuzzyMachine (PARAM_LIST *);
extern NEURON_OUTPUT TransformMap (PARAM_LIST *);
extern NEURON_OUTPUT TurnRobot (PARAM_LIST *);
extern NEURON_OUTPUT ForwardRobot (PARAM_LIST *);
extern NEURON_OUTPUT Delay (PARAM_LIST *);
extern NEURON_OUTPUT GetRotation (PARAM_LIST *);
extern NEURON_OUTPUT UpdateEvidenceMap (PARAM_LIST *);
extern NEURON_OUTPUT GetNextMovement (PARAM_LIST *);
extern NEURON_OUTPUT MakePath (PARAM_LIST *);
extern NEURON_OUTPUT MakePath2 (PARAM_LIST *);

// Global Variables
NEURON_LAYER nl_simple_mono_right;
NEURON_LAYER nl_simple_mono_right_q;
NEURON_LAYER nl_simple_mono_left;
NEURON_LAYER nl_simple_mono_left_q;
NEURON_LAYER nl_simple_binocular;
NEURON_LAYER nl_simple_binocular_q;
NEURON_LAYER nl_complex_mono_right;
NEURON_LAYER nl_complex_mono_left;
NEURON_LAYER nl_complex_binocular;
NEURON_LAYER nl_mt;
NEURON_LAYER nl_mt_gaussian;
NEURON_LAYER nl_mt_gaussian_var;
NEURON_LAYER nl_mt_gaussian_map;
NEURON_LAYER nl_confidence_map;
NEURON_LAYER nl_disparity_map;
NEURON_LAYER nl_gabor_big_h_00;
NEURON_LAYER nl_gabor_big_h_90;
NEURON_LAYER nl_gabor_big_v_00;
NEURON_LAYER nl_gabor_big_v_90;
NEURON_LAYER nl_gabor_small_h_00;
NEURON_LAYER nl_gabor_small_h_90;
NEURON_LAYER nl_gabor_small_v_00;
NEURON_LAYER nl_gabor_small_v_90;
NEURON_LAYER nl_v1_activation_map;
NEURON_LAYER nl_features;
NEURON_LAYER nl_features1;
NEURON_LAYER nl_target_coordinates;
INPUT_DESC image_left;
INPUT_DESC image_right;
OUTPUT_DESC out_confidence_map;
OUTPUT_DESC out_disparity_map;
OUTPUT_DESC out_features;
OUTPUT_DESC out_features1;
OUTPUT_DESC out_v1_activation_map;
OUTPUT_DESC out_target_coordinates;
#endif
