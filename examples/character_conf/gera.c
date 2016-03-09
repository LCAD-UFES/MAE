#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE *fd;
	int cont;
	
	fd = fopen ("characterTrain.cml","r+");
	fseek(fd,0,SEEK_END);
	
	for (cont =0;cont < 59997;cont++)
	{
		fprintf(fd,"%s\n","move character to 12, 10;");
		fprintf(fd,"%s\n","draw char_discr_out based on character move;");
		fprintf(fd,"%s\n","train network;");
	
	} 
	
	fclose(fd);
	
	return 0;
}
