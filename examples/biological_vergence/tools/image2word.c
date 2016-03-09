#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define 	pi           	3.1415926535897932384626433832795029
#define C1			95.77775

struct _xy_pair
    { 
     	int 			x; 
     	int 			y;
    }; 
typedef struct _xy_pair XY_PAIR;

double FOCAL_DISTANCE = 500.0;

double CAMERA_DISTANCE = 0.07;

XY_PAIR CENTER_LEFT;
XY_PAIR CENTER_RIGHT;

struct _world_point
    { 
     	double 			x; 
     	double 			y;
	double			z;
	
    }; 
typedef struct _world_point WORLD_POINT;

struct _spherical_coordinate
    { 
     	double 			r; 
     	double 			theta;
	double			fi;
    }; 
typedef struct _spherical_coordinate SPHERICAL_COORDINATE;

struct _image_coordinate
    { 
     	double 			x; 
     	double 			y;
    }; 
typedef struct _image_coordinate IMAGE_COORDINATE;



IMAGE_COORDINATE 
calculate_euclidean_coordinates(IMAGE_COORDINATE point, XY_PAIR image_dimentions)
{
	IMAGE_COORDINATE euclidean_point;
	euclidean_point.x = point.x - ((float) image_dimentions.x * 0.5);
	euclidean_point.y = point.y - ((float) image_dimentions.y * 0.5);
	return euclidean_point;
}



double
calculate_euclidean_distance (WORLD_POINT start_point, WORLD_POINT end_point)
{
	return sqrt ((start_point.x - end_point.x) * (start_point.x - end_point.x) +
		     (start_point.y - end_point.y) * (start_point.y - end_point.y) +
		     (start_point.z - end_point.z) * (start_point.z - end_point.z));
}



WORLD_POINT
calculate_euclidean_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, 
				 double focal_distance, double camera_distance)
{
	double theta_l, theta_r;  	/* Vergence Angles */
	double z; 			/* Projection of r in World in zx plane */
	double lambda; 			/* Gaze Angle */
	double theta; 
	WORLD_POINT thePoint; 		/*The Euclidean World Point */
	IMAGE_COORDINATE left_point_coordinate;
	IMAGE_COORDINATE right_point_coordinate;
	
	left_point_coordinate = calculate_euclidean_coordinates (left_point, image_dimentions);
	right_point_coordinate = calculate_euclidean_coordinates (right_point, image_dimentions);
	
	theta_l = atan (-left_point_coordinate.x / focal_distance);
	theta_r = atan (right_point_coordinate.x / focal_distance);
	
	z = sqrt (camera_distance * camera_distance *
		 ((sin (theta_l - theta_r) * sin (theta_l - theta_r) / (4.0 * sin (theta_l + theta_r) * sin (theta_l + theta_r))) +
		 (cos (theta_l) * cos (theta_l) * cos (theta_r) * cos (theta_r) / (sin (theta_l + theta_r) * sin (theta_l + theta_r)))));
	
	lambda = atan (sin (theta_l - theta_r) / (2.0 * cos (theta_l) * cos (theta_r))); 
	theta = atan (right_point_coordinate.y / focal_distance);
	
	thePoint.x = -z * sin (lambda);
	thePoint.z = z * cos (lambda);
	thePoint.y = thePoint.z * tan (theta);
	
	return thePoint;
}



WORLD_POINT
calculate_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, 
				 double focal_distance, double camera_distance)
{
	double theta_l, theta_r;  	/* Vergence Angles */
	double z; 			/* Projection of r in World in zx plane */
	double lambda; 			/* Gaze Angle */
	WORLD_POINT thePoint; 		/*The Euclidean World Point */
	IMAGE_COORDINATE left_point_coordinate;
	IMAGE_COORDINATE right_point_coordinate;
	
	left_point_coordinate = calculate_euclidean_coordinates (left_point, image_dimentions);
	right_point_coordinate = calculate_euclidean_coordinates (right_point, image_dimentions);
	
	thePoint.x = camera_distance * (right_point_coordinate.x + left_point_coordinate.x) / (2.0 * (left_point_coordinate.x - right_point_coordinate.x));
	thePoint.y = camera_distance * right_point_coordinate.y / (left_point_coordinate.x - right_point_coordinate.x);
	thePoint.z = focal_distance * camera_distance / (left_point_coordinate.x - right_point_coordinate.x);
	return thePoint;
}



