#include <math.h>
#include "mae.h"

#define DRAWRECTANGLE(x, y, w, h) \
    glBegin(GL_LINES); glVertex2f ((x), (y)); glVertex2f ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2f ((x)+(w), (y)); glVertex2f ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2f ((x)+(w), (y)+(h)); glVertex2f ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2f ((x), (y)+(h)); glVertex2f ((x), (y)); glEnd ()

#define	CROSS_UP_SIZE(x, w) (((x + w/10) < w)? x + w/10: w-1)
#define	CROSS_DOWN_SIZE(x, w) (((x - w/10) >= 0)? x - w/10: 0)

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); \
    		       glVertex2i ((x+w/50), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); \
    		       glVertex2i ((x), (y+h/50)); \
    glEnd (); \
}
	
#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); \
    		       glVertex2i ((x), (h+h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); \
    		       glVertex2i ((x), (-h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); \
    		       glVertex2i ((w+h/20), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); \
    		       glVertex2i ((-h/20), (y)); \
    glEnd (); \
}

#define DRAW_SQUARE_3f(x_min, y_min, x_max, y_max) \
	glBegin (GL_LINES); \
		glVertex3f (x_min, y_min, 0.0); glVertex3f (x_min, y_max, 0.0); \
		glVertex3f (x_min, y_min, 0.0); glVertex3f (x_max, y_min, 0.0); \
		glVertex3f (x_max, y_max, 0.0); glVertex3f (x_min, y_max, 0.0); \
		glVertex3f (x_max, y_max, 0.0); glVertex3f (x_max, y_min, 0.0); \
	glEnd (); 
	
#define DRAW_XY_AXIS(x_min, y_min, x_max, y_max) \
	glEnable (GL_LINE_STIPPLE); \
	glLineStipple (1, 0x00ff); \
	glBegin (GL_LINES); \
		glVertex3f (x_min, 0.0, 0.0); glVertex3f (x_max, 0.0, 0.0); \
		glVertex3f (0.0, y_min, 0.0); glVertex3f (0.0, y_max, 0.0); \
	glEnd (); \
	glDisable (GL_LINE_STIPPLE); 

#define DRAW_STRING(string) { 	int i; \
				for (i = 0; i < strlen (string); i++) \
					glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, string[i]); }

#define DRAW_XY_TICS(x_min, x_max, y_min, y_max, n_tics) { \
	double delta_x, delta_y; \
	delta_x = (x_max - x_min)/ (double) (n_tics - 1); \
	delta_y = (y_max - y_min)/ (double) (n_tics - 1); \
	for (i = 0, x = x_min, y = y_min; i < n_tics; i++, x += delta_x, y += delta_y) { \
		glRasterPos2f (x, y_min - 0.4*delta_y); \
		sprintf (info, "%.1f\r", x); \
		DRAW_STRING(info); \
		glBegin (GL_LINES); \
			glVertex3f (x, y_min, 0.0); glVertex3f (x, y_min - 0.2*delta_y, 0.0); \
		glEnd (); \
		glRasterPos2f (x_min - 0.8*delta_x, y); \
		sprintf (info, "%.2f\r", y); \
		DRAW_STRING(info); \
		glBegin (GL_LINES); \
			glVertex3f (x_min, y, 0.0); glVertex3f (x_min - 0.2*delta_x, y, 0.0); \
		glEnd (); } }
		
#ifndef PI
#define PI 3.1415926535
#endif


GLenum errCode;
const GLubyte *errString;



int 
nearest_power_of_2 (int value)
{
	int i;
	int larger = 0;
	
	for (i = 0; i < 32; i++)
	{
		if ((value >> i) & 1)
			larger = i;
	}
	i = 1 << larger;
	if ((value % i) != 0)
		i = i << 1;

	return (i);
}



/*! 
*********************************************************************************
* \brief Get neuron output from the RGB image matrix.
* \param[in] image The RGB image matrix.
* \param[in] wx The horizontal offset.
* \param[in] wy The vertical offset.
* \param[in] vpw The visual part width of the image.
* \param[in] vph The visual part height of the image.
* \param[in] tfw The texture frame width.
* \param[in] tfh The texture frame height.
* \param[in] xi The pixel X coordinate.
* \param[in] yi The pixel Y coordinate.
* \param[in] output_type The neuron output type (COLOR, GREYSCALE, GREYSCALE_FLOAT, BLACK_WHITE).
* \param[in] pyramid_type The image pyramid type (REGULAR_PYRAMID, GAUSSIAN_PYRAMID, LAPLACIAN_PYRAMID).
* \param[in] pyramid_level The image pyramid level (0-N).
* \pre The image matrix created.
* \post The neuron output sampled.
* \return The neuron output union.
*********************************************************************************
*/

NEURON_OUTPUT 
get_neuron_output_from_image (GLubyte *image, int wx, int wy, int vpw, int vph, int tfw, int tfh, int xi, int yi, int output_type, int pyramid_type, int pyramid_level)
{
        int nRed, nGreen, nBlue, x, y, xo, yo, nRadius, num_points = 0, nSize;
	float fltRed, fltGreen, fltBlue, fltWeight, fltWeightSum, k;
        NEURON_OUTPUT neuron_output;
        neuron_output.ival = 0;
        
/*        for (i = 0, nSize = 1; i < pyramid_level; i++, nSize << 1)
                ;
*/
	nSize = 1;
	                
        // Image sampling
        switch (pyramid_type)
        {
                case REGULAR_PYRAMID:
                        nRed = nGreen = nBlue = 0;
			nRadius = nSize >> 1;
                        for (y = -nRadius; y <= nRadius; y++)
                        {
                                for (x = -nRadius; x <= nRadius; x++)
                                {
					xo = wx + xi + x;
					yo = wy + yi + y;

                                        if ((xo >= 0) && (xo < vpw) && (yo >= 0) && (yo < vph))
                                        {
                                                nRed   += image[3 * (yo * tfw + xo) + 0];
                                                nGreen += image[3 * (yo * tfw + xo) + 1];
                                                nBlue  += image[3 * (yo * tfw + xo) + 2];
                                                num_points++;
                                        }
                                }
                        }
                        if (num_points > 0)
                        {
                                nRed   /= num_points;
                                nGreen /= num_points;
                                nBlue  /= num_points;
                        }
                        break;
                case GAUSSIAN_PYRAMID:
                        nRed = nGreen = nBlue = 0;
			fltWeightSum = .0f;
			fltRed = fltGreen = fltBlue = .0f;
			nRadius = nSize;
			k = 1.0f / 2.0f * (float) (nRadius * nRadius);
                        for (y = -3 * nRadius; y <= 3 * nRadius; y++)
                        {
                                for (x = -3 * nRadius; x <= 3 * nRadius; x++)
                                {
					xo = wx + xi + x;
					yo = wy + yi + y;

                                        if ((xo >= 0) && (xo < vpw) && (yo >= 0) && (yo < vph))
                                        {
						fltWeightSum += fltWeight = exp (-k * (x * x + y * y));
                                                fltRed   += fltWeight * (float) image[3 * (yo * tfw + xo) + 0];
                                                fltGreen += fltWeight * (float) image[3 * (yo * tfw + xo) + 1];
                                                fltBlue  += fltWeight * (float) image[3 * (yo * tfw + xo) + 2];
                                        }
                                }
                        }
                        if (fltWeightSum > .0f)
                        {
                                nRed   = (int) (fltRed   / fltWeightSum + .5f);
                                nGreen = (int) (fltGreen / fltWeightSum + .5f);
                                nBlue  = (int) (fltBlue  / fltWeightSum + .5f);
                        }
                        break;
                case LAPLACIAN_PYRAMID:
                        nRed = nGreen = nBlue = 0;
                        break;
                default:
                        if (((wx + xi) >= 0) && ((wx + xi) < vpw) && ((wy + yi) >= 0) && ((wy + yi) < vph))
                        {
                                nRed   = image[3 * ((yi + wy) * tfw + xi + wx) + 0];
                                nGreen = image[3 * ((yi + wy) * tfw + xi + wx) + 1];
                                nBlue  = image[3 * ((yi + wy) * tfw + xi + wx) + 2];
                        }
                        else
                                nRed = nGreen = nBlue = 0;
        }
       			
        switch (output_type)
        {
                case COLOR:
                        neuron_output.ival = PIXEL(nRed, nGreen, nBlue); 
                        break;
                case GREYSCALE:
                        neuron_output.ival = (nRed + nGreen + nBlue) / 3; 
                        break;
                case BLACK_WHITE:
                        neuron_output.ival = ((nRed + nGreen + nBlue) / 3 > (GLubyte) NUM_GREYLEVELS / 2) ? (GLubyte) (NUM_GREYLEVELS - 1) : 0;
                        break;
                case GREYSCALE_FLOAT:
                        neuron_output.fval = (float) (nRed + nGreen + nBlue) / 3.0f; 
                        break;
                default:
                        Erro("Unexpected neuron output type.", "", "");
        }        
        
        return (neuron_output);
}


