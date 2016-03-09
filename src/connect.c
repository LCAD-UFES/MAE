	/*! \file
* \brief Functions used for connecting NEURON_LAYERS.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <math.h>
#include "mae.h"

/*! reconnect_2_neurons
*
* This function stablish a synapse between two neurons.
*/

void
reconnect_2_neurons (NEURON_LAYER *neuron_layer_src, NEURON *nsrc, NEURON *ndest, NEURON_OUTPUT synapse_sensitivity, float weight)
{
        SYNAPSE *synapse;

	/* The synapse is alloc'ed */
        synapse = (SYNAPSE *) alloc_mem ((size_t) sizeof (SYNAPSE));
        /* The synapse is initialized */
        reinitialize_synapse (synapse, neuron_layer_src, nsrc, synapse_sensitivity, weight);
        add_synapse (ndest, synapse);
        nsrc->n_out_connections++;
        ndest->n_in_connections++;
}

/*! connect_2_neurons
*
* This function stablish a synapse between two neurons.
*/

void 
connect_2_neurons(NEURON_LAYER *neuron_layer_src, NEURON *nsrc, NEURON *ndest, NEURON_OUTPUT synapse_sensitivity, SENSITIVITY_TYPE neuron_layer_sensitivity,INTERCONNECTION_PATTERN_TYPE interconnection_pattern, int synapse_number)
{
	SYNAPSE *synapse;

	/* The synapse is alloc'ed */
	synapse = (SYNAPSE *) alloc_mem ((size_t) sizeof (SYNAPSE));
	/* The synapse is initialized */
	//initialise_synapse (synapse, neuron_layer_src, nsrc, ndest, synapse_sensitivity, neuron_layer_sensitivity);
	initialise_synapse (synapse, neuron_layer_src, nsrc, ndest, synapse_sensitivity, neuron_layer_sensitivity,interconnection_pattern,synapse_number);
	add_synapse (ndest, synapse);
	nsrc->n_out_connections++;
	ndest->n_in_connections++;
}

/* 
********************************************************************************* 
* Setup the interconnection patern vector as non-initialized for every value	* 
********************************************************************************* 
*/

void
set_interconnection_pattern_vector_as_not_initialized(INTERCONNECTION_PATTERN_VECTOR *interconnection_pattern_vector,long int interconnection_pattern_vector_size)
{
	long int i;
	
	for(i = 0; i < interconnection_pattern_vector_size ; i++)
		interconnection_pattern_vector[i].initialized = FALSE;
}


/* 
********************************************************************************* 
* Get the number of neurons in a neuron vector and update order information	* 
********************************************************************************* 
*/
 
long
get_num_neurons (XY_PAIR dimentions) 
{
	return (dimentions.x * dimentions.y);
} 

 
 
/* 
********************************************************************************* 
* Connect neuron vectors 							* 
********************************************************************************* 
*/ 
 
void 
connect_neuron_layers(NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
		       XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
		       XY_PAIR lower_point_dst, XY_PAIR upper_point_dst) 
{
	int x_src, y_src;
	int x_dst, y_dst;
	int x,y;
	NEURON_OUTPUT synapse_sensitivity;
	
	if (((upper_point_src.x - lower_point_src.x) != (upper_point_dst.x - lower_point_dst.x)) ||
	    ((upper_point_src.y - lower_point_src.y) != (upper_point_dst.y - lower_point_dst.y)))
	    	Erro ("Cannot connect neuron layer regions of different dimentions", "", "");
		
	y_src = lower_point_src.y;
	x = 0;
	y = 0;
	for (y_dst = lower_point_dst.y; y_dst <= upper_point_dst.y; y_dst++)
	{
		x_src = lower_point_src.x;
		for (x_dst = lower_point_dst.x; x_dst <= upper_point_dst.x; x_dst++)
		{
			synapse_sensitivity.ival = DIRECT_CONNECTION;
			
			connect_2_neurons(neuron_layer_src, &(neuron_layer_src->neuron_vector[x_src + y_src*neuron_layer_src->dimentions.x]), 
			&(neuron_layer_dst->neuron_vector[x_dst + y_dst*neuron_layer_dst->dimentions.x]), synapse_sensitivity, NOT_SPECIFIED,STANDARD_INTERCONNECTION_PATTERN,x*y);	// A single synapse for each neuron
			
			x_src++;
			x++;
		}
		y_src++;
		y++;
	}
} 

 
 
/* 
********************************************************************************* 
* Randon connect neuron vectors 						* 
********************************************************************************* 
*/ 
 
