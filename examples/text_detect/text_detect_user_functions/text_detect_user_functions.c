#include "text_detect_user_functions.h"
#include "text_detect_files.h"

// Global Variables
static int g_current_input_image_index = 0;
static int g_current_output_image_index = 0;

void text_detect_keyboard (unsigned char key, int x, int y);
/*
***********************************************************
* Function: make_input_image_text_detect
* Description:
* Inputs:
* Output: none
***********************************************************
*/

void make_input_image_text_detect (INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

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
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}

	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if(input->image == NULL)
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
}




/*
***********************************************************
* Function: init_face_recog
* Description:
* Inputs:input - Neuron Layer de input
* Output: none
***********************************************************
*/

void init_text_detect(INPUT_DESC *input)
{
	#ifndef NO_INTERFACE
	int x, y;
	#endif

	make_input_image_text_detect (input, INPUT_WIDTH, INPUT_HEIGHT);

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
	glutKeyboardFunc (text_detect_keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
	#endif
}



/*
***********************************************************
* Function: init_user_functions
* Description:
* Inputs:none
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int init_user_functions (void)
{
	char strCommand[128];

	sprintf(strCommand, "toggle move_active;");
	interpreter(strCommand);

	sprintf(strCommand, "toggle draw_active;");
	interpreter(strCommand);

	return (0);
}


/*
***********************************************************
* Function: input_generator
* Description:
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void input_generator (INPUT_DESC *input, int status)
{
	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_text_detect(input);
		#ifndef NO_INTERFACE
		input->win = glutGetWindow();
		#endif
 	}
	else
	{
		if (status == MOVE)
		{
			load_image(input->name, g_current_input_image_index);
			update_input_neurons(input);
			update_input_image(input);

			#ifndef NO_INTERFACE
			glutSetWindow (input->win);
			input_display ();
			#endif
		}
	}
}



/*
***********************************************************
* Function: draw_output
* Description:Draws the output layer
* Inputs: strOutputName -
*	  strInputName -
* Output: None
***********************************************************
*/

void draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;

	output = get_output_by_name(strOutputName);

	load_image(output->name, g_current_output_image_index);

	#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay(output->win);
	#endif
}



/*
***********************************************************
* Function: input_controler
* Description: handles the mouse input window events
* Inputs: input -
*	  status -
* Output: None
***********************************************************
*/

void input_controler (INPUT_DESC *input, int status)
{
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
	}

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
	}

	input->mouse_button = -1;
}



/*
***********************************************************
* Function: output_handler
* Description:
* Inputs:
* Output:
***********************************************************
*/

void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display (output);
	#endif
}


// ----------------------------------------------------
// f_keyboard - Funcao chamada quando e pressionada uma
//				tecla.
//
// Entrada: key_value - ponteiro para o cacacter pressionado
//
// Saida: Nenhuma
//
// Obs: Esta função não está sendo usada pois o interpretador
//      da MAE não suporta eventos aninhados. Uma tecla é um comando!
// ----------------------------------------------------
void f_keyboard (char *key_value)
{
}

void text_detect_keyboard (unsigned char key, int x, int y)
{
	char strInputCommand[] = "move in_text_detect to 0, 0;";
	char strOutputCommand[] = "draw out_text_detect based on text_detect move;";

	switch (key)
	{
	case 'n':
		g_current_input_image_index++;
		interpreter (strInputCommand);
		break;
	case 'p':
		if (g_current_input_image_index > 0)
			g_current_input_image_index--;
		interpreter (strInputCommand);
		break;
	case 'N':
		g_current_output_image_index++;
		interpreter (strOutputCommand);
		break;
	case 'P':
		if (g_current_output_image_index > 0)
			g_current_output_image_index--;
		interpreter (strOutputCommand);
		break;
	case 'Q':
	case 'q':
		exit(0);
		break;
	}
}

