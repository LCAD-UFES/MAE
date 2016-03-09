/*! \mainpage 
*
* \section introduction Introduction
*
* MAE is a tool for developing artificial neural architectures based on weightless neuron networks.
*
* \section installation Installation
* 
* The installation can be made in the two steps described in the following subsections.
* 
* \subsection installation1 Step 1: Get the code from the cvs server.
*  
* \subsection installation2 Step 2: Use make to generate the MAE library.
*  
* \section neuron_layers Neuron Layers
*
* Neuron layers are the most basic structure of MAE.
* etc...
*/


#include "mae.h"
#include <math.h>

#if !(defined(NO_INTERFACE) || defined(WINDOWS))
#include "forms.h"
#include "mae_control.h"
#endif

int 	global_argc;	//global argc	
char	**global_argv;	//global argv

int
main (int argc, char *argv[])
{
	global_argc = argc;
	global_argv = argv;

#ifndef NO_INTERFACE
#ifndef WINDOWS
	FL_Coord x, x_box, y, y_box;
	Window win;
	
	fl_set_border_width (-2);

   	fl_initialize(&argc, argv, 0, 0, 0);
#endif

	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
#ifndef WINDOWS
   	create_the_forms();
	fl_set_menu (file, "Save Command List|Execute Command List|Quit");
#endif
#endif
	

#if !(defined(NO_INTERFACE) || defined(WINDOWS))
   	if (read_window_position ("mae_control_window", &x, &y))
	{
		fl_set_form_position (mae_control, x, y);
		fl_show_form (mae_control,FL_PLACE_GEOMETRY,FL_FULLBORDER,argv[0]);
	}
	else
	{
		x = 50;
		y = 50;
		fl_set_form_position (mae_control, x, y);
		fl_show_form (mae_control,FL_PLACE_GEOMETRY,FL_FULLBORDER,argv[0]);
	}

	win = FL_ObjWin (main_box);
	fl_get_winorigin (win, &x_box, &y_box);
	decoration_size_x = x_box - x;
	decoration_size_y = y_box - y;
	
	fl_set_input (steps_input, "5");
	strcpy (status_info, "");
#endif

#if !defined (__SSE4_2__) && !defined (__ABM__)
	init_pattern_xor_table ();
#endif
	
	init_measuraments ();
	printf ("# Please wait, building network...\n");
	build_network ();
	running = 1;
	if (CALL_UPDATE_FUNCTIONS)
	{
		all_inputs_update ();
		all_filters_update ();
		all_dendrites_update (); 
		all_neurons_update ();
		all_outputs_update (); 
	}
  
	running = 0;

	if (init_user_functions ())
		Erro ("Fail initializing user functions.", "init_user_functions () returned 1", "");
	
#ifndef NO_INTERFACE
	check_forms ();

	if (argc == 2)
	{
		cmdl_script = argv[1];
		glutIdleFunc ((void (* ) (void)) run_cmdl_script);
	}
	else
		glutIdleFunc ((void (* ) (void)) check_forms);

   	glutMainLoop ();
#else
	if (argc == 2)
	{
		cmdl_script = argv[1];
		run_cmdl_script ();
	}
#endif
   	return (0);
}

