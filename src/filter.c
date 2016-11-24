/*! \file
* \brief Filter functions
*/

/*
*********************************************************************************
* Module : Filter functions							*
* version: 1.0									*
*    date: 13/02/2001								*
*      By: Alberto Ferreira de Souza						*
*********************************************************************************
*/

#include "filter.h"

#define LEVEL	0.4023594

extern int current_kernel;
double gauss_vector[TAM_GAUSS_VEC];
double integ_gauss[TAM_GAUSS_VEC];
double distance_vector[TAM_GAUSS_VEC];
double  y0_val;

static int *complex_cells_distribution = NULL;

/*
*********************************************************************************
* Section: Handler Functions for Basic Filter's Operations			*
*********************************************************************************
*/



/*!
*********************************************************************************
* \brief Update a filter output.
* \param[in] filter_descriptor The pointer to the filter descriptor.
* \pre The filter descriptor created.
* \post The filter output updated.
* \return Nothing.
*********************************************************************************
*/

void filter_update (FILTER_DESC *filter_descriptor)
{
	(*(filter_descriptor->filter)) (filter_descriptor);
}



/*!
*********************************************************************************
* \brief All filters update.
* \param[in] None.
* \pre All filter's descriptors created.
* \post The filter's outputs updated.
* \return Nothing.
*********************************************************************************
*/

void all_filters_update (void)
{
	FILTER_LIST *f_list;

	for (f_list = global_filter_list; f_list != (FILTER_LIST *) NULL; f_list = f_list->next)
		filter_update (f_list->filter_descriptor);
}



/*!
*********************************************************************************
* \brief Add filter to filter list.
* \param[in] filter Pointer to the filter function.
* \param[in] output Pointer to the output neuron layer.
* \param[in] neuron_layer_list Pointer to the neuron layer list.
* \param[in] param_list.
* \pre .
* \post .
* \return Nothing.
*********************************************************************************
*/

void add_filter_to_filter_list (void (*filter)(FILTER_DESC *),
	NEURON_LAYER *output, NEURON_LAYER_LIST *neuron_layer_list, PARAM_LIST *param_list)
{
	FILTER_LIST *f_list;

	if (global_filter_list == (FILTER_LIST *) NULL)
	{
		f_list = (FILTER_LIST *) alloc_mem ((size_t) sizeof (FILTER_LIST));
		f_list->filter_descriptor = (FILTER_DESC *) alloc_mem ((size_t) sizeof (FILTER_DESC));
		memset((void *) (f_list->filter_descriptor), 0, sizeof(FILTER_DESC));
		f_list->filter_descriptor->name = (char *) alloc_mem (strlen (output->name) + strlen ("_filter") + 1);
		strcpy (f_list->filter_descriptor->name, output->name);
		strcat (f_list->filter_descriptor->name, "_filter");
		f_list->filter_descriptor->output = output;
		f_list->filter_descriptor->neuron_layer_list = neuron_layer_list;
		f_list->filter_descriptor->filter = filter;
		f_list->filter_descriptor->filter_params = param_list;
		f_list->filter_descriptor->private_state = NULL;
		f_list->next = NULL;
		global_filter_list = f_list;
	}
	else
	{
		for (f_list = global_filter_list; f_list->next != (FILTER_LIST *) NULL;
			f_list = f_list->next)
			;

		f_list->next = (FILTER_LIST *) alloc_mem ((size_t) sizeof (FILTER_LIST));
		f_list->next->filter_descriptor = (FILTER_DESC *) alloc_mem ((size_t) sizeof (FILTER_DESC));
		memset((void *) (f_list->next->filter_descriptor), 0, sizeof(FILTER_DESC));
		f_list->next->filter_descriptor->name = (char *) alloc_mem (strlen (output->name) + strlen ("_filter") + 1);
		strcpy (f_list->next->filter_descriptor->name, output->name);
		strcat (f_list->next->filter_descriptor->name, "_filter");
		f_list->next->filter_descriptor->output = output;
		f_list->next->filter_descriptor->neuron_layer_list = neuron_layer_list;
		f_list->next->filter_descriptor->filter = filter;
		f_list->next->filter_descriptor->filter_params = param_list;
		f_list->next->filter_descriptor->private_state = NULL;
		f_list->next->next = NULL;
	}
}



/*!
*********************************************************************************
* Function: create_filter							*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void create_filter (void (*filter)(FILTER_DESC *), NEURON_LAYER *output, int num_layers, ...)
{
	va_list argument;
	int i;
	NEURON_LAYER_LIST *neuron_layer_list = NULL, *first_neuron_layer = NULL;
	PARAM_LIST *param_list = NULL;
	char *parameters = NULL;

	va_start (argument, num_layers);

	for (i = 0; i < num_layers; i++)
	{
		if (neuron_layer_list == NULL)
		{
			first_neuron_layer = neuron_layer_list = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
			neuron_layer_list->neuron_layer = (NEURON_LAYER *) va_arg (argument, NEURON_LAYER *);
			neuron_layer_list->next = NULL;
		}
		else
		{
			neuron_layer_list->next = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
			neuron_layer_list->next->neuron_layer = (NEURON_LAYER *) va_arg (argument, NEURON_LAYER *);
			neuron_layer_list->next->next = NULL;
			neuron_layer_list = neuron_layer_list->next;
		}
	}

	parameters = va_arg (argument, char*);

	param_list = get_param_list (parameters, &(argument));

	va_end (argument);

	add_filter_to_filter_list (filter, output, first_neuron_layer, param_list);
}



/*!
*********************************************************************************
* Function: get_filter_by_name							*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

FILTER_DESC *
get_filter_by_name (char *filter_name)
{
	FILTER_LIST *n_list;

	if (global_filter_list == NULL)
		return (NULL);
	else
		for (n_list = global_filter_list; n_list != (FILTER_LIST *) NULL;
			n_list = n_list->next)
			if (strcmp (n_list->filter_descriptor->name, filter_name) == 0)
				return (n_list->filter_descriptor);

	return (NULL);
}



/*!
*********************************************************************************
* Function: create_auxiliary_filter						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

FILTER_DESC *
create_auxiliary_filter (void (*filter)(FILTER_DESC *), NEURON_LAYER *output, int num_layers, ...)
{
	va_list argument;
	int i;
	NEURON_LAYER_LIST *neuron_layer_list = NULL, *first_neuron_layer = NULL;
	PARAM_LIST *param_list = NULL;
	FILTER_DESC *filter_descriptor;
	char *parameters = NULL;

	va_start (argument, num_layers);

	for (i = 0; i < num_layers; i++)
	{
		if (neuron_layer_list == NULL)
		{
			first_neuron_layer = neuron_layer_list = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
			neuron_layer_list->neuron_layer = (NEURON_LAYER *) va_arg (argument, NEURON_LAYER *);
			neuron_layer_list->next = NULL;
		}
		else
		{
			neuron_layer_list->next = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
			neuron_layer_list->next->neuron_layer = (NEURON_LAYER *) va_arg (argument, NEURON_LAYER *);
			neuron_layer_list->next->next = NULL;
			neuron_layer_list = neuron_layer_list->next;
		}
	}

	parameters = va_arg (argument, char*);

	param_list = get_param_list (parameters, &(argument));

	va_end (argument);

	filter_descriptor = (FILTER_DESC *) alloc_mem ((size_t) sizeof (FILTER_DESC));
	filter_descriptor->name = (char *) alloc_mem (strlen ("auxiliary_filter") + 1);
	strcpy (filter_descriptor->name, "auxiliary_filter");
	filter_descriptor->output = output;
	filter_descriptor->neuron_layer_list = first_neuron_layer;
	filter_descriptor->filter = filter;
	filter_descriptor->filter_params = param_list;
	filter_descriptor->private_state = NULL;

	return (filter_descriptor);
}



/*!
*********************************************************************************
* Function: create_auxiliary_neuron_layer					*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_LAYER *
create_auxiliary_neuron_layer (NEURON_TYPE *neuron_type, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type, int x_size, int y_size)
{
	NEURON_LAYER *neuron_layer;

	neuron_layer = (NEURON_LAYER *) alloc_mem (sizeof (NEURON_LAYER));
	neuron_layer->neuron_type = neuron_type;
	neuron_layer->dimentions.x = x_size;
	neuron_layer->dimentions.y = y_size;
	initialise_neuron_vector (neuron_layer, NEURON_MEMORY_SIZE);
	neuron_layer->associated_neuron_vector = NULL;
	neuron_layer->sensitivity = sensitivity;
	neuron_layer->output_type = output_type;
	return (neuron_layer);
}



/*
*********************************************************************************
* Section: Auxiliary Functions							*
*********************************************************************************
*/



/*!
*********************************************************************************
* Function: distance_from_image_center						*
* Description: Compute distance from the log-polar v to input image center	*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
distance_from_image_center (double wi, double hi, double w, double h, double u, double log_factor)
{
	double exp_val, x, y;

	if (((int) log_factor) == SERENO_MODEL)
	{
		x = 63.951256 * (u / (w / 2.0));
		y = pow ((20.05 / (0.26 * (C1 - x))), (1.0 / 0.26)) - 0.08;
		y = 30.0 * (y - y0_val) / (30.0 - y0_val);
		exp_val = (wi / 2.0) * (y / 30.0);
	}
	else
	{
		x = (u / (w / 2.0)) * log_factor;
		exp_val = (wi / 2.0) * (exp(log(log_factor) * (x - log_factor) / log_factor) - (1.0 / log_factor)) * (log_factor / (log_factor - 1.0));
	}

	return (exp_val);
}


/*!
*********************************************************************************
* Function: map_v1_to_image							*
* Description: Compute image's xi and yi using v1 v and u			*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void 
map_v1_to_image(int *xi, int *yi, double wi, double hi, double u, double v, double w, double h, double x_center, double y_center, double correction, double log_factor)
{
	// @@@ Alberto: Esta funcao so funciona direito com w impar. E deve ser deixada assim (use w impar!).
	double d, theta;

	correction = correction; //for keeping the compiler happy
	
	if (u < ((w - 1.0) / 2.0))
	{
		d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center(wi, hi, w, h, (w - 1.0) / 2.0 - u, log_factor);
		theta = pi * ((h * (3.0 / 2.0) - v) /  h) - pi / (2.0 * h);
	}
	else
	{
		d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center(wi, hi, w, h, u - (w - 1.0) / 2.0, log_factor);
		theta = pi * ((h * (3.0 / 2.0) + v) /  h) + pi / (2.0 * h);
	}

	*xi = (int) (d * cos(theta)) + x_center;
	*yi = (int) (d * sin(theta)) + y_center;
}


/*!
*********************************************************************************
* Function: map_image_to_v1							*
* Description: Compute image's xi and yi using v1 v and u			*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
map_image_to_v1(int xi, int yi, double wi, double hi, int *u, int *v, double w, double h, double x_center, double y_center, double correction, double log_factor)
{
	int closest_u, closest_v;	
	int xi_current, yi_current;	
	int distance_closest;
	int distance_current = 0;
	int u_current, v_current;	

	closest_u = 0;
	closest_v = 0;
	distance_closest = 10000000;

	for (u_current = 0; u_current < w; u_current++)
	{
		for (v_current = 0; v_current < h; v_current++)
		{
			map_v1_to_image (&xi_current, &yi_current, wi, hi, u_current, v_current, w, h, x_center, y_center, (double) h / (double) (h -1), log_factor);

			distance_current = ((xi - xi_current) * (xi - xi_current)) + ((yi - yi_current) * (yi - yi_current));

			if (distance_current < distance_closest)
			{
				distance_closest = distance_current;
				closest_u = u_current;
				closest_v = v_current;
			}
		}
	}
	*u = closest_u;
	*v = closest_v;
}


/*!
*********************************************************************************
* Function: map_v1_to_image_angle						*
* Description: Compute the log-polar mapped angle using v1 v and u		*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
map_v1_to_image_angle (int m, int n, int k, int j)
{
	double pi2n = (double) pi / (2 * n);

	if (k < m / 2)
		return (pi * ((double) n * (3.0 / 2.0) - ((double) j / n )) + pi2n);
	else
		return (pi * ((double) n * (3.0 / 2.0) + ((double) j / n )) + pi2n);
}


/*!
*********************************************************************************
* Function: compute_gaussian_kernel						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

void
compute_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int nKernelSize, float fltSigma)
{
	int x, y, num_points = 0;
	double k;
	POINT *points;
	int nRadius;

//	fltSigma = 3.0;
//	printf("nKernelSize: %d - fltSigma: %f\n", nKernelSize, fltSigma);
	points = (POINT *) alloc_mem (sizeof (POINT) * nKernelSize * nKernelSize);

//	k = 1.0 / sqrt (2.0 * pi * fltSigma * fltSigma);
	k = 1.0;
	nRadius = nKernelSize / 2;
	for (y = -nRadius; y <= nRadius; y++)
	{
		for (x = -nRadius; x <= nRadius; x++)
		{
			points[num_points].x = x;
			points[num_points].y = y;
			points[num_points].gaussian = k * exp (-(x * x + y * y) / (2.0 * fltSigma * fltSigma));;

			printf("# %1.03f ", points[num_points].gaussian);

			num_points++;
		}
		printf("\n");
	}

	receptive_field_descriptor->num_points = num_points;
	receptive_field_descriptor->points = points;
}



/*!
*********************************************************************************
* \brief Convolves the stimulus with the gaussian kernel.
* \param[in] receptive_field_descriptor The empty receptive field descriptor structure.
* \param[in] image_n_l The kernel preferred spacial frequency.
* \param[in] xo The Gabor band width.
* \param[in] yo The Gabor aspect ratio (width/height).
* \pre The receptive field descriptor created and initialized.
* \post Nothing.
* \return The convolution result.
*********************************************************************************
*/

float apply_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *image_n_l, int x0, int y0)
{
	int i;
	int xr, yr;
	float fltWeight, fltWeightSum = .0f, fltResult = .0f;

	for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		xr = x0 + receptive_field_descriptor->points[i].x;
		yr = y0 + receptive_field_descriptor->points[i].y;

		if ((xr < 0) || (xr >= image_n_l->dimentions.x) || (yr < 0) || (yr >= image_n_l->dimentions.y))
			continue;

                fltWeightSum += fltWeight = receptive_field_descriptor->points[i].gaussian;
		fltResult += fltWeight * image_n_l->neuron_vector[yr * image_n_l->dimentions.x + xr].output.fval;

	}

	fltResult = (fltWeightSum != .0f) ? fltResult / fltWeightSum : .0f;

	return (fltResult);
}


/*!
*********************************************************************************
* \brief Computes the V1 spatial receptive field modeled by a Gabor kernel.
* \param[in] receptive_field The empty receptive field descriptor structure.
* \param[in] frequency The kernel preferred spacial frequency.
* \param[in] width The Gabor band width.
* \param[in] aspect_ratio The Gabor aspect ratio (width/height).
* \param[in] orientation The Gabor orientation.
* \param[in] phase The Gabor phase.
* \param[in] kernel_type The kernel type dimentions (KERNEL_1D, KERNEL_2D).
* \pre The receptive field descriptor created.
* \post The V1 receptive field points initialized.
* \return Nothing.
*********************************************************************************
*/

void compute_v1_spatial_receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field, float frequency, float width, float aspect_ratio, float orientation, float phase, int kernel_type)
{
	int x, y, kernel_radius, kernel_size, num_points = 0, vertical_kernel_radius = 0;
	float k, u0, v0, a, b, xr, yr;
	POINT *kernel_points;

	FILE *points_file = NULL;
	char file_name[40];
	char strAux[256];
	int i;

	receptive_field->frequency = (double) frequency;

	// Calculates the gaussian's sigma
	a = frequency * width;
	b = a / aspect_ratio;

	// Calculates the kernel size
	kernel_size = (1.0 / a) * 2.0 * (1.5);
	kernel_size = (kernel_size % 2) ? kernel_size : (kernel_size + 1);

	sprintf (file_name, "kernel_%d.out", kernel_size);
	points_file = fopen (file_name, "w");

	// Allocates memory for the receptive field points
	if (receptive_field->points == NULL)
		kernel_points = (POINT *) alloc_mem (sizeof (POINT) * kernel_size * kernel_size);
	else
	{
		free (receptive_field->points);
		kernel_points = (POINT *) alloc_mem (sizeof (POINT) * kernel_size * kernel_size);
	}

	// Calculates the radius size of the receptive field
	kernel_radius = (kernel_size - 1) / 2;

	// Scale Constant
	k = 1.0;

	// Calculates the X and Y Frequency Components
	u0 = frequency * cos (orientation);
	v0 = frequency * sin (orientation);

	switch (kernel_type)
	{
                case KERNEL_1D:
                        vertical_kernel_radius = 0;
                        break;
                case KERNEL_2D:
                        vertical_kernel_radius = kernel_radius;
                        break;
                default:
                        Erro("Invalid kernel_type.", "", "");
        }

	for (y = - vertical_kernel_radius; y <= vertical_kernel_radius; y++)
	{
		for (x = - kernel_radius; x <= kernel_radius; x++)
		{
			// Saves the Point Coordinates
			kernel_points[num_points].x = x;
			kernel_points[num_points].y = y;

			// Calculates the Rotated Coordinates
			xr =   x * cos (orientation) + y * sin (orientation);
			yr = - x * sin (orientation) + y * cos (orientation);

			// Calculates the Gabor Function Only for Points Inside the Circle Enrolled in Kernel
			if ((x * x + y * y) <= (kernel_radius * kernel_radius))
				kernel_points[num_points].gabor = k * exp (- pi * (a * a * xr * xr + b * b * yr * yr)) *
							   (cos (2.0 * pi * (u0 * x + v0 * y) - phase) -
							    exp (- pi * (u0 * u0 / (a * a) + v0 * v0 / (b * b))) * cos (phase));
			else
				kernel_points[num_points].gabor = 0.0;

			// Calculates the Weight Function
			if ((x == - kernel_radius && y == - kernel_radius) || (x == kernel_radius && y == kernel_radius))
				kernel_points[num_points].weight = 1.0; // Points at Kernel Corner (Main Diagonal)
			else if ((x == - kernel_radius && y == kernel_radius) || (x == kernel_radius && y == - kernel_radius))
				kernel_points[num_points].weight = 2.0; // Points at Kernel Corner (Secondary Diagonal)
			else if (x == - kernel_radius || x == kernel_radius || y == - kernel_radius || y == kernel_radius)
				kernel_points[num_points].weight = 3.0; // Points on Kernel Edge
			else
				kernel_points[num_points].weight = 6.0; // Points inside Kernel

			sprintf(strAux, "%d %d %f\n", x, y, kernel_points[num_points].gabor);
			for (i = 0; strAux[i] != '\n'; i++)
				if (strAux[i] == ',') strAux[i] = '.';
			fprintf (points_file, "%s", strAux);

			// Increments the Points Number
			num_points++;
		}
		fprintf(points_file,"\n");
	}

	fclose (points_file);

	// Saves the Receptive Field Descriptor Attributes
	receptive_field->num_points		= num_points;
	receptive_field->points			= kernel_points;
	receptive_field->cut_orientation	= orientation;
	//receptive_field->area 		= num_points;
	receptive_field->area 			= 1.0; // Normalized area
}

float global_factor = 1.0;
float teste = 0.0;