void 
random_connect_neuron_layers (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
		       	      XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
		       	      XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
		       	      int nun_inputs_per_neuron) 
{
	int i;
	int x_src, y_src;
	int x_dst, y_dst;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	
	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;

	for (y_dst = lower_point_dst.y; y_dst <= upper_point_dst.y; y_dst++)
	{
		for (x_dst = lower_point_dst.x; x_dst <= upper_point_dst.x; x_dst++)
		{
			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
					neuron_layer_sensitivity = neuron_layer_src->output_type;
				else
					neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

				switch(neuron_layer_sensitivity)
				{
					case GREYSCALE:
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
						synapse_sensitivity.fval = (((float) (rand () % 100)) / 50.0 - 1.0);
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;
					case COLOR_SELECTIVE:
						synapse_sensitivity.ival = 0;		
						synapse_sensitivity.ival += rand () % 256;
						synapse_sensitivity.ival += (rand () % 256)<<8;
						synapse_sensitivity.ival += (rand () % 256)<<16;
						//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
						//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
						break;

				}

				x_src = (rand () % (upper_point_src.x - lower_point_src.x + 1)) + lower_point_src.x;
				y_src = (rand () % (upper_point_src.y - lower_point_src.y + 1)) + lower_point_src.y;

				connect_2_neurons(neuron_layer_src, &(neuron_layer_src->neuron_vector[x_src + y_src*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x_dst + y_dst*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, STANDARD_INTERCONNECTION_PATTERN, i);
				
			}
		}
	}
	
	//No need for random minchinton
} 


/* 
********************************************************************************* 
* Gaussian random distribution generator					* 
********************************************************************************* 
*/


double 
gaussrand()
{
        static double V1, V2, S;
        static int phase = 0;
        double X;

        if (phase == 0) 
	{
                do 
		{
                        double U1 = (double) rand() / (double) LRAND48_MAX;
                        double U2 = (double) rand() / (double) LRAND48_MAX;

                        V1 = 2.0 * U1 - 1.0;
                        V2 = 2.0 * U2 - 1.0;
                        S = V1 * V1 + V2 * V2;
                } while (S >= 1.0 || S == 0.0);

                X = V1 * sqrt(-2.0 * log(S) / S);
        } 
	else
	{
                X = V2 * sqrt(-2.0 * log(S) / S);
	}

        phase = 1 - phase;

        return (X);
}


/* 
********************************************************************************* 
* Scaled random integer number generator - used for random thresholding		* 
********************************************************************************* 
*/


int 
random_integer(int low, int high)
{
	// This function provides a random integer
	// between the low..high closed interval

	int k;
	double d;
	d = (double) rand () / ((double) RAND_MAX + 1);
	k = d * (high - low + 1);
	
	return(low + k);
}


/* 
********************************************************************************* 
* Scaled random float number generator - used for random thresholding		* 
********************************************************************************* 
*/


float
random_float(float low,float high)
{
	// This function provides a random float
	// between the low..high closed interval

	float k;
	double d;
	d = (double) rand () / ((double) RAND_MAX + 1);
	k = d * (high - low + 1);
	
	return(low + k);
}


/* 
********************************************************************************* 
*  Connect neuron vectors gaussianly						* 
********************************************************************************* 
*/ 

 
void 
connect_neuron_layers_gaussianly_2d (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
				  XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
				  int nun_inputs_per_neuron, double gaussian_radius_x,
				  double gaussian_radius_y, double gaussian_orientation,
				  INTERCONNECTION_PATTERN_TYPE interconnection_pattern) 
{
	int i, tries;
	int x, y, nx_src, ny_src, nx_dst, ny_dst, gx, gy;
	double grx, gry, grx_line, gry_line;
	float x_factor, y_factor;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	INTERCONNECTION_PATTERN_VECTOR first_interconnection_pattern[nun_inputs_per_neuron*3];

	
	//Pattern Interconnection Initialize
	set_interconnection_pattern_vector_as_not_initialized(first_interconnection_pattern, nun_inputs_per_neuron*3);
	
	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;

	if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
	{
		// Alberto: Os valores de nx_src, ny_src, etc. abaixo estao corretos porque upper_point_src e upper_point_dst vem com um a menos
		// Parte de entrada da rede nao pode ser aproveitada por conta das sinapses que ficariam fora da borda
		nx_src = upper_point_src.x - lower_point_src.x + 1 - (int) (2.0 * 4.0 * gaussian_radius_x + 0.5);
		ny_src = upper_point_src.y - lower_point_src.y + 1 - (int) (2.0 * 4.0 * gaussian_radius_y + 0.5);
	}
	else
	{
		// Alberto: Os valores de nx_src, ny_src, etc. abaixo estao corretos porque upper_point_src e upper_point_dst vem com um a menos
		nx_src = upper_point_src.x - lower_point_src.x + 1;
		ny_src = upper_point_src.y - lower_point_src.y + 1;
	}
	nx_dst = upper_point_dst.x - lower_point_dst.x + 1;
	ny_dst = upper_point_dst.y - lower_point_dst.y + 1;

	x_factor = (float) nx_src / (float) nx_dst;
	y_factor = (float) ny_src / (float) ny_dst;

	// Alberto: O <= abaixo esta correto porque upper_point_dst vem com um a menos
	for (y = lower_point_dst.y; y <= upper_point_dst.y; y++)
	{
		// Alberto: O <= abaixo esta correto porque upper_point_dst vem com um a menos
		for (x = lower_point_dst.x; x <= upper_point_dst.x; x++)
		{
			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if ((y != lower_point_dst.y) || (x != lower_point_dst.x))
					{
						grx = first_interconnection_pattern[i*3].rand_x.dval;
						gry = first_interconnection_pattern[i*3].rand_y.dval;
					}
					else
					{
						if (i == 0) // This is necessary for minchinton_center_surround neurons and should not affect other types of neuron
						{
							grx = 0.0;
							gry = 0.0;
						}
						else
						{
							grx_line = gaussrand () * gaussian_radius_x;
							gry_line = gaussrand () * gaussian_radius_y;
							grx = grx_line * cos(gaussian_orientation) + gry_line * sin(gaussian_orientation) + 0.5;
							gry = -grx_line * sin(gaussian_orientation) + gry_line * cos(gaussian_orientation) + 0.5;
						}
						first_interconnection_pattern[i*3].rand_x.dval = grx;
						first_interconnection_pattern[i*3].rand_y.dval = gry;
					}
					
					if (x_factor > 1.0)
						gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + 4.0 * gaussian_radius_x + x_factor / 2.0 + grx);
					else
						gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + 4.0 * gaussian_radius_x + grx);

					if (y_factor > 1.0)
						gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + 4.0 * gaussian_radius_y + y_factor / 2.0 + gry);
					else
						gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + 4.0 * gaussian_radius_y + gry);
				}
				else	// different interconnection pattern (synapses can randomize into the image)
				{
					tries = 0;
					do
					{
						if (i == 0) // This is necessary for minchinton_center_surround neurons and should not affect other types of neuron
						{
							grx = 0.0;
							gry = 0.0;
						}
						else
						{
							grx_line = gaussrand () * gaussian_radius_x;
							gry_line = gaussrand () * gaussian_radius_y;
							grx = grx_line * cos(gaussian_orientation) + gry_line * sin(gaussian_orientation) + 0.5;
							gry = -grx_line * sin(gaussian_orientation) + gry_line * cos(gaussian_orientation) + 0.5;
						}
						
						if (x_factor > 1.0)
							gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + x_factor / 2.0 + grx);
						else
							gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + grx);

						if (y_factor > 1.0)
							gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + y_factor / 2.0 + gry);
						else
							gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + gry);

						tries++;
					}
					while (((gx < 0) || (gy < 0) || (gx >= neuron_layer_src->dimentions.x) || (gy >= neuron_layer_src->dimentions.y)) && (tries < 100));

					if (tries == 100)
					{	
						printf("Could not connect %s to %s after %d tries... Isn't gaussian_radius_x (%lf) or gaussian_radius_y (%lf) too small for the current number of synapes (%d)?\n",
							neuron_layer_src->name, neuron_layer_dst->name, tries, gaussian_radius_x, gaussian_radius_y, nun_inputs_per_neuron);
						Erro ("Could not connect neuron layers", "", "");
					}
				}
				
				if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
					neuron_layer_sensitivity = neuron_layer_src->output_type;
				else
					neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

				switch (neuron_layer_sensitivity)
				{
					case GREYSCALE:
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
						synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;
					case COLOR_SELECTIVE:
						synapse_sensitivity.ival = 0;
						synapse_sensitivity.ival += rand () % 256;
						synapse_sensitivity.ival += (rand () % 256)<<8;
						synapse_sensitivity.ival += (rand () % 256)<<16;
						//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
						//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
						break;

				}

				//1st synapse sensitivity must also be recorded - neuron_layer_sensitivity dependent
				if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
						first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
					else
						first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

					first_interconnection_pattern[i*3].initialized = TRUE;
				}
				else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (first_interconnection_pattern[i*3].initialized == FALSE)
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
						else
							first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

						first_interconnection_pattern[i*3].initialized = TRUE;
					}
					else
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							synapse_sensitivity.fval = first_interconnection_pattern[i*3].sensitivity.fval;
						else
							synapse_sensitivity.ival = first_interconnection_pattern[i*3].sensitivity.ival;
					}					
				}

				// Alberto: Este if ee necessario no caso de interconnection_pattern == SAME_INTERCONNECTION_PATTERN
				if ((gx >= 0) && (gy >= 0) && (gx < neuron_layer_src->dimentions.x) && (gy < neuron_layer_src->dimentions.y))
					connect_2_neurons(neuron_layer_src, &(neuron_layer_src->neuron_vector[gx + gy*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x + y*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, interconnection_pattern, i);
			}
		}
	}
} 


