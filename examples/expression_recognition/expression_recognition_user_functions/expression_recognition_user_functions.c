#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <dirent.h>
#include <sys/types.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "expression_recognition_user_functions.h"
#include "expression_recognition_utils.h"

#ifndef PI
#define PI 3.1415926535
#endif

#define TOP			0
#define MIDDLE			1
#define BOTTON			2

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define NO_DIRECTION		0

//############### Global Variables ###############
int g_nStatus;
int g_time_shift = 0;

void *g_current_font_type = GLUT_STROKE_MONO_ROMAN;
DIR *g_data_set_dir = NULL;
DATA_SET *g_data_set = NULL;
int g_num_inputs = 0;
int LAST_SAMPLE = 1;

//############### User Functions ###############
void
MakeInputImage (INPUT_DESC *input, int w, int h)
{
	char message[256];

	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	//window width and height
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
	//origin x and y of the input window
	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if (input->image == NULL)
	{
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
	}
}

int
get_current_input_number(char *input_name, char *caller_function_name)
{
	int current_input;

	current_input = 0;
	while ((strcmp(g_data_set[current_input].input_name, input_name) != 0) && (current_input < g_num_inputs))
		current_input++;

	if (current_input == g_num_inputs)
	{
		Erro("Could not find input named ", input_name, caller_function_name);
		return (0);
	}
	return (current_input);
}

int
get_next_sentence(INPUT_DESC *input, int nDirection)
{
	int current_input;
	int forward = 0;

	current_input = get_current_input_number(input->name, " in get_next_sentence()");

	if (nDirection != NO_DIRECTION)
	{
		forward = (nDirection == DIRECTION_FORWARD)? 1:0;

		if (g_data_set[current_input].input_index >= (g_data_set[current_input].num_input_sentences - 1))
		{
			if (forward)
				g_data_set[current_input].input_index = 0;
			else
				g_data_set[current_input].input_index -= 1;
		}
		else if (g_data_set[current_input].input_index == 0)
		{
			if (forward)
				g_data_set[current_input].input_index += 1;
			else
				g_data_set[current_input].input_index = g_data_set[current_input].num_input_sentences - 1;
		}
		else
		{
			if (forward)
				g_data_set[current_input].input_index += 1;
			else
				g_data_set[current_input].input_index -= 1;
		}
	}
	return (g_data_set[current_input].input_index);
}

FILE *
get_file(char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];

	strcpy(file_name, DATA_PATH);
	strcat(file_name, base_file_name);
	strcat(file_name, file_type);
	if ((data_set_file = fopen(file_name, "r")) == NULL)
	{
		Erro("Could not open file: ", file_name, "");
		return (NULL);
	}
	return (data_set_file);
}

int
is_not_empty_line(char *line)
{
	int i;

	for (i = 0; i < strlen(line); i++)
		if (isgraph(line[i]))
			return (1);
	return (0);
}

char *
get_valid_line(char *line, FILE *data_set_file)
{
	while (fgets(line, 900, data_set_file) != NULL)
	{
		if (line[0] == '$')
			return (NULL);
		if (line[0] == '#')
			continue;
		if (is_not_empty_line(line))
		{
			if (line[strlen(line)-1] == '\n')
				line[strlen(line)-1] = '\0';
			return (line);
		}
	}
	return (NULL);
}

int
count_data_sets(FILE *data_set_file)
{
	int num_data_sets;
	char line[1000];

	num_data_sets = 0;
	while (fgets(line, 900, data_set_file) != NULL)
	{
		if (line[0] == '$')
			num_data_sets++;
	}

	rewind(data_set_file);

	return (num_data_sets);
}

int
count_input_sentences(FILE *data_set_file, char *input_name)
{
	int num_sentences;
	char line[1000];

	while (fgets(line, 900, data_set_file) != NULL)
	{
		if ((line[0] == '$') && (strncmp(line+1, input_name, strlen(input_name)) == 0))
			break;
	}

	if (line[0] != '$')
		return (0);

	num_sentences = 0;
	while (get_valid_line(line, data_set_file) != NULL)
		num_sentences++;

	rewind(data_set_file);

	return (num_sentences);
}


