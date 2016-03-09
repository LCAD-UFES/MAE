#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int  main(int argc, char **argv)
{
	IplImage *img = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	cvSaveImage(argv[2], img, 0);
	FILE *fout = fopen(argv[3], "w");
	FILE *fout2 = fopen(argv[4], "w");
	FILE *fout3 = fopen(argv[5], "w");

	fprintf(fout, "reset\nevaluate_only	1\n");
	fprintf(fout, "depth_map results/%s\n\n", argv[2]);
	fprintf(fout2,"verbose 2\n");
	fprintf(fout2, "cd ../%s\n", argv[6]);
	fprintf(fout2, "script ../Scripts/%s\n\n",argv[5]);
	fprintf(fout3, "input_params param_in.txt\n");
	fprintf(fout3, "script ../Scripts/%s\n\n",argv[3]);
	fclose(fout);
	fclose(fout2);
	fclose(fout3);
}
