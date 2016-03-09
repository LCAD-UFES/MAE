/*
for i in image.03*.jp2; do echo ${i:0:13}; done
*/
#include "traffic_sign_user_functions.h"

#define N_DEFINED_SIGNS 146
//#define N_DEFINED_SIGNS 51

//############### Global Variables ###############
//char placas[N_DEFINED_SIGNS][64] = {"E1", "begin", "C3", "C29", "B19", "C21", "E9a", "X31", "einde", "X31_buiten", "X32", "X32_buiten", "X3L", "F59", "F45", "B5", "F29_links", "F4a", "laden_lossen", "D7", "F29", "C31RIGHT", "B15A", "D1b_schuin_rechts", "X11", "E9b", "oversteekplaats", "F34A_links", "F34A", "X3R", "F49", "B3", "lang", "A7A", "A14", "meter", "F4b", "B1", "Handic", "A23", "typell", "F29_F23A_F23B", "E9a_disk", "F59_links", "E9a_miva", "X10", "F37_links", "C11", "X33", "A25", "fietsers_rijbaan"};

char placas[N_DEFINED_SIGNS][64] = {"25m","30m","50m","700m","90m","A14","A23","A25","A41","A7A", "B1","B15A","B19","B3","B5","C11","C21","C29","C3","C31RIGHT","D1b_schuin_rechts", "D7", "E1","E9a","E9a_disk","E9a_miva","E9b","F12a","F29","F29_F23A_F23B","F29_links","F34A","F34A_links","F37_links", "F45", "F49", "F4a","F4b", "F50bis variant", "F59", "F59_links", "Handic", "X10", "X11","X31","X31_buiten","X32","X32_buiten","X33","X3L","X3R","begin","einde", "fietsers_rijbaan","laden_lossen", "lang", "meter", "oversteekplaats", "typell", "3,5t", "7,5t_uitgezonderd_laden_en_lossen","7,5t_uitgezonderd_laden_lossen", "7,5tuitgezonderd_laden_en_lossen", "A13", "A15", "A23_geel", "A7C", "B17","B9", "C1", "C31LEFT", "C43", "C5", "D1b_rechts", "D1b_rechts_onder", "D9", "E3", "F13", "F17", "F19", "F21","F23A", "F27", "F30", "F31", "F33a", "F37", "F43", "F50bis", "F97B", "MAX3,5t", "P7t_einde", "STOP_150m","bebouwde_kom_50km_radar_controle", "blabla", "e0c", "einde_zone_idunno", "gevaar_fiets", "groen_fiets","groene_fiets","herhaling", "mag_weg", "max3,5t", "oranje_wandel", "uigezonderd_plaatselijk_verkeer", "uitgez_bus","uitgez_hulpdiensten", "uitgezonderd_blabla", "zeshoekige_fiets", "zone7,5tverboden_combo_P3,5tmax","zone_P_3,5t_max", "zone_einde_P_blabla", "zone_idunno", "zone_verboden_camion_blabla", " F29","A51", "B11", "C35", "D1b", "D5", "E11", "E5", "E7", "E9b_disk", "E9d", "F25", "bewoners_politievoertuigen","m2", "m3", "m5", "m6", "school", "A31", "B7", "Db1_schuin_rechts", "F23a", "F29_F23A", "F29_links_F23A","F47", "F50", "F77", "F87", "X2", "fiets_wandel_routes", "hulpdiensten", "plaatselijk"};

int g_frameID;
int g_camera = 0; //0..7
int g_sequence = 1; //1..4
int g_curr_sign;
char g_sign_names[10][64];
int g_signUniqueID;
int g_index_sign_moving = 0;

int g_number_of_boxes;
int g_number_of_signs;
int g_resp_camera[10]; //em qual camera aparece o box de respectivo indice
float g_corner0_x[10]; //pode haver varios boxes em um frame
float g_corner0_y[10];
float g_corner1_x[10];
float g_corner1_y[10];

int g_nStatus;
int g_nPos;
int g_nTries = 1;
int g_nTry = 1;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;
//int g_nNoOne;

//char g_strRandomFacesFileName[256];
char g_strSignsFileName[256];



//############### User Functions ###############

