#include "robot_user_functions.h"

// Variaveis globais
int g_nLeftEye, g_nRightEye;
SOCKET g_SocketLeft, g_SocketRight;
char chrBuffer[MAX_PACKAGE];	// O tamanho maximo de um pacote TCP �1500 bytes. Foi colocado 1600 apenas para dar uma folga
GLubyte pImage[INPUT_WIDTH * INPUT_HEIGHT];
int nTamImageLeft, nTamImageRight;
int g_nColorImage = COLOR_MONO_256;

double dblTempo1, dblTempo2;
int nFrame = 0;
int nTotalFrame = 0;

int g_Status;

int g_nMoveLeft;

int g_nVerticalGap = VERT_GAP;

float *g_fltDispAux;
float *g_fltConfAux;
DISP_DATA *g_DispData;
DISP_DATA *g_AllDispData;
int nAllSamples;

RECEPTIVE_FIELD_DESCRIPTION *g_ReceptiveField;

// Fuzzy variables
double g_p_dblFuzzyInputs[FUZZY_INPUTS_NUMBER];
double g_p_dblFuzzyOutputs[FUZZY_OUTPUTS_NUMBER];

// ----------------------------------------------------------------------------
// SalvaImagens - Salva as imagens
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SalvaImagens (void)
{
	FILE *file;
	int x, y;
	
	// Salva a imagem esquerda somente se ela for adquira via socket
	if (image_left.input_generator_params->next->param.ival == GET_IMAGE_FROM_SOCKET)
	{
		file = fopen(FILE_RBT_IMAGE_LEFT,"w");
	
		for (x = 0; x < INPUT_WIDTH; x++)
		{
			for (y = 0; y < INPUT_HEIGHT; y++)
			{
				fprintf(file,"%c", (char)image_left.image[3 * (x + y*image_left.tfw)]);
			}
		}
		fclose(file);
	}
	
	// Salva a imagem direita somente se ela for adquira via socket
	if (image_right.input_generator_params->next->param.ival == GET_IMAGE_FROM_SOCKET)
	{
		file = fopen(FILE_RBT_IMAGE_RIGHT,"w");

		for (x = 0; x < INPUT_WIDTH; x++)
		{
			for (y = 0; y < INPUT_HEIGHT; y++)
			{
				fprintf(file,"%c", (char)image_right.image[3 * (x + y*image_right.tfw)]);
			}
		}
		fclose(file);
	}
	glutIdleFunc ((void (* ) (void)) check_forms);
	//glutIdleFunc ((void (* ) (void)) fl_check_forms);
}

// ----------------------------------------------------------------------------
// Connect - Cria o socket e conecta com o servidor
//           de imagens.
//
// Entrada: strAddr - Endereco do servidor
//
// Saida: Socket. Retorna 0 caso falhe.
// ----------------------------------------------------------------------------
#ifndef WINDOWS
int Connect(char *strAddr)
{
	struct sockaddr_in addr;
	struct hostent *he;
	SOCKET r = 0;

	// Cria o socket
	r = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == r)
	{
		printf("Could not create socket.\n");
		return 0;
	}

	// resolve localhost to an IP
	if ((he = gethostbyname(strAddr)) == NULL)
	{
		puts("Error resolving hostname...\n");
		return 0;
	}

	memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOCKET_PORT);

	if(connect(r, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		printf("Cannot connect to server\n");
		closesocket(r);
		return 0;
	}
	printf("Connected.\n");

	return r;
}
#endif
// ----------------------------------------------------------------------------
// init -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
int init (INPUT_DESC *input)
{
	input->wx = input->x_old = (input->vpw - input->neuron_layer->dimentions.x)/2;
	input->wxd = input->wxd_old = input->vpw / 2;

	input->wy = input->y_old = (input->vph - input->neuron_layer->dimentions.y)/2;
	input->wyd = input->wyd_old = input->vph / 2;
	
	return (0);
}

// ----------------------------------------------------------------------------
// robot_make_input_image - Inicializa a estrutura image
// da input
//
// Entrada: input - Descritor da input
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void robot_make_input_image (INPUT_DESC *input)
{
	int w, h;
	char message[256];

	w = input->neuron_layer->dimentions.x;
	h = input->neuron_layer->dimentions.y;

	// Compute the input image dimentions
	input->tfw = nearest_power_of_2 (w);
	input->tfh = nearest_power_of_2 (h);

	// Save the image dimentions
	input->ww = w;
	input->wh = h;
	
	switch(TYPE_SHOW)
	{
		case SHOW_FRAME:
			input->vpw = input->neuron_layer->dimentions.x;
			input->vph = input->neuron_layer->dimentions.y;
			break;
		case SHOW_WINDOW:
			input->vpw = input->ww;
			input->vph = input->wh;
			break;
		default:
			sprintf(message,"%d. It can be SHOW_FRAME or SHOW_WINDOW.",TYPE_SHOW);
			Erro ("Invalid Type Show ", message, " Error in update_input_image.");
			return;
	}
	
	input->vpxo = 0;
	input->vpyo = h - input->vph;
	
	if(input->image == (GLubyte *) NULL)
		input->image = (GLubyte *) alloc_mem (3 * input->tfw * input->tfh * sizeof (GLubyte));
}

// ----------------------------------------------------------------------
// input_special - keyboard special key handler
//
// Inputs:  pInput - a pointer to the input struct
//          key - the keyboard button
//         (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

void input_special (int key, int x, int y)
{
#ifndef WINDOWS        
	switch (key)
	{
		case GLUT_KEY_LEFT:
                        SimulatorRobotTurn (-TURN_LEFT_STEP);
			IRSendSignal (nSinalEsquerda,5);
			break;
		case GLUT_KEY_RIGHT:
                        SimulatorRobotTurn (TURN_RIGHT_STEP);
			IRSendSignal (nSinalDireita,5);
			break;
		case GLUT_KEY_DOWN:
                        SimulatorRobotMove (-MOVE_FORWARD_STEP);
			break;
		case GLUT_KEY_UP:
                        SimulatorRobotMove (MOVE_FORWARD_STEP);
			IRSendSignal (nSinalFrente,5);
                        break;
	}
#else
	switch (key)
	{
		case GLUT_KEY_LEFT:
                        SimulatorRobotTurn (-TURN_LEFT_STEP);
			break;
		case GLUT_KEY_RIGHT:
                        SimulatorRobotTurn (TURN_RIGHT_STEP);
			break;
		case GLUT_KEY_DOWN:
                        SimulatorRobotMove (-MOVE_FORWARD_STEP);
			break;
		case GLUT_KEY_UP:
                        SimulatorRobotMove (MOVE_FORWARD_STEP);
                        break;
	}
#endif
        glutPostRedisplay ();

	return;
}

