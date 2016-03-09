#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"

// ----------------------------------------------------------------------------
// log_polar_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void log_polar_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *n_l = NULL;
	PARAM_LIST *p_list = NULL;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi, index;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	float fltLogFactor;
	float hAux;
	int nNumParam;

	previous_output.ival = 0;

	// Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. The log_polar_filter filter must be applied on only one neuron layer.", "", "");
		return;
	}
	n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;

	// Numero de neuron layers deve ser igual a tres
	if (nNumParam != 1)
	{
		Erro ("The log_polar_filter filter must have one parameter: log_factor.", "", "");
		return;
	}
	fltLogFactor = filter_desc->filter_params->next->param.fval;
// 	printf("fltLogFactor: %f\n", fltLogFactor);

	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

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

	hAux = (float) h / (float) (h - 1.0);
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, hAux, fltLogFactor);

			index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
				if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					previous_output.ival = filter_desc->output->neuron_vector[(v * w) + u].output.ival = 0;
				else
					previous_output = filter_desc->output->neuron_vector[(v * w) + u].output = input->neuron_layer->neuron_vector[yi * wi + xi].output;
			previous_xi = xi;
			previous_yi = yi;
		}
	}
}

// ----------------------------------------------------------------------------
// min_association_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void min_association_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	PARAM_LIST *p_list;
	NEURON_LAYER *filter_output;
	int nNumNL, nNumParam;
	float *fltMinValue;
	int wo, ho;
	float fltDisparity;
	int i, j;
	float fltAvgDisparity;
	
	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
//	nNumNL--;
//	printf("nNumNL: %d\n", nNumNL);

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;
//	printf("nNumParam: %d\n", nNumParam);

	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != nNumParam)
	{
		Erro ("The number of neuron layers must be the same as parameters.", "", "");
		return;
	}

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	// Dimensao da 'output' associada ao filtro
	wo = filter_output->dimentions.x;
	ho = filter_output->dimentions.y;
	
	// Alocar espaco auxiliar
	if (filter_desc->private_state == NULL)
		filter_desc->private_state = (void*)malloc(sizeof(float)*wo*ho);

	fltMinValue = (float*) filter_desc->private_state;

	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;
	fltDisparity = (float) p_list->param.ival;

	for (j = 0; j < wo*ho; j++)
	{
		fltMinValue[j] = nl->neuron_vector[j].output.fval;
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
			if (nl->neuron_vector[j].output.fval < fltMinValue[j])
			{
				fltMinValue[j] = nl->neuron_vector[j].output.fval;
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

	// Imprime a media
	switch (filter_output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += (float) filter_output->neuron_vector[i].output.ival, i++);
		}
		break;
		
		case GREYSCALE_FLOAT:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += filter_output->neuron_vector[i].output.fval, i++);
		}
		break;
	}
	fltAvgDisparity /= wo*ho;
	printf("Avarage disparity = %.2f\n", fltAvgDisparity);
}

// ----------------------------------------------------------------------------
// min_avarage_association_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: float
// ----------------------------------------------------------------------------
void min_avarage_association_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	static RECEPTIVE_FIELD_DESCRIPTION receptive_field_descriptor;
	static NEURON_LAYER *filter_output = NULL;
	static int nNumNL, nNumParam;
	static int flagInit = 0;
	static float *fltMinAvarageValue;
	static int wo, ho;
	float fltDisparity;
	int i, x, y;
	float fltAvgDisparity, fltAux;
	float fltSigma;
	int nKernelSize;

	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
//		nNumNL--;
//		printf("nNumNL: %d\n", nNumNL);
	
		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;
//		printf("nNumParam: %d\n", nNumParam);
	
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
		fltMinAvarageValue = (float*)malloc(sizeof(float)*wo*ho);

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
			fltMinAvarageValue[y * wo + x] = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);

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
				if (fltAux < fltMinAvarageValue[y * wo + x])
				{
					fltMinAvarageValue[y * wo + x] = fltAux;

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

	// Imprime a media
	switch (filter_output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += (float) filter_output->neuron_vector[i].output.ival, i++);
		}
		break;
		
		case GREYSCALE_FLOAT:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += filter_output->neuron_vector[i].output.fval, i++);
		}
		break;
	}
	fltAvgDisparity /= wo*ho;
	printf("Avarage disparity = %.2f\n", fltAvgDisparity);
}

