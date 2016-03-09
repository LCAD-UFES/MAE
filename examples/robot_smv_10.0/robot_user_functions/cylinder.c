#include <math.h>
#include "robot_user_functions.h"
#include "viewer.hpp"
#include "cylinder.h"

#define INVALID -FLT_MAX
#define RADIUS_WEIGHT 0.7



void
cylinder_print (CYLINDER *cylinder, char * initial_msg)
{
	printf("%s", initial_msg);
	printf("\n");
	printf("x = %f\n", cylinder->x);
	printf("y = %f\n", cylinder->y);
	printf("z = %f\n", cylinder->z);
	printf("size = %f\n", cylinder->size);
	printf("diameter = %f\n", cylinder->diameter);
	printf("aplha = %f\n", cylinder->alpha);
	printf("betha = %f\n", cylinder->betha);
}

void
display_points (int num_points, double *left_points, double *right_points)
{
	int i;
	double disparity;
	int w, h, wi, hi, xi_l, xi_r, yi_r, yo, xo;

	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	for (i = 0; i < w*h; i++)
		nl_disparity_map.neuron_vector[i].output.fval = 0.0;
		
	for (i = 0; i < num_points; i++)
	{
		xi_l = left_points[2*i];
		xi_r = right_points[2*i];
		yi_r = hi - right_points[2*i + 1] - 1;
		disparity = xi_l - xi_r;
		xo = (double) (xi_r * w) / (double) wi + 0.5;
		yo = (double) (yi_r * h) / (double) hi + 0.5;
		nl_disparity_map.neuron_vector[w * yo + xo].output.fval = disparity;
	}
	glutSetWindow(out_disparity_map.win);
	update_output_image (&(out_disparity_map));
	glutPostWindowRedisplay (out_disparity_map.win);
}



void
overwrite_points (int num_points, double *left_points, double *right_points)
{
	int i;
	double disparity;
	int w, h, wi, hi, xi_l, xi_r, yi_r, yo, xo;

	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	for (i = 0; i < num_points; i++)
	{
		xi_l = left_points[2*i];
		xi_r = right_points[2*i];
		yi_r = hi - right_points[2*i + 1] - 1;
		disparity = xi_l - xi_r;
		xo = (double) (xi_r * w) / (double) wi + 0.5;
		yo = (double) (yi_r * h) / (double) hi + 0.5;
		nl_disparity_map.neuron_vector[w * yo + xo].output.fval = disparity;
	}
	glutSetWindow(out_disparity_map.win);
	update_output_image (&(out_disparity_map));
	glutPostWindowRedisplay (out_disparity_map.win);
}

void
exchange_to_disparity_map ()
{
	int w, h, yo, xo;
	double temp;
	// Dimensoes do cortex (disparity map)
	h = nl_cylinder_map.dimentions.y;
	w = nl_cylinder_map.dimentions.x;
  
	//Map cylinder to world
	for (xo = 0; xo < w; xo++)
	{
		for (yo = 0; yo < h; yo++)
		{
      			if (nl_cylinder_map.neuron_vector[w * yo + xo].output.fval != INVALID)
			{
        			temp =  nl_cylinder_map.neuron_vector[w * yo + xo].output.fval;
				nl_cylinder_map.neuron_vector[w * yo + xo].output.fval = 
							nl_disparity_map.neuron_vector[w * yo + xo].output.fval;
				nl_disparity_map.neuron_vector[w * yo + xo].output.fval = temp;
		      	}
		}
	}  
}

void
project_cylinder_2D_points_into_disparity_map (int num_points, double *left_points, double *right_points)
{
	int i;
	double disparity, num_nerons;
	int w, h, wi, hi, xi_l, xi_r, yi_r, yo, xo;

	// Dimensoes do cortex (disparity map)
	h = nl_cylinder_map.dimentions.y;
	w = nl_cylinder_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	num_nerons = w * h;

	//Initialize cylinder map
	for (i = 0; i < num_nerons; i++)
	{
		  nl_cylinder_map.neuron_vector[i].output.fval = INVALID;
	}

	//Generate cylinder map
	for (i = 0; i < num_points; i++)
	{
		xi_l = left_points[2*i];
		xi_r = right_points[2*i];
		yi_r = right_points[2*i + 1];
		disparity = xi_l - xi_r;
		xo = (double) (xi_r * w) / (double) wi + 0.5;
		yo = (double) (yi_r * h) / (double) hi + 0.5;

		if (xo < 0 || xo >= w || yo < 0 || yo >= h)
			continue;

		if (nl_cylinder_map.neuron_vector[w * yo + xo].output.fval < disparity)
		{
			nl_cylinder_map.neuron_vector[w * yo + xo].output.fval = disparity;
    		}
	}

}  