void
read_input_sentences(FILE *data_set_file, DATA_SET *data_set)
{
	int current_sentence;
	char line[1000];

	while (fgets(line, 900, data_set_file) != NULL)
	{
		if ((line[0] == '$') && (strncmp(line+1, data_set->input_name, strlen(data_set->input_name)) == 0))
			break;
	}

	if (line[0] != '$')
	{
		Erro("Could not read data_set from input named: ", data_set->input_name, " in read_input_sentences().");
		return;
	}

	data_set->input_sentences = (char **) alloc_mem(data_set->num_input_sentences * sizeof(char *));
	current_sentence = 0;
	while (get_valid_line(line, data_set_file) != NULL)
	{
		data_set->input_sentences[current_sentence] = (char *) alloc_mem((strlen(line) + 1) * sizeof(char));
		strcpy(data_set->input_sentences[current_sentence], line);
#ifdef	VERBOSE
		printf("input: %s, sentence number: %d, sentence: %s\n", data_set->input_name, current_sentence, data_set->input_sentences[current_sentence]);
#endif
		current_sentence++;
	}
}

void
init_g_data_set(int num_inputs)
{
	int i;

	for (i = 0; i < num_inputs; i++)
	{
		g_data_set[i].input_name = NULL;
		g_data_set[i].input_sentences = NULL;
		g_data_set[i].input_index = 0;
		g_data_set[i].num_input_sentences = 0;
	}
}


int
read_train_data_set_data(char *base_file_name, INPUT_DESC *input)
{
	FILE *data_set_file;
	int current_input;

	data_set_file = get_file(base_file_name, ".txt");

	if (g_data_set == NULL)
	{
		g_num_inputs = count_data_sets(data_set_file);
		g_data_set = (DATA_SET *) alloc_mem(g_num_inputs * sizeof(DATA_SET));
		init_g_data_set(g_num_inputs);
	}

	current_input = 0;
	while (g_data_set[current_input].input_name != NULL)
		current_input++;

	g_data_set[current_input].input_name = input->name;
	g_data_set[current_input].num_input_sentences = count_input_sentences(data_set_file, input->name);
	read_input_sentences(data_set_file, &(g_data_set[current_input]));

	fclose(data_set_file);

	return (g_data_set[current_input].num_input_sentences);
}


char *
get_next_train_data_set(INPUT_DESC *input)
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;

	do
	{
		train_data_set_file_entry = readdir(g_data_set_dir);
		if (train_data_set_file_entry == NULL)
			return (NULL);
		aux = strrchr(train_data_set_file_entry->d_name, '.');
	} while (strcmp(aux, ".txt") != 0);

	strcpy(base_file_name, train_data_set_file_entry->d_name);

	aux = strrchr(base_file_name, '.');
	aux[0] = '\0';

	LAST_SAMPLE = num_samples = read_train_data_set_data(base_file_name, input);
	printf("# Data set: %s, Input: %s, num_samples: %d\n", train_data_set_file_entry->d_name, input->name, num_samples);

	return (train_data_set_file_entry->d_name);
}


void
init_input_data_set(INPUT_DESC *input)
{
	char *data_set_name;

	if ((g_data_set_dir = opendir(DATA_PATH)) == NULL)
	{
		show_message("Could not open data set directory named: ", DATA_PATH, "");
		return;
	}

	if ((data_set_name = get_next_train_data_set(input)) == NULL)
	{
		show_message("Could not initialize data set from directory:", DATA_PATH, "");
		exit(1);
	}
}



/*
 ***********************************************************
 * Function: make_input_image_nc
 * Description:
 * Inputs:
 * Output: none
 ***********************************************************
 */

void
make_input_image_nc(INPUT_DESC *input, int w, int h)
{
	char message[256];
	int i;

	input->tfw = nearest_power_of_2(w);
	input->tfh = nearest_power_of_2(h);

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
		Erro("Invalid Type Show ", message, " Error in make_input_image_nc().");
		return;
	}

	input->vpxo = 0;
	input->vpyo = h - input->vph;

	if(input->image == NULL)
	{
		input->image = (GLubyte *) alloc_mem(input->tfw * input->tfh * 3 * sizeof(GLubyte));
		for (i = 0; i < input->tfw * input->tfh * 3; i++)
			input->image[i] = 0;
	}
}


