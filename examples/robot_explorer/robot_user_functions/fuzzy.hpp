#ifndef _FUZZY_H
#define _FUZZY_H

// Definitions
#ifndef FUZZY_INPUTS_NUMBER
#define FUZZY_INPUTS_NUMBER   3
#endif

#ifndef FUZZY_OUTPUTS_NUMBER
#define FUZZY_OUTPUTS_NUMBER  2
#endif

#ifndef FIS_FILE
#define FIS_FILE              "./robot.fis"
#endif

// Macros

// Types

// Global Variables

// Prototypes

#ifdef __cplusplus
extern "C" {
#endif
int FuzzyInitialize (void);
int FuzzyQuit (void);
int FuzzyMachineForward (double *p_dblInputVector, double *p_dblOutputVector);
#ifdef __cplusplus
}
#endif

#endif
