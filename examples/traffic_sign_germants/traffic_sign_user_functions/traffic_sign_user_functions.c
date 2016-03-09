#include "traffic_sign_user_functions.h"

#define N_DEFINED_SIGNS 43

//############### Global Variables ###############
int g_signUniqueID = -1; //id da classe da placa
int g_signID = -1; //id da placa
int g_sign_frameID = 0; //frame de uma placa (uma placa aparece varias vezes)

int g_size_box;
int g_img_w, g_img_h, g_roi_x1, g_roi_y1, g_roi_x2, g_roi_y2;

int g_size_training_set;
int g_size_testing_set;

int g_fptr;

int g_nStatus;
int g_nTries = 1;
int g_nTry = 1;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;

char g_strSignsFileName[256];
FILE *g_signs_data_file = NULL;

//############### User Functions ###############
//***********************************************************
//* Function: ReadFrameInput
//* Description:
//***********************************************************
//PNM P6
int ReadFrameInput2 (INPUT_DESC *input, char *strFileName)
{
    FILE *image_file = (FILE *) NULL;
    int i,j;
    int r,g,b;
    char character = 0;
    
    if ((image_file = fopen (strFileName, "rb")) == (FILE *) NULL)
    {
        Erro ("Cannot open input file (ReadFrameInput2): ", strFileName, "");
        return -1;
    }
   // printf("%s\n",strFileName);
    
    /*Discard magic number in the begining of the image file. */
    while ((character = fgetc (image_file)) != '\n');
    
    /* Discard comments */
    if ((character = fgetc (image_file)) == '#')
    {
        while (character != '\n')
            character = fgetc (image_file);
    }
    else
        ungetc (character, image_file);
    
    /* Discard image dimensions. */
    while ((character = fgetc (image_file)) != '\n');
    
    /* Discard max color value. */
    while ((character = fgetc (image_file)) != '\n');

    /* Clear image */
    for (i=0; i<=input->tfw * input->tfh * 3; i++)
    {
        input->image[i] = 0;
    }
    
    /////////////////////////////////////// 

    /* Fit ROI to a square */
    if (g_roi_x1 > g_roi_y1)
        g_roi_y1 = g_roi_x1;
    else
        g_roi_x1 = g_roi_y1;

    if (g_roi_x2 > g_roi_y2)
        g_roi_y2 = g_roi_x2;
    else
        g_roi_x2 = g_roi_y2;
    
    /* Get ROI box size. */
    if ((g_roi_x2-g_roi_x1) > (g_roi_y2-g_roi_y1))
        g_size_box = g_roi_y2 - g_roi_y1;
    else
        g_size_box = g_roi_x2 - g_roi_x1;
            
    if (g_roi_x1 + g_size_box > g_img_w)
        g_size_box = g_img_w - g_roi_x1;
    if (g_roi_y1 + g_size_box > g_img_h)
        g_size_box = g_img_h - g_roi_y1;
    
    if (g_size_box%2 != 0)
        g_size_box = g_size_box - 1;
    
    fseek(image_file, g_roi_y1*g_img_w*3 + g_roi_x1*3, SEEK_CUR);
    
    for (j = input->vph/2 + g_size_box/2; j > input->vph/2 - g_size_box/2; j--) 
    {
        for (i = input->vpw/2 - g_size_box/2; i < input->vpw/2 + g_size_box/2 ; i++) 
        {
            r = (int) fgetc (image_file);
            g = (int) fgetc (image_file);
            b = (int) fgetc (image_file);
            
            input->image[3 * (j * input->tfw + i) + 0] = (GLubyte) r;
            input->image[3 * (j * input->tfw + i) + 1] = (GLubyte) g;
            input->image[3 * (j * input->tfw + i) + 2] = (GLubyte) b;
        }
        fseek(image_file, ((g_img_w-g_roi_x1-g_size_box)*3 + g_roi_x1*3), SEEK_CUR);

    }
    update_input_neurons (input);
    fclose(image_file);
    return (0);
}