int
set_network_status_interface (int net_status)
{
	switch (net_status)
	{
	case TRAINING_PHASE:
		break;
	case RECALL_PHASE:
		break;
	default:
		printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
		return (-1);
	}

	g_nStatus = net_status;
	return (0);
}

void
init_sound_system()
{
}


//***********************************************************
//* Function: init_expression_recognition
//* Description:
//***********************************************************

void
init_expression_recognition (INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif
	//char file_name[128];

	g_nStatus = TRAINING_PHASE;
	set_network_status_interface(g_nStatus);

	MakeInputImage (input, INPUT_WIDTH, INPUT_HEIGHT);

	input->up2date = 0;
	update_input_neurons (input);

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww, input->wh);

	if (read_window_position (input->name, &x, &y))
	{
		glutInitWindowPosition (x, y);
	}
	else
	{
		glutInitWindowPosition (-1, -1);
	}
	input->win = glutCreateWindow (input->name);

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
#endif
}


void
get_text_lines_content(char *text_line1, char *text_line2, char *text_line3, char *input_string)
{
	int i, j;

	j = i = 0;
	while (input_string[i] != '|')
	{
		text_line1[j] = input_string[i];
		j++; i++;
	}
	text_line1[j] = '\0'; i++;

	j = 0;
	while (input_string[i] != '|')
	{
		text_line2[j] = input_string[i];
		j++; i++;
	}
	text_line2[j] = '\0'; i++;

	j = 0;
	while ((input_string[i] != '\0') && (input_string[i] != '#'))
	{
		text_line3[j] = input_string[i];
		j++; i++;
	}
	text_line3[j] = '\0';
}


void
nc_input_reshape (int w, int h)
{
	INPUT_DESC *input;
	double winput, hinput, d, ang;

	input = get_input_by_win (glutGetWindow ());
	winput = (GLdouble) input->vpw;
	hinput = (GLdouble) input->vph;
	d = sqrt(winput*winput + hinput*hinput);
	ang = 2.5 * ((atan2 (hinput/2.0, d) * 180.0)/PI);
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (ang, (GLfloat)w/(GLfloat)h, 100.0*d, d);
}


void
copy_input_image_to_input_neurons (INPUT_DESC *input)
{

	int p_nViewport[4];
	int nWidth, nHeight;
	static int nPreviusWidth = 0, nPreviusHeight = 0;
	int xi, yi, wi, hi, xo, yo, wo, ho, r, g, b;
	static GLubyte *pScreenPixels = (GLubyte *) NULL;
	NEURON *neuron_vector;
	float k;

	glutSetWindow(input->win);

	glGetIntegerv (GL_VIEWPORT, p_nViewport);
	nWidth = p_nViewport[2];
	nHeight = p_nViewport[3];

	if ((nWidth != nPreviusWidth) || (nHeight != nPreviusHeight))
	{
		free (pScreenPixels);
		if ((pScreenPixels = (GLubyte *) malloc (3 * nWidth * nHeight * sizeof (GLubyte))) == (GLubyte *) NULL)
		{
			Erro ("Cannot allocate more memory", "", "");
			return;
		}
		nPreviusWidth = nWidth;
		nPreviusHeight = nHeight;
	}

	glReadBuffer (GL_BACK);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, pScreenPixels);
	glDisable(GL_READ_BUFFER);

	wi = nWidth;
	hi = nHeight;

	neuron_vector = input->neuron_layer->neuron_vector;
	wo = input->neuron_layer->dimentions.x;
	ho = input->neuron_layer->dimentions.y;

	k = (float) wi / (float) wo;

	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) (k * (float) yo + .5f);
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (k * (float) xo + .5f);
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
			{
				r = pScreenPixels[3 * (yi * wi + xi) + 0];
				g = pScreenPixels[3 * (yi * wi + xi) + 1];
				b = pScreenPixels[3 * (yi * wi + xi) + 2];
			}
			else
				r = g = b = 0;

			switch (input->neuron_layer->output_type)
			{
			case COLOR:
				neuron_vector[yo * wo + xo].output.ival = PIXEL(r, g, b);
				break;
			case GREYSCALE:
				neuron_vector[yo * wo + xo].output.ival = (r + g + b) / 3;
				break;
			case BLACK_WHITE:
				neuron_vector[yo * wo + xo].output.ival = r > 50? NUM_COLORS - 1: 0;
				break;
			case GREYSCALE_FLOAT:
				neuron_vector[yo * wo + xo].output.fval = (float) (r + g + b) / 3.0;
				break;
			}
		}
	}
}

