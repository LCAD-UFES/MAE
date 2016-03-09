/*!
********************************************************************************
********************************* CUDA SESSION *********************************
******************* Camilo A. Carvalho & Alberto F. De Souza *******************
********************** {camilo, alberto}@lcad.inf.ufes.br **********************
*********************************** AGO/2009 ***********************************
********************************************************************************
*/

#include <stdio.h>
#include <cuda.h>
#include <stdlib.h>
#include "mae.h"
#include "filter.h"
#include "cuda_functions.h"


const int BLOCKSIZE = 64;


/*!
Copias de dados - de um NEURON_VECTOR para vetor float simples, e vice-versa
*/
extern "C" void
copyneuron2floatvec (float *float_vec, NEURON *neuron_vec, int size)
{
	int i;

	for (i = 0; i < size; i++)
		float_vec[i] = neuron_vec[i].output.fval;

	return;
}

extern "C" void
copyfloatvec2neuron (NEURON *neuron_vec, float *float_vec, int size)
{
	int i;

	for (i = 0; i < size; i++)
		neuron_vec[i].output.fval = float_vec[i];

	return;
}

/*!
Soma em arvore - retornar um valor simples de uma soma de elementos de um vetor
*/
__device__ void 
sum_tree_like_reduction (float *v, int size)
{
	int j, k;
	j=threadIdx.x;
	for(k = size / 2; k > 0; k >>= 1)
	{
		__syncthreads();

		v[j] += ( j < k )? v[k + j] : 0;
			
	}
}

/*!
Aloca as areas de memoria dos elementos de um PRIVATE_STATE na GPU
*/
extern "C" void 
cuda_alloc_device_data_structures (V1_MT_PRIVATE_STATE *v1_mt_private_state, int wi, int hi, int num_neurons)
{
	int dimension = wi*hi;

	cudaMalloc ((void **) &(v1_mt_private_state->d_image_vector), dimension * sizeof (int));
	cudaMalloc ((void **) &(v1_mt_private_state->d_xi), num_neurons * sizeof (int));
	cudaMalloc ((void **) &(v1_mt_private_state->d_yi), num_neurons * sizeof (int));

	cudaMalloc ((void **) &(v1_mt_private_state->d_s_r), num_neurons * sizeof (float));
	cudaMalloc ((void **) &(v1_mt_private_state->d_s_r_q), num_neurons * sizeof (float));
	cudaMalloc ((void **) &(v1_mt_private_state->d_s_l), num_neurons * sizeof (float));
	cudaMalloc ((void **) &(v1_mt_private_state->d_s_l_q), num_neurons * sizeof (float));

	cudaMalloc ((void **) &(v1_mt_private_state->d_mt), num_neurons * sizeof (float));

	cudaMalloc ((void **) &(v1_mt_private_state->d_mt_gaussian), num_neurons * sizeof (float));
}