// ----------------------------------------------------------------------------
// x_min_lagrange -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: float
// ----------------------------------------------------------------------------
float x_min_lagrange(float x0, float y0, float x1, float y1, float x2, float y2)
{
	float a, b;
	float a0, a1, a2;

	// Dado 3 pontos, calcula o polinomio de lagrange de segundo grau
	// na forma: ax2 + bx + c
	// Para termos a abscissa do ponto de derivada nula, temos que X = -b/2a
	a0 = y0 / ((x0 - x1)  * (x0 - x2));
	a1 = y1 / ((x1 - x0)  * (x1 - x2));
	a2 = y2 / ((x2 - x0)  * (x2 - x1));
	a = a0 + a1 + a2;
	b = -(x1 + x2)*a0 -(x0 + x2)*a1 -(x0 + x1)*a2;
	return -b/(2*a);
}

// ----------------------------------------------------------------------------
// min_association_interpolation_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void min_association_interpolation_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list, *n_list_previous, *n_list_next;
	NEURON_LAYER *nl;
	PARAM_LIST *p_list, *p_list_previous, *p_list_next;
	static NEURON_LAYER *filter_output = NULL;
	static int nNumNL, nNumParam;
	static int flagInit = 0;
	static float *fltMinValue;
	static int wo, ho;
	float fltDisparity;
	static float fltMinDisparity, fltMaxDisparity;
	int i, j;
	float fltAvgDisparity;
	float x0, y0, x1, y1, x2, y2;

	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
//		nNumNL--;
//		printf("nNumNL: %d\n", nNumNL);
	
		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;
//		printf("nNumParam: %d\n", nNumParam);
	
		// Numero de neuron layers deve ser igual ao numero de parametros
		if (nNumNL != nNumParam)
		{
			Erro ("The number of neuron layers must be the same as parameters.", "", "");
			return;
		}

		// Busca a menor e a maior disparidade
		fltMinDisparity = (float) filter_desc->filter_params->next->param.ival;
		for (i = 0, p_list = filter_desc->filter_params; i < nNumParam; i++)
			p_list = p_list->next;
		fltMaxDisparity = (float) p_list->param.ival;
		printf("fltMinDisparity: %f - fltMaxDisparity: %f\n", fltMinDisparity, fltMaxDisparity);

		// Ponteiro para a 'output' associada ao filtro
		filter_output = filter_desc->output;

		// Dimensao da 'output' associada ao filtro
		wo = filter_output->dimentions.x;
		ho = filter_output->dimentions.y;
		
		// Alocar espaco auxiliar
		fltMinValue = (float*)malloc(sizeof(float)*wo*ho);

		flagInit = 1;
	}

	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;
	fltDisparity = (float) p_list->param.ival;

	for (i = 0; i < wo*ho; i++)
	{
		fltMinValue[i] = nl->neuron_vector[i].output.fval;
		switch (filter_output->output_type)
		{
			case GREYSCALE:
			{
				filter_output->neuron_vector[i].output.ival = (int) fltDisparity;
			}
			break;
			
			case GREYSCALE_FLOAT:
			{
				filter_output->neuron_vector[i].output.fval = fltDisparity;
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
			if (nl->neuron_vector[j].output.fval < fltMinValue[j])
			{
				fltMinValue[j] = nl->neuron_vector[j].output.fval;
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

	// Vai saber -- Ai fudeu... Com forca ainda...
	for (i = 0; i < wo * ho; i++)
	{
		fltDisparity = filter_output->neuron_vector[i].output.fval;
		if ((fltDisparity != fltMaxDisparity) && (fltDisparity != fltMinDisparity))
		{
			// Encontrar as neuron_layers com disparidade imediatamente anterior e posterior
			n_list = filter_desc->neuron_layer_list;
			p_list = filter_desc->filter_params->next;
			do
			{
				n_list_previous = n_list;
				n_list = n_list->next;
				n_list_next = n_list->next;
				p_list_previous = p_list;
				p_list = p_list->next;
				p_list_next = p_list->next;
			} while (p_list->param.ival < (int)fltDisparity);

			x0 = (float) p_list_previous->param.ival;
			y0 = (float) n_list_previous->neuron_layer->neuron_vector[i].output.fval;
			x1 = (float) p_list->param.ival;
			y1 = (float) n_list->neuron_layer->neuron_vector[i].output.fval;
			x2 = (float) p_list_next->param.ival;
			y2 = (float) n_list_next->neuron_layer->neuron_vector[i].output.fval;

			if ((y1 < y2) && (y1 < y0))
				filter_output->neuron_vector[i].output.fval = x_min_lagrange(x0, y0, x1, y1, x2, y2);
		}
	}

	// Imprime a media
	switch (filter_output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += (float) filter_output->neuron_vector[i].output.ival, i++);
		}
		break;
		
		case GREYSCALE_FLOAT:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += filter_output->neuron_vector[i].output.fval, i++);
		}
		break;
	}
	fltAvgDisparity /= wo*ho;
	printf("Avarage disparity = %.2f\n", fltAvgDisparity);
}