void
draw_text_into_input(char *text_line, int position, GLdouble w, GLdouble h, GLdouble d)
{
	int i;
	float x, y, z;

	x = -((GLdouble) strlen (text_line)) * 104.76/2.3;

	switch (position)
	{
	case TOP: 		y = +3.9 * (119.05/3.0) + 10.0; break;
	case MIDDLE: 	y =       -(119.05/3.0) + 20.0; break;
	case BOTTON: 	y = -5.5 * (119.05/3.0) + 10.0; break;
	}

	z = -8 * d;

	glPushMatrix();

	glScalef (w / (0.47*(104.76 / (119.05 + 25.0)) * ((GLdouble) strlen (text_line)) * h), 0.8, 1.0);

	glTranslatef(x, y, z);

	glRasterPos3f(x, y, z);

	for (i = 0; i < strlen (text_line); i++)
		glutStrokeCharacter (g_current_font_type, text_line[i]);

	glPopMatrix();
}


void
draw_attention_control_into_input(char *text_line, int position, GLfloat w, GLfloat h)
{
	float red = 0.0, blue = 0.0;
	GLfloat x0, x1, y0, y1;

	if (strchr(text_line, 'u') != NULL)
		red = 1.0;
	if (strchr(text_line, 'd') != NULL)
		blue = 1.0;
	x0 = 0;
	x1 = w;
	switch (position)
	{
	case TOP:
		y0 = 2.0 * h / 3.0;
		y1 = h;
		break;
	case MIDDLE:
		y0 = 1.0 * h / 3.0;
		y1 = 2.0 * h / 3.0;
		break;
	case BOTTON:
		y0 = 0.0;
		y1 = 1.0 * h / 3.0;
		break;
	}


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_DST_ALPHA);
	// Draw a filled square.
	glColor3f (red, 0.0, blue);
	glBegin(GL_POLYGON);
	glVertex2f(x0, y0);
	glVertex2f(x1, y0);
	glVertex2f(x1, y1);
	glVertex2f(x0, y1);
	glEnd();
}


void
draw_text(INPUT_DESC *input, GLdouble w, GLdouble h, GLdouble d)
{
	char text_line1[200];
	char text_line2[200];
	char text_line3[200];
	char *attention_control;

	attention_control = strchr(input->string, '$');
	attention_control[0] = '\0';
	get_text_lines_content(text_line1, text_line2, text_line3, input->string);
	attention_control[0] = '$';
	attention_control++;

	draw_text_into_input(text_line1, TOP, w, h, d);
	draw_text_into_input(text_line2, MIDDLE, w, h, d);
	draw_text_into_input(text_line3, BOTTON, w, h, d);

	get_text_lines_content(text_line1, text_line2, text_line3, attention_control);
	draw_attention_control_into_input(text_line1, TOP,    (GLfloat) input->neuron_layer->dimentions.x, (GLfloat) input->neuron_layer->dimentions.y);
	draw_attention_control_into_input(text_line2, MIDDLE, (GLfloat) input->neuron_layer->dimentions.x, (GLfloat) input->neuron_layer->dimentions.y);
	draw_attention_control_into_input(text_line3, BOTTON, (GLfloat) input->neuron_layer->dimentions.x, (GLfloat) input->neuron_layer->dimentions.y);
}