/*!
*********************************************************************************
* Function: bidimentional_convolution						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT bidimentional_convolution (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON *image_n_l, int x_center, int y_center, int w, int h)
{
	int x_current, y_current, i, pixel, kernel_size;
	float intensity, red, green, blue, delta_area;
//	float mean, sigma, mean_comp, sigma_comp, n;
	NEURON_OUTPUT accumulator;

	// Gets the kernel size
	kernel_size = (int) sqrt ((double) receptive_field_descriptor->num_points);

	// Calculates the normalized delta area
	delta_area = receptive_field_descriptor->area / (float) (kernel_size * kernel_size);



	// Initializes the accumulator variable
	accumulator.fval = 0.0;
//	mean = sigma = 0.0;
/*	for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		// Calculates the current point
		x_current = x_center + receptive_field_descriptor->points[i].x;
		y_current = y_center + receptive_field_descriptor->points[i].y;

		// Verifies if the point is inside of the neuron layer bounds
		if ((x_current < 0) || (x_current >= w) || (y_current < 0) || (y_current >= h))
		{
			accumulator.fval = 0.0 ;
			return (accumulator);
		}

		// Gets the output pixel value
		pixel = image_n_l->neuron_vector[y_current * w + x_current].output.ival;

		// Extracts the red, green and blue components of the pixel
		red   = (float) RED   (pixel);
		green = (float) GREEN (pixel);
		blue  = (float) BLUE  (pixel);

		// Calculates the intensity value
		intensity = (red + green + blue) / 3.0;
		mean += intensity;
		sigma += intensity * intensity;
	}
	n = (float) receptive_field_descriptor->num_points;
	mean /= n;
	sigma = (sigma / n) - mean*mean;

#define SIGMA_CENTRAL 38.0
#define SIGMA_RANGE 0.45

	if (sigma != 0.0)
	{
		sigma = sqrtf ((n*sigma) / (n - 1));
		if ((sigma > (SIGMA_CENTRAL*SIGMA_RANGE)) && (sigma < (SIGMA_CENTRAL*(1.0+SIGMA_RANGE))))
			sigma_comp = SIGMA_CENTRAL / sigma;
		else
			sigma_comp = 1.0;
	}
	else
		sigma_comp = 1.0;
	mean_comp = 127.0 - mean;

	if ((teste != 0) && (teste < 1000))
	{
		printf ("mean = %f, sigma = %f, mean_comp = %f, sigma_comp = %f\n", mean, sigma, mean_comp, sigma_comp);
		teste++;
	}

*/
        for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		// Calculates the current point
		x_current = x_center + receptive_field_descriptor->points[i].x;
		y_current = y_center + receptive_field_descriptor->points[i].y;

		// Verifies if the point is inside of the neuron layer bounds
		if ((x_current < 0) || (x_current >= w) || (y_current < 0) || (y_current >= h))
		{
			//accumulator.fval = 0.0 ;
			//return (accumulator);
			continue;
		}

		// Gets the output pixel value
		pixel = image_n_l[y_current * w + x_current].output.ival;

		// Extracts the red, green and blue components of the pixel
		red   = (float) RED   (pixel);
		green = (float) GREEN (pixel);
		blue  = (float) BLUE  (pixel);

		// Calculates the intensity value
		intensity = teste + (global_factor * (red + green + blue)) / 3.0;
//		intensity = mean_comp + (sigma_comp * (red + green + blue)) / 3.0;

		// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
		accumulator.fval += receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor * intensity;
	}

	// Normalizes the result
	accumulator.fval *= delta_area / 6.0;

	return (accumulator);
}


/*!
*********************************************************************************
* Function: bidimentional_convolution_test_gabor				*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void bidimentional_convolution_test_gabor (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor)
{
	int i, kernel_size;
	float delta_area, accumulator;

	// Gets the kernel size
	kernel_size = (int) sqrt ((double) receptive_field_descriptor->num_points);

	// Calculates the normalized delta area
	delta_area = receptive_field_descriptor->area / (float) (kernel_size * kernel_size);

	// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
	accumulator = 0.0;
	for (i = 0; i < receptive_field_descriptor->num_points; i++)
		accumulator += receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor * 255.0;

	// Normalizes the result
	accumulator *= delta_area / 6.0;

	receptive_field_descriptor->min_dog = accumulator;
}


/*!
*********************************************************************************
* Function: compute_receptive_field_points					*
* Description: Compute the receptive points					*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_receptive_field_points (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int radius)
{
	int x, y, num_points = 0;
	double r, max_r;
	POINT *points;
	double si = 1.0/3.0; 	/* sigma of the big gaussian */
	double se = 1.0;	/* sigma of the small gaussian */
	double gb = 0.0, gs = 0.0, gd = 0.0;
	double cgb = 1.0, cgs = 1.0;
	double max_dog = 0.0;
	double min_dog = 0.0;

	cgb = se/si;
	points = (POINT *) alloc_mem (sizeof (POINT) * (2 * radius + 1) * (2 * radius + 1));
	max_r = sqrt ((double) (radius*radius + radius*radius));
	for (y = -radius; y < (radius + 1); y++)
	{
		for (x = -radius; x < (radius + 1); x++)
		{
			points[num_points].x = x;
			points[num_points].y = y;
			r = (3.0 * se * sqrt ((double) (x*x + y*y))) / max_r; 	/* r is normalized to 3 times the
										   sigma of the small gaussian */
			points[num_points].gaussian_big =   (cgb/sqrt(2.0*pi*si*si))*exp(-((r*r)/(2.0*si*si)));
			points[num_points].gaussian_small = (cgs/sqrt(2.0*pi*se*se))*exp(-((r*r)/(2.0*se*se)));
			gb += points[num_points].gaussian_big;
			gs += points[num_points].gaussian_small;
			gd += points[num_points].gaussian_big - points[num_points].gaussian_small;
			num_points++;
		}
	}
/*	printf ("radius %d, gb = %4.4f, gs = %4.4f, gd = %4.4f\n", radius, gb / (double) num_points, gs / (double) num_points, gd / (double) num_points);
*/	cgb = 0.1000 / (gb / (double) num_points);
	cgs = 0.1000 / (gs / (double) num_points);
	//cgd = 1.0/(gd / (double) num_points);
	gb = gs = gd = 0.0;
/*	printf ("radius %d, cgb = %4.4f, cgs = %4.4f, cgd = %4.4f\n", radius, cgb, cgs, cgd);
*/	for (x = 0; x < num_points; x++)
	{
		points[x].gaussian_big *= cgb;
		points[x].gaussian_small *= cgs;

/*		gb += points[x].gaussian_big;
		gs += points[x].gaussian_small;
		gd += points[x].gaussian_big - points[x].gaussian_small;
*/
		if (points[x].gaussian_big - points[x].gaussian_small > max_dog)
			max_dog = points[x].gaussian_big - points[x].gaussian_small;
		if (points[x].gaussian_big - points[x].gaussian_small < min_dog)
			min_dog = points[x].gaussian_big - points[x].gaussian_small;
	}
/*	printf ("radius %d, gb = %4.4f, gs = %4.4f, gd = %4.4f\n", radius, gb / (double) num_points, gs / (double) num_points, gd / (double) num_points);
*/	receptive_field_descriptor->num_points = num_points;
	receptive_field_descriptor->points = points;
	receptive_field_descriptor->max_dog = max_dog;
	receptive_field_descriptor->min_dog = min_dog;
}



/*!
*********************************************************************************
* Function: compute_receptive_field_descriptor					*
* Description: Compute the receptive descriptor					*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_receptive_field_descriptor (FILTER_DESC *filter, int wi, int hi, int w, int h)
{
	int v;

	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	int radius;

	y0_val = pow ((20.05/(0.26 * C1)), (1.0/0.26)) - 0.08;

	receptive_field_descriptor = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof (RECEPTIVE_FIELD_DESCRIPTION) * w/2);
	filter->private_state = (void *) receptive_field_descriptor;

	for (v = 1; v <= w/2; v++)
	{
		radius = (int) ((distance_from_image_center (wi, hi, w, h, v, SERENO_MODEL) - distance_from_image_center (wi, hi, w, h, v - 1, SERENO_MODEL))/2.0 + 0.5);
		if (radius <= 3)
			radius = 3;
		compute_receptive_field_points (&(receptive_field_descriptor[v-1]), radius);
	}
	filter->private_state = (void *) receptive_field_descriptor;
}



/*!
*********************************************************************************
* Function: ganglion_neuron_response						*
* Description: Return the ganglion neuron response as a function of the 	*
* 		receptive field 						*
* average									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT
ganglion_neuron_response (double receptive_filter_average, double accomodation, double rectification, OUTPUT_TYPE output_type)
{
	NEURON_OUTPUT response;

	// Response is the sigmoid of the receptive_filter_average times NUM_COLORS-1

	if (output_type == GREYSCALE_FLOAT)
		response.fval = 2.0 * (( 1.0 / (1.0 + exp (rectification - accomodation*receptive_filter_average))) - 0.5);
	else
		response.ival = (int) (((double) (NUM_GREYLEVELS - 1) / (1.0 + exp (rectification - accomodation*receptive_filter_average))) + 0.00005);
	return response;
}



/*!
*********************************************************************************
* Function: receptive_field							*
* Description: Compute the receptive average in a specific point x, y in the	*
* 		input at a distance d of the center of the retina		*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT
receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor,
		 INPUT_DESC *input, GLubyte *image, int x, int y, int w, int h, int index,
		 int cone_organization, int concetrically_region, int color_model, OUTPUT_TYPE output_type)
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_p;
	double receptive_field_average;
	int xr, yr, i, num_points;
	double intensity, yellow, rRGB, gRGB, bRGB, red, green, blue;
	double im, k, im_min, im_max;
	int pixel;

	im = 0.0;
	im_min = 512.0;
	im_max = -512.0;
	receptive_field_p = &(receptive_field_descriptor[index]);
	receptive_field_average = 0.0;
	num_points = receptive_field_p->num_points;

	for (i = 0; i < num_points; i++)
	{
		xr = x + receptive_field_p->points[i].x;
		yr = y + receptive_field_p->points[i].y;

		if (xr < 0)
			continue;
		if (xr >= w)
			continue;
		if (yr < 0)
			continue;
		if (yr >= h)
			continue;

		pixel = input->neuron_layer->neuron_vector[yr * w + xr].output.ival;

		rRGB = (double) RED (pixel);
		gRGB = (double) GREEN (pixel);
		bRGB = (double) BLUE (pixel);

		if(color_model == HSI)
		{
			k = 255.0;
			if ((rRGB + gRGB + bRGB) == 0)
			{
				red 	= 0.0;
				green 	= 0.0;
				blue 	= 0.0;
			}
			else
			{
				red 	= rRGB / (rRGB + gRGB + bRGB);
				green 	= gRGB / (rRGB + gRGB + bRGB);
				blue 	= bRGB / (rRGB + gRGB + bRGB);
			}
		}
		else
		{
			k = 1.0;
			red 	= rRGB;
			green 	= gRGB;
			blue 	= bRGB;
		}

		switch (cone_organization)
		{
			case MCELLS:
				intensity = (k * (red + green + blue) / 3.0);
				if (concetrically_region == ONCENTER)
				{
					im =
					 (receptive_field_p->points[i].gaussian_big - receptive_field_p->points[i].gaussian_small) * intensity;
				}
				else
				{
					im =
					 (receptive_field_p->points[i].gaussian_small - receptive_field_p->points[i].gaussian_big) * intensity ;
/*					output2->neuron_layer->neuron_vector[yr*w + xr].output.ival = (NEURON_OUTPUT) (((250.0 + (receptive_field_p->points[i].gaussian_small - receptive_field_p->points[i].gaussian_big)) / 270.0) * 7.0);
*/				}
/*				printf("%f\t%f\t%f\n",0.0,intensity,im);
*/				break;

			case RED_GREEN:
				if (concetrically_region == ONCENTER)
				{
					im = 	(red - green) *
						(receptive_field_p->points[i].gaussian_big -
						receptive_field_p->points[i].gaussian_small);

				}
				else
				{
					im = 	(red - green) *
						(- receptive_field_p->points[i].gaussian_big +
						receptive_field_p->points[i].gaussian_small);
				}
				break;
			case GREEN_RED:
				if (concetrically_region == ONCENTER)
				{
					im = 	(green - red)*
						(receptive_field_p->points[i].gaussian_big -
						receptive_field_p->points[i].gaussian_small);

				}
				else
				{
					im = 	(green - red)*
						(- receptive_field_p->points[i].gaussian_big +
						receptive_field_p->points[i].gaussian_small);
				}
				break;
			case BLUE_YELLOW:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
				{
					im = (blue - yellow) *
					 (receptive_field_p->points[i].gaussian_small -
					  receptive_field_p->points[i].gaussian_big);

				}
				else
				{
					im = (blue - yellow) *
					 (- receptive_field_p->points[i].gaussian_small +
					  receptive_field_p->points[i].gaussian_big);
				}
				break;
			case YELLOW_BLUE:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
				{
					im = (- blue + yellow) *
					 (receptive_field_p->points[i].gaussian_small -
					  receptive_field_p->points[i].gaussian_big);

				}
				else
				{
					im = (- blue + yellow) *
					 (- receptive_field_p->points[i].gaussian_small +
					  receptive_field_p->points[i].gaussian_big);
				}
				break;
		}
		receptive_field_average += im;
		if(im < im_min) im_min = im;
		if(im > im_max) im_max = im;
	}



	receptive_field_average = receptive_field_average / (double) (receptive_field_p->num_points);

	return (ganglion_neuron_response (receptive_field_average, 5.0, 1.0, output_type));
}


/*
*********************************************************************************
* Section: Basic Filters							*
*********************************************************************************
*/



/*!
*********************************************************************************
* Function: add_filter								*
* Description: 									*
* Inputs: Filter Descriptor							*
* Output: 				                			*
*********************************************************************************
*/

// Adds a filter layer to the application.
void add_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	NEURON_LAYER *filter_output;
	int wo, ho;
	int i;

	// Check the output type
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
	{
		if (n_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output of the neuron layers and the output of add_filter must be the same type.", "", "");
			return;
		}
	}

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	// Dimensao da 'output' associada ao filtro
	wo = filter_output->dimentions.x;
	ho = filter_output->dimentions.y;

	// Zera a saida do filtro
	for (i = 0; i < wo*ho; i++)
	{
		switch (filter_output->output_type)
		{
			case GREYSCALE:
			{
				filter_output->neuron_vector[i].output.ival = 0;
			}
			break;

			case GREYSCALE_FLOAT:
			{
				filter_output->neuron_vector[i].output.fval = 0.0;
			}
			break;
		}
	}

	// Percorre as neuron layers somando ( a saída da camada neural associada à esta )
	for (n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next)
	{
		nl = n_list->neuron_layer;

		for (i = 0; i < wo*ho; i++)
		{
			switch (filter_output->output_type)
			{
				case GREYSCALE:
				{
					filter_output->neuron_vector[i].output.ival += nl->neuron_vector[i].output.ival;
				}
				break;

				case GREYSCALE_FLOAT:
				{
					filter_output->neuron_vector[i].output.fval += nl->neuron_vector[i].output.fval;
				}
				break;
			}
		}
	}
}



/*!
*********************************************************************************
* Function: minus_filter
* Description: Computes the vector difference between the n input neuron layers.
* dif = sqrt(SUM((nl[i] - nl[i + (n / 2)])^2))
* Inputs:
* Output:
*********************************************************************************
*/

void
minus_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	int i, j;
	int num_neurons, numNL, dif;
	float dif_float, dif_accumulator;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		if ((numNL % 2) != 0)
		{
			Erro ("Wrong number of neuron layers. The minus_filter must be applied on a even number of neuron layers.", "", "");
			return;
		}

		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		{
			if (n_list->next != NULL)
			{
				if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
				{
					Erro ("The minus_filter must be applied on neuron layers with the same output_type.", "", "");
					return;
				}
			}
		}

		nl = (NEURON_LAYER**)alloc_mem(sizeof(NEURON_LAYER*)*numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void*)nl;
	}
	else
	{
		nl = (NEURON_LAYER**)filter_desc->private_state;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	switch (filter_desc->output->output_type)
	{
		case COLOR:
		case GREYSCALE:
		{
			for (i = 0; i < num_neurons; i++)
			{
				dif_accumulator = 0.0;
				for (j = 0; j < (numNL / 2); j++)
				{
					dif = nl[j]->neuron_vector[i].output.ival - nl[j + (numNL / 2)]->neuron_vector[i].output.ival;
					dif_accumulator += (float)(dif*dif);
				}
				filter_desc->output->neuron_vector[i].output.ival = (int)sqrt(dif_accumulator);
			}
		}
		break;

		case GREYSCALE_FLOAT:
		{
			for (i = 0; i < num_neurons; i++)
			{
				dif_accumulator = 0.0;
				for (j = 0; j < (numNL / 2); j++)
				{
					dif_float = nl[j]->neuron_vector[i].output.fval - nl[j + (numNL / 2)]->neuron_vector[i].output.fval;
					dif_accumulator += (dif_float*dif_float);
				}
				filter_desc->output->neuron_vector[i].output.fval = sqrt(dif_accumulator);
			}
		}
		break;

		default:
		{
			for (i = 0; i < num_neurons; i++)
			{
				dif_accumulator = 0.0;
				for (j = 0; j < (numNL / 2); j++)
				{
					dif = nl[j]->neuron_vector[i].output.ival - nl[j + (numNL / 2)]->neuron_vector[i].output.ival;
					if (dif > 0)
						dif = NUM_COLORS - 1;
					else
						dif = 0;
					dif_accumulator += (float)(dif*dif);
				}
				filter_desc->output->neuron_vector[i].output.ival = (int)sqrt(dif_accumulator);
			}
		}
		break;
	}
}



/*!
*********************************************************************************
* Function: divide_outputs							*
* Description: 									*
* Inputs: Filter Descriptor							*
* Output: 				                			*
*********************************************************************************
*/

void
divide_outputs (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *image_n_l[2] = {NULL, NULL};
	int i, ho, wo, hi, wi, x, y;
	float share, divider, quotient;

	// Checks Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. sum_square_outputs must be applied on 2 neuron layers.", "", "");
		return;
	}

	// Gets the Inputs Neuron Layers
	image_n_l[0] = filter_desc->neuron_layer_list->neuron_layer;
	image_n_l[1] = filter_desc->neuron_layer_list->next->neuron_layer;

	// Gets the Inputs Neuron Layers Dimentions
	wi  = image_n_l[0]->dimentions.x;
	hi  = image_n_l[0]->dimentions.y;

	// Gets the Output Neuron Layer Dimentions
	wo  = filter_desc->output->dimentions.x;
	ho  = filter_desc->output->dimentions.y;

	// Checks if the Inputs and Output have the same size
	if ((wi != wo) || (hi != ho))
	{
		Erro ("Size of inputs and output of the neuron_layer must be the same.", "", "");
		return;
	}

	for (x = 0; x < wo; x++)
	{
		for (y = 0; y < ho; y++)
		{
			// Gets the Division Operands
			share   = image_n_l[0]->neuron_vector[(y * wo) + x].output.fval;
			divider = image_n_l[1]->neuron_vector[(y * wo) + x].output.fval;

			// Checks the Possibilities
			quotient = (divider == 0) ? ((share >= 0) ? FLT_MAX : FLT_MIN) : share / divider;

			// Stores the Result
			filter_desc->output->neuron_vector[(y * wo) + x].output.fval = quotient;
		}
	}
}



/*!
*********************************************************************************
* Function: sum_square_outputs							*
* Description: 									*
* Inputs: Filter Descriptor							*
* Output: 				                			*
*********************************************************************************
*/

void
sum_square_outputs (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *image_n_l[2] = {NULL, NULL};
	int i, ho, wo, hi, wi, x, y;

	// Check Neuron Layer
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. sum_square_outputs must be applied on 2 neuron layers.", "", "");
		return;
	}

	// Get Neuron Layer
	image_n_l[0] = filter_desc->neuron_layer_list->neuron_layer;
	image_n_l[1] = filter_desc->neuron_layer_list->next->neuron_layer;

	// Get Image Dimentions
	wi = image_n_l[0]->dimentions.x;
	hi = image_n_l[0]->dimentions.y;
	wo  = filter_desc->output->dimentions.x;
	ho  = filter_desc->output->dimentions.y;

	if ((wi != wo) || (hi != ho))
	{
		Erro ("Size of inputs and output of the neuron_layer must be the same.", "", "");
		return;
	}

	for (x = 0; x < wo; x++)
	{
		for (y = 0; y < ho; y++)
		{
			filter_desc->output->neuron_vector[(y * wo) + x].output.fval =
				image_n_l[0]->neuron_vector[(y * wo) + x].output.fval * image_n_l[0]->neuron_vector[(y * wo) + x].output.fval +
				image_n_l[1]->neuron_vector[(y * wo) + x].output.fval * image_n_l[1]->neuron_vector[(y * wo) + x].output.fval;
		}
	}
}



/*!
*********************************************************************************
* Function: delay_filter							*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
delay_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_neuron = NULL;
	long num_neurons;
	int i;

	for (i = 0, n_neuron = filter_desc->neuron_layer_list; n_neuron != NULL; n_neuron = n_neuron->next, i++)
		;
	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. Delay filter must be applied on two neuron layers.", "", "");
		return;
	}
	num_neurons = get_num_neurons (filter_desc->output->dimentions);
	memcpy(filter_desc->output->neuron_vector, filter_desc->neuron_layer_list->neuron_layer->neuron_vector, (size_t) (num_neurons * sizeof (NEURON)));
}



/*!
*********************************************************************************
* Function: graph_filter							*
* Description: check the filter parameters and set the glut handle functions of *
* 	       the graph output window						*
* Inputs: filter decriptor of the graph filter					*
* Output:									*
*********************************************************************************
*/

void
graph_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = (NEURON_LAYER_LIST *) NULL;
	PARAM_LIST *p_list = (PARAM_LIST *) NULL;
	int i;

	// Check neuron layers number
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list->next != NULL; n_list = n_list->next, i++);

	if (i == 0)
	{
		Erro ("Wrong number of neuron layers. Graph filter needs at least one neuron layer.", "", "");
		return;
	}

	// Check parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 5)
	{
		Erro ("Wrong number of parameters. Graph filter needs four parameters.", "[x_min,x_max] [y_min,y_max].", "");
		return;
	}