#ifndef NO_INTERFACE
void 
input_init (INPUT_DESC *input)
{    
	glClearColor (0, 0, 0, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, 
		input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glShadeModel(GL_FLAT);
	
	if ((errCode = glGetError()) != GL_NO_ERROR) 
	{
		errString = gluErrorString(errCode);
		fprintf (stderr, "OpenGL Error: %s\n", errString);
	}
	
	return;
}



void 
output_init (OUTPUT_DESC *output)
{    
	glClearColor (0, 0, 0, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture (GL_TEXTURE_2D, (GLuint) output->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output->tfw, 
		output->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, output->image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glShadeModel(GL_FLAT);
	
	return;
}
#endif


/*! 
*********************************************************************************
* \brief Update the input neurons.
* \param[in] input The pointer to the input structure.		
* \pre The input structure created and initialized.
* \post The neuron vector updated.
* \return Nothing.
*********************************************************************************
*/

void 
update_input_neurons (INPUT_DESC *input)
{
	int xi, yi, xo, yo, wo, ho, w, h, wx, wy, vpw, vph, tfw, tfh, output_type, pyramid_level, pyramid_level_offset, pyramid_type;
	float x_factor, y_factor;
        GLubyte *image;
	NEURON	*neuron_vector;
	
	// Tests if the input neuron layer is up to date
	//if (input->up2date)
	//	return;
	//input->up2date++;
	
        wx = input->wx;
        wy = input->wy;
        vpw = input->vpw;
        vph = input->vph;
        tfw = input->tfw;
        tfh = input->tfh;
        image = input->image;
	neuron_vector = input->neuron_layer->neuron_vector;
        output_type = input->neuron_layer->output_type;
        wo = w = input->neuron_layer->dimentions.x;
        ho = h = input->neuron_layer->dimentions.y;
        pyramid_type = input->pyramid_type;
        
        // Updates each input pyramid level
        for (pyramid_level = 0, pyramid_level_offset = 0; pyramid_level <= input->pyramid_height; pyramid_level_offset += wo * ho, wo = wo >> 1, ho = ho >> 1, pyramid_level++)
        {
                x_factor = (float) w / (float) wo;
                y_factor = (float) h / (float) ho;
                
                // Full fills an input pyramid level
                for (yo = yi = 0; yo < ho; yo++, yi = (int) (y_factor * (float) yo + .5f))
                        for (xo = xi = 0; xo < wo; xo++, xi = (int) (x_factor * (float) xo + .5f))
				neuron_vector[pyramid_level_offset + yo * wo + xo].output = get_neuron_output_from_image (image, wx, wy, vpw, vph, tfw, tfh, xi, yi, output_type, pyramid_type, pyramid_level);
        }
}


/*! 
*********************************************************************************
* \brief Update the input neurons.
* \param[in] input The pointer to the input structure.		
* \pre The input structure created and initialized.
* \post The neuron vector updated.
* \return Nothing.
*********************************************************************************
*/

void 
update_output_neurons (OUTPUT_DESC *output)
{
	int xi, yi, xo, yo, wo, ho, w, h, tfw, tfh, output_type;
        GLubyte *image;
	NEURON *neuron_vector;
		
        tfw = output->tfw;
        tfh = output->tfh;
        image = output->image;
	neuron_vector = output->neuron_layer->neuron_vector;
        
        output_type = output->neuron_layer->output_type;
        wo = w = output->neuron_layer->dimentions.x;
        ho = h = output->neuron_layer->dimentions.y;
        
        for (yo = yi = 0; yo < ho; yo++, yi = yo)
                for (xo = xi = 0; xo < wo; xo++, xi = xo)
			neuron_vector[yo * wo + xo].output = get_neuron_output_from_image (image, 0, 0, 0, 0, tfw, tfh, xi, yi, output_type, LAPLACIAN_PYRAMID, 0);
                        
}



/*! 
*********************************************************************************
* \brief Loads the input image with PNM P2 format.
* \param[in] input The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The input initialized and the file openned.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image_with_p2_format (INPUT_DESC *input, FILE *image_file)
{
	int i, j;
	int intensity;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_input_image_with_p2_format");
					
					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
			break;
		case BLACK_WHITE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_input_image_with_p2_format");

					intensity = (intensity > 128) ? 255: 0;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
			break;
		default:
			for (i = input->vph - 1; i >= 0; i--) 
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_input_image_with_p2_format");

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with PNM-P2 format into a specified neuron layer
* \param[in] neuron_layer The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The neuron layer initialized and the file openned.
* \post The image loaded into the neuron layer.
* \return Nothing.
*********************************************************************************
*/

void 
load_neuron_layer_with_p2_format (NEURON_LAYER *neuron_layer, FILE *image_file)
{
	int w, h;
	int i, j;
	int intensity;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	switch(neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = h - 1; i >= 0; i--)  
			{
				for (j = 0; j < w; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_neuron_layer_with_p2_format");
					
					neuron_layer->neuron_vector[3 * (i * w + j) + 0].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 1].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 2].output.ival = (GLubyte) intensity;
				}
			}
			break;
		case BLACK_WHITE:
			for (i = h - 1; i >= 0; i--)  
			{
				for (j = 0; j < w; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_neuron_layer_with_p2_format");

					intensity = (intensity > 128) ? 255: 0;

					neuron_layer->neuron_vector[3 * (i * w + j) + 0].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 1].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 2].output.ival = (GLubyte) intensity;
				}
			}
			break;
		default:
			for (i = h - 1; i >= 0; i--) 
			{
				for (j = 0; j < w; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d", &(intensity)),"load_neuron_layer_with_p2_format");

					neuron_layer->neuron_vector[3 * (i * w + j) + 0].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 1].output.ival = (GLubyte) intensity;
					neuron_layer->neuron_vector[3 * (i * w + j) + 2].output.ival = (GLubyte) intensity;
				}
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with PNM P5 format.
* \param[in] input The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The input initialized and the file openned.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image_with_p5_format (INPUT_DESC *input, FILE *image_file)
{
	int i, j;
	unsigned char intensity;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%c", &(intensity)),"load_input_image_with_p5_format");
					
					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
			break;
		case BLACK_WHITE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%c", &(intensity)),"load_input_image_with_p5_format");

					intensity = (intensity > 128) ? 255: 0;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
			break;
		default:
			for (i = input->vph - 1; i >= 0; i--) 
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%c", &(intensity)),"load_input_image_with_p5_format");

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) intensity;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) intensity;
				}
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with PNM P3 format.
* \param[in] input The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The input initialized and the file openned.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image_with_p3_format (INPUT_DESC *input, FILE *image_file)
{
	int i, j;
	int r, g, b;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d%d%d", &(r), &(g), &(b)),"load_input_image_with_p3_format");
					
					r = g = b = (r + g + b) / 3;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
			break;
		case BLACK_WHITE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d%d%d", &(r), &(g), &(b)),"load_input_image_with_p3_format");

					r = g = b = (((r + g + b) / 3) > 128) ? 255: 0;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
			break;
		default:
			for (i = input->vph - 1; i >= 0; i--) 
			{
				for (j = 0; j < input->vpw; j++) 
				{
					MAE_FSCANF_WRAPPER(fscanf (image_file, "%d%d%d", &(r), &(g), &(b)),"load_input_image_with_p3_format");

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with PNM P6 format.
* \param[in] input The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The input initialized and the file openned.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image_with_p6_format(INPUT_DESC *input, FILE *image_file)
{
	int i, j;
	int r, g, b;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					r = fgetc (image_file);
					g = fgetc (image_file);
					b = fgetc (image_file);
					
					r = g = b = (r + g + b) / 3;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
			break;
		case BLACK_WHITE:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					r = fgetc (image_file);
					g = fgetc (image_file);
					b = fgetc (image_file);
				
					r = g = b = (((r + g + b) / 3) > 128) ? 255: 0;

					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
			break;
		default:
			for (i = input->vph - 1; i >= 0; i--)  
			{
				for (j = 0; j < input->vpw; j++) 
				{
					r = fgetc (image_file);
					g = fgetc (image_file);
					b = fgetc (image_file);
					
					input->image[3 * (i * input->tfw + j) + 0] = (GLubyte) r;
					input->image[3 * (i * input->tfw + j) + 1] = (GLubyte) g;
					input->image[3 * (i * input->tfw + j) + 2] = (GLubyte) b;
				}
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with BMP format.
* \param[in] input The input descriptor structure.
* \param[in] image_file A pointer to the image file.
* \pre The input initialized and the file openned.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image_with_BMP_format (INPUT_DESC *input, FILE *image_file)
{
	int i, j, k;
	int r, g, b;
	
	switch(input->neuron_layer->output_type)
	{
		case GREYSCALE:
		case BLACK_WHITE:
		default:
			for (j = 0; j < input->vph; j++) 
			{
				for (i = 0; i < input->vpw; i++) 
				{
					r = (int) fgetc (image_file);
					g = (int) fgetc (image_file);
					b = (int) fgetc (image_file);
									
					input->image[3 * (j * input->tfw + i) + 0] = (GLubyte) b;
					input->image[3 * (j * input->tfw + i) + 1] = (GLubyte) g;
					input->image[3 * (j * input->tfw + i) + 2] = (GLubyte) r;
				}

				// Each scan line needs have size multiple of 4.
				// otherwise discards until complete the size.
				for (k = (input->vpw % 4); k > 0; k--)
					fgetc (image_file);
			}
	}
}



/*! 
*********************************************************************************
* \brief Loads the input image with BMP format into a specified neuron layer
* \param[in] neuron_layer The input descriptor structure.
* \param[in] image_file A pointer to the image file (bitmapped).
* \pre The neuron layer initialized and the file openned.
* \post The image loaded into the neuron layer.
* \return Nothing.
*********************************************************************************
*/

void 
load_neuron_layer_with_BMP_format (NEURON_LAYER *neuron_layer, FILE *image_file)
{
	int w,h;
	int i, j, k;
	int r, g, b;
	
	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	switch(neuron_layer->output_type)
	{
		case GREYSCALE:
		case BLACK_WHITE:
		default:
			for (j = 0; j < h; j++) 
			{
				for (i = 0; i < w; i++) 
				{
					r = (int) fgetc (image_file);
					g = (int) fgetc (image_file);
					b = (int) fgetc (image_file);
					
					//neuron_layer->neuron_vector[i].output.ival // setting the output value of the neuron layer
					neuron_layer->neuron_vector[3 * (j * w + i) + 0].output.ival = (GLubyte) b;
					neuron_layer->neuron_vector[3 * (j * w + i) + 1].output.ival = (GLubyte) g;
					neuron_layer->neuron_vector[3 * (j * w + i) + 2].output.ival = (GLubyte) r;
				}

				// Each scan line needs have size multiple of 4.
				// otherwise discards until complete the size.
				for (k = (w % 4); k > 0; k--)
					fgetc (image_file);
			}
	}
}


/*! 
*********************************************************************************
* \brief Saves the current image (the image of the current window).
* \param[in] none.
* \pre The glut win variable with a valid value.
* \post A valid MAE input or output associated with the win variable.
* \return Zero (0) if a file with the name of the input{.pnm} or output{.pnm} is created or one (1) otherwise.
*********************************************************************************
*/

#ifndef NO_INTERFACE
int 
save_image ()
{
	int w, h;
	char file_name[256];
	FILE *image_file = (FILE *) NULL;
	INPUT_DESC *input;
	OUTPUT_DESC *output;
	GLubyte *image;
	int i, j, tfw;
	int r, g, b;
	
	// Opens image file
	if ((input = get_input_by_win (glutGetWindow ())) != NULL)
	{
		strcpy (file_name, input->name);
		strcat (file_name, ".pnm");
        	if ((image_file = fopen (file_name, "w")) == (FILE *) NULL)
       			Erro ("Cannot create file: ", file_name, "");

		// Write the image format and comment
		fprintf (image_file, "P3\n# CREATOR: MAE save_image ()\n");	

		h = input->neuron_layer->dimentions.y;
		w = input->neuron_layer->dimentions.x;

		// Write the image dimentions and range
		fprintf (image_file, "%d %d\n%d\n", w, h, 255);
		image = input->image;
		tfw = input->tfw;
	}
	else if ((output = get_output_by_win (glutGetWindow ())) != NULL)
	{
		strcpy (file_name, output->name);
		strcat (file_name, ".pnm");
        	if ((image_file = fopen (file_name, "w")) == (FILE *) NULL)
       			Erro ("Cannot create file: ", file_name, "");

		// Write the image format and comment
		fprintf (image_file, "P3\n# CREATOR: MAE save_image ()\n");	

		h = output->neuron_layer->dimentions.y;
		w = output->neuron_layer->dimentions.x;

		// Write the image dimentions and range
		fprintf (image_file, "%d %d\n%d\n", w, h, 255);
		image = output->image;
		tfw = output->tfw;
	}
	else
		return (1);
		
	for (i = h - 1; i >= 0; i--)  
	{
		for (j = 0; j < w; j++) 
		{
			r = image[3 * (i * tfw + j) + 0];
			g = image[3 * (i * tfw + j) + 1];
			b = image[3 * (i * tfw + j) + 2];
			fprintf (image_file, "%d\n%d\n%d\n", r, g, b);
		}
	}
	
	// Closes image file
	fclose (image_file);

	return (0);
}
#endif


void
remove_white_space_from_image_file(FILE *image_file)
{
	int character;
	char comment[512];

	do
	{
		do
		{
			character = fgetc(image_file);
		} while (isblank(character) || (character == '\n') || (character == '\r'));
		ungetc(character, image_file);

		if (character == '#')
			MAE_FGETS_WRAPPER(fgets(comment, 500, image_file),"remove_white_space_from_image_file");
		do
		{
			character = fgetc(image_file);
		} while (isblank(character) || (character == '\n') || (character == '\r'));
		ungetc(character, image_file);
	} while (character == '#');
}


void
load_input_image_from_pnm_file (INPUT_DESC *input, FILE *image_file)
{
	// See http://netpbm.sourceforge.net/doc/ppm.html
	int w, h;
	int max_range;

	// remove magic number
	fgetc(image_file);
	fgetc(image_file);
	remove_white_space_from_image_file(image_file);
	
	MAE_FSCANF_WRAPPER(fscanf(image_file, "%d", &w),"load_input_image_from_pnm_file");
	remove_white_space_from_image_file(image_file);

	MAE_FSCANF_WRAPPER(fscanf(image_file, "%d", &h),"load_input_image_from_pnm_file");
	remove_white_space_from_image_file(image_file);

	MAE_FSCANF_WRAPPER(fscanf(image_file, "%d", &max_range),"load_input_image_from_pnm_file");
	fgetc(image_file);

	switch (input->image_type)
	{
		case P2:
			load_input_image_with_p2_format (input, image_file);
			break;
		case P3:
			load_input_image_with_p3_format (input, image_file);
			break;
		case P5:
			load_input_image_with_p5_format (input, image_file);
			break;
		case P6:
			load_input_image_with_p6_format (input, image_file);
			break;
		default:
			Erro ("Unknown image format, the correct image format must be PNM-ASCII or BMP 24 Bits.", "", "");
	}
}


/*! 
*********************************************************************************
* \brief Loads the input image from file
* \param[in] input The input descriptor structure.
* \pre The input initialized.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
load_input_image (INPUT_DESC *input, char *file_name)
{
	FILE *image_file = (FILE *) NULL;
	char *image_file_name;
	char header[256];
	int i;

	if (file_name == NULL)
		image_file_name = input->image_file_name;
	else
		image_file_name = file_name;
		
	// Opens image file
        if ((image_file = fopen (image_file_name, "r")) == (FILE *) NULL)
       		Erro ("Cannot open input file: ", image_file_name, "");
	
	// Gets the image format	
	for (i = 0; i < 4; i++)
		header[i] = '\0';
	MAE_FREAD_WRAPPER(fread (header, 2, 1, image_file),"load_input_image");
	rewind (image_file);

	// Sets the image format	
	if (strcmp (header, "P2") == 0)
		input->image_type = P2;
	else if (strcmp (header, "P3") == 0)
		input->image_type = P3;
	else if (strcmp (header, "P5") == 0)
		input->image_type = P5;
	else if (strcmp (header, "P6") == 0)
		input->image_type = P6;
	else if (strcmp (header, "BM") == 0)
		input->image_type = BMP;
	else
		Erro ("Unknown image format, the correct image format must be PNM-ASCII or BMP 24 Bits.", "", "");

	if (input->image_type == BMP)
	{
		fseek(image_file, 54, SEEK_SET);	// BMP Header = 54 bytes
		load_input_image_with_BMP_format (input, image_file);
	}
	else
		load_input_image_from_pnm_file (input, image_file);
		
	// Closes image file
	fclose (image_file);
	
	// Sets the up to date input neuron layer flag to zero
	// input->up2date = 0;
	update_input_neurons (input);

	return;
}


void
normalize_to_rgb(GLubyte *r, GLubyte *g, GLubyte *b, NEURON_OUTPUT value, NEURON_OUTPUT max, NEURON_OUTPUT min, OUTPUT_TYPE output_type)
{
	switch (output_type)
	{
		case GREYSCALE:
		case BLACK_WHITE:
			if (min.ival >= 0)
			{
				*r = *g = *b = (GLubyte) (255.0 * (float) (value.ival - min.ival) / (float) (max.ival - min.ival));
			}
			else
			{
				if (max.ival <= 0)
				{
					*r = (GLubyte) (255.0 * (float) (max.ival - value.ival) / (float) (max.ival - min.ival));
					*g = *b = 0;
				}
				else if (max.ival > abs(min.ival))
				{
					if (value.ival >= 0)
					{
						*g = (GLubyte) (255.0 * (float) value.ival / (float) max.ival);
						*r = *b = 0;
					}
					else
					{
						*r = (GLubyte) (255.0 * (float) -value.ival / (float) max.ival);
						*g = *b = 0;
					}
				}
				else
				{
					if (value.ival >= 0)
					{
						*g = (GLubyte) (255.0 * (float) value.ival / (float) -min.ival);
						*r = *b = 0;
					}
					else
					{
						*r = (GLubyte) (255.0 * (float) -value.ival / (float) -min.ival);
						*g = *b = 0;
					}
				}
			}
		break;

		case GREYSCALE_FLOAT:
			if (min.fval >= 0)
			{
				*r = *g = *b = (GLubyte) (255.0 * (value.fval - min.fval) / (max.fval - min.fval));
			}
			else
			{
				if (max.fval <= 0)
				{
					*r = (GLubyte) (255.0 * (max.fval - value.fval) / (max.fval - min.fval));
					*g = *b = 0;
				}
				else if (max.fval > abs(min.fval))
				{
					if (value.fval >= 0)
					{
						*g = (GLubyte) (255.0 * value.fval / max.fval);
						*r = *b = 0;
					}
					else
					{
						*r = (GLubyte) (255.0 * -value.fval / max.fval);
						*g = *b = 0;
					}
				}
				else
				{
					if (value.fval >= 0)
					{
						*g = (GLubyte) (255.0 * value.fval / -min.fval);
						*r = *b = 0;
					}
					else
					{
						*r = (GLubyte) (255.0 * -value.fval / -min.fval);
						*g = *b = 0;
					}
				}
			}
		break;

		default:
			Erro("Invalid output type in normalize_to_rgb()", "", "");
	}
}



void
update_image_from_neural_layer(GLubyte *image, NEURON_LAYER *neuron_layer, int tfw)
{
	int i, j, w, h;
	NEURON *nv;
	OUTPUT_TYPE output_type;
	NEURON_OUTPUT max, min, value;
	GLubyte r, g, b;
	
	h = neuron_layer->dimentions.y;
	w = neuron_layer->dimentions.x;
	nv = neuron_layer->neuron_vector;

	output_type = neuron_layer->output_type;
	switch (output_type)
	{
		case COLOR:
			for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					value = nv[i * w + j].output;
					image[3 * (i * tfw + j) + 0] = RED   (value.ival);
					image[3 * (i * tfw + j) + 1] = GREEN (value.ival);
					image[3 * (i * tfw + j) + 2] = BLUE  (value.ival);
				}
			}
			break;

		case GREYSCALE:
			max = min = nv[0].output;
	
			for (i = 0; i < w * h; i++)
			{
				value = nv[i].output;
				
				if (value.ival > max.ival)
					max = value;
				if (value.ival < min.ival)
					min = value;
			}
	
			neuron_layer->max_neuron_output = max;
			neuron_layer->min_neuron_output = min;
			
			for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					value = nv[i * w + j].output;
					normalize_to_rgb(&r, &g, &b, value, max, min, output_type);
					image[3 * (i * tfw + j) + 0] = r;
					image[3 * (i * tfw + j) + 1] = g;
					image[3 * (i * tfw + j) + 2] = b;
				}
			}
			break;

		case BLACK_WHITE:
			max = min = nv[0].output;
			
			for (i = 0; i < w * h; i++)
			{
				value = nv[i].output;
				
				if (value.ival > max.ival)
					max = value;
				if (value.ival < min.ival)
					min = value;
			}
	
			neuron_layer->max_neuron_output = max;
			neuron_layer->min_neuron_output = min;
	
			for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					value = nv[i * w + j].output;
					normalize_to_rgb(&r, &g, &b, value, max, min, output_type);
					image[3 * (i * tfw + j) + 0] = r;
					image[3 * (i * tfw + j) + 1] = g;
					image[3 * (i * tfw + j) + 2] = b;
				}
			}
			break;

		case GREYSCALE_FLOAT:
			max = min = nv[0].output;
	
			for (i = 0; i < w * h; i++)
			{
				value = nv[i].output;
				
				if (value.fval > max.fval)
					max = value;
				if (value.fval < min.fval)
					min = value;
			}
	
			neuron_layer->max_neuron_output = max;
			neuron_layer->min_neuron_output = min;
                        
                        for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					value = nv[i * w + j].output;
					normalize_to_rgb(&r, &g, &b, value, max, min, output_type);
					image[3 * (i * tfw + j) + 0] = r;
					image[3 * (i * tfw + j) + 1] = g;
					image[3 * (i * tfw + j) + 2] = b;
				}
			}
			break;
	}
}



/*! 
*********************************************************************************
* \brief Updates the input image.
* \param[in] input The input descriptor structure.
* \pre The input initialized.
* \post The input image loaded.
* \return Nothing.
*********************************************************************************
*/

void 
update_input_image (INPUT_DESC *input)
{
	update_image_from_neural_layer(input->image, input->neuron_layer, input->tfw);
	
#ifndef NO_INTERFACE
	glutSetWindow (input->win);
	glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, input->tfw, 
		input->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, input->image);
	input->waiting_redisplay = 1;
	input_display ();	
#endif
	return;
}



void 
update_output_image (OUTPUT_DESC *output)
{
	update_image_from_neural_layer(output->image, output->neuron_layer, output->tfw);

#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	glBindTexture (GL_TEXTURE_2D, (GLuint) output->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output->tfw, 
		output->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, output->image);
	output_display ();
#endif
	return;
}



void 
rand_output_image (OUTPUT_DESC *output)
{
	int i, w, h;
	NEURON *nv;	
	
	h = output->neuron_layer->dimentions.y;
	w = output->neuron_layer->dimentions.x;
	
	nv = output->neuron_layer->neuron_vector;
	
	switch (output->neuron_layer->output_type)
	{
		case COLOR:
			for (i = 0; i < w * h; i++)
				nv[i].output.ival = rand () % NUM_COLORS;
			break;
		case GREYSCALE:
			for (i = 0; i < w * h; i++)
				nv[i].output.ival = rand () % NUM_GREYLEVELS;
			break;
		case BLACK_WHITE:
			for (i = 0; i < w * h; i++)
				nv[i].output.ival = ((rand () % NUM_GREYLEVELS) >= (NUM_GREYLEVELS / 2)) ? NUM_GREYLEVELS - 1: 0;
			break;
		case GREYSCALE_FLOAT:
			for (i = 0; i < w * h; i++)
				nv[i].output.fval = ((float) NUM_GREYLEVELS) * ((float) (rand () % 100) / 50.0 - 1.0);
			break;
	}
	update_output_image (output);
	
	return;
}



void 
make_input_image (INPUT_DESC *input)
{
	int i;
	int w, h;
	char header[256];
	char message[256];
	char character = 0;
	FILE *path_file = (FILE *) NULL;
	FILE *image_file = (FILE *) NULL;

	// Open the path file
	if ((path_file = fopen (input->name, "r")) == (FILE *) NULL)
		Erro ("Cannot open input file: ", input->name, "");
	
	// Get the image file name
	MAE_FGETS_WRAPPER(fgets (input->image_file_name, 250, path_file),"make_input_image");
	i = 0;
	while (isgraph (input->image_file_name[i]))
		i++;
	input->image_file_name[i] = '\0';
	
	// Close the path file
	fclose (path_file);
	
	// Opens image file
        if ((image_file = fopen (input->image_file_name, "r")) == (FILE *) NULL)
       		Erro ("Cannot open input file: ", input->image_file_name, "");
	
	// Gets the image format	
	for (i = 0; i < 4; i++)
		header[i] = '\0';
	MAE_FREAD_WRAPPER(fread(header, 2, 1, image_file),"make_input_image");

	if (strcmp (header, "P2") == 0)
		input->image_type = P2;
	else if (strcmp (header, "P3") == 0)
		input->image_type = P3;
	else if (strcmp (header, "P6") == 0)
		input->image_type = P6;
	else if (strcmp (header, "BM") == 0)
		input->image_type = BMP;
	else
		Erro ("Unknown image format, the correct image format must be PNM-ASCII or BMP 24 Bits.", "", "");
	
	// Discards any comments in the begin of the image file
	if (input->image_type != BMP)
	{
		while (character != '\n')
			character = fgetc (image_file);
	
		if ((character = fgetc (image_file)) == '#')
		{
			while (character != '\n')
				character = fgetc (image_file);
		}
		else
			ungetc (character, image_file);
	}
	
	// Gets the image dimentions
	if (input->image_type == BMP)
	{
		fseek(image_file, 18, SEEK_SET);	
		MAE_FREAD_WRAPPER(fread(&w, 4, 1, image_file),"make_input_image");
		MAE_FREAD_WRAPPER(fread(&h, 4, 1, image_file),"make_input_image");
		printf("w: %d, h: %d\n", w, h);
	}
	else
		MAE_FSCANF_WRAPPER(fscanf (image_file, "%d %d\n", &(w), &(h)),"make_input_image");
	
	// Gets the pixels value range
	if (input->image_type == BMP)
		input->max_range = 255;
	else
		MAE_FSCANF_WRAPPER(fscanf (image_file, "%d\n", &(input->max_range)),"make_input_image");
	
	// Computes the input image dimentions
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	// Saves the image dimentions
	input->ww = w;
	input->wh = h;
	
	switch(TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vpw = input->ww;
			input->vph = input->wh;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;
			
	if (input->image == (GLubyte *) NULL)
	{
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
		for (i = 0; i < input->tfh * input->tfw * 3; i++)
			input->image[i] = 0;
	}		

	// Closes image file
	fclose (image_file);
	
	load_input_image (input, input->image_file_name);
	
	return;
}



void 
make_output_image (OUTPUT_DESC *output)
{
	int i;
	
	output->tfh = nearest_power_of_2 (output->wh);
	output->tfw = nearest_power_of_2 (output->ww);
	output->image = (GLubyte *) alloc_mem (output->tfh * output->tfw * 3 * sizeof (GLubyte));
	for (i = 0; i < output->tfh * output->tfw * 3; i++)
		output->image[i] = 0;
	
	return;
}


#ifndef NO_INTERFACE
void 
input_reshape (int w, int h)
{
	INPUT_DESC *input;
	double winput, hinput, d, ang;

	input = get_input_by_win (glutGetWindow ());
	winput = (GLdouble) input->vpw;
	hinput = (GLdouble) input->vph;
	d = sqrt(winput*winput + hinput*hinput);
	ang = 2.5 * ((atan2 (hinput/2.0, d) * 180.0)/PI);
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (ang, (GLfloat)w/(GLfloat)h, d/2.0, d+d/1000.0);
	
	return;
}


void 
output_reshape (int w, int h)
{
	OUTPUT_DESC *output;
	double woutput, houtput, d, ang;

	output = get_output_by_win (glutGetWindow ());
	woutput = (GLdouble) output->ww;
	houtput = (GLdouble) output->wh;
	d = sqrt(woutput*woutput + houtput*houtput);
	ang = 2.5 * ((atan2 (houtput/2.0, d) * 180.0)/PI);
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (ang, (GLfloat)w/(GLfloat)h, d/2.0, d+d/1000.0);
	
	return;
}



/*
*********************************************************************************
* Function: graph_reshape							*
* Description: glut reshape function of the graph output window			*
* Inputs: filter decriptor of the graph filter					*
*********************************************************************************
*/

void 
graph_reshape (int w, int h)
{
	OUTPUT_DESC *output = (OUTPUT_DESC *) NULL;
	FILTER_DESC *filter_desc = (FILTER_DESC *) NULL;
	double x_min, x_max, y_min, y_max;
	double x_range, y_range;
	
	// Get current output
	output = get_output_by_win (glutGetWindow ());

	// Get the associated filter descriptor
	filter_desc = get_filter_by_output (output->neuron_layer);

	// Get the Filter Parameters
	x_min = filter_desc->filter_params->next->param.fval;
	x_max = filter_desc->filter_params->next->next->param.fval;
	y_min = filter_desc->filter_params->next->next->next->param.fval;
	y_max = filter_desc->filter_params->next->next->next->next->param.fval;
	
	x_range = x_max - x_min;
	y_range = y_max - y_min;
	
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	// The X and Y range are 20 percent smaller then the window area
	glOrtho (x_min - 0.1*x_range, x_max + 0.1*x_range, y_min - 0.1*y_range, y_max + 0.1*y_range, 0.0, 1.0);	
        
        return;
}


void 
input_display (void)
{
	INPUT_DESC *input;
	GLdouble w, h, d;
	char info[256];
	char max_min[256];
	int i;
	float r, g, b;

	input = get_input_by_win (glutGetWindow ());

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0, 
		   0.0, 0.0, -d,
		   0.0, 1.0, 0.0);
	glTranslatef (-w/2.0, -h/2.0, -d);

	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (0.0, 0.0, 0.0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, (GLuint) input->tex);
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, 0.0); glVertex3i (0, 0, 0);
	glTexCoord2f (0.0, 1.0); glVertex3i (0, input->tfh, 0);
	glTexCoord2f (1.0, 1.0); glVertex3i (input->tfw, input->tfh, 0);
	glTexCoord2f (1.0, 0.0); glVertex3i (input->tfw, 0, 0);
	glEnd ();
	glDisable (GL_TEXTURE_2D);

	glColor3f (1.0, 0.0, 0.0);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	DRAWRECTANGLE (input->wx, input->wy, input->neuron_layer->dimentions.x, input->neuron_layer->dimentions.y);
	glDisable (GL_LINE_STIPPLE);

	if (input->cross_list != NULL)
	{
		for (i = 0; i < input->cross_list_size; i++)
		{
			if (input->cross_list_colored)
			{
				if (((i + 1) & 7) == 0)
				{
					r = g = b = 1.0;
				}
				else
				{
					r = (float) ((i + 1) & 1);
					g = (float) (((i + 1) >> 1) & 1);
					b = (float) (((i + 1) >> 2) & 1);
				}
				glColor3f(r, g, b);
				DRAWCROSS(input->cross_list[i].x, input->cross_list[i].y, input->ww, input->wh);
			}
			else
			{
				DRAWCROSS(input->cross_list[i].x, input->cross_list[i].y, input->ww, input->wh);
			}
		}		
	}

	if (input->rectangle_list != NULL)
	{
		for (i = 0; i < input->rectangle_list_size; i++)
		{
			glColor3f (input->rectangle_list[i].r, input->rectangle_list[i].g, input->rectangle_list[i].b);
			DRAWRECTANGLE (input->rectangle_list[i].x, input->rectangle_list[i].y, input->rectangle_list[i].w, input->rectangle_list[i].h);
		}		
	}
	glColor3f (1.0, 0.0, 0.0);
		
	if (move_active == 1)
		DRAW_XY_MARK(input->wxd, input->wyd, input->vpw, input->vph);
	
	if ((move_active == 1) && (draw_active == 1))
		DRAWCROSS(input->wxd, input->wyd, input->ww, input->wh);
		
	glColor3f (0.0, 1.0, 0.0);
	if (input->green_cross)
		DRAWCROSS(input->green_cross_x, input->green_cross_y, input->ww, input->wh);
	glColor3f (1.0, 0.0, 0.0);
	
	d = (GLdouble) input->vph / 64.0;
	if (d > ((GLdouble) input->vpw / 64.0))
		d = (GLdouble) input->vpw / 64.0;
	if (input->vpxo > 0)
		glBegin(GL_LINES); glVertex2f (-d, 0.0); glVertex2f (0.0, 0.0); glEnd (); 
	if (input->vpxo + input->vpw < input->ww)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw+d, 0.0); glEnd (); 
	if (input->vpyo + input->vph < input->wh)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, 0.0); glVertex2f ((GLdouble) input->vpw, -d); glEnd (); 
	if (input->vpyo > 0)
		glBegin(GL_LINES); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph); glVertex2f ((GLdouble) input->vpw, (GLdouble) input->vph+d); glEnd (); 
	
	switch (input->neuron_layer->output_type)
	{
		case COLOR:	
			sprintf (info, "Color Input\r");	
			break;
		case BLACK_WHITE:
			sprintf (info, "Black & White Input\r");		
			break;	
		case GREYSCALE:	
			sprintf (info, "Greyscale Input\r");
			sprintf (max_min, "Max = %d, Min = %d\r", input->neuron_layer->max_neuron_output.ival,
						          input->neuron_layer->min_neuron_output.ival);
			glRasterPos2f (0.0, -h / 10.0);
			for (i = 0; i < strlen (max_min); i++)
				glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, max_min[i]);
			break;
		case GREYSCALE_FLOAT:
			sprintf (info, "Greyscale_float Input\r");
			sprintf (max_min, "Max = %f, Min = %f\r", input->neuron_layer->max_neuron_output.fval,
						          input->neuron_layer->min_neuron_output.fval);
			glRasterPos2f (0.0, -h / 10.0);
			for (i = 0; i < strlen (max_min); i++)
				glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, max_min[i]);
			break;	
	}
	
	glRasterPos2f (0.0, h + h / 20.0);
	
	for (i = 0; i < strlen (info); i++)
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, info[i]);
		
	glutSwapBuffers ();
	input->waiting_redisplay = 0;
	
	return;
}


