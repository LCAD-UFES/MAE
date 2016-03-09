%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#if !defined(__APPLE__)
#include <malloc.h> 
#endif
#include <ctype.h>
#include "mae.h"

#define	INT_PARAM	1
#define	FLOAT_PARAM	2
#define	STR_PARAM	3
#define	CMD_PARAM	4

#define	FIRST_CMD_P	command->parameters->param.command_param
#define	SECOND_CMD_P	command->parameters->next->param.command_param
#define	THIRD_CMD_P	command->parameters->next->next->param.command_param

#define	FIRST_STR_P	command->parameters->param.sval
#define	SECOND_STR_P	command->parameters->next->param.sval

#define	FIRST_INT_P	command->parameters->param.ival
#define	SECOND_INT_P	command->parameters->next->param.ival
#define	THIRD_INT_P	command->parameters->next->next->param.ival

#define	FIRST_FLOAT_P	command->parameters->param.fval
#define	SECOND_FLOAT_P	command->parameters->next->param.fval

enum commands
{
	C_STR_LIST = 0,
	C_STR_LIST2,
	C_UMINUS,
	C_EXPRESSION,
	C_CONSTANT_INT,
	C_CONSTANT_FLOAT,
	C_VARIABLE,
	C_VARIABLE_VEC,
	C_DIVIDE,
	C_REMINDER_DIVIDE,
	C_MULTIPLY,
	C_SUB,
	C_ADD,
	C_GREATER,
	C_LESS,
	C_GREATER_EQUAL,
	C_LESS_EQUAL,
	C_EQUAL,
	C_NOT,
	C_DIFFERENT,
	C_WHILE,
	C_DO,
	C_IF,
	C_IF_ELSE,
	C_PRINT,
        C_TIME,
	C_ASSIGNMENT,
	C_ASSIGNMENT_VEC,
	C_INT,
	C_FLOAT,
	C_INT_VEC,
	C_FLOAT_VEC,
	C_DRAW,
	C_DRAW_OUTPUT,
	C_DRAW_Y,
	C_DRAW_X,
	C_POP,
	C_PUSH,
	C_TOGGLE_DRAW,
	C_TOGGLE_MOVE,
	C_MOVE,
	C_FORWARD_NETWORK,
	C_FORWARD_NEURAL_LAYERS,
	C_FORWARD_INPUTS,
	C_FORWARD_OUTPUTS,
	C_FORWARD_FILTERS,
	C_FORWARD_LIST,
	C_CORRELATE,
	C_CORRELATE_LIST,
	C_PRUNE,
	C_PRUNE_LIST,
	C_CLEAR_MEMORY,
	C_CLEAR_MEMORY_LIST,
	C_RUN,
	C_TRAIN_NETWORK,
	C_SPARSE_TRAIN_NETWORK,
	C_DESTROY_NETWORK,
	C_TRAIN,
	C_SPARSE_TRAIN,
	C_UNLOAD,
	C_RELOAD,
	C_UNLOAD_NL,
	C_RELOAD_NL,
	C_DESTROY,
	C_RAND,
	C_CLEAR,
	C_PASTE,
	C_COPY,
	C_KEY,
	C_KEY_NUM,
	C_TEST_CONN,
	C_TEST_CONN_V,
	C_WAIT,
	C_QUIT,
	C_PAUSE,
	C_SERIES,
	C_ADD_PARAM,
	C_ADD_PARAM2,
	C_ADD_PARAM3,
	C_FUNCTION,
	C_SAVE,
	C_LOAD,
	C_OR,
	C_AND,
	C_BITWISE_OR,
	C_BITWISE_AND,
	C_SHIFT_RIGHT,
	C_SHIFT_LEFT
};


extern int tokval;
extern char line_info[];
extern char *yytext;

SYMBOL_LIST *int_variables       = NULL;
SYMBOL_LIST *float_variables     = NULL;
SYMBOL_LIST *int_vec_variables   = NULL;
SYMBOL_LIST *float_vec_variables = NULL;
SYMBOL_LIST *int_functions	 = NULL;
SYMBOL_LIST *float_functions	 = NULL;

char error_message[256];

struct _cparam
{
	int		type;
	struct
	{
		int		ival;
		float		fval;
		char		*sval;
		void		*pval;
		struct _command *command_param;
	}param;
	struct _cparam 	*next;
};

typedef struct _cparam CPARAM;

struct _command
{
	int 		command;
	CPARAM 		*parameters;
};

typedef struct _command COMMAND;

void free_tree (void *cmd);
void exec_command (void *cmd);
void *CC (int number, ...);
void yyerror (char *mens);
%}

%union
{
	char 	*char_pointer_val;
	int 	int_val;
	float 	float_val;
	void 	*command;
}

%start command

%token	KEY_ST
%token	COPY_ST
%token	PASTE_ST
%token	SAVE_ST
%token	LOAD_ST
%token	CLEAR_ST
%token	RAND_ST
%token	TRAIN_ST
%token	SPARSE_TRAIN_ST
%token	UNLOAD_ST
%token	RELOAD_ST
%token	DESTROY_ST
%token	RUN_ST
%token	FORWARD_ST
%token	TEST_CONNECTIONS_ST
%token	TEST_CONNECTIONS_V_ST
%token	CORRELATE_ST
%token	PRUNE_ST
%token	MOVE_ST
%token	TOGGLE_ST
%token	PUSH_ST
%token	POP_ST
%token	DRAW_X_ST
%token	DRAW_Y_ST
%token	DRAW_ST
%token	POSITION
%token	BASED
%token	OUTPUT
%token	MOVE_ACTIVE
%token	DRAW_ACTIVE
%token	NETWORK
%token	NEURAL_LAYERS
%token	INPUTS
%token	OUTPUTS
%token	FILTERS
%token	MEMORY
%token	ON
%token	OF
%token	TO
%token	FROM
%token	WAIT
%token	QUIT
%token	PAUSE
%token	DISPLAY
%token	INT
%token	FLOAT
%token	INT_VEC
%token	FLOAT_VEC
%token	WHILE
%token	DO
%token	IF
%token	PRINT
%token	ELSE
%token	OPEN_PARENTESIS
%token	CLOSE_PARENTESIS
%token	OPEN_CHAVES
%token	CLOSE_CHAVES
%token	OPEN_COLCHETES
%token	CLOSE_COLCHETES
%token	COLON
%token	SEMI_COLON
%token	COMA
%token	GREATER
%token	LESS
%token	EQUAL
%token	NOT
%token	EQUAL_EQUAL
%token	DIFFERENT
%token	GREATER_EQUAL
%token	LESS_EQUAL
%token	PLUS
%token	MINUS
%token	TIMES
%token	DIVIDE
%token	REMINDER_DIVIDE
%token	OR
%token	AND
%token	BITWISE_OR
%token	BITWISE_AND
%token	SHIFT_RIGHT
%token	SHIFT_LEFT

