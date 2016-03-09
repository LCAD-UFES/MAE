#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define LEFT_CALIBRATION_DATA_FILE	"LeftCalibrationData.dat"
#define RIGHT_CALIBRATION_DATA_FILE	"RightCalibrationData.dat"
#define X_GRID_STEP			20.0f
#define Y_GRID_STEP			20.0f
#define Z_GRID_STEP			19.8333f

#define X_OFFSET			1000.0f
#define Y_OFFSET			500.0f
#define Z_OFFSET			500.0f

int main (int argc, char **argv)
{
	int x, y, z, nx, ny, nz;
	float X, Y, Z;
	float fltLeftAlpha, fltRightAlpha, fltLeftBeta, fltRightBeta, fltDistance;
	FILE *pDataFile = NULL;
	

	if ((argc != 4) && (argc != 9))
	{
		printf ("Usage: ./make_target <points_x> <points_y> <points_z> [<distance (mm)> <right_angle (deg)> <left_angle (deg)> <right_aperture (deg)> <left_aperture (deg)>]\n");
		return (-1);
	}
	
	nx = atoi (argv[1]);
	ny = atoi (argv[2]);
	nz = atoi (argv[3]);

	if (argc == 9)
	{
		fltDistance   = atof (argv[4]);	 		 // [mm] 
		fltRightAlpha = -M_PI / 180.0f * atof (argv[5]); // [rad]
		fltLeftAlpha  = -M_PI / 180.0f * atof (argv[6]); // [rad]
		fltRightBeta  = -M_PI / 180.0f * atof (argv[7]); // [rad]
		fltLeftBeta   = -M_PI / 180.0f * atof (argv[8]); // [rad]	
	}
	else
	{
		fltDistance   = 500.0;        // [mm]
		fltRightAlpha = -M_PI / 4.0f; // [rad]
		fltLeftAlpha  = -M_PI / 4.0f; // [rad]
		fltRightBeta  = -M_PI / 2.0f; // [rad]
		fltLeftBeta   = -M_PI / 2.0f; // [rad]
	}
	
	if ((pDataFile = fopen (RIGHT_CALIBRATION_DATA_FILE, "w")) == NULL)
	{
		printf ("Error: Cannot open file '%s'\n", RIGHT_CALIBRATION_DATA_FILE);
		return (-1);
	}

	fprintf (pDataFile, "%d\n", (nx + ny + 1) * nz);

	for (Z = .0f;  Z < Z_GRID_STEP * (float) nz; Z += Z_GRID_STEP)
	{
		for (X = X_GRID_STEP * (float) nx, Y = .0f;  X > .0f ; X -= X_GRID_STEP)
			fprintf (pDataFile, "%f %f %f\n", X * cos (fltRightAlpha) + Y * sin (fltRightAlpha) + X_OFFSET, -X * sin (fltRightAlpha) + Y * cos (fltRightAlpha) + Y_OFFSET, Z + Z_OFFSET);
		
		for (X = .0f, Y = .0f;  Y <= Y_GRID_STEP * (float) ny; Y += Y_GRID_STEP)
			fprintf (pDataFile, "%f %f %f\n", X * cos (fltRightAlpha + fltRightBeta + M_PI/2.0f) + Y * sin (fltRightAlpha + fltRightBeta + M_PI/2.0f) + X_OFFSET, -X * sin (fltRightAlpha + fltRightBeta + M_PI/2.0f) + Y * cos (fltRightAlpha + fltRightBeta + M_PI/2.0f) + Y_OFFSET, Z + Z_OFFSET);
		fprintf (pDataFile, "\n");
	}
	
	if (fclose (pDataFile))
	{
		printf ("Error: Cannot close file '%s'\n", RIGHT_CALIBRATION_DATA_FILE);
		return (-1);
	}

	if ((pDataFile = fopen (LEFT_CALIBRATION_DATA_FILE, "w")) == NULL)
	{
		printf ("Error: Cannot open file '%s'\n", LEFT_CALIBRATION_DATA_FILE);
		return (-1);
	}

	fprintf (pDataFile, "%d\n", (nx + ny + 1) * nz);

	for (Z = .0f;  Z < Z_GRID_STEP * (float) nz; Z += Z_GRID_STEP)
	{
		for (X = X_GRID_STEP * (float) nx, Y = .0f;  X > .0f ; X -= X_GRID_STEP)
			fprintf (pDataFile, "%f %f %f\n", X * cos (fltLeftAlpha) + Y * sin (fltLeftAlpha) + fltDistance + X_OFFSET, -X * sin (fltLeftAlpha) + Y * cos (fltLeftAlpha) + Y_OFFSET, Z + Z_OFFSET);
		
		for (X = .0f, Y = .0f;  Y <= Y_GRID_STEP * (float) ny; Y += Y_GRID_STEP)
			fprintf (pDataFile, "%f %f %f\n", X * cos (fltLeftAlpha + fltLeftBeta + M_PI/2.0f) + Y * sin (fltLeftAlpha + fltLeftBeta + M_PI/2.0f) + fltDistance + X_OFFSET, -X * sin (fltLeftAlpha + fltLeftBeta + M_PI/2.0f) + Y * cos (fltLeftAlpha + fltLeftBeta + M_PI/2.0f) + Y_OFFSET, Z + Z_OFFSET);
		fprintf (pDataFile, "\n");
	}
	
	if (fclose (pDataFile))
	{
		printf ("Error: Cannot close file '%s'\n", LEFT_CALIBRATION_DATA_FILE);
		return (-1);
	}

	return (0);
}
