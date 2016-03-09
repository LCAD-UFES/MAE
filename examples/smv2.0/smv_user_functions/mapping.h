#ifndef _MAPPING_H
#define _MAPPING_H

// Includes

// Definitions

// Macros

// Structs
struct _world_point
    { 
     	double x; 
     	double y;
	double z;
	
    }; 
typedef struct _world_point WORLD_POINT;

struct _image_coordinate
    { 
     	double x; 
     	double y;
    }; 
typedef struct _image_coordinate IMAGE_COORDINATE;

// Global Variables

// Prototypes
WORLD_POINT mapCalculateWorldPoint (IMAGE_COORDINATE leftPoint, 
				    IMAGE_COORDINATE rightPoint,
				    IMAGE_COORDINATE leftPrincipalPoint,
				    IMAGE_COORDINATE rightPrincipalPoint,
				    double dblCameraLeftFocus,
				    double dblCameraRightFocus,
				    double dblCameraDistance);

#endif