%token	<int_val>		CONSTANT_INT
%token	<float_val>		CONSTANT_FLOAT
%token  <float_val>             TIME
%token	<char_pointer_val> 	VARIABLE
%token	<char_pointer_val> 	STRING

%type	<command>		series
%type	<command>		statement
%type	<command>		wait_st
%type	<command>		quit_st
%type	<command>		pause_st
%type	<command>		test_conn_st
%type	<command>		test_conn_v_st
%type	<command>		copy_st
%type	<command>		paste_st
%type	<command>		save_st
%type	<command>		load_st
%type	<command>		clear_st
%type	<command>		rand_st
%type	<command>		train_st
%type	<command>		sparse_train_st
%type	<command>		unload_st
%type	<command>		reload_st
%type	<command>		destroy_st
%type	<command>		run_st
%type	<command>		forward_st
%type	<command>		correlate_st
%type	<command>		prune_st
%type	<command>		move_st
%type	<command>		toggle_st
%type	<command>		push_position_st
%type	<command>		pop_position_st
%type	<command>		draw_x_st
%type	<command>		draw_y_st
%type	<command>		draw_st
%type	<command>		draw_output_st
%type	<command>		int_st
%type	<command>		float_st
%type	<command>		int_vec_st
%type	<command>		float_vec_st
%type	<command>		assignment_st
%type	<command>		while_loop
%type	<command>		do_while_loop
%type	<command>		if_st
%type	<command>		print_st
%type	<command>		time_st
%type	<command>		key_st
%type	<command>		function_st
%type	<command>		param_list
%type	<command>		param
%type	<command>		expression
%type	<char_pointer_val>	str_list

%left OR 
%left AND
%left BITWISE_AND BITWISE_OR
%left EQUAL_EQUAL DIFFERENT GREATER LESS GREATER_EQUAL LESS_EQUAL
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left TIMES DIVIDE REMINDER_DIVIDE
%left UMINUS NOT

%%
command		:	series 
		{
			exec_command ($1);
			free_tree ($1);
			error_message[0] = '\0';
		}
	;
series		:	statement
		|	series statement
		{
			$$ = CC (2, C_SERIES, CMD_PARAM, $1, CMD_PARAM, $2);
		}
	;
statement	:	wait_st
		|	quit_st
		|	pause_st
		|	test_conn_st
		|	test_conn_v_st
		|	copy_st
		|	paste_st
		|	save_st
		|	load_st
		|	clear_st
		|	rand_st
		|	train_st
		|	sparse_train_st
		|	unload_st
		|	reload_st
		|	destroy_st
		|	run_st
		|	forward_st
		|	correlate_st
		|	prune_st
		|	move_st
		|	toggle_st
		|	push_position_st
		|	pop_position_st
		|	draw_x_st
		|	draw_y_st
		|	draw_output_st
		|	draw_st
		|	int_st
		|	float_st
		|	int_vec_st
		|	float_vec_st
		|	assignment_st
		|	while_loop
		|	do_while_loop
		|	if_st
                |	print_st
                |	time_st
                |	key_st
		|	function_st
		|	error SEMI_COLON
		{
			yyerrok;
			return (0);
		}
	;
wait_st		:	WAIT DISPLAY SEMI_COLON
		{
			$$ = CC (0, C_WAIT);
		}
	;
quit_st		:	QUIT SEMI_COLON
		{
			$$ = CC (0, C_QUIT);
		}
	;
pause_st	:	PAUSE SEMI_COLON
		{
			$$ = CC (0, C_PAUSE);
		}
	;
test_conn_st	:	TEST_CONNECTIONS_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_TEST_CONN, STR_PARAM, $2);
		}
	;
test_conn_v_st	:	TEST_CONNECTIONS_V_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_TEST_CONN_V, STR_PARAM, $2);
		}
	;
copy_st		:	COPY_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_COPY, STR_PARAM, $2);
		}
	;
paste_st	:	PASTE_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_PASTE, STR_PARAM, $2);
		}
	;
save_st		:	SAVE_ST VARIABLE TO VARIABLE SEMI_COLON
		{
			$$ = CC (2, C_SAVE, STR_PARAM, $2, STR_PARAM, $4);
		}
	;
load_st	:		LOAD_ST VARIABLE FROM VARIABLE SEMI_COLON
		{
			$$ = CC (2, C_LOAD, STR_PARAM, $2, STR_PARAM, $4);
		}
	;
clear_st	:	CLEAR_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_CLEAR, STR_PARAM, $2);
		}
		|	CLEAR_ST NETWORK MEMORY SEMI_COLON
		{
			$$ = CC (0, C_CLEAR_MEMORY);
		}
		|	CLEAR_ST str_list MEMORY SEMI_COLON
		{
			$$ = CC (1, C_CLEAR_MEMORY_LIST, CMD_PARAM, $2);
		}
	;
rand_st		:	RAND_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_RAND, STR_PARAM, $2);
		}
	;
train_st	:	TRAIN_ST NETWORK SEMI_COLON
		{
			$$ = CC (0, C_TRAIN_NETWORK);
		}
		|	TRAIN_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_TRAIN, STR_PARAM, $2);
		}
	;
sparse_train_st	:	SPARSE_TRAIN_ST NETWORK COMA CONSTANT_FLOAT SEMI_COLON
		{
			$$ = CC (1, C_SPARSE_TRAIN_NETWORK, FLOAT_PARAM , $4);
		}
		|	SPARSE_TRAIN_ST VARIABLE COMA CONSTANT_FLOAT SEMI_COLON
		{
			$$ = CC (2, C_SPARSE_TRAIN, STR_PARAM, $2, FLOAT_PARAM, $4);
		}
	;
unload_st	:	UNLOAD_ST TO STRING SEMI_COLON
		{
			$$ = CC (1, C_UNLOAD, STR_PARAM, $3);
		}
		| UNLOAD_ST STRING TO STRING SEMI_COLON
		{
			$$ = CC (2, C_UNLOAD_NL, STR_PARAM, $2, STR_PARAM, $4);
		}
	;
reload_st	:	RELOAD_ST FROM STRING SEMI_COLON
		{
			$$ = CC (1, C_RELOAD, STR_PARAM, $3);
		}
		| RELOAD_ST STRING FROM STRING SEMI_COLON
		{
			$$ = CC (2, C_RELOAD_NL, STR_PARAM, $2, STR_PARAM, $4);
		}
	;
destroy_st	:	DESTROY_ST NETWORK SEMI_COLON
		{
			$$ = CC (0, C_DESTROY_NETWORK);
		}
		|	DESTROY_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_DESTROY, STR_PARAM, $2);
		}
	;
run_st		:	RUN_ST expression SEMI_COLON
		{
			$$ = CC (1, C_RUN, CMD_PARAM, $2);
		}
	;