/* 
********************************************************************************* 
*  Connect neuron vectors gaussianly						* 
********************************************************************************* 
*/ 

 
void 
connect_neuron_layers_gaussianly (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
				  XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
				  int nun_inputs_per_neuron, double gaussian_radius,
				  INTERCONNECTION_PATTERN_TYPE interconnection_pattern ) 
{
	int i, tries;
	int x, y, nx_src, ny_src, nx_dst, ny_dst, gx, gy;
	double grx, gry;
	float x_factor, y_factor;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	INTERCONNECTION_PATTERN_VECTOR first_interconnection_pattern[nun_inputs_per_neuron*3];

	
	//Pattern Interconnection Initialize
	set_interconnection_pattern_vector_as_not_initialized(first_interconnection_pattern, nun_inputs_per_neuron*3);
	
	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;

	if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
	{
		// Alberto: Os valores de nx_src, ny_src, etc. abaixo estao corretos porque upper_point_src e upper_point_dst vem com um a menos
		// Parte de entrada da rede nao pode ser aproveitada por conta das sinapses que ficariam fora da borda
		nx_src = upper_point_src.x - lower_point_src.x + 1 - (int) (2.0 * 4.0 * gaussian_radius + 0.5);
		ny_src = upper_point_src.y - lower_point_src.y + 1 - (int) (2.0 * 4.0 * gaussian_radius + 0.5);
	}
	else
	{
		// Alberto: Os valores de nx_src, ny_src, etc. abaixo estao corretos porque upper_point_src e upper_point_dst vem com um a menos
		nx_src = upper_point_src.x - lower_point_src.x + 1;
		ny_src = upper_point_src.y - lower_point_src.y + 1;
	}
	nx_dst = upper_point_dst.x - lower_point_dst.x + 1;
	ny_dst = upper_point_dst.y - lower_point_dst.y + 1;

	x_factor = (float) nx_src / (float) nx_dst;
	y_factor = (float) ny_src / (float) ny_dst;

	// Alberto: O <= abaixo esta correto porque upper_point_dst vem com um a menos
	for (y = lower_point_dst.y; y <= upper_point_dst.y; y++)
	{
		// Alberto: O <= abaixo esta correto porque upper_point_dst vem com um a menos
		for (x = lower_point_dst.x; x <= upper_point_dst.x; x++)
		{
			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if ((y != lower_point_dst.y) || (x != lower_point_dst.x))
					{
						grx = first_interconnection_pattern[i*3].rand_x.dval;
						gry = first_interconnection_pattern[i*3].rand_y.dval;
					}
					else
					{
						if (i == 0) // This is necessary for minchinton_center_surround neurons and should not affect other types of neuron
						{
							grx = 0.0;
							gry = 0.0;
						}
						else
						{
							grx = gaussrand () * gaussian_radius + 0.5;
							gry = gaussrand () * gaussian_radius + 0.5;
						}
						first_interconnection_pattern[i*3].rand_x.dval = grx;
						first_interconnection_pattern[i*3].rand_y.dval = gry;
					}
					
					if (x_factor > 1.0)
						gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + 4.0 * gaussian_radius + x_factor / 2.0 + grx);
					else
						gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + 4.0 * gaussian_radius + grx);

					if (y_factor > 1.0)
						gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + 4.0 * gaussian_radius + y_factor / 2.0 + gry);
					else
						gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + 4.0 * gaussian_radius + gry);
				}
				else	// different interconnection pattern (synapses can randomize into the image)
				{
					tries = 0;
					do
					{
						if (i == 0) // This is necessary for minchinton_center_surround neurons and should not affect other types of neuron
						{
							grx = 0.0;
							gry = 0.0;
						}
						else
						{
							grx = gaussrand () * gaussian_radius + 0.5;
							gry = gaussrand () * gaussian_radius + 0.5;
						}
						
						if (x_factor > 1.0)
							gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + x_factor / 2.0 + grx);
						else
							gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + grx);

						if (y_factor > 1.0)
							gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + y_factor / 2.0 + gry);
						else
							gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + gry);

						tries++;
					}
					while (((gx < 0) || (gy < 0) || (gx >= neuron_layer_src->dimentions.x) || (gy >= neuron_layer_src->dimentions.y)) && (tries < 100));

					if (tries == 100)
					{	
						printf("Could not connect %s to %s after %d tries... Isn't gaussian_radius (%lf) too small for the current number of synapes (%d)?\n",
							neuron_layer_src->name, neuron_layer_dst->name, tries, gaussian_radius, nun_inputs_per_neuron);
						Erro ("Could not connect neuron layers", "", "");
					}
				}
				
				if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
					neuron_layer_sensitivity = neuron_layer_src->output_type;
				else
					neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

				switch (neuron_layer_sensitivity)
				{
					case GREYSCALE:
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
						synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;
					case COLOR_SELECTIVE:
						synapse_sensitivity.ival = 0;
						synapse_sensitivity.ival += rand () % 256;
						synapse_sensitivity.ival += (rand () % 256)<<8;
						synapse_sensitivity.ival += (rand () % 256)<<16;
						//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
						//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
						break;

				}

				//1st synapse sensitivity must also be recorded - neuron_layer_sensitivity dependent
				if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
						first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
					else
						first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

					first_interconnection_pattern[i*3].initialized = TRUE;
				}
				else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (first_interconnection_pattern[i*3].initialized == FALSE)
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
						else
							first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

						first_interconnection_pattern[i*3].initialized = TRUE;
					}
					else
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							synapse_sensitivity.fval = first_interconnection_pattern[i*3].sensitivity.fval;
						else
							synapse_sensitivity.ival = first_interconnection_pattern[i*3].sensitivity.ival;
					}					
				}

				// Alberto: Este if ee necessario no caso de interconnection_pattern == SAME_INTERCONNECTION_PATTERN
				if ((gx >= 0) && (gy >= 0) && (gx < neuron_layer_src->dimentions.x) && (gy < neuron_layer_src->dimentions.y))
					connect_2_neurons(neuron_layer_src, &(neuron_layer_src->neuron_vector[gx + gy*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x + y*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, interconnection_pattern, i);
			}
		}
	}
} 