#ifndef NO_INTERFACE
	// Get the filter output
	OUTPUT_DESC *output = (OUTPUT_DESC *) NULL;
	output = get_output_by_neural_layer (filter_desc->output);

	// Set the handle functions for the graph output
	glutSetWindow (output->win);
	glutDisplayFunc (graph_display);
	glutPassiveMotionFunc (graph_passive_motion);
	glutReshapeFunc (graph_reshape);
	glutMouseFunc (graph_mouse); // Still doing nothing
#endif
}



/*!
*********************************************************************************
* Function: merge_filter							*
* Description: conbine the filter inputs producing a output			*
* Inputs: filter decriptor of the merge filter					*
* Output:									*
*********************************************************************************
*/

void
merge_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = (NEURON_LAYER_LIST *) NULL;
	PARAM_LIST *p_list = (PARAM_LIST *) NULL;
	NEURON_LAYER *nl_input = (NEURON_LAYER *) NULL;
	NEURON_LAYER *nl_output = (NEURON_LAYER *) NULL;
	int nl_number;
	int i, j;
	int wi, hi;
	int wo, ho;
	double x_scale, y_scale;
	double x_init, x_band, x;

	// Check neuron layers number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
		;

	if (nl_number == 0)
	{
		Erro ("Wrong number of neuron layers. Merge filter needs at least one neuron layer.", "", "");
		return;
	}

	// Check parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of parameters. Merge filter needs 1 parameters.", "", "");
		return;
	}

	// Get the filter output
	nl_output = filter_desc->output;
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	for (x_init = 0.0, x_band = (double) wo/ (double) nl_number, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, x_init += x_band)
	{
		nl_input = n_list->neuron_layer;
		wi = nl_input->dimentions.x;
		hi = nl_input->dimentions.y;

		x_scale = (double) wi / x_band;
		y_scale = (double) hi / (double) ho;

		for (j = 0; j < ho; j++)
		{
			for (x = 0.0; x < x_band; x++)
			{
				nl_output->neuron_vector[(int) (x_init + x + 0.5) + j * wo].output =
					nl_input->neuron_vector[(int) (x * x_scale) + (int) ((double) j * y_scale) * wi].output;
			}
		}
	}
}



/*!
*********************************************************************************
* Function: map_image_v1							*
* Description: map the image to v1 cortex        				*
* Inputs: filter_desc - Filter description					*
* Output:									*
*********************************************************************************
*/

void
map_image_v1 (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *n_l = NULL;
	INPUT_DESC *input = NULL;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi ;
	int x_center, y_center;
	float log_factor;
	NEURON_OUTPUT previous_output;

	previous_output.ival = 0;

        // Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. Map Image V1 filter must be applied on only one neuron layer.", "", "");
		return;
	}

	n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Check Param
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 3)
	{
		Erro ("Wrong number of parameters. Map Image V1 needs two parameters: <input name>, <log_factor>.", "", "");
		return;
	}

	input = get_input_by_name (filter_desc->filter_params->next->param.sval);
	log_factor = filter_desc->filter_params->next->next->param.fval;

	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = wi / 2;
		y_center = hi / 2;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h -1), log_factor);

			//index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
				if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					previous_output.ival = filter_desc->output->neuron_vector[(v * w) + u].output.fval = 0;
				else
					previous_output = filter_desc->output->neuron_vector[(v * w) + u].output = n_l->neuron_vector[yi * wi + xi].output;
			previous_xi = xi;
			previous_yi = yi;
		}
	}
}


/*!
*********************************************************************************
* Function: map_image_v1_d							*
* Description: map the image to v1 cortex (double)        				*
* Inputs: filter_desc - Filter description					*
* Output:									*
*********************************************************************************
*/
void
map_v1_to_image_d(double *xi, double *yi, double wi, double hi, double u, double v, double w, double h, double x_center, double y_center, double correction, double log_factor)
{
    // @@@ Alberto: Esta funcao so funciona direito com w impar. E deve ser deixada assim (use w impar!).
    double d, theta;

    correction = correction; //for keeping the compiler happy
   
    if (u < ((w - 1.0) / 2.0))
    {
        d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center(wi, hi, w, h, (w - 1.0) / 2.0 - u, log_factor);
        theta = pi * ((h * (3.0 / 2.0) - v) /  h) - pi / (2.0 * h);
    }
    else
    {
        d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center(wi, hi, w, h, u - (w - 1.0) / 2.0, log_factor);
        theta = pi * ((h * (3.0 / 2.0) + v) /  h) + pi / (2.0 * h);
    }

    *xi = (double)((d * cos(theta)) + x_center);
    *yi = (double)((d * sin(theta)) + y_center);
}


/*!
*********************************************************************************
* Function: scale_filter							*
* Description: 									*
* Inputs: Filter Descriptor							*
* Output: 				                			*
*********************************************************************************
*/

void
scale_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *nl_input = NULL;
	int i, ho, wo, hi, wi, xo, yo, xi, yi;
	float scale_factor_x, scale_factor_y;

	// Checks Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. Scale filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Inputs Neuron Layers
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Check Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	// Gets the Input Neuron Layer Dimentions
	wi  = nl_input->dimentions.x;
	hi  = nl_input->dimentions.y;

	// Gets the Output Neuron Layer Dimentions
	wo  = filter_desc->output->dimentions.x;
	ho  = filter_desc->output->dimentions.y;

	scale_factor_x = (float) wi / (float) wo;
	scale_factor_y = (float) hi / (float) ho;

	for (xo = 0; xo < wo; xo++)
	{
		xi = scale_factor_x * xo;

		for (yo = 0; yo < ho; yo++)
		{
			yi = scale_factor_y * yo;

			filter_desc->output->neuron_vector[(yo * wo) + xo].output = nl_input->neuron_vector[(yi * wi) + xi].output;
		}
	}
}



/*
*********************************************************************************
* Section: Consolidate Filters 							*
*********************************************************************************
*/



/*!
*********************************************************************************
* Function: v1_filter								*
* Description: apply the V1 layer's filter at the neuron layer			*
* Inputs: filter_desc - Filter description					*
* Output: 						 			*
*********************************************************************************
*/

void
v1_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *n_l = NULL;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi, color_model, index;
	int cone_organization, concetrically_region;
	int x_center, y_center;
	GLubyte *image = NULL;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;

	previous_output.ival = 0;
	/* Check Neuron Layers */
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. V1 filter must be applied on one neuron layer.", "", "");
		return;
	}

	/* Check Params */
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 4)
	{
		Erro ("Wrong number of parameters. V1 filter needs three parameters.",
		"The organization of cone inputs, Concetrically organized regions and the Color Model, respectivelly.", "");
		return;
	}

	cone_organization = filter_desc->filter_params->next->param.ival;
	concetrically_region = filter_desc->filter_params->next->next->param.ival;
	color_model = filter_desc->filter_params->next->next->next->param.ival;
	n_l = filter_desc->neuron_layer_list->neuron_layer;

	image = get_image_via_neuron_layer (n_l);
	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	if (filter_desc->private_state == NULL)
		compute_receptive_field_descriptor (filter_desc, wi, hi, w, h);

	input = get_input_by_neural_layer (n_l);

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			//map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h -1), SERENO_MODEL);
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h -1), 2.0);

			index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
				if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					if (filter_desc->output->output_type == GREYSCALE_FLOAT)
						previous_output.fval = filter_desc->output->neuron_vector[(v * w) + u].output.fval = 0.0;
					else
						previous_output.ival = filter_desc->output->neuron_vector[(v * w) + u].output.ival = 0;
				else
					previous_output = filter_desc->output->neuron_vector[(v * w) + u].output =
						receptive_field ((RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state, input, image,
						xi, yi, wi, hi, index, cone_organization, concetrically_region, color_model,
						filter_desc->output->output_type);
			previous_xi = xi;
			previous_yi = yi;
		}
	}
}



/*!
*********************************************************************************
* Function: te_filter								*
* Description: Tunned exitatory: the output is true if a point in one input 	*
* is equal to the point in the same position in the other input			*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
te_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l1 = NULL;
	NEURON_LAYER *n_l2 = NULL;
	int i, num_neurons;
	int x_disp, y_disp, x, y, h, w, x1, y1, x2, y2;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. TE filter requires exactly two neuron layers.", "", "");
		return;
	}

	n_l1 = filter_desc->neuron_layer_list->neuron_layer;
	n_l2 = filter_desc->neuron_layer_list->next->neuron_layer;
	if (n_l1->output_type != n_l2->output_type)
	{
		Erro ("Neuron layers has two differents output type.", "TE filter must be applied two neuron layers of the same output_type.", "");
		return;
	}

	if (filter_desc->output->output_type != n_l1->output_type)
	{
		Erro ("The output type of the neuron layers are diffents of the filter output.", "", "");
		return;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	if (filter_desc->filter_params->next == NULL)
	{
		if (filter_desc->output->output_type == GREYSCALE_FLOAT)
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.fval = (n_l1->neuron_vector[i].output.fval + n_l2->neuron_vector[i].output.fval) / 2.0;
		else
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival = (n_l1->neuron_vector[i].output.ival + n_l2->neuron_vector[i].output.ival) / 2;
	}
	else
	{
		x_disp = filter_desc->filter_params->next->param.ival;
		if (filter_desc->filter_params->next->next != NULL)
			y_disp = filter_desc->filter_params->next->next->param.ival;
		else
			y_disp = 0;

		w = filter_desc->output->dimentions.x;
		h = filter_desc->output->dimentions.y;
		for(y = 0; y < h; y++)
		{
			y1 = y - y_disp;
			y2 = y + y_disp;
			for(x = 0; x < w; x++)
			{
				x1 = x - x_disp;
				x2 = x + x_disp;
				if (((x1 >= 0) && (x1 < w)) &&
				    ((x2 >= 0) && (x2 < w)) &&
				    ((y1 >= 0) && (y1 < h)) &&
			    	    ((y2 >= 0) && (y2 < h)))
				{
					if (filter_desc->output->output_type == GREYSCALE_FLOAT)
						filter_desc->output->neuron_vector[x+y*w].output.fval =
							(n_l1->neuron_vector[x1+y1*w].output.fval + n_l2->neuron_vector[x2+y2*w].output.fval) / 2;
					else
						filter_desc->output->neuron_vector[x+y*w].output.ival =
							(n_l1->neuron_vector[x1+y1*w].output.ival + n_l2->neuron_vector[x2+y2*w].output.ival) / 2;
				}
				else
					if (filter_desc->output->output_type == GREYSCALE_FLOAT)
						filter_desc->output->neuron_vector[x+y*w].output.fval = 0.0;
					else
						filter_desc->output->neuron_vector[x+y*w].output.ival = 0;
			}
		}
	}
}



/*!
*********************************************************************************
* Function: ti_filter								*
* Description: Tunned inhibitory: the output is true if a point in one input 	*
* is different to the point in the same position in the other input									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
ti_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l1 = NULL;
	NEURON_LAYER *n_l2 = NULL;
	int i, num_neurons;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. Ti filter must be applied on two neuron layers.", "", "");
		return;
	}

	n_l1 = filter_desc->neuron_layer_list->neuron_layer;
	n_l2 = filter_desc->neuron_layer_list->next->neuron_layer;
	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	switch (filter_desc->output->output_type)
	{
		case COLOR:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival   =
					(NEURON_MASK_COLOR(n_l1->neuron_vector[i].output.ival) != NEURON_MASK_COLOR(n_l1->neuron_vector[i].output.ival)) ? (NUM_COLORS - 1) : 0;
			break;
		case GREYSCALE:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival   =
					(NEURON_MASK_GREYSCALE(n_l1->neuron_vector[i].output.ival) != NEURON_MASK_GREYSCALE(n_l1->neuron_vector[i].output.ival)) ?(NUM_GREYLEVELS - 1) : 0;
			break;
		case GREYSCALE_FLOAT:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.fval   =
					(n_l1->neuron_vector[i].output.fval != n_l1->neuron_vector[i].output.fval) ? (float) (NUM_GREYLEVELS - 1) : 0.0;
			break;
		case BLACK_WHITE:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival =
					(NEURON_MASK_BLACK_WHITE(n_l1->neuron_vector[i].output.ival) != NEURON_MASK_BLACK_WHITE(n_l2->neuron_vector[i].output.ival)) ? (NUM_GREYLEVELS - 1) : 0;
			break;
	}
}



/*!
*********************************************************************************
* Function: 									*
* Description: Tunned near tunned far filter: the output is true if a point in 	*
* one input is equal to the point in the contralateral position of the other 	*
* input										*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
tf_tn_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l1 = NULL;
	NEURON_LAYER *n_l2 = NULL;
	int i, j, h, w, dif;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. Tf filter must be applied on two neuron layers.", "", "");
		return;
	}

	n_l1 = filter_desc->neuron_layer_list->neuron_layer;
	n_l2 = filter_desc->neuron_layer_list->next->neuron_layer;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
			filter_desc->output->neuron_vector[(i * w) + j].output.ival =
				((dif = n_l1->neuron_vector[(i * w) + j].output.ival - n_l2->neuron_vector[(i * w) + ((w - 1) - j)].output.ival) >= 0)?dif:-dif;
	}
}


/*!
*********************************************************************************
* \brief The energy filter with biological parameter specification.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void biological_energy_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *image_n_l = NULL;
	RECEPTIVE_FIELD_DESCRIPTION **receptive_field;
	int i, ho, wo, hi, wi, xi, yi, xo, yo, u, v;
	float camera_opening, cycles_per_degree, width, aspect_ratio, orientation, phase, log_factor;
	float frequency;
	double correction, a, b;
	int mapping, shift, kernel_type;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 11)
	{
		Erro ("Wrong number of parameters for %s.", filter_desc->name, "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase			= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift			= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type             = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.ival;

	// Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. %s must be applied on only one neuron layer.", filter_desc->name, "");
		return;
	}

	// Gets the Input Neuron Layers
	image_n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = image_n_l->dimentions.x;
	hi = image_n_l->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

	// Previous output fval initialize
	previous_output.fval = 0.0f;

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION **) alloc_mem (2 * sizeof(RECEPTIVE_FIELD_DESCRIPTION *));
		receptive_field[0] = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		receptive_field[1] = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		// Calculates the spatial frequency (cycles per pixel)
		frequency = (1.0 / (float) wi) * camera_opening * cycles_per_degree;

		receptive_field[0]->points = NULL;
		compute_v1_spatial_receptive_field (receptive_field[0], frequency, width, aspect_ratio, orientation, phase, kernel_type);
		receptive_field[1]->points = NULL;
		compute_v1_spatial_receptive_field (receptive_field[1], frequency, width, aspect_ratio, orientation, phase + pi/2.0, kernel_type);
		filter_desc->private_state = (void *) receptive_field;

		// Calculates the minimum value of the result of a gabor filter
		bidimentional_convolution_test_gabor (receptive_field[0]);
		bidimentional_convolution_test_gabor (receptive_field[1]);
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION **)filter_desc->private_state;

	input = get_input_by_neural_layer (image_n_l);

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	switch (mapping)
	{
		case MAP_PLAN:
		{
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) (xo * wi) / (float) wo + 0.5) + x_center - wi/2 + shift;

				for (yo = 0; yo < ho; yo++)
				{
					yi = (int) ((float) (yo * hi) / (float) ho + 0.5) + y_center - hi/2;

					a = bidimentional_convolution (receptive_field[0], image_n_l->neuron_vector, xi, yi, wi, hi).fval;
					b = bidimentional_convolution (receptive_field[1], image_n_l->neuron_vector, xi, yi, wi, hi).fval;
					filter_desc->output->neuron_vector[(yo * wo) + xo].output.fval = sqrt(a*a + b*b);
				}
			}
		}
		break;

		case MAP_LOG_POLAR:
		{
			previous_xi = -1;
			previous_yi = -1;

			for (u = 0; u < wo; u++)
			{
				for (v = 0; v < ho; v++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, wo, ho, x_center, y_center, correction, log_factor);
					xi += shift;

					//index = (u < (wo / 2)) ? ((wo / 2) - 1 - u) : (u - (wo / 2));
					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[(v * wo) + u].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = 0.0;
					else
					{
						a = bidimentional_convolution (receptive_field[0], image_n_l->neuron_vector, xi, yi, wi, hi).fval;
						b = bidimentional_convolution (receptive_field[1], image_n_l->neuron_vector, xi, yi, wi, hi).fval;
						previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = sqrt(a*a + b*b);
					}
					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
	}
}



void
optimized_2d_point_convolution_greyscale_float (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type)
{
	int i, xi, yi, hi, wi, xo, yo, ho, wo;
	float wi_wo_factor, hi_ho_factor, accumulator;
	int input_line, output_line, input_colunm, intensity_vector_size;
	float *intensity;
	float *intensity_vector;
	float *intensity_line;
	int kernel_size, kernel_div_2;
	NEURON *nv_input = NULL, *nv_output = NULL;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	wi_wo_factor = (float) wi / (float) wo;
	hi_ho_factor = (float) hi / (float) ho;

	nv_input  = nl_input->neuron_vector;
	nv_output = nl_output->neuron_vector;

	if (kernel_type == KERNEL_2D)
	{
		kernel_size = (int) sqrtf (num_points);
		kernel_div_2 = kernel_size >> 1;

		intensity_vector_size = ho * kernel_size + kernel_size * kernel_size + wo;
		intensity_vector = (float *) alloc_mem (intensity_vector_size * sizeof (float));

		for (i = 0; i < intensity_vector_size; i++)
			intensity_vector[i] = .0f;

		intensity = &(intensity_vector[kernel_div_2 * kernel_size]);

		/* These nested loops copy a column of the input, of width kernel_size-1, to the the vector intensity */
		xi = 0;
		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line  = yi * wi;
			output_line = yo * kernel_size;

			for (i = 0; i < (kernel_size - 1); i++)
			{
				input_colunm = xi + i - kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
					intensity[output_line + i] = nv_input[input_line + input_colunm].output.fval;
			}
		}

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo * wi_wo_factor + .5f);

			/* This loop copies a column of the input, of width 1, to the vector intensity; this completes a column
			   of the image of width kernel_size */
			intensity = &(intensity_vector[kernel_div_2 * kernel_size + xo]);
			intensity[- 1] = .0f;
			for (yo = 0; yo < ho; yo++)
			{
				yi = (int) ((float) yo * hi_ho_factor + .5f);
				input_line  = yi * wi;
				output_line = yo * kernel_size;
				input_colunm = xi + kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
					intensity[output_line + kernel_size - 1] = nv_input[input_line + input_colunm].output.fval;
				else
					intensity[output_line + kernel_size - 1] = .0f;
			}

			/* This loop computes the convolution in one column */
			intensity = &(intensity_vector[xo]);
			for (yo = 0; yo < ho; yo++)
			{
				output_line = yo * wo;

				accumulator = .0f;

				/* This is the most time consuming loop */
				for (i = 0; i < kernel_size * kernel_size; i++)
					accumulator += kernel_points[i] * intensity[i];

				// Normalizes the result
				accumulator *= normalizer;
				nv_output[output_line + xo].output.fval = accumulator;

				intensity += kernel_size;
			}
		}
		free (intensity_vector);
	}
	else
	{
		kernel_size = num_points; /* only for 1D kernels */
		kernel_div_2 = kernel_size >> 1;

		intensity_line = (float *) alloc_mem (sizeof (float) * (wo + kernel_size));
		for (i = 0; i < wo + kernel_size; i++)
			intensity_line[i] = 0.0;

		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line = yi * wi;
			intensity = &(intensity_line[kernel_div_2]);
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) xo * wi_wo_factor + 0.5);

				intensity[xo] = nv_input[input_line + xi].output.fval;
			}

			intensity = intensity_line;
			for (xo = 0; xo < wo; xo++)
			{
				accumulator = 0.0;
        			for (i = 0; i < kernel_size; i++)
				{
					// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
					accumulator += kernel_points[i] * intensity[i];
				}

				// Normalizes the result
				accumulator *= normalizer;
				nv_output[yo * wo + xo].output.fval = accumulator;
				intensity = intensity + 1;
			}
		}
		free (intensity_line);
	}
}



