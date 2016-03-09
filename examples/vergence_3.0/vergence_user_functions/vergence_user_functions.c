#include <locale.h>
#include "vergence_user_functions.h"
#include "vergence_filters.h"

// this function is required by the MAE core modules
void
draw_output (char *output_name __attribute__ ((unused)),
		char *input_name __attribute__ ((unused)) ) {}

int
init_user_functions ()
{
	char *locale_string;

	locale_string = setlocale (LC_ALL, "C");
	if (locale_string == NULL)
	{
		fprintf (stderr, "Could not set locale.\n");
		exit (1);
	}
	else
		printf ("Locale set to %s.\n", locale_string);

	interpreter ((char*)"toggle move_active;");

	interpreter ((char*)"toggle draw_active;");

	return (0);
}


void
make_input_image_vergence (INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = w;
	input->tfh = h;

	input->ww = w;
	input->wh = h;

	switch(TYPE_SHOW)
	{
	case SHOW_FRAME:
		input->vpw = input->neuron_layer->dimentions.x;
		input->vph = input->neuron_layer->dimentions.y;
		break;
	case SHOW_WINDOW:
		input->vph = h;
		input->vpw = w;
		break;
	default:
		sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
		Erro ((char*)"Invalid Type Show ", message, (char*)" Error in update_input_image.");
		return;
	}

	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if(input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}

void
init_vergence (INPUT_DESC *input)
{
	int x,y;

	make_input_image_vergence (input, IMAGE_WIDTH, IMAGE_HEIGHT);

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
}

void
input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_vergence (input);
	}
}

void
update_filters_dendrites_neurons_and_outputs()
{
	all_filters_update ();
	all_dendrites_update ();
	all_neurons_update ();
	all_outputs_update ();
}

void
update_filters_and_outputs()
{
	all_filters_update ();
	all_outputs_update ();
}

void
move_left_input_center(int x, int y)
{
	left_eye_center_x = x;
	left_eye_center_y = y;
	move_input_window (in_left.name, x, y);
}

void
move_right_input_center(int x, int y)
{
	right_eye_center_x = x;
	right_eye_center_y = y;
	move_input_window (in_right.name, x, y);
}

void
output_handler_average_value (OUTPUT_DESC *output,
		int type_call __attribute__ ((unused)) ,
		int mouse_button __attribute__ ((unused)) ,
		int mouse_state __attribute__ ((unused)) )
{
	int u, v, w, h;
	int upper_x_lim,lower_x_lim;
	float average_value;

	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;

	lower_x_lim = 0;
	upper_x_lim = w;
//	lower_x_lim = w/3;
//	upper_x_lim = w - (w/3);

	// Accumulates average value position for the whole neuron layer.
	average_value = 0.0;
	for (v = 0; v < h; v++)
	{
		for (u = lower_x_lim; u < upper_x_lim; u++)
		{
			average_value += output->neuron_layer->neuron_vector[v * w + u].output.fval;
		}
	}

	average_value /= (float) (upper_x_lim - lower_x_lim)*h;
	//printf("Average Disparity Value: %f\n",average_value);

	// Saves the average value position
	nl_target_coordinates.neuron_vector[0].output.fval = average_value;
}

int
EvaluateOutput (OUTPUT_DESC *output)
{
	int i, k;
	int nClassAux,nClassID = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;
	int entryId;

	neuron_vector = output->neuron_layer->neuron_vector;

	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de classes. A classe que tiver o maior numero de neuronios
	// setados sera considerada como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		nClassAux = neuron_vector[i].output.ival;

		if (nClassAux != nClassID)
		{
			nAux = 1;

			for (k = i + 1; k < (output->wh * output->ww); k++)
			{
				entryId = neuron_vector[k].output.ival;
				if (entryId == nClassAux)
					nAux++;
			}

			// Verifica se eh a classe com mais neuronios setados ateh agora
			// Se for, seta a classe e atualiza o valor maximo
			if (nAux > nMax)
			{
				nMax = nAux;
				nClassID = nClassAux;
			}
		}
	}

	return (nClassID);
}

void
output_handler_mode_value (OUTPUT_DESC *output,
		int type_call __attribute__ ((unused)) ,
		int mouse_button __attribute__ ((unused)) ,
		int mouse_state __attribute__ ((unused)) )
{
	if (network_state == RUNNING)
	{
		nl_target_coordinates.neuron_vector[0].output.fval = (float) EvaluateOutput(output);

		printf("%f\n", nl_target_coordinates.neuron_vector[0].output.fval);
	}
}

void
input_controller (INPUT_DESC *input, int status)
{
	(void)status;

	input->mouse_button = -1;
}

void
input_controller_left (INPUT_DESC *input, int status)
{
	(void)status;

	if ((move_active == 1) &&
			(input->mouse_button == GLUT_LEFT_BUTTON) &&
			(input->mouse_state == GLUT_DOWN))
	{
		// Move o ponto de atencao na imagem esquerda para (input->wxd, input_RIGHT->wyd)
		move_left_input_center(in_left.wxd, in_right.wyd);

		update_filters_dendrites_neurons_and_outputs();
	}
	input->mouse_button = -1;
}