void
test_StereoTriangulation_CameraProjection (void)
{
	int w, h, wi, hi;
	double lw[3], rw[3], lc[2], rc[2];	

	// Dimensoes do cortex (disparity map)
	h = nl_disparity_map.dimentions.y;
	w = nl_disparity_map.dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;
	
	printf ("********* Zeros **********\n");
	lw[0] = 0;
	lw[1] = 0;
	lw[2] = 1000000;
	CameraProjectionRight(1, rc, lw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, lw, LEFT_CAMERA);
	printf ("0, 0, 1000000 RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("0, 0, 1000000 LC x=%02f; y=%02f\n", lc[0], lc[1]);
	lw[0] = 0;
	lw[1] = 0;
	lw[2] = 500;
	CameraProjectionRight(1, rc, lw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, lw, LEFT_CAMERA);
	printf ("0, 0, 500 RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("0, 0, 500 LC x=%02f; y=%02f\n", lc[0], lc[1]);

	rc[0] = 260;
	rc[1] = 237;
	lc[0] = 260;
	lc[1] = 237;
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("r 0,0 l 0,0 RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("r 0,0 l 0,0 antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
	rc[0] = 0;
	rc[1] = 0;
	lc[0] = 200;
	lc[1] = 0;
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("r 0,0 l 200,0 RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("r 0,0 l 200,0 antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);

	printf ("********* Referencia na camera esquerda **********\n");
	rc[0] = wi/2;
	rc[1] = hi/2;
	lc[0] = wi/2 + 100;
	lc[1] = hi/2;
	printf ("antes RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("antes LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
	
	printf ("\n");
	CameraProjectionRight(1, rc, lw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, lw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
	
	printf ("\n");
	lw[0] = 320.323236;
	lw[1] = 5.854590;
	lw[2] = 1234.601669;
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);

	CameraProjectionRight(1, rc, lw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, lw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("depois RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("depois LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);


	printf ("\n");
	printf ("********* Referencia na camera direita **********\n");
	rc[0] = wi/2;
	rc[1] = hi/2;
	lc[0] = wi/2 + 100;
	lc[1] = hi/2;
	printf ("antes RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("antes LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
	
	printf ("\n");
	CameraProjectionRight(1, rc, rw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
	
	printf ("\n");
	rw[0] = 0.0;
	rw[1] = 0.0;
	rw[2] = 10234.601669;
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);

	printf ("\n");
	CameraProjectionRight(1, rc, rw, RIGHT_CAMERA);
	CameraProjectionRight(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("depois RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("depois LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
  
	GetWorldPointAtDistanceRight (rw, rc, 10234.601669, RIGHT_CAMERA);
	printf ("GetWorldPointAtDistanceRight RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
 /* 
	printf ("\n");
	rw[0] = 0;
	rw[1] = 0;
	rw[2] = 10;
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);

	printf ("\n");
	CameraProjection(1, rc, rw, RIGHT_CAMERA);
	CameraProjection(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);

	printf ("\n");
	rw[0] = 0;
	rw[1] = 0;
	rw[2] = 1000;
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);

	printf ("\n");
	CameraProjection(1, rc, rw, RIGHT_CAMERA);
	CameraProjection(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
  
	printf ("\n");
	rw[0] = 0;
	rw[1] = 0;
	rw[2] = 100000;
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);

	printf ("\n");
	CameraProjection(1, rc, rw, RIGHT_CAMERA);
	CameraProjection(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);

	printf ("\n");
	rw[0] = 0;
	rw[1] = 0;
	rw[2] = 1000000000;
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);

	printf ("\n");
	CameraProjection(1, rc, rw, RIGHT_CAMERA);
	CameraProjection(1, lc, rw, LEFT_CAMERA);
	printf ("depois RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("depois LC x=%02f; y=%02f\n", lc[0], lc[1]);
  
	rc[0] = 0 + 145;
	rc[1] = 0;
	lc[0] = 0;
	lc[1] = 0;
	printf ("antes RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("antes LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
	  
	rc[0] = 0;
	rc[1] = 0;
	lc[0] = 0;
	lc[1] = 0;
	printf ("antes RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("antes LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);

	rc[0] = 0 - 100;
	rc[1] = 0;
	lc[0] = 0;
	lc[1] = 0;
	printf ("antes RC x=%02f; y=%02f\n", rc[0], rc[1]);
	printf ("antes LC x=%02f; y=%02f\n", lc[0], lc[1]);
	StereoTriangulation (1, lw, rw, lc, rc);
	printf ("antes RW x=%02f; y=%02f; z=%02f\n", rw[0], rw[1], rw[2]);
	printf ("antes LW x=%02f; y=%02f; z=%02f\n", lw[0], lw[1], lw[2]);
  */
}

void
compute_3D_cylinder (CYLINDER *cylinder, double *cylinder_3D_points, int arcs, int circles, double angle)
{
	int circle, arc, point_index;
	double theta, x, xP, yP, zP;
	double cosBetha, sinBetha, cosAlpha, sinAlpha;
	double PI;

	PI = acos(-1.0);
	angle = PI * angle/180.0;
	cosBetha = cos(cylinder->betha);
	sinBetha = sin(cylinder->betha);
	cosAlpha = cos(cylinder->alpha);
	sinAlpha = sin(cylinder->alpha);
	for (circle = 0; circle < circles; circle++)
	{
		x = (double) circle * cylinder->size / (double) circles - (cylinder->size / 2.0);
		for (arc = 0; arc < arcs; arc++)
		{
			theta = (double) arc * angle / (double) arcs;
			point_index = 3 * (circle*arcs + arc);
			xP = x;
			yP = cos (theta) * cylinder->diameter / 2.0;
			zP = -sin (theta) * cylinder->diameter / 2.0 + (cylinder->diameter / 2.0);

      			//Rotate
			cylinder_3D_points[point_index + 0] = (xP*cosAlpha - yP*sinAlpha)*cosBetha + sinBetha*zP + cylinder->x;
			cylinder_3D_points[point_index + 1] = xP*sinAlpha + yP*cosAlpha + cylinder->y;
			cylinder_3D_points[point_index + 2] = (xP*cosAlpha - yP*sinAlpha)*(-sinBetha) + cosBetha*zP + cylinder->z;

		}
	}
}

void
cylinder_3D_view_color (CYLINDER *cylinder, int arcs, int circles, int red, int green, int blue)
{
	int i;
	double num_points = arcs*circles;
	double *world3DCylinder = (double*) alloc_mem (3 * num_points * sizeof (double));

	compute_3D_cylinder (cylinder, world3DCylinder, arcs, circles, 360);
	for (i = 0; i < num_points; i++)
	{
		// Updates the map and rotates the frame coordinates			       			
		ViewerUpdateMap(1, world3DCylinder[3*i+2],
		                  -world3DCylinder[3*i+0],
				  world3DCylinder[3*i+1], red, green, blue);
	}

	free(world3DCylinder);
}

void
view_3D_cylinder (CYLINDER *cylinder, int arcs, int circles)
{
	cylinder_3D_view_color (cylinder, arcs, circles, 255, 0, 0);
}

void
cylinder (double x0, double y0, double z0, double size, double diameter, double alpha, 
	  double betha, int arcs, int circles)
{
	int num_points;
	double *cylinder_3D_points, *left_points, *right_points;
	CYLINDER cylinder;
	
	num_points = arcs*circles;
	cylinder_3D_points = (double *) alloc_mem (3 * num_points * sizeof (double));
	left_points = (double *) alloc_mem (2 * num_points * sizeof (double));
	right_points = (double *) alloc_mem (2 * num_points * sizeof (double));

	cylinder.x = x0;
	cylinder.y = y0;
	cylinder.z = z0;
	cylinder.size = size;
	cylinder.diameter = diameter;
	cylinder.alpha = alpha;
	cylinder.betha = betha;

	compute_3D_cylinder (&cylinder, cylinder_3D_points, arcs, circles, 180);
	
	CameraProjectionRight(num_points, left_points, cylinder_3D_points, LEFT_CAMERA);
	CameraProjectionRight(num_points, right_points, cylinder_3D_points, RIGHT_CAMERA);

	
//	overwrite_points (num_points, left_points, right_points);

	project_cylinder_2D_points_into_disparity_map (num_points, left_points, right_points);
 
	free (right_points);
	free (left_points);
	free (cylinder_3D_points);
}

int
is_too_close (CYLINDER *c1, CYLINDER *c2)
{
	double centers_distance = cylinder_centers_distance (c1, c2);

	double radius_weighted_sum = (c1->diameter + c2->diameter) * 0.5 * RADIUS_WEIGHT;
	
	return (centers_distance < radius_weighted_sum);
}

double
cylinder_centers_distance (CYLINDER *c1, CYLINDER *c2)
{
	return sqrt (POW2 (c1->x - c2->x) +
		     POW2 (c1->y - c2->y) +
		     POW2 (c1->z - c2->z));
}

void
copy_c2_to_c1 (CYLINDER *c1, CYLINDER *c2)
{
	c1->x = c2->x;
	c1->y = c2->y;
	c1->z = c2->z;
	c1->size = c2->size;
	c1->diameter = c2->diameter;
	c1->alpha = c2->alpha;
	c1->betha = c2->betha;
}

/*
Propriedades
*/

double
cylinder_radius (CYLINDER *cylinder)
{
	return (cylinder->diameter / 2.0);
}
