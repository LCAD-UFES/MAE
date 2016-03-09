#include <stdlib.h>
#include <math.h>
#include "../include/mae.h"

#define TAM_GAUSS_VEC	1000


double integ_gauss[TAM_GAUSS_VEC];
double distance_vector[TAM_GAUSS_VEC];

double 
gaussian_integral (double radius)
{
	int i;
	double gaussian;
	double gaussian_prev;
	double incr;
	double i_gauss;
	double x;
	
	i_gauss = integ_gauss[0] = gaussian_prev = 0.0;
	incr = radius / (double) TAM_GAUSS_VEC;
	for (i = 1; i < TAM_GAUSS_VEC; i++)
	{
		x = (double) i * incr;
		gaussian = exp (-(x * x));
		i_gauss += ((gaussian_prev + gaussian) * incr) / 2.0;
		integ_gauss[i] = i_gauss;
		gaussian_prev = gaussian;
		printf ("%d %f\n", i, gaussian);
	}
	return (i_gauss);
}


void
comp_distance_vector (double i_gauss)
{
	int d, i;
	
	printf ("\n\n");
	i = 0;
	for (d = 0; d < TAM_GAUSS_VEC; d++)
	{
		if (((double) d / (double) (TAM_GAUSS_VEC-1)) > (integ_gauss[i] / i_gauss))
		{
			i++;
			while (((double) d / (double) (TAM_GAUSS_VEC-1)) > (integ_gauss[i] / i_gauss))
				i++;
		}
		distance_vector[d] = (double) (i) / (double) (TAM_GAUSS_VEC-1);
		printf ("%d %f\n", d, distance_vector[d]);
	}
}


int
main ()
{
	comp_distance_vector (gaussian_integral (3.0));
}
