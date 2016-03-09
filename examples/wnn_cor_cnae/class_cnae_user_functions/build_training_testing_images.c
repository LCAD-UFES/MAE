#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "build_training_testing_images.h"
#include "../class_cnae.h"


int file_id = 1;

/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/

void
build_cnae_file (char *inicial, int *cnae_codes, int cnae_codes_length)
{
	char FileName[256];
	FILE *FileOut = (FILE *) NULL;
	int i;
	
	sprintf (FileName, "%s%d.cnae", inicial, file_id);

        if (!(FileOut = fopen(FileName,"w")))
	{
		fprintf (stderr,"Cannot open input file %s\n",FileName);
	        exit(1);
	}

	if (cnae_codes_length == 0)
	{
		fprintf(FileOut,"%d\n", 100000);
	}
	else
	{
		for(i = 0; i < cnae_codes_length ; i++)
		{
			fprintf(FileOut,"%d\n",cnae_codes[i]);
		}
	}
		
	fclose(FileOut);
	
	return;
}



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/

void
build_image_file (char *inicial, float *vts, int vts_length)
{
	char FileName[256];
	FILE *FileOut = (FILE *) NULL;
	int i, sec_dimension;
	float larger_vts_column = 0.0;
	float *f_vts;
	
	sec_dimension = IN_WIDTH*IN_HEIGHT;

        sprintf (FileName, "%s%d.pnm", inicial, file_id);

        if (!(FileOut = fopen(FileName,"w")))
        {
	        fprintf (stderr,"Cannot open input file %s\n",FileName);
	        exit(1);
	}

	for(i = 0;  i < vts_length; i++)
		if (vts[i] > larger_vts_column)
			larger_vts_column = vts[i];
		
	f_vts = (float *) malloc (vts_length * sizeof(float));
	
	for(i = 0;  i < vts_length; i++)
		f_vts[i] = vts[i] / larger_vts_column;
		
	fprintf(FileOut,"P2\n");
	fprintf(FileOut,"#MAE Image max = 1.000000, min = 0.000000\n");
	fprintf(FileOut,"%d %d\n",IN_WIDTH, IN_HEIGHT);
	fprintf(FileOut,"255\n");
	
	for(i = 0; i < sec_dimension; i++)
        {
		if(i < vts_length)
		{
			if((i+1)% IN_WIDTH == 0)
				fprintf(FileOut,"%f\n", f_vts[i]);
			else
				fprintf(FileOut,"%f ", f_vts[i]);
		}
		else	
		{
			if((i+1)% IN_WIDTH == 0)
				fprintf(FileOut,"%f\n", 0.0);
			else
				fprintf(FileOut,"%f ", 0.0);
		}
	}
  
  	free(f_vts);
	fclose(FileOut);
	
	return;
}



void
build_training_testing_images (float *vts, int vts_length, int *cnae_codes, int cnae_codes_length)
{
	build_cnae_file (PATH_REPOSITORY, cnae_codes, cnae_codes_length);
	
	build_image_file (PATH_REPOSITORY, vts, vts_length);

/* the trainning is online now */
#if 0		
	file_id++;	
#endif	
	return;
} 



#ifdef DEBUG
int 
main (int argc, char *argv[])
{
	int i;
	float *vts;
	int *cnae_codes;
	int vts_length, cnae_codes_length;
	
	vts_length = 100;
	
	cnae_codes_length = 6;
	
	vts = (float *) malloc (vts_length * sizeof (float));
	cnae_codes = (int*) malloc (cnae_codes_length * sizeof (int));
	
	for(i = 0;  i < vts_length; i++)
		vts[i] = (float) (i%2);
	
	for(i = 0;  i < cnae_codes_length; i++)
		cnae_codes[i] = i;
	
	build_training_testing_images (vts, vts_length, cnae_codes, cnae_codes_length);
	
	return 0;
}
#endif
