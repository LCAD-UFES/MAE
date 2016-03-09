#include "convert.hpp"



// ----------------------------------------------------------------------
// ConvertRect2Sphe - converts from rectangle to sphere coordinates
//
// Inputs: (p_fltAlfa, p_fltBeta, p_fltDistance) - the point sphere coordinates to be retorned by reference
//         (fltX, fltY, fltZ) - the point rectangle coordinates
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ConvertRect2Sphe (float *p_fltAlfa, float *p_fltBeta, float *p_fltDistance, float fltX, float fltY, float fltZ)
{
        *p_fltAlfa = (fltX > .0f) ? ATAN(fltY / fltX) :
                     (fltX < .0f) ? ((fltY >= .0f) ? ATAN(fltY / fltX) + M_PI : ATAN(fltY / fltX) - M_PI) :
                     ((fltY > .0f) ? .5f * M_PI : -.5f * M_PI);
        *p_fltBeta = ((fltX == .0f) && (fltY == .0f)) ? ((fltZ > .0f) ? .5f * M_PI : -.5f * M_PI) :
                     ATAN(fltZ / SQRT(fltX * fltX + fltY * fltY));
        *p_fltDistance = SQRT(fltX * fltX + fltY * fltY + fltZ * fltZ);

        return (0);
}



// ----------------------------------------------------------------------
// ConvertSphe2Rect - converts from sphere to rectangle coordinates
//
// Inputs: (p_fltX, p_fltY, p_fltZ) - the point rectangle coordinates to be retorned by reference
//         (fltAlfa, fltBeta, fltDistance) - the point sphere coordinates
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ConvertSphe2Rect (float *p_fltX, float *p_fltY, float *p_fltZ, float fltAlfa, float fltBeta, float fltDistance)
{
        float aux;
        
        *p_fltZ = fltDistance * SIN(fltBeta);
        aux = fltDistance * COS(fltBeta);
        *p_fltX = aux * COS(fltAlfa);
        *p_fltY = aux * SIN(fltAlfa);
        
        return (0);
}



// ----------------------------------------------------------------------
// CalculateWorldPoint -
//
// Inputs: 
//
// Outputs: none
// ----------------------------------------------------------------------

void CalculateWorldPoint (float *X, float *Y, float *Z, float xl, float yl, float xr, float yr, float F, float B)
{
	*X = B * (xr + xl) / (2.0f * (xl - xr));
	*Y = B * yr / (xl - xr);
	*Z = F * B / (xl - xr);

        return;
}
