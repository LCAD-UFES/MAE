#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define	VECTOR_SIZE	100
#define SAMPLES		1000
#define LRAND48_MAX	((unsigned int) -1 >> 1) 

int vector[VECTOR_SIZE];


int
main ()
{
	int i;
	double value;
	double x;
	
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		vector[i] = 0; 
	}
	
	value = 0;
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		x = 4.0 * ((double) i / (double) (VECTOR_SIZE));
		value = exp (-(x * x));
		
		vector[i] = value; 
	}
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		printf ("%d %d\n", i, vector[i]); 
	}
}