void
optimized_2d_point_convolution_greyscale (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type)
{
	int i, xi, yi, hi, wi, xo, yo, ho, wo;
	float wi_wo_factor, hi_ho_factor, accumulator;
	int input_line, output_line, input_colunm, intensity_vector_size;
	float *intensity;
	float *intensity_vector;
	float *intensity_line;
	int kernel_size, kernel_div_2;
	NEURON *nv_input = NULL, *nv_output = NULL;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	wi_wo_factor = (float) wi / (float) wo;
	hi_ho_factor = (float) hi / (float) ho;

	nv_input  = nl_input->neuron_vector;
	nv_output = nl_output->neuron_vector;

	if (kernel_type == KERNEL_2D)
	{
		kernel_size = (int) sqrtf (num_points);
		kernel_div_2 = kernel_size >> 1;

		intensity_vector_size = ho * kernel_size + kernel_size * kernel_size + wo;
		intensity_vector = (float *) alloc_mem (intensity_vector_size * sizeof (float));

		for (i = 0; i < intensity_vector_size; i++)
			intensity_vector[i] = .0f;

		intensity = &(intensity_vector[kernel_div_2 * kernel_size]);

		/* These nested loops copy a column of the input, of width kernel_size-1, to the the vector intensity */
		xi = 0;
		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line  = yi * wi;
			output_line = yo * kernel_size;

			for (i = 0; i < (kernel_size - 1); i++)
			{
				input_colunm = xi + i - kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
					intensity[output_line + i] = (float) nv_input[input_line + input_colunm].output.ival;
			}
		}

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo * wi_wo_factor + .5f);

			/* This loop copies a column of the input, of width 1, to the vector intensity; this completes a column
			   of the image of width kernel_size */
			intensity = &(intensity_vector[kernel_div_2 * kernel_size + xo]);
			intensity[- 1] = .0f;
			for (yo = 0; yo < ho; yo++)
			{
				yi = (int) ((float) yo * hi_ho_factor + .5f);
				input_line  = yi * wi;
				output_line = yo * kernel_size;
				input_colunm = xi + kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
					intensity[output_line + kernel_size - 1] = (float) nv_input[input_line + input_colunm].output.ival;
				else
					intensity[output_line + kernel_size - 1] = .0f;
			}

			/* This loop computes the convolution in one column */
			intensity = &(intensity_vector[xo]);
			for (yo = 0; yo < ho; yo++)
			{
				output_line = yo * wo;

				accumulator = .0f;

				/* This is the most time consuming loop */
				for (i = 0; i < kernel_size * kernel_size; i++)
					accumulator += kernel_points[i] * intensity[i];

				// Normalizes the result
				accumulator *= normalizer;
				nv_output[output_line + xo].output.ival = (int) (accumulator + 0.5);

				intensity += kernel_size;
			}
		}
		free (intensity_vector);
	}
	else
	{
		kernel_size = num_points; /* only for 1D kernels */
		kernel_div_2 = kernel_size >> 1;

		intensity_line = (float *) alloc_mem (sizeof (float) * (wo + kernel_size));
		for (i = 0; i < wo + kernel_size; i++)
			intensity_line[i] = 0.0;

		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line = yi * wi;
			intensity = &(intensity_line[kernel_div_2]);
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) xo * wi_wo_factor + 0.5);

				intensity[xo] = (float) nv_input[input_line + xi].output.ival;
			}

			intensity = intensity_line;
			for (xo = 0; xo < wo; xo++)
			{
				accumulator = 0.0;
        			for (i = 0; i < kernel_size; i++)
				{
					// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
					accumulator += kernel_points[i] * intensity[i];
				}

				// Normalizes the result
				accumulator *= normalizer;
				nv_output[yo * wo + xo].output.ival = (int) (accumulator + 0.5);
				intensity = intensity + 1;
			}
		}
		free (intensity_line);
	}
}



void
optimized_2d_point_convolution_color (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type)
{
	int i, xi, yi, hi, wi, xo, yo, ho, wo;
	float wi_wo_factor, hi_ho_factor;
	int input_line, output_line, input_colunm, intensity_vector_size, pixel;
	float *intensity_r, accumulator_r;
	float *intensity_g, accumulator_g;
	float *intensity_b, accumulator_b;
	float *intensity_vector_r;
	float *intensity_vector_g;
	float *intensity_vector_b;
	float *intensity_line_r;
	float *intensity_line_g;
	float *intensity_line_b;
	int kernel_size, kernel_div_2;
	NEURON *nv_input = NULL, *nv_output = NULL;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	wi_wo_factor = (float) wi / (float) wo;
	hi_ho_factor = (float) hi / (float) ho;

	nv_input  = nl_input->neuron_vector;
	nv_output = nl_output->neuron_vector;

	if (kernel_type == KERNEL_2D)
	{
		kernel_size = (int) sqrtf (num_points);
		kernel_div_2 = kernel_size >> 1;

		intensity_vector_size = ho * kernel_size + kernel_size * kernel_size + wo;
		intensity_vector_r = (float *) alloc_mem (intensity_vector_size * sizeof (float));
		intensity_vector_g = (float *) alloc_mem (intensity_vector_size * sizeof (float));
		intensity_vector_b = (float *) alloc_mem (intensity_vector_size * sizeof (float));

		for (i = 0; i < intensity_vector_size; i++)
			intensity_vector_r[i] = intensity_vector_g[i] = intensity_vector_b[i] = .0f;

		intensity_r = &(intensity_vector_r[kernel_div_2 * kernel_size]);
		intensity_g = &(intensity_vector_g[kernel_div_2 * kernel_size]);
		intensity_b = &(intensity_vector_b[kernel_div_2 * kernel_size]);

		/* These nested loops copy a column of the input, of width kernel_size-1, to the the vector intensity */
		xi = 0;
		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line  = yi * wi;
			output_line = yo * kernel_size;

			for (i = 0; i < (kernel_size - 1); i++)
			{
				input_colunm = xi + i - kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
				{
					pixel = nv_input[input_line + input_colunm].output.ival;
					intensity_r[output_line + i] = (float) RED(pixel);
					intensity_g[output_line + i] = (float) GREEN(pixel);
					intensity_b[output_line + i] = (float) BLUE(pixel);
				}
			}
		}

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo * wi_wo_factor + .5f);

			/* This loop copies a column of the input, of width 1, to the vector intensity; this completes a column
			   of the image of width kernel_size */
			intensity_r = &(intensity_vector_r[kernel_div_2 * kernel_size + xo]);
			intensity_g = &(intensity_vector_g[kernel_div_2 * kernel_size + xo]);
			intensity_b = &(intensity_vector_b[kernel_div_2 * kernel_size + xo]);
			intensity_r[- 1] = .0f;
			intensity_g[- 1] = .0f;
			intensity_b[- 1] = .0f;
			for (yo = 0; yo < ho; yo++)
			{
				yi = (int) ((float) yo * hi_ho_factor + .5f);
				input_line  = yi * wi;
				output_line = yo * kernel_size;
				input_colunm = xi + kernel_div_2;

				if (input_colunm >= 0 && input_colunm < wi)
				{
					pixel = nv_input[input_line + input_colunm].output.ival;
					intensity_r[output_line + kernel_size - 1] = (float) RED(pixel);
					intensity_g[output_line + kernel_size - 1] = (float) GREEN(pixel);
					intensity_b[output_line + kernel_size - 1] = (float) BLUE(pixel);
				}
				else
				{
					intensity_r[output_line + kernel_size - 1] = 0.0;
					intensity_g[output_line + kernel_size - 1] = 0.0;
					intensity_b[output_line + kernel_size - 1] = 0.0;
				}
			}

			/* This loop computes the convolution in one column */
			intensity_r = &(intensity_vector_r[xo]);
			intensity_g = &(intensity_vector_g[xo]);
			intensity_b = &(intensity_vector_b[xo]);
			for (yo = 0; yo < ho; yo++)
			{
				output_line = yo * wo;

				accumulator_r = .0f;
				accumulator_g = .0f;
				accumulator_b = .0f;

				/* This is the most time consuming loop */
				for (i = 0; i < kernel_size * kernel_size; i++)
				{
					accumulator_r += kernel_points[i] * intensity_r[i];
					accumulator_g += kernel_points[i] * intensity_g[i];
					accumulator_b += kernel_points[i] * intensity_b[i];
				}

				// Normalizes the result
				accumulator_r *= normalizer;
				accumulator_g *= normalizer;
				accumulator_b *= normalizer;
				nv_output[output_line + xo].output.ival = PIXEL((int) (accumulator_r + 0.5), (int) (accumulator_g + 0.5), (int) (accumulator_b + 0.5));

				intensity_r += kernel_size;
				intensity_g += kernel_size;
				intensity_b += kernel_size;
			}
		}
		free (intensity_vector_r);
		free (intensity_vector_g);
		free (intensity_vector_b);
	}
	else
	{
		kernel_size = num_points; /* only for 1D kernels */
		kernel_div_2 = kernel_size >> 1;

		intensity_line_r = (float *) alloc_mem (sizeof (float) * (wo + kernel_size));
		intensity_line_g = (float *) alloc_mem (sizeof (float) * (wo + kernel_size));
		intensity_line_b = (float *) alloc_mem (sizeof (float) * (wo + kernel_size));
		for (i = 0; i < wo + kernel_size; i++)
		{
			intensity_line_r[i] = 0.0;
			intensity_line_g[i] = 0.0;
			intensity_line_b[i] = 0.0;
		}

		for (yo = 0; yo < ho; yo++)
		{
			yi = (int) ((float) yo * hi_ho_factor + .5f);
			input_line = yi * wi;
			intensity_r = &(intensity_line_r[kernel_div_2]);
			intensity_g = &(intensity_line_g[kernel_div_2]);
			intensity_b = &(intensity_line_b[kernel_div_2]);
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) xo * wi_wo_factor + 0.5);

				pixel = nv_input[input_line + xi].output.ival;
				intensity_r[xo] = (float) RED(pixel);
				intensity_g[xo] = (float) GREEN(pixel);
				intensity_b[xo] = (float) BLUE(pixel);
			}

			intensity_r = intensity_line_r;
			intensity_g = intensity_line_g;
			intensity_b = intensity_line_b;
			for (xo = 0; xo < wo; xo++)
			{
				accumulator_r = 0.0;
				accumulator_g = 0.0;
				accumulator_b = 0.0;
        			for (i = 0; i < kernel_size; i++)
				{
					// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
					accumulator_r += kernel_points[i] * intensity_r[i];
					accumulator_g += kernel_points[i] * intensity_g[i];
					accumulator_b += kernel_points[i] * intensity_b[i];
				}

				// Normalizes the result
				accumulator_r *= normalizer;
				accumulator_g *= normalizer;
				accumulator_b *= normalizer;
				nv_output[yo * wo + xo].output.ival = PIXEL((int) (accumulator_r + 0.5), (int) (accumulator_g + 0.5), (int) (accumulator_b + 0.5));
				intensity_r = intensity_r + 1;
				intensity_g = intensity_g + 1;
				intensity_b = intensity_b + 1;
			}
		}
		free (intensity_line_r);
		free (intensity_line_g);
		free (intensity_line_b);
	}
}



void
optimized_2d_point_convolution (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type, int input_type)
{
	if (input_type == GREYSCALE_FLOAT)
		optimized_2d_point_convolution_greyscale_float (nl_output, nl_input, kernel_points, normalizer, num_points, kernel_type);
	else if (input_type == GREYSCALE)
		optimized_2d_point_convolution_greyscale (nl_output, nl_input, kernel_points, normalizer, num_points, kernel_type);
	else // (input_type == COLOR)
		optimized_2d_point_convolution_color (nl_output, nl_input, kernel_points, normalizer, num_points, kernel_type);
}


/*!
*********************************************************************************
* \brief The gabor filter with biological parameter specification.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void biological_gabor_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_image = NULL;
	NEURON_LAYER *nl_shift_map = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_fields;
	int i, xi, yi, hi, wi, xo, yo, ho, wo, x_center, y_center;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation, phase, log_factor;
	double correction;
	int mapping, shift, kernel_type;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi;
	int pyramid_height, pyramid_level, pyramid_level_offset;
//	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
//	float *kernel_points;
//	float delta_area;


	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 11)
	{
		Erro ("Wrong number of parameters for biological_gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase			= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift			= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type             = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1 && i != 2)
	{
		Erro ("Wrong number of neuron layers. biological_gabor_filter must be applied on one or two neuron layers.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl_image = filter_desc->neuron_layer_list->neuron_layer;
	nl_shift_map = (i == 2) ? filter_desc->neuron_layer_list->next->neuron_layer : NULL;

	// Gets Input Neuron Layer Dimentions
	wi = nl_image->dimentions.x;
	hi = nl_image->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

	input = get_input_by_neural_layer (nl_image);

	if (input != NULL)
		pyramid_height = input->pyramid_height;
	else
		pyramid_height = 0;
	
	// Previous output fval initialize
	previous_output.fval = 0.0f;

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem ((pyramid_height + 1) * sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		for (pyramid_level = 0; pyramid_level <= pyramid_height; pyramid_level++)
		{
			// Calculates the spatial frequency (cycles per pixel)
			frequency = (camera_opening / (float) wi) * cycles_per_degree * pow (2.0f, (float) pyramid_level);

			receptive_fields[pyramid_level].points = NULL;
			compute_v1_spatial_receptive_field (&(receptive_fields[pyramid_level]), frequency, width, aspect_ratio, orientation, phase, kernel_type);

			// Calculates the minimum value of the result of a gabor filter
			bidimentional_convolution_test_gabor (&(receptive_fields[pyramid_level]));
		}
		filter_desc->private_state = (void *) receptive_fields;
	}
	else
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

	for (pyramid_level = 0, pyramid_level_offset = 0; pyramid_level < PYRAMID_LEVEL; pyramid_level_offset += wi * hi, wi = wi >> 1, hi = hi >> 1, pyramid_level++)
		;

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = (input->wxd - input->wx) >> PYRAMID_LEVEL;
			y_center = (input->wyd - input->wy) >> PYRAMID_LEVEL;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	if (mapping == MAP_PLAN)
	{
/*		receptive_field_descriptor = &(receptive_fields[0]);
		kernel_points = (float *) alloc_mem (receptive_field_descriptor->num_points * sizeof (float));
		for (i = 0; i < receptive_field_descriptor->num_points; i++)
			kernel_points[i] = receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor;
		delta_area = (receptive_field_descriptor->area / (float) receptive_field_descriptor->num_points) / 6.0;

		optimized_2d_point_convolution (filter_desc->output, filter_desc->neuron_layer_list->neuron_layer,
						kernel_points, delta_area, receptive_field_descriptor->num_points, kernel_type, COLOR);

		free (kernel_points);
*/
		for (yo = 0; yo < ho; yo++)
		{
			for (xo = 0; xo < wo; xo++)
			{
				xi = xo;
				yi = yo;
				previous_output = bidimentional_convolution (&(receptive_fields[0]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);
				if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_fields[0].min_dog)) ||
					((previous_output.fval < .0f) && (previous_output.fval >= receptive_fields[0].min_dog)))
					previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;

				filter_desc->output->neuron_vector[yo * wo + xo].output = previous_output;
			}
		}
	}
	else
	{

		for (yo = 0; yo < ho; yo++)
		{
			for (xo = 0; xo < wo; xo++)
			{
				map_v1_to_image (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center, y_center, correction, log_factor);
				xi += (nl_shift_map != NULL) ? -(int) nl_shift_map->neuron_vector[yo * wo + xo].output.fval + shift : shift;

				if ((xi == previous_xi) && (yi == previous_yi))
					filter_desc->output->neuron_vector[yo * wo + xo].output = previous_output;
				else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = 0.0f;
				else
				{
					/*previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[PYRAMID_LEVEL]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

					if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_fields[PYRAMID_LEVEL].min_dog)) ||
						((previous_output.fval < .0f) && (previous_output.fval >= receptive_fields[PYRAMID_LEVEL].min_dog)))
						previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
					*/
					previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[0]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

					if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_fields[0].min_dog)) ||
						((previous_output.fval < .0f) && (previous_output.fval >= receptive_fields[0].min_dog)))
						previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
                                }

				previous_xi = xi;
				previous_yi = yi;
			}
		}
	}
}



/*!
*********************************************************************************
* \brief The gabor filter with biological parameter specification.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void biological_gabor_filter_old (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_image = NULL;
	NEURON_LAYER *nl_shift_map = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_fields;
	int i, xi, yi, hi, wi, xo, yo, ho, wo, x_center, y_center;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation, phase, log_factor;
	double correction;
	int mapping, shift, kernel_type;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi;
	int pyramid_height, pyramid_level, pyramid_level_offset;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 11)
	{
		Erro ("Wrong number of parameters for biological_gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase			= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift			= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type             = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1 && i != 2)
	{
		Erro ("Wrong number of neuron layers. biological_gabor_filter must be applied on one or two neuron layers.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl_image = filter_desc->neuron_layer_list->neuron_layer;
	nl_shift_map = (i == 2) ? filter_desc->neuron_layer_list->next->neuron_layer : NULL;

	// Gets Input Neuron Layer Dimentions
	wi = nl_image->dimentions.x;
	hi = nl_image->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

        input = get_input_by_neural_layer (nl_image);

        pyramid_height = input->pyramid_height;

	// Previous output initialize
	previous_output.fval = 0.0f;

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
                receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem ((pyramid_height + 1) * sizeof(RECEPTIVE_FIELD_DESCRIPTION));

                for (pyramid_level = 0; pyramid_level <= pyramid_height; pyramid_level++)
                {
		      // Calculates the spatial frequency (cycles per pixel)
		      frequency = (camera_opening / (float) wi) * cycles_per_degree * pow (2.0f, (float) pyramid_level);

		      receptive_fields[pyramid_level].points = NULL;
		      compute_v1_spatial_receptive_field (&(receptive_fields[pyramid_level]), frequency, width, aspect_ratio, orientation, phase, kernel_type);

		      // Calculates the minimum value of the result of a gabor filter
		      bidimentional_convolution_test_gabor (&(receptive_fields[pyramid_level]));
                }
                filter_desc->private_state = (void *) receptive_fields;
	}
	else
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

        for (pyramid_level = 0, pyramid_level_offset = 0; pyramid_level < PYRAMID_LEVEL; pyramid_level_offset += wi * hi, wi = wi >> 1, hi = hi >> 1, pyramid_level++)
                ;

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = (input->wxd - input->wx) >> PYRAMID_LEVEL;
			y_center = (input->wyd - input->wy) >> PYRAMID_LEVEL;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	switch (mapping)
	{
		case MAP_PLAN:
		{
			for (yo = 0; yo < ho; yo++)
			{
				yi = (int) ((float) (yo * hi) / (float) ho + 0.5) + y_center - hi/2;

				for (xo = 0; xo < wo; xo++)
				{
					xi = (int) ((float) (xo * wi) / (float) wo + 0.5) + x_center - wi/2 + shift;
					xi += (nl_shift_map != NULL) ? -(int) nl_shift_map->neuron_vector[yo * wo + xo].output.fval : 0;

					/*previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[PYRAMID_LEVEL]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

					if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_fields[PYRAMID_LEVEL].min_dog)) ||
						((previous_output.fval < 0.0) && (previous_output.fval >= receptive_fields[PYRAMID_LEVEL].min_dog)))
						filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;*/

					previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[0]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

					if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_fields[0].min_dog)) ||
						((previous_output.fval < 0.0) && (previous_output.fval >= receptive_fields[0].min_dog)))
						filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;

					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;

		case MAP_LOG_POLAR:
		{
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center, y_center, correction, log_factor);
					xi += (nl_shift_map != NULL) ? -(int) nl_shift_map->neuron_vector[yo * wo + xo].output.fval + shift : shift;

					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[yo * wo + xo].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
					else
					{
						/*previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[PYRAMID_LEVEL]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

						if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_fields[PYRAMID_LEVEL].min_dog)) ||
							((previous_output.fval < .0f) && (previous_output.fval >= receptive_fields[PYRAMID_LEVEL].min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
					        */
					        previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (&(receptive_fields[0]), &(nl_image->neuron_vector[pyramid_level_offset]), xi, yi, wi, hi);

						if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_fields[0].min_dog)) ||
							((previous_output.fval < .0f) && (previous_output.fval >= receptive_fields[0].min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
                                        }

					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
	}
}



