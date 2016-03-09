#include "robot_nl_functions.h"
#include "robot.h"
#include "stereo_api.h"

/*
Seção de Constantes
*/

#define NUM_SAMPLES				5

/*
Seção de Variáveis de Módulo
*/

FILTER_DESC* nl_simple_mono_right_filter;

double g_dblNearVergenceCutPlane = NEAREST_DISTANCE;

double g_dblFarVergenceCutPlane = FAREST_DISTANCE;

int g_nCurrDisparity;

/*
Seção de Funções
*/

/************************************************************************/
/* Name: 								*/
/* Description:								*/
/* Inputs: 								*/
/* Output: 								*/
/************************************************************************/

void 
robot_check_input_bounds (INPUT_DESC *input, int wx, int wy)
{
	int redraw = 0;
	
	if (input->wx < 0)
	{
		if (input->vpxo > 0)
		{
			input->vpxo += input->wx;
			if (input->vpxo < 0)
				input->vpxo = 0;
			redraw = 1;
		} 
		input->wx = 0;
	}
	else if ((input->wx+input->neuron_layer->dimentions.x) >= input->vpw)
	{
		if (input->vpxo+input->vpw < input->ww)
		{
			input->vpxo += (input->wx+input->neuron_layer->dimentions.x) - input->vpw;
			if (input->vpxo+input->vpw > input->ww)
				input->vpxo = input->ww - input->vpw;
			redraw = 1;
		} 
		input->wx = input->vpw - input->neuron_layer->dimentions.x;
	}


	if (input->wy < 0)
	{
		if (input->vpyo+input->vph < input->wh)
		{
			input->vpyo -= input->wy;
			if (input->vpyo+input->vph > input->wh)
				input->vpyo = input->wh - input->vph;
			redraw = 1;
		} 
		input->wy = 0;
	}
	else if ((input->wy+input->neuron_layer->dimentions.y) >= input->vph)
	{
		if (input->vpyo > 0)
		{
			input->vpyo -= (input->wy+input->neuron_layer->dimentions.y) - input->vph;
			if (input->vpyo < 0)
				input->vpyo = 0;
			redraw = 1;
		} 
		input->wy = input->vph - input->neuron_layer->dimentions.y;
	}

	if (redraw && (input->waiting_redisplay == 0))
	{
		glutSetWindow (input->win);
		glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, 
			input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
		input->waiting_redisplay = 1;
	}
}

