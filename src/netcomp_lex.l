%{
#include "mae.h"
#include "netcomp_yacc.tab.h"

int tokval;
int lineno = 1;
char line_info[50];

char *__aux2;
char *__aux3;
char *__aux4;
%}

%%
int 			return (INT);
float 			return (FLOAT);
const 			return (CONST);
linear			return (LINEAR);
radius			return (RADIUS);
radii			return (RADII);
angle			return (ANGLE);
logfactor		return (LOG_FACTOR);
scalefactor		return (SCALE_FACTOR);
size			return (SIZE);
squared			return (SQUARED);
distribution		return (DISTRIBUTION);
gaussian		return (GAUSSIAN_TOKEN);
log_polar		return (LOG_POLAR_TOKEN);
log_polar_scaled	return (LOG_POLAR_SCALED_TOKEN);
laplacian		return (LAPLACIAN_TOKEN);
regular 		return (REGULAR_TOKEN);
and			return (AND);
input			return (INPUT);
output			return (OUTPUT);
outputs			return (OUTPUTS);
neuronlayer		return (NEURON_LAYER_TOKEN);
of			return (OF);
neurons			return (NEURONS);
filter			return (FILTER);
with			return (WITH);
random			return (RANDOM);
inputs			return (INPUTS);
per			return (PER);
neuron			return (NEURON_TOKEN);
using			return (USING);
shared			return (SHARED);
memory			return (MEMORY);
associate		return (ASSOCIATE);
connect			return (CONNECT);
within			return (WITHIN);
same			return (SAME);
interconnection		return (INTERCONNECTION);
pattern			return (PATTERN_TOKEN);
fullconnect		return (FULL_CONNECT);
outputconnect		return (OUTPUT_CONNECT);
to			return (TO);
while			return (WHILE);
for			return (FOR);
set			return (SET);
color			return (COLOR_TOKEN);
color_selective		return (COLOR_SELECTIVE_TOKEN);
sensitive		return (SENSITIVE);
greyscale		return (GREYSCALE_TOKEN);
greyscale_float		return (GREYSCALE_FLOAT_TOKEN);
b&w			return (BLACK_WHITE_TOKEN);
produced		return (PRODUCED);
producing		return (PRODUCING);
controled		return (CONTROLED);
handled			return (HANDLED);
by			return (BY);
pyramid			return (PYRAMID_TOKEN);
copies			return (COPIES);
show			return (SHOW);
\(			return (OPEN_PARENTESIS);
\)			return (CLOSE_PARENTESIS);
\{			return (OPEN_CHAVES);
\}			return (CLOSE_CHAVES);
\[			return (OPEN_COLCHETES);
\]			return (CLOSE_COLCHETES);
;			return (SEMI_COLON);
,			return (COMA);
\>			return (GREATER);
\<			return (LESS);
=			return (EQUAL);
=\!			return (DIFERENT);
\>=			return (GREATER_EQUAL);
\<=			return (LESS_EQUAL);
\+			return (PLUS);
\-			return (MINUS);
\*			return (TIMES);
\/			return (DIVIDIDED);
\@			return (AT);
\"(\\.|\\\n|[^\\"])*\"	{ __aux2 = (char *) malloc (SYMBOL_SIZE); strcpy (__aux2, yytext); yylval.cpval = (char *) __aux2; return (STRING); }
[0-9]*\.?[0-9]+		{ __aux3 = (char *) malloc (SYMBOL_SIZE); strcpy (__aux3, yytext); yylval.cpval = (char *) __aux3; return (CONSTANT); }
[&a-zA-Z][_a-zA-Z0-9]*	{ __aux4 = (char *) malloc (SYMBOL_SIZE); strcpy (__aux4, yytext); yylval.cpval = (char *) __aux4; return (VARIABLE); }
#.*$			;
[ \t]			;
\n			{ lineno++; sprintf (line_info, "__line = %d;\n", lineno); }
%%
