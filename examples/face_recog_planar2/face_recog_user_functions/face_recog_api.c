#include "face_recog_api.h"

// Global variables
FILE *g_pLogFile = NULL;
NEURON_LAYER_OUTPUT_INT *g_neuron_layer_output_int = NULL;
NEURON_LAYER_OUTPUT_FLOAT *g_neuron_layer_output_float = NULL;


/*! 
********************************************************************************
* \brief Initializes the MAE framework.
* \pre None.
* \post The MAE initialized.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeInitialize (void)
{
	init_pattern_xor_table ();
	init_measuraments ();
	build_network ();
	init_user_functions ();

	g_pLogFile = fopen (EXECUTION_LOG_FILE_NAME, "w");
	fprintf (g_pLogFile, "MaeInitialize\n");
	fclose (g_pLogFile);

    return (0);
}



/*! 
********************************************************************************
* \brief Exits the MAE framework.
* \pre The MAE environment initialized.
* \post The MAE finalized.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT int MaeQuit (void)
{
	g_pLogFile = fopen (EXECUTION_LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "MaeQuit\n");
	fclose (g_pLogFile);

	return (0);
}



/*! 
*********************************************************************************
* \brief Updates the image input.
* \param[in] p_nNewImage The new image pixel vector.
* \param[in] nImageWidth The image width.
* \param[in] nImageHeight The image height.
* \pre The stereo module initialized.
* \post The input image updated.
* \return 0 if OK, -1 otherwise.
*********************************************************************************
*/

DLLEXPORT int MaeUpdateImage (int *p_nNewImage, int nImageWidth, int nImageHeight)
{
	int i, j, nPixel;
        
	// Cheks the image dimentions
	if ((nImageWidth != face_recog.ww) || (nImageHeight != face_recog.wh))
	{
		ERROR ("Incompatible image dimentions (MaeUpdateImage).", "", "");
		return (-1);
	}

	// Updates the input image
	for (j = 0; j < nImageHeight; j++)
	{
        	for (i = 0; i < nImageWidth; i++)
        	{
		      nPixel = p_nNewImage[i + (nImageHeight - j - 1) * nImageWidth];

		      face_recog.image[3 * (i + j * face_recog.tfw) + 0] = (GLubyte) RED(nPixel);
		      face_recog.image[3 * (i + j * face_recog.tfw) + 1] = (GLubyte) GREEN(nPixel);
		      face_recog.image[3 * (i + j * face_recog.tfw) + 2] = (GLubyte) BLUE(nPixel);
        	}
	}

	// Updates the input neurons
	face_recog.up2date = 0;
	update_input_neurons (&face_recog);
	
	/*g_pLogFile = fopen (EXECUTION_LOG_FILE_NAME, "a");
	fprintf (g_pLogFile, "MaeUpdateImage \n");
	fclose (g_pLogFile);

	{
		FILE *pImageFile = fopen (face_recog.name, "w");
		fprintf (pImageFile, "P3\n");
		fprintf (pImageFile, "# Test\n");
		fprintf (pImageFile, "%d %d\n", nImageWidth, nImageHeight);
		fprintf (pImageFile, "255\n");

		for (j = nImageHeight; j >= 0 ; j--)
        		for (i = 0; i < nImageWidth; i++)
       			      fprintf (pImageFile, "%d %d %d\n", face_recog.image[3 * (i + j * face_recog.tfw) + 0],
								 face_recog.image[3 * (i + j * face_recog.tfw) + 1],
								 face_recog.mage[3 * (i + j * face_recog.tfw) + 2]);
		fclose (pImageFile);
	}*/
	return (0);
}