/* 
********************************************************************************* 
*  Random connect neuron vectors linearly within squared range			* 
********************************************************************************* 
*/ 
 
void 
random_connect_neuron_layers_lin (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
				  XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
				  int nun_inputs_per_neuron, double side,
				  INTERCONNECTION_PATTERN_TYPE interconnection_pattern ) 
{
	int i;
	int x, y, nx_src, ny_src, nx_dst, ny_dst, gx, gy;
	int x_rand, y_rand;
	float x_factor, y_factor;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	INTERCONNECTION_PATTERN_VECTOR first_interconnection_pattern[nun_inputs_per_neuron*3];
	
	//Pattern Interconnection Initialize
	set_interconnection_pattern_vector_as_not_initialized(first_interconnection_pattern,nun_inputs_per_neuron*3);
	
	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;

	nx_src = upper_point_src.x - lower_point_src.x + 1;
	ny_src = upper_point_src.y - lower_point_src.y + 1;
	nx_dst = upper_point_dst.x - lower_point_dst.x + 1;
	ny_dst = upper_point_dst.y - lower_point_dst.y + 1;

	x_factor = (float) nx_src / (float) nx_dst;
	y_factor = (float) ny_src / (float) ny_dst;
	for (y = lower_point_dst.y; y <= upper_point_dst.y; y++)
	{
		for (x = lower_point_dst.x; x <= upper_point_dst.x; x++)
		{
			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				x_rand = rand();
				y_rand = rand();

				if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{	
					first_interconnection_pattern[i*3].rand_x.ival = x_rand;
					first_interconnection_pattern[i*3].rand_y.ival = y_rand;
				}
				else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					x_rand = first_interconnection_pattern[i*3].rand_x.ival;
					y_rand = first_interconnection_pattern[i*3].rand_y.ival;
				}

				gx = lower_point_src.x + (int)((double) (x - lower_point_dst.x) * x_factor + 
				     (((double) x_rand / (double) LRAND48_MAX) * side) - side/2.0 + 0.5);
				gy = lower_point_src.y + (int)((double) (y - lower_point_dst.y) * y_factor + 
				     (((double) y_rand / (double) LRAND48_MAX) * side) - side/2.0 + 0.5);
				if ((gx < 0) || (gy < 0) || 
				    (gx >= neuron_layer_src->dimentions.x) || (gy >= neuron_layer_src->dimentions.y))
				{
					synapse_sensitivity.ival = 0;
					
					connect_2_neurons (NULL, &(neuron_with_output_zero), &(neuron_layer_dst->neuron_vector[x + y * neuron_layer_dst->dimentions.x]), synapse_sensitivity, NOT_SPECIFIED, interconnection_pattern, i);
					
					//Null synapses will be resetted to a valid new value later - synapse sensitivity type must be recorded
					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
						first_interconnection_pattern[i*3].initialized = FALSE;
				}
				else
				{
					if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
						neuron_layer_sensitivity = neuron_layer_src->output_type;
					else
						neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

					switch(neuron_layer_sensitivity)
					{
						case GREYSCALE:
							synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
							break;
						case BLACK_WHITE:
							synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
							break;
						case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
							synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
							break;
						case COLOR:
							synapse_sensitivity.ival = rand () % NUM_COLORS;
							break;
						case COLOR_SELECTIVE:
							synapse_sensitivity.ival = 0;
							synapse_sensitivity.ival += rand () % 256;
							synapse_sensitivity.ival += (rand () % 256)<<8;
							synapse_sensitivity.ival += (rand () % 256)<<16;
							//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
							//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
							break;
						
					}

					//1st synapse sensitivity must also be recorded - neuron_layer_sensitivity dependent
					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
						else
							first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

						first_interconnection_pattern[i*3].initialized = TRUE;
					}
					else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						if (first_interconnection_pattern[i*3].initialized == FALSE)
						{
							if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
								first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
							else
								first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;
							
							first_interconnection_pattern[i*3].initialized = TRUE;
						}
						else
						{
							if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
								synapse_sensitivity.fval = first_interconnection_pattern[i*3].sensitivity.fval;
							else
								synapse_sensitivity.ival = first_interconnection_pattern[i*3].sensitivity.ival;
						}					
					}
					
					connect_2_neurons (neuron_layer_src, &(neuron_layer_src->neuron_vector[gx + gy*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x + y*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, interconnection_pattern, i);
					
				}

			}
		}
	}
}

