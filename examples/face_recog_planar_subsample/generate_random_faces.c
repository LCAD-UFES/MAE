#include <stdio.h>
#include <stdlib.h>
#include "face_recog_user_functions/face_recog_user_functions.h"


int 
main ()
{
	FILE *random_faces_r, *random_faces_t;
	int i, j, k, get_one;
	
	random_faces_r = fopen ("random_faces_r.txt", "w");
	random_faces_t = fopen ("random_faces_t.txt", "w");
	
	for (i = MIN_PERSON_ID; i <= MAX_PERSON_ID; i++)
	{
		for (j = MIN_ILLUMINATION; j <= (MAX_ILLUMINATION-MIN_ILLUMINATION)/2 + MIN_ILLUMINATION; j++)
		{
			fprintf (random_faces_r, "%d %d\n", i, j);
		}
		
		for ( ; j <= MAX_ILLUMINATION; j++)
		{
			fprintf (random_faces_t, "%d %d\n", i, j);
		}
	}
	
	fclose (random_faces_r);
	fclose (random_faces_t);
}
