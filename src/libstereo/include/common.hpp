/*! \mainpage 
*
* \section introduction Introduction
*
* Stereo System Module provides an API to development of stereo calibrated systems.
*
*/

#ifndef _COMMON_H
#define _COMMON_H

// Includes
#include "../../libtsai/cal_main.h"
#include "matrix.hpp"
#include <GL/glut.h>

// Definitions

// Macros

// Types
using namespace std;
using namespace math;
typedef matrix<double> TMatrix;
typedef matrix_error TMatrixError;
typedef struct camera_parameters TCameraParameters;
typedef struct calibration_constants TCalibrationConstants;
typedef enum {QUARTER_INCH, THIRD_INCH, HALF_INCH, TWO_THIRDS_INCH, ONE_INCH} TCCDFormat;
typedef enum {COPLANAR, COPLANAR_WITH_FULL_OPTIMIZATION, NONCOPLANAR, NONCOPLANAR_WITH_FULL_OPTIMIZATION} TCalibrationType;

class TModuleError
{
	private:
        const char* const data;

	public:
  	TModuleError(const char* const msg = 0) : data(msg) {}
	const char* const Message (void) {return data;}
};

// Exportable variables

// Prototypes


#endif
