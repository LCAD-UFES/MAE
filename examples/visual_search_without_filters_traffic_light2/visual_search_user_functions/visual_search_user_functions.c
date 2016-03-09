#include <locale.h>
#include <sys/stat.h>
#include <errno.h>
#include "visual_search_user_functions.h"

/*
 ********************************************************
 * Function: draw_output  			       *
 * Description: 				 	       *
 * Inputs: 					       *
 * Output: none  				       *
 ********************************************************
 */

void
draw_output(char *output_name, char *input_name) {
    return;
}

static void
move_input(INPUT_DESC *input) {
    if (input->wxd < 0)
        GetNewImage(input, DIRECTION_REWIND);
    else if (input->wxd >= IMAGE_WIDTH)
        GetNewImage(input, DIRECTION_FORWARD);
    else {
        translation_filter_deltaX = (float) input->wxd;
        translation_filter_deltaY = (float) input->wyd;

        check_input_bounds(input, input->wxd, input->wxd);
#ifndef	NO_INTERFACE
        glutSetWindow(input->win);
        input_display();
#endif
        filter_update(get_filter_by_name("in_pattern_translated_filter"));
        filter_update(get_filter_by_name("in_pattern_filtered_translated_filter"));
        filter_update(get_filter_by_name("nl_v1_pattern_filter"));
        all_dendrites_update();
        all_neurons_update();
        filter_update(get_filter_by_name("nl_v1_activation_map_f_filter"));
        all_outputs_update();
    }
}

/*
 ********************************************************
 * Function: make_input_image_visual_search	       *
 * Description: 		      			       *
 * Inputs: input layer, width, height		       *
 * Output: none  				       *
 ********************************************************
 */

void
make_input_image_visual_search(INPUT_DESC *input, int w, int h) {
    char message[256];

    input->tfw = nearest_power_of_2(w);
    input->tfh = nearest_power_of_2(h);

    input->ww = w;
    input->wh = h;

    switch (TYPE_SHOW) {
        case SHOW_FRAME:
            input->vpw = input->neuron_layer->dimentions.x;
            input->vph = input->neuron_layer->dimentions.y;
            break;
        case SHOW_WINDOW:
            input->vph = h;
            input->vpw = w;
            break;
        default:
            sprintf(message, "%d. It can be SHOW_FRAME or SHOW_WINDOW.", TYPE_SHOW);
            Erro("Invalid Type Show ", message, " Error in update_input_image.");
            return;
    }

    input->vpxo = 0;
    input->vpyo = h - input->vph;

    if (input->image == NULL)
        input->image = (GLubyte *) alloc_mem(input->tfw * input->tfh * 3 * sizeof (GLubyte));
}

/*
 ********************************************************
 * Function: init_visual_search			       *
 * Description: initialize variables, structures and    *
 *	       program procedures		       *
 * Inputs: input layer				       *
 * Output: none  				       *
 ********************************************************
 */

void
init_visual_search(INPUT_DESC *input) {
#ifndef	NO_INTERFACE
    int x, y;
#endif

    make_input_image_visual_search(input, IMAGE_WIDTH, IMAGE_HEIGHT);

#ifndef	NO_INTERFACE
    glutInitWindowSize(input->ww / 2, input->wh / 2);
    if (read_window_position(input->name, &x, &y))
        glutInitWindowPosition(x, y);
    else
        glutInitWindowPosition(-1, -1);
    input->win = glutCreateWindow(input->name);

    glGenTextures(1, (GLuint *) (&(input->tex)));
    input_init(input);
    glutReshapeFunc(input_reshape);
    glutDisplayFunc(input_display);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(input_passive_motion);
    glutMouseFunc(input_mouse);
#endif
}

/*
 ********************************************************
 * Function: input_generator			       *
 * Description: pattern generator		       *
 * Inputs: input layer, status			       *
 * Output: none  				       *
 ********************************************************
 */

void
input_generator(INPUT_DESC *input, int status) {
    if (input->win == 0) {
        init_visual_search(input);
#ifdef NO_INTERFACE
        input->win = 1;
#endif
    } else if (status == MOVE) {
        move_input(input);
    }
}

/*
 ********************************************************
 * Function: input_controler			       *
 * Description: input events handler		       *
 * Inputs: input layer, input status		       *
 * Output: none  				       *
 ********************************************************
 */

void
input_controler(INPUT_DESC *input, int status) {
    char strCommand[128];

    if ((move_active == 1) && (input->mouse_button == GLUT_LEFT_BUTTON) && (input->mouse_state == GLUT_DOWN)) {
        // Translate the input image & Move the input center cursor
        sprintf(strCommand, "move %s to %d, %d;", input->name, input->wxd, input->wyd);
        interpreter(strCommand);
    }
    input->mouse_button = -1;

    return;
}

/*
 ********************************************************
 * Function: output_handler_max_value_position	       *
 * Description:  				       *
 * Inputs:					       *
 * Output:					       *
 ********************************************************
 */