//***********************************************************
//* Function: GetUniqueId
//* Description:
//***********************************************************
int GetUniqueID()
{
	int i;
	for (i=0; i<146; i++)
	{
		if (strcmp (g_sign_names[g_curr_sign], placas[i]) == 0)
		{
			return i;
		} 
	}
	return -1;
}

//***********************************************************
//* Function: GetGroundTruths
//* Description:
//***********************************************************
int GetGroundTruths (void)
{
	int number_of_boxes = 0, frameid, camera;
	float corner0_x, corner0_y, corner1_x, corner1_y;
	char sign_name[64];
	FILE *pFile = NULL;
	char strFileName[256];
	
	//sprintf(strFileName, "%sannotation_cam%d.txt", GROUNDT_PATH, g_camera);
	sprintf(strFileName, "%splacas_seq%02d", GROUNDT_PATH, g_sequence);
	
	
	if ((pFile = fopen (strFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s'.\n", strFileName);	
		return (-1);
	}
	g_number_of_boxes=0;
	
	while (fscanf (pFile, "%d %d %s %f %f %f %f\n", &frameid, &camera, sign_name, &corner0_y, &corner0_x, &corner1_y, &corner1_x) > 0)
	{
		if (frameid == g_frameID)
		{
			if (camera == g_camera)
			{
				g_corner0_x[number_of_boxes] = corner0_x;
				g_corner0_y[number_of_boxes] = corner0_y;
				g_corner1_x[number_of_boxes] = corner1_x;
				g_corner1_y[number_of_boxes] = corner1_y;
				strcpy(g_sign_names[number_of_boxes], sign_name);
				number_of_boxes++;
			}
		}
	}
	
	g_number_of_boxes = number_of_boxes;
	fclose (pFile);
	return (0);
}

//***********************************************************
//* Function: GetNextFileName
//* Description:
//***********************************************************
int GetNextFileName(char *strFileName, int nDirection)
{
	FILE *pFile = NULL;

	sprintf (strFileName, "%sSeq%02d/%02d/image.%06d.jp2", INPUT_PATH, g_sequence ,g_camera, g_frameID); //new
	fflush (stdout);
	
	while (!(pFile = fopen (strFileName, "r")))
	{
		printf("file not found: %s\n",strFileName);
		if (g_frameID >= MAX_FRAME_ID)
		{
			g_frameID = MIN_FRAME_ID;
			g_camera = 0; //new
		}
		if (g_frameID < MIN_FRAME_ID)
		{
			g_frameID = MAX_FRAME_ID;
			g_camera = 0; //new
		}
		else		
		{
			if (nDirection == DIRECTION_REWIND)
			{
				if (g_camera == 0)
				{
					g_frameID--;
					g_camera = 7;
				}
				else
				{
					g_camera--;
				}
			}
			else //DIRECTION_FORWARD
			{
				if (g_camera == 7)
				{
					g_frameID++;
					g_camera = 0;
				}
				else
				{
					g_camera++;
				}
			}
		}
		sprintf (strFileName, "%sSeq%02d/%02d/image.%06d.jp2", INPUT_PATH, g_sequence ,g_camera, g_frameID);
		pFile = fopen (strFileName, "r");
	}
	
	printf ("file: %s\n", strFileName);
	fclose (pFile);
	return (0);
}

//***********************************************************
//* Function: ReadFrameInput
//* Description:
//***********************************************************
int ReadFrameInput(INPUT_DESC *input, char *strFileName)
{
	GetGroundTruths ();
	input->green_cross_x = 0;
	input->green_cross_y = 0;
	char cmd[256];
	
//	strcpy(cmd, "");
	system("rm -f ./temp_filename*");
	sprintf(cmd, "cp %s ./temp_image.jp2",strFileName);
	system(cmd);
	system("convert temp_image.jp2 temp_image.pnm");
	load_input_image (input, "./temp_image.pnm");
	//load_input_image (input, strFileName);
	return 0;
}

//sem groundtruths
int ReadFrameInput2(INPUT_DESC *input, char *strFileName)
{
	input->green_cross_x = 0;input->green_cross_y = 0;
	char cmd[256];
	system("rm -f ./temp_filename*");sprintf(cmd, "cp %s ./temp_image.jp2",strFileName);system(cmd);
	system("convert temp_image.jp2 temp_image.pnm");load_input_image (input, "./temp_image.pnm");
	return 0;
}

//***********************************************************
//* Function: MakeInputImage
//* Description:
//***********************************************************
void MakeInputImage(INPUT_DESC *input, int w, int h)
{
	char message[256];

	//texture frame width and height, must be powers of 2
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	//window width and height
	input->ww = w;
	input->wh = h;

	switch(TYPE_SHOW)
	{
		case SHOW_FRAME:
			//visible part (of the window) width and height
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

	if(input->image == NULL)
	{
		input->image = (GLubyte *) alloc_mem (input->tfw * input->tfh * 3 * sizeof (GLubyte));
	}
}



//***********************************************************
//* Function: init_traffic_sign
//* Description:
//***********************************************************
void init_traffic_sign(INPUT_DESC *input)
{
#ifndef NO_INTERFACE
	int x, y;
#endif
	char strFileName[128];

	g_frameID = MIN_FRAME_ID;
	g_nStatus = MOVING_PHASE;
	g_nTotalTested = 0;
	g_nCorrect = 0;
	g_curr_sign = 0;
	//g_nNoOne = 0;
	
	MakeInputImage (input, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Le a primeira imagem
	if (!GetNextFileName (strFileName, DIRECTION_FORWARD))
	{
		ReadFrameInput (input, strFileName);
	}

	//input->green_cross = 1;
	
	g_nPos = 0;
		
	input->up2date = 0;
	update_input_neurons (input);

#ifndef NO_INTERFACE
	glutInitWindowSize (input->ww/4, input->wh/4);
	
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

//***********************************************************
//* Function: init_user_functions
//* Description:
//***********************************************************
int init_user_functions (void)
{
	char strCommand[128];
			
	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	return (0);
}

//***********************************************************
//* Function: GetNewFrame
//* Description:
//***********************************************************
int GetNewFrame(INPUT_DESC *input, int nDirection)
{
	char strFileName[128];

	if (nDirection == DIRECTION_FORWARD)
	{
		if (g_camera == 7)
		{		
			g_frameID++;
			g_camera = 0;
		}
		else
		{
			g_camera++;
		}
	}
		
	else
	{
		if (g_camera == 0)
		{
			g_frameID--;
			g_camera = 7;
		}
		else
		{
			g_camera--;
		}	
	}
	while (GetNextFileName(strFileName, nDirection)) //ver isso
		;
	if (ReadFrameInput(input, strFileName))
	{
		return (-1);
	}
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	input->up2date = 0;
	update_input_neurons (input);
	update_input_image (input);
	input->green_cross_x = 0;
	input->green_cross_y = 0;
	g_nPos = 0;
	//printf("placa = %s\n",g_sign_names[g_curr_sign]);
	return (0);
}

//***********************************************************
//* Function: GetNextSign
//* Description:
//***********************************************************
void GetNextSign(INPUT_DESC *input, int nDirection)
{
	//char strFileName[128];
	if (nDirection == DIRECTION_FORWARD)
	{
		g_curr_sign++;
	}		
	else
	{
		g_curr_sign--;
	}
	if ((g_curr_sign >= g_number_of_boxes) || (g_curr_sign < 0))
	{
		g_curr_sign = 0;
//		g_camera = 0;//new
		GetNewFrame(input, nDirection);
		return;
	}
	//sprintf (strFileName, "%simage.%06d.pnm", INPUT_PATH, g_frameID);
	//sprintf (strFileName, "%sSeq%02d/%02d/image.%06d.jp2", INPUT_PATH, g_sequence ,g_camera, g_frameID);
	//ReadFrameInput(input, strFileName);
	//check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	//input->up2date = 0;
	update_input_neurons (input);	
	update_input_image (input);

	input->green_cross_x = 0;
	input->green_cross_y = 0;
	g_nPos = 0;
	
//	if (GetUniqueID() >=0)
//	{
//		printf("id da placa = %d ",GetUniqueID());
//	}
	//printf("placa = %s\n",g_sign_names[g_curr_sign]);	
}


//***********************************************************
//* Function: input_generator
//* Description:
//***********************************************************
void input_generator (INPUT_DESC *input, int status)
{
	FILTER_DESC *filter;

	// Inicializacao executada apenas uma vez por janela
	if (input->win == 0)
	{
		init_traffic_sign(input);
#ifdef NO_INTERFACE
		input->win = 1;	
#endif
 	}
	else
	{
		if (status == MOVE)
		{
			if (input->wxd < 0)
			{
				//wxd = window x coordinate when mouse button down
				//se clicar na esquerda, vai para a imagem anterior
				GetNewFrame (input, DIRECTION_REWIND);
			}
			else if (input->wxd >= IMAGE_WIDTH)
			{
				GetNewFrame (input, DIRECTION_FORWARD);
			}
			//update the first output
			filter = get_filter_by_output(out_traffic_sign.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign);
			
			//update the gaussian filtered output
			filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign_filtered);

#ifndef NO_INTERFACE			
			glutSetWindow (input->win);
			input_display ();
#endif
		}
	}	
}
//***********************************************************
//* Function:
//* Description:
//***********************************************************
NEURON_OUTPUT MoveInput (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	traffic_sign.wxd = 100;
	traffic_sign.wyd = 100;
	move_input_window (traffic_sign.name, traffic_sign.wxd, traffic_sign.wyd);
	
	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: draw_output
//* Description:
//***********************************************************
void draw_output (char *strOutputName, char *strInputName)
{
	OUTPUT_DESC *output;

	output = get_output_by_name (strOutputName);
	set_neurons (output->neuron_layer->neuron_vector, 0, output->wh * output->ww, g_signUniqueID); 	
	
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
void input_controler (INPUT_DESC *input, int status)
{
	char strCommand[128];

	if ((input->mouse_button == GLUT_RIGHT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (draw_active == 1))
	{
		sprintf (strCommand, "draw out_landmark based on traffic_sign move;");
		interpreter (strCommand);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN))
	{
		sprintf (strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
		interpreter (strCommand);
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
	int i;
	int sign_id = 0;
	int max =0;
	//int sign_count[g_number_of_signs];
	int sign_count[N_DEFINED_SIGNS];
	NEURON *neuron_vector;
	//number of signs = N_DEFINED_SIGNS
	
	neuron_vector = output->neuron_layer->neuron_vector;
	
	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		sign_count[i] = 0;
	}
	
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		if ((neuron_vector[i].output.ival >= 0) && (neuron_vector[i].output.ival < N_DEFINED_SIGNS))
		{
			sign_count[neuron_vector[i].output.ival] += 1;
		}
	}
	
	for (i = 0; i < N_DEFINED_SIGNS; i++)
	{
		if (sign_count[i] > max)
		{
			max = sign_count[i];
			sign_id = i;
		}
	}

	*confidence = (float) max / (float) (output->wh * output->ww);
	return sign_id;
}

//***********************************************************
//* Function: output_handler
//* Description:
//***********************************************************
void output_handler (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{

	int sign;
	float confidence, hit_percent;
	static float best_confidence = -1.0;
	static int sign_output;
	
	if (g_nStatus == RECALL_PHASE)
	{
		//avalia a saida
		if (strcmp (output->name, out_landmark.name) == 0);
		//if (strcmp (output->name, out_traffic_sign.name) == 0)
		{
			if (g_nTry != 0)
			{
				sign = EvaluateOutput (output, &confidence);
				
				if	(confidence > best_confidence)
				{
					best_confidence = confidence;
					sign_output = sign;
				}
				g_nTry--;
			}
			if (g_nTry == 0)
			{
				g_nTotalTested++;
				
				if (sign_output	== g_signUniqueID)
				{
					g_nCorrect++;
					hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
					//printf("HIT (%s) hit_percent = %3.2f\n",placas[g_signUniqueID],hit_percent);
					printf("HIT %06d %02d ",g_frameID, g_camera);
					printf("%3d %3d %s %3d ", sign_output, g_signUniqueID,placas[g_signUniqueID], g_nTotalTested);
					printf("%3d %3.2f\n", g_nCorrect, hit_percent);
					//printf("Total Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f\n", g_nCorrect, best_confidence, hit_percent);
				}
				else
				{
					hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
					//printf("MISS (%s) hit_percent = %3.2f\n",placas[g_signUniqueID],hit_percent);
					printf("MISS %06d %02d ",g_frameID, g_camera);
					printf("%3d %3d %s %3d ", sign_output, g_signUniqueID,placas[g_signUniqueID], g_nTotalTested);
					printf("%3d %3.2f\n", g_nCorrect, hit_percent);
					//printf("Total Correct: %3d - Confidence: %1.2f - Hit Percent: %3.2f\n", g_nCorrect, best_confidence, hit_percent);
				}
				fflush (stdout);
				g_nTry = g_nTries;
				best_confidence = -1.0;
			}
		}
	}	

#ifndef NO_INTERFACE	
	glutSetWindow (output->win);
	output_display (output);
#endif
}

//***********************************************************
//* Function: GetFrame
//* Description: Gets a new frame ** acho que nao vai usar isso
//***********************************************************
NEURON_OUTPUT GetFrame (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	char strFileName[256];
	
	printf(" get frame \n");
	fflush(stdout);
	
	if (ReadFrameInput (&traffic_sign, strFileName))
	{
		printf ("Error: Cannot read frame (GetFrame).\n");
		output.ival = -1;
		return (output);
	}	
	
	check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
	traffic_sign.up2date = 0;
	update_input_neurons (&traffic_sign);
	update_input_image (&traffic_sign);

	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: ShowStatistics
//* Description:
//***********************************************************
NEURON_OUTPUT ShowStatistics (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: ResetStatistics
//* Description:
//***********************************************************
NEURON_OUTPUT ResetStatistics (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nTotalTested = 0;
	g_nCorrect = 0;
	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: SetNetworkStatus
//* Description:
//***********************************************************
NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	g_nStatus = pParamList->next->param.ival;
	//g_nTry = g_nTries = pParamList->next->next->param.ival;

	switch (g_nStatus)
	{
		case TRAINING_PHASE:
			strcpy (g_strSignsFileName, RANDOM_FACES_TRAIN);
			break;
		case RECALL_PHASE:
			strcpy (g_strSignsFileName, RANDOM_FACES_TEST);
			break;
		default:
			printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
			output.ival = -1;
			return (output);
	}

	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: GetNthSign
//* Description: GetRandomFaceN no face recog planar
//				
//***********************************************************
void GetNthSign (int line_number)
{
	FILTER_DESC *filter;
	char strFileName[256];
	FILE *pFile = NULL;
	int i;
	int frameID, camera;
	float corner0_y, corner0_x, corner1_y, corner1_x;
	char sign_name[64];

	if ((pFile = fopen (RANDOM_FACES_TRAIN, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s' (GetNthSignParam).\n", g_strSignsFileName);
	}
	g_curr_sign = 0;
	g_number_of_boxes = 1;
	for (i = 0; i < line_number; i++)
	{
		fscanf (pFile,"%d %d %s %f %f %f %f\n",&frameID, &camera, sign_name, &corner0_y, &corner0_x, &corner1_y, &corner1_x);
		
		if (i == line_number -1 )
		{
			g_frameID = frameID;
			g_camera = camera;
			strcpy(g_sign_names[g_curr_sign],sign_name);
			g_corner0_y[g_curr_sign] = corner0_y;
			g_corner0_x[g_curr_sign] = corner0_x;
			g_corner1_y[g_curr_sign] = corner1_y;
			g_corner1_x[g_curr_sign] = corner1_x;
		}
	}
	
	fclose (pFile);
		
	sprintf (strFileName, "%sSeq%02d/%02d/image.%06d.jp2", INPUT_PATH, g_sequence ,g_camera, g_frameID);	
	g_signUniqueID = GetUniqueID(); //ok
	fflush(stdout);
	
	if (ReadFrameInput2 (&traffic_sign, strFileName))
	{
		printf ("Error: Cannot read face (GetNthSignParam).\n");
	}

	printf("%s (id=%d) ,cam %02d, id %06d\n", g_sign_names[g_curr_sign], g_signUniqueID, g_camera,g_frameID);	
	
	check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
	traffic_sign.up2date = 0;
	update_input_neurons (&traffic_sign);
	update_input_image (&traffic_sign);
	
	//update the first output
	filter = get_filter_by_output(out_traffic_sign.neuron_layer);
	filter_update(filter);
	output_update(&out_traffic_sign);
	//update the gaussian filtered output
	filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
	filter_update(filter);
	output_update(&out_traffic_sign_filtered);
}

NEURON_OUTPUT GetNthSignParam (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	FILTER_DESC *filter;
	char strFileName[256];
	FILE *pFile = NULL;
	int line_number;
	int i;
	int frameID, camera;
	float corner0_y, corner0_x, corner1_y, corner1_x;
	char sign_name[64];
	
	line_number = pParamList->next->param.ival;
	
	if ((pFile = fopen (g_strSignsFileName, "r")) == NULL)
	{
		printf ("Error: cannot open file '%s' (GetNthSignParam).\n", g_strSignsFileName);
		output.ival = -1;
		return (output);
	}
	g_curr_sign = 0;
	g_number_of_boxes = 1;
	for (i = 0; i < line_number; i++)
	{
		fscanf (pFile,"%d %d %s %f %f %f %f\n",&frameID, &camera, sign_name, &corner0_y, &corner0_x, &corner1_y, &corner1_x);
		
		if (i == line_number -1 )
		{
			g_frameID = frameID;
			g_camera = camera;
			strcpy(g_sign_names[g_curr_sign],sign_name);
			g_corner0_y[g_curr_sign] = corner0_y;
			g_corner0_x[g_curr_sign] = corner0_x;
			g_corner1_y[g_curr_sign] = corner1_y;
			g_corner1_x[g_curr_sign] = corner1_x;
		}
	}
	
	fclose (pFile);
		
	sprintf (strFileName, "%sSeq%02d/%02d/image.%06d.jp2", INPUT_PATH, g_sequence ,g_camera, g_frameID);	
	g_signUniqueID = GetUniqueID(); //ok
	fflush(stdout);
	
	if (ReadFrameInput2 (&traffic_sign, strFileName))
	{
		printf ("Error: Cannot read face (GetNthSignParam).\n");
		output.ival = -1;
		return (output);
	}
	
	g_index_sign_moving = line_number; //debug deleteme
	
	check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
	traffic_sign.up2date = 0;
	update_input_neurons (&traffic_sign);
	update_input_image (&traffic_sign);
	
	//update the first output
	filter = get_filter_by_output(out_traffic_sign.neuron_layer);
	filter_update(filter);
	output_update(&out_traffic_sign);
	//update the gaussian filtered output
	filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
	filter_update(filter);
	output_update(&out_traffic_sign_filtered);
		
	output.ival = 0;
	return (output);
}

//***********************************************************
//* Function: f_keyboard
//* Description: Called whenever a key is pressed
//***********************************************************
void f_keyboard (char *key_value)
{
	char key;
	FILTER_DESC *filter;
	
	key = key_value[0];
	switch (key)
	{
		// Moves the input to the next photo
		case 'n':
			GetNewFrame (&traffic_sign, DIRECTION_FORWARD);
			
			//update the first output
			filter = get_filter_by_output(out_traffic_sign.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign);
			//update the gaussian filtered output
			filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign_filtered);
			break;
		case 'b':
			GetNextSign (&traffic_sign, DIRECTION_FORWARD);
			//update the first output
			filter = get_filter_by_output(out_traffic_sign.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign);
			//update the gaussian filtered output
			filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign_filtered);
			break;
		case 'v':
			GetNextSign (&traffic_sign, DIRECTION_REWIND);
			//update the first output
			filter = get_filter_by_output(out_traffic_sign.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign);
			//update the gaussian filtered output
			filter = get_filter_by_output(out_traffic_sign_filtered.neuron_layer);
			filter_update(filter);
			output_update(&out_traffic_sign_filtered);
			break;
		case 'a':
			g_index_sign_moving++;
			GetNthSign (g_index_sign_moving);
			break;
		case 'q':
			g_index_sign_moving--;
			GetNthSign (g_index_sign_moving);
			break;
		case 'd':
			printf("ok\n");
			break;
	}
}