/* 
********************************************************************************* 
* Log-Polar neuron layer connection 						* 
********************************************************************************* 
*/ 

void
connect_neuron_layers_log_polar (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
				XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
				XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
				int nun_inputs_per_neuron, double gaussian_radius, double log_factor,
				INTERCONNECTION_PATTERN_TYPE interconnection_pattern)
{

	int x, y, gx, gy;
	int i, h, w, hi, wi, xi, yi, x_center, y_center;
	double grx, gry;
	//double angle;
	
	double correction;
	//INPUT_DESC *input = NULL;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	INTERCONNECTION_PATTERN_VECTOR first_interconnection_pattern[nun_inputs_per_neuron*3];
	
	//Pattern Interconnection Initialize
	set_interconnection_pattern_vector_as_not_initialized(first_interconnection_pattern,nun_inputs_per_neuron*3);

	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;
	
	wi = neuron_layer_src->dimentions.x;
	hi = neuron_layer_src->dimentions.y;
	w = neuron_layer_dst->dimentions.x;
	h = neuron_layer_dst->dimentions.y;

	x_center = wi/2 ;
	y_center = hi/2 ;

	// log-polar transform correction factor
	correction = (double) h / (double) (h -1);

	for (y = lower_point_dst.y; y <= upper_point_dst.y; y++)
	{
		for (x = lower_point_dst.x; x <= upper_point_dst.x; x++)
		{
			//Here, we perform the log-polar mapping for determining the neuron position in the map
			map_v1_to_image (&xi, &yi, wi, hi, x, y, w, h, x_center, y_center, correction, log_factor);
			//angle = map_v1_to_image_angle (x,y,w,h);

			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				#if 0
				if(interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					grx = gaussrand () * gaussian_radius + 0.5;
					gry = gaussrand () * gaussian_radius + 0.5;

					if (y == lower_point_dst.y && x == lower_point_dst.x)
					{	
						first_interconnection_pattern[i*3].rand_x.dval = grx;
						first_interconnection_pattern[i*3].rand_y.dval = gry;
					}
					else
					{
						grx = first_interconnection_pattern[i*3].rand_x.dval;
						gry = first_interconnection_pattern[i*3].rand_y.dval;
					}

					gx = lower_point_src.x + (int)((double) (xi - lower_point_dst.x) + grx);
					gy = lower_point_src.y + (int)((double) (yi - lower_point_dst.y) + gry);
				}
				else	// different interconnection pattern (synapses can randomize into the image)
				{
					do
					{
						grx = gaussrand () * gaussian_radius + 0.5;
						gry = gaussrand () * gaussian_radius + 0.5;

						gx = lower_point_src.x + (int)((double) (xi - lower_point_dst.x) + grx);
						gy = lower_point_src.y + (int)((double) (yi - lower_point_dst.y) + gry);
					}
					while ((gx < 0) || (gy < 0) || (gx >= wi) || (gy >= hi));
				}
				#endif
				
				//#if 0
				//do
				//{
					grx = gaussrand () * gaussian_radius + 0.5;
					gry = gaussrand () * gaussian_radius + 0.5;

					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{	
						first_interconnection_pattern[i*3].rand_x.dval = grx;
						first_interconnection_pattern[i*3].rand_y.dval = gry;
					}
					else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						grx = first_interconnection_pattern[i*3].rand_x.dval;
						gry = first_interconnection_pattern[i*3].rand_y.dval;
						//grx = first_interconnection_pattern[i*3].rand_x.dval*cos(angle) - first_interconnection_pattern[i*3].rand_y.dval*sin(angle);
						//gry = first_interconnection_pattern[i*3].rand_x.dval*sin(angle) + first_interconnection_pattern[i*3].rand_y.dval*cos(angle);
					}

					gx = lower_point_src.x + (int)((double) (xi - lower_point_dst.x) + grx);
					gy = lower_point_src.y + (int)((double) (yi - lower_point_dst.y) + gry);
				//}
				//while ((gx < 0) || (gy < 0) || (gx >= wi) || (gy >= hi));
				//#endif

				//Boundary checks must be kept in order to avoid invalid synapse posotioning
				if ((gx < 0) || (gy < 0) || (gx >= wi) || (gy >= hi))
				{
					synapse_sensitivity.ival = 0;
					
					connect_2_neurons (NULL, &(neuron_with_output_zero), &(neuron_layer_dst->neuron_vector[x + y * neuron_layer_dst->dimentions.x]), synapse_sensitivity, NOT_SPECIFIED, interconnection_pattern, i);
					
					//Null synapses will be resetted to a valid new value later - synapse sensitivity type must be recorded
					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
						first_interconnection_pattern[i*3].initialized = FALSE;
				}
				else
				{
					if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
						neuron_layer_sensitivity = neuron_layer_src->output_type;
					else
						neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

					switch(neuron_layer_sensitivity)
					{
						case GREYSCALE:
							synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
							break;
						case BLACK_WHITE:
							synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
							break;
						case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
							synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
							break;
						case COLOR:
							synapse_sensitivity.ival = rand () % NUM_COLORS;
							break;
						case COLOR_SELECTIVE:
							synapse_sensitivity.ival = 0;
							synapse_sensitivity.ival += rand () % 256;
							synapse_sensitivity.ival += (rand () % 256)<<8;
							synapse_sensitivity.ival += (rand () % 256)<<16;
							//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;	//composed color channels for composed type synapse
							//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;		//total number of color channels
							break;

					}

					//1st synapse sensitivity must also be recorded - neuron_layer_sensitivity dependent
					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
						else
							first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

						first_interconnection_pattern[i*3].initialized = TRUE;
					}
					else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						if (first_interconnection_pattern[i*3].initialized == FALSE)
						{
							if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
								first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
							else
								first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

							first_interconnection_pattern[i*3].initialized = TRUE;
						}
						else
						{
							if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
								synapse_sensitivity.fval = first_interconnection_pattern[i*3].sensitivity.fval;
							else
								synapse_sensitivity.ival = first_interconnection_pattern[i*3].sensitivity.ival;
						}					
					}

					connect_2_neurons (neuron_layer_src, &(neuron_layer_src->neuron_vector[gx + gy*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x + y*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, interconnection_pattern, i);
				}
			}
		}
	}
}