float
get_image_focal_distance (float calibrate_focal_distance, int calibrate_image_width, int image_width)
{
	return calibrate_focal_distance * ((float) image_width) / ((float) calibrate_image_width);
}



double
distance_from_image_center (int wi, int hi, int w, int h, int u)
{

	double exp_val, x, y;
	double  y0_val;

	y0_val = pow ((20.05/(0.26 * C1)), (1.0/0.26)) - 0.08;

	x = 63.951256 * ((double) u/(double) (w/2));
	y = pow ((20.05 / (0.26 * (C1 - x))), (1.0 / 0.26)) - 0.08;
	y = 30.0 * (y - y0_val) / (30.0 - y0_val);
	exp_val = (double) (wi/2) * (y / 30.0);
	
	return (exp_val);
}



double
calculate_pixel_size (int wi, int hi, int w, int h, int u)
{
	double d;
	
	if (u < w/2)
		d = (distance_from_image_center (wi, hi, w, h, w/2 - 1 - u) -
	    	     distance_from_image_center (wi, hi, w, h, w/2 - 2 - u));
	else
		d = (distance_from_image_center (wi, hi, w, h, u - w/2) -
	    	     distance_from_image_center (wi, hi, w, h, u - w/2 - 1));
		     
	return (0.005 + d/5000.0); 
}




void
map_v1_to_image (int *xi, int *yi, int wi, int hi, int u, int v, int w, int h, int x_center, int y_center)
{
	static int previous_u = -1;
	static double previous_d;
	double d, theta, correction;
	
	correction = (double) h / (double) (h - 1);
	if (u < w/2)
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = distance_from_image_center (wi, hi, w, h, w/2 - 1 - u);

		theta = pi * (((double) h * (3.0 / 2.0) - ((double) v * correction)) / (double) h);
		
	}
	else
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = distance_from_image_center (wi, hi, w, h, u - w/2);
		
		theta = pi * (((double) h * (3.0 / 2.0) + ((double) v * correction)) / (double) h);
		
	}

	*xi = (int) (d * cos(theta) + (float) x_center + 0.5);
	*yi = (int) (d * sin(theta) + (float) y_center + 0.5);

	previous_u = u;
	previous_d = d;
}