forward_st	:	FORWARD_ST NETWORK SEMI_COLON
		{
			$$ = CC (0, C_FORWARD_NETWORK);
		}
		|	FORWARD_ST NEURAL_LAYERS SEMI_COLON
		{
			$$ = CC (0, C_FORWARD_NEURAL_LAYERS);
		}
		|	FORWARD_ST INPUTS SEMI_COLON
		{
			$$ = CC (0, C_FORWARD_INPUTS);
		}
		|	FORWARD_ST OUTPUTS SEMI_COLON
		{
			$$ = CC (0, C_FORWARD_OUTPUTS);
		}
		|	FORWARD_ST FILTERS SEMI_COLON
		{
			$$ = CC (0, C_FORWARD_FILTERS);
		}
		|	FORWARD_ST str_list SEMI_COLON
		{
			$$ = CC (1, C_FORWARD_LIST, CMD_PARAM, $2);
		}
	;
correlate_st	:	CORRELATE_ST NETWORK SEMI_COLON
		{
			$$ = CC (0, C_CORRELATE);
		}
		|	CORRELATE_ST str_list SEMI_COLON
		{
			$$ = CC (1, C_CORRELATE_LIST, CMD_PARAM, $2);
		}
	;
prune_st	:	PRUNE_ST NETWORK SEMI_COLON
		{
			$$ = CC (0, C_PRUNE);
		}
		|	PRUNE_ST str_list SEMI_COLON
		{
			$$ = CC (1, C_PRUNE_LIST, CMD_PARAM, $2);
		}
	;
str_list	: 	VARIABLE
		{
			$$ = CC (1, C_STR_LIST, STR_PARAM, $1);
		}
		|	str_list COMA VARIABLE
		{
			$$ = CC (2, C_STR_LIST2, CMD_PARAM, $1, STR_PARAM, $3);
		}
	;
move_st		:	MOVE_ST VARIABLE TO expression COMA expression SEMI_COLON
		{
			$$ = CC (3, C_MOVE, STR_PARAM, $2, CMD_PARAM, $4, CMD_PARAM, $6);
		}
	;
toggle_st	:	TOGGLE_ST MOVE_ACTIVE SEMI_COLON
		{
			$$ = CC (0, C_TOGGLE_MOVE);
		}
		|	TOGGLE_ST DRAW_ACTIVE SEMI_COLON
		{
			$$ = CC (0, C_TOGGLE_DRAW);
		}
	;
push_position_st:	PUSH_ST POSITION OF VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_PUSH, STR_PARAM, $4);
		}
	;
pop_position_st:	POP_ST POSITION OF VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_POP, STR_PARAM, $4);
		}
	;
draw_x_st	:	DRAW_X_ST VARIABLE BASED ON VARIABLE MOVE_ST SEMI_COLON
		{
			$$ = CC (2, C_DRAW_X, STR_PARAM, $2, STR_PARAM, $5);
		}
	;
draw_y_st	:	DRAW_Y_ST VARIABLE BASED ON VARIABLE MOVE_ST SEMI_COLON
		{
			$$ = CC (2, C_DRAW_Y, STR_PARAM, $2, STR_PARAM, $5);
		}
	;
draw_output_st	:	DRAW_ST OUTPUT VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_DRAW_OUTPUT, STR_PARAM, $3);
		}
	;
draw_st		:	DRAW_ST VARIABLE BASED ON VARIABLE MOVE_ST SEMI_COLON
		{
			$$ = CC (2, C_DRAW, STR_PARAM, $2, STR_PARAM, $5);
		}
	;
key_st		:	KEY_ST VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_KEY, STR_PARAM, $2);
		}
		|       KEY_ST expression SEMI_COLON
                {
                        $$ = CC (1, C_KEY_NUM, CMD_PARAM, $2);
                }
	;
int_st		:	INT VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_INT, STR_PARAM, $2);
		}
	;
int_vec_st	:	INT VARIABLE OPEN_COLCHETES expression CLOSE_COLCHETES SEMI_COLON 
		{
			$$ = CC (2, C_INT_VEC, STR_PARAM, $2, CMD_PARAM, $4);
		}
	;
float_st	:	FLOAT VARIABLE SEMI_COLON
		{
			$$ = CC (1, C_FLOAT, STR_PARAM, $2);
		}
	;
float_vec_st	:	FLOAT VARIABLE OPEN_COLCHETES expression CLOSE_COLCHETES SEMI_COLON 
		{
			$$ = CC (2, C_FLOAT_VEC, STR_PARAM, $2, CMD_PARAM, $4);
		}
	;
assignment_st	:	VARIABLE EQUAL expression SEMI_COLON
		{
			$$ = CC (2, C_ASSIGNMENT, STR_PARAM, $1, CMD_PARAM, $3);
		}
		|	VARIABLE OPEN_COLCHETES expression CLOSE_COLCHETES EQUAL expression SEMI_COLON
		{
			$$ = CC (3, C_ASSIGNMENT_VEC, STR_PARAM, $1, CMD_PARAM, $3, CMD_PARAM, $6);
		}
	;
while_loop	:	WHILE OPEN_PARENTESIS expression CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES 
		{
			$$ = CC (2, C_WHILE, CMD_PARAM, $3, CMD_PARAM, $6);
		}
		|	WHILE OPEN_PARENTESIS expression CLOSE_PARENTESIS statement 
		{
			$$ = CC (2, C_WHILE, CMD_PARAM, $3, CMD_PARAM, $5);
 		}
	;
do_while_loop	:	DO OPEN_CHAVES series CLOSE_CHAVES WHILE OPEN_PARENTESIS expression CLOSE_PARENTESIS SEMI_COLON
		{
			$$ = CC (2, C_DO, CMD_PARAM, $3, CMD_PARAM, $7);
		}
		|	DO statement WHILE OPEN_PARENTESIS expression CLOSE_PARENTESIS SEMI_COLON
		{
			$$ = CC (2, C_DO, CMD_PARAM, $2, CMD_PARAM, $5);
		}
	;
if_st		:	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES ELSE OPEN_CHAVES series CLOSE_CHAVES
		{
			$$ = CC (3, C_IF_ELSE, CMD_PARAM, $3, CMD_PARAM, $6, CMD_PARAM, $10);
		}
		|	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES ELSE statement 
		{
			$$ = CC (3, C_IF_ELSE, CMD_PARAM, $3, CMD_PARAM, $6, CMD_PARAM, $9);
		}
		|	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS OPEN_CHAVES series CLOSE_CHAVES
		{
			$$ = CC (2, C_IF, CMD_PARAM, $3, CMD_PARAM, $6);
		}
		|	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS statement ELSE OPEN_CHAVES series CLOSE_CHAVES
		{
			$$ = CC (3, C_IF_ELSE, CMD_PARAM, $3, CMD_PARAM, $5, CMD_PARAM, $8);
		}
		|	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS statement ELSE statement 
		{
			$$ = CC (3, C_IF_ELSE, CMD_PARAM, $3, CMD_PARAM, $5, CMD_PARAM, $7);
		}
		|	IF OPEN_PARENTESIS expression CLOSE_PARENTESIS statement 
		{
			$$ = CC (2, C_IF, CMD_PARAM, $3, CMD_PARAM, $5);
		}
	;