void
nc_input_display (void)
{
	INPUT_DESC *input;
	GLdouble w, h, d;

	input = get_input_by_win (glutGetWindow ());

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	w = (GLdouble) input->vpw;
	h = (GLdouble) input->vph;
	d = sqrt(w*w + h*h);
	gluLookAt (0.0, 0.0, 0.0,
			0.0, 0.0, -d,
			0.0, 1.0, 0.0);
	glScalef (sqrt(1.6), sqrt(1.6), 1.0);
	glTranslatef (-w/2.0, -h/2.0, -d);

	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (0.0, 1.0, 0.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(d/5.0);

	draw_text(input, w, h, d);
	copy_input_image_to_input_neurons(input);

	glutSwapBuffers ();

	input->waiting_redisplay = 0;
}

/*
 ***********************************************************
 * Function: read_input_sentence
 * Description: Writes a set of returns into the MAE input
 * Inputs: input - input image
 * Output: 0 if OK, -1 otherwise
 ***********************************************************
 */

void
read_input_sentence(INPUT_DESC *input, int sentence_number)
{
	int current_input;

	current_input = get_current_input_number(input->name, " in read_input_sentence()");
	input->string = g_data_set[current_input].input_sentences[sentence_number];
	glutSetWindow(input->win);
	nc_input_display();
}


void
initialize_data_set(INPUT_DESC *input)
{
	int sentence_number;

	init_input_data_set(input);

	sentence_number = get_next_sentence(input, NO_DIRECTION);
	read_input_sentence(input, sentence_number);
}

/*
 ***********************************************************
 * Function: init_nc_input
 * Description:
 * Inputs:input - Neuron Layer de input
 * Output: none
 ***********************************************************
 */

void
init_nc_input(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif

	make_input_image_nc(input, 3*NL_WIDTH, 3*NL_HEIGHT/2);

	input->up2date = 0;

#ifndef NO_INTERFACE
	glutInitWindowSize(input->ww, input->wh);
	if (read_window_position(input->name, &x, &y))
		glutInitWindowPosition(x, y);
	else
		glutInitWindowPosition(-1, -1);
	input->win = glutCreateWindow(input->name);

	glGenTextures(1, (GLuint *)(&(input->tex)));
	input_init(input);
	glutReshapeFunc(nc_input_reshape);
	glutDisplayFunc(nc_input_display);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(input_passive_motion);
	glutMouseFunc(input_mouse);
#endif
}


//***********************************************************
//* Function: init_user_functions
//* Description:
//***********************************************************
int
init_user_functions (void)
{
	char strCommand[128];

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);

	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	return (0);
}

void get_new_input_image(INPUT_DESC *input)
{
	int w, h, x, y;

	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;

	CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);

	if (!capture)
	{
		fprintf(stderr, "Erro: verifique se a camera esta ligada\n");
		exit(-1);
	}

	IplImage* frame = cvQueryFrame(capture);
	IplImage* frame_resized = cvCreateImage (cvSize(w, h), frame->depth, frame->nChannels);

	while (!frame)
		frame = cvQueryFrame(capture);

	const char* cascade_name = "haarcascade_frontalface_alt.xml"; /* "haarcascade_profileface.xml"; */

	// Load the HaarClassifierCascade
	CvHaarClassifierCascade* cascade = 0;
	cascade = (CvHaarClassifierCascade*) cvLoad(cascade_name, 0, 0, 0);

	// Check whether the cascade has loaded successfully. Else report and error and quit
	if(!cascade)
	{
		fprintf(stderr, "ERROR: Could not load classifier cascade\n");
		exit(-1);
	}

	CvMemStorage* storage = 0;
	storage = cvCreateMemStorage(0);
	cvClearMemStorage(storage);

	// Find whether the cascade is loaded, to find the faces. If yes, then:
	if(cascade)
	{
		// There can be more than one face in an image. So create a growable sequence of faces.
		// Detect the objects and store them in the sequence
		CvSeq* faces = cvHaarDetectObjects(frame, cascade, storage,
				1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
				cvSize(40, 40));

		if (faces->total > 0)
		{
			CvRect* r = (CvRect*) cvGetSeqElem(faces, 0);
			cvSetImageROI(frame, *r);
		}
	}

	cvResize(frame, frame_resized, CV_INTER_CUBIC);
	cvReleaseCapture(&capture);

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			input->neuron_layer->neuron_vector[(h - y) * w + x].output.ival = PIXEL(
					frame_resized->imageData[y * frame_resized->widthStep + 3 * x + 2],
					frame_resized->imageData[y * frame_resized->widthStep + 3 * x + 1],
					frame_resized->imageData[y * frame_resized->widthStep + 3 * x + 0]
			);
		}
	}

	update_input_image(input);
}

void
input_generator (INPUT_DESC *input, int status)
{
	if (input->win == 0)
	{
		init_expression_recognition(input);
#ifdef NO_INTERFACE
		input->win = 1;
#endif
	}
	else
	{
		if (status == MOVE)
		{
			if ((input->wxd < 0) || (input->wxd >= INPUT_WIDTH))
			{
				get_new_input_image (input);
			}

			all_filters_update();
			all_dendrites_update();
			all_neurons_update();
			all_outputs_update();

#ifndef NO_INTERFACE
			glutSetWindow(input->win);
			input_display();
#endif
		}
	}
}