int *get_neuron_layer_output_int_pointer (NEURON_LAYER *neuron_layer)
{
	if (g_neuron_layer_output_int == NULL)
	{
		g_neuron_layer_output_int = (NEURON_LAYER_OUTPUT_INT *) malloc (sizeof(NEURON_LAYER_OUTPUT_INT));
		g_neuron_layer_output_int->neuron_layer = neuron_layer;
		g_neuron_layer_output_int->output_int = (int *) malloc (neuron_layer->dimentions.x * neuron_layer->dimentions.y * sizeof(int));
		g_neuron_layer_output_int->next = NULL;
		return (g_neuron_layer_output_int->output_int);
	}
	else
	{
		NEURON_LAYER_OUTPUT_INT *neuron_layer_output_int_list;
		
		for (neuron_layer_output_int_list = g_neuron_layer_output_int; 
		     neuron_layer_output_int_list->next != NULL; 
		     neuron_layer_output_int_list = neuron_layer_output_int_list->next)
		{
			if (neuron_layer_output_int_list->neuron_layer == neuron_layer)
				return (neuron_layer_output_int_list->output_int);
		}
		neuron_layer_output_int_list->next = (NEURON_LAYER_OUTPUT_INT *) malloc (sizeof(NEURON_LAYER_OUTPUT_INT));
		neuron_layer_output_int_list->next->neuron_layer = neuron_layer;
		neuron_layer_output_int_list->next->output_int = (int *) malloc (neuron_layer->dimentions.x * neuron_layer->dimentions.y * sizeof(int));
		neuron_layer_output_int_list->next->next = NULL;
		return (neuron_layer_output_int_list->next->output_int);
	}
}



float *get_neuron_layer_output_float_pointer (NEURON_LAYER *neuron_layer)
{
	if (g_neuron_layer_output_float == NULL)
	{
		g_neuron_layer_output_float = (NEURON_LAYER_OUTPUT_FLOAT *) malloc (sizeof(NEURON_LAYER_OUTPUT_FLOAT));
		g_neuron_layer_output_float->neuron_layer = neuron_layer;
		g_neuron_layer_output_float->output_float = (float *) malloc (neuron_layer->dimentions.x * neuron_layer->dimentions.y * sizeof(float));
		g_neuron_layer_output_float->next = NULL;
		return (g_neuron_layer_output_float->output_float);
	}
	else
	{
		NEURON_LAYER_OUTPUT_FLOAT *neuron_layer_output_float_list;
		
		for (neuron_layer_output_float_list = g_neuron_layer_output_float; 
		     neuron_layer_output_float_list->next != NULL; 
		     neuron_layer_output_float_list = neuron_layer_output_float_list->next)
		{
			if (neuron_layer_output_float_list->neuron_layer == neuron_layer)
				return (neuron_layer_output_float_list->output_float);
		}
		neuron_layer_output_float_list->next = (NEURON_LAYER_OUTPUT_FLOAT *) malloc (sizeof(NEURON_LAYER_OUTPUT_FLOAT));
		neuron_layer_output_float_list->next->neuron_layer = neuron_layer;
		neuron_layer_output_float_list->next->output_float = (float *) malloc (neuron_layer->dimentions.x * neuron_layer->dimentions.y * sizeof(float));
		neuron_layer_output_float_list->next->next = NULL;
		return (neuron_layer_output_float_list->next->output_float);
	}
}



/*! 
*********************************************************************************
* \brief Get neural layer output - int type.
* \param[in] neuron_layer_name The name of the neural layer.
* \pre The MAE initialized.
* \return int vector with the neural output.
*********************************************************************************
*/

DLLEXPORT int *GetNeuronLayerOutputInt (char *neuron_layer_name)
{
	int w, h, x, y, i;
        int *neuron_layer_output_int;
        NEURON_LAYER *neuron_layer;
        NEURON *neuron_vector;
        
        neuron_layer = get_neuron_layer_by_name (neuron_layer_name);
        if (neuron_layer == NULL)
        	return NULL;

		neuron_layer_output_int = get_neuron_layer_output_int_pointer (neuron_layer);
        if (neuron_layer_output_int == NULL)
        	return NULL;
        
        w = neuron_layer->dimentions.x;
        h = neuron_layer->dimentions.y;
        neuron_vector = neuron_layer->neuron_vector;
        for (y = 0; y < h; y++)
        {
		for (x = 0; x < w; x++)
		{
			i = y * w + x;
			neuron_layer_output_int[i] = neuron_vector[i].output.ival;
		}
	}

	return (neuron_layer_output_int);
}