//***********************************************************
//* Function: MakeInputImage
//* Description:
//***********************************************************
void MakeInputImage (INPUT_DESC *input, int w, int h)
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
//* Function: get_first_sign_id
//* Description:
//***********************************************************
int get_first_sign_id ()
{
    fscanf(g_signs_data_file, "%05d_%05d_%05d %d %d %d %d %d %d\n", &g_signUniqueID, &g_signID, &g_sign_frameID, &g_img_w, &g_img_h, &g_roi_x1, &g_roi_y1, &g_roi_x2, &g_roi_y2);
    return 0;
}

//***********************************************************
//* Function: get_next_sign_id
//* Description:
//***********************************************************
int get_next_sign_id (int direction)
{
    int file_size, i;
    char curr_filename[128];
    char filename[128];
    char previous_filename[128];
    int width, height, roi_x1, roi_y1, roi_x2, roi_y2;
    int prev_width, prev_height, prev_roi_x1, prev_roi_y1, prev_roi_x2, prev_roi_y2;
    fpos_t position;

    sprintf(curr_filename, "%05d_%05d_%05d.ppm", g_signUniqueID, g_signID, g_sign_frameID);

    if (g_nStatus == TRAINING_PHASE)
    {
        file_size = g_size_training_set;
    }
        else if (g_nStatus == RECALL_PHASE)
        {
            file_size = g_size_testing_set;
        }
            else
            {
                printf ("Error: invalid net status (get_next_sign_id).\n");
                return (-1);
            }
    rewind(g_signs_data_file);
    fgetpos (g_signs_data_file, &position);

    if (direction == DIRECTION_FORWARD)
    {
        for (i=0; i<=file_size; i++)
        {
            fscanf(g_signs_data_file, "%s %*d %*d %*d %*d %*d %*d\n", filename);
            if (strcmp(filename, curr_filename) == 0)
            {
                /* return next filename */
                if (i==file_size-1) //after the last line comes the first line
                {
                    fsetpos (g_signs_data_file, &position);
                }
                /* read next line */
                fscanf(g_signs_data_file, "%s %d %d %d %d %d %d\n",filename, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);
                g_img_w = width;
                g_img_h = height;
                g_roi_x1 = roi_x1;
                g_roi_y1 = roi_y1;
                g_roi_x2 = roi_x2;
                g_roi_y2 = roi_y2;
                sscanf(filename, "%05d_%05d_%05d\n", &g_signUniqueID, &g_signID, &g_sign_frameID);
                
                rewind(g_signs_data_file);
                return (0);
            }
        }
        printf ("Error: cannot find next file from current file '%s' (get_next_sign_id).\n", curr_filename);
        return (-1);
    }
    else //direction == DIRECTION_REWIND
    {
        for (i=0; i<=file_size; i++)
        {
            if (i!=0)
            {
                strcpy(previous_filename, filename);
                prev_width = width;
                prev_height = height;
                prev_roi_x1 = roi_x1;
                prev_roi_y1 = roi_y1;
                prev_roi_x2 = roi_x2;
                prev_roi_y2 = roi_y2;
            }
            fscanf(g_signs_data_file, "%s %d %d %d %d %d %d\n",filename, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);

            if (strcmp(filename, curr_filename) == 0)
            {
                //return previous filename
                if (i==0)
                {
                    while(i<=file_size)
                    {
                        fscanf(g_signs_data_file, "%s %d %d %d %d %d %d\n",filename, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);
                        i++;
                    }
                    strcpy(previous_filename, filename);
                    prev_width = width;
                    prev_height = height;
                    prev_roi_x1 = roi_x1;
                    prev_roi_y1 = roi_y1;
                    prev_roi_x2 = roi_x2;
                    prev_roi_y2 = roi_y2;
                }
                g_img_w = prev_width;
                g_img_h = prev_height;
                g_roi_x1 = prev_roi_x1;
                g_roi_y1 = prev_roi_y1;
                g_roi_x2 = prev_roi_x2;
                g_roi_y2 = prev_roi_y2;
                sscanf(previous_filename, "%05d_%05d_%05d\n", &g_signUniqueID, &g_signID, &g_sign_frameID);
                
                rewind(g_signs_data_file);
                return 0;
            }
        }
        printf ("Error: cannot find previous file from current file '%s' (get_next_sign_id).\n", curr_filename);
        return (-1);
    }
}