/* 
********************************************************************************* 
* Log-Polar-Scaled neuron layer connection  (DEPRECATED)			* 
********************************************************************************* 
*/ 

void
connect_neuron_layers_log_polar_scaled (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
					XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
					XY_PAIR lower_point_dst, XY_PAIR upper_point_dst, 
					int nun_inputs_per_neuron, double gaussian_radius, double log_factor, double scale_factor,
					INTERCONNECTION_PATTERN_TYPE interconnection_pattern)
{

	int x, y, gx, gy;
	int i, h, w, hi, wi, xi, yi, x_center, y_center;
	double grx, gry;
	double correction;
	//INPUT_DESC *input = NULL;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	INTERCONNECTION_PATTERN_VECTOR first_interconnection_pattern[nun_inputs_per_neuron*3];
	
	//Pattern Interconnection Initialize
	set_interconnection_pattern_vector_as_not_initialized(first_interconnection_pattern,nun_inputs_per_neuron*3);

	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;

	wi = neuron_layer_src->dimentions.x;
	hi = neuron_layer_src->dimentions.y;
	w = neuron_layer_dst->dimentions.x;
	h = neuron_layer_dst->dimentions.y;

	x_center = wi/2 ;
	y_center = hi/2 ;

	// log-polar transform correction factor
	correction = (double) h / (double) (h -1);

	for (y = lower_point_dst.y; y <= upper_point_dst.y; y++)
	{
		for (x = lower_point_dst.x; x <= upper_point_dst.x; x++)
		{
			//Here, we perform the log-polar mapping for determining the neuron position in the map
			map_v1_to_image (&xi, &yi, wi, hi, x, y, w, h, x_center, y_center, correction, log_factor);

			xi = x_center + (xi - x_center)*scale_factor;
			yi = y_center +	(yi - y_center)*scale_factor;
			
			

			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				do 
				{
					grx = gaussrand () * gaussian_radius + 0.5;
					gry = gaussrand () * gaussian_radius + 0.5;

					if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{	
						first_interconnection_pattern[i*3].rand_x.dval = grx;
						first_interconnection_pattern[i*3].rand_y.dval = gry;
					}
					else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
					{
						grx = first_interconnection_pattern[i*3].rand_x.dval;
						gry = first_interconnection_pattern[i*3].rand_y.dval;
					}

					gx = lower_point_src.x + (int)((double) (xi - lower_point_dst.x) + grx);
					gy = lower_point_src.y + (int)((double) (yi - lower_point_dst.y) + gry);
				} while ((gx < 0) || (gy < 0) || (gx >= wi) || (gy >= hi));

				if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
					neuron_layer_sensitivity = neuron_layer_src->output_type;
				else
					neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

				switch (neuron_layer_sensitivity)
				{
					case GREYSCALE:
						//synapse_sensitivity.ival = rand () % NUM_GREYLEVELS_2;
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
						synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;
					case COLOR_SELECTIVE:
						synapse_sensitivity.ival = 0;
						synapse_sensitivity.ival += rand () % 256;
						synapse_sensitivity.ival += (rand () % 256)<<8;
						synapse_sensitivity.ival += (rand () % 256)<<16;
						//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
						//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
						break;

				}

				//1st synapse sensitivity must also be recorded - neuron_layer_sensitivity dependent
				if (y == lower_point_dst.y && x == lower_point_dst.x && interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
						first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
					else
						first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

					first_interconnection_pattern[i*3].initialized = TRUE;
				}
				else if (interconnection_pattern == SAME_INTERCONNECTION_PATTERN)
				{
					if (first_interconnection_pattern[i*3].initialized == FALSE)
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							first_interconnection_pattern[i*3].sensitivity.fval = synapse_sensitivity.fval;
						else
							first_interconnection_pattern[i*3].sensitivity.ival = synapse_sensitivity.ival;

						first_interconnection_pattern[i*3].initialized = TRUE;
					}
					else
					{
						if (neuron_layer_sensitivity == GREYSCALE_FLOAT)
							synapse_sensitivity.fval = first_interconnection_pattern[i*3].sensitivity.fval;
						else
							synapse_sensitivity.ival = first_interconnection_pattern[i*3].sensitivity.ival;
					}					
				}

				connect_2_neurons (neuron_layer_src, &(neuron_layer_src->neuron_vector[gx + gy*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x + y*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, interconnection_pattern, i);
			}
		}
	}
}