// ----------------------------------------------------------------------------
// InitWindow - Inicializa os recursos de cada janela
//				de entrada
//
// Entrada: input - Janela de entrada
//			nInputImage - Indica qual e a janela.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void InitWindow(INPUT_DESC *input, int nInputImage)
{
	int x, y;
	int nImageSource;
	
	printf("%s\n",input->name);

	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;

	switch (nImageSource)
	{
#ifndef WINDOWS
		case GET_IMAGE_FROM_SOCKET:
			// Conecta com o servidor
			if (VISION_STEREO)
			{
				if (nInputImage == INPUT_IMAGE_LEFT)
				{
					if ((g_SocketLeft = Connect(HOST_ADDR_LEFT)) == 0)
						ERRO_CONEXAO
				}
				else
				{
					if ((g_SocketRight = Connect(HOST_ADDR_RIGHT)) == 0)
						ERRO_CONEXAO
				}
			}
			else
			{
				if (nInputImage == INPUT_IMAGE_LEFT)
				{
					if ((g_SocketLeft = Connect(HOST_ADDR_MONO)) == 0)
						ERRO_CONEXAO
				}
				else
				{
					g_SocketRight = g_SocketLeft;
				}
			}
			IRInitialize ();
			robot_make_input_image (input);
#endif
			break;
		case GET_IMAGE_FROM_RBT:
			robot_make_input_image (input);
			break;
		case GET_IMAGE_FROM_PNM:
			make_input_image (input);
			break;
		case GET_IMAGE_FROM_SIMULATOR:
			robot_make_input_image (input);
			SimulatorSetParameterf (SIMULATOR_FOVY, ROBOT_FOVY);
			SimulatorSetParameterf (SIMULATOR_FAR, ROBOT_FAR);
			SimulatorSetParameterf (SIMULATOR_NEAR, ROBOT_NEAR);
			SimulatorSetParameterf (SIMULATOR_FLOOR, ROBOT_FLOOR);
			SimulatorSetParameterf (SIMULATOR_FOCUS, ROBOT_FOCUS);
			SimulatorSetParameterf (SIMULATOR_CAMERA_SEPARATION, ROBOT_CAMERA_SEPARATION);
			SimulatorSetParameterf (SIMULATOR_ROBOT_RADIUS, ROBOT_RADIUS);
			SimulatorSetParameterf (SIMULATOR_ROBOT_HEIGHT, ROBOT_HEIGHT);		
			SimulatorSetParameterf (SIMULATOR_TERRAIN_SIZE, TERRAIN_SIZE);		
			SimulatorInitialize ();
                        SimulatorLinkLeftImage (image_left.image, image_left.ww, image_left.wh);
                        SimulatorLinkRightImage (image_right.image, image_right.ww, image_right.wh);
			break;
		default:
			Erro ("Invalid image source.", "", "");
	}

	init (input);

	glutInitWindowSize (input->ww, input->wh);
	if (read_window_position (input->name, &x, &y))
		glutInitWindowPosition (x, y);
	else
		glutInitWindowPosition (-1, -1);
	input->win = glutCreateWindow (input->name);

	if (strcmp(input->name, image_left.name) == 0)
		g_nLeftEye = input->win;
	else
		g_nRightEye = input->win;

	glGenTextures (1, (GLuint *)(&(input->tex)));
	input_init (input);
	glutReshapeFunc (input_reshape);
	glutDisplayFunc (input_display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc (input_passive_motion);
	glutMouseFunc (input_mouse);
	glutSpecialFunc (input_special);

}



// ----------------------------------------------------------------------------
// GetImage - Busca uma imagem do servidor
//                      de imagens
//
// Entrada: input - INPUT_DESC
//          s - Socket no qual ser�lida a imagem
//          nTamImage - Tamanho da imagem a ser lida
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------

void GetImage(INPUT_DESC *input, int nInputImage)
{
#ifndef WINDOWS
	int nBytesPacote, nBytesImagem;
	int *n_pAux;
	SOCKET *s;
#endif
	int nTamImage;
	int i, x, y;
	GLubyte pPonto;
	FILE *file;
	int nImageSource;
	
	// Determina qual a fonte da imagem
	nImageSource = input->input_generator_params->next->param.ival;
		
	switch (nImageSource)
	{
#ifndef WINDOWS
		case GET_IMAGE_FROM_SOCKET:
			// Recebe o tamanho da Imagem
			s = (nInputImage == INPUT_IMAGE_LEFT) ? &g_SocketLeft : &g_SocketRight;

			switch(g_nColorImage)
			{
				case COLOR_MONO_256:
					send(*s, MSG_SNAP_MONO_256, strlen(MSG_SNAP_MONO_256), 0);
					break;
				case COLOR_332:
					send(*s, MSG_SNAP_332, strlen(MSG_SNAP_332), 0);
					break;
				case COLOR_MONO_8:
					send(*s, MSG_SNAP_MONO_8, strlen(MSG_SNAP_MONO_8), 0);
					break;
			}
			
			nBytesPacote = recv(*s, chrBuffer, 4, 0);
			n_pAux = (int*)chrBuffer;
			nTamImage = *n_pAux;
	
			// Recebe os bits da imagem
			nBytesImagem = 0;
			while (nBytesImagem < nTamImage)
			{
				//nBytesPacote = recv(*s, chrBuffer, nTamImage, 0);
				nBytesPacote = read(*s, &pImage[nBytesImagem], nTamImage);
				nBytesImagem += nBytesPacote;
			}
			
			// Coloca a imagem na estrutura da MAE
			switch (g_nColorImage)
			{
				case COLOR_MONO_256:
					for (x = 0; x < INPUT_WIDTH; x++)
					{
						for (y = 0; y < INPUT_HEIGHT; y++)
						{
							pPonto = pImage[x + y * INPUT_WIDTH];
							input->image[3 * (x + y*input->tfw) + 0] = pPonto;
							input->image[3 * (x + y*input->tfw) + 1] = pPonto;
							input->image[3 * (x + y*input->tfw) + 2] = pPonto;
						}
					}
					break;
				case COLOR_332:
					for (x = 0; x < INPUT_WIDTH; x++)
					{
						for (y = 0; y < INPUT_HEIGHT; y++)
						{
							pPonto = pImage[x + y * INPUT_WIDTH];
							input->image[3 * (x + y*input->tfw) + 0] = pPonto & 0x00E0;
							input->image[3 * (x + y*input->tfw) + 1] = (pPonto & 0x001C) << 3;
							input->image[3 * (x + y*input->tfw) + 2] = (pPonto & 0x0003) << 6;
						}
					}
					break;
				case COLOR_MONO_8:
					for (x = 0; x < INPUT_WIDTH; x++)
					{
						for (y = 0; y < INPUT_HEIGHT; y++)
						{
							pPonto = pImage[x + y * INPUT_WIDTH];
							input->image[3 * (x + y*input->tfw) + 0] = pPonto * 32;
							input->image[3 * (x + y*input->tfw) + 1] = pPonto * 32;
							input->image[3 * (x + y*input->tfw) + 2] = pPonto * 32;
						}
					}
					break;
			}
			input->up2date = 0;
			break;
#endif
		case GET_IMAGE_FROM_RBT:
			nTamImage = INPUT_WIDTH * INPUT_HEIGHT;
			if (input == (&image_left))
				file = fopen(FILE_RBT_IMAGE_LEFT,"r");
			else
				file = fopen(FILE_RBT_IMAGE_RIGHT,"r");

			fread ((void*)pImage, 1, nTamImage, file);
			fclose(file);
			
			// Coloca a imagem na estrutura da MAE
			for (x = 0, i = 0; x < INPUT_WIDTH; x++)
			{
				for (y = 0; y < INPUT_HEIGHT; y++)
				{
					pPonto = pImage[i++];
					input->image[3 * (x + y*input->tfw) + 0] = pPonto;
					input->image[3 * (x + y*input->tfw) + 1] = pPonto;
					input->image[3 * (x + y*input->tfw) + 2] = pPonto;
				}
			}
			input->up2date = 0;
			break;
		case GET_IMAGE_FROM_PNM:
			break;
		case GET_IMAGE_FROM_SIMULATOR:
			SimulatorForceUpdate ();
			input->up2date = 0;
			break;
		default:
			Erro ("Invalid image source.", "", "");
	}
	
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
	update_input_neurons (input);
	update_input_image (input);
}



// ----------------------------------------------------------------------------
// SendSnap - Envia uma mensage para capturar a imagem
//
// Entrada: s - Socket no qual ser�lida a imagem
//
// Saida: int - Tamanho da imagem a ser capturada
// ----------------------------------------------------------------------------
#ifndef WINDOWS
int SendSnap(SOCKET *s)
{
	int *nTamImagem;

	send(*s, MSG_SNAP_MONO_256, strlen(MSG_SNAP_MONO_256), 0);
	recv(*s, chrBuffer, 4, 0);
	nTamImagem = (int*)chrBuffer;
	return (*nTamImagem);
}
#endif

// ----------------------------------------------------------------------------
// LoadCameraParams - Loads up the cameras parameters and calibration constants from the data file
//
// Entrada: 
//
// Saida: 
// ----------------------------------------------------------------------------
void LoadCameraParams (void)
{
	FILE *data_fd;
	
	data_fd = fopen (CAMERA_LEFT_PARAMETERS_FILE_NAME, "r");
	load_cp_cc_data (data_fd, &cameraLeftCalibrationParameters, &cameraLeftCalibrationConstants);
	fclose (data_fd);
			
	data_fd = fopen (CAMERA_RIGHT_PARAMETERS_FILE_NAME, "r");
	load_cp_cc_data (data_fd, &cameraRightCalibrationParameters, &cameraRightCalibrationConstants);
	fclose (data_fd);	
	printf("Left and right camera parameters and calibration constants loaded up.\n");
}

// ----------------------------------------------------------------------------
// init_user_functions -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
int init_user_functions()
{
	char strCommand[128];

	sprintf (strCommand, "toggle move_active;");
	interpreter (strCommand);
	
	sprintf (strCommand, "toggle draw_active;");
	interpreter (strCommand);

	LoadCameraParams();
	
	// Initializes the Viewer module	
	ViewerSetParameterf (VIEWER_FOVY, ROBOT_FOVY);
	ViewerSetParameterf (VIEWER_FAR, ROBOT_FAR);
	ViewerSetParameterf (VIEWER_NEAR, ROBOT_NEAR);
	ViewerSetParameterf (VIEWER_FLOOR, ROBOT_FLOOR);
	ViewerSetParameterf (VIEWER_FOCUS, ROBOT_FOCUS);
	ViewerSetParameterf (VIEWER_ROBOT_RADIUS, ROBOT_RADIUS);
	ViewerSetParameterf (VIEWER_TERRAIN_SIZE, TERRAIN_SIZE);		
	ViewerSetParameteri (VIEWER_IMAGE_WIDTH, INPUT_WIDTH);
	ViewerSetParameteri (VIEWER_IMAGE_HEIGHT, INPUT_HEIGHT);
	ViewerInitialize ();
	
	// Initializes the Fuzzy module
	FuzzyInitialize ();

	return (0);
}



// ----------------------------------------------------------------------------
// UpdateStereoSystem - updates the stereo system architecture
//
// Inputs: None
//
// Outputs: None
// ----------------------------------------------------------------------------

void UpdateStereoSystem (void)
{
	filter_update (get_filter_by_output (&nl_simple_mono_right));
	filter_update (get_filter_by_output (&nl_simple_mono_right_q));
	filter_update (get_filter_by_output (&nl_simple_mono_left));
	filter_update (get_filter_by_output (&nl_simple_mono_left_q));
	filter_update (get_filter_by_output (&nl_simple_binocular));
	filter_update (get_filter_by_output (&nl_simple_binocular_q));
	filter_update (get_filter_by_output (&nl_complex_mono_right));
	filter_update (get_filter_by_output (&nl_complex_mono_left));
	filter_update (get_filter_by_output (&nl_complex_binocular));
	filter_update (get_filter_by_output (&nl_mt));
	filter_update (get_filter_by_output (&nl_mt_gaussian));
	output_update (&out_confidence_map);
	output_update (&out_disparity_map);

	return;
}



// ----------------------------------------------------------------------------
// UpdateVisualSearchSystem - updates the visual search system mechanism
//
// Inputs: None
//
// Outputs: None
// ----------------------------------------------------------------------------

void UpdateVisualSearchSystem (void)
{
/*	filter_update (get_filter_by_output (&nl_gabor_big_h_00));
	filter_update (get_filter_by_output (&nl_gabor_big_h_90));
	filter_update (get_filter_by_output (&nl_gabor_big_v_00));
	filter_update (get_filter_by_output (&nl_gabor_big_v_90));
	filter_update (get_filter_by_output (&nl_gabor_small_h_00));
	filter_update (get_filter_by_output (&nl_gabor_small_h_90));
	filter_update (get_filter_by_output (&nl_gabor_small_v_00));
	filter_update (get_filter_by_output (&nl_gabor_small_v_90));
	filter_update (get_filter_by_output (&nl_features1));
	filter_update (get_filter_by_output (&nl_v1_activation_map));
	output_update (&out_features);
	output_update (&out_v1_activation_map);
	output_update (&out_target_coordinates);

*/	return;
}



// ----------------------------------------------------------------------------
// input_generator -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------

void input_generator (INPUT_DESC *input, int status)
{
	int nInputImage;
	//struct timeval tv1;
	//struct timeval tv2;

	// Gets the image side (left or right)
	nInputImage = (strcmp (input->name, image_left.name) == 0) ? INPUT_IMAGE_LEFT : INPUT_IMAGE_RIGHT;

	// Initializes the input OpenGL window
	if (input->win == 0)
	{
		InitWindow(input, nInputImage);
		//g_Status = ST_GET_IMAGE;
	}
	
	/*if ((nFrame < remaining_steps) && (running))
	{
		gettimeofday(&tv1,NULL);
		nFrame = remaining_steps;
		nTotalFrame = remaining_steps;
		dblTempo1 = (double)tv1.tv_sec + (double)tv1.tv_usec*1e-6;

		g_Status = ST_GET_IMAGE;
	}*/
	
	// Refreshes the input image
	GetImage(input, nInputImage);

	/* Refreshes the input image
	if (g_Status == ST_GET_IMAGE)
	{
		GetImage(input, nInputImage);
		
		if (nInputImage == INPUT_IMAGE_LEFT)
			GetImage(input, nInputImage);
		else
		{
			GetImage(input, nInputImage);
			nFrame--;
		}
		
		check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);
		update_input_neurons (input);
		update_input_image (input);
	}*/

	// Forca a atualizacao dos filtros quando e trocado o ponto de atencao,
	// mas nao esta em modo running. So eh necessario quando for a o olho
	// esquerdo pois, se mover o olho direito, o esquerdo move junto.
	
	// Updates the stereo vision system when the left input moves 
	if ((status == MOVE) && (!running) && (nInputImage == INPUT_IMAGE_LEFT))
		UpdateStereoSystem ();

	// Updates the visual search mechanism when the right input moves
	if ((status == MOVE) && (!running) && (nInputImage == INPUT_IMAGE_RIGHT))
		UpdateVisualSearchSystem ();
		
	// Calcula o Frame Rate
/*	if ((nFrame == 0) && (running))
	{
		g_Status = ST_WAIT;
		gettimeofday(&tv2,NULL);
		dblTempo2 = (double)tv2.tv_sec + (double)tv2.tv_usec*1e-6;
		printf("Tempo: %2.2lfseg - FrameRate: %2.2lf\n",dblTempo2 - dblTempo1, (double)nTotalFrame / (dblTempo2 - dblTempo1));
	}*/
}



// ----------------------------------------------------------------------------
// draw_output -
//
// Entrada:
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void draw_output (char *output_name, char *input_name)
{
}



// ----------------------------------------------------------------------------------
// calculateWorldPoint -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------------
WORLD_POINT calculateWorldPoint (IMAGE_COORDINATE distortedLeftPoint, IMAGE_COORDINATE distortedRightPoint)
{
	IMAGE_COORDINATE undistortedLeftPoint;
	IMAGE_COORDINATE undistortedRightPoint;
	IMAGE_COORDINATE leftPrincipalPoint;
	IMAGE_COORDINATE rightPrincipalPoint;
	double fltCameraLeftFocus;
	double fltCameraRightFocus;
	double fltCameraDistance;
	
	// Calculates the undistorted image left coordinates  
	//cc = cameraLeftCalibrationConstants;
	//cp = cameraLeftCalibrationParameters;
	//distorted_to_undistorted_image_coord (distortedLeftPoint.x, distortedLeftPoint.y, &undistortedLeftPoint.x, &undistortedLeftPoint.y);
	undistortedLeftPoint.x = distortedLeftPoint.x;
	undistortedLeftPoint.y = distortedLeftPoint.y;
	
	// Calculates the undistorted image right coordinates  
	//cc = cameraRightCalibrationConstants;
	//cp = cameraRightCalibrationParameters;
	//distorted_to_undistorted_image_coord (distortedRightPoint.x, distortedRightPoint.y, &undistortedRightPoint.x, &undistortedRightPoint.y);
	undistortedRightPoint.x = distortedRightPoint.x;
	undistortedRightPoint.y = distortedRightPoint.y;

	// Gets the camera left principal point
	//leftPrincipalPoint.x = cameraLeftCalibrationParameters.Cx;
	//leftPrincipalPoint.y = cameraLeftCalibrationParameters.Cy;
	leftPrincipalPoint.x = (float) INPUT_WIDTH/2.0f;
	leftPrincipalPoint.y = (float) INPUT_WIDTH/2.0f;
	
	// Gets the camera right principal point
	//rightPrincipalPoint.x = cameraRightCalibrationParameters.Cx;
	//rightPrincipalPoint.y = cameraRightCalibrationParameters.Cy;
	rightPrincipalPoint.x = (float) INPUT_WIDTH/2.0f;
	rightPrincipalPoint.y = (float) INPUT_WIDTH/2.0f;

	// Gets the camera left focal distance
	//fltCameraLeftFocus = cameraLeftCalibrationConstants.f;
	fltCameraLeftFocus = ROBOT_FOCUS;

	// Gets the camera right focal distance
	//fltCameraRightFocus = cameraRightCalibrationConstants.f;
	fltCameraRightFocus = ROBOT_FOCUS;

	// Gets the displacement between camera left and camera right
	fltCameraDistance = ROBOT_CAMERA_SEPARATION;
	
	return TMapCalculateWorldPoint (undistortedLeftPoint,
				       				undistortedRightPoint,
				      				leftPrincipalPoint,
				       				rightPrincipalPoint,
				       				fltCameraLeftFocus,
				       				fltCameraRightFocus,
				       				fltCameraDistance);
}



// ----------------------------------------------------------------------------
// ReconstructByDisparityMap - Reconstroi o mundo em
// torno do ponto de vergencia atraves do mapa de
// disparidade.
//
// Entrada: Nenhuma.
//
// Saida: Nenhuma.
// ----------------------------------------------------------------------------
void ReconstructByDisparityMap (void)
{
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point;
	int  u, v;
	int w, h, wi, hi, xi, yi;
	int x_center, x_center_left, y_center_left, y_center;
	float fltDisparity;
	GLubyte red, green, blue;
	int pixel;
	float fltRangeCols;
	int nStartCol, nEndCol;

	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	// NAO SEI PORQUE.
	// PRECISA DISTO PARA FUNCIONAR DEVIDO A ALTERACAO FEITA POR DIJALMA
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = image_right.wxd - image_right.wx;
		y_center = image_right.wyd - image_right.wy;
		x_center_left = image_left.wxd - image_left.wx;
		y_center_left = image_left.wyd - image_left.wy;
	}
	else
	{
		x_center = wi/2;
		y_center = hi/2;
		x_center_left = wi/2;
		y_center_left = hi/2 - 20;
	}

	fltRangeCols = 1.0;
	nStartCol = w * ((1.0 - fltRangeCols) / 2.0);
	nEndCol = w - nStartCol;

	for (v = 0; v < h; v++)
	{
		for (u = nStartCol; u < nEndCol; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			pixel = image_right.neuron_layer->neuron_vector[(yi * wi) + xi].output.ival;
			red = (GLubyte) RED(pixel);
			green = (GLubyte) GREEN(pixel);
			blue = (GLubyte) BLUE(pixel);
			
			right_point.x = (double) (xi);
			right_point.y = (double) (yi);

			switch (nl_disparity_map.output_type)
			{
				case GREYSCALE:
					fltDisparity = (float) nl_disparity_map.neuron_vector[w * v + u].output.ival;
					break;
				case GREYSCALE_FLOAT:
					fltDisparity = nl_disparity_map.neuron_vector[w * v + u].output.fval;
					break;
				default:
					fltDisparity = nl_disparity_map.neuron_vector[w * v + u].output.fval;
			}

			// Achar a coordenada relativa na imagem esquerda
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center_left, y_center_left, (double) h / (double) (h - 1), LOG_FACTOR);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			left_point.x = (double) (xi + fltDisparity);
			left_point.y = (double) (yi);

			world_point = calculateWorldPoint (left_point, right_point);
			
			// Updates the map and rotates the frame coordinates			       			
			ViewerUpdateMap(world_point.z, -world_point.x, world_point.y, red, green, blue);
		}
	}
}



