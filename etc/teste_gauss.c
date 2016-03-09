#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define	VECTOR_SIZE	100
#define SAMPLES		1000
#define LRAND48_MAX	((unsigned int) -1 >> 1) 

int vector[VECTOR_SIZE];

double gaussrand()
{
        static double V1, V2, S;
        static int phase = 0;
        double X;

        if(phase == 0) {
                do {
                        double U1 = (double)lrand48() / LRAND48_MAX;
                        double U2 = (double)lrand48() / LRAND48_MAX;

                        V1 = 2 * U1 - 1;
                        V2 = 2 * U2 - 1;
                        S = V1 * V1 + V2 * V2;
                        } while(S >= 1 || S == 0);

                X = V1 * sqrt(-2 * log(S) / S);
        } else
                X = V2 * sqrt(-2 * log(S) / S);

        phase = 1 - phase;

        return X;
}


int
main ()
{
	int i;
	double value;
	
	srand48 (5);
	
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		vector[i] = 0; 
	}
	for (i = 0; i < (VECTOR_SIZE * SAMPLES); i++)
	{
		value = gaussrand() + 4.0;
		if (value < 0.0)
			value = 0.0;
		else if (value > 8.0)
			value = 7.99;
		 
		(vector[(int) ((value/8.0) * (double) VECTOR_SIZE)])++; 
	}
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		printf ("%d %d\n", i, vector[i]); 
	}
}