/*!
*********************************************************************************
* \brief The gabor filter.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void gabor_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *image_n_l = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	int i, ho, wo, hi, wi, xi, yi, xo, yo, u, v;
	float harmonic, width, aspect_ratio, orientation, phase, log_factor;
	float frequency;
	double correction;
	int mapping, shift, kernel_type;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 10)
	{
		Erro ("Wrong number of parameters for gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	harmonic	= filter_desc->filter_params->next->param.fval;
	width		= filter_desc->filter_params->next->next->param.fval;
	aspect_ratio	= filter_desc->filter_params->next->next->next->param.fval;
	orientation	= filter_desc->filter_params->next->next->next->next->param.fval;
	phase		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	shift		= filter_desc->filter_params->next->next->next->next->next->next->param.ival;
	mapping		= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	log_factor	= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.fval;
	kernel_type     = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. gabor_filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	image_n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = image_n_l->dimentions.x;
	hi = image_n_l->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

	// Previous output initialize
	previous_output.fval = 0.0f;

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		// Calculates the sinusoidal frequency (cycles per pixel)
		frequency = (1.0f / (float) wi) * harmonic;

		receptive_field->points	= NULL;
		compute_v1_spatial_receptive_field (receptive_field, frequency, width, aspect_ratio, orientation, phase, kernel_type);
		filter_desc->private_state = (void *) receptive_field;

		// Calculates the minimum value of the result of a gabor filter
		bidimentional_convolution_test_gabor (receptive_field);
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *)filter_desc->private_state;

	input = get_input_by_neural_layer (image_n_l);

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	switch (mapping)
	{
		case MAP_PLAN:
		{
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) (xo * wi) / (float) wo + 0.5) + x_center - wi/2 + shift;

				for (yo = 0; yo < ho; yo++)
				{
					yi = (int) ((float) (yo * hi) / (float) ho + 0.5) + y_center - hi/2;

					previous_output = filter_desc->output->neuron_vector[(yo * wo) + xo].output = bidimentional_convolution (receptive_field, image_n_l->neuron_vector, xi, yi, wi, hi);

					if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_field->min_dog)) ||
						((previous_output.fval < 0.0) && (previous_output.fval >= receptive_field->min_dog)))
						filter_desc->output->neuron_vector[(yo * wo) + xo].output.fval = 0.0;
				}
			}
		}
		break;

		case MAP_LOG_POLAR:
		{
			previous_xi = -1;
			previous_yi = -1;

			for (u = 0; u < wo; u++)
			{
				for (v = 0; v < ho; v++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, wo, ho, x_center, y_center, correction, log_factor);
					xi += shift;

					//index = (u < (wo / 2)) ? ((wo / 2) - 1 - u) : (u - (wo / 2));
					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[(v * wo) + u].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = 0.0;
					else
					{
						previous_output = filter_desc->output->neuron_vector[(v * wo) + u].output = bidimentional_convolution (receptive_field, image_n_l->neuron_vector, xi, yi, wi, hi);

						if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_field->min_dog)) ||
							((previous_output.fval < 0.0) && (previous_output.fval >= receptive_field->min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = 0.0;
					}
					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
	}
}


/*!
*********************************************************************************
* \brief The edge filter.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void edge_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_neuron = NULL;
	NEURON_LAYER *n_neuron_next = NULL;
	int i, num_neurons;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. Edge filter must be applied on two neuron layers.", "", "");
		return;
	}

	n_neuron = filter_desc->neuron_layer_list->neuron_layer;
	n_neuron_next = filter_desc->neuron_layer_list->next->neuron_layer;
	num_neurons = get_num_neurons (filter_desc->output->dimentions);


	switch (filter_desc->output->output_type)
	{
		case COLOR:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival   =
					(NEURON_MASK_COLOR(n_neuron->neuron_vector[i].output.ival) == NEURON_MASK_COLOR(n_neuron_next->neuron_vector[i].output.ival)) ? 0 : (NUM_COLORS - 1);
			break;
		case GREYSCALE:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival   =
					(NEURON_MASK_GREYSCALE(n_neuron->neuron_vector[i].output.ival) == NEURON_MASK_GREYSCALE(n_neuron_next->neuron_vector[i].output.ival)) ? 0 : (NUM_GREYLEVELS - 1);
			break;
		case GREYSCALE_FLOAT:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.fval   =
					(n_neuron->neuron_vector[i].output.fval == n_neuron_next->neuron_vector[i].output.fval) ? 0.0 : (float) (NUM_GREYLEVELS - 1);
			break;
		case BLACK_WHITE:
			for (i = 0; i < num_neurons; i++)
				filter_desc->output->neuron_vector[i].output.ival =
					(NEURON_MASK_BLACK_WHITE(n_neuron->neuron_vector[i].output.ival) == NEURON_MASK_BLACK_WHITE(n_neuron_next->neuron_vector[i].output.ival)) ? 0: (NUM_GREYLEVELS - 1);
			break;
	}
}



/*!
*********************************************************************************
* \brief The dynamic detection layer.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void ddl_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_neuron = NULL;
	NEURON_LAYER *n_neuron_next = NULL;
	int i, num_neurons;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. Edge filter must be applied on two neuron layers.", "", "");
		return;
	}

	n_neuron = filter_desc->neuron_layer_list->neuron_layer;
	n_neuron_next = filter_desc->neuron_layer_list->next->neuron_layer;
	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
		filter_desc->output->neuron_vector[i].output.ival =
			(n_neuron->neuron_vector[i].output.ival == n_neuron_next->neuron_vector[i].output.ival)?0:n_neuron->neuron_vector[i].output.ival;
}



/*!
*********************************************************************************
* \brief The static detection layer.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void sdl_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_neuron = NULL;
	NEURON_LAYER *n_neuron_next = NULL;
	int i, num_neurons;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;
	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. Edge filter must be applied on two neuron layers.", "", "");
		return;
	}

	n_neuron = filter_desc->neuron_layer_list->neuron_layer;
	n_neuron_next = filter_desc->neuron_layer_list->next->neuron_layer;
	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
		filter_desc->output->neuron_vector[i].output.ival =
			(n_neuron->neuron_vector[i].output.ival != n_neuron_next->neuron_vector[i].output.ival)?0:n_neuron->neuron_vector[i].output.ival;
}



/*!
*********************************************************************************
* Function: apply_kernel							*
* Description: apply kernel to a neuron vector region centered at point (x,y)	*
* Inputs: 	kernel => kernel points						*
*		kernel_size => kernel points number				*
*		nv => the neuron vector						*
*		(x,y) => center point						*
*		(w,h) => the neuron vector dimentions				*
* Output: the result of the application of the kernel	 			*
*********************************************************************************
*/

float
apply_kernel (float *kernel, int kernel_size,
	      NEURON *nv, int x, int y, int w, int h)
{
	float result = 0.0;
	int i, j, xi = 0, yi = 0;

	for (i = 0; i < kernel_size; i++)
	{
		for (j = 0; j < kernel_size; j++)
		{
			xi = x+j-(kernel_size/2);
			yi = y+i-(kernel_size/2);
			if (((xi >= 0) && (xi < w)) &&
			    ((yi >= 0) || (yi < h)))
				result += kernel[i+j*kernel_size] * (float) nv[xi + (yi*w)].output.ival;
		}
	}
	return (result);
}



/*!
*********************************************************************************
* Function: kernel_filter							*
* Description: apply a kernel to a neuron leyear				*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void
kernel_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l = NULL;
	int i, j, h, w, n_l_h, n_l_w;
	float prop_x, prop_y;
	static int flag = 0;
	PARAM_LIST *paran;
	KERNEL *kernel;

	if (flag == 0)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			;
		if (i != 1)
		{
			Erro ("Wrong number of neuron layers.",
			      "The Kernel filter must be applied a single neuron layers.", "");
		}

		if (filter_desc->filter_params == NULL)
		{
			Erro ("Wrong number of parameters.",
			      "The Kernel filter requires a kernel size and a kernel.", "");
		}
		else
		{
			filter_desc->private_state = (void *) alloc_mem (sizeof (struct kernel));
			kernel = (KERNEL *) filter_desc->private_state;
			paran = filter_desc->filter_params;
			kernel->kernel_size = paran->param.ival;
			kernel->kernel_sigma = 0.0; // not used
			kernel->kernel_points = (float *) alloc_mem (kernel->kernel_size * kernel->kernel_size * sizeof (float));
			for (i = 0; i < kernel->kernel_size * kernel->kernel_size; i++)
			{
				paran = paran->next;
				kernel->kernel_points[i] = paran->param.fval;
			}
		}
		flag = 1;
	}

	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	n_l = filter_desc->neuron_layer_list->neuron_layer;
	n_l_w = n_l->dimentions.x;
	n_l_h = n_l->dimentions.y;

	kernel = (struct kernel *) filter_desc->private_state;
	prop_x = (float) n_l_w / (float) w;
	prop_y = (float) n_l_h / (float) h;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
			filter_desc->output->neuron_vector[(i * w) + j].output.ival =
				(int) apply_kernel (kernel->kernel_points, kernel->kernel_size, n_l->neuron_vector,
				(int) ((float) j * prop_x), (int) ((float) i * prop_y), n_l_w, n_l_h);
	}
}



/*!
*********************************************************************************
* Function: ganglion								*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

float
ganglion (int i, int j, int width, int gaussianPart)
{
	float si;
	float se;

	si = (float) width/3.0;
	se = si/3.0;

	switch (gaussianPart)
	{

		case POSITIVE:
			return ((si/se)/(se*sqrt(2.0*pi)))*exp(-(((i-width)*(i-width)+(j-width)*(j-width))/(2.0*se*se)));
		case NEGATIVE:
			return  (1.0/(si*sqrt(2.0*pi)))*exp(-(((i-width)*(i-width)+(j-width)*(j-width))/(2.0*si*si)));
		default:
			return (((si/se)/(se*sqrt(2.0*pi)))*exp(-(((i-width)*(i-width)+(j-width)*(j-width))/(2.0*se*se))) -
				(1.0/(si*sqrt(2.0*pi)))*exp(-(((i-width)*(i-width)+(j-width)*(j-width))/(2.0*si*si))));
	}
}



/*!
*********************************************************************************
* Function: comp_kernel								*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void
comp_kernel (float *kernel, int size, int gaussianPart)
{
	int i, j;

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			kernel[i+j*size] = ganglion (i, j, size/2, gaussianPart);
		}
	}

}



/*!
*********************************************************************************
* Function: gaussian_integral							*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

double
gaussian_integral (double radius)
{
	int i;
	double gaussian;
	double gaussian_prev;
	double incr;
	double i_gauss;
	double x;

	gauss_vector[0] = 1;
	i_gauss = integ_gauss[0] = gaussian_prev = 0.0;
	incr = radius / (double) TAM_GAUSS_VEC;
	for (i = 1; i < TAM_GAUSS_VEC; i++)
	{
		x = (double) i * incr;
		gaussian = exp (-(x * x));
		gauss_vector[i] = gaussian;
		i_gauss += ((gaussian_prev + gaussian) * incr) / 2.0;
		integ_gauss[i] = i_gauss;
		gaussian_prev = gaussian;
	}
	return (i_gauss);
}



/*!
*********************************************************************************
* Function: comp_distance_vector						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void
comp_distance_vector (double i_gauss)
{
	int d, i;

	i = 0;
	for (d = 0; d < TAM_GAUSS_VEC; d++)
	{
		if (((double) d / (double) (TAM_GAUSS_VEC-1)) > (integ_gauss[i] / i_gauss))
		{
			i++;
			while (((double) d / (double) (TAM_GAUSS_VEC-1)) > (integ_gauss[i] / i_gauss))
				i++;
		}
		distance_vector[d] = (double) (i) / (double) (TAM_GAUSS_VEC-1);
	}
}



/*!
*********************************************************************************
* Function: scale								*
* Description: This function scales a value in a range automatically		*
* Inputs:	valueToScale	- Value to be scaled				*
*		initial_min	- The min value of the the range that the	*
*				valueToScale in					*
*		initial_max	- The max value of the the range that the	*
*				valueToScale in					*
*		scale_min	- The min value that the valueToScale can be.	*
*		scale_max	- The max value that the valueToScale can be.	*
* Output: 						 			*
*********************************************************************************
*/

float
scale (float valueToScale, float initial_min, float initial_max, float scale_min, float scale_max)
{
	return (((valueToScale - initial_min)*(scale_max - scale_min) / (initial_max - initial_min)) + scale_min);
}



/*!
*********************************************************************************
* Function: compute_gaussian_kernel2						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

KERNEL *compute_gaussian_kernel2 (int kernel_size, float sigma)
{
	// http://en.wikipedia.org/wiki/Gaussian_blur
	int x, y, index, radius;
	float k1, k2, *kernel_points = NULL;
	KERNEL *gaussian_kernel = NULL;
	double sum;

	if ((kernel_size % 2) == 0)
	{
		Erro ("The kernel size must be an odd number (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	if ((gaussian_kernel = (KERNEL *) alloc_mem (sizeof(KERNEL))) == NULL)
	{
		Erro ("Cannot allocate more memory (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	gaussian_kernel->kernel_size = kernel_size;
	gaussian_kernel->kernel_sigma = sigma;

	if ((gaussian_kernel->kernel_points = kernel_points = (float *) alloc_mem (kernel_size * kernel_size * sizeof (float))) == NULL)
	{
		Erro ("Cannot allocate more memory (compute_gaussian_kernel2).", "", "");
		return NULL;
	}

	//TODO: k1 nao tem sqrt no kernel 2D
	k1 = 1.0 / sqrt (2.0 * pi * sigma * sigma);
	k2 = -1.0 / (2.0 * (sigma * sigma));
	radius = kernel_size / 2.0;
	
	sum = 0.0;
	index = 0;
	for (y = -radius; y <= radius; y++)
	{
		for (x = -radius; x <= radius; x++)
		{
			kernel_points[index] = k1 * exp(k2 * (float) (x * x + y * y));
			sum += kernel_points[index];
			index++;
		}
	}

	index = 0;
	for (y = -radius; y <= radius; y++)
	{
		for (x = -radius; x <= radius; x++)
		{
			kernel_points[index] /= sum; // Normalize kernel
			//printf("# %1.03f ", kernel_points[index]);
			index++;
		}
		//printf("\n");
	}

	return gaussian_kernel;
}



/*!
*********************************************************************************
* Function: gaussian_filter
* Description: Applies a gaussian kernel on a neuron layer
* Inputs:
* Output:
*********************************************************************************
*/

void gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	KERNEL *gaussian_kernel;
	PARAM_LIST *p_list;
	int i, kernel_size, num_points;
	float sigma;

	if (filter_desc->private_state == NULL)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			;

		if (i != 1)
		{
			Erro ("Wrong number of neuron layers. The gaussian_filter must be applied on only one neuron layer (gaussian_filter).", "", "");
			return;
		}

		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the gaussian_filter output (gaussian_filter).", "", "");
			return;
		}

		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
			;

		if (--i != 2)
		{
			Erro ("Wrong number of parameters. The gaussian_filter have 2 parameters: kernel_size and sigma respectivally (gaussian_filter).", "", "");
			return;
		}

		kernel_size = filter_desc->filter_params->next->param.ival;
		sigma	    = filter_desc->filter_params->next->next->param.fval;

		gaussian_kernel = compute_gaussian_kernel2 (kernel_size, sigma);

		if ((filter_desc->private_state = (void *) gaussian_kernel) == NULL)
		{
			Erro ("Cannot create gaussian kernel (gaussian_filter).", "", "");
			return;
		}
	}
	else
		gaussian_kernel = (KERNEL *) filter_desc->private_state;

	num_points = gaussian_kernel->kernel_size * gaussian_kernel->kernel_size;
	optimized_2d_point_convolution (filter_desc->output, filter_desc->neuron_layer_list->neuron_layer,
					gaussian_kernel->kernel_points, LEVEL, num_points, KERNEL_2D, filter_desc->neuron_layer_list->neuron_layer->output_type);
}



/*!
*********************************************************************************
* Function: gaussian_filter
* Description: Applies a gaussian kernel on a neuron layer
* Inputs:
* Output:
*********************************************************************************
*/

void gaussian_dynamic_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	KERNEL *gaussian_kernel;
	PARAM_LIST *p_list;
	int i, kernel_size, num_points;
	float sigma;

	if (filter_desc->private_state == NULL)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			;

		if (i != 1)
		{
			Erro ("Wrong number of neuron layers. The gaussian_filter must be applied on only one neuron layer (gaussian_filter).", "", "");
			return;
		}

		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the gaussian_filter output (gaussian_filter).", "", "");
			return;
		}

		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
			;

		if (--i != 2)
		{
			Erro ("Wrong number of parameters. The gaussian_filter have 2 parameters: kernel_size and sigma respectivally (gaussian_filter).", "", "");
			return;
		}

		kernel_size = *((int *) (filter_desc->filter_params->next->param.pval));
		sigma	    = *((float *) (filter_desc->filter_params->next->next->param.pval));

		gaussian_kernel = compute_gaussian_kernel2 (kernel_size, sigma);

		if ((filter_desc->private_state = (void *) gaussian_kernel) == NULL)
		{
			Erro ("Cannot create gaussian kernel (gaussian_filter).", "", "");
			return;
		}
	}
	else
	{
		kernel_size = *((int *) (filter_desc->filter_params->next->param.pval));
		sigma	    = *((float *) (filter_desc->filter_params->next->next->param.pval));
		
		gaussian_kernel = (KERNEL *) filter_desc->private_state;
		if ((gaussian_kernel->kernel_size != kernel_size) || (gaussian_kernel->kernel_sigma != sigma))
		{
			free(gaussian_kernel->kernel_points);
			free(gaussian_kernel);
			gaussian_kernel = compute_gaussian_kernel2 (kernel_size, sigma);
		}
	}

	num_points = gaussian_kernel->kernel_size * gaussian_kernel->kernel_size;
	optimized_2d_point_convolution (filter_desc->output, filter_desc->neuron_layer_list->neuron_layer,
					gaussian_kernel->kernel_points, LEVEL, num_points, KERNEL_2D, filter_desc->neuron_layer_list->neuron_layer->output_type);
}



/*!
*********************************************************************************
* Function: magnification_factor_v1						*
* Description: 	computes the V1 magnification factor 				*
* 		studied by Sereno et all.					*
* Reference: 	M. I. Sereno, A. M. Dale, J. B. Reppas, K.K. Kwomg, 		*
*		J. W. Belliveau, T. J. Brady, B. R. Rosen and R. B. H. Tootell,	*
*		"Borders of Multiple Visual  Areas in Human Revealed by 	*
*		Functional Magnetic Resonance Imaging", Science 268: 889-893,	*
*		1995.								*
* Inputs:	r - angle							*
* Output:	magnification factor						*
*********************************************************************************
*/

double
magnification_factor_v1 (double r)
{
	return (10.95 * pow(r + 0.08, -1.16));
}



/*!
*********************************************************************************
* Function: magfactor_integral							*
* Description: 	computes the integral of magnification factor 			*
* 		studied by Sereno et all: M. I. Sereno, A. M. Dale, 		*
*		J. B. Reppas, K.K. Kwomg, J. W. Belliveau, T. J. Brady, 	*
*		B. R. Rosen and R. B. H. Tootell, "Borders of Multiple Visual 	*
*		Areas in Human Revealed by Functional Magnetic Resonance 	*
*		Imaging", Science 268: 889-893, 1995				*
* Inputs:	Maximum angle							*
* Output:	The image of the function.					*
*********************************************************************************

*/

double
magfactor_v1_integral (double max_angle, double *integ_magfactor)
{
	int i;
	double magfactor;
	double magfactor_prev;
	double incr;
	double i_magfactor;
	double r;

	i_magfactor = integ_magfactor[0] = magfactor_prev = 0.0;
	incr = max_angle / (double) TAM_MAGFACTOR_VEC;
	for (i = 1; i < TAM_MAGFACTOR_VEC ;i++)
	{
		r = (double) i * incr;
		magfactor = magnification_factor_v1 (r);
		i_magfactor += ((magfactor_prev + magfactor) * incr) / 2.0;
		integ_magfactor[i] = i_magfactor;
		magfactor_prev = magfactor;
		/* printf("%f %f\n",r,integ_magfactor[i]); */
	}
	return (i_magfactor);
}



double par = 400.0;