// ----------------------------------------------------------------------------
// min_avarage_association_interpolation_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void min_avarage_association_interpolation_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list, *n_list_previous, *n_list_next;
	NEURON_LAYER *nl;
	PARAM_LIST *p_list, *p_list_previous, *p_list_next;
	static RECEPTIVE_FIELD_DESCRIPTION receptive_field_descriptor;
	static NEURON_LAYER *filter_output = NULL;
	static int nNumNL, nNumParam;
	static int flagInit = 0;
	static float *fltMinAvarageValue;
	static int wo, ho;
	float fltDisparity;
	static float fltMinDisparity, fltMaxDisparity;
	int i, x, y;
	float fltAvgDisparity, fltAux;
	float fltSigma;
	int nKernelSize;
	float x0, y0, x1, y1, x2, y2;

	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
//		nNumNL--;
//		printf("nNumNL: %d\n", nNumNL);
	
		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;
//		printf("nNumParam: %d\n", nNumParam);
	
		// Numero de neuron layers deve ser igual ao numero de parametros
		if (nNumNL != (nNumParam-2))
		{
			Erro ("Wrong number of parameters for min_avarage_association_interpolation_filter.", "", "");
			return;
		}

		// Buscar o KernelSize e o Sigma
		for (i = 0, p_list = filter_desc->filter_params; i <= nNumNL; p_list = p_list->next, i++);
		nKernelSize = p_list->param.ival;
		fltSigma = p_list->next->param.fval;

		// Busca a menor e a maior disparidade
		fltMinDisparity = (float) filter_desc->filter_params->next->param.ival;
		for (i = 0, p_list = filter_desc->filter_params->next; i <= (nNumNL - 2); p_list = p_list->next, i++);
		fltMaxDisparity = (float) p_list->param.ival;
		printf("fltMinDisparity: %f - fltMaxDisparity: %f\n", fltMinDisparity, fltMaxDisparity);

		// Ponteiro para a 'output' associada ao filtro
		filter_output = filter_desc->output;

		// Dimensao da 'output' associada ao filtro
		wo = filter_output->dimentions.x;
		ho = filter_output->dimentions.y;
		
		// Alocar espaco auxiliar
		fltMinAvarageValue = (float*)malloc(sizeof(float)*wo*ho);

		// Inicializa o kernel com a gausiana
		compute_gaussian_kernel(&receptive_field_descriptor, nKernelSize, fltSigma);

		flagInit = 1;
	}

	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;
	fltDisparity = (float) p_list->param.ival;

	for (x = 0; x < wo; x++)
	{
		for (y = 0; y < ho; y++)
		{
			fltMinAvarageValue[y * wo + x] = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);

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
		fltDisparity = (float) p_list->param.ival;

		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				fltAux = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);
				if (fltAux < fltMinAvarageValue[y * wo + x])
				{
					fltMinAvarageValue[y * wo + x] = fltAux;

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

	// Vai saber -- Ai fudeu com forca ainda...
	for (i = 0; i < wo * ho; i++)
	{
		fltDisparity = filter_output->neuron_vector[i].output.fval;
		if ((fltDisparity != fltMaxDisparity) && (fltDisparity != fltMinDisparity))
		{
			// Encontrar as neuron_layers com disparidade imediatamente anterior e posterior
			n_list = filter_desc->neuron_layer_list;
			p_list = filter_desc->filter_params->next;
			do
			{
				n_list_previous = n_list;
				n_list = n_list->next;
				n_list_next = n_list->next;
				p_list_previous = p_list;
				p_list = p_list->next;
				p_list_next = p_list->next;
			} while (p_list->param.ival < (int)fltDisparity);

			x0 = (float) p_list_previous->param.ival;
			y0 = (float) n_list_previous->neuron_layer->neuron_vector[i].output.fval;
			x1 = (float) p_list->param.ival;
			y1 = (float) n_list->neuron_layer->neuron_vector[i].output.fval;
			x2 = (float) p_list_next->param.ival;
			y2 = (float) n_list_next->neuron_layer->neuron_vector[i].output.fval;

			if ((y1 < y2) && (y1 < y0))
				filter_output->neuron_vector[i].output.fval = x_min_lagrange(x0, y0, x1, y1, x2, y2);
		}
	}

	// Imprime a media
	switch (filter_output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += (float) filter_output->neuron_vector[i].output.ival, i++);
		}
		break;
		
		case GREYSCALE_FLOAT:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += filter_output->neuron_vector[i].output.fval, i++);
		}
		break;
	}
	fltAvgDisparity /= wo*ho;
	printf("Avarage disparity = %.2f\n", fltAvgDisparity);
}

