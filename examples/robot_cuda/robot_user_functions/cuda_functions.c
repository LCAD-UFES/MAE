/*!
********************************************************************************
***************************** CUDA (on CPU) SESSION ****************************
******************* Camilo A. Carvalho & Alberto F. De Souza *******************
********************** {camilo, alberto}@lcad.inf.ufes.br **********************
*********************************** AGO/2009 ***********************************
********************************************************************************
*/

#include "mae.h"
#include "filter.h"
#include "cuda_functions.h"



/*!
Copias de dados - de um NEURON_VECTOR para vetor float simples, e vice-versa
*/
void
copyneuron2floatvec (float *float_vec, NEURON *neuron_vec, int size)
{
	int i;

	for (i = 0; i < size; i++)
		float_vec[i] = neuron_vec[i].output.fval;

	return;
}



void
copyfloatvec2neuron (NEURON *neuron_vec, float *float_vec, int size)
{
	int i;

	for (i = 0; i < size; i++)
		neuron_vec[i].output.fval = float_vec[i];

	return;
}



/*!
Aloca e Desaloca areas de memoria com chamadas semelhantes as do CUDA
*/
void
cudaMemcpy (void *d_A, void *h_A, int size)
{
	memcpy (d_A, h_A, size);
}



void
cudaFree (void *d_A)
{
	free (d_A);
}



/*!
Aloca as areas de memoria dos elementos de um PRIVATE_STATE
*/
void
cuda_alloc_device_data_structures (V1_MT_PRIVATE_STATE *v1_mt_private_state, int wi, int hi, int num_neurons)
{
	int dimension = wi*hi;

	v1_mt_private_state->d_image_vector = (int *) malloc (dimension * sizeof (int));
	v1_mt_private_state->d_xi = (int *) malloc (num_neurons * sizeof (int));
	v1_mt_private_state->d_yi = (int *) malloc (num_neurons * sizeof (int));

	v1_mt_private_state->d_s_r = (float *) malloc (num_neurons * sizeof (float));
	v1_mt_private_state->d_s_r_q = (float *) malloc (num_neurons * sizeof (float));
	v1_mt_private_state->d_s_l = (float *) malloc (num_neurons * sizeof (float));
	v1_mt_private_state->d_s_l_q = (float *) malloc (num_neurons * sizeof (float));

	v1_mt_private_state->d_mt = (float *) malloc (num_neurons * sizeof (float));

	v1_mt_private_state->d_mt_gaussian = (float *) malloc (num_neurons * sizeof (float));
}



