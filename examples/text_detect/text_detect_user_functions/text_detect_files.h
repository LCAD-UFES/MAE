#ifndef TEXT_DETECT_FILES_H_
#define TEXT_DETECT_FILES_H_

#include <mae.h>

#define TO_INPUT  0
#define TO_OUTPUT 1

NEURON_OUTPUT
read_input(PARAM_LIST *pParamList);

NEURON_OUTPUT
read_output(PARAM_LIST *pParamList);

void
load_image(char * object_name, int index);

#endif /* TEXT_DETECT_FILES_H_ */