print_st	:	PRINT OPEN_PARENTESIS STRING COMA expression CLOSE_PARENTESIS SEMI_COLON
		{
			$$ = CC (2, C_PRINT, STR_PARAM, $3, CMD_PARAM, $5);
		}
	;
time_st         :	TIME SEMI_COLON
                {
                        $$ = CC (1, C_TIME, FLOAT_PARAM, $1);
                }
        ;
param_list	: 	param
		{
			$$ = CC (1, C_ADD_PARAM, CMD_PARAM, $1);
		}
		|	param_list COMA param
		{
			$$ = CC (2, C_ADD_PARAM2, CMD_PARAM, $1, CMD_PARAM, $3);
		}
	;
param		: 	expression
		{
			$$ = CC (1, C_ADD_PARAM, CMD_PARAM, $1);
		}
		|	STRING
		{
			$$ = CC (1, C_ADD_PARAM3, STR_PARAM, $1);
		}
	;
function_st	:	VARIABLE OPEN_PARENTESIS param_list CLOSE_PARENTESIS SEMI_COLON
		{
			$$ = CC (2, C_FUNCTION, STR_PARAM, $1, CMD_PARAM, $3);
		}
	;	
expression	:	expression PLUS expression 
		{
			$$ = CC (2, C_ADD, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression MINUS expression   	     
		{
			$$ = CC (2, C_SUB, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression TIMES expression   	  
		{
			$$ = CC (2, C_MULTIPLY, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression DIVIDE expression	 
		{
			$$ = CC (2, C_DIVIDE, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression REMINDER_DIVIDE expression	 
		{
			$$ = CC (2, C_REMINDER_DIVIDE, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression OR expression
		{
			$$ = CC (2, C_OR, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression AND expression
		{
			$$ = CC (2, C_AND, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression BITWISE_OR expression
		{
			$$ = CC (2, C_BITWISE_OR, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression BITWISE_AND expression
		{
			$$ = CC (2, C_BITWISE_AND, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression SHIFT_RIGHT expression
		{
			$$ = CC (2, C_SHIFT_RIGHT, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression SHIFT_LEFT expression
		{
			$$ = CC (2, C_SHIFT_LEFT, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression GREATER expression
		{
			$$ = CC (2, C_GREATER, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression LESS expression
		{
			$$ = CC (2, C_LESS, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression LESS_EQUAL expression
		{
			$$ = CC (2, C_LESS_EQUAL, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression GREATER_EQUAL expression
		{
			$$ = CC (2, C_GREATER_EQUAL, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression EQUAL_EQUAL expression
		{
			$$ = CC (2, C_EQUAL, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	expression DIFFERENT expression
		{
			$$ = CC (2, C_DIFFERENT, CMD_PARAM, $1, CMD_PARAM, $3);
		}
		|	VARIABLE
		{
			$$ = CC (1, C_VARIABLE, STR_PARAM, $1);
		}
		|	VARIABLE OPEN_COLCHETES expression CLOSE_COLCHETES
		{
			$$ = CC (2, C_VARIABLE_VEC, STR_PARAM, $1, CMD_PARAM, $3);
		}
		|	VARIABLE OPEN_PARENTESIS param_list CLOSE_PARENTESIS
		{
			$$ = CC (2, C_FUNCTION, STR_PARAM, $1, CMD_PARAM, $3);
		}
		|	CONSTANT_INT 
		{
			$$ = CC (1, C_CONSTANT_INT, INT_PARAM, $1);
		}
		|	CONSTANT_FLOAT 
		{
			$$ = CC (1, C_CONSTANT_FLOAT, FLOAT_PARAM, $1);
		}
                |       TIME
                {
                        $$ = CC (1, C_TIME, FLOAT_PARAM, $1);
                }
                |	OPEN_PARENTESIS expression CLOSE_PARENTESIS
		{
			$$ = CC (1, C_EXPRESSION, CMD_PARAM, $2);
		}
		|	MINUS expression %prec UMINUS
		{
			$$ = CC (1, C_UMINUS, CMD_PARAM, $2);
		}
		|	NOT expression
		{
			$$ = CC (1, C_NOT, CMD_PARAM, $2);
		}
	;
%%


 
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
		s_list = (SYMBOL_LIST *) alloc_mem ((size_t) sizeof (SYMBOL_LIST)); 
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
		s_list->next = (SYMBOL_LIST *) alloc_mem ((size_t) sizeof (SYMBOL_LIST)); 
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
* Get the value of a symbol of a symbol list					* 
********************************************************************************* 
*/ 
 
int 
get_int_variable (char *symbol) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = int_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.ival);
			}
		} 
	}
	return (0);
} 



/* 
********************************************************************************* 
* Set the value of a integer variable 						* 
********************************************************************************* 
*/ 
 
int 
set_int_variable (char *symbol, int value) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = int_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.ival = value);
			}
		} 
	}
	return (0);
} 



/* 
********************************************************************************* 
* Get the address of a int variable in the symbol list				* 
********************************************************************************* 
*/ 
 
void * 
get_address_of_int_variable (char *symbol) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = int_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return ((void *) NULL); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return ((void *) &(s_list->symbol_val.ival));
			}
		} 
	}
	return ((void *) NULL);
} 



/* 
********************************************************************************* 
* Get the value of a symbol of a symbol list 					* 
********************************************************************************* 
*/ 
 
float
get_float_variable (char *symbol) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = float_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0.0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.fval);
			}
		} 
	}
	return (0.0);
} 



/* 
********************************************************************************* 
* Set the value of a float variable 						* 
********************************************************************************* 
*/ 
 
int 
set_float_variable (char *symbol, float value) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = float_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.fval = value);
			}
		} 
	}
	return (0);
} 
 
 

/* 
********************************************************************************* 
* Get the address of a float variable in the a symbol list 			* 
********************************************************************************* 
*/ 
 
void *
get_address_of_float_variable (char *symbol) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = float_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return ((void *) NULL); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return ((void *) &(s_list->symbol_val.fval));
			}
		} 
	}
	return ((void *) NULL);
} 


 
/* 
********************************************************************************* 
* Get the value of a symbol of a symbol list					* 
********************************************************************************* 
*/ 
 
int 
get_int_vec_variable (char *symbol, int index) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = int_vec_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.ivval[index]);
			}
		} 
	}
	return (0);
} 



/* 
********************************************************************************* 
* Set the value of a integer vector variable					* 
********************************************************************************* 
*/ 
 
int 
set_int_vec_variable (char *symbol, int index, int value) 
{ 
	SYMBOL_LIST *s_list; 

 	s_list = int_vec_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.ivval[index] = value);
			}
		} 
	}
	return (0);
} 



/* 
********************************************************************************* 
* Get the value of a symbol of a symbol list 					* 
********************************************************************************* 
*/ 
 
float
get_float_vec_variable (char *symbol, int index) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = float_vec_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0.0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.fvval[index]);
			}
		} 
	}
	return (0.0);
} 