void 
output_display (void)
{
	OUTPUT_DESC *output;
	double w, h, d;	
	char info[256];
	char max_min[256];
	int i;
	float r, g, b;

	output = get_output_by_win (glutGetWindow ());

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) output->ww;
	h = (GLdouble) output->wh;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0, 
		   0.0, 0.0, -d,
		   0.0, 1.0, 0.0);
	glTranslatef (-w/2.0, -h/2.0, -d);

	glClear (GL_COLOR_BUFFER_BIT);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, (GLuint) output->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output->tfw, 
		output->tfh, 0, GL_RGB, GL_UNSIGNED_BYTE, output->image);
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, 0.0); glVertex3i (0, 0, 0);
	glTexCoord2f (0.0, 1.0); glVertex3i (0, output->tfh, 0);
	glTexCoord2f (1.0, 1.0); glVertex3i (output->tfw, output->tfh, 0);
	glTexCoord2f (1.0, 0.0); glVertex3i (output->tfw, 0, 0);
	glEnd ();
	glDisable (GL_TEXTURE_2D);

	glColor3f (1.0, 0.0, 0.0);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	DRAWRECTANGLE (0, 0, output->ww, output->wh);
	glDisable (GL_LINE_STIPPLE);
	
	if (output->cross_list != NULL)
	{
		for (i = 0; i < output->cross_list_size; i++)
		{
			if (output->cross_list_colored)
			{
				if (((i + 1) & 7) == 0)
				{
					r = g = b = 1.0;
				}
				else
				{
					r = (float) ((i + 1) & 1);
					g = (float) (((i + 1) >> 1) & 1);
					b = (float) (((i + 1) >> 2) & 1);
				}
				glColor3f(r, g, b);
				DRAWCROSS(output->cross_list[i].x, output->cross_list[i].y, output->ww, output->wh);
			}
			else
			{
				DRAWCROSS(output->cross_list[i].x, output->cross_list[i].y, output->ww, output->wh);
			}
		}
	}

	if (output->rectangle_list != NULL)
	{
		for (i = 0; i < output->rectangle_list_size; i++)
		{
			glColor3f (output->rectangle_list[i].r, output->rectangle_list[i].g, output->rectangle_list[i].b);
			DRAWRECTANGLE (output->rectangle_list[i].x, output->rectangle_list[i].y, output->rectangle_list[i].w, output->rectangle_list[i].h);
		}		
	}
	glColor3f (1.0, 0.0, 0.0);
		
	switch (output->neuron_layer->output_type)
	{
		case COLOR:	
			sprintf (info, "Color Output\r");	
			break;
		case BLACK_WHITE:
			sprintf (info, "Black & White Output\r");		
			break;	
		case GREYSCALE:	
			sprintf (info, "Greyscale Output\r");
			sprintf (max_min, "Max = %d, Min = %d\r", output->neuron_layer->max_neuron_output.ival,
						          output->neuron_layer->min_neuron_output.ival);
			glRasterPos2f (0.0, -h / 10.0);
			for (i = 0; i < strlen (max_min); i++)
				glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, max_min[i]);
			break;
		case GREYSCALE_FLOAT:
			sprintf (info, "Greyscale_float Output\r");
			sprintf (max_min, "Max = %f, Min = %f\r", output->neuron_layer->max_neuron_output.fval,
						          output->neuron_layer->min_neuron_output.fval);
			glRasterPos2f (0.0, -h / 10.0);
			for (i = 0; i < strlen (max_min); i++)
				glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, max_min[i]);
			break;	
	}
	
	glRasterPos2f (0.0, h + h / 20.0);
	for (i = 0; i < strlen (info); i++)
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, info[i]);
	
	glutSwapBuffers ();

	return;
}