// ----------------------------------------------------------------------------
// rectify_image - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
rectify_image ()
{
	int i, j, w, h;
	static unsigned char *p_cUndistortedImage = NULL;
	static unsigned char *p_cDistortedImage = NULL;
	time_t start, end;

	start = time (NULL);

	w = image_left.ww;
	h = image_left.wh;

	p_cDistortedImage = (unsigned char*) malloc (3 * w * h * sizeof (unsigned char));
	p_cUndistortedImage = (unsigned char*) malloc (3 * w * h * sizeof (unsigned char));

	//Rectify left image
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			p_cDistortedImage[(h - j - 1) + i * h + 0 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 0];
			p_cDistortedImage[(h - j - 1) + i * h + 1 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 1];
			p_cDistortedImage[(h - j - 1) + i * h + 2 * w * h] = image_left.image[3 * (i + j * image_left.tfw) + 2];			
		}	
	}

	RectifyLeftImage (p_cUndistortedImage, p_cDistortedImage, w, h);

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			image_left.image[3 * (i + j * image_left.tfw) + 0] = p_cUndistortedImage[(h - j - 1) + i * h + 0 * w * h];
			image_left.image[3 * (i + j * image_left.tfw) + 1] = p_cUndistortedImage[(h - j - 1) + i * h + 1 * w * h];
			image_left.image[3 * (i + j * image_left.tfw) + 2] = p_cUndistortedImage[(h - j - 1) + i * h + 2 * w * h];
		}
	}

	//Rectify right image
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			p_cDistortedImage[(h - j - 1) + i * h + 0 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 0];
			p_cDistortedImage[(h - j - 1) + i * h + 1 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 1];
			p_cDistortedImage[(h - j - 1) + i * h + 2 * w * h] = image_right.image[3 * (i + j * image_right.tfw) + 2];			
		}	
	}

	RectifyRightImage (p_cUndistortedImage, p_cDistortedImage, w, h);

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			image_right.image[3 * (i + j * image_right.tfw) + 0] = p_cUndistortedImage[(h - j - 1) + i * h + 0 * w * h];
			image_right.image[3 * (i + j * image_right.tfw) + 1] = p_cUndistortedImage[(h - j - 1) + i * h + 1 * w * h];
			image_right.image[3 * (i + j * image_right.tfw) + 2] = p_cUndistortedImage[(h - j - 1) + i * h + 2 * w * h];
        	}
	}
      
	update_input_neurons (&(image_left));
	robot_check_input_bounds (&(image_left), image_left.wxd, image_left.wyd);
	update_input_image (&(image_left));

	update_input_neurons (&(image_right));
	robot_check_input_bounds (&(image_right), image_right.wxd, image_right.wyd);
	update_input_image (&(image_right));
			
	end = time (NULL);
	printf ("Time to rectify image: %f s\n", difftime (end, start));

	free(p_cDistortedImage);
	free(p_cUndistortedImage);
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void 
update_simple_mono_cells_filters ()
{
	FILTER_LIST *f_list;

	//Move to center of the image
	image_left.wxd = ((double)image_left.ww)/2.0;
	image_left.wyd = ((double)image_left.wh)/2.0;

	//Move to center of the image
	image_right.wxd = ((double)image_right.ww)/2.0;
	image_right.wyd = ((double)image_right.wh)/2.0;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next)
	{
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_filter") == 0)
		{
			nl_simple_mono_right_filter = f_list->filter_descriptor;
			filter_update (nl_simple_mono_right_filter);
		}

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_q_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_q_filter") == 0)
			filter_update (f_list->filter_descriptor);
			
		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_right_q_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_simple_mono_left_q_subpixel_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_illuminance_right_filter") == 0)
		{
			int i;
			for(i = 0; i < NUMBER_ILLUMINANCE_ITERACTIONS; i++)
				filter_update (f_list->filter_descriptor);
		}
			
		if (strcmp(f_list->filter_descriptor->name, "nl_reflectance_right_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_trunk_segmentation_map_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_vertical_segmentation_map_filter") == 0)
			filter_update (f_list->filter_descriptor);
	}
}

// ----------------------------------------------------------------------------
// filters_update_by_name - Update simple mono cells
//
// Inputs:
//
// Output:
// ----------------------------------------------------------------------------
void 
update_mt_cells_filters ()
{
	FILTER_LIST *f_list;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next)
	{
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_filter") == 0)
			filter_update (f_list->filter_descriptor);
		if (strcmp(f_list->filter_descriptor->name, "nl_mt_gaussian_filter") == 0)
			filter_update (f_list->filter_descriptor);
	}
}

void
alloc_and_init_generate_disparity_map_plan_variables (DISP_DATA **p_dispData, float **p_dispAux)
{
	int num_neurons, i, sample;
	float *dispAux;
	DISP_DATA *dispData;

	num_neurons = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;

	dispAux = (float *) malloc (sizeof (float) * num_neurons);

	// Aloca memoria para a estrutura DISP_DATA
	dispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
	dispData->neg_slope = (char *) alloc_mem (sizeof (char) * num_neurons);
	dispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
	for (i = 0; i < NUM_SAMPLES; i++)
		dispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * num_neurons);

	// Inicializar a estrutura DISP_DATA
	for (i = 0; i < num_neurons; i++)
		dispData->neg_slope[i] = 1;
	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < num_neurons; i++)
		{
			dispData->samples[sample][i].val = FLT_MAX;
			dispData->samples[sample][i].pos = 0;
		}
	}

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < num_neurons; i++)
	{
		dispAux[i] = FLT_MAX;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = .0f;
		nl_disparity_map.neuron_vector[i].output.fval = .0f;
	}
	
	*p_dispAux = dispAux;
	*p_dispData = dispData;
}

