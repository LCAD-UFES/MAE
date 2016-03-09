#include "mae.h"
#include "../robot_neural_computer.h"

//Varibles to draw things
int g_nCenterX, g_nCenterY;

void
init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;

	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;

	g_nCenterX = input->vpw / 2.0;
	g_nCenterY = input->vph / 2.0;
}

int init_user_functions (void)
{    
	return 0;
}

// ----------------------------------------------------------------------------
// InitWindow - Inicializa os recursos de cada janela
//				de entrada
//
// Entrada: input - Janela de entrada
//			nInputImage - Indica qual e a janela.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------

void
InitWindow(INPUT_DESC *input)
{
	int x, y;
	int nImageSource;

	if (input->input_generator_params->next == NULL)
	{
		return;
	}

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	switch (nImageSource)
	{
		case GET_IMAGE_FROM_PNM:
		{
			make_input_image (input);
		}
		break;

		default:
			Erro ("Invalid image source.", "", "");
	}

	//init (input);
	input->up2date = 0;
	update_input_neurons (input);

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
#endif
}

void input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		InitWindow(input);
		update_input_neurons (input);
	}
	else if (status == MOVE)
	{
		check_input_bounds (input, input->wxd, input->wyd);
#ifndef NO_INTERFACE
		glutSetWindow(input->win);
		input_display ();
#endif
		all_outputs_update ();
	}

	return;
}
 
void input_controler (INPUT_DESC *input, int status)
{
	return;
}

void draw_output (char *output_name, char *input_name)
{
	OUTPUT_DESC *output;

	output = get_output_by_name (output_name);

	update_output_image (output);
#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
#endif
}

void f_keyboard (char *key_value)
{	
	switch (key_value[0]) 
	{
	case 'A':
	case 'a':
		break;
	case 'B':
	case 'b':
		break;
	case 'C':
	case 'c':
		break;
	case 'D':
	case 'd':
		break;
	case 'E':
	case 'e':	
		break;
	case 'Q':
	case 'q':
		exit (0);
	}

	return;
}