// ----------------------------------------------------------------------------
// GetShiftDisparity - Retorna o valor da output do
//					   filtro shift_disparity_filter
//
// Entrada: strNLName - Nome da neuron layer associada
//						ao filtro
//
// Saida: int - Valor da output do filtro
// ----------------------------------------------------------------------------
int GetShiftDisparity (char *strNLName)
{
	NEURON_LAYER *nl;

	nl = get_neuron_layer_by_name(strNLName);
	return nl->neuron_vector[0].output.ival;
}

// ----------------------------------------------------------------------------
// SumOutputCells - Soma o valor da saida de todas as celulas de uma neuron
//					layer
//
// Entrada: nl - Neuron Layer
//
// Saida: float - Somatorio da saida das celulas de nl
// ----------------------------------------------------------------------------
float SumOutputCells(NEURON_LAYER *nl)
{
	int i;
	float fltSum;

	fltSum = 0.0;
	for (i = 0; i < (nl->dimentions.x * nl->dimentions.y); i++)
		fltSum += nl->neuron_vector[i].output.fval;

	return fltSum;
}

// ----------------------------------------------------------------------------
// InitDispDataState - Inicializa o estado da estrutura DISP_DATA
//
// Entrada: nNumNeurons: Quantidade de neuronios da neuron_layer de disparidade
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void InitDispDataState (int nNumNeurons)
{
	int i, sample;

	for (i = 0; i < nNumNeurons; i++)
	{
		g_DispData->neg_slope[i] = 1;
	}

	for (sample = 0; sample < NUM_SAMPLES; sample++)
	{
		for (i = 0; i < nNumNeurons; i++)
		{
			g_DispData->samples[sample][i].val = FLT_MAX;
			g_DispData->samples[sample][i].conf = 0.0;
			g_DispData->samples[sample][i].pos = 0;
		}
	}
}

