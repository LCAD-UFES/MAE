
#ifndef __GROUND_TRUTH_MANIPULATION__
#define __GROUND_TRUTH_MANIPULATION__

typedef struct
{
	int image_id;
	double scale_factor;
	int x;
	int y;
	double confidence;
	int TS_type;
	int x_g;
	int y_g;
	int width;
	int height;
	int hit;
} Line;

typedef struct
{
	int ImgNo, leftCol, topRow, rightCol, bottomRow, ClassID;
	int found;
	int used_for_trainning;
	double best_confidence;
	int best_x;
	int best_y;
	double best_scale_factor;
} IMAGE_INFO;

#define IMG_WIDTH 1360
#define IMG_HEIGHT 800

int check_is_hit(Line *l);
int check_is_hit_based_on_gt(Line *line, IMAGE_INFO *gt_line);
void print_hit_information();
int is_prohibitory(int ClassID);
void load_ground_truth(char *filename);
IMAGE_INFO* get_next_ground_truth_sign();

#endif