/* 
********************************************************************************* 
* Output connect neuron vectors (deprecated)					* 
********************************************************************************* 
*/ 
 
void 
output_connect_neuron_layer (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst) 
{
	if (!same_dimentions (neuron_layer_src->dimentions, neuron_layer_dst->dimentions))
		Erro ("Cannot connect a neuron layer to an output of different dimention", "", "");
	neuron_layer_dst->neuron_vector = neuron_layer_src->neuron_vector;
	neuron_layer_dst->sensitivity = neuron_layer_src->output_type;
	neuron_layer_dst->output_type = neuron_layer_src->output_type;
} 

 
 
/* 
********************************************************************************* 
* Full connect neuron vectors 							* 
********************************************************************************* 
*/ 
 
void 
full_connect_neuron_layers (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
			    XY_PAIR lower_point_src, XY_PAIR upper_point_src, 
			    XY_PAIR lower_point_dst, XY_PAIR upper_point_dst) 
{
	int x_src, y_src;
	int x_dst, y_dst;
	int x,y;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	// Synapse sensitivity initialize
	synapse_sensitivity.ival = 0;
	
	x = 0;
	y = 0;	
	for (y_dst = lower_point_dst.y; y_dst <= upper_point_dst.y; y_dst++)
	{
		for (x_dst = lower_point_dst.x; x_dst <= upper_point_dst.x; x_dst++)
		{
			for (y_src = lower_point_src.y; y_src <= upper_point_src.y; y_src++)
			{
				for (x_src = lower_point_src.x; x_src <= upper_point_src.x; x_src++)
				{
					if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
						neuron_layer_sensitivity = neuron_layer_src->output_type;
					else
						neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

					switch(neuron_layer_sensitivity)
					{
						case GREYSCALE:
							synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
							break;
						case BLACK_WHITE:
							synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
							break;
						case GREYSCALE_FLOAT: /* Range = [-1.0 : 1.0] */
							synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
							break;
						case COLOR:
							synapse_sensitivity.ival = rand () % NUM_COLORS;
							break;
						case COLOR_SELECTIVE:
							synapse_sensitivity.ival = 0;
							synapse_sensitivity.ival += rand () % 256;
							synapse_sensitivity.ival += (rand () % 256)<<8;
							synapse_sensitivity.ival += (rand () % 256)<<16;
							//synapse_sensitivity.ival = rand () % NUM_COMPOSED_COLOR_CHANNELS;
							//synapse_sensitivity.ival = rand () % NUM_COLOR_CHANNELS;
							break;
					}
					
					connect_2_neurons (neuron_layer_src, &(neuron_layer_src->neuron_vector[x_src + y_src*neuron_layer_src->dimentions.x]), &(neuron_layer_dst->neuron_vector[x_dst + y_dst*neuron_layer_dst->dimentions.x]), synapse_sensitivity, neuron_layer_sensitivity, STANDARD_INTERCONNECTION_PATTERN, x*y);	//one synapse for each neuron
					
					x++;
				}
				y++;
			}
		}
	}
} 
 
 
 
