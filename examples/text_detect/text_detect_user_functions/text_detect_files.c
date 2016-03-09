#include "text_detect_files.h"
#include "../text_detect.h"
#include "mae.h"


char*
get_image_name_from_file(char* index_file_name, int index_image)
{
	int i, count = 0;
	FILE *input_file;
	FILE *image_file_list;
	char *image_file_name;
	char image_file_list_name[256];

	if ((input_file = fopen(index_file_name, "r")) == NULL)
		Erro ("Cannot open input file: ", index_file_name, "");

	fgets (image_file_list_name, 256, input_file);

	i = 0;
	while (isgraph (image_file_list_name[i])) i++;
	image_file_list_name[i] = '\0';

	if ((image_file_list = fopen(image_file_list_name, "r")) == NULL)
		Erro ("Cannot open input file: ", image_file_list_name, "");

	image_file_name = (char*) malloc(256 * sizeof(char));

	while( fgets (image_file_name, 256, image_file_list) != NULL)
	{
		if (count == index_image)
		{
			i = 0;
			while (isgraph (image_file_name[i])) i++;
			image_file_name[i] = '\0';

			return image_file_name;
		}
		count++;
	}

	fclose(image_file_list);
	fclose(input_file);

	return NULL;
}

void
load_image(char * object_name, int index)
{
	char image_path[1024];
	char* image_name;

	image_name = get_image_name_from_file(object_name, index);

	if (image_name != NULL)
	{
		sprintf(image_path, "%s/%s", DATA_PATH, image_name);
		load_image_to_object(object_name, image_path);
		free(image_name);
	}

}

NEURON_OUTPUT
read_input(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	int index = pParamList->next->param.ival;

	load_image(in_text_detect.name, index);

	output.ival = 0;
	return (output);
}

NEURON_OUTPUT
read_output(PARAM_LIST *pParamList)
{
	NEURON_OUTPUT output;

	int index = pParamList->next->param.ival;

	load_image(nl_text_detect.name, index);

	output.ival = 0;
	return (output);
}