/*!
*********************************************************************************
* Function: compute_v1_receptive_field						*
* Description: Compute the receptive field					*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_v1_receptive_field (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_description, int kernel_size,
			    double orientation, double sigma_x, double sigma_y, double phase_x, double phase_y,
			    double cycles_x, double cycles_y)
{
	int x, y, num_points = 0;
	double  x1, y1;
	POINT *points;
	double omega_x, omega_y;
	int radius;

	points = (POINT *) alloc_mem (sizeof (POINT) * kernel_size * kernel_size);

	omega_x = 2.0 * pi * cycles_x / (double) kernel_size;
	omega_y = 2.0 * pi * cycles_y / (double) kernel_size;

	radius = (kernel_size - 1) >> 1;

	for (y = -radius; y < (radius + 1); y++)
	{
		for (x = -radius; x < (radius + 1); x++)
		{
			points[num_points].x = x;
			points[num_points].y = y;

			x1 =   cos(orientation) * ((double) x) + sin(orientation) * ((double) y);
			y1 = - sin(orientation) * ((double) x) + cos(orientation) * ((double) y);

			if((x * x + y * y) <= (radius * radius))
				points[num_points].gabor = exp(-((x1 * x1) / (2.0 * sigma_x * sigma_x))) * cos(omega_x * x1 - phase_x)
							 * exp(-((y1 * y1) / (2.0 * sigma_y * sigma_y))) * cos(omega_y * y1 - phase_y);

			else
				points[num_points].gabor = 0.0;
			num_points++;
		}
	}

	receptive_field_description->num_points = num_points;
	receptive_field_description->points = points;
	receptive_field_description->cut_orientation = orientation;
}



/*!
*********************************************************************************
* Function: apply_kernel_on_v1_image						*
* Description: Apply kernel to a neuron vector (nv) region: Return the result 	*
* of the application of the "kernel" to the region of "nv" centered at "x,y" 	*									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
apply_kernel_on_v1_image (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_description,
	      NEURON *nv_right, NEURON *nv_left, int u, int v, int w, int h, double displacement, double displacement_angle)
{
	double result = 0.0;
	double cut_orientation;
	int i, j, x, y;
	int x_current, y_current;
	double kernel;
	NEURON_OUTPUT neuron_value;

	neuron_value.ival = 0;

	for (i = 0; i < KERNEL_SIZE; i++)
	{
		for (j = 0; j < KERNEL_SIZE; j++)
		{
			x =  receptive_field_description->points[i * KERNEL_SIZE + j].x;
			y =  receptive_field_description->points[i * KERNEL_SIZE + j].y;
			kernel = receptive_field_description->points[i * KERNEL_SIZE + j].gabor;
			cut_orientation = receptive_field_description->cut_orientation;

			if(y > (tan(cut_orientation) * x))
			{
				x_current = u + x;
				y_current = v + y;

				if((x_current >= 0) && (x_current < w))
				{

					if(y_current < 0)
						neuron_value = nv_right[w * (-y_current) - x_current - 1].output;
					else
					{
						if(y_current >= h)
							neuron_value = nv_right[w * (2 * h - y_current) - x_current - 1].output;
						else
							neuron_value = nv_right[w * (y_current) + x_current].output;
					}

				}
			}
			else
			{
				x_current = u + x + (int)(displacement * cos(cut_orientation + displacement_angle));
				y_current = v + y + (int)(displacement * sin(cut_orientation + displacement_angle));

				if((x_current >= 0) && (x_current < w))
				{

					if(y_current < 0)
						neuron_value = nv_left[w * (-y_current) - x_current - 1].output;
					else
					{
						if(y_current >= h)
							neuron_value = nv_left[w * (2 * h - y_current) - x_current - 1].output;
						else
							neuron_value = nv_left[w * (y_current) + x_current].output;
					}
				}
			}
			result += kernel * (double) neuron_value.ival;
		}
	}
	return (result);
}



/*!
*********************************************************************************
* Function: compute_simple_cell							*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_simple_cell (BINOCULAR_SIMPLE_CELL *binocular_simple_cell, double phase_left, double phase_right, int kernel_size)
{
	double sigma;

	binocular_simple_cell->receptive_field_left = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));
	binocular_simple_cell->receptive_field_right = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));

	sigma = (double) kernel_size / 2.0;

	compute_v1_receptive_field (binocular_simple_cell->receptive_field_left, kernel_size, ORIENTATION,
				    sigma, sigma, phase_left, 0.0, CYCLES, 0.0);

	compute_v1_receptive_field (binocular_simple_cell->receptive_field_right, kernel_size, ORIENTATION,
				    sigma, sigma, phase_right, 0.0, CYCLES, 0.0);
}



/*!
*********************************************************************************
* Function: compute_quadrature_pair						*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_quadrature_pair (QUADRATURE_PAIR *quadrature_pair, double phase_left, double phase_right, int kernel_size)
{
	quadrature_pair->binocular_simple_cell_cos = (BINOCULAR_SIMPLE_CELL *) alloc_mem (sizeof(BINOCULAR_SIMPLE_CELL));
	quadrature_pair->binocular_simple_cell_sin = (BINOCULAR_SIMPLE_CELL *) alloc_mem (sizeof(BINOCULAR_SIMPLE_CELL));
	quadrature_pair->responses = NULL;

	compute_simple_cell (quadrature_pair->binocular_simple_cell_cos, phase_left, phase_right, kernel_size);

	compute_simple_cell (quadrature_pair->binocular_simple_cell_sin, phase_left + (pi/2.0), phase_right + (pi/2.0), kernel_size);
}



/*!
*********************************************************************************
* Function: compute_complex_cell						*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_complex_cell(COMPLEX_CELL *complex_cell, double sigma_w, double phase_left, double phase_right, int kernel_size)
{
	int x, y, radius, index = 0;

	radius = (kernel_size - 1) / 2;

	complex_cell->weights_vector = (double *) alloc_mem (kernel_size * kernel_size * sizeof(double));

	for(y = -radius; y <= radius; y++)
	{
		for(x = -radius; x <= radius; x++)
		{
			complex_cell->weights_vector[index] = exp(-((double) (x * x + y * y) / (2.0 * sigma_w * sigma_w)));
			index++;
		}
	}

	complex_cell->phase_left = phase_left;
	complex_cell->phase_right = phase_right;

	complex_cell->omega_zero = 2.0 * pi * CYCLES / (double) kernel_size;

	complex_cell->quadrature_pair = (QUADRATURE_PAIR *) alloc_mem (sizeof(QUADRATURE_PAIR));

	compute_quadrature_pair (complex_cell->quadrature_pair, phase_left, phase_right, kernel_size);
}



/*!
*********************************************************************************
* Function: compute_complex_cells_clustering					*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_complex_cells_clustering (COMPLEX_CELLS_CLUSTERING *complex_cells_clustering, double sigma_w, int kernel_size)
{
	unsigned int i;
	double phase_left [] = {-6.0*pi/8.0, -5.0*pi/8.0, -4.0*pi/8.0, -3.0*pi/8.0, -2.0*pi/8.0, -pi/8.0, 0.0, pi/8.0};
	double phase_right [] = {2.0*pi/8.0, pi/8.0, 0.0, -pi/8.0, -2.0*pi/8.0, -3.0*pi/8.0, -4.0*pi/8.0, -5.0*pi/8.0};

	complex_cells_clustering->complex_cells = (COMPLEX_CELL*) alloc_mem (sizeof(phase_left) * sizeof(COMPLEX_CELL));

	for (i = 0; i < 8; i++)
		compute_complex_cell(&(complex_cells_clustering->complex_cells[i]), sigma_w, phase_left[i], phase_right[i], kernel_size);
}



/*!
*********************************************************************************
* Function: pop_intensity							*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
pop_intensity (INPUT_DESC *input, int xr, int yr)
{
	double intensity, red, green, blue;
	double k;

	k = 1.0;
      	red   = input->image[3 * (yr * input->tfw + xr) + 0];
	green = input->image[3 * (yr * input->tfw + xr) + 1];
	blue  =	input->image[3 * (yr * input->tfw + xr) + 2];

	intensity = (k * (red + green + blue) / 3.0);
	return intensity;
}



/*!
*********************************************************************************
* Function: apply_kernel_on_image						*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
apply_kernel_on_image (RECEPTIVE_FIELD_DESCRIPTION *receptive_field, NEURON_LAYER *neuron_layer, int u, int v, int w, int h, double orientation, int kernel_size)
{
	int i;
	int x_current, y_current;
	double kernel;
	int offset, radius, r;
	double result = 0.0;
	INPUT_DESC *input;

	input = get_input_by_neural_layer (neuron_layer);

	radius = offset = (kernel_size - 1)/2;



	if(u > w/2)
		orientation = -orientation;

	r = -radius;
	kernel = receptive_field->points[offset * kernel_size].gabor;

	x_current = u + (int)((double) r * cos(orientation));
	y_current = v + (int)((double) r * sin(orientation));

	if((x_current >= 0) && (x_current < w) && (y_current >= 0) && (y_current < h))
		result += kernel * pop_intensity (input, x_current, y_current);

	for (i = 1; i < kernel_size - 1; i++)
	{
		r++;
		kernel = receptive_field->points[offset * kernel_size + i].gabor;

		x_current = u + (int)((double) r * cos(orientation));
		y_current = v + (int)((double) r * sin(orientation));

		if((x_current >= 0) && (x_current < w) && (y_current >= 0) && (y_current < h))
			result += i%2 ? 2.0 * kernel * pop_intensity (input, x_current, y_current) : 4.0 * kernel * pop_intensity (input, x_current, y_current);
	}

	r++;
	kernel = receptive_field->points[offset * kernel_size + kernel_size - 1].gabor;

	x_current = u + (int)((double) r * cos(orientation));
	y_current = v + (int)((double) r * sin(orientation));

	if((x_current >= 0) && (x_current < w) && (y_current >= 0) && (y_current < h))
		result += kernel * pop_intensity (input, x_current, y_current);

	result /= 3.0;

	return result;

}



/*!
*********************************************************************************
* Function: compute_simple_cell_response					*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
compute_simple_cell_response (BINOCULAR_SIMPLE_CELL *binocular_simple_cell, NEURON_LAYER *nl_left_eye, NEURON_LAYER *nl_right_eye, int x, int y, double orientation, int kernel_size)
{
	double response, left, right;

	left = apply_kernel_on_image (binocular_simple_cell->receptive_field_left, nl_left_eye, x, y,
				    nl_left_eye->dimentions.x, nl_left_eye->dimentions.y, orientation, kernel_size);

	right = apply_kernel_on_image (binocular_simple_cell->receptive_field_right, nl_right_eye, x, y,
				     nl_right_eye->dimentions.x, nl_right_eye->dimentions.y, orientation, kernel_size);

	response = left + right;

	return response;
}



/*!
*********************************************************************************
* Function: compute_quadrature_pair_response 					*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_quadrature_pair_response (QUADRATURE_PAIR *quadrature_pair, NEURON_LAYER *nl_left_eye, NEURON_LAYER *nl_right_eye, int kernel_size)
{
	double response, cos, sin;
	int x, y, w, h;

	w = nl_left_eye->dimentions.x;
	h = nl_left_eye->dimentions.y;

	if(quadrature_pair->responses == NULL)
		quadrature_pair->responses = (double *) alloc_mem (w * h * sizeof(double));

	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cos = compute_simple_cell_response (quadrature_pair->binocular_simple_cell_cos, nl_left_eye, nl_right_eye, x, y, ORIENTATION, kernel_size);
			sin = compute_simple_cell_response (quadrature_pair->binocular_simple_cell_sin, nl_left_eye, nl_right_eye, x, y, ORIENTATION, kernel_size);
			response = cos * cos + sin * sin;

			quadrature_pair->responses[x + w * y] = response;
		}
	}
}



/*!
*********************************************************************************
* Function: compute_complex_cell_response					*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
compute_complex_cell_response (COMPLEX_CELL *complex_cell, NEURON_LAYER *nl_left_eye, NEURON_LAYER *nl_right_eye, int x0, int y0, int kernel_size)
{
	int x, y, w, h, index = 0, radius;
	int x_current, y_current;
	double average = 0.0;

	radius = (kernel_size - 1) / 2;
	w = nl_left_eye->dimentions.x;
	h = nl_left_eye->dimentions.y;

	for(y = -radius; y <= radius; y++)
	{
		for(x = -radius; x <= radius; x++)
		{
			x_current = x + x0;
			y_current = y + y0;

			if(x_current >= 0 && x_current < w && y_current >= 0 && y_current < h)
				average += (complex_cell->weights_vector[index] * complex_cell->quadrature_pair->responses[x_current + w * y_current]);
			index++;
		}
	}
	return average;
}



/*!
*********************************************************************************
* Function: compute_disparity 							*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

double
compute_disparity (COMPLEX_CELLS_CLUSTERING *complex_cells_clustering, NEURON_LAYER *nl_left_eye, NEURON_LAYER *nl_right_eye, int x, int y, int kernel_size)
{
	int i, max_pos;
	double max, result, disparity, phase_left, phase_right, omega_zero;

	max = compute_complex_cell_response (&(complex_cells_clustering->complex_cells[0]), nl_left_eye, nl_right_eye, x, y, kernel_size);
	max_pos = 0;

	for(i = 1; i < 8; i++)
	{
		result = compute_complex_cell_response (&(complex_cells_clustering->complex_cells[i]), nl_left_eye, nl_right_eye, x, y, kernel_size);
		if(result > max)
		{
			max = result;
			max_pos = i;
		}
	}

	phase_left = complex_cells_clustering->complex_cells[max_pos].phase_left;
	phase_right = complex_cells_clustering->complex_cells[max_pos].phase_right;
	omega_zero = complex_cells_clustering->complex_cells[max_pos].omega_zero;

	disparity = (phase_left - phase_right)/omega_zero;

	return disparity;
}



/*!
*********************************************************************************
* Function: binocular_disparity_filter						*
* Description: 									*
* Inputs: 									*
* Output:									*
*********************************************************************************
*/

void
binocular_disparity_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_left_eye = NULL;
	NEURON_LAYER *nl_right_eye = NULL;
	int nl_left_eye_w, nl_left_eye_h, nl_right_eye_w, nl_right_eye_h;
	int i, j, k;
	double sigma_w;
	COMPLEX_CELLS_CLUSTERING *complex_cells_clustering;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list->next != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. Occular dominance vergence filter must be applied on 2 neuron layers.", "", "");
		return;
	}

	nl_left_eye = filter_desc->neuron_layer_list->neuron_layer;
	nl_left_eye_w = nl_left_eye->dimentions.x;
	nl_left_eye_h = nl_left_eye->dimentions.y;

	nl_right_eye = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_right_eye_w = nl_right_eye->dimentions.x;
	nl_right_eye_h = nl_right_eye->dimentions.y;

	if(nl_left_eye_w != nl_right_eye_w || nl_left_eye_h != nl_right_eye_h)
	{
		Erro ("Image left and image right size must be the same.", "", "");
		return;
	}

	if(filter_desc->private_state == NULL)
	{
		complex_cells_clustering = (COMPLEX_CELLS_CLUSTERING *) alloc_mem (sizeof(COMPLEX_CELLS_CLUSTERING));

		sigma_w = filter_desc->filter_params->param.fval;

 	        compute_complex_cells_clustering (complex_cells_clustering, sigma_w, KERNEL_SIZE);

		filter_desc->private_state = (void *) complex_cells_clustering;
	}
	else
		complex_cells_clustering = (COMPLEX_CELLS_CLUSTERING *) filter_desc->private_state;


	switch (filter_desc->output->output_type)
	{
		case COLOR:
			break;
		case GREYSCALE_FLOAT:
			for (k = 0; k < 8; k++)
				compute_quadrature_pair_response(complex_cells_clustering->complex_cells[k].quadrature_pair, nl_left_eye, nl_right_eye, KERNEL_SIZE);

			for (i = 0; i < nl_right_eye_w; i++)
			{
				for (j = 0; j < nl_right_eye_h; j++)
					filter_desc->output->neuron_vector[i + nl_right_eye_w * j].output.fval =
						compute_disparity(complex_cells_clustering, nl_left_eye, nl_right_eye, i, j, KERNEL_SIZE);
			}
			break;
		default:
			break;
	}
}


/*!
*********************************************************************************
* Description:    			                     			*
* Function: distance_filter                         				*
* Inputs: filter_desc - Filter description                  			*
* Output: 				                			*
*********************************************************************************
*/

void
distance_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_input, *nl_output, *nl_features;
	int feature, i, u, v, ho, wo, nl_number, p_number;
	float value, feature_value;

	// Check Neuron Layers
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Check Params
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number == 0)
	{
		Erro ("Error: The distance filter don't need any parameters.", "", "");
		return;
	}

	// Get Filter Output
	nl_output = filter_desc->output;

	// Get Image Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	// Initialize Filter Output
	for (i = 0; i < wo * ho; i++)
		nl_output->neuron_vector[i].output.fval = 0.0;

	// Get the Features Neuron Layer
	nl_features = filter_desc->neuron_layer_list->neuron_layer;

	for (feature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; feature++, n_list = n_list->next)
	{
		nl_input = n_list->neuron_layer;
		feature_value = nl_features->neuron_vector[feature].output.fval;

		for (v = 0; v < ho; v++)
		{
			for (u = 0; u < wo; u++)
			{
				value = nl_input->neuron_vector[v * wo + u].output.fval;
				nl_output->neuron_vector[v * wo + u].output.fval += (feature_value - value) * (feature_value - value);
			}
		}
	}

	// Normalize Filter Output
	for (i = 0; i < wo * ho; i++)
	{
		nl_output->neuron_vector[i].output.fval = 1.0 / nl_output->neuron_vector[i].output.fval;
		//nl_output->neuron_vector[i].output.fval = exp (-nl_output->neuron_vector[i].output.fval);
	}
}



/*!
*********************************************************************************
* Function: mean_filter                         				*
* Description: Compute Mean Value                        			*
* Inputs: filter_desc - Filter description                  			*
* Output: 				                			*
*********************************************************************************
*/

void
mean_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_input  = NULL;
	NEURON_LAYER *nl_output = NULL;
	int num_params, num_layers, xi, yi, wi, hi, wo, /*ho,*/ columns;
	float width, average;

	// Check Params
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
	;

	if (num_params != 2)
	{
		Erro ("Error: Wrong number of parameters. The mean_filter must have only one parameter.", "", "");
		return;
	}

	// Get Params
	width = filter_desc->filter_params->next->param.fval;

	if ((width < 0.0) && (width > 1.0))
	{
		Erro ("Error: The band width parameter must be in the interval [0.0, 1.0].", "", "");
		return;
	}

	// Get the Output Neuron Layer
	nl_output = filter_desc->output;

	// Get Output Dimentions
	wo = nl_output->dimentions.x;
	//ho = nl_output->dimentions.y;

	// Check Neuron Layers
	for (num_layers = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, num_layers++)
		;

	if (num_layers != wo)
	{
		Erro ("Error: The number of input neural layers must be the same as the output width.", "", "");
		return;
	}

	// Compute Mean Value
	nl_output = filter_desc->output;

	for (num_layers = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, num_layers++)
	{
		// Get the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;

		// Get Input Dimentions
		wi = nl_input->dimentions.x;
		hi = nl_input->dimentions.y;

		// Computes the Avarage
		for (average = 0.0, columns = 0, xi = (int) ((0.5 - width/2.0) * (float) wi);
		     xi <= (int) ((0.5 + width/2.0) * (float) wi);
		     columns++, xi++)
		{
			for (yi = 0; yi < hi; yi++)
			{
				average += nl_input->neuron_vector[yi * wi + xi].output.fval;
			}
		}

		// Compute the final average of the columns
		average /= (float) (hi * columns);

		nl_output->neuron_vector[num_layers].output.fval = average;
	}
}



/*!
*********************************************************************************
* Function: near_cells								*
* Description: compute the near cells output from neuron_layers left and right	*
* Inputs: filter_desc - Filter description					*
* Output: 						 			*
*********************************************************************************
*/

