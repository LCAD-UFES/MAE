#ifndef __CUDA_FUNCTIONS_H
#define __CUDA_FUNCTIONS_H

struct _v1_mt_private_state
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_fields;
	int  *d_xi, *d_yi, *d_image_vector;
	float *d_s_r, *d_s_r_q, *d_s_l, *d_s_l_q;
	float *d_mt;
	int *image_vector_right, *image_vector_left;
	float **rf_points_w, **rf_points_gabor, **rf_points_gauss;
	int **rf_points_x, **rf_points_y;
	float *mt_gaussian, *d_mt_gaussian;
	int *xi, *yi;
	int *I_translated_image_left;

};

typedef struct _v1_mt_private_state V1_MT_PRIVATE_STATE;




#endif