/*
*********************************************************************************
* Function: graph_display							*
* Description: glut display function of the graph output window			*
* Inputs: filter decriptor of the graph filter					*
*********************************************************************************
*/

void 
graph_display (void)
{
	NEURON_LAYER_LIST *nl_list = (NEURON_LAYER_LIST *) NULL;		
	NEURON_LAYER *nl_in = (NEURON_LAYER *) NULL;
	OUTPUT_DESC *output = (OUTPUT_DESC *) NULL;
	NEURON *neuron_vector = (NEURON *) NULL;
	static FILTER_DESC *filter_desc = (FILTER_DESC *) NULL;
	static double x_min, x_max, y_min, y_max; 
	static double a, b;
	int i, j, w, h;
	double x, y;
	// Default colors: red, green, blue, yellow, pink, light blue and white
	double red[]   = {1.0,0.0,0.0,1.0,1.0,0.0,1.0}; 
	double green[] = {0.0,1.0,0.0,1.0,0.0,1.0,1.0};
	double blue[]  = {0.0,0.0,1.0,0.0,1.0,1.0,1.0};
	char info[256];
	
	// Get current output
	output = get_output_by_win (glutGetWindow ());

	// Get the associated filter descriptor
	filter_desc = get_filter_by_output (output->neuron_layer);

	// Get the Filter Parameters
	x_min = filter_desc->filter_params->next->param.fval;
	x_max = filter_desc->filter_params->next->next->param.fval;
	y_min = filter_desc->filter_params->next->next->next->param.fval;
	y_max = filter_desc->filter_params->next->next->next->next->param.fval;

	// Scale all curves with the first one
	nl_in = filter_desc->neuron_layer_list->neuron_layer; 
	w = nl_in->dimentions.x;

	a = (x_max - x_min)/ (double) w;
	b = (x_max + x_min)/ 2.0;
	
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClear (GL_COLOR_BUFFER_BIT);
	
	// Draw a white square around the graph
	glColor3f (1.0, 1.0, 1.0);
	DRAW_SQUARE_3f(x_min, y_min, x_max, y_max);
	
	// Draw the X and Y axis
	DRAW_XY_AXIS(x_min, y_min, x_max, y_max);
	
	// Mark the X and Y tics
	DRAW_XY_TICS(x_min, x_max, y_min, y_max, 11);
				
	// Plot all neuron layers contents
	for (j = 0, nl_list = filter_desc->neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
	{
		nl_in = nl_list->neuron_layer;
		
		w = nl_in->dimentions.x;
		h = nl_in->dimentions.y;
		
		// Get the midle row of the neuron layer
		neuron_vector = &(nl_in->neuron_vector[(h/2)*w]);
		
		glColor3f (red[j], green[j], blue[j]);
		
		glBegin (GL_LINE_STRIP);
			for (i = 0; i < w; i++) 
			{
				x = a * (double) (i - w/2) + b;
				y = neuron_vector[i].output.fval;
				glVertex3f (x, y, 0.0);
			}
		glEnd ();
		
		// Show the legend
		glRasterPos2f (x_max - (x_max - x_min)/4.0, y_max - (double) (j + 1) * (y_max - y_min)/20.0);
		sprintf (info, "- %s\r", nl_in->name);	
		DRAW_STRING(info);
		j++;
	}
	glFlush ();
	
	glutSwapBuffers ();
	
	return;
}
#endif


void 
update_input_status (INPUT_DESC *input)
{
	char *output_type;
	
	switch (input->neuron_layer->output_type)
	{
		case COLOR:		
			output_type = "color output";
			break;
		case GREYSCALE:	
			output_type = "greyscale output";
			break;
		case BLACK_WHITE:		
			output_type = "b&w output";
			break;	
		case GREYSCALE_FLOAT:
			output_type = "greyscale_float output";
			break;
		default:
			output_type = "greyscale output";
			break;
	}
	
	sprintf (status_info, "%s: (%d, %d), [%d, %d]  -  %s", input->name, input->wxd, input->wyd, input->ww, input->wh, output_type);
	set_status_bar (status_info);

	selected_input = input;
	selected_output = NULL;
	
	return;
}



void 
update_output_status (OUTPUT_DESC *output)
{
	char *output_type;
	
	switch (output->neuron_layer->output_type)
	{
		case COLOR:		
			output_type = "color output";
			break;
		case GREYSCALE:	
			output_type = "greyscale output";
			break;
		case BLACK_WHITE:		
			output_type = "b&w output";
			break;	
		case GREYSCALE_FLOAT:
			output_type = "greyscale_float output";
			break;
		default:	
			output_type = "greyscale output";
			break;
	}
	
	sprintf (status_info, "%s: [%d, %d]  -  %s", output->name, output->ww, output->wh, output_type);
	set_status_bar (status_info);

	selected_output = output;
	selected_input = NULL;
	
	return;
}



void 
check_input_bounds (INPUT_DESC *input, int wx, int wy)
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
		update_input_image (input);
	
	return;
}


