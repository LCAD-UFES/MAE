#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define	VECTOR_SIZE	100
#define SAMPLES		1000

int vector[VECTOR_SIZE];


int
main ()
{
	int i;
	double value, x;
	
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		vector[i] = 0; 
	}

	x = 0.0;
	for (i = - VECTOR_SIZE/2; i < VECTOR_SIZE/2; i++)
	{
		x = x + 1.0/exp (-((x/8.0)*(x/8.0)));
		 
		vector[i] = x; 
	}
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		printf ("%d %d\n", i, vector[i]); 
	}
}
