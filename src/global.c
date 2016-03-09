/*
*********************************************************************************
* Module : General functions and global variables				*
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <sys/time.h>
#include "mae.h"


/*
*********************************************************************************
* Global variables								*
*********************************************************************************
*/

#ifdef FAT_FAST_HASH_SIZE_AUTO
int SUB_PATTERN_UNIT_SIZE;
#endif

int	NEURON_MEMORY_INDEX_SIZE = 5;
int	NEURON_MEMORY_SIZE = 32;
int	N_SYNAPSES = 8;

int	DUMMY_OUTPUT_ACTIVATION_VALUE = -1;			// dummy value

long   NUM_ISOLATE_NEURON_LAYERS;
long   NUM_INPUTS;
long   NUM_OUTPUTS;
long   NUM_FILTERS = 1;

NEURON_LAYER_LIST *global_neuron_layer_list;
INPUT_LIST *global_input_list;
OUTPUT_LIST *global_output_list;
FILTER_LIST *global_filter_list;
FUNCTION_LIST *global_function_list = NULL;
FILTER_DESC *global_filter_desc = (FILTER_DESC *) NULL;
FUNCTION_LIST *functions = NULL;

float *neuron_outputs;
FILE *neural_connections;
NEURON_LAYER **n_layer_vec;
int n_inputs = 0;
int n_outputs = 0;
int n_n_layers = 0;
int total_n_neurons = 0;
int n_connections_per_neuron;
int n_patterns;
int __line = 1;


int interpreter_pipe[2];
FILE *interpreter_pipe_in;
FILE *interpreter_pipe_out;

char status_info[256];
INPUT_DESC *selected_input = NULL;
OUTPUT_DESC *selected_output = NULL;

COMMAND_LIST *command_list = NULL;
COMMAND_LIST *command_list_tail = NULL;

int wx_saved;
int wy_saved;
int x_saved;
int y_saved;

NEURON neuron_with_output_zero = {{0}, {0}, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0};

char *cmdl_script;

int decoration_size_x, decoration_size_y;

NEURON_TYPE *default_neuron_type;

PATTERN pattern_xor_table[256];
//PATTERN pattern_xor_table[65536];	//for using 16-bit counting based tables

MEASURAMENTS measuraments;

unsigned int g_seed = 1;		//the seed for the mae_fastrand() function

// Vergence global variables
double SELECTIVITY = 0.05;

double RANGE = 3.0 * pi;

double SIGMA = 5.0 * pi / 6.0;

double FOCAL_DISTANCE = 2000.0;

double CAMERA_DISTANCE = 7.0;

double CUT_POINT = 30.0;

int KERNEL_SIZE = 15;

double threshold = 0.0;

int delta_x = 0;

// Decide whether call update functions on initialize or not
// TODO: this can cause context errors !
int CALL_UPDATE_FUNCTIONS = 1;

// Show types
int TYPE_SHOW = SHOW_WINDOW;

int TYPE_MOVING_FRAME = MOVING_CENTER;

// State variables
int remaining_steps = 0;

int move_active = 0;
int draw_active = 0;

//double delay;
unsigned int delay = 0;

int running = 0;

BOOLEAN hide_interface = FALSE;

int PYRAMID_LEVEL = 0;

int VERGENCE_SCAN_TYPE = LINEAR_SCAN;

double LOG_POLAR_SCALE_FACTOR = 1.0;

double LOG_POLAR_THETA_CORRECTION = 0.0;


/*
*********************************************************************************
* Initialize measuraments							*
*********************************************************************************
*/

void
init_measuraments (void)
{
	int i;
	
	measuraments.neuron_mem_access = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++)
		measuraments.neuron_mem_hits_at_distance[i] = 0;
}


/*
*********************************************************************************
* Initialize measuraments							*
*********************************************************************************
*/

void
print_measuraments (void)
{
	int i;
	int total;
	
	printf ("neuron_mem_access = %d\n", measuraments.neuron_mem_access);
	for (i = total = 0; i < MAX_CONNECTIONS; i++)
	{
		printf ("neuron_mem_hits_at_distance %d = %f\n", i, 
		        (float) measuraments.neuron_mem_hits_at_distance[i] / (float) measuraments.neuron_mem_access);
		total += measuraments.neuron_mem_hits_at_distance[i];
		if (total == measuraments.neuron_mem_access)
			break;
	}
}



/*
*********************************************************************************
* Initialize pattern_xor_table							*
*********************************************************************************
*/

void
init_pattern_xor_table (void)
{
	int i, bit;
	
	if(PATTERN_UNIT_SIZE == 16)
	{
		for (i = 0; i < 65536; i++)
		{
			pattern_xor_table[i] = 0;
			for (bit = 0; bit < PATTERN_UNIT_SIZE; bit++)
			{
				if ((i >> bit) & 1)
					pattern_xor_table[i] += 1;
			}
		}
	}
	else /* if(PATTERN_UNIT_SIZE == 8) */
	{
		for (i = 0; i < 256; i++)
		{
			pattern_xor_table[i] = 0;
			for (bit = 0; bit < PATTERN_UNIT_SIZE; bit++)
			{
				if ((i >> bit) & 1)
					pattern_xor_table[i] += 1;
			}
		}
	}
}



/*
*********************************************************************************
* Notify error and terminate							*
*********************************************************************************
*/

void
Erro (char *msg1, char *msg2, char *msg3)
{
	fprintf (stderr, "line %d - %s%s%s\n", __line, msg1, msg2, msg3);
	exit (1);
}



/*
*********************************************************************************
* Alloc memory at building time							*
*********************************************************************************
*/

