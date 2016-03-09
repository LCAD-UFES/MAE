#ifndef _SMV_USER_FUNCTIONS_H
#define _SMV_USER_FUNCTIONS_H


#define READ_FROM_IMAGE_FILE			0
#define PASS_BY_PARAMETER			1
#define CAPTURED_FROM_CAMERA			2
#define CAPTURED_FROM_IP_ADDRESS		3

int inputType;
int image_type;

SHAPE_MAP shape_map;

WORLD_POINT calculateWorldPoint ();
void generateShape ();
XY_PAIR calculateVergencePoint (int vergence_type, XY_PAIR dominantEyePoint);

#endif
