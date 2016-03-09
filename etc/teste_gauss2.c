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

	for (i = 0; i < (VECTOR_SIZE * SAMPLES); i++)
	{
		x = (((double) i) * 16.0) / ((double) VECTOR_SIZE * SAMPLES);
		value = (x*exp(-1.0))/exp (-((x/8.0)*(x/8.0)));
		if (value < 0.0)
			value = 0.0;
		else if (value > 16.0)
			value = 15.99;
		 
		 
		(vector[(int) ((value/16.0) * (double) VECTOR_SIZE)])++; 
	}
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		printf ("%d %d\n", i, vector[i]); 
	}
}
