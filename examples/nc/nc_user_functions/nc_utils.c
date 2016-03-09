#include "nc_utils.h"
#include <sys/stat.h>

int
file_exists(char *name)
{
   struct stat buf;
   return !stat(name, &buf);
}

void
set_input_image_file_name(INPUT_DESC *input)
{
	int i;
	FILE *path_file = (FILE *) NULL;

	// Open the path file
	if ((path_file = fopen (input->name, "r")) == (FILE *) NULL)
		Erro ("Cannot open input file: ", input->name, "");

	// Get the image file name
	fgets (input->image_file_name, 250, path_file);
	i = 0;
	while (isgraph (input->image_file_name[i]))
		i++;
	input->image_file_name[i] = '\0';

	// Close the path file
	fclose (path_file);
}