void
near_cells (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON *sc_l1_nv, *sc_r1_nv, *sc_q_l1_nv, *sc_q_r1_nv;
	NEURON *sc_l2_nv, *sc_r2_nv, *sc_q_l2_nv, *sc_q_r2_nv;
	NEURON *disparity_nv;
	int i, u, v, h, w;
	double sc_l1, sc_r1, sc_q_l1, sc_q_r1;
	double sc_l2, sc_r2, sc_q_l2, sc_q_r2;
	double phase_left1, phase_right1, phase_left2, phase_right2;
	double c1, c2, a, b, d, sin_val, w0 = 1.0;

	// Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 8)
	{
		Erro ("Wrong number of neuron layers. near_cells filter must be applied on eight neuron layers.", "", "");
		return;
	}

	// Get Neuron Layers (Neuron Vectors)
	sc_l1_nv   = filter_desc->neuron_layer_list->neuron_layer->neuron_vector;
	sc_r1_nv   = filter_desc->neuron_layer_list->next->neuron_layer->neuron_vector;
	sc_q_l1_nv = filter_desc->neuron_layer_list->next->next->neuron_layer->neuron_vector;
 	sc_q_r1_nv = filter_desc->neuron_layer_list->next->next->next->neuron_layer->neuron_vector;
 	sc_l2_nv   = filter_desc->neuron_layer_list->next->next->next->next->neuron_layer->neuron_vector;
	sc_r2_nv   = filter_desc->neuron_layer_list->next->next->next->next->next->neuron_layer->neuron_vector;
	sc_q_l2_nv = filter_desc->neuron_layer_list->next->next->next->next->next->next->neuron_layer->neuron_vector;
 	sc_q_r2_nv = filter_desc->neuron_layer_list->next->next->next->next->next->next->next->neuron_layer->neuron_vector;

	// Check Params
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);

	if (i != 5)
	{
		Erro ("Wrong number of parameters. near_cells filter needs four parameters, i.e. the four simple cells phases.", "", "");
		return;
	}

	// Get Params
	phase_left1  = filter_desc->filter_params->next->param.fval;
	phase_right1 = filter_desc->filter_params->next->next->param.fval;
	phase_left2  = filter_desc->filter_params->next->next->next->param.fval;
	phase_right2 = filter_desc->filter_params->next->next->next->next->param.fval;

	// Get Filter Output (Neuron Vector)
	disparity_nv = filter_desc->output->neuron_vector;

	// Get Output Dimentions
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	// Calculate Disparity
	for (u = 0; u < h; u++)
	{
		for (v = 0; v < w; v++)
		{
			switch (complex_cells_distribution[(u * w) + v])
			{
				case SMALL:
					w0 = (3.0 * pi) / (double) (KERNEL_SIZE);
					break;
				case MEDIUM:
					w0 = (3.0 * pi) / (double) (2 * KERNEL_SIZE - 1);
					break;
				case BIG:
					w0 = (3.0 * pi) / (double) (4 * KERNEL_SIZE - 1);
					break;
			}

			sc_l1   = sc_l1_nv[(u * w) + v].output.fval;
			sc_r1   = sc_r1_nv[(u * w) + v].output.fval;
			sc_q_l1 = sc_q_l1_nv[(u * w) + v].output.fval;
			sc_q_r1 = sc_q_r1_nv[(u * w) + v].output.fval;

			sc_l2   = sc_l2_nv[(u * w) + v].output.fval;
			sc_r2   = sc_r2_nv[(u * w) + v].output.fval;
			sc_q_l2 = sc_q_l2_nv[(u * w) + v].output.fval;
			sc_q_r2 = sc_q_r2_nv[(u * w) + v].output.fval;

			c1 = (sc_l1 + sc_r1) * (sc_l1 + sc_r1) +
			     (sc_q_l1 + sc_q_r1) * (sc_q_l1 + sc_q_r1);

			c2 = (sc_l2 + sc_r2) * (sc_l2 + sc_r2) +
			     (sc_q_l2 + sc_q_r2) * (sc_q_l2 + sc_q_r2);

/*			if ((c1 == 0.0) && (c2 == 0.0))
			{
				disparity_nv[(u * w) + v].output.fval = 0.0;
				continue;
			}*/

			if ((sqrt (c1) < CUT_POINT) && (sqrt (c2) < CUT_POINT))
			{
				disparity_nv[(u * w) + v].output.fval = 0.0;
				continue;
			}

			a = c2 * cos (phase_left1 - phase_right1) - c1 * cos (phase_left2 - phase_right2);
			b = c2 * sin (phase_left1 - phase_right1) - c1 * sin (phase_left2 - phase_right2);

			if ((a == 0.0) && (b == 0.0))
			{
				disparity_nv[(u * w) + v].output.fval = 0.0;
				continue;
			}

			if (b != 0.0)
				d = atan (a / b);
			else
			{
				if (a > 0.0)
					d = pi/2.0;
				else
					d = -pi/2.0;
			}

			sin_val = (c2 - c1) / sqrt (a * a + b * b);

			if (sin_val > 1.0)
				sin_val = 1.0;
			if (sin_val < -1.0)
				sin_val = -1.0;

			disparity_nv[(u * w) + v].output.fval = (1.0 / w0) * asin (sin_val) - d / w0;
		}
	}
}


/*!
*********************************************************************************
* Function: density_function_cs							*
* Description: 									*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
density_function_cs (double *weigth_vector, int w, double sigma, double phase, double cycles, double offset, double scale_factor)
{
	int x, num_points = 0;
	double omega;
	int radius;

	omega = 2.0 * pi * cycles / (double) w;

	radius = w / 2;

	for (x = -radius; x < radius ; x++)
	{
		weigth_vector[num_points] =  offset + scale_factor * exp(-((x * x) / (2.0 * sigma * sigma))) * cos(omega * x - phase);
		num_points++;
	}
}



/*!
*********************************************************************************
* Function: distribution_array_cs						*
* Description: Calculate the distribution array of complex cells along V1	*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
distribution_array_cs (int *complex_cells_distribution, int w, int h)
{
	int i, j, mod;
	double *small_kernel, *medium_kernel, *big_kernel;
	static NEURON_LAYER *distribution = NULL;
	//FILE *file;

	small_kernel =  (double *) alloc_mem (w * sizeof(double));
	medium_kernel = (double *) alloc_mem (w * sizeof(double));
	big_kernel =    (double *) alloc_mem (w * sizeof(double));

	density_function_cs (small_kernel, w, 50.0, 0.0, 0.5, 0.0, 0.75);
	density_function_cs (big_kernel, w,  55.0, pi, 0.3, 0.75, 0.65);

	if(distribution == (NEURON_LAYER *) NULL)
		distribution = get_neuron_layer_by_name("complex_cells_distribution");
	//file = fopen("density_distribution.dat", "w");

	for (i = 0; i < w; i++)
	{
		medium_kernel[i] = (1.0 - small_kernel[i] - big_kernel[i]);

		for (j = 0; j < h; j++)
	      	{
	        	mod = rand() % h;

	                if(mod < small_kernel[i] * h)
	        		complex_cells_distribution[i + w * j] = SMALL;
	                else
	                {
	        	        if(mod < (small_kernel[i] + medium_kernel[i]) * h)
	        		        complex_cells_distribution[i + w * j] = MEDIUM;
	        	        else
	        		        complex_cells_distribution[i + w * j] = BIG;
	                }

			distribution->neuron_vector[i + w * j].output.ival = complex_cells_distribution[i + w * j];
			//fprintf(file, "%d %d %d\n", i, j, complex_cells_distribution[i + w * j]);
	        }
	}

	//fclose(file);
}



/*!
*********************************************************************************
* Function: compute_receptive_field_points_sc					*
* Description: Compute the receptive points of simple cells			*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_receptive_field_points_sc (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, int radius, double phase)
{
	int x, num_points = 0;
	double k, delta_angle, angle;
	POINT *points;

	points = (POINT *) alloc_mem (sizeof (POINT) * (2 * radius + 1));

	k = 1.0 / (2.0 * SIGMA * SIGMA);

	delta_angle = RANGE / (double) (2 * radius + 1);

	for (x = -radius, angle = -RANGE / 2.0 + delta_angle / 2.0; x < (radius + 1); x++)
	{
		points[num_points].x = x;
		points[num_points].y = 0;
		points[num_points].gabor = exp (-k * angle * angle) * cos (angle + phase);
		num_points++;
		angle += delta_angle;
	}
	receptive_field_descriptor->num_points = num_points;
	receptive_field_descriptor->points = points;
}



/*!
*********************************************************************************
* Function: compute_receptive_field_descriptor_sc_distribution			*
* Description: Compute the receptive descriptors of simple cells distribuiton	*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void
compute_receptive_field_descriptor_sc_distribution (FILTER_DESC *filter, double phase)
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	int radius;

	radius = (KERNEL_SIZE - 1)/ 2;

	receptive_field_descriptor = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (3 * sizeof (RECEPTIVE_FIELD_DESCRIPTION));

	compute_receptive_field_points_sc (&(receptive_field_descriptor[0]), radius, phase);
	compute_receptive_field_points_sc (&(receptive_field_descriptor[1]), 2 * radius, phase);
	compute_receptive_field_points_sc (&(receptive_field_descriptor[2]), 4 * radius, phase);

	filter->private_state = (void *) receptive_field_descriptor;
}



/*!
*********************************************************************************
* Function: receptive_field_sc							*
* Description: Compute the receptive average in a specific point x, y in the	*
* input at a distance d of the center of the retina				*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT
receptive_field_sc_distribution (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *filter_n_l, int x, int y,
		    		 int w, int h, int cone_organization, int concetrically_region, int color_model, int distribution)
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_p = NULL;
	int xr, yr, i;
	float intensity, yellow, rRGB, gRGB, bRGB, red, green, blue, k;
	NEURON_OUTPUT im;
	float d_im, d_im_prev;
	float delta_angle, angle;
	int pixel;

	receptive_field_p = &(receptive_field_descriptor[distribution]);

	delta_angle = RANGE / (float) receptive_field_p->num_points;

	d_im_prev = d_im = im.fval = 0.0;

	for (i = 0, angle = -RANGE / 2.0 + delta_angle / 2.0; i < receptive_field_p->num_points; i++)
	{
		xr = x + receptive_field_p->points[i].x;
		yr = y + receptive_field_p->points[i].y;

		if (xr < 0)
			continue;
		if (xr >= w)
			continue;
		if (yr < 0)
			continue;
		if (yr >= h)
			continue;

		pixel = filter_n_l->neuron_vector[yr * w + xr].output.ival;

		rRGB = (float) RED   (pixel);
		gRGB = (float) GREEN (pixel);
		bRGB = (float) BLUE  (pixel);

		if(color_model == HSI)
		{
			k = 255.0;
			if ((rRGB + gRGB + bRGB) == 0)
			{
				red   = 0.0;
				green = 0.0;
				blue  = 0.0;
			}
			else
			{
				red   = rRGB / (rRGB + gRGB + bRGB);
				green = gRGB / (rRGB + gRGB + bRGB);
				blue  = bRGB / (rRGB + gRGB + bRGB);
			}
		}
		else
		{
			k = 1.0;
			red   = rRGB;
			green = gRGB;
			blue  = bRGB;
		}

		switch (cone_organization)
		{
			case MCELLS:
				intensity = (k * (red + green + blue) / 3.0);
				if (concetrically_region == ONCENTER)
					d_im = receptive_field_p->points[i].gabor * intensity;
				else
					d_im = -receptive_field_p->points[i].gabor * intensity;
				break;

			case RED_GREEN:
				if (concetrically_region == ONCENTER)
					d_im = (red - green) * receptive_field_p->points[i].gabor;
				else
					d_im = (red - green) * (-receptive_field_p->points[i].gabor);
				break;

			case GREEN_RED:
				if (concetrically_region == ONCENTER)
					d_im = (green - red) * receptive_field_p->points[i].gabor;
				else
					d_im = (green - red) * (-receptive_field_p->points[i].gabor);
				break;

			case BLUE_YELLOW:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
					d_im = (blue - yellow) * receptive_field_p->points[i].gabor;
				else
					d_im = (blue - yellow) * (-receptive_field_p->points[i].gabor);
				break;

			case YELLOW_BLUE:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
					d_im = (- blue + yellow) * receptive_field_p->points[i].gabor;
				else
					d_im = (- blue + yellow) * (-receptive_field_p->points[i].gabor);
				break;
		}

		im.fval += ((d_im + d_im_prev) * delta_angle) / 2.0;
		d_im_prev = d_im;
		angle += delta_angle;
	}

	return im;
}



/*!
*********************************************************************************
* Function: receptive_field_sc							*
* Description: Compute the receptive average in a specific point x, y in the	*
* input at a distance d of the center of the retina				*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT
receptive_field_sc (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *filter_n_l, int x, int y,
		    int w, int h, int cone_organization, int concetrically_region, int color_model)
{
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_p = NULL;
	int xr, yr, i;
	float intensity, yellow, rRGB, gRGB, bRGB, red, green, blue, k;
	NEURON_OUTPUT im;
	float d_im, d_im_prev;
	float delta_angle, angle;
	int pixel;

	receptive_field_p = receptive_field_descriptor;

	delta_angle = RANGE / (float) receptive_field_p->num_points;

	d_im_prev = d_im = im.fval = 0.0;

	for (i = 0, angle = -RANGE / 2.0 + delta_angle / 2.0; i < receptive_field_p->num_points; i++)
	{
		xr = x + receptive_field_p->points[i].x;
		yr = y + receptive_field_p->points[i].y;

		if (xr < 0)
			continue;
		if (xr >= w)
			continue;
		if (yr < 0)
			continue;
		if (yr >= h)
			continue;

		pixel = filter_n_l->neuron_vector[yr * w + xr].output.ival;

		rRGB = (float) RED   (pixel);
		gRGB = (float) GREEN (pixel);
		bRGB = (float) BLUE  (pixel);

		if(color_model == HSI)
		{
			k = 255.0;
			if ((rRGB + gRGB + bRGB) == 0)
			{
				red   = 0.0;
				green = 0.0;
				blue  = 0.0;
			}
			else
			{
				red   = rRGB / (rRGB + gRGB + bRGB);
				green = gRGB / (rRGB + gRGB + bRGB);
				blue  = bRGB / (rRGB + gRGB + bRGB);
			}
		}
		else
		{
			k = 1.0;
			red   = rRGB;
			green = gRGB;
			blue  = bRGB;
		}

		switch (cone_organization)
		{
			case MCELLS:
				intensity = (k * (red + green + blue) / 3.0);
				if (concetrically_region == ONCENTER)
					d_im = receptive_field_p->points[i].gabor * intensity;
				else
					d_im = -receptive_field_p->points[i].gabor * intensity;
				break;

			case RED_GREEN:
				if (concetrically_region == ONCENTER)
					d_im = (red - green) * receptive_field_p->points[i].gabor;
				else
					d_im = (red - green) * (-receptive_field_p->points[i].gabor);
				break;

			case GREEN_RED:
				if (concetrically_region == ONCENTER)
					d_im = (green - red) * receptive_field_p->points[i].gabor;
				else
					d_im = (green - red) * (-receptive_field_p->points[i].gabor);
				break;

			case BLUE_YELLOW:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
					d_im = (blue - yellow) * receptive_field_p->points[i].gabor;
				else
					d_im = (blue - yellow) * (-receptive_field_p->points[i].gabor);
				break;

			case YELLOW_BLUE:
				yellow = (red + green) / 2.0;
				if (concetrically_region == ONCENTER)
					d_im = (- blue + yellow) * receptive_field_p->points[i].gabor;
				else
					d_im = (- blue + yellow) * (-receptive_field_p->points[i].gabor);
				break;
		}

		im.fval += ((d_im + d_im_prev) * delta_angle) / 2.0;
		d_im_prev = d_im;
		angle += delta_angle;
	}

	return im;
}



/*!
*********************************************************************************
* Function: compute_receptive_field_descriptor_sc				*
* Description: Compute the receptive descriptor of simple cells 		*
* Inputs: wavelength - wavelength of the stripes of the Gabor function in pixels*
*	  sigma - variance of the gaussian of the Gabor function in pixels	*
*	  aspect_ratio - ellipticity of the support of the Gabor function	*
*	  	aspect_ratio = 1, the support is circular			*
*		aspect_ratio < 1, the support is elongated in the orientation 	*
*		 of the parallel stripes of the function 			*
*	  orientation - oientation of the normal to the parallel stripes 	*
*	  	of the Gabor function						*
*	  phase - phase of the stripes						*
* Output: -						 			*
*********************************************************************************
*/

void
compute_receptive_field_descriptor_sc (FILTER_DESC *filter, double wavelength, double sigma, double aspect_ratio, double orientation, double phase)
{
	SC_RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	int x, y, kernel_radius, kernel_size, num_points;
	float xr, yr;
	float *kernel_points;
	FILE *points_file = NULL;
	char file_name[40];
	char strAux[256];
	int i;

	receptive_field_descriptor = (SC_RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof (SC_RECEPTIVE_FIELD_DESCRIPTION));

	// Calculates the kernel size
	kernel_size = 2.0 * 3.0 * sigma + 1; /* kernel size must be an odd number */

	sprintf (file_name, "sc_kernel_%d.out", kernel_size);
	points_file = fopen (file_name, "w");

	kernel_points = (float *) alloc_mem (sizeof (float) * kernel_size * kernel_size);

	// Calculates the radius size of the receptive field
	kernel_radius = (kernel_size - 1) / 2;

	num_points = 0;
	for (y = - kernel_radius; y <= kernel_radius; y++)
	{
		for (x = - kernel_radius; x <= kernel_radius; x++)
		{
			// Calculates the Rotated Coordinates
			xr =   x * cos (orientation) + y * sin (orientation);
			yr = - x * sin (orientation) + y * cos (orientation);

			kernel_points[num_points] = exp (- (xr * xr + yr * yr * aspect_ratio * aspect_ratio) / (2.0 * sigma * sigma)) *
						    cos (2.0 * pi * xr / wavelength + phase);

			sprintf(strAux, "%d %d %f\n", x, y, kernel_points[num_points]);
			for (i = 0; strAux[i] != '\n'; i++)
				if (strAux[i] == ',') strAux[i] = '.';
			fprintf (points_file, "%s", strAux);

			num_points++;
		}
		fprintf(points_file,"\n");
	}

	fclose (points_file);

	// Saves the Receptive Field Descriptor Attributes
	receptive_field_descriptor->num_points	= num_points;
	receptive_field_descriptor->kernel_size	= kernel_size;
	receptive_field_descriptor->points	= kernel_points;
	filter->private_state = (void *) receptive_field_descriptor;
}



/*!
*********************************************************************************
* Function: simple_cells							*
* Description: compute the simple_cells output from neuron_layer input		*
* Inputs: filter_desc - Filter description					*
* Output: 						 			*
*********************************************************************************
*/

void
simple_cells (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	PARAM_LIST *p_list;
	NEURON_LAYER *input_nl;
	int i;
	int wavelength, sigma, kernel_type, num_points;
	double aspect_ratio, orientation, phase;
	SC_RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	float delta_area;
	float *kernel_points;

	// Check Params
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);

	if (i != 7)
	{
		Erro ("Wrong number of parameters. simple_cells filter needs six parameters.\n",
		      "wavelength, sigma, aspect_ratio, orientation, phase and kernel_type, respectivelly.", "");
		return;
	}

	// Get Params
	wavelength    	= filter_desc->filter_params->next->param.ival;
	sigma 		= filter_desc->filter_params->next->next->param.ival;
	aspect_ratio    = filter_desc->filter_params->next->next->next->param.fval;
	orientation     = filter_desc->filter_params->next->next->next->next->param.fval;
	phase           = filter_desc->filter_params->next->next->next->next->next->param.fval;
	kernel_type	= filter_desc->filter_params->next->next->next->next->next->next->param.ival;

	// Compute Kernel
	if (filter_desc->private_state == NULL)
		compute_receptive_field_descriptor_sc (filter_desc, wavelength, sigma, aspect_ratio, orientation, phase);
	receptive_field_descriptor = (SC_RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

	// Check Neuron Layer
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. simple_cells filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Get Neuron Layer
	input_nl = filter_desc->neuron_layer_list->neuron_layer;

	if (kernel_type == KERNEL_2D)
	{
		kernel_points = receptive_field_descriptor->points;
		num_points = receptive_field_descriptor->num_points;
	}
	else
	{
		kernel_points = &(receptive_field_descriptor->points[receptive_field_descriptor->kernel_size * (receptive_field_descriptor->kernel_size / 2)]);
		num_points = receptive_field_descriptor->kernel_size;
	}
	delta_area = 1.0 / (float) receptive_field_descriptor->num_points;

	optimized_2d_point_convolution (filter_desc->output, input_nl, kernel_points, delta_area,
					num_points, kernel_type, input_nl->output_type);
}



/*!
*********************************************************************************
* Function: simple_cells							*
* Description: compute the simple_cells output from neuron_layer input		*
* Inputs: filter_desc - Filter description					*
* Output: 						 			*
*********************************************************************************
*/