void
free_generate_disparity_map_plan_variables (DISP_DATA *dispData, float *dispAux)
{
	int i;

	for (i = 0; i < NUM_SAMPLES; i++)
		free (dispData->samples[i]);

	free (dispData->samples);
	free (dispData->neg_slope);
	free (dispData);
	free (dispAux);
}


void
add_subpixel_disparity_to_disparity_map_plan (void)
{
	int i, n = nl_disparity_map.dimentions.x * nl_disparity_map.dimentions.y;
	for (i = 0; i < n; i++)
		nl_disparity_map.neuron_vector[i].output.fval +=
			nl_subpixel_disparity_map.neuron_vector[i].output.fval;
}

/*void
generate_subpixel_disparity_map_plan (void)
{
	double phaseL1 = PHASE_SIMPLE_LEFT_1;
	double phaseR1 = PHASE_SIMPLE_RIGHT_2;
	double phaseL2 = PHASE_SIMPLE_LEFT_2;
	double phaseR2 = PHASE_SIMPLE_RIGHT_1 + pi;

	RECEPTIVE_FIELD_DESCRIPTION *receptive_field = nl_simple_mono_right_filter->private_state;
	double frequency = receptive_field->frequency;
	int kernel_type = nl_simple_mono_right_filter->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;
	int kernel_num_points = receptive_field->num_points;

	double deltaPhase1 = phaseL1 - phaseR1;
	double deltaPhase2 = phaseL2 - phaseR2;
	int kernel_size = kernel_num_points / (kernel_type + 1);
			
	robot_mt_cell_subpixel_disparity (&nl_subpixel_disparity_map, &nl_disparity_map,
					  &nl_simple_mono_left, &nl_simple_mono_right_subpixel,
					  &nl_simple_mono_left_q, &nl_simple_mono_right_q_subpixel,
					  &nl_simple_mono_left_subpixel, &nl_simple_mono_right,
					  &nl_simple_mono_left_q_subpixel, &nl_simple_mono_right_q,
					  deltaPhase1, deltaPhase2, kernel_size);
}*/

// ----------------------------------------------------------------------------
// get_scan_range - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
get_scan_range (int *nMinScan, int *nMaxScan)
{
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3]; 

	// Define the scaning range
	p_dblRightPoint[0] = ((double) image_right.ww) / 2.0;
	p_dblRightPoint[1] = ((double) image_right.wh) / 2.0 - 1.0;

	// Gets the fartest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblFarVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	*nMinScan = (int) (p_dblLeftPoint[0] + 0.5);
	*nMinScan = (*nMinScan < 0) ? 0 : *nMinScan;

	// Gets the nearest left point
	GetWorldPointAtDistanceRight (p_dblWorldPoint, p_dblRightPoint, g_dblNearVergenceCutPlane, RIGHT_CAMERA);
	CameraProjectionRight (1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	*nMaxScan = (int) (p_dblLeftPoint[0] + 0.5);
	*nMaxScan = (*nMaxScan >= image_left.ww) ? image_left.ww - 1: *nMaxScan;
}

// ----------------------------------------------------------------------------
// AddLocalMin - 
//
// Inputs: 
//
// Output: none
// ----------------------------------------------------------------------------

void 
AddLocalMin (DISP_DATA* dispData, int i, double minus_out, int pos)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
	{
		if (minus_out <= dispData->samples[victim_sample][i].val)
			break;
	}
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			dispData->samples[moving_sample][i] = dispData->samples[moving_sample - 1][i];
			moving_sample--;
		}
		dispData->samples[moving_sample][i].val = minus_out;
		dispData->samples[moving_sample][i].pos = pos;
	}
}

