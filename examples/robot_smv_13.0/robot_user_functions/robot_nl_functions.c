#include "robot_nl_functions.h"

#include "crobot_smv.h"
#include "robot.h"
#include "stereo_api.h"
#include "subpixel_disparity.h"

/*
Seção de Constantes
*/

#define NUM_SAMPLES				5

/*
Seção de Declarações
*/

//FILTER_DESC* nl_simple_mono_right_filter;

double g_dblNearVergenceCutPlane = NEAREST_DISTANCE;

double g_dblFarVergenceCutPlane = FAREST_DISTANCE;

int g_nCurrDisparity;

/* Quebra-galho para suprimir warning de compilação. Aparentemente este protótipo está faltando nos arquivos de header do gcc. */
long lround(double value);

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

void rectify_image(INPUT_DESC* input, int side)
{
	if (side == 0)
		robot_smv_rectify_left(input->name);
	else
		robot_smv_rectify_right(input->name);

	update_input_neurons(input);
	robot_check_input_bounds(input, input->wxd, input->wyd);
	update_input_image(input);
		
}

void rectify_all_images()
{
	time_t start = time(NULL);
      
	rectify_image(&image_left,  0);
	rectify_image(&image_right, 1);
	
	time_t end = time(NULL);
	printf("Time to rectify image: %f s\n", difftime(end, start));
}

void filter_segmentation_update()
{
	FILTER_LIST *f_list = NULL;
	for (f_list = global_filter_list; f_list != NULL; f_list = f_list->next)
	{
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

		if (strcmp(f_list->filter_descriptor->name, "nl_trunk_segmentation_map_vertical_filter") == 0)
			filter_update (f_list->filter_descriptor);

		if (strcmp(f_list->filter_descriptor->name, "nl_trunk_segmentation_map_vertical_rotate_filter") == 0)
			filter_update (f_list->filter_descriptor);

	}
}

void filter_find_update(char* prefix)
{
	FILTER_LIST *f_list;
	int length = strlen(prefix);
	for (f_list = global_filter_list; f_list != NULL; f_list = f_list->next)
	{
		if (strncmp(f_list->filter_descriptor->name, prefix, length) == 0)
			filter_update (f_list->filter_descriptor);
	}	
}

void filter_subpixel_update ()
{
	filter_find_update ("simple_cell_nl");
}

void filter_simple_mono_update(INPUT_DESC* input1, INPUT_DESC* input2, char* prefix)
{
	//Move to center of the input layer
	input1->wxd = ((double) input1->ww) / 2.0;
	input1->wyd = ((double) input1->wh) / 2.0;

	//Move to center of the input layer
	input2->wxd = ((double) input2->ww) / 2.0;
	input2->wyd = ((double) input2->wh) / 2.0;
	
	filter_find_update(prefix);
}

void update_simple_mono_cells_filters()
{
	filter_simple_mono_update(&image_left, &image_right, "nl_simple_mono");
	filter_subpixel_update();
	filter_segmentation_update();
}

long compute_horizontal_offset()
{
	filter_simple_mono_update(&match_back, &match_front, "match_simple_mono");
	generate_disparity_map_plan(&match_mt_gaussian, &match_mt_gaussian_map, &match_disparity_map, "match_mt");
	all_outputs_update();

	int i = 0;
	double value = 0;
	int size = match_disparity_map.dimentions.x * match_disparity_map.dimentions.y;
	NEURON* neuron_vector = match_disparity_map.neuron_vector;
	for (i = 0; i < size; i++)
		value += neuron_vector[i].output.fval;
	value /= i;

	return lround(value);
}

void
alloc_and_init_generate_disparity_map_plan_variables(
	DISP_DATA **p_dispData,
	float **p_dispAux,
	NEURON_LAYER* gaussian_map,
	NEURON_LAYER* disparity_map)
{
	int i, sample;

	int num_neurons = disparity_map->dimentions.x * disparity_map->dimentions.y;

	float* dispAux = (float *) malloc (sizeof (float) * num_neurons);

	// Aloca memoria para a estrutura DISP_DATA
	DISP_DATA* dispData = (DISP_DATA*) alloc_mem(sizeof(DISP_DATA));
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
		gaussian_map->neuron_vector[i].output.fval = .0f;
		disparity_map->neuron_vector[i].output.fval = .0f;
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


void
generate_subpixel_disparity_map_plan (void)
{
/*	robot_mt_cell_subpixel_disparity (&nl_subpixel_disparity_map, &nl_disparity_map,
					  &simple_cell_nl_1,
					  &simple_cell_nl_2,
					  &simple_cell_nl_3,
					  &simple_cell_nl_4,
					  &simple_cell_nl_5,
					  &simple_cell_nl_6,
					  &simple_cell_nl_7,
					  &simple_cell_nl_8,
					  PHI_L1, PHI_R1, PHI_L2, PHI_R2, WAVELENGTH); */
}


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
generate_disparity_map_plan(NEURON_LAYER* gaussian, NEURON_LAYER* gaussian_map, NEURON_LAYER* disparity_map, char* prefix)
{
	int nMaxScan, nMinScan, leftPoint;
	int i, x, y;
	float *mt_gaussian_with_previous_disparity;
	DISP_DATA *dispData;
	
	int wo = disparity_map->dimentions.x;
	int ho = disparity_map->dimentions.y;

	alloc_and_init_generate_disparity_map_plan_variables(&dispData, &mt_gaussian_with_previous_disparity, gaussian_map, disparity_map);
	
	get_scan_range (&nMinScan, &nMaxScan);	

	for (leftPoint = nMinScan; leftPoint <= nMaxScan; leftPoint++)
	{
		g_nCurrDisparity = leftPoint - (int) (((double) image_right.ww) / 2.0);
		filter_find_update(prefix);
		for (y = 0; y < ho; y++) 
		{
			for (x = 0; x < wo; x++)
			{
				int nIndex = y * wo + x;
				float current_mt_gaussian = gaussian->neuron_vector[nIndex].output.fval;
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
		gaussian_map->neuron_vector[i].output.fval = dispData->samples[0][i].val;
		disparity_map->neuron_vector[i].output.fval = dispData->samples[0][i].pos;
	}

	// Improve the disparity map by applying the WINNER TAKES IT ALL algorithm. 
	for (i = 0; i < WINNER_TAKES_IT_ALL_STEPS; i++)
		winner_takes_it_all (dispData, disparity_map, image_left.wxd);

	free_generate_disparity_map_plan_variables (dispData, mt_gaussian_with_previous_disparity);
}