/*
 ***********************************************************
 * Function: get_new_sentence
 * Description:
 * Inputs: input -
 *	  nDirection -
 * Output: 0 if OK, -1 otherwise
 ***********************************************************
 */

void
get_new_sentence(INPUT_DESC *input, int nDirection)
{
	int sentence_number;

	sentence_number = get_next_sentence(input, nDirection);
	read_input_sentence(input, sentence_number);
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

void
input_generator1(INPUT_DESC *input, int status)
{
	NEURON_LAYER *target_neuron_layer;
	OUTPUT_DESC *associated_output;

	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_nc_input(input);
		input->win = glutGetWindow();
		initialize_data_set(input);
	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < NL_WIDTH/5)
				get_new_sentence(input, DIRECTION_REWIND);
			else if (input->wxd >= NL_WIDTH-NL_WIDTH/5)
				get_new_sentence(input, DIRECTION_FORWARD);

			target_neuron_layer = (NEURON_LAYER *) input->input_generator_params->next->param.pval;
			copy_neuron_outputs(target_neuron_layer, input->neuron_layer);
			associated_output = get_output_by_neural_layer(target_neuron_layer);
			output_update(associated_output);

			glutSetWindow(input->win);

			all_filters_update();
		}
		else if (status == FORWARD)
		{
			target_neuron_layer = (NEURON_LAYER *) input->input_generator_params->next->param.pval;
			copy_neuron_outputs(target_neuron_layer, input->neuron_layer);
			associated_output = get_output_by_neural_layer(target_neuron_layer);
			output_update(associated_output);

			glutSetWindow(input->win);

			all_filters_update();
		}
	}
}



//***********************************************************
//* Function: draw_output
//* Description:
//***********************************************************
void
draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;
	output = get_output_by_name (strOutputName);
	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, 23132);

	update_output_image (output);
#ifndef NO_INTERFACE
	glutSetWindow(output->win);
	glutPostWindowRedisplay (output->win);
#endif
}


//***********************************************************
//* Function: input_controller
//* Description:
//***********************************************************
void
input_controler (INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
			(input->mouse_state == GLUT_DOWN) &&
			(draw_active == 1))
	{
		sprintf (strCommand, "draw out_nl_expression_memory based on expression_input move;");
		interpreter (strCommand);
	}

	if ((move_active == 1) &&
			(input->mouse_button == GLUT_LEFT_BUTTON) &&
			(input->mouse_state == GLUT_DOWN))
	{
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
		interpreter("forward network;");
	}

	input->mouse_button = -1;
}


/**
 * Description: handles the mouse input window events
 * -When mouse left button is pressed then a move command is triggered passing x, y coordinates.
 * @param input
 * @param status
 */
void input_controler1(INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((move_active == 1) &&
			(input->mouse_button == GLUT_LEFT_BUTTON) &&
			(input->mouse_state == GLUT_DOWN))
	{
		sprintf(strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter(strCommand);
	}

	input->mouse_button = -1;
}


//***********************************************************
//* Function: EvaluateOutput
//* Description: evaluates the output value
//* Inputs: output
//* Output: person ID
//***********************************************************
int EvaluateOutput(OUTPUT_DESC *output, float *confidence)
{
	return 0;
}

//***********************************************************
//* Function: output_handler
//* Description:
//***********************************************************
void output_handler(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display (output);
#endif
}

void output_handler1(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
#ifndef NO_INTERFACE
	glutSetWindow (output->win);
	output_display (output);
#endif
}

NEURON_OUTPUT
SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;
	output.ival = set_network_status_interface(g_nStatus);

	return (output);
}

//***********************************************************
//* Function: f_keyboard
//* Description: Called whenever a key is pressed
//***********************************************************
void f_keyboard (char *key_value)
{
	char key;

	key = key_value[0];
	switch (key)
	{
	case 't': //set training status
		set_network_status_interface(TRAINING_PHASE);
		break;

	case 'r': //set recall status
		set_network_status_interface(RECALL_PHASE);
		break;

	default:
		break;
	}
}