void
output_handler_max_value_position(OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state) {
    char *nl_target_coordinates_name = NULL;
    int u, v, u_max, v_max, w, h, xi, yi; //, step;
    float current_value, max_value = -FLT_MAX, log_factor;
    NEURON_LAYER *nl_target_coordinates = NULL;
    int num_max_value = 0;
    NEURON *unfiltered_activation_map;

    // Gets the output handler parameters
    nl_target_coordinates_name = output->output_handler_params->next->param.sval;
    log_factor = output->output_handler_params->next->next->param.fval;

    // Gets the target coordinates neuron layer
    nl_target_coordinates = get_neuron_layer_by_name(nl_target_coordinates_name);

    // Gets the Neuron Layer Dimentions
    w = output->neuron_layer->dimentions.x;
    h = output->neuron_layer->dimentions.y;
    unfiltered_activation_map = nl_v1_activation_map.neuron_vector;
    g_confidence = 0.0;
    // Finds the max value position

    for (v = 0, u_max = v_max = 0; v < h; v++) {
        for (u = 0; u < w; u++) {
#ifdef	CUDA_COMPILED
            current_value = output->neuron_layer->host_neuron_vector[v * w + u].output.fval;
#else
            current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
#endif
            if (current_value > max_value) {
                max_value = current_value;
                u_max = u;
                v_max = v;
                num_max_value = 1;
            } else if (current_value == max_value) {
                u_max += u;
                v_max += v;
                num_max_value += 1;
            }

            if ((unfiltered_activation_map[v * w + u].output.fval > 0.0) &&
                    (u > (int) ((float) (w - 1) / 2.0) - g_halph_band_width) &&
                    (u < (int) ((float) (w - 1) / 2.0) + g_halph_band_width)) {
                g_confidence += 1.0;
            }
        }
    }
    u_max /= num_max_value;
    v_max /= num_max_value;

    g_confidence /= g_halph_band_width * 2.0 * (double) h;

    // Saves the max value
    global_max_value = max_value;

    // Map the max value coordinates to image
    map_v1_to_image(&xi, &yi, IMAGE_WIDTH_RESIZED, IMAGE_HEIGHT_RESIZED, u_max, v_max, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

    // Saves the max value position
#ifdef	CUDA_COMPILED
    nl_target_coordinates->host_neuron_vector[0].output.fval = (float) xi;
    nl_target_coordinates->host_neuron_vector[1].output.fval = (float) yi;
#else
    nl_target_coordinates->neuron_vector[0].output.fval = (float) xi;
    nl_target_coordinates->neuron_vector[1].output.fval = (float) yi;
#endif
}

int
is_stop(int ClassID) {
    int i;

    for (i = 0; i < red_stop_size; i++)
        if (ClassID == red_stop[i])
            return (1);

    return (0);
}

int
is_caution(int ClassID) {
    int i;

    for (i = 0; i < yellow_caution_size; i++)
        if (ClassID == yellow_caution[i])
            return (1);

    return (0);
}

int
is_go(int ClassID) {
    int i;

    for (i = 0; i < green_go_size; i++)
        if (ClassID == green_go[i])
            return (1);

    return (0);
}

int
traffic_light_within_criterion(IMAGE_INFO *image_info) {
    int it_is = 0;

    switch (g_CurrentCriterion) {
        case NONE:
            it_is = 1;
            break;
        case RED_STOP:
            if (is_stop(image_info->ClassID))
                it_is = 1;
            else
                it_is = 0;
            break;
        case YELLOW_CAUTION:
            if (is_caution(image_info->ClassID))
                it_is = 1;
            else
                it_is = 0;
            break;
        case GREEN_GO:
            if (is_go(image_info->ClassID))
                it_is = 1;
            else
                it_is = 0;
            break;
        case FOUND:
            if (image_info->found && (image_info->best_confidence > 0.75) && ((image_info->ClassID <= 2) || (image_info->ClassID > 16)))
                it_is = 1;
            else
                it_is = 0;
            break;

    }

    return (it_is);
}

int
num_images_with_traffic_lights_according_to_criterion(void) {
    int i;
    int num_samples;
    int last_image_with_traffic_light_within_criterion;

    if (g_nStatus == TRAINING_PHASE) {
        i = num_samples = 0;
        while (i < g_num_image_info) {
            if (traffic_light_within_criterion(&(g_image_info[i]))) {
                if (g_CurrentCriterion != FOUND) {
                    last_image_with_traffic_light_within_criterion = i;
                    while ((i < g_num_image_info) && (g_image_info[i].ImgNo == g_image_info[last_image_with_traffic_light_within_criterion].ImgNo))
                        i++;
                    num_samples++;
                } else {
                    i++;
                    num_samples++;
                }
            } else
                i++;
        }
    } else // RECALL_PHASE
        num_samples = g_num_image_info;

    return (num_samples);
}

NEURON_OUTPUT
SetCriterion(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    g_CurrentCriterion = pParamList->next->param.ival;
    g_nNumImagesWithTrafficLightsAccordingToCriterion = num_images_with_traffic_lights_according_to_criterion();

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
GetNumImages(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    output.ival = g_nNumImagesWithTrafficLightsAccordingToCriterion;
    fprintf(stderr, "Number of images within current criterion = %d\n", g_nNumImagesWithTrafficLightsAccordingToCriterion);

    return (output);
}

void
get_traffic_light_file_name(char *strFileName) {
    int sample_num = -1;
    int last_image_with_traffic_light_within_criterion;
    int i;

    if (g_nStatus == TRAINING_PHASE) {
        g_nCurrentImageInfo = 0;
        while (g_nCurrentImageInfo < g_num_image_info) {
            if (traffic_light_within_criterion(&(g_image_info[g_nCurrentImageInfo]))) {
                if (g_CurrentCriterion != FOUND) {
                    i = last_image_with_traffic_light_within_criterion = g_nCurrentImageInfo;
                    while ((i < g_num_image_info) && (g_image_info[i].ImgNo == g_image_info[last_image_with_traffic_light_within_criterion].ImgNo))
                        i++;

                    sample_num++;
                    if (sample_num == g_nImageOrder)
                        break;
                    else
                        g_nCurrentImageInfo = i;
                } else {
                    sample_num++;
                    if (sample_num == g_nImageOrder)
                        break;

                    g_nCurrentImageInfo++;
                }
            } else
                g_nCurrentImageInfo++;
        }
    } else // RECALL_PHASE
        g_nCurrentImageInfo = g_nImageOrder;
    sprintf(strFileName, "%simage%05d.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
    fflush(stdout);
}

int
truncate_value(double *value) {
    double fractpart, intpart;

    fractpart = modf(*value, &intpart);
    fractpart = fractpart > 0.0 ? 0.5 : 0.0;
    *value = fractpart;

    return ((int) intpart);
}

#if 0

void
set_scale_factor() {
    int log_factor = LOG_FACTOR;
    int m = nl_v1_pattern.dimentions.x;
    double halph_band_width_fp = 7.0 * ((double) NL_WIDTH / 65.0);
    int halph_band_width_int = truncate_value(&halph_band_width_fp);
    g_halph_band_width = halph_band_width_int + halph_band_width_fp;

    double d_width = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);
    double expon = pow(log_factor, ((2.0 * g_halph_band_width) / m)) - 1;
    double a = d_width * (log_factor - 1);

    g_scale_factor = (IMAGE_WIDTH_RESIZED * expon) / a;
}
#else

void
set_scale_factor() {
    int log_factor = LOG_FACTOR;
    int m = nl_v1_pattern.dimentions.x;
    double halph_band_width_fp = 7.0 * ((double) NL_WIDTH / 65.0);
    int halph_band_width_int = truncate_value(&halph_band_width_fp);
    g_halph_band_width = halph_band_width_int + halph_band_width_fp;

    double d_width = fabs(g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol);

    g_scale_factor = compute_scale_factor(
            2.0 * g_halph_band_width,
            d_width,
            IMAGE_WIDTH_RESIZED,
            m,
            log_factor);
}
#endif

void
draw_traffic_light_rectangles(INPUT_DESC *input) {
    if (input->rectangle_list == NULL)
        input->rectangle_list = (RECTANGLE *) calloc(2, sizeof (RECTANGLE));

    input->rectangle_list_size = 1;

    input->rectangle_list[0].x = g_image_info[g_nCurrentImageInfo].leftCol;
    input->rectangle_list[0].y = g_image_info[g_nCurrentImageInfo].bottomRow;
    input->rectangle_list[0].w = g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol;
    input->rectangle_list[0].h = g_image_info[g_nCurrentImageInfo].topRow - g_image_info[g_nCurrentImageInfo].bottomRow;
    input->rectangle_list[0].r = 1.0;
    input->rectangle_list[0].g = 0.0;
    input->rectangle_list[0].b = 0.0;
}

void
reset_gaussian_filter_parameters(void) {
    g_sigma = sqrt(g_scale_factor * 0.8 * 128.0 / 35.0);
    g_kernel_size = (int) (6.0 * g_sigma);
    g_kernel_size = (g_kernel_size % 2) == 0 ? g_kernel_size + 1 : g_kernel_size;

    if (g_sigma < 1.0) {
        g_sigma = 1.0;
        g_kernel_size = 5.0;
    }
}

/*
 ***********************************************************
 * Function: LoadImage
 * Description: Loads a PNM image to the MAE input
 * Inputs: input - input image
 *	  strFileName - Image file name
 * Output: 0 if OK, -1 otherwise
 * Comment: Image format: PNM
 ***********************************************************
 */

void
LoadImage(INPUT_DESC *input, char *strFileName) {
    set_scale_factor();

    load_input_image(input, strFileName);

    check_input_bounds(input, input->wx + input->ww / 2, input->wy + input->wh / 2);
    input->up2date = 0;
    update_input_neurons(input);
    update_input_image(input);

    draw_traffic_light_rectangles(input);

    reset_gaussian_filter_parameters();
}

NEURON_OUTPUT
GetImage(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    char strFileName[256];
    int image_order;

    image_order = pParamList->next->param.ival;

    if (image_order < g_nNumImagesWithTrafficLightsAccordingToCriterion) {
        g_nImageOrder = image_order;
        get_traffic_light_file_name(strFileName);
        LoadImage(&in_pattern, strFileName);
        output.ival = g_image_info[g_nCurrentImageInfo].ClassID;
    } else {
        sprintf(strFileName, "%d", image_order);
        show_message("Could not find image of order: ", strFileName, "");
        output.ival = -1;
    }

    return (output);
}

NEURON_OUTPUT
MoveToTrafficLight(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
    in_pattern.wyd = (g_image_info[g_nCurrentImageInfo].topRow + g_image_info[g_nCurrentImageInfo].bottomRow) / 2;
    move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
MoveToNearTrafficLight(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    is_confidence = 'C';

    int d_width = g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol;
    int d_height = g_image_info[g_nCurrentImageInfo].bottomRow - g_image_info[g_nCurrentImageInfo].topRow;

    int x_center = (g_image_info[g_nCurrentImageInfo].rightCol + g_image_info[g_nCurrentImageInfo].leftCol) / 2;
    int y_center = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow)) / 2;

    in_pattern.wxd = (x_center > (IMAGE_WIDTH / 2)) ? x_center - 1.5 * d_width : x_center + 1.5 * d_width;
    in_pattern.wyd = (y_center > (IMAGE_HEIGHT / 2)) ? y_center - 1.5 * d_height : y_center + 1.5 * d_height;

    move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
MoveToFarTrafficLight(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    is_confidence = 'D';

    int d_width = g_image_info[g_nCurrentImageInfo].rightCol - g_image_info[g_nCurrentImageInfo].leftCol;
    int d_height = g_image_info[g_nCurrentImageInfo].bottomRow - g_image_info[g_nCurrentImageInfo].topRow;

    int x_center = (g_image_info[g_nCurrentImageInfo].rightCol + g_image_info[g_nCurrentImageInfo].leftCol) / 2;
    int y_center = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow)) / 2;

    int far_width;
    int far_height;

    if (x_center > (IMAGE_WIDTH / 2)) {
        far_width = x_center - 8.0 * d_width;
        in_pattern.wxd = far_width < 0 ? d_width : far_width;
    } else {
        far_width = x_center + 8.0 * d_width;
        in_pattern.wxd = far_width > IMAGE_WIDTH ? IMAGE_WIDTH - d_width : far_width;
    }

    if (y_center > (IMAGE_HEIGHT / 2)) {
        far_height = y_center - 8.0 * d_height;
        in_pattern.wyd = far_height < 0 ? d_height : far_height;
    } else {
        far_height = y_center + 8.0 * d_height;
        in_pattern.wyd = far_height > IMAGE_HEIGHT ? IMAGE_HEIGHT - d_height : far_height;
    }

    move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

    output.ival = 0;

    return (output);
}

