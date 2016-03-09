#include <stdio.h> 
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mae.h"

#if !(defined(NO_INTERFACE) || defined(WINDOWS))
#include "forms.h"
#include "mae_control.h"
#endif

void
save_script (char *file_name)
{
	FILE *file;
	COMMAND_LIST *cmd_list;
	
	if ((file = fopen (file_name, "w")) == NULL)
	{
		show_message ("Could not create file ", file_name, "");
		return;
	}
		
	for (cmd_list = command_list; cmd_list != NULL; cmd_list = cmd_list->next)
	{
		if (fprintf (file, "%s\n", cmd_list->command) < strlen (cmd_list->command) + 1)
		{
			show_message ("Could not save all commands in ", file_name, "(Disk full?)");
			return;
		}
	}
	fclose (file);
}


int
get_file_size (char *file_name)
{
	FILE *command_file;
	int size = 0;

	if ((command_file = fopen (file_name, "r")) == NULL)
	{
		show_message ("Could not open file ", file_name, "");
#ifndef NO_INTERFACE
		glutIdleFunc ((void (* ) (void)) check_forms);
#endif
		return (0);
	}
	
	while (fgetc(command_file) != EOF)
		size++;
	
	fclose (command_file);
	return (size);
}
              

void
execute_script (char *file_name)
{
	int file_size;
	char *commands;
	FILE *command_file;
	
	file_size = get_file_size (file_name);
	if (file_size == 0)
	{
		show_message ("File ", file_name, " is empty");
#ifndef NO_INTERFACE
		glutIdleFunc ((void (* ) (void)) check_forms);
#endif
		return;
	}
	
	commands = (char *) alloc_mem (file_size + 1);
	
	if ((command_file = fopen (file_name, "r")) == NULL)
	{
		show_message ("Could not open file ", file_name, "");
#ifndef NO_INTERFACE
		glutIdleFunc ((void (* ) (void)) check_forms);
#endif
		return;
	}

	if (fread (commands, 1, file_size, command_file) != file_size)
	{
		show_message ("Could not read file ", file_name, "");
#ifndef NO_INTERFACE
		glutIdleFunc ((void (* ) (void)) check_forms);
#endif
		fclose (command_file);
		return;
	}
	else
		fclose (command_file);
	
	commands[file_size] = '\0';
	//interprete_many_commands_in_a_file (commands, file_name);
	interpreter (commands);
	free (commands);
	
#ifndef NO_INTERFACE
	glutIdleFunc ((void (* ) (void)) check_forms);
#endif
}

void
set_window_position_config_name(char *config)
{
	char *app_path = strdup(global_argv[0]);
	char *app_name = basename(app_path);
	sprintf(config, ".wp_%s", app_name);
	printf("%s\n", config);
	free(app_path);
}

int
read_window_position (char *window_name, int *x, int *y)
{
	FILE *wp;
	char buf[256];
	char window_name_wp[256];
	char config_name_wp[256];
	
	set_window_position_config_name(config_name_wp);
	if ((wp = fopen (config_name_wp, "r")) == NULL)
	{
		printf ("Erro: Could not open file .wp for reading\n");
		return (0);
	}

	while (fgets (buf, 256, wp) != NULL)
	{
		sscanf (buf, "%s %d %d\n", window_name_wp, x, y);
		if (strcmp (window_name, window_name_wp) == 0)
		{
			if (fclose (wp) != 0)
			{
				printf ("Erro: Could close file .wp\n");
				return (0);
			}
			else
				return (1);
		}
	}

	if (fclose (wp) != 0)
		printf ("Erro: Could close file .wp\n");
		return (0);
}


#ifndef NO_INTERFACE
void
write_window_position (FILE *wp, char *window_name, int x, int y)
{
	fprintf (wp, "%s %d %d\n", window_name, x, y);
}
#endif

#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void
save_windows_positions ()
{
	INPUT_LIST *input;
	OUTPUT_LIST *output;
	int x, y;
	FILE *wp;
	char config_name_wp[256];

	set_window_position_config_name(config_name_wp);

	if ((wp = fopen (config_name_wp, "w")) == NULL)
	{
		printf ("Erro: Could not create file .wp for writting\n");
		return;
	}

	for (input = global_input_list; input != (INPUT_LIST *) NULL; input = input->next)
	{
		glutSetWindow (input->input->win);
		x = glutGet (GLUT_WINDOW_X) - decoration_size_x;
		y = glutGet (GLUT_WINDOW_Y) - decoration_size_y;
		write_window_position (wp, input->input->name, x, y);
	}

	for (output = global_output_list; output != (OUTPUT_LIST *) NULL; output = output->next)
	{
		glutSetWindow (output->output->win);
		x = glutGet (GLUT_WINDOW_X) - decoration_size_x;
		y = glutGet (GLUT_WINDOW_Y) - decoration_size_y;
		write_window_position (wp, output->output->name, x, y);
	}
	
	x = (int) (mae_control->x - decoration_size_x);
	y = (int) (mae_control->y - decoration_size_y);
	write_window_position (wp, "mae_control_window", x, y);

	if (fclose (wp) != 0)
		printf ("Erro: Could close file .wp\n");
}
#endif


#if !(defined(NO_INTERFACE) || defined(WINDOWS))
void callback_file(FL_OBJECT *ob, long data)
{
  	int item;
  	const char *aux;
	char file_name[256];
		
	item = fl_get_menu (ob);
	switch (item)
	{
		case 1: /* Option Save Command List */
 		aux = fl_show_file_selector ("File to Save","","*.cml","");
  		if (aux != NULL)
  		{
  			aux = strrchr (aux, '/');
  			strcpy (file_name, ++aux);
			save_script (file_name);
 		}
		break;
		
		case 2: /* Option Execute Command List */
 		aux = fl_show_file_selector ("File to Execute","","*.cml","");
  		if (aux != NULL)
  		{
  			aux = strrchr (aux, '/');
  			strcpy (file_name, ++aux);
			execute_script (file_name);
 		}
		break;
			
	  	case 3: /* Option Quit selected */
  		if (fl_show_question ("Do you want to Quit?", 0))
  		{
			save_windows_positions ();
			print_measuraments ();
  			fl_finish ();
  			exit (0);
  		}
		break;
  	}
}
#endif


void
run_cmdl_script (void)
{
	execute_script (cmdl_script);
}