#ifndef NO_INTERFACE
void 
input_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	INPUT_DESC *input;

	input = get_input_by_win (glutGetWindow ());
	if (input->valid == 1)
	{
		glGetIntegerv (GL_VIEWPORT, viewport);
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
		/*  note viewport[3] is height of window in pixels  */
		real_y = viewport[3] - (GLint) y - 1;

		gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

		input->wx += (GLint) (wx) - input->wxd;
		input->wy += (GLint) (wy) - input->wyd;
		check_input_bounds (input, wx, wy);

		input->mouse_move = 1;
		glutPostWindowRedisplay (input->win);
	}
	if (input->input_controler != NULL)
		(*(input->input_controler)) (input, SET_POSITION);
		
	return;
}


char * 
get_neuron_layer_neuron_val(char *color_val, NEURON_LAYER *neuron_layer, int wx, int wy)
{
	int w, h;
	NEURON_OUTPUT pixel;
	int r, g, b;

	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;
	
	if ((wx >= 0) && (wx < w) && (wy >= 0) && (wy < h))
		pixel = neuron_layer->neuron_vector[w * (int) (wy) + (int) (wx)].output;
	else
		pixel.ival = 0;
		
	switch (neuron_layer->output_type)
	{
		case COLOR:
			r = RED   (pixel.ival);
			g = GREEN (pixel.ival);
			b = BLUE  (pixel.ival);
			
			sprintf (color_val, "r = %03d, g = %03d, b = %03d (color)", r, g, b);
			break;
		case GREYSCALE:
			sprintf (color_val, "intensity = %d (greyscale)", pixel.ival);
			break;
		case BLACK_WHITE:
			sprintf (color_val, "value = %d (b&w)", pixel.ival / 255);
			break;	
		case GREYSCALE_FLOAT:
			sprintf (color_val, "intensity = %f (greyscale_float)", pixel.fval);
			break;	
	}

	return (color_val);
}