int
max(int a, int b) {
    return (a < b) ? b : a;
}

int
min(int a, int b) {
    return (b < a) ? b : a;
}

double
getJaccardCoefficient(int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow) {
    double jaccCoeff = 0.;

    if (!(leftCol > gtRightCol || rightCol < gtLeftCol || topRow > gtBottomRow || bottomRow < gtTopRow)) {
        int interLeftCol = max(leftCol, gtLeftCol);
        int interTopRow = max(topRow, gtTopRow);
        int interRightCol = min(rightCol, gtRightCol);
        int interBottomRow = min(bottomRow, gtBottomRow);

        const double areaIntersection = (abs(interRightCol - interLeftCol) + 1) * (abs(interBottomRow - interTopRow) + 1);
        const double lhRoiSize = (abs(rightCol - leftCol) + 1) * (abs(bottomRow - topRow) + 1);
        const double rhRoiSize = (abs(gtRightCol - gtLeftCol) + 1) * (abs(gtBottomRow - gtTopRow) + 1);

        jaccCoeff = areaIntersection / (lhRoiSize + rhRoiSize - areaIntersection);
    }

    return (jaccCoeff);
}

NEURON_OUTPUT 
SaveState(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;
	FILE *state;
	int i;
	char file_name[512];
	struct stat file_status;
	
	i = -1;
	do
	{
		i++;
		sprintf(file_name, "state%d.bin", i);
	} while ((stat(file_name, &file_status) != -1) || (errno != ENOENT)); // while file exist

	state = fopen(file_name, "w");
	fwrite(&g_num_image_info, sizeof(int), 1, state);
	for (i = 0; i < g_num_image_info; i++)
		fwrite(&(g_image_info[i]), sizeof(IMAGE_INFO), 1, state);
		
	fclose(state);
	
	output.ival = 0;
	
	return (output);
}