void
simple_cells_distribution (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *image_n_l = NULL;
	int i, u, v, h, w, hi, wi, xi, yi, color_model;
	double d, theta, correction, phase;
	int cone_organization, concetrically_region;

	// Check Params
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);

	if (i != 5)
	{
		Erro ("Wrong number of parameters. simple_cells_distribuiton filter needs four parameters.",
		"The organization of cone inputs, Concetrically organized regions, the Color Model and phase, respectivelly.", "");
		return;
	}

	// Get Params
	cone_organization    = filter_desc->filter_params->next->param.ival;
	concetrically_region = filter_desc->filter_params->next->next->param.ival;
	color_model          = filter_desc->filter_params->next->next->next->param.ival;
	phase                = filter_desc->filter_params->next->next->next->next->param.fval;

	// Check Neuron Layer
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. simple_cells_distribution filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Get Neuron Layer
	image_n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Get Image Dimentions
	wi = image_n_l->dimentions.x;
	hi = image_n_l->dimentions.y;
	w  = filter_desc->output->dimentions.x;
	h  = filter_desc->output->dimentions.y;

	// Compute Kernels
	if (filter_desc->private_state == NULL)
		compute_receptive_field_descriptor_sc_distribution (filter_desc, phase);

	if (complex_cells_distribution == NULL)
	{
		// Calculate Comple Cells Distribution
		complex_cells_distribution = (int *) alloc_mem (w * h* sizeof(int));
		distribution_array_cs (complex_cells_distribution, w, h);
	}

	correction = (double) h / (double) (h-1);

	for (u = 0; u < w/2; u++)
	{
		d = distance_from_image_center (wi, hi, w, h, w/2 - 1 - u, SERENO_MODEL);
		for (v = 0; v < h; v++)
		{
			theta = pi * (((double) h * (3.0 / 2.0) - ((double) v * correction)) / (double) h);
			xi = (int) (d * cos(theta) + 0.5) + wi/2;
			yi = (int) (d * sin(theta) + 0.5) + hi/2;

			filter_desc->output->neuron_vector[(v * w) + u].output =
				receptive_field_sc_distribution ((RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state, image_n_l, xi, yi, wi, hi,
								 cone_organization, concetrically_region, color_model, complex_cells_distribution[(v * w) + u]);
		}
	}

	for (u = w/2; u < w; u++)
	{
		d = distance_from_image_center (wi, hi, w, h, u - w/2, SERENO_MODEL);
		for (v = 0; v < h; v++)
		{
			theta = pi * (((double) h * (3.0 / 2.0) + ((double) v * correction)) / (double) h);
			xi = (int) (d * cos(theta) + 0.5) + wi/2;
			yi = (int) (d * sin(theta) + 0.5) + hi/2;

			filter_desc->output->neuron_vector[(v * w) + u].output =
				receptive_field_sc_distribution ((RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state, image_n_l, xi, yi, wi, hi,
								 cone_organization, concetrically_region, color_model, complex_cells_distribution[(v * w) + u]);
		}
	}
}



/*!
*********************************************************************************
* \brief Compute a complex cell neuron response from four gabor outputs.
* \param[in] filter_descriptor The pointer to the filter descriptor.
* \pre The filter descriptor created.
* \post The filter output updated.
* \return Nothing.
*********************************************************************************
*/

void
complex_cell_filter(FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_right, *nl_right_f, *nl_left, *nl_left_f;
	int i;
	int nNumNeurons;
	float fltSimpleCell, fltSimpleCell_f;

	// Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 4)
	{
		Erro ("Wrong number of neuron layers. The complex_cell_filter must be applied on four neuron layers.", "", "");
		return;
	}

	// Check the output type
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
	{
		if (n_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output of the neuron layers and the output of complex_cell_filter must be the same type.", "", "");
			return;
		}
	}

	// Get Neuron Layer
	nl_right = filter_desc->neuron_layer_list->neuron_layer;
	nl_left = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_right_f = filter_desc->neuron_layer_list->next->next->neuron_layer;
	nl_left_f = filter_desc->neuron_layer_list->next->next->next->neuron_layer;

	nNumNeurons = nl_right->dimentions.x * nl_right->dimentions.y;

	switch (filter_desc->output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0; i < nNumNeurons; i++)
			{
				// Simple Cell
				fltSimpleCell = nl_right->neuron_vector[i].output.ival + nl_left->neuron_vector[i].output.ival;
				fltSimpleCell_f = nl_right_f->neuron_vector[i].output.ival + nl_left_f->neuron_vector[i].output.ival;

				// Complex Cell
				filter_desc->output->neuron_vector[i].output.ival = (fltSimpleCell * fltSimpleCell) + (fltSimpleCell_f * fltSimpleCell_f);
			}
		}
		break;

		case GREYSCALE_FLOAT:
		{
			for (i = 0; i < nNumNeurons; i++)
			{
				// Simple Cell
				fltSimpleCell = nl_right->neuron_vector[i].output.fval + nl_left->neuron_vector[i].output.fval;
				fltSimpleCell_f = nl_right_f->neuron_vector[i].output.fval + nl_left_f->neuron_vector[i].output.fval;

				// Complex Cell
				filter_desc->output->neuron_vector[i].output.fval = (fltSimpleCell * fltSimpleCell) + (fltSimpleCell_f * fltSimpleCell_f);
			}
		}
		break;
	}
}



/*!
*********************************************************************************
* \brief max_association_filter.
* \param[in] filter_descriptor The pointer to the filter descriptor.
* \pre The filter descriptor created.
* \post The filter output updated.
* \return Nothing.
*********************************************************************************
*/

void max_association_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *filter_output = NULL;
	int nNumNL, nNumParam;
	int wo, ho;
	float *fltMaxValue;
	float fltDisparity;
	int i, j;

	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;

	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != nNumParam)
	{
		Erro ("Wrong number of neuron layers. When using the max_association_filter, the number of neuron layers must be the same as parameters.", "", "");
		return;
	}

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	// Dimensao da 'output' associada ao filtro
	wo = filter_output->dimentions.x;
	ho = filter_output->dimentions.y;

	// Alocar espaco auxiliar
	if (filter_desc->private_state == NULL)
		filter_desc->private_state = (void*)alloc_mem(sizeof(float)*wo*ho);

	fltMaxValue = (float*) filter_desc->private_state;

	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;
	fltDisparity = (float) p_list->param.ival;

	for (j = 0; j < wo*ho; j++)
	{
		fltMaxValue[j] = nl->neuron_vector[j].output.fval;
		switch (filter_output->output_type)
		{
			case GREYSCALE:
			{
				filter_output->neuron_vector[j].output.ival = (int) fltDisparity;
			}
			break;

			case GREYSCALE_FLOAT:
			{
				filter_output->neuron_vector[j].output.fval = fltDisparity;
			}
			break;
		}
	}

	n_list = n_list->next;
	p_list = p_list->next;

	// Percorre as proximas neuron layer (se houver)
	for (i = 1; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		nl = n_list->neuron_layer;
		fltDisparity = (float) p_list->param.ival;

		for (j = 0; j < wo*ho; j++)
		{
			if (nl->neuron_vector[j].output.fval > fltMaxValue[j])
			{
				fltMaxValue[j] = nl->neuron_vector[j].output.fval;
				switch (filter_output->output_type)
				{
					case GREYSCALE:
					{
						filter_output->neuron_vector[j].output.ival = (int) fltDisparity;
					}
					break;

					case GREYSCALE_FLOAT:
					{
						filter_output->neuron_vector[j].output.fval = fltDisparity;
					}
					break;
				}
			}
		}
	}
}



/*!
*********************************************************************************
* \brief max_average_association_filter.
* \param[in] filter_descriptor The pointer to the filter descriptor.
* \pre The filter descriptor created.
* \post The filter output updated.
* \return Nothing.
*********************************************************************************
*/

void max_average_association_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	static RECEPTIVE_FIELD_DESCRIPTION receptive_field_descriptor;
	static NEURON_LAYER *filter_output = NULL;
	static int nNumNL, nNumParam;
	static int flagInit = 0;
	static float *fltMaxAvarageValue;
	static int wo, ho;
	float fltDisparity;
	int i, x, y;
	float fltAux;
	float fltSigma;
	int nKernelSize;

	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);

		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;

		// Numero de neuron layers deve ser igual ao numero de parametros
		if (nNumNL != (nNumParam-2))
		{
			Erro ("Wrong number of parameters for min_avarage_association_filter.", "", "");
			return;
		}

		// Buscar o KernelSize e o Sigma
		for (i = 0, p_list = filter_desc->filter_params; i <= nNumNL; p_list = p_list->next, i++);
		nKernelSize = p_list->param.ival;
		fltSigma = p_list->next->param.fval;

		// Ponteiro para a 'output' associada ao filtro
		filter_output = filter_desc->output;

		// Dimensao da 'output' associada ao filtro
		wo = filter_output->dimentions.x;
		ho = filter_output->dimentions.y;

		// Alocar espaco auxiliar
		fltMaxAvarageValue = (float*)alloc_mem(sizeof(float)*wo*ho);

		// Inicializa o kernel com a gausiana
		compute_gaussian_kernel(&receptive_field_descriptor, nKernelSize, fltSigma);

		flagInit = 1;
	}

	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;
	fltDisparity = (int) p_list->param.ival;

	for (x = 0; x < wo; x++)
	{
		for (y = 0; y < ho; y++)
		{
			fltMaxAvarageValue[y * wo + x] = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);

			switch (filter_output->output_type)
			{
				case GREYSCALE:
				{
					filter_output->neuron_vector[y * wo + x].output.ival = (int) fltDisparity;
				}
				break;

				case GREYSCALE_FLOAT:
				{
					filter_output->neuron_vector[y * wo + x].output.fval = fltDisparity;
				}
				break;
			}
		}
	}

	n_list = n_list->next;
	p_list = p_list->next;

	// Percorre as proximas neuron layers (se houver)
	for (i = 1; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		nl = n_list->neuron_layer;
		fltDisparity = (int) p_list->param.ival;

		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				fltAux = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);
				if (fltAux > fltMaxAvarageValue[y * wo + x])
				{
					fltMaxAvarageValue[y * wo + x] = fltAux;

					switch (filter_output->output_type)
					{
						case GREYSCALE:
						{
							filter_output->neuron_vector[y * wo + x].output.ival = (int) fltDisparity;
						}
						break;

						case GREYSCALE_FLOAT:
						{
							filter_output->neuron_vector[y * wo + x].output.fval = fltDisparity;
						}
						break;
					}
				}
			}
		}
	}
}



/*!
*********************************************************************************
* Function: mt_cell
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void
mt_cell (FILTER_DESC *filter_desc)
{
	int i;
	NEURON_LAYER_LIST *n_list;
	float selectivity;
	int *disparity;

	NEURON *nl_simple_mono_right;
	NEURON *nl_simple_mono_right_q;
	NEURON *nl_simple_mono_left;
	NEURON *nl_simple_mono_left_q;

	NEURON *filter_output;

	int hi;
	int wi;
	int ho;
	int wo;

	//compute each mt cell
	int yo, xo;

	int yi;
	int xir;
	int xil;

	//CELULA SIMPLES BINOCULAR
	double r;
	double l;
	double sumSimpleBinocularCell;

	//CELULA SIMPLES BINOCULAR
	double rq;
	double lq;
	double sumSimpleBinocularCellQ;

	//CELULA COMPLEXA MONOCULAR
	double sumComplexMonoRightCell;
	double sumComplexMonoLeftCell;

	//CELULA COMPLEXA BINOCULAR
	double sumComplexBinocular;

	//CELULA MT
	double mt;

	PARAM_LIST *p_list;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
	{
		if (n_list->next != NULL)
		{
			if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
			{
				Erro ("The mt_cell must be applied on neuron layers with the same output_type.", "", "");
				return;
			}
		}
	}

	if (i != 4)
	{
		Erro ("Wrong number of neuron layers. The mt_cell must be applied on four neuron layers.", "", "");
		return;
	}

	// Achar o numero de parametros

	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;
	i--;

	// Numero de neuron layers deve ser igual a um
	if (i != 2)
	{
		Erro ("use: mt_cell (float Selectivity, int *Disparity)", "", "");
		return;
	}

	selectivity = filter_desc->filter_params->next->param.fval;
	disparity = (int *) (filter_desc->filter_params->next->next->param.pval);

	nl_simple_mono_right   = filter_desc->neuron_layer_list->neuron_layer->neuron_vector;
	nl_simple_mono_right_q = filter_desc->neuron_layer_list->next->neuron_layer->neuron_vector;
	nl_simple_mono_left    = filter_desc->neuron_layer_list->next->next->neuron_layer->neuron_vector;
	nl_simple_mono_left_q  = filter_desc->neuron_layer_list->next->next->next->neuron_layer->neuron_vector;

	filter_output 	       = filter_desc->output->neuron_vector;

	hi = filter_desc->neuron_layer_list->neuron_layer->dimentions.y;
	wi = filter_desc->neuron_layer_list->neuron_layer->dimentions.x;
	ho = filter_desc->output->dimentions.y;
	wo = filter_desc->output->dimentions.x;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			yi  = (int) ((float) (yo * hi) / (float) ho + 0.5);
			xir = (int) ((float) (xo * wi) / (float) wo + 0.5);
			xil = xir + *disparity;

			//CELULA SIMPLES BINOCULAR
			r = nl_simple_mono_right[(yi*wi) + xir].output.fval;
			l = nl_simple_mono_left[(yi*wi) + xil].output.fval;
			sumSimpleBinocularCell = (r + l);

			//CELULA SIMPLES BINOCULAR
			rq = nl_simple_mono_right_q[(yi*wi) + xir].output.fval;
			lq = nl_simple_mono_left_q[(yi*wi) + xil].output.fval;
			sumSimpleBinocularCellQ = (rq + lq);

#define POW2(a) ((a)*(a))

			//CELULA COMPLEXA MONOCULAR
			sumComplexMonoRightCell = POW2(r) + POW2(rq);
			sumComplexMonoLeftCell  = POW2(l) + POW2(lq);

			//CELULA COMPLEXA BINOCULAR
			sumComplexBinocular = POW2(sumSimpleBinocularCell) + POW2(sumSimpleBinocularCellQ);

			//CELULA MT
			mt = sumComplexBinocular / (sumComplexMonoLeftCell + sumComplexMonoRightCell + selectivity);
			filter_output[(yo*wo) + xo].output.fval = mt;
		}
	}

}


int
disparity_map_filter_distance_neighbour (DISPARITY_SAMPLE *sample, int disparity, int x, int y, int w, int h)
{
	int dist, count;

	dist = count = 0;
	if (x-1 >= 0)
	{
		dist += abs (sample[y*w + x-1].disparity - disparity);
		count++;
	}
	if (x+1 < w)
	{
		dist += abs (sample[y*w + x+1].disparity - disparity);
		count++;
	}
	if (y-1 >= 0)
	{
		dist += abs (sample[(y-1)*w + x].disparity - disparity);
		count++;
	}
	if (y+1 < h)
	{
		dist += abs (sample[(y+1)*w + x].disparity - disparity);
		count++;
	}

	if ((x-1 >= 0) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x-1].disparity - disparity);
		count++;
	}
	if ((x-1 >= 0) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x-1].disparity - disparity);
		count++;
	}
	if ((x+1 < w) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x+1].disparity - disparity);
		count++;
	}
	if ((x+1 < w) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x+1].disparity - disparity);
		count++;
	}

	return ((int) ((double) dist / (double) count + 0.5));
}


void
disparity_map_filter_winner_takes_it_all (NEURON_LAYER *nlDisparityMap, DISPARITY_DATA* dispData)
{
	DISPARITY_SAMPLE temp;
	int x, y, w, h, i, sample, best_sample, cur_dist, dist;
	NEURON *nv;

	w = nlDisparityMap->dimentions.x;
	h = nlDisparityMap->dimentions.y;
	nv = nlDisparityMap->neuron_vector;

	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (best_sample = sample = 0; sample < NUM_DISPARITY_SAMPLES; sample++)
			{
				dist = disparity_map_filter_distance_neighbour (dispData->samples[0], dispData->samples[sample][y * w + x].disparity, x, y, w, h);
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
		nv[i].output.fval = (float) dispData->samples[0][i].disparity;
	}
}


void
init_disparity_map_filter_private_data (DISPARITY_DATA* dispData, int num_neurons)
{
	int i, sample;

	for (i = 0; i < num_neurons; i++)
		dispData->neg_slope[i] = 1;
	for (sample = 0; sample < NUM_DISPARITY_SAMPLES; sample++)
	{
		for (i = 0; i < num_neurons; i++)
		{
			dispData->samples[sample][i].val = FLT_MAX;
			dispData->samples[sample][i].disparity = 0;
		}
	}
	for (i = 0; i < num_neurons; i++)
		dispData->mt_gaussian_with_previous_disparity[i] = FLT_MAX;
}


DISPARITY_DATA *
alloc_and_init_disparity_map_filter_private_data (int num_neurons)
{
	int i;

	// Aloca memoria para a estrutura DISPARITY_DATA
	DISPARITY_DATA* dispData = (DISPARITY_DATA *) alloc_mem (sizeof(DISPARITY_DATA));

	dispData->neg_slope = (char *) alloc_mem (sizeof (char) * num_neurons);

	dispData->samples = (DISPARITY_SAMPLE **) alloc_mem (sizeof (DISPARITY_SAMPLE *) * NUM_DISPARITY_SAMPLES);
	for (i = 0; i < NUM_DISPARITY_SAMPLES; i++)
		dispData->samples[i] = (DISPARITY_SAMPLE *) alloc_mem (sizeof (DISPARITY_SAMPLE) * num_neurons);

	dispData->mt_gaussian_with_previous_disparity = (float *) alloc_mem (sizeof (float) * num_neurons);

	init_disparity_map_filter_private_data (dispData, num_neurons);

	return (dispData);
}


void
disparity_map_filter_add_local_min (DISPARITY_DATA* dispData, int i, int disparity)
{
	int num_samples = NUM_DISPARITY_SAMPLES;
	int victim_sample, moving_sample;

	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
	{
		if (dispData->mt_gaussian_with_previous_disparity[i] <= dispData->samples[victim_sample][i].val)
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
		dispData->samples[moving_sample][i].val = dispData->mt_gaussian_with_previous_disparity[i];
		dispData->samples[moving_sample][i].disparity = disparity;
	}
}


/*!
*********************************************************************************
* Function: disparity_map_filter
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void
disparity_map_filter (FILTER_DESC *filter_desc)
{
	int i, x, y;
	NEURON_LAYER_LIST *n_list;
	PARAM_LIST *p_list;
	DISPARITY_DATA *dispData;
	int *filter_state;
	int *disparity;

	NEURON *filter_input;
	NEURON *filter_output;

	int wo;
	int ho;

	float *mt_gaussian_with_previous_disparity;
	char *neg_slope;

	int n;
	float current_mt_gaussian;

	if (filter_desc->private_state == NULL)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			;

		if (i != 1)
		{
			Erro ("Wrong number of neuron layers. The disparity_map_filter must be applied on only one neuron layer.", "", "");
			return;
		}

		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the disparity_map_filter output.", "", "");
			return;
		}

		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
			;

		if (--i != 2)
		{
			Erro ("Wrong number of parameters. The disparity_map_filter have 2 parameters: state and disparity respectivally.", "", "");
			return;
		}

		dispData = alloc_and_init_disparity_map_filter_private_data (filter_desc->output->dimentions.x * filter_desc->output->dimentions.y);
		filter_desc->private_state = (void *) dispData;
	}
	else
		dispData = (DISPARITY_DATA *) filter_desc->private_state;

	filter_state   		= (int *) &(filter_desc->filter_params->next->param.ival);
	disparity		= (int *) (filter_desc->filter_params->next->next->param.pval);

	filter_input 	= filter_desc->neuron_layer_list->neuron_layer->neuron_vector;
	filter_output 	= filter_desc->output->neuron_vector;

	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	switch (*filter_state)
	{
		case DISPARITY_MAP_FILTER_INIT:
		{
			init_disparity_map_filter_private_data (dispData, wo*ho);
		}
		break;

		case DISPARITY_MAP_FILTER_SAMPLE:
		{
			mt_gaussian_with_previous_disparity = dispData->mt_gaussian_with_previous_disparity;
			neg_slope = dispData->neg_slope;

			for (y = 0; y < ho; y++)
			{
				for (x = 0; x < wo; x++)
				{
					n = y * wo + x;
					current_mt_gaussian = filter_input[n].output.fval;
					if (current_mt_gaussian >= mt_gaussian_with_previous_disparity[n])
					{
						if (neg_slope[n])
						{
							neg_slope[n] = 0;
							disparity_map_filter_add_local_min (dispData, n, (int) (filter_output[n].output.fval));
						}
					}
					else
					{
						neg_slope[n] = 1;
					}
					filter_output[n].output.fval = (float) (*disparity);
					mt_gaussian_with_previous_disparity[n] = current_mt_gaussian;
				}
			}
		}
		break;

		case DISPARITY_MAP_FILTER_OPTIMIZE:
		{
			// Escolhe, inicialmente, a menor disparidade
			for (i = 0; i < (wo * ho); i++)
				filter_output[i].output.fval = dispData->samples[0][i].disparity;

			// Improve the disparity map by applying the WINNER TAKES IT ALL algorithm.
			for (i = 0; i < DISPARITY_MAP_FILTER_WINNER_TAKES_IT_ALL_STEPS; i++)
				disparity_map_filter_winner_takes_it_all (filter_desc->output, dispData);
		}
		break;
	}
}

/*!
*********************************************************************************
* Function: compute_scale_factor
* Description: Computes scale factor based object of interest width in input image (planar)
* 				and in neurons activation band considering log polar transformation
* Inputs:
* 			l -> activation band width of the neuron layer
* 			c -> original width of the region of interest
* 			u -> target width (resized) of the region of interest
* 			m -> width of the neuron layer
* 			a -> log factor of the neuron's distribution
* Output: scale factor multiplier (values greater than 1.0 scale up and values less than 1.0 scale down)
*********************************************************************************
*/
float
compute_scale_factor(float l, float c, float u, float m, float a)
{
	if ( (l <= 0.0f) ||(c <= 0.0f) || (u <= 0.0f) || (m <= 0.0f) || (a < 2.0f) )
		return 0.0f;

	return (u * (pow(a, (l / m)) - 1.0f)) / (c * (a - 1.0f));
}