// ----------------------------------------------------------------------------
// min_avarage_association2_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void min_avarage_association2_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	static RECEPTIVE_FIELD_DESCRIPTION receptive_field_descriptor;
	static NEURON_LAYER *filter_output = NULL;
	static int nNumNL, nNumParam;
	static int flagInit = 0;
	static float *fltMinAvarageValue;
	static int wo, ho;
	float fltDisparity;
	int i, x, y;
	float fltAvgDisparity, fltAux;
	float fltSigma;
	int nKernelSize;

	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
//		nNumNL--;
//		printf("nNumNL: %d\n", nNumNL);
	
		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;
//		printf("nNumParam: %d\n", nNumParam);
	
		// Numero de neuron layers deve ser igual ao numero de parametros
		if (nNumNL != (nNumParam-2))
		{
			Erro ("Wrong number of parameters for min_avarage_association2_filter.", "", "");
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
		fltMinAvarageValue = (float*)malloc(sizeof(float)*wo*ho);

		// Inicializa o kernel com a gausiana
		compute_gaussian_kernel(&receptive_field_descriptor, nKernelSize, fltSigma);

		flagInit = 1;
	}

	for (x = 0; x < wo * ho; x++)
	{
		fltMinAvarageValue[x] = 0.0;

		switch (filter_output->output_type)
		{
			case GREYSCALE:
			{
				filter_output->neuron_vector[x].output.ival = 0;
			}
			break;
			
			case GREYSCALE_FLOAT:
			{
				filter_output->neuron_vector[x].output.fval = 0.0;
			}
			break;
		}
	}
	
	// Copia a primeira camada neural
	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;
	nl = n_list->neuron_layer;

	// Percorre as proximas neuron layers (se houver)
	for (i = 0; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		nl = n_list->neuron_layer;
		fltDisparity = (float) p_list->param.ival;

		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				fltAux = apply_gaussian_kernel(&receptive_field_descriptor, nl, x, y);

				// RTBC
				if (fltAux == 0.0)
					fltAux = 0.000001;
					
				fltMinAvarageValue[y * wo + x] += 1.0 / (fltAux * fltAux);

				filter_output->neuron_vector[y * wo + x].output.fval += fltDisparity / (fltAux * fltAux);
			}
		}
	}
	
	for (x = 0; x < wo * ho; x++)
	{
		switch (filter_output->output_type)
		{
			case GREYSCALE:
			{
				filter_output->neuron_vector[x].output.ival = filter_output->neuron_vector[x].output.fval / fltMinAvarageValue[x];
			}
			break;
			
			case GREYSCALE_FLOAT:
			{
				filter_output->neuron_vector[x].output.fval = filter_output->neuron_vector[x].output.fval / fltMinAvarageValue[x];
			}
			break;
		}
	}
	// Imprime a media
	switch (filter_output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += (float) filter_output->neuron_vector[i].output.ival, i++);
		}
		break;
		
		case GREYSCALE_FLOAT:
		{
			for (i = 0, fltAvgDisparity = 0.0; i < wo*ho; fltAvgDisparity += filter_output->neuron_vector[i].output.fval, i++);
		}
		break;
	}
	fltAvgDisparity /= wo*ho;
	printf("Avarage disparity = %.2f\n", fltAvgDisparity);
}

