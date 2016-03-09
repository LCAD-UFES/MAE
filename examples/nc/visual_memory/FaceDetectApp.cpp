#include "face_detect_api.h"

int main(int argc, char* argv[])
{
	LoadDetectionDevice();
	TrainNetwork(384, 288);
	char* name = getPersonName(1);
	printf("Name: %s\n", name);
	exit(0);
}