/* 
********************************************************************************* 
* Set the value of a float vector variable 					* 
********************************************************************************* 
*/ 
 
int 
set_float_vec_variable (char *symbol, int index, float value) 
{ 
	SYMBOL_LIST *s_list; 
 
 	s_list = float_vec_variables;
	
	if (s_list == (SYMBOL_LIST *) NULL) 
	{ 
		return (0); 
	} 
	else 
	{ 
		for (; s_list != (SYMBOL_LIST *) NULL; 
			s_list = s_list->next)
		{
			if (strcmp (symbol, s_list->symbol) == 0)
			{
				return (s_list->symbol_val.fvval[index] = value);
			}
		} 
	}
	return (0);
}



/* 
********************************************************************************* 
* Evaluate command								* 
********************************************************************************* 
*/ 
 
void 
free_tree (void *cmd) 
{
	COMMAND *command;
	//char *aux_str;

	command = (COMMAND *) cmd;
	switch (command->command)
	{
		case C_SERIES:
		case C_WHILE:
		case C_DO:
		case C_IF:
		case C_DIVIDE:
		case C_REMINDER_DIVIDE:
		case C_MULTIPLY:
		case C_SUB:
		case C_ADD:
		case C_GREATER:
		case C_LESS:
		case C_LESS_EQUAL:
		case C_GREATER_EQUAL:
		case C_EQUAL:
		case C_DIFFERENT:
		case C_OR:
		case C_AND:
		case C_BITWISE_OR:
		case C_BITWISE_AND:
		case C_SHIFT_RIGHT:
		case C_SHIFT_LEFT:
			free_tree (SECOND_CMD_P); free_tree (FIRST_CMD_P); 
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_IF_ELSE:
			free_tree (THIRD_CMD_P); free_tree (SECOND_CMD_P); free_tree (FIRST_CMD_P); 
			free (command->parameters->next->next); free (command->parameters->next); 
			free (command->parameters); free (command); break;
			
		case C_DRAW:
		case C_DRAW_Y:
		case C_DRAW_X:
		case C_SAVE:
		case C_LOAD:
			free (SECOND_STR_P); free (FIRST_STR_P); 
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_FUNCTION:
			free_tree (SECOND_CMD_P); free (FIRST_STR_P);
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_PRINT:
			free_tree (SECOND_CMD_P); free (FIRST_STR_P); 
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_ASSIGNMENT:
		case C_INT_VEC:
		case C_FLOAT_VEC:
		case C_VARIABLE_VEC:
			free_tree (SECOND_CMD_P); free (FIRST_STR_P); 
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_INT:
		case C_FLOAT:
		case C_POP:
		case C_PUSH:
		case C_RAND:
		case C_TRAIN:
		case C_UNLOAD:
		case C_UNLOAD_NL:
		case C_RELOAD:
		case C_RELOAD_NL:
		case C_DESTROY:
		case C_CLEAR:
		case C_PASTE:
		case C_TEST_CONN:
		case C_TEST_CONN_V:
		case C_COPY:
		case C_KEY:
		case C_VARIABLE:
		case C_STR_LIST:
		case C_DRAW_OUTPUT:
			free (FIRST_STR_P); free (command->parameters); free (command); break;
			
		case C_TOGGLE_DRAW:
		case C_TOGGLE_MOVE:
		case C_FORWARD_NETWORK:
		case C_FORWARD_NEURAL_LAYERS:
		case C_FORWARD_INPUTS:
		case C_FORWARD_OUTPUTS:
		case C_FORWARD_FILTERS:
		case C_CORRELATE:
		case C_PRUNE:
		case C_CLEAR_MEMORY:
		case C_TRAIN_NETWORK:
		case C_DESTROY_NETWORK:
		case C_CONSTANT_INT:
		case C_CONSTANT_FLOAT:
		case C_WAIT:
		case C_QUIT:
		case C_PAUSE:
                case C_TIME:
			free (command); break;
			
		case C_ASSIGNMENT_VEC:	
		case C_MOVE:
			free_tree (THIRD_CMD_P); free_tree (SECOND_CMD_P); free (FIRST_STR_P); 
			free (command->parameters->next->next); free (command->parameters->next);
			free (command->parameters); free (command); break;
			
		case C_FORWARD_LIST:
 		case C_CORRELATE_LIST:
 		case C_PRUNE_LIST:
		case C_CLEAR_MEMORY_LIST:
		case C_RUN:
		case C_UMINUS:
		case C_NOT:
		case C_EXPRESSION:
		case C_KEY_NUM:
			free_tree (FIRST_CMD_P); 
			free (command->parameters); free (command); break;
			
		case C_STR_LIST2:
			free (SECOND_STR_P); free_tree (FIRST_CMD_P); 
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		case C_ADD_PARAM:
			free_tree (FIRST_CMD_P);
			free (command->parameters); free (command); break;
			
		case C_ADD_PARAM3:
			free (FIRST_STR_P); free (command->parameters); free (command); break;
			
		case C_ADD_PARAM2:
			free_tree (SECOND_CMD_P); free_tree (FIRST_CMD_P);
			free (command->parameters->next); free (command->parameters); free (command); break;
			
		default:
			yyerror ("freeing unknown command token");
	}
}



/* 
********************************************************************************* 
* Evaluate string command							* 
********************************************************************************* 
*/ 
 
char * 
evaluate_str (COMMAND *command) 
{
	char *aux_str;
	char *result_str;
	
	switch (command->command)
	{
		case C_STR_LIST:
			result_str = (char *) alloc_mem ((size_t) (strlen (FIRST_STR_P) + 1));
			strcpy (result_str, FIRST_STR_P);
			return (result_str);
		case C_STR_LIST2:
			aux_str = evaluate_str (FIRST_CMD_P);
			result_str = (char *) alloc_mem ((size_t) (strlen (aux_str) + 
				     			 	   strlen (SECOND_STR_P) + 
								   strlen (", ") + 1));
			strcpy (result_str, aux_str);
			strcat (result_str, ", ");
			strcat (result_str, SECOND_STR_P);
			free (aux_str);
			return (result_str);
		default:
			yyerror ("unknown expression token");
			return ("");
	}
}



/* 
********************************************************************************* 
* Add one param 								* 
********************************************************************************* 
*/ 

CPARAM
add_param (CPARAM param)
{	
	param.next = (CPARAM *) NULL;
	return (param);
}



/* 
********************************************************************************* 
* Add two params 								* 
********************************************************************************* 
*/ 

CPARAM
add_param2 (CPARAM param_head, CPARAM param_noh)
{
	CPARAM *param;
	
	for (param = &(param_head); param->next != (CPARAM *) NULL; param = param->next);
	
	param->next = (CPARAM *) alloc_mem ((size_t) sizeof (CPARAM));
	
	*(param->next) = param_noh;
	param->next->next = (CPARAM *) NULL;
	
	return (param_head);
}



 /* 
********************************************************************************* 
* Destroy global param list							* 
********************************************************************************* 
*/ 