// ----------------------------------------------------------------------------
// shift_vergence_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void shift_vergence_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *filter_output = NULL;
	int nNumNL;
	float fltRangeCols;
	float fltAvgDisparity, fltMinAvgDisparity;
	int nStartCol, nEndCol;
	int nNumParam;
	int nDisparity;
	int i, j, k;

	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
// 	printf("nNumNL: %d\n", nNumNL);
	
	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;
//	printf("nNumParam: %d\n", nNumParam);
	
	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != (nNumParam - 1))
	{
		Erro ("Wrong number of parameters for shift_vergence_filter.", "", "");
		return;
	}

	// Calcula o range de colunas para realizar a vergencia
	for (i = 0, p_list = filter_desc->filter_params; i < nNumParam; p_list = p_list->next, i++);
	fltRangeCols = p_list->param.fval / 100.0;	// Porcentagem
//	printf("fltRangeCols: %f\n", fltRangeCols);

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;

	// Inicializa com "infinito"
	fltMinAvgDisparity = FLT_MAX;
	
	// Percorre todas as neuron layers procurando a de menor saida media
	// e associando a disparidade correspondente
	for (i = 0; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		nl = n_list->neuron_layer;

		nStartCol = nl->dimentions.x * ((1.0 - fltRangeCols) / 2.0);
		nEndCol = nl->dimentions.x - nStartCol;

		fltAvgDisparity = 0.0;
		for (j = 0; j < nl->dimentions.y; j++)
		{
			for (k = nStartCol; k < nEndCol; k++)
				fltAvgDisparity += nl->neuron_vector[j * nl->dimentions.x + k].output.fval;
		}
		fltAvgDisparity /= ((nEndCol - nStartCol) * nl->dimentions.y);

		if (fltAvgDisparity < fltMinAvgDisparity)
		{
			fltMinAvgDisparity = fltAvgDisparity;
			nDisparity = p_list->param.ival;
		}

		printf("%s - fltAvg: %.4f\n", nl->name, fltAvgDisparity);
	}

	// Setar a saida
	filter_output->neuron_vector[0].output.ival = nDisparity;
//	printf("nDisparity: %d\n", nDisparity);
}

// ----------------------------------------------------------------------------
// shift_vergence_complex_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void shift_vergence_complex_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *filter_output = NULL;
	int nNumNL;
	float fltRangeCols;
	float fltAvgDisparity, fltMaxAvgDisparity, fltMaxDisparity, fltAux;
	int nStartCol, nEndCol;
	int nNumParam;
	int nDisparity;
	int i, j, k;

	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
// 	printf("nNumNL: %d\n", nNumNL);
	
	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;
//	printf("nNumParam: %d\n", nNumParam);
	
	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != (nNumParam - 1))
	{
		Erro ("Wrong number of parameters for shift_vergence_complex_filter.", "", "");
		return;
	}

	// Calcula o range de colunas para realizar a vergencia
	for (i = 0, p_list = filter_desc->filter_params; i < nNumParam; p_list = p_list->next, i++);
	fltRangeCols = p_list->param.fval / 100.0;	// Porcentagem