void 
input_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	INPUT_DESC *input;
	
	input = get_input_by_win (glutGetWindow ());
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	sprintf (mouse_pos, "mouse: (%d, %d)", (int) (wx), (int) (wy));
	set_mouse_bar (mouse_pos);
	set_color_bar (get_neuron_layer_neuron_val(color_val, input->neuron_layer, wx, wy));
	
	return;
}


void 
output_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;		/*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;	/*  returned world x, y, z coords  */
	static char mouse_pos[256];
	static char color_val[256];
	OUTPUT_DESC *output;
	
	output = get_output_by_win (glutGetWindow ());
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	sprintf (mouse_pos, "mouse: (%d, %d)", (int) (wx), (int) (wy));
	set_mouse_bar (mouse_pos);
	set_color_bar (get_neuron_layer_neuron_val(color_val, output->neuron_layer, wx, wy));
	
	return;
}



/*
*********************************************************************************
* Function: graph_passive_motion						*
* Description: glut passive motion function of the graph output window		*
* Inputs: output descriptor of the graph output					*
*********************************************************************************
*/

void 
graph_passive_motion (int x, int y)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  //  OpenGL y coordinate position 
	GLdouble wx, wy, wz;  // returned world x, y, z coords
	static char mouse_pos[256];
	static char color_val[256];

	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	real_y = viewport[3] - (GLint) y - 1;

	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	sprintf (mouse_pos, "(%.2e, %.2e)", wx, wy);
	set_mouse_bar (mouse_pos);
	
	sprintf (color_val, " ");
	set_color_bar (color_val);

	return;
}