/*!
*********************************************************************************
* Function: map_v1
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Kernel MAP_V1
__global__ void
map_v1 (int *xi, int *yi, int wi, int hi, int w, int h, int x_center, int y_center, float correction, float log_factor, int shift)
{
	float CUDA_LOG_POLAR_SCALE_FACTOR = 1.0;
	float CUDA_LOG_POLAR_THETA_CORRECTION = 0.0;
	float d, theta, exp_val, x;
	int i;

	int u = 0, v = 0;
	int adjust;

	for (v = blockIdx.x; v < h; v += gridDim.x)
	{
		for (u = threadIdx.x; u < w; u += blockDim.x)
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
extern "C" void 
cuda_map_v1_to_image (int *xi, int *yi, int wi, int hi, int w, int h, int x_center, int y_center, float correction, float log_factor, int shift)
{
	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid (((w*h)/dimBlock.x) + (!((w*h)%dimBlock.x)?0:1));

	map_v1 <<< dimGrid , dimBlock >>> (xi, yi, wi, hi, w, h, x_center, y_center, correction, log_factor, shift);
}



/*!
*********************************************************************************
* Function: cuda_biological_gabor_nls
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

// Alocacao de areas na memoria de constantes dos vetores dos campos receptivos
__constant__ int d_rf_points_x[256];
__constant__ int d_rf_points_y[256];
__constant__ float d_rf_points_w[256];
__constant__ float d_rf_points_g[256];

// Device CUDA_BIDIMENTIONAL_CONVOLUTION
__device__ float 
cuda_bidimentional_convolution (int rf_num_points, int *image_vector, int x_center, int y_center, int w, int h,
				float p_global_factor, float p_teste)
{
	int x_current, y_current, i, pixel;
	float intensity, red, green, blue;
	float accumulator=0.0f;

	// Initializes the accumulator variable
	//accumulator[threadIdx.x] = 0.0;

        for (i = 0; i < rf_num_points; i ++)
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

	//sum_tree_like_reduction (accumulator, BLOCKSIZE);

	//__syncthreads();

	return (accumulator);
}


// Kernel CUDA_BIOLOGICAL_GABOR
__global__ void
cuda_biological_gabor (float *s_m, int *xi, int *yi, int wi, int hi, int size, int rf_num_points, int *image_vector,
			float global_factor, float c_delta_area, float teste)
{
	//int offset = gridDim.x*blockDim.x;
	int i = blockDim.x*blockIdx.x+threadIdx.x;
	
	//for (int i = tid; i < size; i += offset)
	if(i<size)
	{
		s_m[i] = c_delta_area * cuda_bidimentional_convolution (rf_num_points, image_vector, xi[i], yi[i],
									wi, hi, global_factor, teste) / 6.0;
	}
}



// O filtro CUDA_BIOLOGICAL_GABOR_NLS eh a chamada de kernel do filtro V1_MT
extern "C" void 
cuda_biological_gabor_nls (float *d_s_m, int *d_xi, int *d_yi, int *d_image_vector, int *xi, int *yi, int wi, int hi,
				int num_neurons, int rf_num_points, int *rf_points_x, int *rf_points_y, int *image_vector,
				float *rf_points_w, float *rf_points_g, float global_factor, float c_delta_area, float teste)
{
	int dimension = wi*hi;

	cudaMemcpy (d_image_vector, image_vector, dimension * sizeof (int), cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol (d_rf_points_x, rf_points_x, rf_num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_y, rf_points_y, rf_num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_w, rf_points_w, rf_num_points * sizeof (float));
	cudaMemcpyToSymbol (d_rf_points_g, rf_points_g, rf_num_points * sizeof (float));

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((num_neurons/dimBlock.x) + (!(num_neurons%dimBlock.x)?0:1));

	cuda_biological_gabor <<< dimGrid , dimBlock >>> (d_s_m, d_xi, d_yi, wi, hi, num_neurons, rf_num_points, d_image_vector,
								global_factor, c_delta_area, teste);

	return;
}


// O filtro CUDA_BIOLOGICAL_GABOR_NLS_OLD so pode ser chamado individualmente, na estrutura original
extern "C" void 
cuda_biological_gabor_nls_old (float *s_m, int *xi, int *yi, int wi, int hi, int num_neurons, int rf_num_points,
				int *rf_points_x, int *rf_points_y, int *image_vector, float *rf_points_w, float *rf_points_g,
				float global_factor, float c_delta_area, float teste)
{
	int  *d_xi, *d_yi, *d_image_vector;
	float *d_s_m;
	int dimension = wi*hi;

	cudaMalloc ((void **) &d_image_vector, dimension * sizeof (int));
	cudaMalloc ((void **) &d_xi, num_neurons * sizeof (int));
	cudaMalloc ((void **) &d_yi, num_neurons * sizeof (int));

	cudaMemcpy (d_image_vector, image_vector, dimension * sizeof (int), cudaMemcpyHostToDevice);
	cudaMemcpy (d_xi, xi, num_neurons * sizeof (int), cudaMemcpyHostToDevice);
	cudaMemcpy (d_yi, yi, num_neurons * sizeof (int), cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol (d_rf_points_x, rf_points_x, rf_num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_y, rf_points_y, rf_num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_w, rf_points_w, rf_num_points * sizeof (float));
	cudaMemcpyToSymbol (d_rf_points_g, rf_points_g, rf_num_points * sizeof (float));

	cudaMalloc ((void **) &d_s_m, num_neurons * sizeof (float));

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid (num_neurons);

	cuda_biological_gabor <<< dimGrid , dimBlock >>> (d_s_m, d_xi, d_yi, wi, hi, num_neurons, rf_num_points, d_image_vector,
								global_factor, c_delta_area, teste);

	cudaMemcpy (s_m, d_s_m, num_neurons * sizeof (float), cudaMemcpyDeviceToHost);

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
__global__ void
cuda_add (float *d_C, float *d_A, float *d_B, int size)
{

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if ( idx < size )
		d_C[idx] = d_A[idx] + d_B[idx];

}

// O filtro CUDA_ADD_NLS so pode ser chamado individualmente, na estrutura original
extern "C" void
cuda_add_nls (NEURON *C, NEURON *A, NEURON *B, int size)
{

	float *h_A, *h_B, *h_C;
	float *d_A, *d_B, *d_C;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));

	cudaMalloc ((void **) &d_A, size * sizeof (float));
	cudaMalloc ((void **) &d_B, size * sizeof (float));
	cudaMalloc ((void **) &d_C, size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_B, h_B, size * sizeof (float), cudaMemcpyHostToDevice);

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((size/dimBlock.x) + (!(size%dimBlock.x)?0:1));

	cuda_add <<< dimGrid , dimBlock >>> (d_C, d_A, d_B, size);

	cudaMemcpy(h_C, d_C, size * sizeof (float), cudaMemcpyDeviceToHost);

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
__global__ void
cuda_mult_add (float *d_C, float *d_A, float *d_B, int size)
{

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if ( idx < size )
		d_C[idx] = (d_A[idx] * d_A[idx]) + (d_B[idx] * d_B[idx]);

}

// O filtro CUDA_MULT_NLS so pode ser chamado individualmente, na estrutura original
extern "C" void
cuda_mult_nls (NEURON *C, NEURON *A, NEURON *B, int size)
{
	float *h_A, *h_B, *h_C;
	float *d_A, *d_B, *d_C;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));

	cudaMalloc ((void **) &d_A, size * sizeof (float));
	cudaMalloc ((void **) &d_B, size * sizeof (float));
	cudaMalloc ((void **) &d_C, size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_B, h_B, size * sizeof (float), cudaMemcpyHostToDevice);

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((size/dimBlock.x) + (!(size%dimBlock.x)?0:1));

	cuda_mult_add <<< dimGrid , dimBlock >>> (d_C, d_A, d_B, size);

	cudaMemcpy(h_C, d_C, size * sizeof (float), cudaMemcpyDeviceToHost);

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
__global__ void
cuda_div (float *d_D, float *d_A, float *d_B, float *d_C, int size, float k_param)
{

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if ( idx < size )
		d_D[idx] = d_C[idx] / (d_A[idx] + d_B[idx] + k_param);

}

// O filtro CUDA_DIV_NLS so pode ser chamado individualmente, na estrutura original
extern "C" void
cuda_div_nls (NEURON *D, NEURON *A, NEURON *B, NEURON *C, int size, float k_param)
{
	float *h_A, *h_B, *h_C, *h_D;
	float *d_A, *d_B, *d_C, *d_D;

	h_A = (float *) malloc (size * sizeof (float));
	h_B = (float *) malloc (size * sizeof (float));
	h_C = (float *) malloc (size * sizeof (float));
	h_D = (float *) malloc (size * sizeof (float));

	cudaMalloc ((void **) &d_A, size * sizeof (float));
	cudaMalloc ((void **) &d_B, size * sizeof (float));
	cudaMalloc ((void **) &d_C, size * sizeof (float));
	cudaMalloc ((void **) &d_D, size * sizeof (float));

	copyneuron2floatvec (h_A, A, size);
	copyneuron2floatvec (h_B, B, size);
	copyneuron2floatvec (h_C, C, size);

	cudaMemcpy (d_A, h_A, size * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_B, h_B, size * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_C, h_C, size * sizeof (float), cudaMemcpyHostToDevice);

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((size/dimBlock.x) + (!(size%dimBlock.x)?0:1));

	cuda_div <<< dimGrid , dimBlock >>> (d_D, d_A, d_B, d_C, size, k_param);

	cudaMemcpy (h_D, d_D, size * sizeof (float), cudaMemcpyDeviceToHost);

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
__global__ void
cuda_bigfilter (float *mt, float *s_r, float *s_r_q, float *s_l, float *s_l_q, int num_neurons, float k)
{
	float s_lr, s_lr_q, c_l, c_r, c_lr;

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if ( idx < num_neurons )
	{

		s_lr = s_l[idx] + s_r[idx];
		s_lr_q = s_l_q[idx] + s_r_q[idx];

		c_l = (s_l[idx] * s_l[idx]) + (s_l_q[idx] * s_l_q[idx]);
		c_r = (s_r[idx] * s_r[idx]) + (s_r_q[idx] * s_r_q[idx]);
		c_lr = (s_lr * s_lr) + (s_lr_q * s_lr_q);

		mt[idx] = c_lr / (c_l + c_r + k);
	}
}

// O filtro CUDA_BIGFILTER_NLS eh a chamada de kernel do filtro V1_MT
extern "C" void
cuda_bigfilter_nls (float *d_mt, float *d_s_r, float *d_s_r_q, float *d_s_l, float *d_s_l_q, int num_neurons, float k)
{
	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((num_neurons/dimBlock.x) + (!(num_neurons%dimBlock.x)?0:1));

	cuda_bigfilter <<< dimGrid , dimBlock >>> (d_mt, d_s_r, d_s_r_q, d_s_l, d_s_l_q, num_neurons, k);

	return;
}

// O filtro CUDA_BIGFILTER_NLS_OLD so pode ser chamado individualmente, na estrutura original
extern "C" void
cuda_bigfilter_nls_old (float *mt, float *s_r, float *s_r_q, float *s_l, float *s_l_q, int num_neurons, float k)
{
	float *d_mt, *d_s_r, *d_s_r_q, *d_s_l, *d_s_l_q;

	cudaMalloc ((void **) &d_mt, num_neurons * sizeof (float));
	cudaMalloc ((void **) &d_s_r, num_neurons * sizeof (float));
	cudaMalloc ((void **) &d_s_r_q, num_neurons * sizeof (float));
	cudaMalloc ((void **) &d_s_l, num_neurons * sizeof (float));
	cudaMalloc ((void **) &d_s_l_q, num_neurons * sizeof (float));

	cudaMemcpy (d_s_r, s_r, num_neurons * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_s_r_q, s_r_q, num_neurons * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_s_l, s_l, num_neurons * sizeof (float), cudaMemcpyHostToDevice);
	cudaMemcpy (d_s_l_q, s_l_q, num_neurons * sizeof (float), cudaMemcpyHostToDevice);

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid ((num_neurons/dimBlock.x) + (!(num_neurons%dimBlock.x)?0:1));

	cuda_bigfilter <<< dimGrid , dimBlock >>> (d_mt, d_s_r, d_s_r_q, d_s_l, d_s_l_q, num_neurons, k);

	cudaMemcpy (mt, d_mt, num_neurons * sizeof (float), cudaMemcpyDeviceToHost);

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
// __device__ float
// cuda_apply_gaussian_kernel (float *mt, int num_points, int x0, int y0, int wi, int hi)
// {
// 	int i, j, k;
// 	int xr, yr;
// 	float fltWeight;
// 
// 	__shared__ float fltWeightSum[BLOCKSIZE];
// 	__shared__ float fltResult[BLOCKSIZE];
// 
// 	fltWeightSum[threadIdx.x] = 0.0f;
// 	fltResult[threadIdx.x] = 0.0f;
// 
// 	for (i = threadIdx.x; i < num_points; i += blockDim.x)
// 	{
// 		xr = x0 + d_rf_points_x[i];
// 		yr = y0 + d_rf_points_y[i];
// 
// 		if ((xr < 0) || (xr >= wi) || (yr < 0) || (yr >= hi))
// 			continue;
// 
// 		fltWeightSum[threadIdx.x] += fltWeight = d_rf_points_g[i];
// 		fltResult[threadIdx.x] += fltWeight * mt[yr * wi + xr];
// 	}
// 
// 	for (k = BLOCKSIZE / 2; k > 0; k >>=1)
// 	{
// 		__syncthreads();
// 
// 		for (j = threadIdx.x; j < k; j += blockDim.x)
// 		{
// 			fltWeightSum[j] += fltWeightSum[k + j];
// 			fltResult[j] += fltResult[k + j];
// 		}
// 	}
// 
// 	__syncthreads();
// 
// 	return ((fltWeightSum[0] != .0f) ? fltResult[0] / fltWeightSum[0] : .0f);
// }

__device__ float
cuda_apply_gaussian_kernel (float *mt, int num_points, int x0, int y0, int wi, int hi)
{
	int i, k;
	int xr, yr;
	float fltWeight;

	__shared__ float fltWeightSum[BLOCKSIZE];
	__shared__ float fltResult[BLOCKSIZE];

	fltWeightSum[threadIdx.x] = 0.0f;
	fltResult[threadIdx.x] = 0.0f;

	for (i = threadIdx.x; i < num_points; i += blockDim.x)
	{
		xr = x0 + d_rf_points_x[i];
		yr = y0 + d_rf_points_y[i];

		if ((xr < 0) || (xr >= wi) || (yr < 0) || (yr >= hi))
			continue;

		fltWeightSum[threadIdx.x] += fltWeight = d_rf_points_g[i];
		fltResult[threadIdx.x] += fltWeight * mt[yr * wi + xr];
	}
	i = threadIdx.x;
	for (k = BLOCKSIZE / 2; k > 0; k >>=1)
	{
		__syncthreads();

		if( i < k )
		{
			fltWeightSum[i] += fltWeightSum[k + i];
			fltResult[i] += fltResult[k + i];
		}
	}

	__syncthreads();

	return ((fltWeightSum[0] != .0f) ? fltResult[0] / fltWeightSum[0] : .0f);
}

// Kernel CUDA_GAUSSIAN
__global__ void
cuda_gaussian (float *mt_gaussian, float *mt, int num_points, int wo, int ho, int wi, int hi)
{
	for (int i = blockIdx.x; i < wo; i += gridDim.x)
	{
		for (int j = blockIdx.y; j < ho; j += gridDim.y)
		{
			mt_gaussian[j * wo + i] = cuda_apply_gaussian_kernel (mt, num_points, i, j, wo, ho);
		}
	}
}

// O filtro CUDA_GAUSSIAN_NLS eh a chamada de kernel do filtro V1_MT
extern "C" void
cuda_gaussian_nls (float *mt_gaussian, float *d_mt_gaussian, float *d_mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi)
{
	cudaMemcpyToSymbol (d_rf_points_x, rf_points_x, num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_y, rf_points_y, num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_g, rf_points_g, num_points * sizeof (float));

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid (wo,ho);

	cuda_gaussian <<< dimGrid , dimBlock >>> (d_mt_gaussian, d_mt, num_points, wo, ho, wi, hi);

	cudaMemcpy (mt_gaussian, d_mt_gaussian, wo*ho * sizeof (float), cudaMemcpyDeviceToHost);
}

// O filtro CUDA_GAUSSIAN_NLS_OLD so pode ser chamado individualmente, na estrutura original
extern "C" void
cuda_gaussian_nls_old (float *mt_gaussian, float *mt, int *rf_points_x, int *rf_points_y, 
		  float *rf_points_g, int num_points, int wo, int ho, int wi, int hi)
{
	float *d_mt, *d_mt_gaussian;

	cudaMemcpyToSymbol (d_rf_points_x, rf_points_x, num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_y, rf_points_y, num_points * sizeof (int));
	cudaMemcpyToSymbol (d_rf_points_g, rf_points_g, num_points * sizeof (float));

	cudaMalloc ((void **) &d_mt, wi*hi * sizeof (float));
	cudaMemcpy (d_mt, mt, wi*hi * sizeof (float), cudaMemcpyHostToDevice);

	cudaMalloc ((void **) &d_mt_gaussian, wo*ho * sizeof (float));

	dim3 dimBlock (BLOCKSIZE);
	dim3 dimGrid (wo,ho);

	cuda_gaussian <<< dimGrid , dimBlock >>> (d_mt_gaussian, d_mt, num_points, wo, ho, wi, hi);

	cudaMemcpy (mt_gaussian, d_mt_gaussian, wo*ho * sizeof (float), cudaMemcpyDeviceToHost);

	cudaFree (d_mt_gaussian);
	cudaFree (d_mt);
}