int
main(int argc, char **argv)
{
	FILE *disparity_map_file = (FILE *) NULL;
	FILE *color_map_file     = (FILE *) NULL;
	FILE *iv_file 	    	 = (FILE *) NULL;
	int u, v, w, h;
	int xi, yi, wi, hi;
	float disparity;
	float r, g, b;
	IMAGE_COORDINATE left_point, right_point;
	WORLD_POINT world_point, previus_world_point;
	XY_PAIR image_dimentions;
	float pixel_size;
	float *disparity_map;
	int *red_map, *green_map, *blue_map;
	int i;
	char option = '\0';
	
	if (argc != 3)
	{
		printf ("./convert <disparity_map_file> <color_map_file>\n");
		exit(-1);
	}
	
	if ((disparity_map_file = fopen (argv[1], "r")) == (FILE *) NULL)
	{
		printf ("Erro: cannot open file '%s' to read.\n", argv[1]);
		exit (-1);
	}
	
	if ((color_map_file = fopen (argv[2], "r")) == (FILE *) NULL)
	{
		printf ("Erro: cannot open file '%s' to read.\n", argv[2]);
		exit (-1);
	}
	
	if ((iv_file = fopen ("word.iv", "w")) == (FILE *) NULL)
	{
		printf ("Erro: cannot open file '%s' to write.\n", argv[3]);
		exit (-1);
	}
	
	CENTER_LEFT.x = 290;
	CENTER_LEFT.y = 189;
	
	CENTER_RIGHT.x = 238;
	CENTER_RIGHT.y = 189;
	
	printf ("Default values:\n");
	printf ("\tFOCAL_DISTANCE  = %.4f\n", FOCAL_DISTANCE);
	printf ("\tCAMERA_DISTANCE = %.4f\n", CAMERA_DISTANCE);
	printf ("\tCENTER_RIGHT    = (%d, %d)\n", CENTER_RIGHT.x, CENTER_RIGHT.y);
	printf ("\tCENTER_LEFT     = (%d, %d)\n", CENTER_LEFT.x, CENTER_LEFT.y);
	
	/*while ((option != 'y') && (option != 'n'))
	{
		printf ("Enter menu?\n[y|n]");
		option = getchar();
	}
	
	if (option == 'y')
	{
		printf ("\n\nInteractive Menu\n\n");
		printf ("FOCAL_DISTANCE\n");
		scanf ("%f", FOCAL_DISTANCE);
		printf ("CAMERA_DISTANCE\n");
		scanf ("%f", CAMERA_DISTANCE);
		printf ("CENTER_RIGHT\n");
		scanf ("%d %d", CENTER_RIGHT.x, CENTER_RIGHT.y);
		printf ("CENTER_LEFT\n");
		scanf ("%d %d", CENTER_LEFT.x, CENTER_LEFT.y);
	}*/
	
	/* Get the disparity_map dimentions */
	fscanf(disparity_map_file, "%d %d\n", &(w), &(h));
	
	disparity_map = (float *) malloc (w * h * sizeof (float));
	
	for (i = 0; i < (w * h); i++)
		fscanf(disparity_map_file, "%f", &(disparity_map[i]));
		
	fclose (disparity_map_file);
	
	/* Get the color_map dimentions */
	fscanf(color_map_file, "%d %d\n", &(wi), &(hi));
	
	red_map   = (int *) malloc (wi * hi * sizeof (int));
	green_map = (int *) malloc (wi * hi * sizeof (int));
	blue_map  = (int *) malloc (wi * hi * sizeof (int));

	for (i = 0; i < (wi * hi); i++)
		fscanf (color_map_file, "%d %d %d",  &(red_map[i]), &(green_map[i]), &(blue_map[i]));
	
	fclose (color_map_file);

	/* Set the image dimentions */
	image_dimentions.x = wi;
	image_dimentions.y = hi;
	
	/* Prolog */
	fprintf (iv_file, "#Inventor V2.0 ascii\n\n");
	fprintf (iv_file, "Separator \n{\n\tPerspectiveCamera {position 0 0 0}\n");
		
	/* Body */
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			disparity = disparity_map[u + v * w];
	
			map_v1_to_image (&(xi), &(yi), wi, hi, u, v, w, h, CENTER_RIGHT.x, CENTER_RIGHT.y);
			
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
			{
				r = (float) red_map[xi + yi * wi]  / 255.0;
				g = (float) green_map[xi + yi * wi]/ 255.0;
				b = (float) blue_map[xi + yi * wi] / 255.0;
			}
			
			pixel_size = calculate_pixel_size (wi, hi, w, h, u);
			/*pixel_size = 0.005;*/
			
			/* Discard points in the infinity */
			/*if (disparity == 0.0)
				continue;*/

			right_point.x = (double) (CENTER_RIGHT.x + xi);
			right_point.y = (double) (CENTER_RIGHT.y + yi);

			left_point.x = (double) (CENTER_LEFT.x + xi) - disparity;
			left_point.y = (double) (CENTER_LEFT.y + yi);

			world_point = calculate_world_point (left_point, right_point, image_dimentions, FOCAL_DISTANCE, CAMERA_DISTANCE);
			
			/*if (abs (world_point.z) > 100.0)
				continue;*/
			fprintf (iv_file, "\tTranslation {translation  %.8f %.8f %.8f}\n", world_point.x, world_point.y, -world_point.z);
    			fprintf (iv_file, "\tMaterial {diffuseColor %f %f %f}\n",  r, g, b);
			fprintf (iv_file, "\tCube {width %f height %f depth %f}\n", pixel_size, pixel_size, 0.01 * pixel_size);
			fprintf (iv_file, "\tTranslation {translation  %.8f %.8f %.8f}\n\n", -world_point.x, -world_point.y, world_point.z);
		}
	}
	
	/* Epilog */
	fprintf (iv_file, "}\n");

	fclose (iv_file);
	
	system ("SceneViewer word.iv");
	
	printf ("done\n");

	return (0);
}