/*!
*********************************************************************************
* Function: cuda_biological_gabor_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel MAP_V1
void
map_v1 (int *xi, int *yi, int wi, int hi, int w, int h, int x_center, int y_center, float correction, float log_factor, int shift)
{
	float CUDA_LOG_POLAR_SCALE_FACTOR = 1.0;
	float CUDA_LOG_POLAR_THETA_CORRECTION = 0.0;
	float d, theta, exp_val, x;
	int i;

	int u = 0, v = 0;
	int adjust;

	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			i = v * w + u;

			if (u < w/2)
			{
				adjust = (w-1)/2 - u;
				x = ((float) adjust / (float) (w/2)) * log_factor;
				exp_val = (float) (wi/2) * (exp (log (log_factor) * (x - log_factor) / log_factor) - (1.0/log_factor)) * (log_factor / (log_factor - 1.0));
				d = CUDA_LOG_POLAR_SCALE_FACTOR * exp_val;
				theta = pi * ((h * (3.0 / 2.0) - (v * correction)) / h) + CUDA_LOG_POLAR_THETA_CORRECTION;
			}
			else
			{
				adjust = u - w/2;
				x = ((float) adjust / (float) (w/2)) * log_factor;
				exp_val = (float) (wi/2) * (exp (log (log_factor) * (x - log_factor) / log_factor) - (1.0/log_factor)) * (log_factor / (log_factor - 1.0));
				d = CUDA_LOG_POLAR_SCALE_FACTOR * exp_val;
				theta = pi * ((h * (3.0 / 2.0) + (v * correction)) / h) + CUDA_LOG_POLAR_THETA_CORRECTION;
			}

			xi[i] = (int) (d * cos(theta) + 0.5) + x_center + shift;
			yi[i] = (int) (d * sin(theta) + 0.5) + y_center;
		}
	}
}

// O filtro CUDA_MAP_V1_TO_IMAGE eh a chamada de kernel do filtro V1_MT
void 
cuda_map_v1_to_image (int *xi, int *yi, int wi, int hi, int w, int h, int x_center, int y_center, float correction, float log_factor, int shift)
{
	map_v1 (xi, yi, wi, hi, w, h, x_center, y_center, correction, log_factor, shift);
}



/*!
*********************************************************************************
* Function: cuda_biological_gabor_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Alocacao dos vetores (globais) dos campos receptivos
int *d_rf_points_x, *d_rf_points_y;
float *d_rf_points_w, *d_rf_points_g;

// Device CUDA_BIDIMENTIONAL_CONVOLUTION
float 
cuda_bidimentional_convolution (int rf_num_points, int *image_vector, int x_center, int y_center, int w, int h,
				float p_global_factor, float p_teste)
{
	int x_current, y_current, i, pixel;
	float intensity, red, green, blue;
	float accumulator;

	// Initializes the accumulator variable
	accumulator = 0.0;

        for (i = 0; i < rf_num_points; i++)
	{
		// Calculates the current point
		x_current = x_center + d_rf_points_x[i];
		y_current = y_center + d_rf_points_y[i];

		// Verifies if the point is inside of the neuron layer bounds 
		if ((x_current < 0) || (x_current >= w) || (y_current < 0) || (y_current >= h))
			continue;

		// Gets the output pixel value 
		pixel = image_vector[y_current * w + x_current];

		// Extracts the red, green and blue components of the pixel
		red   = (float) RED   (pixel);
		green = (float) GREEN (pixel);
		blue  = (float) BLUE  (pixel);

		// Calculates the intensity value
		intensity = p_teste + (p_global_factor * (red + green + blue)) / 3.0;

		// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
		accumulator += d_rf_points_w[i] * d_rf_points_g[i] * intensity;
	}

	return (accumulator);
}

// Kernel CUDA_BIOLOGICAL_GABOR
void
cuda_biological_gabor (float *s_m, int *xi, int *yi, int wi, int hi, int size, int rf_num_points,
			int *image_vector, float global_factor, float c_delta_area, float teste)
{
	int i;
	for (i = 0; i < size; i++)
	{
		s_m[i] = c_delta_area * cuda_bidimentional_convolution (rf_num_points, image_vector, xi[i], yi[i],
										wi, hi, global_factor, teste) / 6.0;
	}
}

// O filtro CUDA_BIOLOGICAL_GABOR_NLS eh a chamada de kernel do filtro V1_MT
void 
cuda_biological_gabor_nls (float *d_s_m, int *d_xi, int *d_yi, int *d_image_vector, int *xi, int *yi, int wi, int hi,
				int num_neurons, int rf_num_points, int *rf_points_x, int *rf_points_y, int *image_vector,
				float *rf_points_w, float *rf_points_g, float global_factor, float c_delta_area, float teste)
{
	int dimension = wi*hi;

	cudaMemcpy (d_image_vector, image_vector, dimension * sizeof (int));

	d_rf_points_x = (int *) malloc (rf_num_points * sizeof (int));
	d_rf_points_y = (int *) malloc (rf_num_points * sizeof (int));
	d_rf_points_w = (float *) malloc (rf_num_points * sizeof (float));
	d_rf_points_g = (float *) malloc (rf_num_points * sizeof (float));

	cudaMemcpy (d_rf_points_x, rf_points_x, rf_num_points * sizeof (int));
	cudaMemcpy (d_rf_points_y, rf_points_y, rf_num_points * sizeof (int));
	cudaMemcpy (d_rf_points_w, rf_points_w, rf_num_points * sizeof (float));
	cudaMemcpy (d_rf_points_g, rf_points_g, rf_num_points * sizeof (float));

	cuda_biological_gabor (d_s_m, d_xi, d_yi, wi, hi, num_neurons, rf_num_points, d_image_vector,
								global_factor, c_delta_area, teste);

	return;
}

// O filtro CUDA_BIOLOGICAL_GABOR_NLS_OLD so pode ser chamado individualmente, na estrutura original
void
cuda_biological_gabor_nls_old (float *s_m, int *xi, int *yi, int wi, int hi, int size, int rf_num_points, int *rf_points_x,
				int *rf_points_y, int *image_vector, float *rf_points_w,
				float *rf_points_g, float global_factor, float c_delta_area, float teste)
{
	int  *d_xi, *d_yi, *d_image_vector;
	float *d_s_m;
	int dimension = wi*hi;

	d_image_vector 	= (int *) malloc (dimension * sizeof (int));
	d_xi 		= (int *) malloc (size * sizeof (int));
	d_yi 		= (int *) malloc (size * sizeof (int));

	cudaMemcpy (d_image_vector, image_vector, dimension * sizeof (int));
	cudaMemcpy (d_xi, xi, size * sizeof (int));
	cudaMemcpy (d_yi, yi, size * sizeof (int));

	d_rf_points_x = (int *) malloc (rf_num_points * sizeof (int));
	d_rf_points_y = (int *) malloc (rf_num_points * sizeof (int));
	d_rf_points_w = (float *) malloc (rf_num_points * sizeof (float));
	d_rf_points_g = (float *) malloc (rf_num_points * sizeof (float));

	cudaMemcpy (d_rf_points_x, rf_points_x, rf_num_points * sizeof (int));
	cudaMemcpy (d_rf_points_y, rf_points_y, rf_num_points * sizeof (int));
	cudaMemcpy (d_rf_points_w, rf_points_w, rf_num_points * sizeof (float));
	cudaMemcpy (d_rf_points_g, rf_points_g, rf_num_points * sizeof (float));

	d_s_m = (float *) malloc (size * sizeof (int));

	cuda_biological_gabor (d_s_m, d_xi, d_yi, wi, hi, size, rf_num_points, d_image_vector, global_factor, c_delta_area, teste);

	cudaMemcpy (s_m, d_s_m, size * sizeof (float));

	cudaFree (d_s_m);
	cudaFree (d_image_vector);
	cudaFree (d_xi);
	cudaFree (d_yi);

	return;
}



/*!
*********************************************************************************
* Function: cuda_add_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel CUDA_ADD
void
cuda_add (float *d_C, float *d_A, float *d_B, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		d_C[i] = d_A[i] + d_B[i];
	}
}

// O filtro CUDA_ADD_NLS so pode ser chamado individualmente, na estrutura original
void
cuda_add_nls (NEURON *C, NEURON *A, NEURON *B, int size)
{
	float *h_A, *h_B, *h_C, *d_A, *d_B, *d_C;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));

	d_A = (float *) malloc (size * sizeof (float));
	d_B = (float *) malloc (size * sizeof (float));
	d_C = (float *) malloc (size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float));
	cudaMemcpy (d_B, h_B, size * sizeof (float));

	cuda_add (d_C, d_A, d_B, size);

	cudaMemcpy (h_C, d_C, size * sizeof (float));

	copyfloatvec2neuron (C, h_C, size);

	cudaFree (d_A);
	cudaFree (d_B);
	cudaFree (d_C);

	return;
}



/*!
*********************************************************************************
* Function: cuda_mult_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel CUDA_MULT_ADD
void
cuda_mult_add (float *d_C, float *d_A, float *d_B, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		d_C[i] = ((d_A[i])*(d_A[i])) + ((d_B[i])*(d_B[i]));
	}
}

// O filtro CUDA_MULT_NLS so pode ser chamado individualmente, na estrutura original
void
cuda_mult_nls (NEURON *C, NEURON *A, NEURON *B, int size)
{
	float *h_A, *h_B, *h_C, *d_A, *d_B, *d_C;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));

	d_A = (float *) malloc (size * sizeof (float));
	d_B = (float *) malloc (size * sizeof (float));
	d_C = (float *) malloc (size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float));
	cudaMemcpy (d_B, h_B, size * sizeof (float));

	cuda_mult_add (d_C, d_A, d_B, size);

	cudaMemcpy (h_C, d_C, size * sizeof (float));

	copyfloatvec2neuron (C, h_C, size);

	cudaFree (d_A);
	cudaFree (d_B);
	cudaFree (d_C);

	return;
}



/*!
*********************************************************************************
* Function: cuda_div_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel CUDA_DIV
void
cuda_div (float *d_D, float *d_A, float *d_B, float *d_C, int size, float k_param)
{
	int i;

	for (i = 0; i < size; i++)
	{
		d_D[i] = d_C[i] / (d_A[i] + d_B[i] + k_param);
	}
}

// O filtro CUDA_DIV_NLS so pode ser chamado individualmente, na estrutura original
void
cuda_div_nls (NEURON *D, NEURON *A, NEURON *B, NEURON *C, int size, float k_param)
{
	float *h_A, *h_B, *h_C, *h_D, *d_A, *d_B, *d_C, *d_D;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));
	h_D = (float *) malloc (size * sizeof (float));

	d_A = (float *) malloc (size * sizeof (float));
	d_B = (float *) malloc (size * sizeof (float));
	d_C = (float *) malloc (size * sizeof (float));
	d_D = (float *) malloc (size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);
	copyneuron2floatvec (h_C, C, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float));
	cudaMemcpy (d_B, h_B, size * sizeof (float));
	cudaMemcpy (d_C, h_C, size * sizeof (float));

	cuda_div (d_D, d_A, d_B, d_C, size, k_param);

	cudaMemcpy (h_D, d_D, size * sizeof (float));

	copyfloatvec2neuron (D, h_D, size);

	cudaFree (d_A);
	cudaFree (d_B);
	cudaFree (d_C);
	cudaFree (d_D);

	return;
}



/*!
*********************************************************************************
* Function: cuda_bigfilter_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel CUDA_BIGFILTER
void
bigfilter (float *d_mt, float *d_s_r, float *d_s_r_q, float *d_s_l, float *d_s_l_q, int num_neurons, float k)
{
	int i;
	float s_lr, s_lr_q, c_l, c_r, c_lr;

	for (i = 0; i < num_neurons; i++)
	{
		s_lr   = d_s_l[i] + d_s_r[i];
		s_lr_q = d_s_l_q[i] + d_s_r_q[i];

		c_l = d_s_l[i] * d_s_l[i] + d_s_l_q[i] * d_s_l_q[i];
		c_r = d_s_r[i] * d_s_r[i] + d_s_r_q[i] * d_s_r_q[i];

		c_lr = s_lr * s_lr + s_lr_q * s_lr_q;

		d_mt[i] = c_lr / (c_l + c_r + k);
	}
}

// O filtro CUDA_BIGFILTER_NLS eh a chamada de kernel do filtro V1_MT
void
cuda_bigfilter_nls (float *d_mt, float *d_s_r, float *d_s_r_q, float *d_s_l, float *d_s_l_q, int num_neurons, float k)
{
	bigfilter (d_mt, d_s_r, d_s_r_q, d_s_l, d_s_l_q, num_neurons, k);

	return;
}

// O filtro CUDA_BIGFILTER_NLS_OLD so pode ser chamado individualmente, na estrutura original
void
cuda_bigfilter_nls_old (float *mt, float *s_r, float *s_r_q, float *s_l, float *s_l_q, int num_neurons, float k)
{
	float *d_mt, *d_s_r, *d_s_r_q, *d_s_l, *d_s_l_q;

	d_mt    = (float *) malloc (num_neurons * sizeof (float));
	d_s_r   = (float *) malloc (num_neurons * sizeof (float));
	d_s_r_q = (float *) malloc (num_neurons * sizeof (float));
	d_s_l   = (float *) malloc (num_neurons * sizeof (float));
	d_s_l_q = (float *) malloc (num_neurons * sizeof (float));

	cudaMemcpy (d_s_r, s_r, num_neurons * sizeof (float));
	cudaMemcpy (d_s_r_q, s_r_q, num_neurons * sizeof (float));
	cudaMemcpy (d_s_l, s_l, num_neurons * sizeof (float));
	cudaMemcpy (d_s_l_q, s_l_q, num_neurons * sizeof (float));

	bigfilter (d_mt, d_s_r, d_s_r_q, d_s_l, d_s_l_q, num_neurons, k);

	cudaMemcpy (mt, d_mt, num_neurons * sizeof (float));

	cudaFree (d_mt);
	cudaFree (d_s_r);
	cudaFree (d_s_r_q);
	cudaFree (d_s_l);
	cudaFree (d_s_l_q);

	return;
}



/*!
*********************************************************************************
* Function: cuda_gaussian_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Device CUDA_GAUSSIAN
float
cuda_apply_gaussian_kernel (float *mt, int num_points, int x0, int y0, int wi, int hi)
{
	int i;
	int xr, yr;
	float fltWeight, fltWeightSum = .0f, fltResult = .0f;

	for (i = 0; i < num_points; i++)
	{
		xr = x0 + d_rf_points_x[i];
		yr = y0 + d_rf_points_y[i];

		if ((xr < 0) || (xr >= wi) || (yr < 0) || (yr >= hi))
			continue;

		fltWeightSum += fltWeight = d_rf_points_g[i];
		fltResult += fltWeight * mt[yr * wi + xr];
	}

	fltResult = (fltWeightSum != .0f) ? fltResult / fltWeightSum : .0f;

	return (fltResult);
}

// Kernel CUDA_GAUSSIAN
void
cuda_gaussian (float *mt_gaussian, float *mt, int num_points, int wo, int ho, int wi, int hi)
{
	int i, j;
	for (i = 0; i < wo; i++)
	{
		for (j = 0; j < ho; j++)
		{
			mt_gaussian[j * wo + i] = cuda_apply_gaussian_kernel (mt, num_points, i, j, wo, ho);
		}
	}
}

// O filtro CUDA_GAUSSIAN_NLS eh a chamada de kernel do filtro V1_MT
void
cuda_gaussian_nls (float *mt_gaussian, float *d_mt_gaussian, float *d_mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi)
{
	cudaMemcpy (d_rf_points_x, rf_points_x, num_points * sizeof (int));
	cudaMemcpy (d_rf_points_y, rf_points_y, num_points * sizeof (int));
	cudaMemcpy (d_rf_points_g, rf_points_g, num_points * sizeof (float));

	cuda_gaussian (d_mt_gaussian, d_mt, num_points, wo, ho, wi, hi);

	cudaMemcpy (mt_gaussian, d_mt_gaussian, wo*ho * sizeof (float));

	return;
}

// O filtro CUDA_GAUSSIAN_NLS_OLD so pode ser chamado individualmente, na estrutura original
void
cuda_gaussian_nls_old (float *mt_gaussian, float *mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi)
{
	float *d_mt, *d_mt_gaussian;

	d_mt = (float *) malloc (wi*hi * sizeof (float));
	cudaMemcpy (d_mt, mt, wi*hi * sizeof (float));

	cudaMemcpy (d_rf_points_x, rf_points_x, num_points * sizeof (int));
	cudaMemcpy (d_rf_points_y, rf_points_y, num_points * sizeof (int));
	cudaMemcpy (d_rf_points_g, rf_points_g, num_points * sizeof (float));

	d_mt_gaussian = (float *) malloc (wo*ho * sizeof (float));

	cuda_gaussian (d_mt_gaussian, d_mt, num_points, wo, ho, wi, hi);

	cudaMemcpy (mt_gaussian, d_mt_gaussian, wo*ho * sizeof (float));

	cudaFree (d_mt_gaussian);
	cudaFree (d_mt);

	return;
}
