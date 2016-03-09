#include "robot.hpp"



// ----------------------------------------------------------------------
// TRobot::TRobot -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TRobot::TRobot (float fltRadius, float fltHeight, float fltCameraSeparation, float fltCameraHeight)
{
	// Sets the attributes
	radius = fltRadius;
	height = fltHeight;
	quadric = gluNewQuadric ();
	
	// Sets the state variables
	position.x = .0f;
	position.y = .0f;
	position.z = .0f;
	rotation.x = .0f;
	rotation.y = .0f;
	rotation.z = 90.0f;
	
	if ((leftEye = new TCamera ()) == NULL)
	{
		Error ("Error: cannot create left camera (TRobot::TRobot)", "", "");
		return;
	}
	
	leftEye->SetPosition (.75f * radius, .5f * fltCameraSeparation, fltCameraHeight);
	
	if ((rightEye = new TCamera ()) == NULL)
	{
		Error ("Cannot create right camera (TRobot::TRobot)", "", "");
		return;
	}
	
	rightEye->SetPosition (.75f * radius, -.5f * fltCameraSeparation, fltCameraHeight);
	
	return;
}



// ----------------------------------------------------------------------
// TRobot::~TRobot -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TRobot::~TRobot (void)
{
	delete leftEye;
	delete rightEye;
	
	return;
}



// ----------------------------------------------------------------------
// TRobot::Move - moves the robot forward
//
// Inputs: fltStep - the step [cm]
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TRobot::Move (float fltStep)
{
	position.x += fltStep * COS (rotation.z * M_PI / 180.0f);
	position.y += fltStep * SIN (rotation.z * M_PI / 180.0f);

        return (true);
}



// ----------------------------------------------------------------------
// TRobot::Rotate - turns the robot 
//
// Inputs: fltAngle - the angle [deg]
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TRobot::Rotate (float fltAngle)
{
	rotation.z += fltAngle;

	return (true);
}



// ----------------------------------------------------------------------
// TRobot::Draw -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TRobot::Draw (void)
{
	glPushMatrix ();

	glTranslatef (position.x, position.y, position.z);
	glRotatef (rotation.z,  .0f,  .0f, 1.0f);

        glPushMatrix ();
	glColor3f (1.0f, .0f, .0f);
	glTranslatef (.0f, .0f, .25f * height);
	gluCylinder (quadric, radius, radius, height, 20, 20);
	glPopMatrix ();
	
	glPushMatrix ();
	glColor3f (.4f, .0f, .0f);
	glTranslatef (.0f, .0f, 1.25f * height);
	gluDisk (quadric, .0f, radius, 20, 20);
        glPopMatrix ();
        
        glPushMatrix ();
	glColor3f (.0f, .0f, .0f);
	glTranslatef (.75f * radius, .0f, .125f * height);
	glutSolidSphere (.125f * height, 20, 20);
	glPopMatrix ();
	
        glPushMatrix ();
	glColor3f (.0f, .0f, .0f);
	glTranslatef (-.25f * radius, -.75f * radius + .25f * height, .25f * height);
	glRotatef (90.0f, 1.0f, .0f, .0f);
	gluCylinder (quadric, .25f * height, .25f * height, .25f * radius, 20, 20);
        glColor3f (.1f, .1f, .1f);
        glTranslatef (.0f, .0f, .25f * radius);
        gluDisk (quadric, .0f, .25f * height, 20, 20);
	glPopMatrix ();
	
	glPushMatrix ();
	glColor3f (.0f, .0f, .0f);
	glTranslatef (-.25f * radius, .75f * radius + .25f * height, .25f * height);
        glRotatef (90.0f, 1.0f, .0f, .0f);
        gluCylinder (quadric, .25f * height, .25f * height, .25f * radius, 20, 20);
        glColor3f (.1f, .1f, .1f);
        gluDisk (quadric, .0f, .25f * height, 20, 20);
	glPopMatrix ();
	
	leftEye->Draw ();
	rightEye->Draw ();

	glPopMatrix ();
	
	return;
}
