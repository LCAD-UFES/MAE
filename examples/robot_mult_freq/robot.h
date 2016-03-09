#ifndef _ROBOT_H
#define _ROBOT_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "mae.h"

// Definitions
#define INPUT_WIDTH (320)
#define INPUT_HEIGHT (240)
#define NL_WIDTH (128)
#define NL_HEIGHT (64)
#define FOV_HORIZONTAL (47.05)
#define FOV_VERTICAL (35.71)
#define BACKGROUND_COLOR (128)
#define STIMULUS_LEFT_COLOR (255)
#define STIMULUS_RIGHT_COLOR (255)
#define STIMULUS_WIDTH (0.5)
#define STIMULUS_HEIGHT (17.0)
#define TEST_SCAN_WIDTH (5.0)
#define INPUT_NAME_LEFT ("image_left")
#define INPUT_NAME_RIGHT ("image_right")
#define CAMERA_LEFT_PARAMETERS_FILE_NAME ("CameraLeftParameters.dat")
#define CAMERA_RIGHT_PARAMETERS_FILE_NAME ("CameraRightParameters.dat")
#define FILE_RBT_IMAGE_LEFT ("img_left.rbt")
#define FILE_RBT_IMAGE_RIGHT ("img_right.rbt")
#define FILE_PLOT_SIMPLE_M_R ("s_m_r.out")
#define FILE_PLOT_SIMPLE_M_R_Q ("s_m_r_q.out")
#define FILE_PLOT_SIMPLE_M_L ("s_m_l.out")
#define FILE_PLOT_SIMPLE_M_L_Q ("s_m_l_q.out")
#define FILE_PLOT_SIMPLE_B ("s_b.out")
#define FILE_PLOT_SIMPLE_B_Q ("s_b_q.out")
#define FILE_PLOT_COMPLEX_M_R ("c_m_r.out")
#define FILE_PLOT_COMPLEX_M_L ("c_m_l.out")
#define FILE_PLOT_COMPLEX_B ("c_b.out")
#define FILE_PLOT_MT ("mt.out")
#define FILE_PLOT_MT_GAUSSIAN ("mt_g.out")
#define FILE_PLOT_VERGENCE ("vergence.out")
#define FILE_PLOT_MT_CELL ("mt_cell.out")
#define SOCKET_PORT (27015)
#define HOST_ADDR_MONO ("192.168.0.1")
#define HOST_ADDR_LEFT ("10.50.5.121")
#define HOST_ADDR_RIGHT ("10.50.5.122")
#define GET_IMAGE_FROM_SOCKET (0)
#define GET_IMAGE_FROM_RBT (1)
#define GET_IMAGE_FROM_PNM (2)
#define GET_IMAGE_TEST_STIMULUS (3)
#define LOG_FACTOR (2.0)
#define CYCLES_PER_DEGREE (0.85)
#define MT_SENSIBILITY (0.1)
#define SMOOTHNESS (1.0)
#define CONTRAST_RATIO (10.0)

// Macros
#define INDEX_MT_CELL ((NL_WIDTH * NL_HEIGHT / 2) + (NL_WIDTH / 2))
#define VISION_STEREO (TRUE)
#define KERNEL (KERNEL_2D)

// Structs

// Prototypes
extern void input_generator (INPUT_DESC *, int status);
extern void input_controler (INPUT_DESC *, int status);
extern void log_polar_filter (FILTER_DESC *);
extern void biological_gabor_filter (FILTER_DESC *);
extern void robot_sum_filter (FILTER_DESC *);
extern void robot_complex_cell (FILTER_DESC *);
extern void robot_mt_cell (FILTER_DESC *);
extern void robot_gaussian_filter (FILTER_DESC *);

// Global Variables
NEURON_LAYER nl_lp_image_left;
NEURON_LAYER nl_lp_image_right;
NEURON_LAYER nl_simple_mono_right;
NEURON_LAYER nl_simple_mono_left;
NEURON_LAYER nl_simple_mono_right_q;
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
INPUT_DESC image_left;
INPUT_DESC image_right;
OUTPUT_DESC out_lp_image_left;
OUTPUT_DESC out_lp_image_right;
OUTPUT_DESC out_simple_mono_right;
OUTPUT_DESC out_simple_mono_left;
OUTPUT_DESC out_simple_mono_right_q;
OUTPUT_DESC out_simple_mono_left_q;
OUTPUT_DESC out_simple_binocular;
OUTPUT_DESC out_simple_binocular_q;
OUTPUT_DESC out_complex_mono_right;
OUTPUT_DESC out_complex_mono_left;
OUTPUT_DESC out_complex_binocular;
OUTPUT_DESC out_mt;
OUTPUT_DESC out_mt_gaussian;
OUTPUT_DESC out_mt_gaussian_var;
OUTPUT_DESC out_mt_gaussian_map;
OUTPUT_DESC out_confidence_map;
OUTPUT_DESC out_disparity_map;
#endif
