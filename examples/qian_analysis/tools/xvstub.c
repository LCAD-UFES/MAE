#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 0;

char *
BaseName (char *fname)
{
	return (fname);
}

void
WaitCursor (void)
{
}

FILE *
xv_fopen (char *name, char *mode)
{
	return (fopen (name, mode));
}

void
FatalError (char *mens)
{
	fprintf (stderr, "Fatal Error: %s\n", mens);
	exit (1);
}

void
SetISTR (int caco, char *format, char *par1, char *par2)
{
    char mens[256];

	strcpy (mens, format);
	strcat (mens, "\n");	
	fprintf (stderr, mens, par1, par2);
}
