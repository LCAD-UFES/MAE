/*
*********************************************************************************
* Module : Definitions								*
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

#ifndef __MAE_H
#define __MAE_H

#ifdef WINDOWS
#define inline __inline
#endif

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <stdlib.h>
#include <stdarg.h> 
#include <time.h>

typedef unsigned char byte;

/* General Definitions */
#define		DIRECT_CONNECTION	-1	/* direct connection (1 to 1) */
#define		FUL			1	/* full connection (1 to all) */
#define		OUT			2	/* output connection */
#define		RAN			4	/* random connection */
#define		GAU			8	/* connection with random gaussian distribution */
#define		LIN			16	/* connection with random distribution within linear range */
#define		LOG			32	/* connection with random log-polar distribution */
#define		LOG_SCALED		64	/* connection with random log-polar-scaled distribution */
#define		GAU2			128	/* connection with 2D random gaussian distribution */

#define		NEVER_COMPUTED		INT_MIN	/* dendrite state */

/* Interpreter Definitions */
#define		INT_TYPE		0
#define		FLOAT_TYPE		1
#define 	STR_TYPE		2
#define		CMD_TYPE		3

#define		INT_LIST		0
#define		FLOAT_LIST		1

// Output Types
#define		NOT_SPECIFIED		0
#define		COLOR			1
#define		GREYSCALE		2
#define		BLACK_WHITE		3
#define		GREYSCALE_FLOAT		4
#define		COLOR_SELECTIVE		5

// Memory Types
#define		DISTRIBUTED_MEMORY	0
#define		SHARED_MEMORY		1

// Interconnection Pattern Types
#define		STANDARD_INTERCONNECTION_PATTERN	0
#define		DIFFERENT_INTERCONNECTION_PATTERN	1
#define		SAME_INTERCONNECTION_PATTERN		2

// Pyramid Types
#define		REGULAR_PYRAMID	        0
#define		GAUSSIAN_PYRAMID	1
#define		LAPLACIAN_PYRAMID	2

// Partial level masks
#define		NUM_COLOR_CHANNELS		3		//standard 3 channel RGB
#define		NUM_COMPOSED_COLOR_CHANNELS	7		//up to 7 composed color channels
#define		NUM_COLOR_LEVELS	256			//256 color levels per channel
#define 	NUM_COLORS		(256 * 256 * 256)
#define 	NUM_GREYLEVELS		(256 * 256 * 256)
#define		NUM_GREYLEVELS_2	256

// Neuron Masks for pixel noise filtering
#define		NEURON_MASK_COLOR(neuron_output)		(0x00808080 & neuron_output)
#define		NEURON_MASK_GREYSCALE(neuron_output)		(0x000000e0 & neuron_output)
#define		NEURON_MASK_BLACK_WHITE(neuron_output)		(0x00000080 & neuron_output)

#define		NEURON_MASK_COLOR_RED(neuron_output)		((neuron_output & 0x000000f0L))		//0 bytes shift
#define		NEURON_MASK_COLOR_GREEN(neuron_output)		((neuron_output & 0x0000f000L) >> 8)	//1 byte shift
#define		NEURON_MASK_COLOR_BLUE(neuron_output)		((neuron_output & 0x00f00000L) >> 16)	//2 bytes shift	
#define		NEURON_MASK_COLOR_FULL(neuron_output)		((neuron_output & 0x00f0f0f0L))
#define		NEURON_MASK_COLOR_SUM(neuron_output)		((neuron_output & 0x000000f0L) + ((neuron_output & 0x0000f000L) >> 8) + ((neuron_output & 0x00f00000L) >> 16))

// Full level masks (for complete colors and grayscale levels) 
#define		NEURON_FULL_MASK_COLOR(neuron_output)			(0x00ffffff & neuron_output)
#define		NEURON_FULL_MASK_GREYSCALE(neuron_output)		(0x000000ff & neuron_output)

#define		NEURON_FULL_MASK_GREYSCALE2(neuron_output)		(float)(0x000000ff & neuron_output)
#define		NEURON_FULL_BLACK_WHITE(neuron_output)			(0x00000080 & neuron_output) //same as BW mask

#define		FILE_NAME_SIZE		256

#define		MAX_CONNECTIONS		4096

#if defined (__SSE4_2__) || defined (__ABM__)			/* If Intel SSE4.2 or AMD ABM avalilable -> */
#define		PATTERN_UNIT_SIZE			64			/* unsigned long long int bits */
#define		PATTERN						unsigned long long int	/* unsigned long long int as a pattern type */
#else								
#define         PATTERN_UNIT_SIZE       8              /* byte size bits */
#define         PATTERN                 unsigned char  /* unsigned char(byte) as a pattern type */
//#define         PATTERN_UNIT_SIZE       16			/* 2 bytes size bits */
//#define         PATTERN                 unsigned short int	/* unsigned short int (2 bytes) as a pattern type */
#endif					