void
input_controller_right (INPUT_DESC *input, int status)
{
	(void)status;

	if ((move_active == 1) &&
			(input->mouse_button == GLUT_LEFT_BUTTON) &&
			(input->mouse_state == GLUT_DOWN))
	{
		// Move o ponto de atencao nas imagens esquerda e direita para (input->wxd, input->wyd)
		move_right_input_center(in_right.wxd, in_right.wyd);
		move_left_input_center(in_right.wxd, in_right.wyd);
		update_input_image(&in_left);
		update_filters_dendrites_neurons_and_outputs();
	}
	input->mouse_button = -1;
}

double
mae_uniform_random(double min, double max)
{
  return min + (rand() / (double)RAND_MAX) * (max - min);
}

double
mae_gaussian_random(double mean, double std)
{
  const double norm = 1.0 / (RAND_MAX + 1.0);
  double u = 1.0 - rand() * norm;                  /* can't let u == 0 */
  double v = rand() * norm;
  double z = sqrt(-2.0 * log(u)) * cos(2.0 * M_PI * v);

  return mean + std * z;
}

/* Sets the neuron layer band with an integer constant value */
void
set_neuron_layer_value (NEURON_LAYER *neuron_layer, int activation_value)
{
	int u, v, w, h;

	w = neuron_layer->dimentions.x;
	h = neuron_layer->dimentions.y;

	for(u = 0; u < w; u++)
		for(v = 0;v < h; v++)
			neuron_layer->neuron_vector[u + w * v].output.fval =
					mae_uniform_random(activation_value-DISPARITY_ERROR, activation_value+DISPARITY_ERROR);
//					mae_gaussian_random(activation_value, DISPARITY_ERROR);
}

void
vergence_train(int x, int y)
{
	int i;

	network_state = TRAINNING;

	clear_neural_layers_memory ("nl_v1_activation_map");

	for (i = 0; i < MAX_DISP_NUM ; i++)
	{
		move_left_input_center(x+i, y);
		update_input_image(&in_left);
		update_filters_and_outputs();

		set_neuron_layer_value(&nl_v1_activation_map, i);

		train_neuron_layer((char*)"nl_v1_activation_map");
	}

	move_left_input_center(x, y);
	update_input_image(&in_left);

	network_state = RUNNING;

	update_filters_dendrites_neurons_and_outputs();
}

void
vergence_saccade ()
{
	in_left.wxd = in_right.wxd + (int) (nl_target_coordinates.neuron_vector[0].output.fval + 0.5);

	move_left_input_center(in_left.wxd, in_left.wyd);

	update_filters_dendrites_neurons_and_outputs();

	update_input_image(&in_left);
}

// Keyboard actions are still acceptable
void f_keyboard (char *key_value)
{
	char key;

	switch (key = key_value[0])
	{
	// Train network
	case 'T':
	case 't':
		vergence_train(in_right.wxd, in_right.wyd);
		break;
		// Vergence until reach the target
	case 'S':
	case 's':
		vergence_saccade();
		break;
	case 'V':
	case 'v':
		vergence_train(in_right.wxd, in_right.wyd);
		vergence_saccade();
		break;
	}

	return;
}

NEURON_OUTPUT
run_vergence(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	vergence_train(in_right.wxd, in_right.wyd);

	vergence_saccade();

	output.ival = 0;

	return (output);
}

NEURON_OUTPUT
get_disparity_computed(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.fval = nl_target_coordinates.neuron_vector[0].output.fval;

	return (output);
}

NEURON_OUTPUT
get_disparity_benchmark(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int w, h, s, x, y;

	output.fval = 0.0;
	if (pParamList && pParamList->next && pParamList->next->next && pParamList->next->next)
	{
		x = pParamList->next->param.ival;
		y = pParamList->next->next->param.ival;
		s = pParamList->next->next->next->param.ival;

		w = in_disparity.neuron_layer->dimentions.x;
		h = in_disparity.neuron_layer->dimentions.y;

		if (x >= 0 && x < w && y >= 0 && y < h)
		{
			output.fval  = (float) in_disparity.neuron_layer->neuron_vector[x + w * y].output.ival;
			output.fval /= (float) s;
		}
	}

	return (output);
}

NEURON_OUTPUT
set_disparity_output(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int w, h, s, x, y;
	float d;

	output.ival = 0;
	if (pParamList && pParamList->next && pParamList->next->next && pParamList->next->next->next && pParamList->next->next->next->next)
	{
		x = pParamList->next->param.ival;
		y = pParamList->next->next->param.ival;
		s = pParamList->next->next->next->param.ival;
		d = pParamList->next->next->next->next->param.fval;

		w = nl_disparity.dimentions.x;
		h = nl_disparity.dimentions.y;

		if (x >= 0 && x < w && y >= 0 && y < h)
		{
			nl_disparity.neuron_vector[x + w * y].output.fval = (float)s * d;
		}
	}

	return (output);
}

NEURON_OUTPUT
move_left_eye(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.ival = -1;

	if (pParamList && pParamList->next && pParamList->next->next)
	{
		in_left.wxd = pParamList->next->param.ival;
		in_left.wyd = pParamList->next->next->param.ival;

		move_left_input_center(in_left.wxd, in_left.wyd);
		update_input_image(&in_left);

		output.ival = 0;
	}

	return (output);
}

NEURON_OUTPUT
move_right_eye(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	output.ival = -1;

	if (pParamList && pParamList->next && pParamList->next->next)
	{
		in_right.wxd = pParamList->next->param.ival;
		in_right.wyd = pParamList->next->next->param.ival;

		move_right_input_center(in_right.wxd, in_right.wyd);
		update_input_image(&in_right);

		output.ival = 0;
	}

	return (output);
}
