#include "mapping.h"



/************************************************************************/
/* Name: 							        */
/* Description:						                */
/* Inputs: 	                    	                                */
/* Output: 							        */
/************************************************************************/

WORLD_POINT mapCalculateWorldPoint (IMAGE_COORDINATE leftPoint, 
				    IMAGE_COORDINATE rightPoint,
				    IMAGE_COORDINATE leftPrincipalPoint,
				    IMAGE_COORDINATE rightPrincipalPoint,
				    double dblCameraLeftFocus,
				    double dblCameraRightFocus,
				    double dblCameraDistance)
{
	WORLD_POINT worldPoint;
	
	if (leftPoint.x == rightPoint.x)
		worldPoint.x = worldPoint.y = worldPoint.z = 0.0;
	else
	{
		worldPoint.x = (dblCameraDistance / 2.0) * (dblCameraRightFocus * (leftPoint.x - leftPrincipalPoint.x) + dblCameraLeftFocus * (rightPoint.x - rightPrincipalPoint.x)) / (dblCameraRightFocus * (leftPoint.x - leftPrincipalPoint.x) - dblCameraLeftFocus * (rightPoint.x - rightPrincipalPoint.x));
		worldPoint.y = dblCameraDistance * dblCameraLeftFocus * (rightPoint.y - rightPrincipalPoint.y) / (dblCameraRightFocus * (leftPoint.x - leftPrincipalPoint.x) - dblCameraLeftFocus * (rightPoint.x - rightPrincipalPoint.x));
		worldPoint.z = dblCameraDistance * (dblCameraRightFocus * dblCameraLeftFocus) / (dblCameraRightFocus * (leftPoint.x - leftPrincipalPoint.x) - dblCameraLeftFocus * (rightPoint.x - rightPrincipalPoint.x));
	}
	
	return (worldPoint);
}