//Cache prefetch stride size in bytes
#define PREFETCH_STRIDE 16	//16B = 128b

//Choose one out of three below
//#define FAT_FAST_HASH_SIZE_AUTO
#define SUB_TABLE_SIZE_16
//#define SUB_TABLE_SIZE_8

#if defined (SUB_TABLE_SIZE_16)
#define         SUB_PATTERN_UNIT_SIZE	16
#define		SUB_PATTERN		unsigned short int
#elif defined (SUB_TABLE_SIZE_8)
#define         SUB_PATTERN_UNIT_SIZE	8
#define		SUB_PATTERN		unsigned char
#else
#define		SUB_PATTERN		unsigned short int
#endif

#define 	N_PATTERNS		(((connections % PATTERN_UNIT_SIZE) == 0) ? \
			 		(connections / PATTERN_UNIT_SIZE): (connections / PATTERN_UNIT_SIZE) + 1)

#define		MAX_N_PATTERNS		MAX_CONNECTIONS/PATTERN_UNIT_SIZE

// max and min Macros
#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#define		NEURON_LAYER_OBJECT	0
#define		INPUT_OBJECT		1
#define		OUTPUT_OBJECT		2
#define		FILTER_OBJECT		3

#define		SYMBOL_SIZE		100

#define		OUTPUT_TYPE		int
#define		SENSITIVITY_TYPE	int
#define		MEMORY_TYPE		int

#define		INTERCONNECTION_PATTERN_TYPE	int
#define		NEURON_HASH_TYPE		void*		//can be either long long ints or just int

#define 	LRAND48_MAX	((unsigned int) -1 >> 1) 

#define 	NUM_SIZE 20 

#define 	SET_POSITION	0
#define 	FORWARD		1 /* indicates that some specific function was called during the forward interpreter command */
#define 	RUN		2 /* indicates that some specific function was called during the run interpreter command */
#define 	MOVE		3

/* Organization of Cone Inputs */
#define 	MCELLS		1
#define 	BLUE_YELLOW	2 
#define 	YELLOW_BLUE	4 
#define 	RED_GREEN	8 
#define 	GREEN_RED	16 

/* Concetrically Organized Regions */
#define 	ONCENTER	-1
#define 	OFFCENTER	0

/* Color Model */
#define 	RGB		3
#define 	HSI		5

/* Math Constants */
#define 	pi				M_PI
#define		GAMMA_CORRECTION_CONST		0.45454545454

/* Image Types */
#define 	LOG_POLAR			1
#define 	NORMAL				0
#define 	NORMALIZED			-1
#define 	NOT_NORMALIZED			1
#define 	LEFT				0
#define 	RIGHT				1
#define		WITH_FREQUENCY_FILTER 		0
#define		WITHOUT_FREQUENCY_FILTER 	1
#define 	P2				0
#define		P3				1
#define 	P5				4
#define 	P6				2
#define		BMP				3

/* log factor */
#define         SERENO_MODEL    -1.0

/* Distribution Type */
#define CONSTANT_DISTRIBUTION	0
#define STATISTIC_DISTRIBUTION	1
#define GAUSSIAN_DISTRIBUTION	2

/* Kernels Definitions */
#define		SMALL		0
#define 	MEDIUM		1
#define 	BIG		2
#define		ALL		3
#define	        KERNEL_1D       0
#define	        KERNEL_2D       1

/* IO Display */
#define 	RED(pixel)	((pixel & 0x000000ff) >> 0)
#define 	GREEN(pixel)	((pixel & 0x0000ff00) >> 8)
#define 	BLUE(pixel)	((pixel & 0x00ff0000) >> 16)
#define 	PIXEL(r, g, b)  (((r & 0x000000ff) << 0) | ((g & 0x000000ff) << 8) | ((b & 0x000000ff) << 16))

/* Type Show */
#define		SHOW_FRAME	0
#define		SHOW_WINDOW	1

/* Type Moving Frame */
#define		MOVING_CENTER	0
#define		STOP		1

/* Mouse Definitions */
#define		NO_BUTTON	-1
#define		NO_STATE	-1

/* Mapping Definitions */
#define MAP_PLAN		0
#define MAP_LOG_POLAR		1

/* Buttons Definitions */
#define DISABLE			0
#define ENABLE			1

/* Boolean Definitions */
#ifndef TRUE
#define TRUE			1
#endif
#ifndef FALSE
#define FALSE			0
#endif
#define BOOLEAN			int

/* Vergence Scan Type Definitions */
#define LINEAR_SCAN             0
#define COARSE2FINE_SCAN        1

/* Measure Time Macro */
#define ACTIVATE_MEASURE_TIME

#ifdef ACTIVATE_MEASURE_TIME
#define MEASURE_TIME(message, body) \
			{time_t start, end; \
			 start = time (NULL); \
			 body \
			 end = time (NULL); \
			 printf ("%s: %f s\n", message, difftime (end, start));}