NEURON_OUTPUT
CheckTrafficLightDetection(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    int traffic_light_center_x;
    int traffic_light_center_y;
    int largura;
    int altura;
    int i;
    float jaccard;
    int x1, y1, x2, y2, x1_j, y1_j, x2_j, y2_j;

    i = g_nCurrentImageInfo;

    if (isnan(g_image_info[i].leftCol) || isnan(g_image_info[i].rightCol) || isnan(g_image_info[i].topRow) || isnan(g_image_info[i].bottomRow)) {
        printf("FORA DE CENA!\n");
        printf("CONFIANCA = %f\n", g_confidence);
        output.ival = 2;
    } else {
        traffic_light_center_x = (int) ((double) (g_image_info[i].leftCol + g_image_info[i].rightCol) / 2.0 + 0.5);
        traffic_light_center_y = (int) ((double) ((g_image_info[i].topRow) + (g_image_info[i].bottomRow)) / 2.0 + 0.5);

        altura = ((int) ((double) (g_image_info[i].topRow - g_image_info[i].bottomRow) + 0.5));
        largura = ((int) ((double) (g_image_info[i].rightCol - g_image_info[i].leftCol) + 0.5));
        altura = abs(altura);
        largura = abs(largura);

        x1 = in_pattern.wxd - largura / 2;
        y1 = in_pattern.wyd - altura / 2;
        x2 = x1 + largura;
        y2 = y1 + altura;
        x1_j = x1;
        y1_j = y2;
        x2_j = x2;
        y2_j = y1;

        jaccard = getJaccardCoefficient(x1_j, y1_j, x2_j, y2_j, g_image_info[i].leftCol, g_image_info[i].topRow, g_image_info[i].rightCol, g_image_info[i].bottomRow);

        printf("@;%c;%dx%d;%d;%.2f;%.2f;%F;image=%05d;jaccard=%f;scale_factor=%.5lf;x=%d;y=%d;T.S.Type=%d;x_g=%d;y_g=%d;width=%d;height=%d\n",
                is_confidence, NL_WIDTH, NL_HEIGHT, INPUTS_PER_NEURON, GAUSSIAN_RADIUS, LOG_FACTOR, g_confidence,
                g_image_info[i].ImgNo, jaccard, g_scale_factor, in_pattern.wxd, in_pattern.wyd,
                g_image_info[i].ClassID, traffic_light_center_x, traffic_light_center_y,
                (int) (g_image_info[i].rightCol - g_image_info[i].leftCol), (int) (g_image_info[i].topRow - g_image_info[i].bottomRow));

        fflush(stdout);

        output.ival = 0;
    }
    return (output);
}

