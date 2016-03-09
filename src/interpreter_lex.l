%{
#include <stdio.h> 
#include <stdlib.h>
#include "mae.h"
#include "interpreter_yacc.tab.h"

char *__aux;

%}
%START	COMMENT
%%
<INITIAL>key			return (KEY_ST);
<INITIAL>test_connections	return (TEST_CONNECTIONS_ST);
<INITIAL>test_connections_v	return (TEST_CONNECTIONS_V_ST);
<INITIAL>copy			return (COPY_ST);
<INITIAL>paste			return (PASTE_ST);
<INITIAL>save			return (SAVE_ST);
<INITIAL>load			return (LOAD_ST);
<INITIAL>clear			return (CLEAR_ST);
<INITIAL>rand			return (RAND_ST);
<INITIAL>train			return (TRAIN_ST);
<INITIAL>sparse_train		return (SPARSE_TRAIN_ST);
<INITIAL>unload			return (UNLOAD_ST);
<INITIAL>reload			return (RELOAD_ST);
<INITIAL>destroy		return (DESTROY_ST);
<INITIAL>run			return (RUN_ST);
<INITIAL>forward		return (FORWARD_ST);
<INITIAL>correlate		return (CORRELATE_ST);
<INITIAL>prune			return (PRUNE_ST);
<INITIAL>move			return (MOVE_ST);
<INITIAL>toggle			return (TOGGLE_ST);
<INITIAL>push			return (PUSH_ST);
<INITIAL>pop			return (POP_ST);
<INITIAL>draw_x			return (DRAW_X_ST);
<INITIAL>draw_y			return (DRAW_Y_ST);
<INITIAL>draw			return (DRAW_ST);
<INITIAL>based			return (BASED);
<INITIAL>output			return (OUTPUT);
<INITIAL>position		return (POSITION);
<INITIAL>network		return (NETWORK);
<INITIAL>neural_layers		return (NEURAL_LAYERS);
<INITIAL>inputs			return (INPUTS);
<INITIAL>outputs		return (OUTPUTS);
<INITIAL>filters		return (FILTERS);
<INITIAL>memory			return (MEMORY);
<INITIAL>move_active		return (MOVE_ACTIVE);
<INITIAL>draw_active		return (DRAW_ACTIVE);
<INITIAL>on			return (ON);
<INITIAL>of			return (OF);
<INITIAL>to			return (TO);
<INITIAL>from			return (FROM);
<INITIAL>wait			return (WAIT);
<INITIAL>quit			return (QUIT);
<INITIAL>display		return (DISPLAY);
<INITIAL>int			return (INT);
<INITIAL>float			return (FLOAT);
<INITIAL>while			return (WHILE);
<INITIAL>do			return (DO);
<INITIAL>if			return (IF);
<INITIAL>else			return (ELSE);
<INITIAL>print			return (PRINT);
<INITIAL>time           return (TIME);
<INITIAL>pause			return (PAUSE);
"/*"				{ 
					BEGIN COMMENT; 
				}
<COMMENT>[^*/]*			;
<COMMENT>"*"			;
<COMMENT>"/"			;
<COMMENT>"*/"			{ 
					BEGIN INITIAL; 
				}
<INITIAL>\(			return (OPEN_PARENTESIS);
<INITIAL>\)			return (CLOSE_PARENTESIS);
<INITIAL>\{			return (OPEN_CHAVES);
<INITIAL>\}			return (CLOSE_CHAVES);
<INITIAL>\[			return (OPEN_COLCHETES);
<INITIAL>\]			return (CLOSE_COLCHETES);
<INITIAL>;			return (SEMI_COLON);
<INITIAL>\:			return (COLON);
<INITIAL>\,			return (COMA);
<INITIAL>\+			return (PLUS);
<INITIAL>\-			return (MINUS);
<INITIAL>\*			return (TIMES);
<INITIAL>\/			return (DIVIDE);
<INITIAL>\%			return (REMINDER_DIVIDE);
<INITIAL>\>			return (GREATER);
<INITIAL>\<			return (LESS);
<INITIAL>=			return (EQUAL);
<INITIAL>==			return (EQUAL_EQUAL);
<INITIAL>\!			return (NOT);
<INITIAL>\!=			return (DIFFERENT);
<INITIAL>\>=			return (GREATER_EQUAL);
<INITIAL>\<=			return (LESS_EQUAL);
<INITIAL>\|\|			return (OR);
<INITIAL>\&\&			return (AND);
<INITIAL>\|			return (BITWISE_OR);
<INITIAL>\&			return (BITWISE_AND);
<INITIAL>\>\>			return (SHIFT_RIGHT);
<INITIAL>\<\<			return (SHIFT_LEFT);
<INITIAL>[0-9]*			{ 
					yylval.int_val = atoi (yytext); 
					return (CONSTANT_INT); 
				}
<INITIAL>[0-9]*\.[0-9]+		{ 
					yylval.float_val = (float) atof (yytext); 
					return (CONSTANT_FLOAT); 
				}
<INITIAL>\"[^"]*\"		{ 
					
					__aux = (char *) alloc_mem ((size_t) yyleng + 1); 
					strcpy (__aux, yytext); 
					yylval.char_pointer_val = (char *) __aux; 
					return (STRING); 
				}
<INITIAL>[a-zA-Z]*[\._a-zA-Z0-9]*	{ 
					__aux = (char *) alloc_mem ((size_t) yyleng + 1); 
					strcpy (__aux, yytext); 
					yylval.char_pointer_val = (char *) __aux; 
					return (VARIABLE); 
				}
\#.*$				;
[ \t\n]				;
%%