// ----------------------------------------------------------------------------
// AddLocalMin - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void AddLocalMin(int i, double minus_out, double minus_conf, int pos)
{
	int num_samples = NUM_SAMPLES;
	int victim_sample, moving_sample;
	
	for (victim_sample = 0; victim_sample < num_samples; victim_sample++)
		if (minus_out <= g_DispData->samples[victim_sample][i].val)
			break;
			
	if (victim_sample < num_samples)
	{
		moving_sample = num_samples - 1;
		while (moving_sample > victim_sample)
		{
			g_DispData->samples[moving_sample][i] = g_DispData->samples[moving_sample - 1][i];
			moving_sample--;
		}
		g_DispData->samples[moving_sample][i].val = minus_out;
		g_DispData->samples[moving_sample][i].conf = minus_conf;
		g_DispData->samples[moving_sample][i].pos = pos;
	}
}

// ----------------------------------------------------------------------------
// SetVergence - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void SetVergence (void)
{
	int nMaxScan, nMinScan;
	int i, j, x, y, nIndex;
	int wo, ho;
	float fltSumOutputCells;
	float fltMinSum, fltAux, fltSum;
	int nVergLeft;
#define	FRACTION_OF_IMAGE_SCANNED	5

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	if (g_fltDispAux == NULL)
	{
		g_fltDispAux = (float*)malloc(sizeof(float)*wo*ho);
		g_fltConfAux = (float*)malloc(sizeof(float)*wo*ho);
	}

	if (g_DispData == NULL)
	{
		// Aloca memoria para a estrutura DISP_DATA
		g_DispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_DispData->neg_slope = (char *) alloc_mem (wo * ho);
		g_DispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * NUM_SAMPLES);
		for (i = 0; i < NUM_SAMPLES; i++)
			g_DispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
		nAllSamples = (image_right.neuron_layer->dimentions.x / FRACTION_OF_IMAGE_SCANNED);
		g_AllDispData = (DISP_DATA *) alloc_mem (sizeof (DISP_DATA));
		g_AllDispData->samples = (SAMPLES **) alloc_mem (sizeof (SAMPLES *) * nAllSamples);
		for (i = 0; i < nAllSamples; i++)
			g_AllDispData->samples[i] = (SAMPLES *) alloc_mem (sizeof (SAMPLES) * (wo * ho));
	}

	// Inicializar a estrutura DISP_DATA
	InitDispDataState(wo * ho);

	// Inicializar o mapa de disparidade e a estrutura auxiliar
	for (i = 0; i < (wo * ho); i++)
	{
		g_fltDispAux[i] = FLT_MAX;
		g_fltConfAux[i] = 0.0;
		nl_mt_gaussian_map.neuron_vector[i].output.fval = 0.0;
		nl_confidence_map.neuron_vector[i].output.fval = 0.0;
		nl_disparity_map.neuron_vector[i].output.fval = 0.0;
	}

	nMinScan = image_right.wxd;
	nMaxScan = image_right.wxd + (image_right.neuron_layer->dimentions.x / FRACTION_OF_IMAGE_SCANNED);
	if (nMaxScan > image_right.neuron_layer->dimentions.x)
		nMaxScan = image_right.neuron_layer->dimentions.x;
	nAllSamples = (nMaxScan - nMinScan);

	fltMinSum = FLT_MAX;
	for (image_left.wxd = nVergLeft = nMinScan, image_left.wyd = image_right.wyd; image_left.wxd < nMaxScan; image_left.wxd++)
	{
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		fltSumOutputCells = 0.0;
		for (x = 0; x < wo; x++)
		{
			for (y = 0; y < ho; y++)
			{
				nIndex = y * wo + x;
				fltSumOutputCells += nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				fltAux = nl_mt_gaussian.neuron_vector[nIndex].output.fval;
				
				if (fltAux >= g_fltDispAux[nIndex])
				{
					if (g_DispData->neg_slope[nIndex])
					{
						g_DispData->neg_slope[nIndex] = 0;
						AddLocalMin(nIndex, g_fltDispAux[nIndex], g_fltConfAux[nIndex], image_left.wxd - 1);
						nl_mt_gaussian_map.neuron_vector[nIndex].output.fval = g_fltDispAux[nIndex];
						nl_confidence_map.neuron_vector[nIndex].output.fval = g_fltConfAux[nIndex];
						nl_disparity_map.neuron_vector[nIndex].output.fval = image_left.wxd - 1;
					}
				}
				else
					g_DispData->neg_slope[nIndex] = 1;

				g_fltDispAux[nIndex] = fltAux;
				g_fltConfAux[nIndex] = nl_complex_mono_right.neuron_vector[nIndex].output.fval + nl_complex_mono_left.neuron_vector[nIndex].output.fval;
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].val = g_fltDispAux[nIndex];
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].conf = g_fltConfAux[nIndex];
				g_AllDispData->samples[image_left.wxd - nMinScan][nIndex].pos = image_left.wxd;
			}
		}

		if (fltSumOutputCells < fltMinSum)
		{
			fltMinSum = fltSumOutputCells;
			nVergLeft = image_left.wxd;
		}
	}

	// Imprime os minimos armazenados na MT CELL que foi plotada.
	for (i = 0; i < NUM_SAMPLES; i++)
		printf ("Min %d - mt_cell[%d] - pos:%d - val: %.3f - conf: %.3f\n", i, INDEX_MT_CELL, g_DispData->samples[i][INDEX_MT_CELL].pos, g_DispData->samples[i][INDEX_MT_CELL].val, g_DispData->samples[i][INDEX_MT_CELL].conf);

	// Escolhe, inicialmente, a menor disparidade
	for (i = 0; i < (wo * ho); i++)
	{
		nl_mt_gaussian_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].val;
		nl_confidence_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].conf;
		nl_disparity_map.neuron_vector[i].output.fval = g_DispData->samples[0][i].pos;
	}

	// Correcao do mapa de disparidade de acordo com a vergencia escolhida
	for (i = 0; i < (wo * ho); i++)
	{
		nl_disparity_map.neuron_vector[i].output.fval -= nVergLeft;
		for (j = 0; j < nAllSamples; j++)
			g_AllDispData->samples[j][i].pos -= nVergLeft;
	}

	// Calcula as variacoes das saidas de MT gaussian
	for (i = 0; i < (wo * ho); i++)
	{
		fltSum = 0.0;
		for (j = 0; j < nAllSamples; j++) 
			fltSum += g_AllDispData->samples[j][i].val;
		nl_mt_gaussian_var.neuron_vector[i].output.fval = (fltSum / nAllSamples) - g_DispData->samples[0][i].val;
	}

	image_left.wxd = nVergLeft;
	move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	printf("nVergLeft: %d\n", nVergLeft);

	glutPostWindowRedisplay (image_left.win);
}



// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void input_controler (INPUT_DESC *input, int status)
{
	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nRightEye))
	{
		image_left.wxd += image_right.wxd - image_right.wxd_old;
		image_left.wyd = image_right.wyd + g_nVerticalGap;

		move_input_window (image_right.name, image_right.wxd, image_right.wyd);
		move_input_window (image_left.name, image_left.wxd, image_left.wyd);
	}

	if ((move_active == 1) &&
	    (input->mouse_button == GLUT_LEFT_BUTTON) &&
	    (input->mouse_state == GLUT_DOWN) &&
	    (input->win == g_nLeftEye))
	{
		image_left.wyd = image_right.wyd + g_nVerticalGap;

		move_input_window (image_left.name, image_left.wxd, image_left.wyd);

		glutPostWindowRedisplay (image_left.win);
	}

	input->mouse_button = -1;
}

// ----------------------------------------------------------------------------
// input_controler -
//
// Entrada:
//
// Saida:
// ----------------------------------------------------------------------------
void output_handler(OUTPUT_DESC *output, int status)
{
	printf("output_handler\n");
}

// ----------------------------------------------------------------------------
// LowPass - Passa uma gaussiana no mapa de disparidade
//
// Entrada: Nenhuma
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void LowPass()
{
	int x, y, wo, ho;

	if (g_fltDispAux == NULL)
	{
		return;
	}

	if (g_ReceptiveField == NULL)
	{
		g_ReceptiveField = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(g_ReceptiveField, 5, 1.0);
	}

	wo = nl_disparity_map.dimentions.x;
	ho = nl_disparity_map.dimentions.y;

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			g_fltDispAux[y * wo + x] = apply_gaussian_kernel(g_ReceptiveField, &nl_disparity_map, x, y);
		}
	}

	for (y = 0; y < ho; y++)
	{
		for (x = 0; x < wo; x++)
		{
			nl_disparity_map.neuron_vector[y * wo + x].output.fval = g_fltDispAux[y * wo + x];
		}
	}
	//glutIdleFunc ((void (* ) (void))fl_check_forms);
	glutIdleFunc ((void (* ) (void)) check_forms);
}