void
read_images_info_file(char *file_name) {
    FILE *image_list = NULL;
    char file_line[512];
    int num_lines = 0;

    if ((image_list = fopen(file_name, "r")) == NULL) {
        printf("Error: could not open file '%s' in read_images_info_file().\n", file_name);
        exit(1);
    }

    while (fgets(file_line, 256, image_list))
        num_lines++;

    rewind(image_list);

    if (g_image_info != NULL)
        free(g_image_info);

    g_image_info = (IMAGE_INFO *) calloc(num_lines, sizeof (IMAGE_INFO));
    if (!g_image_info) {
        printf("Error: could not allocate memory in read_images_info_file().\n");
        exit(1);
    }

    for (g_num_image_info = 0; g_num_image_info < num_lines; g_num_image_info++) {
        if (fgets(file_line, 256, image_list)) {
            if (g_nStatus == TRAINING_PHASE) {
                sscanf(file_line, "image%d.pnm;%d;%d;%d;%d", &(g_image_info[g_num_image_info].ImgNo), &(g_image_info[g_num_image_info].leftCol),
                        &(g_image_info[g_num_image_info].topRow), &(g_image_info[g_num_image_info].rightCol),
                        &(g_image_info[g_num_image_info].bottomRow));
                g_image_info[g_num_image_info].ClassID = 0;
            } else // RECALL_PHASE
            {
                sscanf(file_line, "image%d.pnm", &(g_image_info[g_num_image_info].ImgNo));
                g_image_info[g_num_image_info].leftCol = -1;
                g_image_info[g_num_image_info].topRow = -1;
                g_image_info[g_num_image_info].rightCol = -1;
                g_image_info[g_num_image_info].bottomRow = -1;
                g_image_info[g_num_image_info].ClassID = -1;
                g_image_info[g_num_image_info].found = -1;
                g_image_info[g_num_image_info].used_for_trainning = -1;
                g_image_info[g_num_image_info].best_confidence = -1.0;
                g_image_info[g_num_image_info].best_x = -1;
                g_image_info[g_num_image_info].best_y = -1;
                g_image_info[g_num_image_info].best_scale_factor = -1.0;
            }
        }
    }
    fclose(image_list);
}

/*
 ***********************************************************
 * Function: GetNextTrafficLightFileName
 * Description:
 * Inputs: strFileName -
 *	  nDirection -
 * Output: returns 0 if OK, -1 otherwise
 ***********************************************************
 */

void
GetNextTrafficLightFileName(char *strFileName, int nDirection) {
    if (nDirection == DIRECTION_FORWARD) {
        if (g_nImageOrder == (g_nNumImagesWithTrafficLightsAccordingToCriterion - 1))
            g_nImageOrder = 0; // first valid image number
        else
            g_nImageOrder++;
    } else {
        if (g_nImageOrder == 0)
            g_nImageOrder = g_nNumImagesWithTrafficLightsAccordingToCriterion - 1;
        else
            g_nImageOrder--;
    }
    get_traffic_light_file_name(strFileName);
}

/*
 ***********************************************************
 * Function: GetNewImage
 * Description:
 * Inputs: input -
 *	  nDirection -
 * Output: 0 if OK, -1 otherwise
 ***********************************************************
 */

int
GetNewImage(INPUT_DESC *input, int nDirection) {
    char strFileName[128];

    GetNextTrafficLightFileName(strFileName, nDirection);
    LoadImage(input, strFileName);

    return (0);
}

