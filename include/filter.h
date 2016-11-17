/*
*********************************************************************************
* Module : MAE Filter Definitions						*
* version: 1.0									*
*    date: 30/12/2003								*
*      By: Stiven Schwanz & Alberto Ferreira de Souza				*
********************************************************************************* 
*/
#ifndef __FILTER_H
#define __FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include "mae.h"

#define TAM_GAUSS_VEC		1000
#define TAM_MAGFACTOR_VEC	1000
#define POSITIVE		0
#define	NEGATIVE		1
#define DIFFERENCE		2
#define DIMENSIONS		3
#define CYCLES 			1.375
#define	ORIENTATION		0.0
#define SIGMA_F			1.0
#define SIGMA_W			0.0
#define C1			95.77775

// Bidimensional Convolution Definitions
#define CORNER_POINT_WEIGHT	1.0
#define EDGE_POINT_WEIGHT	2.0
#define MIDDLE_POINT_WEIGHT	4.0

struct _point
{
	int x;
	int y;
	double gaussian;
	double gaussian_big;
	double gaussian_small;
	double gabor;
	double weight;
	
};

typedef struct _point POINT;

struct _item
{
	int x;
	int y;
	double features[DIMENSIONS];
	double activation;
};

typedef struct _item ITEM;

struct kernel
{
	int    kernel_size;
	float  kernel_sigma;
	float *kernel_points;
};

typedef struct kernel KERNEL;

struct _receptive_field_description
{
	int num_points;
	int kernel_size;
 	POINT *points;
	double max_dog;
	double min_dog;
	double cut_orientation;
	double area;
	double delta_area;
	double frequency;
};

typedef struct _receptive_field_description RECEPTIVE_FIELD_DESCRIPTION;

struct _sc_receptive_field_description
{
	int num_points;
	int kernel_size;
	float *points;
};

typedef struct _sc_receptive_field_description SC_RECEPTIVE_FIELD_DESCRIPTION;


struct _binocular_simple_cell
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_left;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_right;
};

typedef struct _binocular_simple_cell BINOCULAR_SIMPLE_CELL;

struct _quadrature_pair
{
	BINOCULAR_SIMPLE_CELL *binocular_simple_cell_cos;
	BINOCULAR_SIMPLE_CELL *binocular_simple_cell_sin;
	double *responses;
};

typedef struct _quadrature_pair QUADRATURE_PAIR;

struct _complex_cell
{
	double phase_left;
	double phase_right;
	double omega_zero;
	double *weights_vector;
	double sigma_w;
	double response;
	QUADRATURE_PAIR *quadrature_pair;
};

typedef struct _complex_cell COMPLEX_CELL;

struct _complex_cells_clustering
{
	COMPLEX_CELL *complex_cells;
};

typedef struct _complex_cells_clustering COMPLEX_CELLS_CLUSTERING;


typedef struct
{
	double val;
	int disparity;
} DISPARITY_SAMPLE;
	
typedef struct
{
	char *neg_slope;
	DISPARITY_SAMPLE **samples;
	float *mt_gaussian_with_previous_disparity;
} DISPARITY_DATA;


#define NUM_DISPARITY_SAMPLES 		5
#define DISPARITY_MAP_FILTER_INIT 	0
#define DISPARITY_MAP_FILTER_SAMPLE	1
#define DISPARITY_MAP_FILTER_OPTIMIZE	2
#define DISPARITY_MAP_FILTER_WINNER_TAKES_IT_ALL_STEPS 10

double distance_from_image_center (double wi, double hi, double w, double h, double u, double log_factor);
void density_function_cs (double *weigth_vector, int w, double sigma, double phase, double cycles, double offset, double scale_factor);
void map_v1_to_image(int *xi, int *yi, double wi, double hi, double u, double v, double w, double h, double x_center, double y_center, double correction, double log_factor);
void map_v1_to_image_d(double *xi, double *yi, double wi, double hi, double u, double v, double w, double h, double x_center, double y_center, double correction, double log_factor);
double map_image_to_v1_old (int *xi, int *yi, int wi, int hi, int u, int v, int w, int h, int wei, double log_factor);
void map_image_to_v1(int xi, int yi, double wi, double hi, int *u, int *v, double w, double h, double x_center, double y_center, double correction, double log_factor);
NEURON_OUTPUT bidimentional_convolution (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON *image_n_l, int x_center, int y_center, int w, int h);
void compute_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int nKernelSize, float fltSigma);
KERNEL *compute_gaussian_kernel2 (int kernel_size, float sigma);
float apply_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *image_n_l, int x0, int y0);
void compute_receptive_field_points (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int radius);
void compute_v1_spatial_receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field, float frequency, float width, float aspect_ratio, float orientation, float phase, int kernel_type);
void bidimentional_convolution_test_gabor (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor);

void optimized_2d_point_convolution (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type, int input_type);

float compute_scale_factor(float l, float c, float u, float m, float a);

#endif
