/*
 * parser.h
 *
 *  Created on: Mar 14, 2013
 *      Author: cayo
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace statistics2 {

class parser {

	typedef struct
	{
		int image_id;
		double scale_factor;
		int x;
		int y;
		double confidence;
		int TS_type;
		int x_g;
		int y_g;
		int width;
		int height;
		int hit;
	} Line;

	#define IMG_WIDTH 1360
	#define IMG_HEIGHT 800
	#define NUM_BINS_X 10
	#define NUM_BINS_Y 10
	#define NUM_BINS_Z 12
	#define NUM_BINS_C 10
	#define NUM_DIVISIONS_C 1000

	#define MAX_SCALE_FACTOR 2.03166
	#define MIN_SCALE_FACTOR 0.27344
	double zoom_levels[5] = {0.31250, 0.48906, 0.76538, 1.19782, 1.87460};
	//double zoom_levels[12] = {0.27344, 0.32812, 0.39375, 0.47250, 0.56700, 0.68040, 0.81648, 0.97978, 1.17573, 1.41088, 1.69305, 2.03166};

	double prob_x_bins_given_hit[NUM_BINS_X];
	double prob_y_bins_given_hit[NUM_BINS_Y];
	double prob_z_bins_given_hit[NUM_BINS_Z];
	double prob_x_bins_given_miss[NUM_BINS_X];
	double prob_y_bins_given_miss[NUM_BINS_Y];
	double prob_z_bins_given_miss[NUM_BINS_Z];
	double prob_c_bins_given_hit[NUM_DIVISIONS_C];
	double prob_c_bins_given_miss[NUM_DIVISIONS_C];

	Line *data;
	int data_length;
	int data_alloc;

public:
	parser();

	virtual ~parser();
};

} /* namespace std */
#endif /* PARSER_H_ */