float
triangular_dist(float x, float mean, float dev) {
    float inverse_distance;

    if (fabs(mean - x) < (dev * 0.5))
        inverse_distance = 1.0;
    else if (fabs(mean - x) < (dev * 0.6))
        inverse_distance = 0.9;
    else if (fabs(mean - x) < (dev * 0.7))
        inverse_distance = 0.8;
    else if (fabs(mean - x) < (dev * 0.8))
        inverse_distance = 0.6;
    else if (fabs(mean - x) < (dev * 0.9))
        inverse_distance = 0.3;
    else if (fabs(mean - x) < (dev * 1.0))
        inverse_distance = 0.1;
    else
        inverse_distance = 0.0;

    return (inverse_distance);
}

void
init_nl_landmark_eval_mask_for_log_polar(NEURON_LAYER *nl_landmark_eval_mask) {
    int w, h, x, y;
    NEURON *neuron_vector;

    neuron_vector = nl_landmark_eval_mask->neuron_vector;
    w = nl_landmark_eval_mask->dimentions.x;
    h = nl_landmark_eval_mask->dimentions.y;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            neuron_vector[y * w + x].output.fval = triangular_dist((float) x, (float) (w - 1) / 2.0, g_halph_band_width);
        }
    }
}

void
train_traffic_light(void) {
    g_nNetworkStatus = TRAINNING;

    init_nl_landmark_eval_mask_for_log_polar(&nl_v1_activation_map_f);
    train_neuron_layer("nl_v1_activation_map");

    g_image_info[g_nCurrentImageInfo].used_for_trainning = 1;

    all_outputs_update();
    g_nNetworkStatus = RUNNING;
}

/*
 ********************************************************
 * Function: saccade				       *
 * Description:   				       *
 * Inputs: input			 		       *
 * Output: none  				       *
 ********************************************************
 */

void
saccade(INPUT_DESC *input) {
    float x, y;
    int count = 0;
    double d_delta_x, d_delta_y;
    int delta_x, delta_y;

    // Saccade until reach the target
#ifdef	CUDA_COMPILED
    x = nl_target_coordinates.host_neuron_vector[0].output.fval;
    y = nl_target_coordinates.host_neuron_vector[1].output.fval;
#else
    x = nl_target_coordinates.neuron_vector[0].output.fval;
    y = nl_target_coordinates.neuron_vector[1].output.fval;
#endif
    d_delta_x = x / g_scale_factor;
    if (d_delta_x > 0.0)
        delta_x = (int) (d_delta_x + 0.5);
    else if (d_delta_x < 0.0)
        delta_x = (int) (d_delta_x - 0.5);
    else
        delta_x = 0;
    input->wxd += delta_x;

    if (input->wxd >= IMAGE_WIDTH)
        input->wxd = IMAGE_WIDTH - 1;
    if (input->wxd < 0)
        input->wxd = 0;

    d_delta_y = y / g_scale_factor;
    if (d_delta_y > 0.0)
        delta_y = (int) (d_delta_y + 0.5);
    else if (d_delta_y < 0.0)
        delta_y = (int) (d_delta_y - 0.5);
    else
        delta_y = 0;
    input->wyd += delta_y;

    if (input->wyd >= IMAGE_HEIGHT)
        input->wyd = IMAGE_HEIGHT - 1;
    if (input->wyd < 0)
        input->wyd = 0;

    move_input_window(input->name, input->wxd, input->wyd);

    count++;
#ifdef	CUDA_COMPILED
    x = nl_target_coordinates.host_neuron_vector[0].output.fval;
    y = nl_target_coordinates.host_neuron_vector[1].output.fval;
#else
    x = nl_target_coordinates.neuron_vector[0].output.fval;
    y = nl_target_coordinates.neuron_vector[1].output.fval;
#endif

    return;
}

