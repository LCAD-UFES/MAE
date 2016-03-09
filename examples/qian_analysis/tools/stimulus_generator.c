#include <stdio.h>
#include <stdlib.h>
#include "../../../include/xv.h"

#define pi           	3.1415926535897932384626433832795029
#define SENOIDAL	0
#define TRIANGULAR	1
#define QUADRADA	2



int
main (int argc, char **argv)
{
	FILE *image_out;
	PICINFO picinfo;
	char *file_name;
	int i, x, y, value, stimulus_type;
	double omega, cycles;
	
	// Check parameters
	if (argc != 4)
	{
		printf ("Erro: Invalid number of parameters.\n Usage: stimulus_generator <image_out> <stimulus_type> <cycles>\n");
		printf ("0 - SENOIDAL\n1 - TRIANGULAR\n2 - QUADRADA\n");
		exit (1);
	}
	
	// Get parameters
	file_name = argv[1];
	stimulus_type = atoi (argv[2]); 
	cycles = atof (argv[3]);
	
	// Open image file to write
	if ((image_out = fopen (file_name, "w")) == NULL)
	{
		printf ("Could not create %s for writting\n", file_name);
		exit (1);
	}
	
	// Use greyscale color map
	for (i = 0; i < 256; i++)
		picinfo.r[i] = picinfo.g[i] = picinfo.b[i] = i;
		
	// Set the picinfo atributes
	picinfo.w = picinfo.normw = 256;
	picinfo.h = picinfo.normh = 256;
	picinfo.type = PIC8;
	picinfo.pic = (byte *) malloc (picinfo.w * picinfo.h * sizeof (byte));
	
	// Draw image
	omega = 2.0 * pi * cycles / (double) picinfo.w;
	switch (stimulus_type)
	{
		case SENOIDAL:
			for (x = 0; x < picinfo.w; x++)
			{
				value = (int) (127.5 * sin (omega * (double) x) + 127.5);
				//printf ("%d %d\n", x, value);
				for (y = 0; y < picinfo.h; y++)
					picinfo.pic[x + y * picinfo.w] = value;
			}
			break;
		case TRIANGULAR:
			for (x = 0; x < picinfo.w; x++)
			{
				value = (int) (255.0 * (asin (sin (omega * (double) x)) / pi) + 127.5);
				//printf ("%d %d\n", x, value);
				for (y = 0; y < picinfo.h; y++)
					picinfo.pic[x + y * picinfo.w] = value;
			}
			break;
		case QUADRADA:
			for (x = 0; x < picinfo.w; x++)
			{
				value = sin (omega * (double) x) >= 0.0 ? 255:0;
				//printf ("%d %d\n", x, value);
				for (y = 0; y < picinfo.h; y++)
					picinfo.pic[x + y * picinfo.w] = value;
			}
			break;
		default:
			printf ("Invalid image type\n");
			printf ("0 - SENOIDAL\n1 - TRIANGULAR\n2 - QUADRADA\n");
			exit (-1);
	}
	
	// Save image in file
	WriteBMP (image_out, picinfo.pic, picinfo.type, picinfo.w, picinfo.h, picinfo.r, picinfo.g, picinfo.b, 256, F_FULLCOLOR);
	fclose (image_out);
	
	return (0);
}