void 
input_mouse (int button, int state, int x, int y) 
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	INPUT_DESC *input;

	input = get_input_by_win (glutGetWindow ());

	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note: viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;
	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 
	input->wxme = (GLint) (wx + 0.5);
	input->wyme = (GLint) (wy + 0.5);

	switch (button) 
	{
		case GLUT_LEFT_BUTTON:
			input->mouse_button = GLUT_LEFT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->x_old = input->x;
				input->y_old = input->y;
				input->x = x;
				input->y = y;
				input->mouse_state = GLUT_DOWN;
				input->wxd_old = input->wxd;
				input->wyd_old = input->wyd;
				input->wxd = (GLint) (wx + 0.5);
				input->wyd = (GLint) (wy + 0.5);
				
				if (((input->wxd >= input->wx) && (input->wxd <= (input->wx+input->ww))) && 
				    ((input->wyd >= input->wy) && (input->wyd <= (input->wy+input->wh))))
					input->valid = 1;

				update_input_status (input);
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
				input->valid = 0;
				glutPostWindowRedisplay (input->win);
			}
			break;
		case GLUT_RIGHT_BUTTON:
			input->mouse_button = GLUT_RIGHT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			input->mouse_button = GLUT_MIDDLE_BUTTON;
			if (state == GLUT_DOWN) 
			{
				input->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				input->mouse_state = GLUT_UP;
			}
			break;
		default:
			break;
	}
	
	if (input->input_controler != NULL)
		(*(input->input_controler)) (input, SET_POSITION);
                
        return;	
}