void
destroy_param_list (PARAM_LIST *global_param_list)
{
	PARAM_LIST *param_list, *aux;
	
	param_list = global_param_list;
	while (param_list != (PARAM_LIST *) NULL)
	{
		aux = param_list->next;
		free (param_list);
		param_list = aux;
	}
}



/* 
********************************************************************************* 
* Extract param list from commmand						* 
********************************************************************************* 
*/

PARAM_LIST *
get_param_list_from_command (char *parameters, CPARAM *cparam_list)
{
	CPARAM *cparam;
	PARAM_LIST *param_list, *aux;
	char *p;
	
	p = parameters;
	
	aux = param_list = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
	param_list->param.sval = p;
	param_list->next = NULL;
	
	cparam = cparam_list;
	while ((*p != '\0') && (cparam != (CPARAM *) NULL))
	{			
		switch (*p)
		{
			case 'd': /* Integer */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.ival = cparam->param.ival;
				param_list->next->next = NULL;
				param_list = param_list->next;
				cparam = cparam->next;			
				break;
			case 'f': /* Float */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.fval = cparam->param.fval;
				param_list->next->next = NULL;
				param_list = param_list->next;
				cparam = cparam->next;
				break;
			case 'p': /* Pointer */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.pval = cparam->param.pval;
				param_list->next->next = NULL;
				param_list = param_list->next;
				cparam = cparam->next;
				break;
			case 's': /* String */
				param_list->next = (PARAM_LIST *) alloc_mem ((size_t) sizeof (PARAM_LIST));
				param_list->next->param.sval = cparam->param.sval;
				param_list->next->next = NULL;
				param_list = param_list->next;
				cparam = cparam->next;
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
	return (aux);
}
	


/* 
********************************************************************************* 
* Call function									* 
********************************************************************************* 
*/ 

CPARAM
call_function (char *function_name, CPARAM cparam_list)
{
	CPARAM ret_val;
	FUNCTION_LIST *n_list;	
	int return_type;
	NEURON_OUTPUT (*function) (PARAM_LIST *);
	char *parameters;
	PARAM_LIST *param_list;
	
	for (n_list = global_function_list; n_list->next != (FUNCTION_LIST *) NULL; n_list = n_list->next)
	{
		if (strcmp (n_list->function_name, function_name) == 0) 
			break;			
	}

	if (strcmp (n_list->function_name, function_name) != 0)
	{
		strcpy (error_message, "The function ");
		strcat (error_message, function_name);
		strcat (error_message, " doesn't exist in global_function_list.");
		yyerror ("");
	}
		
	return_type = n_list->return_type;
	function    = n_list->function;
	parameters  = n_list->parameters;
		
	param_list = get_param_list_from_command (parameters, &(cparam_list));

	if (return_type == INT_TYPE)
	{
		ret_val.type = INT_TYPE;
		ret_val.param.ival = (function (param_list)).ival;
		ret_val.param.fval = (float) ret_val.param.ival;
	}
	else if (return_type == FLOAT_TYPE)
	{
		ret_val.type = FLOAT_TYPE;
		ret_val.param.fval = (function (param_list)).fval;
		ret_val.param.ival = (int) ret_val.param.fval;
	}
	else
	{
		strcpy (error_message, "The return type of the function ");
		strcat (error_message, function_name);
		strcat (error_message, " called doesn't exist.");
		yyerror ("");
	}	

	destroy_param_list (param_list);	
	return (ret_val);
}



/* 
********************************************************************************* 
* Evaluate command								* 
********************************************************************************* 
*/ 

CPARAM 
evaluate (COMMAND *command) 
{
	CPARAM ret_val;
	CPARAM operand1, operand2;
	int type1, type2;
	
	switch (command->command)
	{
		case C_UMINUS:
			operand1 = evaluate (FIRST_CMD_P);
			type1 = operand1.type;
			
			switch (type1)
			{
				case INT_TYPE:
					ret_val.param.ival = -(operand1.param.ival);
					break;
				case FLOAT_TYPE:
					ret_val.param.fval = -(operand1.param.fval);
					break;
				default:
					strcpy (error_message, "Type doesn't exist.");
					yyerror ("");
					ret_val.param.ival = 0;
					break;
			}
			return (ret_val);
		case C_NOT:
			operand1 = evaluate (FIRST_CMD_P);
			ret_val.param.ival = (operand1.param.ival)? 0: 1;
			return (ret_val);
		case C_EXPRESSION:
			return (evaluate (FIRST_CMD_P));
		case C_CONSTANT_INT:
			ret_val.param.ival = command->parameters->param.ival;
			ret_val.type = INT_TYPE;
			return (ret_val);
		case C_CONSTANT_FLOAT:
			ret_val.param.fval = command->parameters->param.fval;
			ret_val.type = FLOAT_TYPE;
			return (ret_val);
                case C_TIME:
                        ret_val.param.fval = get_time();
                        ret_val.type = FLOAT_TYPE;
                        return (ret_val);
                case C_VARIABLE:
			if (list_member (FIRST_STR_P, int_variables))
			{
				ret_val.param.ival = get_int_variable (FIRST_STR_P);
				ret_val.param.pval = get_address_of_int_variable (FIRST_STR_P);
				command->parameters->type = INT_TYPE;
				ret_val.type = INT_TYPE;
				return (ret_val);
			}
			else if (list_member (FIRST_STR_P, float_variables))
			{	
				ret_val.param.fval = get_float_variable (FIRST_STR_P);
				ret_val.param.pval = get_address_of_float_variable (FIRST_STR_P);	
				command->parameters->type = FLOAT_TYPE;
				ret_val.type = FLOAT_TYPE;
				return (ret_val);
			}
			else
			{
				strcpy (error_message, "Variable ");
				strcat (error_message, FIRST_STR_P);
				strcat (error_message, " not declared.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);
			}
		case C_VARIABLE_VEC:
			if (list_member (FIRST_STR_P, int_vec_variables))
			{				
				ret_val.param.ival = get_int_vec_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival);
				command->parameters->type = INT_TYPE;
				ret_val.type = INT_TYPE;
				return (ret_val);
			}
			else if (list_member (FIRST_STR_P, float_vec_variables))
			{
				ret_val.param.fval = get_float_vec_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival);
				command->parameters->type = FLOAT_TYPE;
				ret_val.type = FLOAT_TYPE;
				return (ret_val);
			}	
			else
			{
				strcpy (error_message, "Variable ");
				strcat (error_message, FIRST_STR_P);
				strcat (error_message, " not declared.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
		case C_DIVIDE:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) / (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) / ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) / (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) / (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_REMINDER_DIVIDE:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) % (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = ((int) operand1.param.fval) % (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) % ((int) operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = ((int) operand1.param.fval) % ((int) operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_MULTIPLY:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) * (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) * ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) * (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) * (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_SUB:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) - (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) - ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) - (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) - (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_ADD:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) + (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) + ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) + (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) + (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_GREATER:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) > (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) > ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) > (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) > (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_GREATER_EQUAL:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) >= (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) >= ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) >= (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) >= (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_LESS:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) < (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) < ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) < (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) < (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_LESS_EQUAL:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) <= (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) <= ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) <= (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) <= (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_EQUAL:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) == (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) == ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) == (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) == (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_DIFFERENT:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) != (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) != ((float) operand2.param.ival);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = ((float) operand1.param.ival) != (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.fval = (operand1.param.fval) != (operand2.param.fval);
				ret_val.param.ival = (int) ret_val.param.fval;
				ret_val.type = FLOAT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			return (ret_val);
		case C_ADD_PARAM:
			return (add_param (evaluate (FIRST_CMD_P)));
		case C_ADD_PARAM3:
			return (*(command->parameters));
		case C_ADD_PARAM2:
			return (add_param2 (evaluate (FIRST_CMD_P), evaluate (SECOND_CMD_P)));
		case C_FUNCTION:
			return (call_function (FIRST_STR_P, evaluate (SECOND_CMD_P)));
		case C_OR:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) || (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.fval) || (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) || (operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = (operand1.param.fval) || (operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		case C_AND:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) && (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.fval) && (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == INT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) && (operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else if ((type1 == FLOAT_TYPE) && (type2 == FLOAT_TYPE))
			{
				ret_val.param.ival = (operand1.param.fval) && (operand2.param.fval);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Type doesn't exist.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		case C_BITWISE_OR:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) | (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Invalid operands to binary |.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		case C_BITWISE_AND:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) & (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Invalid operands to binary &.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		case C_SHIFT_RIGHT:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) >> (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Invalid operands to binary >>.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		case C_SHIFT_LEFT:
			operand1 = evaluate (FIRST_CMD_P);
			operand2 = evaluate (SECOND_CMD_P);
			
			type1 = operand1.type;
			type2 = operand2.type;
			
			if ((type1 == INT_TYPE) && (type2 == INT_TYPE))
			{
				ret_val.param.ival = (operand1.param.ival) << (operand2.param.ival);
				ret_val.param.fval = (float) ret_val.param.ival;
				ret_val.type = INT_TYPE;
			}
			else
			{
				strcpy (error_message, "Invalid operands to binary <<.");
				yyerror ("");
				ret_val.param.ival = 0;
				return (ret_val);			
			}
			
			return (ret_val);
		default:
			yyerror ("unknown expression token");
			ret_val.param.ival = 0;
			return (ret_val);
	}
}



/* 
********************************************************************************* 
* Alloc memory to a vetor variable						* 
********************************************************************************* 
*/ 

void
alloc_vec_var (char *symbol, int list, int size)
{
 	SYMBOL_LIST *s_list; 
 	
	switch (list)
	{
		case INT_LIST:
 			s_list = int_vec_variables;
	
			if (s_list != (SYMBOL_LIST *) NULL) 
			{
				for (; s_list != (SYMBOL_LIST *) NULL; s_list = s_list->next)
				{
					if (strcmp (symbol, s_list->symbol) == 0)
					{
						s_list->symbol_val.ivval = (int *) alloc_mem (size * sizeof(int));
						return;
					}
				} 
			}
			break;
		case FLOAT_LIST:
 			s_list = float_vec_variables;
	
			if (s_list != (SYMBOL_LIST *) NULL) 
			{
				for (; s_list != (SYMBOL_LIST *) NULL; s_list = s_list->next)
				{
					if (strcmp (symbol, s_list->symbol) == 0)
					{
						s_list->symbol_val.fvval = (float *) alloc_mem (size * sizeof(float));
						return;
					}
				} 
			}
			break;
	}
	
	strcpy (error_message, "Variable ");
	strcat (error_message, symbol);
	strcat (error_message, " not declared.");
	yyerror ("");
	return;
}



/* 
********************************************************************************* 
* Execute command								* 
********************************************************************************* 
*/ 
 
void 
exec_command (void *cmd) 
{
	COMMAND *command;
	char *aux_str;
	char num[100];
        CPARAM operand;

	command = (COMMAND *) cmd;
	switch (command->command)
	{
		case C_SERIES:
			exec_command (FIRST_CMD_P); exec_command (SECOND_CMD_P);break;
		case C_WHILE:
			while ((evaluate (FIRST_CMD_P)).param.ival) 
				exec_command (SECOND_CMD_P);
			break;
		case C_DO:
			do
				exec_command (FIRST_CMD_P);
			while ((evaluate (SECOND_CMD_P)).param.ival);
			break;
		case C_IF:
			if ((evaluate (FIRST_CMD_P)).param.ival) 
				exec_command (SECOND_CMD_P);
			break;
		case C_IF_ELSE:
			if ((evaluate (FIRST_CMD_P)).param.ival) 
				exec_command (SECOND_CMD_P);
			else
				exec_command (THIRD_CMD_P);
			break;
		case C_PRINT:
			aux_str = alloc_mem (strlen (FIRST_STR_P) + 1);
			strcpy (aux_str, FIRST_STR_P + 1);
			aux_str[strlen (FIRST_STR_P) - 2] = '\0';

                        operand = evaluate (SECOND_CMD_P);

                        if (operand.type == INT_TYPE)
                            printf ("%s%d\n", aux_str, operand.param.ival);
                        else if (operand.type == FLOAT_TYPE)
                            printf ("%s%f\n", aux_str, operand.param.fval);

                        break;
                case C_TIME:
                        get_time();
                        break;
                case C_ASSIGNMENT:
			if (list_member (FIRST_STR_P, int_variables))
				set_int_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival);
			else if (list_member (FIRST_STR_P, float_variables))
				set_float_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.fval);
			else
			{
				strcpy (error_message, "Variable ");
				strcat (error_message, FIRST_STR_P);
				strcat (error_message, " not declared.");
				yyerror ("");
			}
			break;
		case C_ASSIGNMENT_VEC:
			if (list_member (FIRST_STR_P, int_vec_variables))
				set_int_vec_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival, (evaluate (THIRD_CMD_P)).param.ival);
			else if (list_member (FIRST_STR_P, float_vec_variables))
				set_float_vec_variable (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival, (evaluate (THIRD_CMD_P)).param.fval);
			else
			{
				strcpy (error_message, "Variable ");
				strcat (error_message, FIRST_STR_P);
				strcat (error_message, " not declared.");
				yyerror ("");
			}
			break;
		case C_INT:
			add_symbol (FIRST_STR_P, &int_variables); break;
		case C_FLOAT:
			add_symbol (FIRST_STR_P, &float_variables); break;
		case C_INT_VEC:
			add_symbol (FIRST_STR_P, &int_vec_variables); alloc_vec_var (FIRST_STR_P, INT_LIST, (evaluate (SECOND_CMD_P)).param.ival); break;
		case C_FLOAT_VEC:
			add_symbol (FIRST_STR_P, &float_vec_variables); alloc_vec_var (FIRST_STR_P, FLOAT_LIST, (evaluate (SECOND_CMD_P)).param.ival); break;
		case C_DRAW:
			draw_output (FIRST_STR_P, SECOND_STR_P); break;
		case C_DRAW_OUTPUT:
			draw_output (FIRST_STR_P, ""); break;
		case C_DRAW_Y:
			draw_y_output (FIRST_STR_P, SECOND_STR_P); break;
		case C_DRAW_X:
			draw_x_output (FIRST_STR_P, SECOND_STR_P); break;
		case C_POP:
			pop_input_position (FIRST_STR_P); break;
		case C_PUSH:
			push_input_position (FIRST_STR_P); break;
		case C_TOGGLE_DRAW:
			toggle_draw_active (); break;
		case C_TOGGLE_MOVE:
			toggle_move_active (); break;
		case C_MOVE:
			move_input_window (FIRST_STR_P, (evaluate (SECOND_CMD_P)).param.ival, (evaluate (THIRD_CMD_P)).param.ival); break;
		case C_FORWARD_NETWORK:
#ifdef INTERFACE
   			all_inputs_update ();
#endif
   			all_filters_update ();
			all_dendrites_update (); 
			all_neurons_update ();
			all_outputs_update (); 
			break;
		case C_FORWARD_NEURAL_LAYERS:
			all_dendrites_update (); 
			all_neurons_update ();
			break;
		case C_FORWARD_INPUTS:
   			all_inputs_update ();
			break;
		case C_FORWARD_OUTPUTS:
			all_outputs_update (); 
			break;
		case C_FORWARD_FILTERS:
   			all_filters_update ();
			break;
		case C_CORRELATE:
   			correlate_network (); break;
		case C_PRUNE:
   			prune_network (); break;
		case C_CLEAR_MEMORY:
   			clear_network_memory (); break;
		case C_FORWARD_LIST:
   			forward_objects (aux_str = evaluate_str (FIRST_CMD_P)); free (aux_str); break;
		case C_CORRELATE_LIST:
   			correlate_neural_layers (aux_str = evaluate_str (FIRST_CMD_P)); free (aux_str); break;
		case C_PRUNE_LIST:
   			prune_neural_layers (aux_str = evaluate_str (FIRST_CMD_P)); free (aux_str); break;
		case C_CLEAR_MEMORY_LIST:
   			clear_neural_layers_memory (aux_str = evaluate_str (FIRST_CMD_P)); free (aux_str); break;
		case C_RUN:
			run_network ((evaluate (FIRST_CMD_P)).param.ival); break;
			// run_network_locked ((evaluate (FIRST_CMD_P)).param.ival); break;
		case C_TRAIN_NETWORK:
   			train_network (); break;
   		case C_SPARSE_TRAIN_NETWORK:
   			sparse_train_network (FIRST_FLOAT_P); break;
		case C_DESTROY_NETWORK:
   			destroy_network (); break;	
		case C_TRAIN:
   			train_neuron_layer (FIRST_STR_P); break;
   		case C_SPARSE_TRAIN:
   			sparse_train_neuron_layer (FIRST_STR_P,SECOND_FLOAT_P); break;
		case C_UNLOAD:
   			unload_network(FIRST_STR_P); break;
		case C_UNLOAD_NL:
   			unload_neuron_layer_to_file(SECOND_STR_P, FIRST_STR_P); break;
		case C_RELOAD:
   			reload_network(FIRST_STR_P); break;	
		case C_RELOAD_NL:
   			reload_neuron_layer_from_file(SECOND_STR_P, FIRST_STR_P); break;	
		case C_DESTROY:
			destroy_connections2neuron_layer_by_name (FIRST_STR_P); break;
		case C_RAND:
			rand_output (FIRST_STR_P); break;
		case C_CLEAR:
			clear_output_by_name (FIRST_STR_P); break;
		case C_PASTE:
			paste_to_output (FIRST_STR_P); break;
		case C_COPY:
			copy_input_or_output (FIRST_STR_P); break;
		case C_SAVE:
			save_object_image (SECOND_STR_P, FIRST_STR_P); break;
		case C_LOAD:
			load_image_to_object (FIRST_STR_P, SECOND_STR_P); break;
		case C_TEST_CONN:
   			test_connections (FIRST_STR_P); break;
		case C_TEST_CONN_V:
			test_connections_v (FIRST_STR_P); break;
		//case C_PRINT_THRESHOLD:
		//	print_neuron_synapses_threshold (FIRST_STR_P,SECOND_INT_P*THIRD_INT_P); break;
		case C_KEY:
			f_keyboard (FIRST_STR_P); break;
		case C_KEY_NUM:
                        sprintf (num, "%d", (evaluate (FIRST_CMD_P)).param.ival);
                        f_keyboard (num); break;
		case C_WAIT:
   			wait_display (); break;
		case C_QUIT:
   			exit (0); break;
		case C_PAUSE:
   			fprintf (stderr, "Hit any key.\n"); wait_display (); getchar (); break;
		case C_FUNCTION:
			call_function (FIRST_STR_P, evaluate (SECOND_CMD_P)); break;
		default:
			yyerror ("unknown command token");
	}
}
 
 
 
