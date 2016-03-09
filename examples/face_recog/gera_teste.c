#include <stdio.h>
#include <stdlib.h>

#define ALL_MAN		76
#define ALL_WOMAN	60
#define MAN		30
#define WOMAN		20
#define TOTAL_POSES	14
#define POSES2TRAIN	7
#define POSES2TEST	7

int g_nAllMan[ALL_MAN];
int g_nAllWoman[ALL_WOMAN];
int g_nAllPoses[POSES2TRAIN + POSES2TEST];

int g_nMan[MAN];
int g_nWoman[WOMAN];
int g_nManPoses[MAN][POSES2TRAIN + POSES2TEST];
int g_nWomanPoses[WOMAN][POSES2TRAIN + POSES2TEST];

int UniqueRandom (int *p_nVector, int nVectorLength)
{
	int i, nPos, nRandValue;
	
	nPos = rand () % nVectorLength;
	nRandValue = p_nVector[nPos];
	
	for (i = nPos; i < (nVectorLength - 1); i++)
	{
		p_nVector[i] = p_nVector[i + 1];
	}
	
	return (nRandValue);
}



int main (int argc, char **argv)
{
	FILE *fp = NULL;
	int i, j, nPose, nPerson, nPersonLength, nPoseLength;
	
	for (i = 0; i < ALL_MAN; i++)
		g_nAllMan[i] = i + 1;
			
	for (i = 0, nPersonLength = ALL_MAN; i < MAN; i++, nPersonLength--)
	{
		while ((g_nMan[i] = UniqueRandom (g_nAllMan, nPersonLength)) == 11 || g_nMan[i] == 24 ||
		        g_nMan[i] == 28 || g_nMan[i] == 29 || g_nMan[i] == 34 || g_nMan[i] == 35  || 
			g_nMan[i] == 50 || g_nMan[i] == 57 || g_nMan[i] == 62 || g_nMan[i] == 63 || g_nMan[i] == 64 || g_nMan[i] == 68)
			nPersonLength--;

		for (j = 0; j < (POSES2TRAIN + POSES2TEST); j++)
		{
			if (j < POSES2TRAIN)
				g_nAllPoses[j] = j + 1;
			else
				g_nAllPoses[j] = j + 7;
		}
		
		for (j = 0, nPoseLength = POSES2TRAIN + POSES2TEST; j < (POSES2TRAIN + POSES2TEST); j++, nPoseLength--)
			g_nManPoses[i][j] = UniqueRandom (g_nAllPoses, nPoseLength);
	}

	for (i = 0; i < ALL_WOMAN; i++)
		g_nAllWoman[i] = i + 1;
		
	for (i = 0, nPersonLength = ALL_WOMAN; i < WOMAN; i++, nPersonLength--)
	{
		while ((g_nWoman[i] = UniqueRandom (g_nAllWoman, nPersonLength)) == 1 || g_nWoman[i] == 6 || g_nWoman[i] == 10 ||
			g_nWoman[i] == 27 || g_nWoman[i] == 47 || g_nWoman[i] == 49 || g_nWoman[i] == 56)
			nPersonLength--;

		for (j = 0; j < (POSES2TRAIN + POSES2TEST); j++)
		{
			if (j < POSES2TRAIN)
				g_nAllPoses[j] = j + 1;
			else
				g_nAllPoses[j] = j + 7;
		}
			
		for (j = 0, nPoseLength = POSES2TRAIN + POSES2TEST; j < (POSES2TRAIN + POSES2TEST); j++, nPoseLength--)
			g_nWomanPoses[i][j] = UniqueRandom (g_nAllPoses, nPoseLength);
	}	
		
	fp = fopen ("random_faces.txt", "w");
	
	for (i = 0; i < MAN; i++)
		for (j = 0; j < POSES2TRAIN; j++)			
			fprintf (fp, "m %d %d\n", g_nMan[i], g_nManPoses[i][j]);
		
	for (i = 0; i < WOMAN; i++)
		
		for (j = 0; j < POSES2TRAIN; j++)	
			fprintf (fp, "w %d %d\n", g_nWoman[i], g_nWomanPoses[i][j]);
	
	for (i = 0; i < MAN; i++)
		for (j = 0; j < POSES2TEST; j++)			
			fprintf (fp, "m %d %d\n", g_nMan[i], g_nManPoses[i][j + POSES2TRAIN]);
		
	for (i = 0; i < WOMAN; i++)
		
		for (j = 0; j < POSES2TEST; j++)	
			fprintf (fp, "w %d %d\n", g_nWoman[i], g_nWomanPoses[i][j + POSES2TRAIN]);
	
	fclose (fp);

	return 0;
}