// ----------------------------------------------------------------------------
// distance_neig - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
int distance_neig (SAMPLES *sample, int pos, int x, int y, int w, int h)
{
	int dist, count;
	
	dist = count = 0;
	if (x-1 >= 0)
	{
		dist += abs (sample[y*w + x-1].pos - pos);
		count++;
	}
	if (x+1 < w)
	{
		dist += abs (sample[y*w + x+1].pos - pos);
		count++;
	}
	if (y-1 >= 0)
	{
		dist += abs (sample[(y-1)*w + x].pos - pos);
		count++;
	}
	if (y+1 < h)
	{
		dist += abs (sample[(y+1)*w + x].pos - pos);
		count++;
	}

	if ((x-1 >= 0) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x-1].pos - pos);
		count++;
	}
	if ((x-1 >= 0) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x-1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y-1 >= 0))
	{
		dist += abs (sample[(y-1)*w + x+1].pos - pos);
		count++;
	}
	if ((x+1 < w) && (y+1 < h))
	{
		dist += abs (sample[(y+1)*w + x+1].pos - pos);
		count++;
	}
	return ((int) ((double) dist / (double) count + 0.5));
}

// ----------------------------------------------------------------------------
// calculate_disparity - 
//
// Entrada: 
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void calculate_disparity (NEURON_LAYER *nlDisparityMap, int wxd)
{
	SAMPLES temp;
	int x, y, w, h, i, sample, best_sample, cur_dist, dist;
	
	w = nlDisparityMap->dimentions.x;
	h = nlDisparityMap->dimentions.y;
	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			cur_dist = 100000;
			for (sample = best_sample = 0; sample < NUM_SAMPLES; sample++)
			{
				dist = distance_neig (g_DispData->samples[0], g_DispData->samples[sample][y * w + x].pos, x, y, w, h);
				if (dist < cur_dist)
				{
					cur_dist = dist;
					best_sample = sample;
				}
			}
			temp = g_DispData->samples[0][y * w + x];
			g_DispData->samples[0][y * w + x] = g_DispData->samples[best_sample][y * w + x];
			g_DispData->samples[best_sample][y * w + x] = temp;
		}
	}

	for (i = 0; i < w*h; i++)
		nlDisparityMap->neuron_vector[i].output.fval = (float) (g_DispData->samples[0][i].pos - wxd);
}



// ----------------------------------------------------------------------
// InputKeyboardHandler - input keyboard handler
//
// Inputs: key - the keyboard button
//
// Outputs: none
// ----------------------------------------------------------------------


extern int g_nViewerViewMode;
extern int g_nViewerWindowId;