/* 
********************************************************************************* 
* Concatenate commands								* 
********************************************************************************* 
*/ 
 
void * 
CC (int number, ...) 
{
	//va_list argument;
	int i;
{
	va_list argument;
	COMMAND *command;
	CPARAM **param;
	
	va_start (argument, number);
	command = (COMMAND *) alloc_mem ((size_t) sizeof (COMMAND)); 
	command->command = va_arg (argument, int);

	command->parameters = NULL;
	param = &(command->parameters);
	
	i = 0;
	while (i < number)
	{
		*param = (CPARAM *) alloc_mem ((size_t) sizeof (CPARAM)); 
		switch (va_arg (argument, int))
		{
			case INT_PARAM:		
				(*param)->type = INT_TYPE;
				(*param)->param.ival = va_arg (argument, int);
				(*param)->next = NULL;
				break;
			case FLOAT_PARAM:
				(*param)->type = FLOAT_TYPE;
				(*param)->param.fval = (float) va_arg (argument, double);
				(*param)->next = NULL;
				break;
			case STR_PARAM:			
				(*param)->type = STR_TYPE;
				(*param)->param.sval = va_arg (argument, char *);
				(*param)->next = NULL;
				break;
			case CMD_PARAM:
				(*param)->type = CMD_TYPE;
				(*param)->param.command_param = va_arg (argument, COMMAND *);
				(*param)->next = NULL;
				break;
			default:
				yyerror ("unknown parameter type"); 
		}
		param = &((*param)->next);
		i++;
	}
	
	va_end (argument);
	return ((void *) command);
} 
}



/* 
********************************************************************************* 
* Show error messages								* 
********************************************************************************* 
*/ 

void 
yyerror (char *mens)
{
	show_message (mens, error_message, yytext);
}
