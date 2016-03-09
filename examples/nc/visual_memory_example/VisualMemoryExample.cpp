#include <stdio.h>
#include <stdlib.h>

#include "face_detect_api.cpp"
//#include "face_recog_api.h"

int main(int argc, char* argv[])
{
	char* name = getPersonName(1);
	printf("Name: %s\n", name);
	exit(0);
}