void f_keyboard (char *key_value)
{
	int i, nStepsNumber;

	switch (key_value[0])
	{
		// Sets the vergence
		case 'V':
		case 'v':
			SetVergence ();
			break;
		// Applyes a gaussian low pass filter
		case 'F':
		case 'f':
			LowPass ();
			break;
		// Calculates the nearest minimal value
		case 'D':
		case 'd':
			calculate_disparity (&nl_disparity_map, image_left.wxd);
			glutPostWindowRedisplay (out_disparity_map.win);
			break;
		// Initializes the viewer module
		case 'A':
                case 'a':
                        ViewerInitialize ();
                        break;
		// Reconstructs by disparity map
		case 'R':
		case 'r':
			ReconstructByDisparityMap ();
			break;
		// Erases the viewer map
                case 'E':
                case 'e':
                        ViewerEraseMap ();
                        break;
		// Generates the occupancy map
                case 'O':
                case 'o':
                        ViewerGenerateOccupancyMap ();
                        break;
                // Alternates to the external view mode
                case 'X':
                case 'x':
                        g_nViewerViewMode = 0; // EXTERNAL_VIEW;
 			glutPostWindowRedisplay (g_nViewerWindowId);
                       break;
		// Initializes the simulator module
                case 'K':
                case 'k':
                        SimulatorInitialize ();
                        SimulatorLinkLeftImage (image_left.image, image_left.ww, image_left.wh);
                        SimulatorLinkRightImage (image_right.image, image_right.ww, image_right.wh);
                        break;
#ifndef WINDOWS
		// Initializes the infra-red transmitter module
		case 'I':
		case 'i':
                        IRInitialize ();
                        break;
#endif
		// Initializes the fuzzy inference machine
                case 'B':
                case 'b':
                        FuzzyInitialize ();
                        break;
		// Forward the fuzzy machine in simulation mode
                case 'W':
                case 'w':
			ViewerGetFuzzyInputs (g_p_dblFuzzyInputs, FUZZY_INPUTS_NUMBER, -VIRTUAL_FOV/2.0f, VIRTUAL_FOV/2.0f);
                        FuzzyMachineForward (g_p_dblFuzzyInputs, g_p_dblFuzzyOutputs);
                        ViewerTransformMap (-g_p_dblFuzzyOutputs[0], -g_p_dblFuzzyOutputs[1]);
                        SimulatorRobotTurn (g_p_dblFuzzyOutputs[0]);
                        SimulatorRobotMove (g_p_dblFuzzyOutputs[1]);
                        break;
		// Forward the fuzzy machine in real environment mode
		case 'G':
		case 'g':
			ViewerGetFuzzyInputs (g_p_dblFuzzyInputs, FUZZY_INPUTS_NUMBER, -VIRTUAL_FOV/2.0f, VIRTUAL_FOV/2.0f);
                        FuzzyMachineForward (g_p_dblFuzzyInputs, g_p_dblFuzzyOutputs);
                        ViewerTransformMap (-g_p_dblFuzzyOutputs[0], -g_p_dblFuzzyOutputs[1]);

			if (g_p_dblFuzzyOutputs[0] > 0.0)
			{
				nStepsNumber = (int) (g_p_dblFuzzyOutputs[0] / TURN_LEFT_STEP + 0.5);
#ifndef WINDOWS
				SendIR(nSinalEsquerda, nStepsNumber);
#endif
				printf ("Turn Left (%d steps of %.2f degrees)\n", nStepsNumber, TURN_LEFT_STEP);
			}
			else
			{
				nStepsNumber = -(int) (g_p_dblFuzzyOutputs[0] / TURN_RIGHT_STEP - 0.5);
#ifndef WINDOWS
				SendIR(nSinalDireita, nStepsNumber);
#endif
				printf ("Turn Right (%d steps of %.2f degrees)\n", nStepsNumber, TURN_RIGHT_STEP);
			}

			// Delay
			for (i = 0; i < 10000000; i++)
				;

			nStepsNumber = (int) (g_p_dblFuzzyOutputs[1] / MOVE_FORWARD_STEP + 0.5);
#ifndef WINDOWS
			SendIR(nSinalFrente, nStepsNumber);
#endif
			printf ("Forward (%d steps of %.2f centimeters)\n", nStepsNumber, MOVE_FORWARD_STEP);
			break;
		// Memorizes the target features
		case 'C':	
		case 'c':
			copy_neuron_outputs (&nl_features, &nl_features1);
			break;
		// Saccade until reach the target
		case 'S':
		case 's':
			saccade (&image_right);
			break;	
		// Jump to the target 
		case 'J':
		case 'j':
			jump (&image_right);
			break;
		// Quits application 
                case 'Q':
                case 'q':
                        FuzzyQuit ();
			ViewerQuit ();
			SimulatorQuit ();
#ifndef WINDOWS
			IRQuit ();
#endif
                        exit (0);
	}
        
	glutPostRedisplay ();

	return;
}



// ----------------------------------------------------------------------
// Verge -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT Verge (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
		
	SetVergence ();
	
	return (output);
}



// ----------------------------------------------------------------------
// SetVergenceAtPoint -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT SetVergenceAtPoint (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int xr, yr;
	
	xr = pParamList->next->param.ival;
	yr = pParamList->next->next->param.ival;
	
	image_right.wxd = xr;
	image_right.wyd = yr;
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);
	
	SetVergence ();
	
	return (output);
}




// ----------------------------------------------------------------------
// Reconstruct3DWorld -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT Reconstruct3DWorld (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i;
	
	for (i = 0; i < 10; i++)
		calculate_disparity (&nl_disparity_map, image_left.wxd);
	
	LowPass ();
	ReconstructByDisparityMap ();

	return (output);
}



// ----------------------------------------------------------------------
// GetLandmark -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT GetLandmark (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int *xl, *yl, u, v, w, h, xi, yi, xi_max = 0, yi_max = 0, wi, hi, x_center, y_center;
	float fltAlpha, fltAlphaMin, fltAlphaMax, fltBeta, fltBetaMin, fltBetaMax, fltLogFactor, fltCurrentValue, fltMaxValue = FLT_MIN;

	xl = (int *) pParamList->next->param.pval;
	yl = (int *) pParamList->next->next->param.pval;
	fltAlphaMin = pParamList->next->next->next->param.fval;
	fltAlphaMax = pParamList->next->next->next->next->param.fval;
	fltBetaMin = pParamList->next->next->next->next->next->param.fval;
	fltBetaMax = pParamList->next->next->next->next->next->next->param.fval;
	fltLogFactor = pParamList->next->next->next->next->next->next->next->param.fval;

	w = nl_confidence_map.dimentions.x;
	h = nl_confidence_map.dimentions.y;
	
	wi = image_right.neuron_layer->dimentions.x;
	hi = image_right.neuron_layer->dimentions.y;
	
	if (TYPE_MOVING_FRAME == STOP)
	{
		x_center = image_right.wxd - image_right.wx;
		y_center = image_right.wyd - image_right.wy;
	}
	else
	{
		x_center = wi/2;
		y_center = hi/2;
	}
	
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, (double) h / (double) (h - 1), fltLogFactor);
			
			fltAlpha = ROBOT_FOVY / 2.0f - ROBOT_FOVY / (float) wi * (float) xi;
			fltBeta = (ROBOT_FOVY / (float) hi * (float) yi - ROBOT_FOVY / 2.0f) * (float) hi / (float) wi;
			
			if ((fltAlpha < fltAlphaMin) || (fltAlpha > fltAlphaMax) || (fltBeta < fltBetaMin) || (fltBeta > fltBetaMax))
				continue;
				
			fltCurrentValue = nl_confidence_map.neuron_vector[v * w + u].output.fval;
			
			if (fltCurrentValue > fltMaxValue)
			{
				fltMaxValue = fltCurrentValue;
				xi_max = xi;
				yi_max = yi;
			}
		}
	}
	
	*xl = xi_max;
	*yl = yi_max;
	
	printf ("Landmark selected: (%d, %d).\n", *xl, *yl);
	
	return (output);
}



// ----------------------------------------------------------------------
// MemorizeLandmark - memorize the landmark features
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT MemorizeLandmark (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int xr, yr, xi, yi;

	// Get the landmark position
	xr = pParamList->next->param.ival;
	yr = pParamList->next->next->param.ival;
	
	// Save the current position
	xi = image_right.wxd;
	yi = image_right.wyd;

	// Aime the right input to the landmark
	image_right.wxd = xr;
	image_right.wyd = yr;
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);
	
	// Save the landmark features
	copy_neuron_outputs (&nl_features, &nl_features1);

	// Return to the least position
	image_right.wxd = xi;
	image_right.wyd = yi;
	move_input_window (image_right.name, image_right.wxd, image_right.wyd);

	printf ("Landmark memorized.\n");
	
	return (output);
}