void *
alloc_mem (size_t size)
{    
	void *pointer;

	if ((pointer = calloc (1, size)) == (void *) NULL)
		Erro ("cannot alloc more memory.", "", "");
	return (pointer);
}
 
/*
*********************************************************************************
* Reallocate memory								*
*********************************************************************************
*/

void *
realloc_mem (size_t size, void *ptr)
{    
	void *pointer;

	if ((pointer = realloc (ptr,size)) == (void *) NULL)
	{
		show_message ("Fatal Error.", "Can not realloc more memory.", "");
		exit (1);
	}
	return (pointer);
}

/*
*********************************************************************************
* Alloc memory									*
*********************************************************************************
*/

void *
alloc_mem_nn (size_t size)
{    
	void *pointer;

	if ((pointer = calloc (1, size)) == (void *) NULL)
	{
		show_message ("Fatal Error.", "Can not malloc more memory.", "");
		exit (1);
	}
	return (pointer);
}

/* 
********************************************************************************* 
* Concatenate many strings							* 
********************************************************************************* 
*/
 
char * 
C (char *number, ...) 
{
	va_list argument;
	char *string, *aux, *result;
	int i;
	
	va_start (argument, number);
	result = (char *) alloc_mem_nn (1);
	strcpy (result, "");
	for (i = 0; i < atoi (number); i++)
	{
		string = va_arg (argument, char *);
		if ((aux = (char *) alloc_mem_nn (strlen (string) + strlen (result) + 1)) == NULL)
		{
			printf ("cannot allocate more memory.\n"); 
			exit (1);
		}
		strcpy (aux, result);
		strcat (aux, string);
		free (result);
		result = aux;
	}
	va_end (argument);
	return (result);
} 



/* 
********************************************************************************* 
* Return Param List Based on Describer String of Parameters 			* 
********************************************************************************* 
*/ 

PARAM_LIST *
get_param_list (char *parameters, va_list *argument)
{
	PARAM_LIST *param_list = NULL;
	PARAM_LIST *param_list_head = NULL;
	char *p;

	p = (char *) alloc_mem (strlen (parameters) + 1);
	strcpy (p, parameters);
	
	param_list = param_list_head = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
	param_list->param.sval = p;
	param_list->next = NULL;
		
	while (*p != '\0')
	{			
		switch (*p)
		{
			case 'd': /* Integer */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.ival = va_arg (*argument, int);
				param_list->next->next = NULL;
				param_list = param_list->next;
				break;
			case 'f': /* Float */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.fval = (float) va_arg (*argument, double);
				param_list->next->next = NULL;
				param_list = param_list->next;
				break;
			case 'p': /* Pointer */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.pval= va_arg (*argument, void*);
				param_list->next->next = NULL;
				param_list = param_list->next;
				break;
			case 's': /* String */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.sval = va_arg (*argument, char*);
				param_list->next->next = NULL;
				param_list = param_list->next;
				break;
			case '%': /* Percent */
				break;
			case ' ': /* Space */
				break;
			default:
				Erro ("Invalid string in function parameter descriptor\n", 
				      "function parameter strings can only contain \'%\', \' \', \'d\', \'f\', \'p\' or \'s\'", "");
		}
		p++;			
	}

	return (param_list_head);
}



/* 
********************************************************************************* 
* Add a interpreter user function to the global function list			* 
********************************************************************************* 
*/ 

void 
create_interpreter_user_function (int return_type, NEURON_OUTPUT (* function) (PARAM_LIST *), char *function_name, char *parameters)
{
	FUNCTION_LIST *n_list;
	
	if (global_function_list == (FUNCTION_LIST *) NULL)
	{
		n_list = (FUNCTION_LIST *) alloc_mem ((size_t) sizeof (FUNCTION_LIST));
		n_list->next = (FUNCTION_LIST *) NULL;
		
		n_list->return_type = return_type;
		
		n_list->function = function;

		n_list->function_name = (char *) alloc_mem (strlen (function_name) + 1);
		strcpy (n_list->function_name, function_name);
		
		n_list->parameters = (char *) alloc_mem (strlen (parameters) + 1);
		strcpy (n_list->parameters, parameters);
		
		global_function_list = n_list;
	}
	else
	{
		for (n_list = global_function_list; n_list->next != (FUNCTION_LIST *) NULL; n_list = n_list->next)
		{
			if (n_list->function == function)
				Erro ("two instances of the same function in global_function_list.", "", "");
		}
		
		if (n_list->function == function)
			Erro ("two instances of the same function in global_function_list.", "", "");
			
		n_list->next = (FUNCTION_LIST *) alloc_mem ((size_t) sizeof (FUNCTION_LIST));
		n_list->next->next = (FUNCTION_LIST *) NULL;
		
		n_list->next->return_type = return_type;
		
		n_list->next->function = function;
		
		n_list->next->function_name = (char *) alloc_mem (strlen (function_name) + 1);
		strcpy (n_list->next->function_name, function_name);
		
		n_list->next->parameters = (char *) alloc_mem (strlen (parameters) + 1);
		strcpy (n_list->next->parameters, parameters);
	}
}

double
get_time(void)
{
    double start_time, end_time;
    static int started = 0;
    static struct timeval start;
    struct timeval end;

    if (!started)
    {
        gettimeofday(&start,NULL);
        started = 1;
    }

    gettimeofday(&end,NULL);

    start_time = (double) start.tv_sec*1000.0 + (double) start.tv_usec/1000.0;
    end_time   = (double) end.tv_sec*1000.0   + (double) end.tv_usec/1000.0;

    return end_time - start_time;
}