//***********************************************************
//* Function: get_file_lines_len
//* Description:
//***********************************************************
int get_file_lines_len (char *file)
{
    int len;
    FILE * linhas = fopen("linhas.deleteme", "r");
    
    char command[128];
    sprintf(command, "wc -l %s > linhas.deleteme;", file);
    system(command);
    fscanf(linhas, "%d %*s\n", &len);
    return len;
}

//***********************************************************
//* Function: init_traffic_sign
//* Description:
//***********************************************************

void init_traffic_sign (INPUT_DESC *input)
{
#ifndef NO_INTERFACE
    int x, y;
#endif
    //char file_name[128];
    
    g_size_training_set = get_file_lines_len(TRAINING_INPUT_FILES);
    g_size_testing_set = get_file_lines_len(TESTING_INPUT_FILES);
    
    g_nStatus = MOVING_PHASE;
    g_nTotalTested = 0;
    g_nCorrect = 0;
    
    MakeInputImage (input, INPUT_WIDTH, INPUT_HEIGHT);

    // Le a primeira imagem
    //if (!GetNextFileName (strFileName, DIRECTION_FORWARD))
    //{
    //    ReadFrameInput (input, strFileName);
    //}
        
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
                //GetNewFrame (input, DIRECTION_REWIND);
            }
            else if (input->wxd >= INPUT_WIDTH)
            {
                //GetNewFrame (input, DIRECTION_FORWARD);
            }
            //update the first output
            filter = get_filter_by_output(out_traffic_sign.neuron_layer);
            filter_update(filter);
            output_update(&out_traffic_sign);

#ifndef NO_INTERFACE            
            glutSetWindow (input->win);
            input_display ();
#endif
        }
    }    
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
                
                if    (confidence > best_confidence)
                {
                    best_confidence = confidence;
                    sign_output = sign;
                }
                g_nTry--;
            }
            if (g_nTry == 0)
            {
                g_nTotalTested++;
                
                if (sign_output    == g_signUniqueID)
                {
                    g_nCorrect++;
                    hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
                    
                    printf("HIT %d %d ",g_signUniqueID, sign_output);
                    printf("%d %d ", g_signID, g_sign_frameID);
                    printf("%3d %3d %3.2f %2.5f\n", g_nTotalTested,g_nCorrect, hit_percent, confidence);
                    
                }
                else
                {
                    hit_percent = (float)(g_nCorrect * 100.0 / g_nTotalTested);
                    printf("MISS %d %d ",g_signUniqueID, sign_output);
                    printf("%d %d ", g_signID, g_sign_frameID);
                    printf("%3d %3d %3.2f %2.5f\n", g_nTotalTested,g_nCorrect, hit_percent, confidence);
                    
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
//* Function: SetNetworkStatus
//* Description:
//***********************************************************
NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *pParamList)
{
    NEURON_OUTPUT output;

    g_nStatus = pParamList->next->param.ival;

    switch (g_nStatus)
    {
        case TRAINING_PHASE:
            if ((g_signs_data_file = fopen (TRAINING_INPUT_FILES, "r")) == NULL)
            {
                printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TRAINING_INPUT_FILES);
                output.ival = -1;
                return (output);
            }
            break;
        case RECALL_PHASE:
            if ((g_signs_data_file = fopen (TESTING_INPUT_FILES, "r")) == NULL)
            {
                printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TESTING_INPUT_FILES);
                output.ival = -1;
                return (output);
            }
            break;
        default:
            printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
            output.ival = -1;
            return (output);
    }
    //g_fptr = g_signs_data_file
    output.ival = 0;
    return (output);
}