/* 
********************************************************************************* 
* Connect neurons 								* 
********************************************************************************* 
*/ 
 
void 
connect_neurons (int type, NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
		 int num_connections_per_neuron, double sub_region_size, double sub_region_size_2, double sub_region_size_3, 
		 int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, INTERCONNECTION_PATTERN_TYPE ic_pt_type) 
{
	XY_PAIR lower_point_src, upper_point_src; 
	XY_PAIR lower_point_dst, upper_point_dst; 
	
	n_connections_per_neuron = num_connections_per_neuron;

	if (x1 == -1)
	{
		lower_point_src.x = 0;
		lower_point_src.y = 0;
		upper_point_src.x = neuron_layer_src->dimentions.x - 1;
		upper_point_src.y = neuron_layer_src->dimentions.y - 1;
	}
	else
	{
		if ((x1 > x2) || (y1 > y2))
		    	Erro ("region lower point above upper point", "", "");
		if ((x2 >= neuron_layer_src->dimentions.x) || (y2 >= neuron_layer_src->dimentions.y))
		    	Erro ("region upper point outside neuron layer", "", "");
		lower_point_src.x = x1;
		lower_point_src.y = y1;
		upper_point_src.x = x2;
		upper_point_src.y = y2;
	}
			
	if (x3 == -1)
	{
		lower_point_dst.x = 0;
		lower_point_dst.y = 0;
		upper_point_dst.x = neuron_layer_dst->dimentions.x - 1;
		upper_point_dst.y = neuron_layer_dst->dimentions.y - 1;
	}
	else
	{
		if ((x3 > x4) || (y3 > y4))
		    	Erro ("region lower point above upper point", "", "");
		if ((x4 >= neuron_layer_dst->dimentions.x) || (y4 >= neuron_layer_dst->dimentions.y))
		    	Erro ("region upper point outside neuron layer", "", "");
		lower_point_dst.x = x3;
		lower_point_dst.y = y3;
		upper_point_dst.x = x4;
		upper_point_dst.y = y4;
	}
		
	if (type == FUL)
		full_connect_neuron_layers (neuron_layer_src, neuron_layer_dst, 
					    lower_point_src, upper_point_src, 
					    lower_point_dst, upper_point_dst);
	else if (type == OUT)
		output_connect_neuron_layer (neuron_layer_src, neuron_layer_dst);
	else if (type == RAN)
		random_connect_neuron_layers (neuron_layer_src, neuron_layer_dst, 
					      lower_point_src, upper_point_src, 
					      lower_point_dst, upper_point_dst, 
					      num_connections_per_neuron);
	else if (type == GAU)
		connect_neuron_layers_gaussianly (neuron_layer_src, neuron_layer_dst, 
						  lower_point_src, upper_point_src, 
						  lower_point_dst, upper_point_dst,
						  num_connections_per_neuron,
						  sub_region_size, ic_pt_type);
	else if (type == GAU2)
		connect_neuron_layers_gaussianly_2d (neuron_layer_src, neuron_layer_dst, 
						  lower_point_src, upper_point_src, 
						  lower_point_dst, upper_point_dst,
						  num_connections_per_neuron,
						  sub_region_size, sub_region_size_2, 
						  (sub_region_size_3 * M_PI) / 180.0, ic_pt_type);
	else if (type == LIN)
		random_connect_neuron_layers_lin (neuron_layer_src, neuron_layer_dst, 
						  lower_point_src, upper_point_src, 
						  lower_point_dst, upper_point_dst,
						  num_connections_per_neuron,
						  sub_region_size, ic_pt_type);
	else if (type == LOG)
		connect_neuron_layers_log_polar (neuron_layer_src, neuron_layer_dst, 
						  lower_point_src, upper_point_src, 
						  lower_point_dst, upper_point_dst,
						  num_connections_per_neuron,
						  sub_region_size, sub_region_size_2, ic_pt_type);
	else if (type == LOG_SCALED)
		connect_neuron_layers_log_polar_scaled (neuron_layer_src, neuron_layer_dst, 
						  lower_point_src, upper_point_src, 
						  lower_point_dst, upper_point_dst,
						  num_connections_per_neuron,
						  sub_region_size, sub_region_size_2, sub_region_size_3, ic_pt_type);
	else
		connect_neuron_layers (neuron_layer_src, neuron_layer_dst, 
					    lower_point_src, upper_point_src, 
					    lower_point_dst, upper_point_dst);
}



/*
*********************************************************************************
* Connect outputs                                                        	*
*********************************************************************************
*/

void
output_connect (NEURON_LAYER *neuron_layer_src, OUTPUT_DESC *output)
{
	if ((neuron_layer_src->dimentions.x != output->ww) || (neuron_layer_src->dimentions.y != output->wh))
		Erro ("Cannot connect a neuron layer to an output of different dimention", "", "");

	output->neuron_layer = neuron_layer_src;
}


 
/* 
********************************************************************************* 
* Associate neuron layers 							* 
********************************************************************************* 
*/ 
 
void 
associate_neurons (NEURON_LAYER *associated_neuron_layer, NEURON_LAYER *neuron_layer) 
{
	if (!same_dimentions (associated_neuron_layer->dimentions, neuron_layer->dimentions))
		Erro ("Cannot associate neuron layers of different dimentions", "", "");
	if (associated_neuron_layer->output_type != neuron_layer->output_type)
		Erro ("Cannot associate neuron layers with different output types", "", "");

	neuron_layer->associated_neuron_vector = associated_neuron_layer->neuron_vector;
}

