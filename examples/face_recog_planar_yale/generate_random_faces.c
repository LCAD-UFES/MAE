#include <stdio.h>
#include <stdlib.h>
#include "face_recog_user_functions/face_recog_user_functions.h"

int state_taken[MAX_ILLUMINATION-MIN_ILLUMINATION];


void
init_taken()
{
	int i;
	
	for (i = MIN_ILLUMINATION; i < MAX_ILLUMINATION; i++)
		state_taken[i] = 0;
}


int
taken(int k)
{
	return (state_taken[k]);
}


int 
main ()
{
	FILE *random_faces_r, *random_faces_t;
	int i, j, k, get_one;
	
	random_faces_r = fopen ("random_faces_r.txt", "w");
	random_faces_t = fopen ("random_faces_t.txt", "w");
	
	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
	{
		init_taken();
		for (j = MIN_ILLUMINATION; j <= (MAX_ILLUMINATION-MIN_ILLUMINATION)/2 + MIN_ILLUMINATION; j++)
		{
			get_one = 0;
			while (!get_one)
			{
				k = (rand() % (MAX_ILLUMINATION-MIN_ILLUMINATION)) + MIN_ILLUMINATION;
				if (!taken (k))
				{
					state_taken[k] = 1;
					fprintf (random_faces_r, "%d %d\n", i, k);
					//printf ("#%d %d\n", i, k);
					get_one = 1;
				}
			}
		}
		
		k = 0;
		for (j = MIN_ILLUMINATION; j <= (MAX_ILLUMINATION-MIN_ILLUMINATION)/2 + MIN_ILLUMINATION; j++)
		{
			get_one = 0;
			while (!get_one)
			{
				if (!taken (k))
				{
					fprintf (random_faces_t, "%d %d\n", i, k);
					//printf ("@%d %d\n", i, k);
					get_one = 1;
					k++;
				}
				else
				{
					k++;
				}
			}
		}
	}
	
	fclose (random_faces_r);
	fclose (random_faces_t);
}
