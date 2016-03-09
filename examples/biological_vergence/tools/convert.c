#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define 	pi           	3.1415926535897932384626433832795029
#define C1			95.77775


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
	FILE *file_in, * file_out;
	int u, v;
	int xi, yi;
	float disparity;
	
	if (argc != 3)
	{
		printf("./convert <file_in> <file_out>\n");
		exit(-1);
	}
	
	
	file_in = fopen (argv[1], "r");
	file_out = fopen (argv[2], "w");
	
	while (!feof(file_in))
	{
		fscanf(file_in, "%d %d %f\n", &(u), &(v), &(disparity));

		map_v1_to_image (&(xi), &(yi), 512, 512, 
		u, v, 128, 32, 256, 256);
		
		fprintf(file_out, "%d %d %f\n", xi, yi, disparity);
	}
	
	fclose (file_in);
	fclose (file_out);
	
	return (0);
}