// ----------------------------------------------------------------------
// SearchLandmark - search for the memorized landmark
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT SearchLandmark (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int *xt, *yt;

	xt = (int *) pParamList->next->param.pval;
	yt = (int *) pParamList->next->next->param.pval;

	// Saccade until reach the target
	saccade (&image_right);

	// Pass by reference the target position
	*xt = image_right.wxd;
	*yt = image_right.wyd;
		
	printf ("Landmark founded: (%d, %d).\n", *xt, *yt);

	return (output);
}



// ----------------------------------------------------------------------
// GetRotation - infer the robot rotation
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT GetRotation (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int x, xOld, w;
	float *p_fltAngle;
	
	p_fltAngle = (float *) pParamList->next->param.pval;
	x = pParamList->next->next->param.ival;
	xOld = pParamList->next->next->next->param.ival;
	
	w = image_right.neuron_layer->dimentions.x;
	
	*p_fltAngle = ROBOT_FOVY * (float) (x - xOld) / (float) w;
	
	printf ("Inferred rotational angle: %.2f [deg].\n", *p_fltAngle);
	
	return (output);
}



// ----------------------------------------------------------------------
// GetFuzzyInputsFromMap -
//
// Inputs: pParamList - parameter list
//
// Outputs: the fuzzy inputs vector
// ----------------------------------------------------------------------


NEURON_OUTPUT GetFuzzyInputsFromMap (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float *p_fltLeft, *p_fltCenter, *p_fltRight;
	
	p_fltLeft   = (float *) pParamList->next->param.pval;
	p_fltCenter = (float *) pParamList->next->next->param.pval;
	p_fltRight  = (float *) pParamList->next->next->next->param.pval;
	
	ViewerGetFuzzyInputs (g_p_dblFuzzyInputs, FUZZY_INPUTS_NUMBER, -VIRTUAL_FOV/2.0f, VIRTUAL_FOV/2.0f);

	*p_fltLeft   = (float) g_p_dblFuzzyInputs[0];
	*p_fltCenter = (float) g_p_dblFuzzyInputs[1];
	*p_fltRight  = (float) g_p_dblFuzzyInputs[2];
	
	return (output);
}



// ----------------------------------------------------------------------
// ForwardFuzzyMachine -
//
// Inputs: pParamList - parameter list
//
// Outputs: the fuzzy outputs vector
// ----------------------------------------------------------------------

NEURON_OUTPUT ForwardFuzzyMachine (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float *p_fltAngle, *p_fltStep, fltLeft, fltCenter, fltRight;

	p_fltAngle = (float *) pParamList->next->param.pval;
	p_fltStep  = (float *) pParamList->next->next->param.pval;
	fltLeft   = pParamList->next->next->next->param.fval;
	fltCenter = pParamList->next->next->next->next->param.fval;
	fltRight  = pParamList->next->next->next->next->next->param.fval;

	g_p_dblFuzzyInputs[0] = (double) fltLeft;
	g_p_dblFuzzyInputs[1] = (double) fltCenter;
	g_p_dblFuzzyInputs[2] = (double) fltRight;
	
        FuzzyMachineForward (g_p_dblFuzzyInputs, g_p_dblFuzzyOutputs);

	*p_fltAngle = (float) g_p_dblFuzzyOutputs[0];
	*p_fltStep  = (float) g_p_dblFuzzyOutputs[1];
	
	return (output);
}



// ----------------------------------------------------------------------
// TransformMap -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT TransformMap (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float fltAngle, fltStep;

	fltAngle = pParamList->next->param.fval;
	fltStep  = pParamList->next->next->param.fval;

        ViewerTransformMap (fltAngle, fltStep);
	
	printf ("Map transformation: %.2f [deg], %.2f [cm].\n", fltAngle, fltStep);
	
	return (output);
}



// ----------------------------------------------------------------------
// TurnRobot -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT TurnRobot (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float fltAngle;
	int nStepsNumber;
	
	fltAngle = pParamList->next->param.fval;
		
	// Turns the robotic platform if the InfraRed module is initialized
	if (fltAngle > .0f)
	{
		nStepsNumber = (int) (fltAngle / TURN_LEFT_STEP + 0.5);
#ifndef WINDOWS
		IRSendSignal (nSinalEsquerda, nStepsNumber);
#endif
		printf ("Turn Left: %.2f [deg].\n", fltAngle);
	}
	else
	{
		nStepsNumber = -(int) (fltAngle / TURN_RIGHT_STEP - 0.5);
#ifndef WINDOWS
		IRSendSignal (nSinalDireita, nStepsNumber);
#endif
		printf ("Turn Right: %.2f [deg].\n", fltAngle);
	}
	
	// Turns the virtual robotic platform if the Simulator module is initialized
	SimulatorRobotTurn (fltAngle);
	
	all_inputs_update ();
	all_filters_update ();
	all_outputs_update ();

	return (output);
}



// ----------------------------------------------------------------------
// ForwardRobot -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT ForwardRobot (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float fltStep;
	int nStepsNumber;

	fltStep = pParamList->next->param.fval;
				
	// Moves forward the robotic platform if the InfraRed module is initialized
	nStepsNumber = (int) (fltStep / MOVE_FORWARD_STEP + 0.5);
#ifndef WINDOWS
	IRSendSignal (nSinalFrente, nStepsNumber);
#endif
	printf ("Forward: %.2f [cm].\n", fltStep);
	
	// Moves forward the virtual robotic platform if the Simulator module is initialized
        SimulatorRobotMove (fltStep);
	
	all_inputs_update ();
	all_filters_update ();
	all_outputs_update ();
	
	return (output);
}



// ----------------------------------------------------------------------
// UpdateEvidenceMap -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT UpdateEvidenceMap (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float fltMinDistance;
	
	fltMinDistance = pParamList->next->param.fval;
	
        ViewerUpdateEvidenceMap (fltMinDistance);
	
	return (output);
}



// ----------------------------------------------------------------------
// MakePath -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT MakePath (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

        output.ival = ViewerMakePath ();
	
	return (output);
}



// ----------------------------------------------------------------------
// GetNextMovement -
//
// Inputs: pParamList - parameter list
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

NEURON_OUTPUT GetNextMovement (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float *p_fltAngle, *p_fltStep;

	p_fltAngle = (float *) pParamList->next->param.pval;
	p_fltStep  = (float *) pParamList->next->next->param.pval;
	
	output.ival = ViewerGetPath (p_fltStep, p_fltAngle);
    	
	return (output);
}

// ----------------------------------------------------------------------
// MakePath2 -
//
// Inputs: pParamList - parameter list
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

NEURON_OUTPUT MakePath2 (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	float fltGoalX, fltGoalY;

	fltGoalX = pParamList->next->param.fval;
	fltGoalY  = pParamList->next->next->param.fval;
	output.ival = ViewerMakePath2Point (fltGoalX, fltGoalY);
	    	
	return (output);
}



// ----------------------------------------------------------------------
// Delay -
//
// Inputs: pParamList - parameter list
//
// Outputs: none
// ----------------------------------------------------------------------

NEURON_OUTPUT Delay (PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	int i, nCycles;
	
	nCycles = pParamList->next->param.ival;

	// Delay
	for (i = 0; i < nCycles; i++)
		;

	return (output);
}