void 
output_mouse (int button, int state, int x, int y) 
{
	OUTPUT_DESC *output;
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint real_y;  /*  OpenGL y coordinate position  */
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */

	output = get_output_by_win (glutGetWindow ());

	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	/*  note: viewport[3] is height of window in pixels  */
	real_y = viewport[3] - (GLint) y - 1;
	gluUnProject ((GLdouble) x, (GLdouble) real_y, 1.0, 
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 
	output->wxme = (GLint) (wx);
	output->wyme = (GLint) (wy);

	switch (button) 
	{
		case GLUT_LEFT_BUTTON:
			output->mouse_button = GLUT_LEFT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				output->x_old = output->x;
				output->y_old = output->y;
				output->x = x;
				output->y = y;
				output->mouse_state = GLUT_DOWN;
				output->wxd_old = output->wxd;
				output->wyd_old = output->wyd;
				output->wxd = (GLint) (wx);
				output->wyd = (GLint) (wy);
				
				update_output_status (output);
				update_output_image (output);
				glutPostWindowRedisplay (output->win);
			}
			else if (state == GLUT_UP)
			{
				output->mouse_state = GLUT_UP;
				output->valid = 0;
				glutPostWindowRedisplay (output->win);
			}
			break;
		case GLUT_RIGHT_BUTTON:
			output->mouse_button = GLUT_RIGHT_BUTTON;
			if (state == GLUT_DOWN) 
			{
				output->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				output->mouse_state = GLUT_UP;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			output->mouse_button = GLUT_MIDDLE_BUTTON;
			if (state == GLUT_DOWN) 
			{
				output->mouse_state = GLUT_DOWN;
			}
			else if (state == GLUT_UP)
			{
				output->mouse_state = GLUT_UP;
			}
			break;
		default:
			break;
	}

	if (output->output_handler != NULL)
		(*(output->output_handler)) (output, -1, button, state);
        
        return;	
}
#endif


/*
*********************************************************************************
* Function: graph_mouse								*
* Description: 									*
* Inputs: 									*
*********************************************************************************
*/

void 
graph_mouse (int button, int state, int x, int y)
{
	return;
}



void 
keyboard (unsigned char key, int x, int y)
{
	char command[256];
	
	sprintf (command, "key %c;", key);
	interpreter (command);
	
	return;
}



void 
create_input_window (INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	float f;
	int x, y;
#endif
	input->wx = 0;
	input->wy = 0;
	input->x = input->x_old = input->ww / 2;
	input->y = input->y_old = input->wh / 2;
	input->wxd = input->wxd_old = input->ww / 2;
	input->wyd = input->wyd_old = input->wh / 2;

	input->green_cross_x = input->ww / 2;
	input->green_cross_y = input->wh / 2;
	input->green_cross = 0;

	input->valid = 0;
	
	if (input->input_generator != NULL)
	{
		(*(input->input_generator)) (input, 0);
	}
	else
	{
		make_input_image (input);
#ifndef NO_INTERFACE
		f = 1.0;
		while ((((float)input->ww * f) < 128.0) || (((float)input->wh * f) < 128.0))
			f += 1.0;
		while ((((float)input->ww * f) > 1024.0) || (((float)input->wh * f) > 1024.0))
			f *= 0.9;
		glutInitWindowSize ((int) ((float)input->ww * f), (int) ((float)input->wh * f));
		if (read_window_position (input->name, &x, &y))
			glutInitWindowPosition (x, y);
		else
			glutInitWindowPosition (-1, -1);
		input->win = glutCreateWindow (input->name);
		glGenTextures (1, (GLuint *)(&(input->tex)));
		input_init (input);
		glutReshapeFunc (input_reshape);
		glutDisplayFunc (input_display);
		glutMouseFunc (input_mouse);
		glutPassiveMotionFunc (input_passive_motion);
		glutKeyboardFunc (keyboard);
#endif
	}
	
	return;
}



void 
create_output_window (OUTPUT_DESC *output)
{
#ifndef NO_INTERFACE
	float f;
	int x, y;
#endif
	if (output->neuron_layer == NULL)
		Erro("The output named ", output->name, " does not appear to be connected to a neuron_layer.");
		
	output->ww = output->neuron_layer->dimentions.x;
	output->wh = output->neuron_layer->dimentions.y;
	
	make_output_image (output);
	
#ifndef NO_INTERFACE	
	f = 1.0;
	while ((((float)output->ww * f) < 128.0) || (((float)output->wh * f) < 128.0))
		f += 1.0;
	while ((((float)output->ww * f) > 1024.0) || (((float)output->wh * f) > 1024.0))
		f *= 0.9;
	glutInitWindowSize ((int) ((float)output->ww * f), (int) ((float)output->wh * f));
	if (read_window_position (output->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	output->win = glutCreateWindow (output->name);
	glGenTextures (1, (GLuint *)(&(output->tex)));
	output_init (output);
	glutReshapeFunc (output_reshape);
	glutDisplayFunc (output_display);
	glutMouseFunc (output_mouse);
	glutPassiveMotionFunc (output_passive_motion);
	glutKeyboardFunc (keyboard);
#endif
	rand_output_image (output);
	return;
}