//	printf("fltRangeCols: %f\n", fltRangeCols);

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;

	// Inicializa com "infinito"
	fltMaxAvgDisparity = FLT_MIN;
	
	// Percorre todas as neuron layers procurando a de maior saida media
	// e associando a disparidade correspondente
	for (i = 0; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		fltMaxDisparity = FLT_MIN;
		nl = n_list->neuron_layer;

		nStartCol = nl->dimentions.x * ((1.0 - fltRangeCols) / 2.0);
		nEndCol = nl->dimentions.x - nStartCol;

		fltAvgDisparity = 0.0;
		for (j = 0; j < nl->dimentions.y; j++)
		{
			for (k = nStartCol; k < nEndCol; k++)
			{
				fltAux = nl->neuron_vector[j * nl->dimentions.x + k].output.fval;
				fltAvgDisparity += fltAux;
				if (fltAux > fltMaxDisparity)
					fltMaxDisparity = fltAux;
			}
		}
		fltAvgDisparity /= ((nEndCol - nStartCol) * nl->dimentions.y);

		if (fltAvgDisparity > fltMaxAvgDisparity)
		{
			fltMaxAvgDisparity = fltAvgDisparity;
			nDisparity = p_list->param.ival;
		}

		printf("%s - fltAvg: %.4f - fltMax: %.4f\n", nl->name, fltAvgDisparity, fltMaxDisparity);
	}

	// Setar a saida
	filter_output->neuron_vector[0].output.ival = nDisparity;
//	printf("nDisparity: %d\n", nDisparity);
}

// ----------------------------------------------------------------------------
// statistic_shift_vergence_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void statistic_shift_vergence_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *filter_output = NULL;
	int nNumNL;
	float fltRangeCols;
	float fltAvgDisparity;
	int nStartCol, nEndCol;
	int nDiffNeurons, nMinDiffNeurons;
	int nNumParam;
	int nDisparity;
	int i, j, k;

	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
	printf("nNumNL: %d\n", nNumNL);

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;
	printf("nNumParam: %d\n", nNumParam);

	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != (nNumParam - 1))
	{
		Erro ("Wrong number of parameters for shift_vergence_filter.", "", "");
		return;
	}

	// Calcula o range de colunas para realizar a vergencia
	for (i = 0, p_list = filter_desc->filter_params; i < nNumParam; p_list = p_list->next, i++);
	fltRangeCols = p_list->param.fval / 100.0;		// Porcentagem

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	n_list = filter_desc->neuron_layer_list;
	p_list = filter_desc->filter_params->next;

	// Inicializa com o maximo possivel
	nMinDiffNeurons = INT_MAX;
	
	// Percorre todas as neuron layers procurando a de menor saida media
	// e associando a disparidade correspondente
	for (i = 0; i < nNumNL; n_list = n_list->next, p_list = p_list->next, i++)
	{
		nl = n_list->neuron_layer;

		nStartCol = nl->dimentions.x * ((1.0 - fltRangeCols) / 2.0);
		nEndCol = nl->dimentions.x - nStartCol;

		nDiffNeurons = 0;
		fltAvgDisparity = 0.0;
		for (j = 0; j < nl->dimentions.y; j++)
		{
			for (k = nStartCol; k < nEndCol; k++)
			{
				fltAvgDisparity += nl->neuron_vector[j * nl->dimentions.x + k].output.fval;
				if (nl->neuron_vector[j * nl->dimentions.x + k].output.fval > 1.0)
					nDiffNeurons++;
			}
		}
		fltAvgDisparity /= ((nEndCol - nStartCol) * nl->dimentions.y);

		if (nDiffNeurons < nMinDiffNeurons)
		{
			nMinDiffNeurons = nDiffNeurons;
			nDisparity = p_list->param.ival;
		}

		printf("%s - fltAvg: %.4f - DiffNeurons: %d\n", nl->name, fltAvgDisparity, nDiffNeurons);
	}

	// Setar a saida
	filter_output->neuron_vector[0].output.ival = nDisparity;

	printf("nDisparity: %d\n", nDisparity);
}

