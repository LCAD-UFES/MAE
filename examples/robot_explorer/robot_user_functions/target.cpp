#include "target.hpp"



// ----------------------------------------------------------------------
// TTarget::TTarget -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TTarget::TTarget (float fltOuterRadius, float fltWidth, float fltHeight, float fltDepth, int nNumberRings)
{	
	// Sets the attributes
	outerRadius = fltOuterRadius;
	width = fltWidth;
	height = fltHeight;
	depth = fltDepth;
	numberRings = nNumberRings;
	quadric = gluNewQuadric ();
	
	return;
}



// ----------------------------------------------------------------------
// TTarget::~TTarget -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TTarget::~TTarget (void)
{
	gluDeleteQuadric (quadric);

	return;
}



// ----------------------------------------------------------------------
// TTarget::SetPosition -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TTarget::SetPosition (float fltX, float fltY, float fltZ)
{
	position.x = fltX;
	position.y = fltY;
	position.z = fltZ;

	return;
}



// ----------------------------------------------------------------------
// TTarget::Draw -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TTarget::Draw (void)
{	
	float d, r;
	float fltRingWidth;
	int i;

	fltRingWidth =  outerRadius / (float) numberRings;
	
	glPushMatrix ();
	glTranslatef (position.x, position.y, position.z);

	glPushMatrix ();
	glColor3f (1.0f, 0.8235f, 0.4706f);
	glTranslatef (.0f, .0f, .5f * height);
	glScalef(depth, width, height);
	glutSolidCube(1.0);
	glPopMatrix ();
	
	glPushMatrix ();
	glColor3f (1.0f, 1.0f, 1.0f);
	glTranslatef (.0f, .5f * width, height + outerRadius);
	glRotatef (90.0f, 1.0f, .0f, .0f);
	gluCylinder (quadric, outerRadius, outerRadius, width, TARGET_PIPE_SLICES, TARGET_PIPE_STACKS);
        glPopMatrix ();
        
	for(d = -.5f * width; d <= .5f * width; d += width)
	{
		i = 0;
		glPushMatrix ();
	        glTranslatef (.0f, d, height + outerRadius);
                glRotatef (90.0f, 1.0f, .0f, .0f);
                for (r = .0f; r <= (outerRadius - fltRingWidth); r += fltRingWidth, i = !i)
		{
			glColor3f (1.0f, (float) i, (float) i);
			gluDisk (quadric, r, r + fltRingWidth, TARGET_PIPE_SLICES, TARGET_PIPE_STACKS);
		}
		glPopMatrix ();
	}
	
	glPopMatrix ();
	
	return;
}