/*! 
*********************************************************************************
* \brief Get neural layer output - float type.
* \param[in] neuron_layer_name The name of the neural layer.
* \pre The MAE initialized.
* \return float vector with the neural output.
*********************************************************************************
*/

DLLEXPORT int *GetNeuronLayerOutputFloat (char *neuron_layer_name)
{
		int w, h, x, y, i;
        float *neuron_layer_output_float;
        NEURON_LAYER *neuron_layer;
        NEURON *neuron_vector;
        
        neuron_layer = get_neuron_layer_by_name (neuron_layer_name);
        if (neuron_layer == NULL)
        	return NULL;

		neuron_layer_output_float = get_neuron_layer_output_float_pointer (neuron_layer);
        if (neuron_layer_output_float == NULL)
        	return NULL;
        
        w = neuron_layer->dimentions.x;
        h = neuron_layer->dimentions.y;
        neuron_vector = neuron_layer->neuron_vector;
        for (y = 0; y < h; y++)
        {
		for (x = 0; x < w; x++)
		{
			i = y * w + x;
			neuron_layer_output_float[i] = neuron_vector[i].output.fval;
		}
	}
	
	/* Por que esta funcao retorna inteiro se o ponteiro ee para um vetor de float ? */
	return ((int *) neuron_layer_output_float);
}



/*! 
*********************************************************************************
* \brief Put neural layer output - int type.
* \param[in] neuron_layer_name The name of the neural layer.
* \pre The MAE initialized.
* \return 1 if OK, 0 otherwise.
*********************************************************************************
*/

DLLEXPORT int PutNeuronLayerOutputInt (char *neuron_layer_name, int *neural_layer_output_int)
{
	int w, h, x, y, i;
        NEURON_LAYER *neuron_layer;
        NEURON *neuron_vector;
        
        neuron_layer = get_neuron_layer_by_name (neuron_layer_name);
        if (neuron_layer == NULL)
        	return 0;
        
        w = neuron_layer->dimentions.x;
        h = neuron_layer->dimentions.y;
        neuron_vector = neuron_layer->neuron_vector;
        for (y = 0; y < h; y++)
        {
		for (x = 0; x < w; x++)
		{
			i = y * w + x;
			neuron_vector[i].output.ival = neural_layer_output_int[i];
		}
	}

	return 1;
}



/*! 
*********************************************************************************
* \brief Get neural layer width.
* \param[in] neuron_layer_name The name of the neural layer.
* \pre The MAE initialized.
* \return neural layer width.
*********************************************************************************
*/

DLLEXPORT int GetNeuronLayerWidth (char *neuron_layer_name)
{
        NEURON_LAYER *neuron_layer;
        
        neuron_layer = get_neuron_layer_by_name (neuron_layer_name);
        if (neuron_layer == NULL)
        	return 0;

        return (neuron_layer->dimentions.x);
}



/*! 
*********************************************************************************
* \brief Get neural layer hight.
* \param[in] neuron_layer_name The name of the neural layer.
* \pre The MAE initialized.
* \return neural layer hight.
*********************************************************************************
*/

DLLEXPORT int GetNeuronLayerHight (char *neuron_layer_name)
{
        NEURON_LAYER *neuron_layer;
        
        neuron_layer = get_neuron_layer_by_name (neuron_layer_name);
        if (neuron_layer == NULL)
        	return 0;

        return (neuron_layer->dimentions.y);
}



/*! 
********************************************************************************
* \brief Initializes the MAE framework.
* \pre None.
* \post The MAE initialized.
* \return 0 if OK, -1 otherwise.
********************************************************************************
*/

DLLEXPORT void MaeExecuteCommandScript (char *script)
{
    interpreter (script);
}