// ----------------------------------------------------------------------------
// image_fusion_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void image_fusion_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	PARAM_LIST *p_list = NULL;
	static INPUT_DESC *input_right, *input_left;
	static NEURON_LAYER *nl_input_left, *nl_input_right, *nl_disparity_map;
	static NEURON_LAYER *filter_output = NULL;
	static int flagInit = 0;
	static int wo, ho, wi, hi, w, h;
	static float fltLogFactor;
	static float hAux;
	int nNumNL, nNumParam;
	int u, v, xi, yi, xi_left, yi_left, x_center, y_center, x_center_left, y_center_left;
	int nRed, nGreen, nBlue;
	int nPixelLeft, nPixelRight;
//	int nDispXVergOffset;
	float fltDisparity;
	
	if (!flagInit)
	{
		// Achar o numero de neuron layers
		for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);
	
		// Numero de neuron layers deve ser igual a tres
		if (nNumNL != 3)
		{
			Erro ("The image_fusion_filter must have three neuron layers: image_left, image_right and disparity_map.", "", "");
			return;
		}
		nl_input_left = filter_desc->neuron_layer_list->neuron_layer;
		nl_input_right = filter_desc->neuron_layer_list->next->neuron_layer;
		nl_disparity_map = filter_desc->neuron_layer_list->next->next->neuron_layer;

		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;

		// Numero de neuron layers deve ser igual a tres
		if (nNumParam != 1)
		{
			Erro ("The image_fusion_filter must have one parameter: log_factor.", "", "");
			return;
		}
		fltLogFactor = filter_desc->filter_params->next->param.fval;
		printf("fltLogFactor: %f\n", fltLogFactor);

		// Ponteiro para a 'output' associada ao filtro
		filter_output = filter_desc->output;

		// Dimensao da 'output' associada ao filtro
		wo = filter_output->dimentions.x;
		ho = filter_output->dimentions.y;

		// Dimensao das neuron layers de entrada
		wi = nl_input_right->dimentions.x;
		hi = nl_input_right->dimentions.y;

		// Dimensao do mapa de disparidade
		w = nl_disparity_map->dimentions.x;
		h = nl_disparity_map->dimentions.y;
		hAux = (float) h / (h - 1);

		input_right = get_input_by_neural_layer (nl_input_right);
		input_left = get_input_by_neural_layer (nl_input_left);

		flagInit = 1;
	}

	// NAO SEI PORQUE.
	// PRECISA DISTO PARA FUNCIONAR DEVIDO A ALTERACAO FEITA POR DIJALMA
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = input_right->wxd - input_right->wx;
		y_center = input_right->wyd - input_right->wy;
		x_center_left = input_left->wxd - input_left->wx;
		y_center_left = input_left->wyd - input_left->wy;
	}
	else
	{
		x_center = wi/2;
		y_center = hi/2;
		x_center_left = wi/2;
		y_center_left = hi/2;
	}

	// Zera a saida
	for (u = 0; u < (wo * ho); u++)
		filter_output->neuron_vector[u].output.ival = 0;

	// Percorre a disparity_map
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			// Mapeamento logpolar->plano
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, hAux, fltLogFactor);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			switch (nl_disparity_map->output_type)
			{
				case GREYSCALE:
				{
					fltDisparity = (float) nl_disparity_map->neuron_vector[(v * w) + u].output.ival;
				}
				break;
				
				case GREYSCALE_FLOAT:
				{
					fltDisparity = nl_disparity_map->neuron_vector[(v * w) + u].output.fval;
				}
				break;
			}

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi_left, &yi_left, wi, hi, u, v, w, h, x_center_left, y_center_left, hAux, fltLogFactor);
			xi_left += (int) fltDisparity;

			if (xi_left >= wi || xi_left < 0 || yi_left >= hi || yi_left < 0)
				continue;

			nPixelRight = nl_input_right->neuron_vector[(yi * wi) + xi].output.ival;
			nPixelLeft = nl_input_left->neuron_vector[(yi_left * wi) + xi_left].output.ival;

			nRed = (RED(nPixelLeft) + RED(nPixelRight)) / 2;
			nGreen = (GREEN(nPixelLeft) + GREEN(nPixelRight)) / 2;
			nBlue = (BLUE(nPixelLeft) + BLUE(nPixelRight)) / 2;

			filter_output->neuron_vector[(yi * wo) + xi].output.ival = PIXEL(nRed, nGreen, nBlue);
		}
	}
}

