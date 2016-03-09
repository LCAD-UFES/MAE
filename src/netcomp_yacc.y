 /*
*********************************************************************************
* Module : Biological Neural Network language descrition syntax implementation  *
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#if !defined(__APPLE__)
#include <malloc.h> 
#endif
#include <ctype.h>
#include "mae.h"

#define STRING_LENGTH	256

extern int tokval;
extern int lineno;
extern char line_info[];

SYMBOL_LIST *integers = NULL;	
SYMBOL_LIST *floats = NULL;
SYMBOL_LIST *consts = NULL;
SYMBOL_LIST *macros = NULL;
SYMBOL_LIST *s_list = NULL;
SYMBOL_LIST *neuron_layers = NULL;
SYMBOL_LIST *neuron_types = NULL;
SYMBOL_LIST *inputs = NULL;
SYMBOL_LIST *outputs = NULL;
SYMBOL_LIST *filters = NULL;
SYMBOL_LIST *input_functions = NULL;
SYMBOL_LIST *output_functions = NULL;
SYMBOL_LIST *filter_functions = NULL;
SYMBOL_LIST *user_functions = NULL;

int __num_isolate_neuron_layers = 0;
int __num_inputs = 0;
int __num_outputs = 0;
int __num_filters = 0;
int __num_layers = 0;

char __char_num_layers[10];
char __char_num_params[10];

void add_symbol (char *symbol, SYMBOL_LIST **list);
char *C (char *number, ...);
int list_member (char *symbol, SYMBOL_LIST *list);
void yyerror (char *mens);

FILE *file;
char header_name[STRING_LENGTH];
char source_name[STRING_LENGTH];
char define_directive[STRING_LENGTH];

%}

%union 
{
	char *cpval;
}

%start program

%token	INT
%token	FLOAT
%token	CONST
%token	RADIUS
%token	RADII
%token	ANGLE
%token	LOG_FACTOR
%token	SCALE_FACTOR
%token	LINEAR
%token	SIZE
%token	SQUARED
%token	DISTRIBUTION
%token	GAUSSIAN_TOKEN
%token	LOG_POLAR_TOKEN
%token	LOG_POLAR_SCALED_TOKEN
%token	LAPLACIAN_TOKEN
%token	REGULAR_TOKEN
%token	AND
%token	INPUT
%token	OUTPUT
%token	OUTPUTS
%token	NEURON_LAYER_TOKEN
%token	OF
%token	NEURONS
%token	FILTER
%token	PRODUCING
%token	WITH
%token	RANDOM
%token	INPUTS
%token	PER
%token	NEURON_TOKEN
%token	USING
%token	SHARED
%token	MEMORY
%token	ASSOCIATE
%token	WITHIN
%token	CONNECT
%token	SAME
%token	INTERCONNECTION
%token	PATTERN_TOKEN
%token	FULL_CONNECT
%token	OUTPUT_CONNECT
%token	TO
%token	WHILE
%token	FOR
%token	SET
%token	SENSITIVE
%token	COLOR_TOKEN
%token	COLOR_SELECTIVE_TOKEN
%token	GREYSCALE_TOKEN
%token	GREYSCALE_FLOAT_TOKEN
%token	BLACK_WHITE_TOKEN
%token	PRODUCED
%token	CONTROLED
%token	BY
%token	HANDLED
%token	OPEN_PARENTESIS
%token	CLOSE_PARENTESIS
%token	OPEN_CHAVES
%token	CLOSE_CHAVES
%token	OPEN_COLCHETES
%token	CLOSE_COLCHETES
%token	SEMI_COLON
%token	COMA
%token	GREATER
%token	LESS
%token	EQUAL
%token	DIFERENT
%token	GREATER_EQUAL
%token	LESS_EQUAL
%token	PLUS
%token	MINUS
%token	TIMES
%token	DIVIDIDED
%token	<cpval> CONSTANT
%token	<cpval> VARIABLE
%token	<cpval> STRING
%token	PYRAMID_TOKEN
%token	COPIES
%token	SHOW
%token	AT

%type	<cpval>	series
%type	<cpval>	command
%type	<cpval>	statement
%type	<cpval>	input_st
%type	<cpval>	output_st
%type	<cpval>	neuronlayer_st
%type	<cpval>	filter_st
%type	<cpval>	assignment_st
%type	<cpval>	association_st
%type	<cpval>	interconnection_pattern
%type	<cpval>	memory_type_st
%type	<cpval>	connection_st
%type	<cpval>	full_connection_st
%type	<cpval>	output_connection_st
%type	<cpval>	indefinite_loop
%type	<cpval>	definite_loop
%type	<cpval>	set_st
%type	<cpval>	function_st
%type	<cpval>	sensitivity
%type	<cpval>	output_type
%type	<cpval>	pyramid_type
%type	<cpval>	o_type
%type	<cpval>	function
%type	<cpval>	params_list
%type	<cpval>	params
%type	<cpval>	param
%type	<cpval>	layers
%type	<cpval>	layer
%type	<cpval>	ranges
%type	<cpval>	dimentions
%type	<cpval>	comparison
%type	<cpval>	string
%type	<cpval>	expression

%left PLUS MINUS
%left TIMES DIVIDIDED
%left UMINUS

%%
program		:	series 
		{
			// Generates the header file
			file = fopen (header_name, "w");
			
			fprintf (file, "// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY\n\n");
			fprintf (file, "#ifndef %s\n", define_directive);
			fprintf (file, "#define %s\n", define_directive);

			fprintf (file, "\n// Includes\n");
			fprintf (file, "#include <stdio.h>\n");
			fprintf (file, "#include <stdlib.h>\n");
			fprintf (file, "#include \"mae.h\"\n");
			fprintf (file, "#include \"filter.h\"\n");
			
			fprintf (file, "\n// Definitions\n");
			for (s_list = consts; s_list != NULL; s_list = s_list->next)
				fprintf (file, "#define %s\n", s_list->symbol);
			
			fprintf (file, "\n// Macros\n");
			for (s_list = macros; s_list != NULL; s_list = s_list->next)
				fprintf (file, "#define %s\n", s_list->symbol);
			
			fprintf (file, "\n// Structs\n");

			// For C++ usage prototype definition (extern "C" defined)			
			fprintf (file, "\n// For avoiding symbol table errors on C++ linkage\n");
			//fprintf (file, "\n#ifdef __cplusplus\nextern \"C\"{\n");
			fprintf (file, "\n#ifdef __cplusplus\n");
			
			fprintf (file, "\n// Prototypes\n");
			
			for (s_list = input_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern \"C\" void %s (INPUT_DESC *, int status);\n", s_list->symbol);

			for (s_list = output_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern \"C\" void %s (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); \n", s_list->symbol);

			/*for (s_list = filter_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern void %s (NEURON_LAYER *, NEURON_LAYER_LIST *);\n", s_list->symbol);*/
			
			for (s_list = filter_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern \"C\" void %s (FILTER_DESC *);\n", s_list->symbol);
					
			for (s_list = neuron_types; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern \"C\" NEURON_TYPE %s;\n", s_list->symbol);
				
			for (s_list = user_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern \"C\" NEURON_OUTPUT %s (PARAM_LIST *);\n", s_list->symbol);
				
			fprintf (file, "\n// Global Variables\n");
			for (s_list = integers; s_list != NULL; s_list = s_list->next)
				fprintf (file, "extern \"C\" int %s;\n", s_list->symbol);
			
			for (s_list = floats; s_list != NULL; s_list = s_list->next)
				fprintf (file, "extern \"C\" float %s;\n", s_list->symbol);
			for (s_list = neuron_layers; s_list != NULL; s_list = s_list->next)
			{
				fprintf (file, "extern \"C\" NEURON_LAYER %s;\n", s_list->symbol);
				__num_isolate_neuron_layers++;
			}

			for (s_list = inputs, __num_inputs = 0; s_list != NULL; 
			     s_list = s_list->next)
			{
				fprintf (file, "extern \"C\" INPUT_DESC %s;\n", s_list->symbol);
				__num_inputs++;
			}

			for (s_list = outputs, __num_outputs = 0; s_list != NULL;
			     s_list = s_list->next)
			{
				fprintf (file, "extern \"C\" OUTPUT_DESC %s;\n", s_list->symbol);
				__num_outputs++;
			}
				
			// If C++ compilation is NOT-used - (simply define the Global variables and Prototypes as external).
			//fprintf (file, "\n}\n");
			fprintf (file, "\n#else\n");
			
			fprintf (file, "\n// Prototypes\n");
			
			for (s_list = input_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern void %s (INPUT_DESC *, int status);\n", s_list->symbol);

			for (s_list = output_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern void %s (OUTPUT_DESC *, int type_call, int mouse_button, int mouse_state); \n", s_list->symbol);

			/*for (s_list = filter_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern void %s (NEURON_LAYER *, NEURON_LAYER_LIST *);\n", s_list->symbol);*/
			
			for (s_list = filter_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern void %s (FILTER_DESC *);\n", s_list->symbol);
					
			for (s_list = neuron_types; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern NEURON_TYPE %s;\n", s_list->symbol);
				
			for (s_list = user_functions; s_list != NULL;
			     s_list = s_list->next)
				fprintf (file, "extern NEURON_OUTPUT %s (PARAM_LIST *);\n", s_list->symbol);
				
			fprintf (file, "\n// Global Variables\n");
			for (s_list = integers; s_list != NULL; s_list = s_list->next)
				fprintf (file, "int %s;\n", s_list->symbol);
			
			for (s_list = floats; s_list != NULL; s_list = s_list->next)
				fprintf (file, "float %s;\n", s_list->symbol);
			for (s_list = neuron_layers; s_list != NULL; s_list = s_list->next)
			{
				fprintf (file, "NEURON_LAYER %s;\n", s_list->symbol);
				__num_isolate_neuron_layers++;
			}

			for (s_list = inputs, __num_inputs = 0; s_list != NULL; 
			     s_list = s_list->next)
			{
				fprintf (file, "INPUT_DESC %s;\n", s_list->symbol);
				__num_inputs++;
			}

			for (s_list = outputs, __num_outputs = 0; s_list != NULL;
			     s_list = s_list->next)
			{
				fprintf (file, "OUTPUT_DESC %s;\n", s_list->symbol);
				__num_outputs++;
			}
			
			// If C++ conditional compilation terminator is used
			fprintf (file, "\n#endif\n\n");
			
			fprintf (file, "#endif\n");
			
			fclose (file);
	
			// Generates the source file	
			file = fopen (source_name, "w");
			
			fprintf (file, "// MAE 1.0 - THIS FILE WAS GERNERATED AUTOMATICALY\n\n");
			fprintf (file, "#include \"%s\"\n\n", header_name);

			fprintf (file, "void build_network (void)\n{\n");
			
			fprintf (file, "NUM_ISOLATE_NEURON_LAYERS = %d;\n", __num_isolate_neuron_layers);
			fprintf (file, "NUM_INPUTS = %d;\n", __num_inputs);
			fprintf (file, "NUM_OUTPUTS = %d;\n", __num_outputs);
			fprintf (file, "NUM_FILTERS = %d;\n", __num_filters);
			fprintf (file, "srand (5); \n");

			for (s_list = neuron_layers; s_list != NULL; s_list = s_list->next)
			{
				fprintf (file, "memset((void *) &(%s), 0, sizeof(NEURON_LAYER));\n", s_list->symbol);
				fprintf (file, "%s.name = \"%s\";\n", s_list->symbol, s_list->symbol);
			}

			for (s_list = inputs; s_list != NULL; s_list = s_list->next)
			{
				fprintf (file, "memset((void *) &(%s), 0, sizeof(INPUT_DESC));\n", s_list->symbol);
				fprintf (file, "%s.name = \"%s\";\n", s_list->symbol, s_list->symbol);
			}
			for (s_list = outputs; s_list != NULL; s_list = s_list->next)
			{
				fprintf (file, "memset((void *) &(%s), 0, sizeof(OUTPUT_DESC));\n", s_list->symbol);
				fprintf (file, "%s.name = \"%s\";\n", s_list->symbol, s_list->symbol);
			}
			fprintf (file, "\n%s", $1);
				
			fprintf (file, "map_layers2id (); \n");
			fprintf (file, "count_num_neurons (); \n");
   			fprintf (file, "initialise_memory (); \n");				// Memory initialise before GPU nl-list buildup
   			fprintf (file, "create_io_windows (); \n");				// I/O Window creation as a last action
			
			fprintf (file, "}\n");
			
			fclose (file);
			
			exit (0);
		}
	;
series		:	command 
		|	series command
		{
			$$ = C ("2", $1, $2);
		}
	;
command		:	statement SEMI_COLON
		{
			$$ = C ("3", line_info, $1, ";\n");
		}
		|	definite_loop 
		{
			$$ = C ("2", line_info, $1);
		}
	;
statement	:	input_st
		|	output_st
		|	neuronlayer_st
		|	filter_st
		|	association_st
		|	connection_st
		|	full_connection_st
		|	output_connection_st
		|	assignment_st
		|	indefinite_loop
		|	set_st
		|	function_st
		|	error
		{
			yyerrok;
			exit (1);
		}
	;
input_st	:	INPUT VARIABLE dimentions output_type pyramid_type PRODUCED BY function params CONTROLED BY function params 
		{
			$$ = C ("17", "create_input (&", $2, ", ", $3, ", ", $4, ", ", $5, ", ",
                                $8, ", ", $12, ", ", $9, ", ", $13, ")");
			add_symbol ($2, &inputs);
			add_symbol ($8, &input_functions);
			add_symbol ($12, &input_functions);
			free ($2);
		}
		|	INPUT VARIABLE dimentions output_type pyramid_type PRODUCED BY function params 
		{
			$$ = C ("13", "create_input (&", $2, ", ", $3, ", ", $4, ", ", $5, ", ",
                                $8, ", NULL, ", $9, ")");
			add_symbol ($2, &inputs);
			add_symbol ($8, &input_functions);
			free ($2);
		}
		|	INPUT VARIABLE dimentions output_type pyramid_type CONTROLED BY function params PRODUCED BY function params 
		{
			$$ = C ("17", "create_input (&", $2, ", ", $3, ", ", $4, ", ", $5, ", ",
                                $12, ", ", $8, ", ", $13, ", ", $9, ")");
			add_symbol ($2, &inputs);
			add_symbol ($8, &input_functions);
			add_symbol ($12, &input_functions);
			free ($2);
		}
		|	INPUT VARIABLE dimentions output_type pyramid_type CONTROLED BY function params 
		{
			$$ = C ("13", "create_input (&", $2, ", ", $3, ", ", $4, ", ", $5, 
				", NULL, ", $8, ", ", $9, ")");
			add_symbol ($2, &inputs);
			add_symbol ($8, &input_functions);
			free ($2);
		}
		|	INPUT VARIABLE dimentions output_type pyramid_type
		{
			$$ = C ("9", "create_input (&", $2, ", ", $3, ", ", $4, ", ", $5, 
				", NULL, NULL)");
			add_symbol ($2, &inputs);
			free ($2);
		}
	;
output_st	:	OUTPUT VARIABLE dimentions HANDLED BY function params
		{
			$$ = C ("9", "create_output (&", $2, ", ", $3, 
				", ", $6, ", ", $7, ")");
			add_symbol ($2, &outputs);
			add_symbol ($6, &output_functions);
			free ($2);
			free ($6);
		}
		|	OUTPUT VARIABLE dimentions 
		{
			$$ = C ("5", "create_output (&", $2, ", ", $3, 
				", NULL, 0)");
			add_symbol ($2, &outputs);
			free ($2);
		}
	;
memory_type_st :
		{
			$$ = "DISTRIBUTED_MEMORY" ;
		}
		|	USING SHARED MEMORY
		{
			$$ = "SHARED_MEMORY" ;
		}
	;
neuronlayer_st	:	NEURON_LAYER_TOKEN VARIABLE dimentions OF VARIABLE NEURONS sensitivity output_type memory_type_st COMA SHOW
		{
			$$ = C ("21", "create_neuron_layer (&", $2, ", &", $5, ", ", $7, ", ", $8, ", ", $3, 
			              ",",$9,");\ncreate_output (&", $2, "_out, ", $3, 
				      ", NULL, 0);\noutput_connect (&", $2, ", &", $2, "_out)");
			add_symbol ($2, &neuron_layers);
			add_symbol (C ("2", $2, "_out"), &outputs);
			add_symbol ($5, &neuron_types);
			free ($2);
			free ($5);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions OF VARIABLE NEURONS sensitivity output_type memory_type_st SIZE expression
		{
			$$ = C ("15", "create_neuron_layer (&", $2, ", &", $5, ", ", $7, ", ", $8, ", ", $3, ", ", $9, ", ", $11, ")");
			add_symbol ($2, &neuron_layers);
			add_symbol ($5, &neuron_types);
			free ($2);
			free ($5);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions OF VARIABLE NEURONS sensitivity output_type memory_type_st
		{
			$$ = C ("15", "create_neuron_layer (&", $2, ", &", $5, ", ", $7, ", ", $8, 
				", ", $3, ",",$9, ", ", "NEURON_MEMORY_SIZE", ")");
			add_symbol ($2, &neuron_layers);
			add_symbol ($5, &neuron_types);
			free ($2);
			free ($5);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions OF VARIABLE NEURONS output_type memory_type_st COMA SHOW
		{
			$$ = C ("23", "create_neuron_layer (&", $2, ", &", $5, ", ", "NOT_SPECIFIED", ", ", $7, ", ", $3, 
			              ",", $8, ", ", "NEURON_MEMORY_SIZE", ");\ncreate_output (&", $2, "_out, ", $3, 
				      ", NULL, 0);\noutput_connect (&", $2, ", &", $2, "_out)");
			add_symbol ($2, &neuron_layers);
			add_symbol ($5, &neuron_types);
			add_symbol (C ("2", $2, "_out"), &outputs);
			free ($2);
			free ($5);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions OF VARIABLE NEURONS output_type memory_type_st
		{
			$$ = C ("15", "create_neuron_layer (&", $2, ", &", $5, ", ", "NOT_SPECIFIED", ", ", $7, 
				 ", ", $3, ",", $8, ", ", "NEURON_MEMORY_SIZE", ")");
			add_symbol ($2, &neuron_layers);
			add_symbol ($5, &neuron_types);
			free ($2);
			free ($5);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions sensitivity output_type memory_type_st COMA SHOW
		{
			$$ = C ("21", "create_neuron_layer (&", $2, ", NULL, ", $4, ", ", $5, ", ", $3, 
			              ",",$6, ", ", "NEURON_MEMORY_SIZE", ");\ncreate_output (&", $2, "_out, ", $3, 
				      ", NULL, 0);\noutput_connect (&", $2, ", &", $2, "_out)");
			add_symbol ($2, &neuron_layers);
			add_symbol (C ("2", $2, "_out"), &outputs);
			free ($2);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions sensitivity output_type memory_type_st
		{
			$$ = C ("13", "create_neuron_layer (&", $2, ", NULL, ", $4, ", ", $5, 
				", ", $3, ",", $6, ", ", "NEURON_MEMORY_SIZE", ")");
			add_symbol ($2, &neuron_layers);
			free ($2);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions output_type memory_type_st COMA SHOW
		{
			$$ = C ("21", "create_neuron_layer (&", $2, ", NULL, ", "NOT_SPECIFIED", ", ", $4, ", ", $3, 
			              ",", $5, ", ", "NEURON_MEMORY_SIZE", ");\ncreate_output (&", $2, "_out, ", $3, 
				      ", NULL, 0);\noutput_connect (&", $2, ", &", $2, "_out)");
			add_symbol ($2, &neuron_layers);
			add_symbol (C ("2", $2, "_out"), &outputs);
			free ($2);
		}
		|	NEURON_LAYER_TOKEN VARIABLE dimentions output_type memory_type_st
		{
			$$ = C ("13", "create_neuron_layer (&", $2, ", NULL, ", "NOT_SPECIFIED", ", ", $4, 
				 ", ", $3, ",", $5, ", ", "NEURON_MEMORY_SIZE", ")");
			add_symbol ($2, &neuron_layers);
			free ($2);
		}
	;
filter_st	:	FILTER layers WITH function params PRODUCING VARIABLE
		{
			sprintf (__char_num_layers, "%d", __num_layers);

			if (list_member ($7, inputs))
				$$ = C ("11", "create_filter (", $4, ", ", $7, ".neuron_layer, ", __char_num_layers, ", ", $2, ", ", $5, ")");
			else
				$$ = C ("11", "create_filter (", $4, ", &", $7, ", ", __char_num_layers, ", ", $2, ", ", $5, ")");

			add_symbol ($4, &filter_functions);
			__num_filters++;
			free ($7);
		}
	;
association_st	:	ASSOCIATE VARIABLE WITH VARIABLE 
		{
			if ((list_member ($2, inputs)) || (list_member ($2, outputs)))
				$$ = C ("5", "associate_neurons (", $2, ".neuron_layer, &", $4, ")");
			else
				$$ = C ("5", "associate_neurons (&", $2, ", &", $4, ")");
			free ($2);
			free ($4);
		}
	;
interconnection_pattern:
		{
			$$ = "DIFFERENT_INTERCONNECTION_PATTERN";
		}
		|	COMA SAME INTERCONNECTION PATTERN_TOKEN
		{
			$$ = "SAME_INTERCONNECTION_PATTERN" ;
		}
	;
connection_st	:	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN
		{
			if (list_member ($2, inputs))
				$$ = C ("12", "connect_neurons (RAN, ", $2, ".neuron_layer, &", $5, ", ", $8, ", 0, 0, 0,",
					 $3, ", ", $6,",STANDARD_INTERCONNECTION_PATTERN",")");
			else
				$$ = C ("12", "connect_neurons (RAN, &", $2, ", &", $5, ", ", $8, ", 0, 0, 0,",
					 $3, ", ", $6,",STANDARD_INTERCONNECTION_PATTERN",")");
			free ($2);
			free ($5);
		}
		|	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN AND GAUSSIAN_TOKEN DISTRIBUTION WITH RADIUS expression interconnection_pattern
		{
			if (list_member ($2, inputs))
				$$ = C ("15", "connect_neurons (GAU, ", $2, ".neuron_layer, &", $5, ", ", $8, ", ", $18, ", 0, 0, ",
					 $3, ", ", $6, ", ", $19, ")");
			else
				$$ = C ("15", "connect_neurons (GAU, &", $2, ", &", $5, ", ", $8, ", ", $18, ", 0, 0, ",
					 $3, ", ", $6, ", ", $19, ")");
			free ($2);
			free ($5);
		}
		|	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN AND GAUSSIAN_TOKEN DISTRIBUTION WITH RADII expression AND expression AND ANGLE expression interconnection_pattern
		{
			if (list_member ($2, inputs))
				$$ = C ("19", "connect_neurons (GAU2, ", $2, ".neuron_layer, &", $5, ", ", $8, ", ", $18, ", ", $20, ", ", $23, ", ",
					 $3, ", ", $6, ", ", $24, ")");
			else
				$$ = C ("19", "connect_neurons (GAU2, &", $2, ", &", $5, ", ", $8, ", ", $18, ", ", $20, ", ", $23, ", ",
					 $3, ", ", $6, ", ", $24, ")");
			free ($2);
			free ($5);
		}
		|	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN AND LINEAR DISTRIBUTION WITH SIZE expression SQUARED interconnection_pattern
		{
			if (list_member ($2, inputs))
				$$ = C ("15", "connect_neurons (LIN, ", $2, ".neuron_layer, &", $5, ", ", $8, ", ", $18, ",0,0, ",
					 $3, ", ", $6, ",", $20,")");
			else
				$$ = C ("15", "connect_neurons (LIN, &", $2, ", &", $5, ", ", $8, ", ", $18, ",0,0, ",
					 $3, ", ", $6, ",", $20,")");
			free ($2);
			free ($5);
		}
		|	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN AND LOG_POLAR_TOKEN DISTRIBUTION WITH RADIUS expression AND LOG_FACTOR expression interconnection_pattern
		{
			if (list_member ($2, inputs))
				$$ = C ("17", "connect_neurons (LOG, ", $2, ".neuron_layer, &", $5, ", ", $8, ", ", $18, ", ", $21 ,",0 , " ,
					 $3, ", ", $6, ", ", $22,")");
			else
				$$ = C ("17", "connect_neurons (LOG, &", $2, ", &", $5, ", ", $8, ", ", $18, ", ", $21 ,",0 , " ,
					 $3, ", ", $6, ", ", $22,")");
			free ($2);
			free ($5);
		}	//log-polar-scaled
		|	CONNECT VARIABLE ranges TO VARIABLE ranges WITH expression RANDOM INPUTS PER NEURON_TOKEN AND LOG_POLAR_SCALED_TOKEN DISTRIBUTION WITH RADIUS expression AND LOG_FACTOR expression AND SCALE_FACTOR expression interconnection_pattern
		{
			if (list_member ($2, inputs))
				$$ = C ("19", "connect_neurons (LOG_SCALED, ", $2, ".neuron_layer, &", $5, ", ", $8, ", ", $18, ",", $21 ,"," , $24 , ",",
					 $3, ", ", $6, ", ", $25,")");
			else
				$$ = C ("19", "connect_neurons (LOG_SCALED, &", $2, ", &", $5, ", ", $8, ", ", $18, ", ", $21 ,"," , $24 , ",",
					 $3, ", ", $6, ", ", $25,")");
			free ($2);
			free ($5);
		}
		|	CONNECT VARIABLE ranges TO VARIABLE ranges
		{
			if (list_member ($2, inputs))
				$$ = C ("11", "connect_neurons (DIRECT_CONNECTION, ", $2, ".neuron_layer, &", $5, ", ", "0, 0, 0, 0,",
					 $3, ", ", $6, ",STANDARD_INTERCONNECTION_PATTERN",")");
			else
				$$ = C ("11", "connect_neurons (DIRECT_CONNECTION, &", $2, ", &", $5, ", ", "0, 0, 0, 0,",
					 $3, ", ", $6, ",STANDARD_INTERCONNECTION_PATTERN",")");
			free ($2);
			free ($5);
		}
	;
full_connection_st:	FULL_CONNECT VARIABLE ranges TO VARIABLE ranges
		{
			if (list_member ($2, inputs))
				$$ = C ("11", "connect_neurons (FUL, ", $2, ".neuron_layer, &", $5, ", ", "0, 0, 0,",
				         $3, ", ", $6, ",STANDARD_INTERCONNECTION_PATTERN",")");
			else
				$$ = C ("11", "connect_neurons (FUL, &", $2, ", &", $5, ", ", "0, 0, 0,",
				         $3, ", ", $6, ",STANDARD_INTERCONNECTION_PATTERN",")");
			free ($2);
			free ($5);
		}
	;
output_connection_st:	OUTPUT_CONNECT VARIABLE TO VARIABLE
		{
			if (list_member ($2, inputs))
				$$ = C ("5", "output_connect (", $2, ".neuron_layer, &", $4, ")");
			else
				$$ = C ("5", "output_connect (&", $2, ", &", $4, ")");
			free ($2);
			free ($4);
		}
	;
set_st		:	SET VARIABLE EQUAL expression
		{
			$$ = C ("3", $2, " = ", $4);
			free ($2);
		}
	;
assignment_st	: INT VARIABLE EQUAL expression
		{
			$$ = C ("3", $2, " = ", $4);
			add_symbol ($2, &integers);
			free ($2);
		}
		| FLOAT VARIABLE EQUAL expression
		{
			$$ = C ("3", $2, " = ", $4);
			add_symbol ($2, &floats);
			free ($2);
		}
		/* TODO: A reduce/reduce conflict is generated here, due to the CONSTANT in the "expression" rule */
		| CONST VARIABLE EQUAL CONSTANT
		{
                        $$ = "//";
			add_symbol (C ("4", $2, " (", $4, ")"), &consts);
			free ($2);
		}
		| CONST VARIABLE EQUAL STRING
		{
                        $$ = "//";
			add_symbol (C ("4", $2, " (", $4, ")"), &consts);
			free ($2);
		}
		| CONST VARIABLE EQUAL VARIABLE
		{
                        $$ = "//";
			add_symbol (C ("4", $2, " (", $4, ")"), &macros);
			free ($2);
		}
		| CONST VARIABLE EQUAL expression
		{
                        $$ = "//";
			add_symbol (C ("4", $2, " (", $4, ")"), &macros);
			free ($2);
		}
	;
indefinite_loop	:	WHILE OPEN_PARENTESIS comparison CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES
		{
			$$ = C ("5", "while (", $3, ") {\n", $6, "}\n");
		}
	;
definite_loop	:	FOR OPEN_PARENTESIS statement SEMI_COLON comparison SEMI_COLON statement CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES
		{
			$$ = C ("9", "for (", $3, "; ", $5, "; ", $7, ") {\n", $10, "}\n");
		}
	;
sensitivity	:	o_type SENSITIVE
		{
			$$ = $1;
		}
	;
output_type	:	WITH o_type OUTPUTS
		{
			$$ = $2;
		}
	;
pyramid_type    :	AND expression REGULAR_TOKEN PYRAMID_TOKEN COPIES
		{
			$$ = C ("2", $2, ", REGULAR_PYRAMID");
		}
		|
		        AND expression GAUSSIAN_TOKEN PYRAMID_TOKEN COPIES
		{
			$$ = C ("2", $2, ", GAUSSIAN_PYRAMID");
		}
		|
		        AND expression LAPLACIAN_TOKEN PYRAMID_TOKEN COPIES
		{
			$$ = C ("2", $2, ", LAPLACIAN_PYRAMID");
		}
		|
		        AND expression PYRAMID_TOKEN COPIES
		{
			$$ = C ("2", $2, ", REGULAR_PYRAMID");
		}
		|
		{
			$$ = "0, REGULAR_PYRAMID";
		}
	;
o_type		:	COLOR_TOKEN
		{
			$$ = "COLOR";
		}
		|	COLOR_SELECTIVE_TOKEN
		{
			$$ = "COLOR_SELECTIVE";
		}
		|	GREYSCALE_TOKEN
		{
			$$ = "GREYSCALE";
		}
		|	GREYSCALE_FLOAT_TOKEN
		{
			$$ = "GREYSCALE_FLOAT";
		}
		|	BLACK_WHITE_TOKEN
		{
			$$ = "BLACK_WHITE";
		}
	;
function_st	:	INT VARIABLE OPEN_PARENTESIS string CLOSE_PARENTESIS 
		{
			add_symbol ($2, &user_functions);
			$$ = C ("7", "create_interpreter_user_function (INT_TYPE, ", $2, ", \"", $2, "\", ", $4, ");\n");
			free ($2);
		}
		|	FLOAT VARIABLE OPEN_PARENTESIS string CLOSE_PARENTESIS 
		{
			add_symbol ($2, &user_functions);
			$$ = C ("7", "create_interpreter_user_function (FLOAT_TYPE, ", $2, ", \"", $2, "\", ", $4, ");\n");
			free ($2);
		}
	;
function	:	VARIABLE 
		{
			$$ = $1;
		}
	;
params		:	OPEN_PARENTESIS params_list CLOSE_PARENTESIS 
		{
			$$ = $2;
		}
	;
params_list	:	/* empty */
		{
			$$ = "\" \"";
		}
		| 	param
		{
			$$ = $1;
		}
		| 	params_list COMA param
		{
			$$ = C ("3", $1, ", ", $3);
			free ($3);
		}
	;
param		: 	string
		{
			$$ = $1;
		}
		|	expression
		{
			if ((list_member ($1, inputs)) || (list_member ($1, outputs)) || (list_member ($1, neuron_layers)))
				$$ = C ("2", "&", $1);
			else
				$$ = $1;
		}
	;
layers		:	layer
		{
			$$ = $1;
			__num_layers = 1;
		}
		|	layers COMA layer      
		{
			$$ = C ("3", $1, ", ", $3);
			free ($3);
			__num_layers++;
		}
	;
layer		:	VARIABLE 
		{
			if ((list_member ($1, inputs)) || (list_member ($1, outputs)))
				$$ = C ("2", $1, ".neuron_layer");
			else
				$$ = C ("2", "&", $1);
		}	
	;
ranges		:	COMA WITHIN dimentions AND dimentions COMA      
		{
			$$ = C ("3", $3, ",", $5);
		}
		|	COMA WITHIN dimentions AND dimentions      
		{
			$$ = C ("3", $3, ",", $5);
		}
		| 
		{
			$$ = "-1,-1,-1,-1";
		}
	;
dimentions	:	OPEN_COLCHETES expression CLOSE_COLCHETES OPEN_COLCHETES expression CLOSE_COLCHETES
		{
			$$ = C ("3", $2, ",", $5);
		}
		|	OPEN_COLCHETES expression COMA expression CLOSE_COLCHETES
		{
			$$ = C ("3", $2, ",", $4);
		}
	;
comparison	:	expression GREATER expression
		{
			$$ = C ("3", $1, " > ", $3);
		}
		|	expression LESS expression
		{
			$$ = C ("3", $1, " < ", $3);
		}
		|	expression LESS_EQUAL expression
		{
			$$ = C ("3", $1, " <= ", $3);
		}
		|	expression GREATER_EQUAL expression
		{
			$$ = C ("3", $1, " >= ", $3);
		}
		|	expression EQUAL expression
		{
			$$ = C ("3", $1, " == ", $3);
		}
		|	expression DIFERENT expression
		{
			$$ = C ("3", $1, " != ", $3);
		}
	;
string	:	STRING
		{
			$$ = C ("1", $1);
			free ($1);
		}
	;
expression	:	expression PLUS expression      
		{
			$$ = C ("3", $1, " + ", $3);
		}
		|	expression MINUS expression   	     
		{
			$$ = C ("3", $1, " - ", $3);
		}
		|	expression TIMES expression   	  
		{
			$$ = C ("3", $1, " * ", $3);
		}
		|	expression DIVIDIDED expression	 
		{
			$$ = C ("3", $1, " / ", $3);
		}
		|	VARIABLE
		{
			$$ = $1;
		}
		|	CONSTANT 
		{
			$$ = $1;
		}
		|	OPEN_PARENTESIS expression CLOSE_PARENTESIS
		{
			$$ = C ("3", "(", $2, ")");
		}
		|	MINUS expression %prec UMINUS
		{
			$$ = C ("2", "-", $2);
		}
		|	OPEN_PARENTESIS INT CLOSE_PARENTESIS expression   	     
		{
			$$ = C ("2", "(int) ", $4);
		}
		|	OPEN_PARENTESIS FLOAT CLOSE_PARENTESIS expression   	     
		{
			$$ = C ("2", "(float) ", $4);
		}
	;
%%
 
 
 
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
	result = (char *) malloc (1);
	strcpy (result, "");
	for (i = 0; i < atoi (number); i++)
	{
		string = va_arg (argument, char *);
		if ((aux = (char *) malloc (strlen (string) + strlen (result) + 1)) == NULL)
			yyerror ("can not allocate more memory"); 
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
* Add symbol to a symbol list							* 
********************************************************************************* 
*/ 
 
void 
add_symbol (char *symbol, SYMBOL_LIST **list) 
{ 
	SYMBOL_LIST *s_list; 
 
	if (*list == (SYMBOL_LIST *) NULL) 
	{ 
		s_list = (SYMBOL_LIST *) malloc ((size_t) sizeof (SYMBOL_LIST)); 
		s_list->next = (SYMBOL_LIST *) NULL; 
		strcpy (s_list->symbol, symbol); 
		*list = s_list; 
	} 
	else 
	{ 
		for (s_list = *list; s_list->next != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return;
			}
		} 
		if (strcmp (symbol, s_list->symbol) == 0)
		{
			return;
		}
		s_list->next = (SYMBOL_LIST *) malloc ((size_t) sizeof (SYMBOL_LIST)); 
		s_list->next->next = (SYMBOL_LIST *) NULL; 
		strcpy (s_list->next->symbol, symbol); 
	} 
} 



 
/* 
********************************************************************************* 
* Search symbol in a symbol list						* 
********************************************************************************* 
*/ 
 
int 
list_member (char *symbol, SYMBOL_LIST *list) 
{ 
	SYMBOL_LIST *s_list; 
 
	if (list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (s_list = list; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (1);
			}
		} 
	}
	return (0);
}



/* 
********************************************************************************* 
* Show error messages 								* 
********************************************************************************* 
*/ 

void 
yyerror (char *mens)
{
	char conteudo[STRING_LENGTH];
	
	strcpy (conteudo, mens);
	sprintf (&(conteudo[strlen (conteudo)]), ": line = %d", lineno);
	fprintf (stderr, "%s\n", conteudo);
}
 


/* 
********************************************************************************* 
* Main function 								* 
********************************************************************************* 
*/  

int 
main (int argc, char **argv)
{
	int i;
	
	if (argc == 1)
	{
		sprintf (header_name, "example.h");
		sprintf (source_name, "example.c");
		sprintf (define_directive, "_EXAMPLE_H");
	}
	else if ((argc == 3) && (strcmp (argv[1], "-o") == 0))
	{
		sprintf (header_name, "%s.h", argv[2]);
		sprintf (source_name, "%s.c", argv[2]);
		sprintf (define_directive, "_%s_H", argv[2]);
		for (i = 0; define_directive[i] != '\0' && i < STRING_LENGTH; i++)
		{
			if ((define_directive[i] >= 'a') && (define_directive[i] <= 'z'))
				define_directive[i] -= 'a' - 'A';
		}
				
	}
	else
	{
		printf ("Usage:\n");
		printf ("./netcomp < <input_name>\n");
		printf ("./netcomp -o <output_name> < <input_name>\n");
		printf ("Default outputs: example.h, example.c\n");
		exit (-1);
	}
	
	yyparse ();
	
	return (0);
}
