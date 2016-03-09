// Includes
#include "common.hpp"
#include "fis.h"
#include "fuzzy.hpp"



// Global variables
FIS *g_fis;
double **g_dataMatrix, **g_fisMatrix, **g_outputMatrix;
int g_fis_row_n, g_fis_col_n;
TModuleStatus g_eFuzzyStatus = UNINITIALIZED;



// ----------------------------------------------------------------------
// FuzzyInitialize -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int FuzzyInitialize (void)
{
        if (g_eFuzzyStatus == INITIALIZED)
                return (-1);
        g_eFuzzyStatus = INITIALIZED;
        
        // Obtain FIS matrix
	g_fisMatrix = returnFismatrix (FIS_FILE, &g_fis_row_n, &g_fis_col_n);

	// Create input matrix
	g_dataMatrix = (double **) fisCreateMatrix (1, FUZZY_INPUTS_NUMBER, sizeof (double));
	
	// Create output matrix
	g_outputMatrix = (double **) fisCreateMatrix (1, FUZZY_OUTPUTS_NUMBER, sizeof (double));
	
	// Build FIS data structure
	g_fis = (FIS *) calloc (1, sizeof (FIS));
	fisBuildFisNode (g_fis, g_fisMatrix, g_fis_col_n, MF_POINT_N);

	return (0);
}



// ----------------------------------------------------------------------
// FuzzyQuit -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int FuzzyQuit (void)
{
        if (g_eFuzzyStatus == UNINITIALIZED)
                return (-1);
        g_eFuzzyStatus = UNINITIALIZED;
        
        fisFreeFisNode(g_fis);
        fisFreeMatrix((void **) g_fisMatrix, g_fis_row_n);
        fisFreeMatrix((void **) g_dataMatrix, 1);
        fisFreeMatrix((void **) g_outputMatrix, 1);
        
        return (0);
}



// ----------------------------------------------------------------------
// FuzzyMachineForward -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int FuzzyMachineForward (double *p_dblInputVector, double *p_dblOutputVector)
{
        int i;

        if (g_eFuzzyStatus == UNINITIALIZED)
                return (-1);

        cout << "Input values:\nL\tC\tR\n";
	for (i = 0; i < FUZZY_INPUTS_NUMBER; i++)
	{
                g_dataMatrix[0][i] = p_dblInputVector[i];
                cout <<  g_dataMatrix[0][i] << "\t";
        }
	cout << endl;

	cout << "Getting the fuzzy control system outputs...\n";
	getFisOutput(g_dataMatrix[0], g_fis, g_outputMatrix[0]);

	cout << "Output values:\nAngle\tDistance\n";
	for (i = 0; i < FUZZY_OUTPUTS_NUMBER; i++)
	{
                p_dblOutputVector[i] = g_outputMatrix[0][i];
                cout <<  g_outputMatrix[0][i] << "\t";
        }
        cout << endl;
	
	return (0);
}