// ----------------------------------------------------------------------------
// distance_neig - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

int 
distance_neig (SAMPLES *sample, int pos, int x, int y, int w, int h)
{
	int dist, count;
	
	dist = count = 0;
	if (x-1 >= 0)
	{
		dist += abs (sample[y*w + x-1].pos - pos);
		count++;
	}
	if (x+1 < w)
	{
		dist += abs (sample[y*w + x+1].pos - pos);
		count++;
	}
	if (y-1 >= 0)
	{
		dist += abs (sample[(y-1)*w + x].pos - pos);
		count++;
	}
	if (y+1 < h)
	{
		dist += abs (sample[(y+1)*w + x].pos - pos);
		count++;
	}

	if ((x-1 >= 0) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x-1].pos - pos);
		count++;
	}
	if ((x-1 >= 0) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x-1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x+1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x+1].pos - pos);
		count++;
	}
	
	return ((int) ((double) dist / (double) count + 0.5));
}

// ----------------------------------------------------------------------------
// calculate_disparity - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
winner_takes_it_all (DISP_DATA* dispData, NEURON_LAYER *nlDisparityMap, int wxd)
{
	SAMPLES temp;
	int x, y, w, h, i, sample, best_sample, cur_dist, dist;
	
	w = nlDisparityMap->dimentions.x;
	h = nlDisparityMap->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (best_sample = sample = 0; sample < NUM_SAMPLES; sample++)
			{
				dist = distance_neig (dispData->samples[0], dispData->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = dispData->samples[0][y * w + x];
			dispData->samples[0][y * w + x] = dispData->samples[best_sample][y * w + x];
			dispData->samples[best_sample][y * w + x] = temp;
		}
	}

	for (i = 0; i < w*h; i++)
	{
		nlDisparityMap->neuron_vector[i].output.fval = (float) dispData->samples[0][i].pos;
	}
}

// ----------------------------------------------------------------------------
// generate_disparity_map_plan - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void 
generate_disparity_map_plan (void)
{
	int nMaxScan, nMinScan, leftPoint;
	int i, x, y, nIndex;
	float fltMinSum, current_mt_gaussian;
	float *mt_gaussian_with_previous_disparity;
	DISP_DATA *dispData;
	int wo, ho;
	
	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	alloc_and_init_generate_disparity_map_plan_variables (&dispData, &mt_gaussian_with_previous_disparity);
	
	get_scan_range (&nMinScan, &nMaxScan);	

	fltMinSum = FLT_MAX;
	for (leftPoint = nMinScan; leftPoint <= nMaxScan; leftPoint++)
	{
		g_nCurrDisparity = leftPoint - (int) (((double) image_right.ww) / 2.0);
		update_mt_cells_filters ();
		for (y = 0; y < ho; y++) 
		{
			for (x = 0; x < wo; x++)
			{
				nIndex = y * wo + x;
				current_mt_gaussian = nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				if (current_mt_gaussian >= mt_gaussian_with_previous_disparity[nIndex])
				{
					if (dispData->neg_slope[nIndex])
					{
						dispData->neg_slope[nIndex] = 0;
						AddLocalMin (dispData, nIndex, mt_gaussian_with_previous_disparity[nIndex], g_nCurrDisparity);
					}
				}
				else
				{
					dispData->neg_slope[nIndex] = 1;
				}
				mt_gaussian_with_previous_disparity[nIndex] = current_mt_gaussian;
			}
		}
	}

	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = dispData->samples[0][i].val;
		nl_disparity_map.neuron_vector[i].output.fval = dispData->samples[0][i].pos;
	}

	// Improve the disparity map by applying the WINNER TAKES IT ALL algorithm. 
	for (i = 0; i < WINNER_TAKES_IT_ALL_STEPS; i++)
		winner_takes_it_all (dispData, &nl_disparity_map, image_left.wxd);

	free_generate_disparity_map_plan_variables (dispData, mt_gaussian_with_previous_disparity);
}
