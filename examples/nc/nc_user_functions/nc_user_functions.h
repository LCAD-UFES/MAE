#include "../nc.h"

struct _data_set
{
	char *input_name;
	char **input_sentences;
	int input_index;
	int num_input_sentences;
};

typedef struct _data_set DATA_SET;