// ----------------------------------------------------------------------------
// min_association_v1_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void min_association_v1_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	NEURON_LAYER *filter_output;
	INPUT_DESC *input_left;
	int nNumNL, wo, ho, i, j, x_left;
	float fltAux;
	TStructMinAssocV1Filter	*aux;
	
	// Achar o numero de neuron layers
	for (nNumNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nNumNL++);

	// Numero de neuron layers deve ser igual ao numero de parametros
	if (nNumNL != 3)
	{
		Erro ("The min_association_v1_filter must be aplied on three neuron layers: image_rigth, image_left, minus_v1 respectivelly.", "", "");
		return;
	}

	// Ponteiro para a 'output' associada ao filtro
	filter_output = filter_desc->output;

	// Dimensao da 'output' associada ao filtro
	wo = filter_output->dimentions.x;
	ho = filter_output->dimentions.y;
	
	// Alocar espaco auxiliar
	if (filter_desc->private_state == NULL)
	{
		filter_desc->private_state = (void*)malloc(sizeof(TStructMinAssocV1Filter)*wo*ho);

		aux = (TStructMinAssocV1Filter*) filter_desc->private_state;

		// Inicializa a estrutura
		for (i = 0; i < wo * ho; i++)
		{
			aux[i].value = FLT_MAX;
			aux[i].x = -1;
		}
	}
	else
		aux = (TStructMinAssocV1Filter*) filter_desc->private_state;

	// Busca o valor da coordenada x da imagem esquerda (segunda neuron layer de entrada do filtro)
	input_left = get_input_by_neural_layer (filter_desc->neuron_layer_list->next->neuron_layer);
	x_left = input_left->wxd;
//	printf("x_left: %d\n", x_left);

	// Busca a neuron layer "minus_v1" (terceira neuron layer de entrada do filtro)
	nl = filter_desc->neuron_layer_list->next->next->neuron_layer;

	// Verifica se os valores de "minus_v1" sao menores que os valores do filtro
	// Caso sejam, armazena estes valores
	for (i = 0; i < wo*ho; i++)
	{
		if (nl->neuron_vector[i].output.fval < aux[i].value)
		{
			aux[i].value = nl->neuron_vector[i].output.fval;
			aux[i].x = x_left;
		}
	}
}

/*!
*********************************************************************************
* Function: robot_minus_filter
* Description: Computes de vector difference between the n input neuron layers.
* dif = sqrt(SUM((nl[i] - nl[i + (n / 2)])^2))
* Inputs:
* Output:
*********************************************************************************
*/

void gabor_minus_filter (FILTER_DESC *filter_desc)
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

		nl = (NEURON_LAYER**)malloc(sizeof(NEURON_LAYER*)*numNL);
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
					if ((nl[j]->neuron_vector[i].output.ival != 0) && (nl[j + (numNL / 2)]->neuron_vector[i].output.ival != 0))
					{
						dif = nl[j]->neuron_vector[i].output.ival - nl[j + (numNL / 2)]->neuron_vector[i].output.ival;
						dif_accumulator += (float)(dif*dif);
					}
				}
				filter_desc->output->neuron_vector[i].output.ival = (int)sqrtf(dif_accumulator);
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
					if ((nl[j]->neuron_vector[i].output.fval != 0.0) && (nl[j + (numNL / 2)]->neuron_vector[i].output.fval != 0.0))
						dif_float = nl[j]->neuron_vector[i].output.fval - nl[j + (numNL / 2)]->neuron_vector[i].output.fval;
					else
						dif_float = 0.001;

					dif_accumulator += (dif_float*dif_float);
				}
				filter_desc->output->neuron_vector[i].output.fval = sqrtf(dif_accumulator);
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
				filter_desc->output->neuron_vector[i].output.ival = (int)sqrtf(dif_accumulator);
			}
		}
		break;
	}
}