int
get_best_specific_type_traffic_light_file_name(char *strFileName, int type, int first_considered, int last_considered) {
    int i;
    int width;
    int best_width;
    int found;

    i = 0;
    while (g_image_info[i].ImgNo < first_considered)
        i++;

    g_nCurrentImageInfo = -1;
    best_width = 500;
    while (g_image_info[i].ImgNo <= last_considered) {
        if (g_image_info[i].ClassID == type) {
            width = g_image_info[i].rightCol - g_image_info[i].leftCol;
            if (abs(width - 50) < best_width) {
                best_width = abs(width - 50);
                g_nCurrentImageInfo = i;
            }
        }
        i++;
    }

    sprintf(strFileName, "%simage%05d.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
    printf("Image file name: %s\n", strFileName);
    fflush(stdout);

    if (g_nCurrentImageInfo != -1)
        found = 1;
    else
        found = 0;

    return (found);
}

NEURON_OUTPUT
GetBestStopTrafficLight(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    char strFileName[256];
    int code;
    int found;
    int first_considered;
    int last_considered;

    code = pParamList->next->param.ival;
    first_considered = pParamList->next->next->param.ival;
    last_considered = pParamList->next->next->next->param.ival;

    if (code >= red_stop_size) {
        sprintf(strFileName, "%d", code);
        show_message("Unknow prohibitory traffic light code: ", strFileName, "");
    }
    found = get_best_specific_type_traffic_light_file_name(strFileName, red_stop[code], first_considered, last_considered);
    printf("Found best class id %d in image %s", code, strFileName);

    if (found) {
        LoadImage(&in_pattern, strFileName);
        in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
        in_pattern.wyd = (((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].topRow) + ((IMAGE_HEIGHT - 1) - g_image_info[g_nCurrentImageInfo].bottomRow)) / 2;
        move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
    } else {
        sprintf(strFileName, "%d", code);
        show_message("Could not find prohibitory traffic light of code: ", strFileName, "");
    }

    output.ival = found;

    return (output);
}

int
get_best_specific_type_traffic_light_file_name_and_not_trained(char *strFileName, int type, int first_considered, int last_considered) {
    int i;
    int width;
    int best_width;
    int found;

    i = 0;
    while (g_image_info[i].ImgNo < first_considered)
        i++;

    g_nCurrentImageInfo = -1;
    best_width = 500;
    while (g_image_info[i].ImgNo <= last_considered) {
        if (g_image_info[i].ClassID == type && g_image_info[i].used_for_trainning != 1) {
            width = g_image_info[i].rightCol - g_image_info[i].leftCol;
            if (abs(width - 50) < best_width) {
                best_width = abs(width - 50);
                g_nCurrentImageInfo = i;
            }
        }
        i++;
    }

    sprintf(strFileName, "%simage%05d.pnm", g_input_path, g_image_info[g_nCurrentImageInfo].ImgNo);
    printf("Image file name: %s\n", strFileName);
    fflush(stdout);

    if (g_nCurrentImageInfo != -1)
        found = 1;
    else
        found = 0;

    return (found);
}

NEURON_OUTPUT
GetBestStopTrafficLight_various(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    char strFileName[256];
    int code;
    int found;
    int first_considered;
    int last_considered;

    code = pParamList->next->param.ival;
    first_considered = pParamList->next->next->param.ival;
    last_considered = pParamList->next->next->next->param.ival;

    if (code >= red_stop_size) {
        sprintf(strFileName, "%d", code);
        show_message("Unknow prohibitory traffic light code: ", strFileName, "");
    }
    found = get_best_specific_type_traffic_light_file_name_and_not_trained(strFileName, red_stop[code], first_considered, last_considered);

    if (found) {
        LoadImage(&in_pattern, strFileName);
        in_pattern.wxd = (g_image_info[g_nCurrentImageInfo].leftCol + g_image_info[g_nCurrentImageInfo].rightCol) / 2;
        in_pattern.wyd = (g_image_info[g_nCurrentImageInfo].topRow + g_image_info[g_nCurrentImageInfo].bottomRow) / 2;
        move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
    }
    /*else
    {
            sprintf(strFileName, "%d", code);
            show_message("Could not find prohibitory traffic light of code: ", strFileName, "");
    }*/

    output.ival = found;

    return (output);
}

NEURON_OUTPUT
SetNetworkStatus(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    g_nStatus = pParamList->next->param.ival;

    switch (g_nStatus) {
        case TRAINING_PHASE:
            strcpy(g_strRandomImagesFileName, RANDOM_IMAGES_TRAIN);
            g_input_path = TRAINING_INPUT_PATH;
            break;
        case RECALL_PHASE:
            g_input_path = REACALL_INPUT_PATH;
            strcpy(g_strRandomImagesFileName, RANDOM_IMAGES_RECALL);
            break;
    }
    read_images_info_file(g_strRandomImagesFileName);
    g_nNumImagesWithTrafficLightsAccordingToCriterion = num_images_with_traffic_lights_according_to_criterion();

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
TrainTrafficLight(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    train_traffic_light();

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
GetDeltaMove(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    //	output.ival = (int) (1.3 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));
    output.ival = (int) (1.0 * (((double) IMAGE_WIDTH_RESIZED / 2.0) / g_scale_factor));
    //printf("delta = %d\n", output.ival);

    return (output);
}

NEURON_OUTPUT
MoveToPoint(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    in_pattern.wxd = pParamList->next->param.ival;
    in_pattern.wyd = pParamList->next->next->param.ival;

    move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
SetScaleFactor(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;

    g_scale_factor = pParamList->next->param.fval;
    reset_gaussian_filter_parameters();

    g_nNumImagesWithTrafficLightsAccordingToCriterion = num_images_with_traffic_lights_according_to_criterion();

    output.ival = 0;

    return (output);
}

NEURON_OUTPUT
ReportParcialResults(PARAM_LIST *pParamList) {
    NEURON_OUTPUT output;
    int i;

    for (i = ((g_nCurrentImageInfo - 10) > 0) ? g_nCurrentImageInfo - 10 : 0;
            (i < (g_nCurrentImageInfo + 10)) && i < g_num_image_info;
            i++) {
        if ((g_image_info[i].ImgNo == g_image_info[g_nCurrentImageInfo].ImgNo) && traffic_light_within_criterion(&g_image_info[i]) && !(g_image_info[i].used_for_trainning)) {
            if (is_stop(g_image_info[i].ClassID)) {
                g_NumTrafficSignsSearched++;
                if (g_image_info[i].found)
                    g_NumTrafficSignsFound++;

                printf("### Image = %05d; Scale = %.5lf; T.S. Type = %2d; Width = %3d; x = %4d; y = %3d; Confidence = %.3lf; Total = %4d; Found = %4d; %% = %3.2f; %s\n",
                        g_image_info[i].ImgNo,
                        g_image_info[i].best_scale_factor,
                        g_image_info[i].ClassID,
                        g_image_info[i].rightCol - g_image_info[i].leftCol,
                        g_image_info[i].best_x,
                        g_image_info[i].best_y,
                        g_image_info[i].best_confidence,
                        g_NumTrafficSignsSearched,
                        g_NumTrafficSignsFound,
                        100.0 * ((float) g_NumTrafficSignsFound / (float) g_NumTrafficSignsSearched),
                        (g_image_info[i].found) ? "HIT" : "MISS");
                fprintf(stderr, "### Image = %05d; Scale = %.5lf; T.S. Type = %2d; Width = %3d; x = %4d; y = %3d; Confidence = %.3lf; Total = %4d; Found = %4d; %% = %3.2f; %s\n",
                        g_image_info[i].ImgNo,
                        g_image_info[i].best_scale_factor,
                        g_image_info[i].ClassID,
                        g_image_info[i].rightCol - g_image_info[i].leftCol,
                        g_image_info[i].best_x,
                        g_image_info[i].best_y,
                        g_image_info[i].best_confidence,
                        g_NumTrafficSignsSearched,
                        g_NumTrafficSignsFound,
                        100.0 * ((float) g_NumTrafficSignsFound / (float) g_NumTrafficSignsSearched),
                        (g_image_info[i].found) ? "HIT" : "MISS");
            } else
                printf("### Image = %05d; Scale = %.5lf; T.S. Type = %2d; Width = %3d; x = %4d; y = %3d; Confidence = %.3lf; Total = %4d; Found = %4d; %% = %3.2f; %s\n",
                    g_image_info[i].ImgNo,
                    g_image_info[i].best_scale_factor,
                    g_image_info[i].ClassID,
                    g_image_info[i].rightCol - g_image_info[i].leftCol,
                    g_image_info[i].best_x,
                    g_image_info[i].best_y,
                    g_image_info[i].best_confidence,
                    g_NumTrafficSignsSearched,
                    g_NumTrafficSignsFound,
                    100.0 * ((float) g_NumTrafficSignsFound / (float) g_NumTrafficSignsSearched),
                    "NA");

        }
    }

    output.ival = 0;

    return (output);
}

/*
 ********************************************************
 * Function: f_keyboard  			       *
 * Description: keyboard events handler  	       *
 * Inputs: key_value (pointer to pressed character)     *
 * Output: none  				       *
 ********************************************************
 */

void
f_keyboard(char *key_value) {
    char key;

    switch (key = key_value[0]) {
            // Train network
        case 'T':
        case 't':
            train_traffic_light();
            break;
            // Saccade until reach the target
        case 'S':
        case 's':
            saccade(&in_pattern);
            break;
            // Move input
        case 'J':
        case 'j':
            in_pattern.wxd--;
            move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
            break;
        case 'K':
        case 'k':
            in_pattern.wxd++;
            move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
            break;
        case 'I':
        case 'i':
            in_pattern.wyd++;
            move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
            break;
        case 'M':
        case 'm':
            in_pattern.wyd--;
            move_input_window(in_pattern.name, in_pattern.wxd, in_pattern.wyd);
            break;
        case 'F':
        case 'f':
            clear_network_memory();
            break;
    }

    return;
}

/*
 ********************************************************
 * Function: init_user_functions 		       *
 * Description:  				       *
 * Inputs: none  				       *
 * Output:					       *
 ********************************************************
 */

int
init_user_functions(void) {
    char strCommand[128];
    char *locale_string;
    char strFileName[128];

    locale_string = setlocale(LC_ALL, "C");
    if (locale_string == NULL) {
        fprintf(stderr, "Could not set locale.\n");
        exit(1);
    } else
        printf("Locale set to %s.\n", locale_string);

    g_nStatus = TRAINING_PHASE;
    strcpy(g_strRandomImagesFileName, RANDOM_IMAGES_TRAIN);
    g_input_path = TRAINING_INPUT_PATH;
    read_images_info_file(g_strRandomImagesFileName);
    g_nNumImagesWithTrafficLightsAccordingToCriterion = num_images_with_traffic_lights_according_to_criterion();
    printf("Number of images with traffic lights within criterion = %d\n", g_nNumImagesWithTrafficLightsAccordingToCriterion);
    GetNextTrafficLightFileName(strFileName, DIRECTION_FORWARD);

    if (strcmp(strFileName, "") != 0)
        LoadImage(&in_pattern, strFileName);

    sprintf(strCommand, "move %s to %d, %d;", in_pattern.name, in_pattern.wxd, in_pattern.wyd);
    interpreter(strCommand);

    sprintf(strCommand, "toggle move_active;");
    interpreter(strCommand);

    sprintf(strCommand, "toggle draw_active;");
    interpreter(strCommand);

    update_input_neurons(&in_pattern);
    all_filters_update();
    all_outputs_update();

    srand(5);

    return (0);
}