#else
#define MEASURE_TIME(body, message) body
#endif

/* extern "C" definition needed due to cross-compilation with C++ code, must be kept */
#ifdef __cplusplus
extern "C" {
#endif

/* fread, fwrite e fgets wrappers for handling the */
#define MAE_FREAD_WRAPPER(value,function) \
	{ if(!value) { \
	printf("Error, fread returned 0 in %s function.\n", function); } \
	}

#define MAE_FGETS_WRAPPER(value,function) \
	{ if(!value) { \
	printf("Error, fgets returned 0 in %s function.\n", function); } \
	}

#define MAE_FSCANF_WRAPPER(value,function) \
	{ if(!value) { \
	printf("Error, fscanf returned 0 in %s function.\n", function); } \
	}

#define MAE_SYSTEM_WRAPPER(value,function) \
	{ if(value == -1) { \
	printf("Error, sytem returned -1 in %s function.\n", function); } \
	}

/* extern "C" definition needed due to cross-compilation with C++ code, must be kept */

union _neuron_output
    {
    	int		ival;
	float 		fval;
    };
typedef union _neuron_output NEURON_OUTPUT;

union _synapse_sensitivity_and_displacement
    {
	int		ival;
	float		fval;
	double		dval;
    };
typedef union _synapse_sensitivity_and_displacement SYNAPSE_SENSITIVITY_AND_DISPLACEMENT;

struct	_interconnection_pattern_vector
    {
	SYNAPSE_SENSITIVITY_AND_DISPLACEMENT	rand_x;
	SYNAPSE_SENSITIVITY_AND_DISPLACEMENT	rand_y;
	SYNAPSE_SENSITIVITY_AND_DISPLACEMENT	sensitivity;
	BOOLEAN		initialized;
    };
typedef struct _interconnection_pattern_vector INTERCONNECTION_PATTERN_VECTOR;

union _symbol_val
    { 
	int 		ival;
	float 		fval;
	int 		*ivval;
	float		*fvval;
    }; 
typedef union _symbol_val SYMBOL_VAL;

struct _symbol_list 
    { 
	char 		symbol[SYMBOL_SIZE]; 
	SYMBOL_VAL	symbol_val;
	struct 		_symbol_list *next; 
    }; 
 
typedef struct _symbol_list SYMBOL_LIST; 

struct _xy_pair
    { 
     	int 			x; 
     	int 			y;
    }; 
typedef struct _xy_pair XY_PAIR;

struct _rectangle
    { 
     	float 			x; 
     	float 			y;
     	float 			w; 
     	float 			h;
	float 			r, g, b;
    }; 
typedef struct _rectangle RECTANGLE;

typedef struct
    {
     	PATTERN			*pattern;
     	PATTERN			*valid_bits;
        NEURON_OUTPUT		associated_value;
        int			confidence;	
    } ASSOCIATION;
 

typedef struct
    {
     	NEURON_OUTPUT		dendrite_state;
	NEURON_OUTPUT		output;
	short int		n_out_connections;
	short int		n_in_connections;
 	PATTERN			**pattern_xor_table;
	void 			*synapses; 		/* list of synapses */
	ASSOCIATION	 	*memory;
	int			*memory_index;
	NEURON_HASH_TYPE	*hash;
	int			id;
	struct _neuron_layer	*holder_neuron_layer;	/* pointer to holder neuron layer */
	
	int			last_hamming_distance;	/* the last minimum hamming distance */
	int			last_best_pattern; /* the last pattern that generate the min hamming distance */
	int 			last_num_candidates;    /* the number of patterns that generate the min hamming distance */
	double			time_metrics;
	int			hash_collisions;
	int			memory_size;
    } NEURON;

/** \brief Neuron layer structure
Neuron layer structure */
struct _neuron_layer
    { 
    	char 			*name;
     	XY_PAIR 		dimentions;
     	int                     num_neurons;
	int			id;
	NEURON 			*neuron_vector;
	NEURON 			*associated_neuron_vector;
	OUTPUT_TYPE 		output_type;
	SENSITIVITY_TYPE	sensitivity;				/* Input Sensitivity - if not specified is the output type of the neuron_layer to wich is connected. */
	MEMORY_TYPE		memory_t;				/* Memory Type - it is set to distributed memory on default ... */
	NEURON_OUTPUT 		max_neuron_output;
	NEURON_OUTPUT 		min_neuron_output;
	struct _neuron_type
	{
		void 		(*compute_input_pattern) (NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type);
		NEURON_OUTPUT 	(*neuron_memory_read) (struct _neuron_layer *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type);
		int		(*neuron_memory_write) (struct _neuron_layer *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type);
		int		(*find_nearest_pattern) (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance);
		void		(*initialise_neuron_memory) (struct _neuron_layer *neuron_layer, int n);
		void 		(*initialize_input_pattern) (PATTERN **input_pattern, int connections);
	} 	*neuron_type;					/* Neuron layer - Neuron type descriptor */  
    };
typedef struct _neuron_layer 	NEURON_LAYER;
typedef struct _neuron_type 	NEURON_TYPE;

typedef struct
{
	NEURON_OUTPUT		sensitivity;	// Synapse sensitivity type
	float			weight;		// Synapse weight value
	NEURON 			*source;	// Source neuron
	NEURON_LAYER		*nl_source;	// Source neuron layer
} SYNAPSE;

typedef struct _output_desc OUTPUT_DESC;

typedef struct _input_desc INPUT_DESC;

union _param
    { 
    	int		ival;
	float		fval;
	char		*sval;
	void		*pval;
	NEURON_LAYER	*neuron_layer;
	OUTPUT_DESC 	*output;
	INPUT_DESC 	*input;
    }; 
typedef union _param PARAM;

struct _param_list
    { 
     	PARAM	 		param; 
	struct _param_list 	*next;
    }; 
typedef struct _param_list PARAM_LIST;

struct _function_list
    { 
	NEURON_OUTPUT		(*function) (PARAM_LIST *); 
     	char 			*function_name; 
	char 			*parameters; 
	int 			return_type;
	struct _function_list 	*next;
    }; 
typedef struct _function_list FUNCTION_LIST;

struct _output_desc
    { 
    	char 		*name;
     	NEURON_LAYER 	*neuron_layer;	/* Output - Associated Neuron Layer - Both GPU and CPU outputs alloced in CUDA */
	int 		win; 		/* GLUT window identifier */
	GLubyte 	*image;		/* image is an array that holds the images show in MAE outputs.
					   Frequently it is an OpenGL texture painted over a retangle
					   which is seen with a camera at an certain distance. That is
					   the method we have used to display flat images in MAE. */
	int 		tex; 		/* OpenGL texture identifier */
					   
	GLint 		wh;		/* window height - the window holds the whole output */
	GLint 		ww;		/* window width - the window holds the whole output */

	GLint 		tfw;  		/* width of the texture frame - An OpenGL texture is used to hold 
					   most images shown in MAE outputs. Must be a power of two (OpenGL 
					   requirement). */
	GLint 		tfh;  		/* height of the texture frame - An OpenGL texture is used to hold 
					   most images shown in MAE outputs. Must be a power of two (OpenGL 
					   requirement). */
					   
	GLint 		wxd;		/* window x coordinate when mouse button down */
	GLint 		wyd;		/* window y coordinate when mouse button down */
	GLint 		wxd_old;	/* previous window x coordinate when mouse button down */
	GLint 		wyd_old;	/* previous window y coordinate when mouse button down */
	GLint		x;		/* x mouse coordinate (=parameter of inputmouse) */
	GLint		y;		/* y mouse coordinate (=parameter of inputmouse) */
	GLint		x_old;		/* previous x mouse coordinate (=parameter of inputmouse) */
	GLint		y_old;		/* previous y mouse coordinate (=parameter of inputmouse) */
	GLint 		valid;		/* valid wxd wyd */
	GLint 		wxme;		/* window x coordinate when mouse event happened */
	GLint 		wyme;		/* window y coordinate when mouse event happened */

	XY_PAIR		*cross_list;	/* list of points to be ovelayed by a cross in the output image */
	int		cross_list_size;
	int		cross_list_colored;

	RECTANGLE	*rectangle_list;/* list of rectangles to be ovelayed in the output image */
	int		rectangle_list_size;
	
	int 		mouse_button;
	int 		mouse_state;
	void 		(*output_handler) (struct _output_desc *, int type_call, int mouse_button, int mouse_state);
	PARAM_LIST	*output_handler_params;
    }; 


struct _input_desc
    { 
    	char 		*name;
	char		image_file_name[256];
     	NEURON_LAYER 	*neuron_layer;	/* Input - Associated Neuron Layer */
	int 		win; 		/* GLUT window identifier */
	GLubyte 	*image;		/* image is an array that holds the images show in MAE inputs.
					   Frequently it is an OpenGL texture painted over a retangle
					   which is seen with a camera at an certain distance. That is
					   the method we have used to display flat images in MAE. */
	
	int		max_range;	/* PNM image format, interval of values that one pixel can assume */
	int		image_type;	/* PNM image format, can be P2 for greyscale images or P3 for 
					   color images */
	int             pyramid_type;   /* pyramid type: REGULAR_PYRAMID, GAUSSIAN_PYRAMID or LAPLACIAN_PYRAMID */
	int             pyramid_height; /* pyramid levels number */
	int		up2date;	/* up to date input neuron layer flag ->  */
	char		*string;	/* input string */
	int 		tex; 		/* OpenGL texture identifier */
					   
	GLint 		wx;		/* window x coordinate of the neuron_layer origin - the window 
					   holds the whole input */
	GLint 		wy;		/* window y coordinate of the neuron_layer origin - the window 
					   holds the whole input */

	GLint 		wh;		/* window height - the window holds the whole input */
	GLint 		ww;		/* window width - the window holds the whole input */

	GLint 		tfw;  		/* width of the texture frame - An OpenGL texture is used to hold 
					   most images shown in MAE inputs. Must be a power of two (OpenGL 
					   requirement). */
	GLint 		tfh;  		/* height of the texture frame - An OpenGL texture is used to hold 
					   most images shown in MAE inputs. Must be a power of two (OpenGL 
					   requirement). */
	GLint 		vpw; 		/* width of the visible part of the window - With some configurations, 
					   only a retangular region of the input window is shown. */
	GLint 		vph; 		/* height of the visible part of the window - With some configurations,
					   only a retangular region of the input window is shown. */
					   
	GLint 		vpxo; 		/* window x origin of the visible part of the window - With some 
					   configurations, only a retangular region of the input window 
					   is shown. */
	GLint 		vpyo; 		/* window y origin of the visible part of the window - With some 
					   configurations, only a retangular region of the input window 
					   is shown. */
					   
	GLint 		wxd;		/* window x coordinate when mouse button down */
	GLint 		wyd;		/* window y coordinate when mouse button down */
	GLint 		wxd_old;	/* previous window x coordinate when mouse button down */
	GLint 		wyd_old;	/* previous window y coordinate when mouse button down */
	GLint		x;		/* x mouse coordinate (=parameter of inputmouse) */
	GLint		y;		/* y mouse coordinate (=parameter of inputmouse) */
	GLint		x_old;		/* previous x mouse coordinate (=parameter of inputmouse) */
	GLint		y_old;		/* previous y mouse coordinate (=parameter of inputmouse) */
	GLint 		valid;		/* valid wxd wyd */
	GLint 		wxme;		/* window x coordinate when mouse event happened */
	GLint 		wyme;		/* window y coordinate when mouse event happened */
	GLint 		green_cross_x;	/* green cross x coordinate */
	GLint 		green_cross_y;	/* green cross y coordinate */
	int 		green_cross;	/* green cross indicator active */

	XY_PAIR		*cross_list;	/* list of points to be ovelayed by a cross in the input image */
	int		cross_list_size;
	int		cross_list_colored;
	
	RECTANGLE	*rectangle_list;/* list of rectangles to be ovelayed in the input image */
	int		rectangle_list_size;
	
	int 		mouse_move;
	int 		mouse_button;
	int 		mouse_state;
	int 		waiting_redisplay;
	void 		(*input_generator)(struct _input_desc *, int status);
	PARAM_LIST	*input_generator_params;
	void 		(*input_controler)(struct _input_desc *, int status);
	PARAM_LIST	*input_controler_params;
}; 
 
struct _synapse_list
    {
	SYNAPSE 		*synapse;
	struct _synapse_list 	*next;
    };
typedef struct _synapse_list SYNAPSE_LIST;


struct _neuron_layer_list
    {
	NEURON_LAYER			*neuron_layer;
	struct _neuron_layer_list 	*next;
    };
typedef struct _neuron_layer_list NEURON_LAYER_LIST;


struct _filter_desc
    { 
    	char 			*name;
     	NEURON_LAYER 		*output; 
	NEURON_LAYER_LIST 	*neuron_layer_list;
	void 			(*filter)(struct _filter_desc *);
	PARAM_LIST		*filter_params;
	void			*private_state;
    };
typedef struct _filter_desc FILTER_DESC; 


struct _filter_list
    {
	FILTER_DESC		*filter_descriptor;
	struct _filter_list 	*next;
    };
typedef struct _filter_list FILTER_LIST;


struct _input_list 
    { 
	INPUT_DESC 		*input; 
	struct _input_list 	*next; 
    }; 
typedef struct _input_list INPUT_LIST; 


struct _output_list 
    { 
	OUTPUT_DESC 		*output; 
	struct _output_list 	*next; 
    }; 
typedef struct _output_list OUTPUT_LIST; 


struct _command_list 
    { 
	char 			*command; 
	struct _command_list 	*next; 
    }; 
typedef struct _command_list COMMAND_LIST; 


struct _measuraments 
    { 
	int	neuron_mem_access; 
	int	neuron_mem_hits_at_distance[MAX_CONNECTIONS]; 
    }; 
typedef struct _measuraments MEASURAMENTS; 


struct _correlation
    { 
	int	order; 
	int	importance; 
	float	correlation_measure; 
    }; 
typedef struct _correlation CORRELATION; 

// syn_neu.c Neuron types headers must be specified here
extern NEURON_TYPE default_type;
extern NEURON_TYPE mem_overwrite;
extern NEURON_TYPE rnd_mem_type;
extern NEURON_TYPE logiredu;
extern NEURON_TYPE minchinton;
extern NEURON_TYPE wisard;
extern NEURON_TYPE minchinton_fat_fast;
extern NEURON_TYPE minchinton_fat_fast_index;
extern NEURON_TYPE minchinton_fat_fast_rand;
extern NEURON_TYPE minchinton_fat_fast_overwrite;
extern NEURON_TYPE correlate;
extern NEURON_TYPE minchinton_kcorrelate;
extern NEURON_TYPE label;

// __line variable for NADL compiling s
extern int   __line;

#ifdef FAT_FAST_HASH_SIZE_AUTO
extern int SUB_PATTERN_UNIT_SIZE;
#endif
extern int NEURON_MEMORY_INDEX_SIZE;
extern int NEURON_MEMORY_SIZE;
extern int N_SYNAPSES;

// Dummy value for selective neuron layer training 
extern int DUMMY_OUTPUT_ACTIVATION_VALUE;

extern long   NUM_ISOLATE_NEURON_LAYERS;
extern long   NUM_INPUTS;
extern long   NUM_OUTPUTS;
extern long   NUM_FILTERS;

extern NEURON_LAYER_LIST *global_neuron_layer_list;
extern INPUT_LIST *global_input_list;
extern OUTPUT_LIST *global_output_list;
extern FILTER_LIST *global_filter_list;
extern FUNCTION_LIST *global_function_list;

extern FILE *neural_connections;
extern int n_inputs;
extern int n_outputs;
extern int n_n_layers;
extern NEURON_LAYER **n_layer_vec;
extern int total_n_neurons;
extern int n_connections_per_neuron;
extern long EPOCH;

//extern double delay;
extern unsigned int delay;

extern void build_network (void);
extern void outputs_update (void);

extern int interpreter_pipe[2];
extern FILE *interpreter_pipe_in;
extern FILE *interpreter_pipe_out;
extern FILE *yyin;
extern int remaining_steps;

extern int move_active;
extern int draw_active;

extern char status_info[256];
extern INPUT_DESC *selected_input;
extern OUTPUT_DESC *selected_output;

extern COMMAND_LIST *command_list;
extern COMMAND_LIST *command_list_tail;

extern int wx_saved;
extern int wy_saved;
extern int x_saved;
extern int y_saved;

extern NEURON neuron_with_output_zero;

extern char *cmdl_script;

extern int decoration_size_x, decoration_size_y;

extern PATTERN pattern_xor_table[256];
//extern PATTERN pattern_xor_table[65536];	//for using 16-bit counting based tables

extern MEASURAMENTS measuraments;

extern unsigned int g_seed;			//the seed for the mae_fastrand() function

extern int running;

extern int KERNEL_SIZE;

extern double threshold;

extern int delta_x;

extern double SELECTIVITY;

extern double RANGE;

extern double SIGMA;

extern double FOCAL_DISTANCE;

extern double CAMERA_DISTANCE;

extern int TYPE_SHOW;

extern int CALL_UPDATE_FUNCTIONS;

extern int TYPE_MOVING_FRAME;

extern double CUT_POINT;

extern BOOLEAN hide_interface;

extern int PYRAMID_LEVEL;

extern int VERGENCE_SCAN_TYPE;

extern double LOG_POLAR_SCALE_FACTOR;

extern double LOG_POLAR_THETA_CORRECTION;

extern int global_argc;		//Globally visible argc

extern char **global_argv;	//Globally visible argv

/* prototypes */

char	*C (char *, ...);
void	*alloc_mem (size_t);
void	*realloc_mem (size_t,void*);
void	*alloc_mem_nn (size_t);
void	init_color_tables (void);
void	Erro (char *, char *, char *);
void	initialise_neuron (NEURON_LAYER *, int, int);
void	add_synapse (NEURON *, SYNAPSE *);
void	reinitialize_synapse (SYNAPSE *, NEURON_LAYER *, NEURON *, NEURON_OUTPUT , float );
//void	initialise_synapse (SYNAPSE*, NEURON_LAYER*,NEURON*,NEURON*,NEURON_OUTPUT,SENSITIVITY_TYPE);	//modified
void	initialise_synapse (SYNAPSE*, NEURON_LAYER*, NEURON*, NEURON* , NEURON_OUTPUT, SENSITIVITY_TYPE ,INTERCONNECTION_PATTERN_TYPE, int);		//modified
void	initialise_neuron_vector (NEURON_LAYER *, int);
void	initialise_memory (void);
INPUT_DESC	*get_input_by_win (int);
INPUT_DESC	*get_input_by_name (char *);
INPUT_DESC	*get_input_by_neural_layer (NEURON_LAYER *neuron_layer);
OUTPUT_DESC	*get_output_by_win (int);
OUTPUT_DESC	*get_output_by_name (char *);
OUTPUT_DESC	*get_output_by_neural_layer (NEURON_LAYER *neuron_layer);
NEURON_LAYER	*get_neuron_layer_by_name (char *neuron_layer_name);
GLubyte	*get_image_via_neuron_layer (NEURON_LAYER *neuron_layer);
FILTER_DESC	*get_filter_by_name (char *filter_name);
FILTER_DESC	*get_filter_by_output (NEURON_LAYER *output);
void	clear_output_by_name (char *name);
void	create_neurons (NEURON_LAYER *, char *);
void	connect_neurons (int type, NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst, 
		      	 int num_connections_per_neuron, double sub_region_size, double sub_region_size_2, double sub_region_size_3,
		      	 int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, INTERCONNECTION_PATTERN_TYPE ic_pt_type);
void	associate_neurons (NEURON_LAYER *associated_neuron_layer, NEURON_LAYER *neuron_layer);
void	create_inputs (INPUT_DESC *, char *);
void	create_outputs (OUTPUT_DESC *, char *);
void	output_update (OUTPUT_DESC *output);
void	all_outputs_update (void);
void	input_update (INPUT_DESC *);
void	all_inputs_update (void);
void	all_synapses_update (void);
void	all_neurons_update (void);
long	get_num_neurons (XY_PAIR);
void	set_neurons (NEURON *, int, int, int);
void	setx_neurons (NEURON *, int, int, int, int, int);
void	sety_neurons (NEURON *, int, int, int, int, int);
void	randomize_output_image (OUTPUT_DESC *output);
void	draw_output (char *, char *);
void	run_cmdl_script (void);
void	input_display (void);
void	input_passive_motion (int x, int y);
void	input_mouse (int button, int state, int x, int y);
void	keyboard (unsigned char key, int x, int y);
void	input_reshape (int w, int h);
void	check_input_bounds (INPUT_DESC *input, int wx, int wy);
void	init_pattern_xor_table (void);
void	init_measuraments (void);
void	print_measuraments (void);
void	run_network (int run_steps);
void	run_network_locked (int run_steps);
int	init_user_functions (void);
int	read_window_position (char *window_name, int *x, int *y);
void	interpreter (char *command);
void	execute_script (char *file_name);
void	interprete_many_commands_in_a_file (char *commands, char *filename);
void	update_input_neurons (INPUT_DESC *input);
void	update_output_neurons (OUTPUT_DESC *output);
void	update_output_image (OUTPUT_DESC *output);
void	update_input_image (INPUT_DESC *input);
void	rand_output_image (OUTPUT_DESC *output);
int	yyparse (void);
void	all_filters_update (void);
void	all_dendrites_update (void);
int	get_object_by_name (char *name, void **object);
void	filter_update (FILTER_DESC *filter_descriptor);
int	same_dimentions (XY_PAIR dim1, XY_PAIR dim2);
void	create_input_window (INPUT_DESC *input);
void	create_output_window (OUTPUT_DESC *output);
void	reconnect_2_neurons (NEURON_LAYER *, NEURON *, NEURON *, NEURON_OUTPUT , float);
//void	connect_2_neurons (NEURON_LAYER *neuron_layer_src, NEURON *nsrc, NEURON *ndest, NEURON_OUTPUT synapse_sensitivity,SENSITIVITY_TYPE neuron_layer_sensitivity);
void	connect_2_neurons (NEURON_LAYER*, NEURON*, NEURON*, NEURON_OUTPUT, SENSITIVITY_TYPE, INTERCONNECTION_PATTERN_TYPE, int); //modified
char	*get_name (char *list, char *name);
int	find_nearest_pattern_correlate (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance);
void	output_passive_motion (int x, int y);
void	clear_network_memory(void);
PARAM_LIST	*get_param_list (char *parameters, va_list *argument);
void	create_interpreter_user_function (int return_type, NEURON_OUTPUT (* function) (PARAM_LIST *), char *function_name, char *parameters);
void	map_layers2id (void);
void	count_num_neurons (void);
void	get_wisard_table_size_and_log2_table_size_and_number_of_tables( int*, int*, int, NEURON*);
void	mae_fastrand_srand(int);
int	mae_fastrand();

// Synapse thresholding generation
int	random_integer(int low, int high);
float	random_float(float low,float high);
float	evaluate_gaussian_function(float,float,float,float);
float		evaluate_normal_distribution(float,float,float);

// Build network function requires these prototypes
void create_input (INPUT_DESC *input, int x_size, int y_size, int output_type, int pyramid_height, int input_type, 
	      void (*input_generator)(INPUT_DESC *, int input_generator_status), 
	      void (*input_controler)(INPUT_DESC *, int input_controler_status), ...);
void create_output (OUTPUT_DESC *output, int x_size, int y_size, void (*output_handler)(OUTPUT_DESC *,
	       int type_call, int mouse_button, int mouse_state), char *parameters, ...);
void create_io_windows (void);
void create_filter (void (* filter) (FILTER_DESC *), NEURON_LAYER *output, int num_layers, ...);
void output_connect (NEURON_LAYER *neuron_layer_src, OUTPUT_DESC *output);

//Prefetch functions headers
void mae_long_prefetch_high_priority(void*,size_t); // len is the lenght in bytes
void mae_long_prefetch_low_priority(void*,size_t);

// User functions requires these prototypes in some examples    
int number_neurons_on (NEURON *n, int n0, int num_neurons);
void move_input_window (char *name, int wx, int wy);
void load_input (INPUT_DESC *input);
void make_input_image (INPUT_DESC *input);
void input_init (INPUT_DESC *input);
void copy_neuron_outputs (NEURON_LAYER *dest, NEURON_LAYER *source);
int save_image ();
void save_object_image (char *file_name, char *object_name);
void load_image_to_object (char *object_name, char *file_name);
void optimized_2d_point_convolution (NEURON_LAYER *nl_output, NEURON_LAYER *nl_input, float *kernel_points, float normalizer, int num_points, int kernel_type, int input_type);
double map_v1_to_image_angle (int u, int v, int w, int h); 
double distance_from_image_center (double wi, double hi, double w, double h, double u, double log_factor);
void map_v1_to_image (int *xi, int *yi, double wi, double hi, double u, double v, double w, double h, double x_center, double y_center, double correction, double log_factor);

// Graph functions prototypes
void graph_display (void);
void graph_passive_motion (int x, int y);
void graph_reshape (int w, int h);
void graph_mouse (int button, int state, int x, int y);
void create_neuron_layer (NEURON_LAYER *neuron_layer, 
		NEURON_TYPE *neuron_type, SENSITIVITY_TYPE sensitivity, 
		OUTPUT_TYPE output_type, int x_size, int y_size,
		MEMORY_TYPE memory, int memory_size );

void char_input_display (void);	

// I/O prototypes
void input_display ();
void output_display ();
int nearest_power_of_2 (int value);
void update_input_status (INPUT_DESC *input);
void load_input_image_from_pnm_file (INPUT_DESC *input, FILE *image_file);
void load_input_image_with_p2_format (INPUT_DESC *input, FILE *image_file);
void load_neuron_layer_with_p2_format (NEURON_LAYER *neuron_layer, FILE *image_file);
void load_input_image_with_BMP_format (INPUT_DESC *input, FILE *image_file);
void load_neuron_layer_with_BMP_format (NEURON_LAYER *neuron_layer, FILE *image_file);
void load_input_image (INPUT_DESC *input, char *file_name);
void load_neuron_layer (NEURON_LAYER *neuron_layer, char *file_name);
void save_neuron_layer (NEURON_LAYER *neuron_layer, char *file_name);
void load_neuron_layer_from_file (NEURON_LAYER *neuron_layer, char *file_name);
void input_forward (INPUT_DESC *input);
void output_forward (OUTPUT_DESC *output);

// Interface access functions prototypes
void check_forms (void);
void show_message (char *msg1, char *msg2, char *msg3);
void set_status_bar (char *status_info);
void set_mouse_bar (char *mouse_pos);
void set_color_bar (char *color_val);
void clear_browser (void);
void addto_browser (char *cmd);
void evaluate_output_rate (void);
void toggle_move_active (void);
void toggle_draw_active (void);
void set_move (int status);
void set_draw (int status);

// Interpreter functions prototypes
void draw_x_output (char *output_name, char *input_name);
void draw_y_output (char *output_name, char *input_name);
void pop_input_position (char *input_name);
void push_input_position (char *input_name);
void correlate_network (void);
void prune_network (void);
void forward_objects (char *objects);
void correlate_neural_layers (char *neural_layers);
void prune_neural_layers (char *neural_layers);
void clear_neural_layers_memory (char *neural_layers);
void train_network (void);
void sparse_train_network (float percentage);
void unload_network (char *file_name);
void unload_neuron_layer_to_file (char *file_name, char *neuron_layer_name);
void reload_network (char *file_name);
void reload_neuron_layer_from_file (char *file_name, char *neuron_layer_name);
void destroy_connections2neuron_layer_by_name (char *neuron_layer_name);
void rand_output (char *name);
void paste_to_output (char *name);
void copy_input_or_output (char *name);
void test_connections (char *output_name);
void f_keyboard (char *key_value);
void wait_display (void);
void destroy_network (void);
void train_neuron (NEURON_LAYER *neuron_layer, int n);
void train_neuron_layer (char *name);
void train_neuron_layer_selective (char *name);
void sparse_train_neuron_layer (char *name,float percentage);
float count_neuron_layer_average_candidates (char *neuron_layer_name);
float count_neuron_layer_average_hash_collisions (char *neuron_layer_name);
void print_neuron_synapses_threshold (char *neuron_layer_name, int neuron_id);
double get_time(void);

#ifdef __cplusplus
}
#endif

#endif