int SetNetworkStatus_interface (int net_status)
{
    switch (net_status)
    {
        case TRAINING_PHASE:
            if ((g_signs_data_file = fopen (TRAINING_INPUT_FILES, "r")) == NULL)
            {
                printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TRAINING_INPUT_FILES);
                return (-1);
            }
            break;
        case RECALL_PHASE:
            if ((g_signs_data_file = fopen (TESTING_INPUT_FILES, "r")) == NULL)
            {
                printf ("Error: cannot open file '%s' (get_sign_by_index).\n", TESTING_INPUT_FILES);
                return (-1);
            }
            break;
        default:
            printf ("Error: invalid Net Status '%d' (SetNetworkStatus).\n", g_nStatus);
            return (-1);
    }
    
    g_nStatus = net_status;
    return (0);
}

//***********************************************************
//* Function: get_sign_by_index
//* Description:
//***********************************************************
NEURON_OUTPUT get_sign_by_index (PARAM_LIST *pParamList)
{
    NEURON_OUTPUT output;
    FILTER_DESC *filter;
    
    int line_number;
    int i, width, height, roi_x1, roi_y1, roi_x2, roi_y2;

    char file_name[256];
    char file_path[256];
    
    line_number = pParamList->next->param.ival;
    
    for (i = 1; i <= line_number; i++)
    {
        if (g_nStatus == TRAINING_PHASE)
        {
            fscanf (g_signs_data_file, "%s %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);
            if (i == line_number)
            {
                sscanf (file_name, "%05d_%05d_%05d.ppm", &g_signUniqueID, &g_signID, &g_sign_frameID);
                g_img_w = width;
                g_img_h = height;
                g_roi_x1 = roi_x1;
                g_roi_y1 = roi_y1;
                g_roi_x2 = roi_x2;
                g_roi_y2 = roi_y2;    
                sprintf (file_path, "%s%s", TRAINING_INPUT_PATH,file_name);
            }
        }
        else if (g_nStatus == RECALL_PHASE)
        {
            fscanf (g_signs_data_file, "%s %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);
            if (i == line_number)
            {
                sscanf (file_name, "%05d_%05d_%05d.ppm", &g_signUniqueID, &g_signID, &g_sign_frameID);
                g_img_w = width;
                g_img_h = height;
                g_roi_x1 = roi_x1;
                g_roi_y1 = roi_y1;
                g_roi_x2 = roi_x2;
                g_roi_y2 = roi_y2;
                sprintf (file_path, "%s%s", TESTING_INPUT_PATH,file_name);
            }
        }
        else
        {
            printf ("Error: Invalid net status (get_sign_by_index).\n");
            output.ival = -1;
            return (output);
        }
    }
    
    //fclose (pFile);
    rewind(g_signs_data_file);
    fflush (stdout);
    
    /*read sign image*/
    ReadFrameInput2 (&traffic_sign, file_path);

    check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
    traffic_sign.up2date = 0;
    update_input_neurons (&traffic_sign);
    update_input_image (&traffic_sign);
    
    /*update the first output*/
    filter = get_filter_by_output (out_traffic_sign.neuron_layer);
    filter_update (filter);
    output_update (&out_traffic_sign);
        
    output.ival = 0;
    return (output);
}

NEURON_OUTPUT get_sign_by_index2 (PARAM_LIST *pParamList)
{
    NEURON_OUTPUT output;
    FILTER_DESC *filter;
    
    int line_number;
    int width, height, roi_x1, roi_y1, roi_x2, roi_y2;

    char file_name[256];
    char file_path[256];
    
    line_number = pParamList->next->param.ival;
    
    
    if (g_nStatus == TRAINING_PHASE)
    {
        fscanf (g_signs_data_file, "%s %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);
        
        sscanf (file_name, "%05d_%05d_%05d.ppm", &g_signUniqueID, &g_signID, &g_sign_frameID);
        g_img_w = width;
        g_img_h = height;
        g_roi_x1 = roi_x1;
        g_roi_y1 = roi_y1;
        g_roi_x2 = roi_x2;
        g_roi_y2 = roi_y2;    
        sprintf (file_path, "%s%s", TRAINING_INPUT_PATH,file_name);
        printf("classe %d; placa %d; id img %d\n",g_signUniqueID, g_signID, g_sign_frameID);
    }
    else if (g_nStatus == RECALL_PHASE)
    {
        fscanf (g_signs_data_file, "%s %d %d %d %d %d %d\n",file_name, &width, &height, &roi_x1, &roi_y1, &roi_x2, &roi_y2);

        sscanf (file_name, "%05d_%05d_%05d.ppm", &g_signUniqueID, &g_signID, &g_sign_frameID);
        g_img_w = width;
        g_img_h = height;
        g_roi_x1 = roi_x1;
        g_roi_y1 = roi_y1;
        g_roi_x2 = roi_x2;
        g_roi_y2 = roi_y2;
        sprintf (file_path, "%s%s", TESTING_INPUT_PATH,file_name);

    }
    else
    {
        printf ("Error: Invalid net status (get_sign_by_index).\n");
        output.ival = -1;
        return (output);
    }
    
    
    //fclose (pFile);
    fflush (stdout);
    
    /*read sign image*/
    ReadFrameInput2 (&traffic_sign, file_path);

    check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
    traffic_sign.up2date = 0;
    update_input_neurons (&traffic_sign);
    update_input_image (&traffic_sign);
    
    /*update the first output*/
    filter = get_filter_by_output (out_traffic_sign.neuron_layer);
    filter_update (filter);
    output_update (&out_traffic_sign);
        
    output.ival = 0;
    return (output);
}

//***********************************************************
//* Function: get_current_sign_image
//* Description:
//***********************************************************
int get_current_sign_image ()
{
    FILTER_DESC *filter;
    char curr_filename[128];
    char file_path[256];
    
    sprintf(curr_filename, "%05d_%05d_%05d.ppm", g_signUniqueID, g_signID, g_sign_frameID);
    
    if (g_nStatus == TRAINING_PHASE)
    {
        sprintf (file_path, "%s%s", TRAINING_INPUT_PATH, curr_filename);
    }
    else if (g_nStatus == RECALL_PHASE)
    {
        sprintf (file_path, "%s%s", TESTING_INPUT_PATH, curr_filename);
    }
    else
    {
        printf ("Error: Invalid net status (get_current_sign_image).\n");
            return -1;
    }
    
    /*read sign image*/
    ReadFrameInput2 (&traffic_sign, file_path);
    check_input_bounds (&traffic_sign, traffic_sign.wx + traffic_sign.ww/2, traffic_sign.wy + traffic_sign.wh/2);
    traffic_sign.up2date = 0;
    update_input_neurons (&traffic_sign);
    update_input_image (&traffic_sign);
    //input_update (&traffic_sign);
    /*update the outputs*/
    filter = get_filter_by_output (out_traffic_sign.neuron_layer);
    filter_update (filter);
    output_update (&out_traffic_sign);
    
    filter = get_filter_by_output (out_traffic_sign_f.neuron_layer);
    filter_update (filter);
    output_update (&out_traffic_sign_f);
        
    return 0;
}

//***********************************************************
//* Function: f_keyboard
//* Description: Called whenever a key is pressed
//***********************************************************
void f_keyboard (char *key_value)
{
    char key;
//    FILTER_DESC *filter;
    
    key = key_value[0];
    switch (key)
    {
        case 't': //set training status
            SetNetworkStatus_interface(TRAINING_PHASE);
            get_first_sign_id();
            break;
        
        case 'r': //set recall status
            SetNetworkStatus_interface(RECALL_PHASE);
            get_first_sign_id();
            break;
        
        case 'N':
        
        case 'n': // next sign
            if ((g_sign_frameID == -1) && (g_signID == -1))
                get_first_sign_id();
            else
                get_next_sign_id(DIRECTION_FORWARD);
            
            get_current_sign_image();
            break;
        
        case 'P':
        
        case 'p': // previous sign
            if ((g_sign_frameID == -1) && (g_signID == -1))
                get_first_sign_id();
            else
                get_next_sign_id(DIRECTION_REWIND);
            
            get_current_sign_image();
            break;
            
        default:
            break;
    }
}


NEURON_OUTPUT wait_key (PARAM_LIST *pParamList)
{
    NEURON_OUTPUT output;
    getchar();
    return output;
}

