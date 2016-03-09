// Includes
#include "common.hpp"
#include "robot.hpp"
#include "camera.hpp"
#include "terrain.hpp"
#include "target.hpp"
#include "simulator.hpp"

// Global Variables
int g_nWindowId;
int g_nViewMode = MONO_MODE;
TRobot *g_pRobot;
TCamera *g_pExternalCamera;
TTerrain *g_pTerrain;
TTarget *g_pTarget;
GLubyte *g_pLeftImage, *g_pRightImage;
int g_nLeftImageWidth, g_nLeftImageHeight, g_nRightImageWidth, g_nRightImageHeight;
TModuleStatus g_eSimulatorStatus = UNINITIALIZED;
bool g_bForceUpdate = false;

// Integer module parameters
GLint g_nSimulatorWindowWidth = 512; // [pix]
GLint g_nSimulatorWindowHeight = 512; // [pix]
GLfloat g_fltSimulatorAspectRatio = (GLfloat) g_nSimulatorWindowWidth / (GLfloat) g_nSimulatorWindowHeight; // []

// Real module parameters
GLfloat g_fltSimulatorFovy = 47.05f; // [deg]
GLfloat g_fltSimulatorFar = 100.0f; // [cm]
GLfloat g_fltSimulatorNear = .1f; // [cm]
GLfloat g_fltSimulatorFocus = 400.0f; // [pix]
GLfloat g_fltSimulatorFloor = 10.0f; // [cm]
GLfloat g_fltSimulatorCameraSeparation = 6.9f; // [cm]
GLfloat g_fltRobotRadius = 15.0f; // [cm]
GLfloat g_fltRobotHeight = 7.5f; // [cm]
GLfloat g_fltSimulatorTerrainSize = 250.0f; // [cm]

// ----------------------------------------------------------------------
// SimulatorSetParameteri - sets a module integer parameter
//
// Inputs: nParameter - the parameter
//	   nValue - the new value
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int SimulatorSetParameteri (int nParameter, int nValue)
{
        if (g_eSimulatorStatus == INITIALIZED)
                return (-1);
		
	switch (nParameter)
	{
		case SIMULATOR_WINDOW_WIDTH:
			g_nSimulatorWindowWidth = nValue;
			g_fltSimulatorAspectRatio = (GLfloat) g_nSimulatorWindowWidth / (GLfloat) g_nSimulatorWindowHeight; 
			break;	
		case SIMULATOR_WINDOW_HEIGHT:
			g_nSimulatorWindowHeight = nValue;
			g_fltSimulatorAspectRatio = (GLfloat) g_nSimulatorWindowWidth / (GLfloat) g_nSimulatorWindowHeight; 
			break;
		default:
			WARNING("Invalid parameter (SimulatorSetParameteri)", "", "");
			return (-1);
	}
	
        return (0);
}



// ----------------------------------------------------------------------
// SimulatorSetParameterf - sets a module real parameter
//
// Inputs: nParameter - the parameter
//	   fltValue - the new value
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int SimulatorSetParameterf (int nParameter, float fltValue)
{
        if (g_eSimulatorStatus == INITIALIZED)
                return (-1);
	
	switch (nParameter)
	{
		case SIMULATOR_FOVY:
			g_fltSimulatorFovy = fltValue;
			break;
		case SIMULATOR_FAR:
			g_fltSimulatorFar = fltValue;
			break;
		case SIMULATOR_NEAR:
			g_fltSimulatorNear = fltValue;
			break;	
		case SIMULATOR_FOCUS:
			g_fltSimulatorFocus = fltValue;
			break;
		case SIMULATOR_FLOOR:
			g_fltSimulatorFloor = fltValue;
			break;
		case SIMULATOR_CAMERA_SEPARATION:
			g_fltSimulatorCameraSeparation = fltValue;
			break;
		case SIMULATOR_ROBOT_RADIUS:
			g_fltRobotRadius = fltValue;
			break;
		case SIMULATOR_ROBOT_HEIGHT:
			g_fltRobotHeight = fltValue;
			break;
		case SIMULATOR_TERRAIN_SIZE:
			g_fltSimulatorTerrainSize = fltValue;
			break;
		default:
			WARNING("Invalid parameter (SimulatorSetParameterf)", "", "");
			return (-1);
	}
	
        return (0);
}

// ----------------------------------------------------------------------
// SimulatorGetImage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorGetImageFromScreen (GLubyte *pImage, int nImageWidth, int nImageHeight, GLsizei *p_nViewport)
{	
	int nWidth, nHeight;
	static int nPreviusWidth = 0, nPreviusHeight = 0;
	static GLubyte *pScreenPixels = (GLubyte *) NULL;
	
	nWidth = p_nViewport[2];
	nHeight = p_nViewport[3];
	
	if ((nWidth != nPreviusWidth) || (nHeight != nPreviusHeight))
	{
		delete pScreenPixels;
		if ((pScreenPixels = new GLubyte[3 * nWidth * nHeight]) == (GLubyte *) NULL)
		{
			Error ("Cannot allocate more memory", "", "");
			return (-1);
		}
		nPreviusWidth = nWidth;
		nPreviusHeight = nHeight;
	}
	
	glReadBuffer (GL_BACK);
	glEnable(GL_READ_BUFFER);
	glReadPixels(p_nViewport[0], p_nViewport[1], p_nViewport[2], p_nViewport[3], GL_RGB, GL_UNSIGNED_BYTE, pScreenPixels);
	glDisable(GL_READ_BUFFER);
	gluScaleImage (GL_RGB, nWidth, nHeight, GL_UNSIGNED_BYTE, pScreenPixels, nImageWidth, nImageHeight, GL_UNSIGNED_BYTE, pImage);

	return (0);
}



// ----------------------------------------------------------------------
// SimulatorMonoDisplay -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorMonoDisplay (void)
{
	float fltEyeX, fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ;
        GLsizei p_nViewport[4];
        
	// Draws the main screen
	glViewport (0, 0, (GLsizei) g_nSimulatorWindowWidth, (GLsizei) g_nSimulatorWindowHeight);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0, (GLsizei) g_nSimulatorWindowWidth, 0, (GLsizei) g_nSimulatorWindowHeight);	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glColor3ub (0, 0, 0);
    	glBegin (GL_LINES);
		glVertex2i ((GLsizei) g_nSimulatorWindowWidth / 2, (GLsizei) g_nSimulatorWindowHeight / 2);
		glVertex2i ((GLsizei) g_nSimulatorWindowWidth / 2, (GLsizei) g_nSimulatorWindowHeight);
		glVertex2i (0, (GLsizei) g_nSimulatorWindowHeight / 2);
		glVertex2i ((GLsizei) g_nSimulatorWindowWidth, (GLsizei) g_nSimulatorWindowHeight / 2);
	glEnd();

	// Draws the left view
	glViewport (0, (GLsizei) (g_nSimulatorWindowHeight + 1) / 2, (GLsizei) (g_nSimulatorWindowWidth + 1) / 2, (GLsizei) (g_nSimulatorWindowHeight + 1) / 2);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (g_fltSimulatorFovy, (float) g_nSimulatorWindowWidth / (float) g_nSimulatorWindowHeight, g_fltSimulatorNear, g_fltSimulatorFar);	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	fltEyeX = g_pRobot->position.x + g_pRobot->leftEye->position.x * COS (g_pRobot->rotation.z * M_PI / 180.0) - g_pRobot->leftEye->position.y * SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeY = g_pRobot->position.y + g_pRobot->leftEye->position.x * SIN (g_pRobot->rotation.z * M_PI / 180.0) + g_pRobot->leftEye->position.y * COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeZ = g_pRobot->position.z + g_pRobot->leftEye->position.z;
	fltCenterX = fltEyeX + COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterY = fltEyeY + SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterZ = fltEyeZ;

	gluLookAt (fltEyeX, fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ, .0f, .0f, 1.0f);

	g_pRobot->Draw ();
	g_pTerrain->Draw ();
	//g_pTarget->Draw ();
	
	if ((g_pLeftImage) != NULL && g_bForceUpdate)
	{
                glGetIntegerv (GL_VIEWPORT, p_nViewport);
                SimulatorGetImageFromScreen (g_pLeftImage, g_nLeftImageWidth, g_nLeftImageHeight, p_nViewport);
        }
        
	// Draws the right view
	glViewport ((GLsizei) (g_nSimulatorWindowWidth + 1) / 2, (GLsizei) (g_nSimulatorWindowHeight + 1) / 2, (GLsizei) (g_nSimulatorWindowWidth + 1) / 2, (GLsizei) (g_nSimulatorWindowHeight + 1) / 2);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (g_fltSimulatorFovy, (float) g_nSimulatorWindowWidth / (float) g_nSimulatorWindowHeight, g_fltSimulatorNear, g_fltSimulatorFar);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	fltEyeX = g_pRobot->position.x + g_pRobot->rightEye->position.x * COS (g_pRobot->rotation.z * M_PI / 180.0) - g_pRobot->rightEye->position.y * SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeY = g_pRobot->position.y + g_pRobot->rightEye->position.x * SIN (g_pRobot->rotation.z * M_PI / 180.0) + g_pRobot->rightEye->position.y * COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeZ = g_pRobot->position.z + g_pRobot->rightEye->position.z;
	fltCenterX = fltEyeX + COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterY = fltEyeY + SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterZ = fltEyeZ;
	
	gluLookAt (fltEyeX,fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ, .0f, .0f, 1.0f);

	g_pRobot->Draw ();
	g_pTerrain->Draw ();
	//g_pTarget->Draw ();

	if ((g_pRightImage != NULL) && g_bForceUpdate)
	{
                glGetIntegerv (GL_VIEWPORT, p_nViewport);
                SimulatorGetImageFromScreen (g_pRightImage, g_nRightImageWidth, g_nRightImageHeight, p_nViewport);
        }
	// Draws the external view
	glViewport (0, 0, (GLsizei) g_nSimulatorWindowWidth, (GLsizei) (g_nSimulatorWindowHeight + 1) / 2);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (FOVY, 2.0f * (float) g_nSimulatorWindowWidth / (float) g_nSimulatorWindowHeight, NEAR, FAR);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	fltCenterX = g_pRobot->position.x;
	fltCenterY = g_pRobot->position.y;
	fltCenterZ = g_pRobot->position.z;
	fltEyeX = fltCenterX + g_pExternalCamera->position.x;
	fltEyeY = fltCenterY + g_pExternalCamera->position.y;
	fltEyeZ = fltCenterZ + g_pExternalCamera->position.z;

	gluLookAt (fltEyeX, fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ, .0f, .0f, 1.0f);

	g_pRobot->Draw ();
	g_pTerrain->Draw ();
	//g_pTarget->Draw ();
	
	glutSwapBuffers();
	
	return;
}



// ----------------------------------------------------------------------
// SimulatorStereoDisplay -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorStereoDisplay (void)
{
	float fltEyeX, fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ;

	glDrawBuffer(GL_BACK_LEFT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport (0, 0, (GLsizei) g_nSimulatorWindowWidth, (GLsizei) g_nSimulatorWindowHeight);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (g_fltSimulatorFovy, (float) g_nSimulatorWindowWidth / (float) g_nSimulatorWindowHeight, g_fltSimulatorNear, g_fltSimulatorFar);	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glDrawBuffer (GL_BACK_LEFT);
	fltEyeX = g_pRobot->position.x + g_pRobot->leftEye->position.z * SIN (g_pRobot->rotation.z * M_PI / 180.0) + g_pRobot->leftEye->position.x * COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeY = g_pRobot->position.y + g_pRobot->leftEye->position.y;
	fltEyeZ = g_pRobot->position.z + g_pRobot->leftEye->position.z * COS (g_pRobot->rotation.z * M_PI / 180.0) - g_pRobot->leftEye->position.x * SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterX = fltEyeX + SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterY = fltEyeY;
	fltCenterZ = fltEyeZ + COS (g_pRobot->rotation.z * M_PI / 180.0);

	gluLookAt (fltEyeX,fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ, 0.0, 1.0, 0.0);

	g_pTerrain->Draw ();
	g_pTarget->Draw ();
	
	glDrawBuffer (GL_BACK_RIGHT);
	fltEyeX = g_pRobot->position.x + g_pRobot->rightEye->position.z * SIN (g_pRobot->rotation.z * M_PI / 180.0) + g_pRobot->rightEye->position.x * COS (g_pRobot->rotation.z * M_PI / 180.0);
	fltEyeY = g_pRobot->position.y + g_pRobot->rightEye->position.y;
	fltEyeZ = g_pRobot->position.z + g_pRobot->rightEye->position.z * COS (g_pRobot->rotation.z * M_PI / 180.0) - g_pRobot->rightEye->position.x * SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterX = fltEyeX + SIN (g_pRobot->rotation.z * M_PI / 180.0);
	fltCenterY = fltEyeY;
	fltCenterZ = fltEyeZ + COS (g_pRobot->rotation.z * M_PI / 180.0);

	gluLookAt (fltEyeX,fltEyeY, fltEyeZ, fltCenterX, fltCenterY, fltCenterZ, 0.0, 1.0, 0.0);

	g_pTerrain->Draw ();
	g_pTarget->Draw ();

	glutSwapBuffers();
	
	return;
}



// ----------------------------------------------------------------------
// SimulatorMouse -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorMouse (int button, int state, int x, int y)
{
		
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
	{
	
	}
	
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
	{
			
	}
		
	return;
}



// ----------------------------------------------------------------------
// SimulatorReshape -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorReshape (int width, int height)
{
	g_nSimulatorWindowWidth = width;
	g_nSimulatorWindowHeight = height;

	glutPostRedisplay ();
	
	return;
}



// ----------------------------------------------------------------------
// SimulatorMotion -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorMotion (int x, int y)
{
	
	glutPostRedisplay ();

	return;
}



// ----------------------------------------------------------------------
// SimulatorKeyboard -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{	
		// Pans the external camera left
		case '4':
			g_pExternalCamera->RotateAroundFixationPoint (-CAMERA_DELTA_ANGLE, .0f);
			break;
		// Pans the external camera right
		case '6': 
			g_pExternalCamera->RotateAroundFixationPoint (CAMERA_DELTA_ANGLE, .0f);
			break;
		// Tilts the external camera down
		case '2':
			g_pExternalCamera->RotateAroundFixationPoint (.0f, -CAMERA_DELTA_ANGLE);
			break;
		// Tilts the external camera up
		case '8':
			g_pExternalCamera->RotateAroundFixationPoint (.0f, CAMERA_DELTA_ANGLE);
			break;
		// Zoom out the external camera
		case '-':
			g_pExternalCamera->Zoom (CAMERA_ZOOM_STEP);
			break;
		// Zoom in the external camera
		case '+':
			g_pExternalCamera->Zoom (1.0f / CAMERA_ZOOM_STEP);
			break;
		// Print robot odometry
		case 'H':
		case 'h':
			cout << "Simulator:" << endl;
			cout << "Robot position = (" << g_pRobot->position.x << ", " << g_pRobot->position.y << ")" << endl;
			cout << "Robot orientation = " << g_pRobot->rotation.z << endl;
			break;
		// Quits simulator
		case 'Q':
		case 'q':
                        SimulatorQuit ();
			return;
	 	default:
                        WARNING("Invalid key pressed '", key, "' (SimulatorKeyboard).");
	}
	
	glutPostRedisplay ();
	
	return;
}



// ----------------------------------------------------------------------
// SimulatorSpecialKey -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorSpecialKey (int key, int x, int y)
{
	switch (key)
	{	
		// Controls the robot
		case GLUT_KEY_LEFT:
			g_pRobot->Rotate (ROBOT_ANGLE_STEP);
			break;
		case GLUT_KEY_RIGHT:
			g_pRobot->Rotate (-ROBOT_ANGLE_STEP);
			break;
		case GLUT_KEY_DOWN:
			g_pRobot->Move (-ROBOT_MOVE_STEP);
			break;
		case GLUT_KEY_UP:
			g_pRobot->Move (ROBOT_MOVE_STEP);
			break;
	}
	
	glutPostRedisplay ();

	return;
}



// ----------------------------------------------------------------------
// SimulatorLinkLeftImage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorLinkLeftImage (GLubyte *pLeftImage, int nImageWidth, int nImageHeight)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);

        g_pLeftImage = pLeftImage;
        g_nLeftImageWidth = nImageWidth;
        g_nLeftImageHeight = nImageHeight;

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorUnlinkLeftImage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorUnlinkLeftImage (void)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);

        g_pLeftImage = NULL;

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorLinkRightImage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorLinkRightImage (GLubyte *pRightImage, int nImageWidth, int nImageHeight)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);

        g_pRightImage = pRightImage;
        g_nRightImageWidth = nImageWidth;
        g_nRightImageHeight = nImageHeight;

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorUnlinkRightImage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorUnlinkRightImage (void)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);

        g_pRightImage = NULL;

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorForceUpdate -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorForceUpdate (void)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);
	
	g_bForceUpdate = true;
	glutSetWindow (g_nWindowId);
	SimulatorMonoDisplay ();
	g_bForceUpdate = false;

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorRobotTurn - turns the robot smoothly
//
// Inputs: fltAngle - the angle [deg]
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int SimulatorRobotTurn (float fltAngle)
{	
	if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);
	
	float fltEndAlpha;
	g_pRobot->GetRotation (&fltEndAlpha);
	fltEndAlpha += fltAngle;
	
	for (int i = 0; i < (int) (fabs (fltAngle) / (float) ROBOT_ANGLE_STEP + .5f); i++)
        {
                g_pRobot->Rotate ((fltAngle / fabs (fltAngle)) * ROBOT_ANGLE_STEP);
                glutSetWindow (g_nWindowId);
                SimulatorMonoDisplay ();
        }

	g_pRobot->SetRotation (fltEndAlpha);
 	glutSetWindow (g_nWindowId);
        SimulatorMonoDisplay ();

        return (0);
}



// ----------------------------------------------------------------------
// SimulatorRobotMove - moves the robot smoothly
//
// Inputs: fltStep - the step [cm]
//
// Outputs:0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int SimulatorRobotMove (float fltStep)
{
	if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);
        
	float fltStartX, fltStartY, fltEndX, fltEndY;
	g_pRobot->GetPosition (&fltStartX, &fltStartY);
        g_pRobot->Move (fltStep);
	g_pRobot->GetPosition (&fltEndX, &fltEndY);
	g_pRobot->SetPosition (fltStartX, fltStartY);

        for (int i = 0; i < (int) (fabs (fltStep) / (float) ROBOT_MOVE_STEP + .5f); i++)
        {
                g_pRobot->Move ((fltStep / fabs (fltStep)) * ROBOT_MOVE_STEP);
                glutSetWindow (g_nWindowId);
                SimulatorMonoDisplay ();
        }
	
	g_pRobot->SetPosition (fltEndX, fltEndY);
	glutSetWindow (g_nWindowId);
	SimulatorMonoDisplay ();
        
	return (0);
}



// ----------------------------------------------------------------------
// SimulatorInitialize -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorInitialize (void)
{
	GLfloat materialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat materialShininess[] = {70.0f};
	GLfloat lightAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat lightPosition[] = {.0f, 50.0f, .0f};
	GLfloat p_fltViewUpDirection[] = {.0f, .0f, 1.0f};
	
	if (g_eSimulatorStatus == INITIALIZED)
                return (-1);
        g_eSimulatorStatus = INITIALIZED;
        
	if (g_nViewMode == STEREO_MODE)
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);
	else
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	glutInitWindowSize (g_nSimulatorWindowWidth, g_nSimulatorWindowHeight);
	glutInitWindowPosition (100, 100);
       	g_nWindowId = glutCreateWindow (WINDOW_TITLE);
	
	glClearColor (.5f, .7f, 1.0f, 1.0f);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT,GL_NICEST);
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
	glLightfv (GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	
	if (g_nViewMode == STEREO_MODE)
		glutDisplayFunc (SimulatorStereoDisplay);
	else
		glutDisplayFunc (SimulatorMonoDisplay);
		
	glutMouseFunc (SimulatorMouse);
	glutMotionFunc (SimulatorMotion);
	glutKeyboardFunc (SimulatorKeyboard);
	glutSpecialFunc (SimulatorSpecialKey);
	glutReshapeFunc (SimulatorReshape);

	if ((g_pRobot = new TRobot (g_fltRobotRadius, g_fltRobotHeight, g_fltSimulatorCameraSeparation, -g_fltSimulatorFloor)) == NULL)
	{
		Error ("Cannot create robot (SimulatorInitialize)", "", "");
		return (-1);
	}
	
	if ((g_pExternalCamera = new TCamera ()) == NULL)
	{
		Error ("Cannot create external camera (SimulatorInitialize)", "", "");
		return (-1);
	}
	
	g_pExternalCamera->SetFixationPoint (.0f, .0f, .0f);
	g_pExternalCamera->SetPosition (.0f, -500.0f, 500.0f);
	g_pExternalCamera->SetViewUpDirection (p_fltViewUpDirection);
	
	if ((g_pTerrain = new TTerrain (g_fltSimulatorTerrainSize)) == NULL)
	{
		Error ("Cannot create terrain (SimulatorInitialize)", "", "");
		return (-1);
	}
	
	if ((g_pTarget = new TTarget (5.0f, 1.0f, 10.0f, 1.0f, 3)) == NULL)
	{
		Error ("Cannot create target (SimulatorInitialize)", "", "");
		return (-1);
	}
	
	g_pTarget->SetPosition (.0f, 10.0f, .0f);

        g_pLeftImage = g_pRightImage = NULL;
        
	return (0);
}



// ----------------------------------------------------------------------
// SimulatorQuit -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int SimulatorQuit (void)
{
        if (g_eSimulatorStatus == UNINITIALIZED)
                return (-1);
        g_eSimulatorStatus = UNINITIALIZED;
        
        glutDestroyWindow (g_nWindowId);
        
        delete g_pRobot;
	delete g_pExternalCamera;
	delete g_pTerrain;
	delete g_pTarget;
	
	return (0);
}



#ifdef _TESTING_SIMULATOR
// ----------------------------------------------------------------------
// SimulatorShowUsageMessage -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorShowUsageMessage (void)
{
	cout << "Usage: ./simulator [option]\nOptions:\n";
	cout << "\t-h\t\tShow this help.\n";
	cout << "\t-v\t\tShow version information.\n";
	cout << "\t-s\t\tActivate the stereo mode.\n";
	exit (-1);
}



// ----------------------------------------------------------------------
// SimulatorParseArguments -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void SimulatorParseArguments (int argc, char **argv)
{
	if ((argc != 1) && (argc != 2))
		SimulatorShowUsageMessage ();
	
	if (argc == 2)
	{
		if (strcmp ("-s", argv[1]) == 0)
			g_nViewMode = STEREO_MODE;
		else if (strcmp ("-h", argv[1]) == 0)
			SimulatorShowUsageMessage ();
		else if (strcmp ("-v", argv[1]) == 0)
		{
			MESSAGE ("Simulator version '", SIMULATOR_VERSION, "'");
			exit (0);
		}
		else
		{
			Error ("Invalid input argument '", argv[1], "'");
			SimulatorShowUsageMessage ();
		}
	}
			
	return;
}



// ----------------------------------------------------------------------
// main -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int main (int argc, char **argv)
{
	// Parse the command line arguments
	SimulatorParseArguments (argc, argv);
	
	glutInit (&argc, argv);
	
	if (SimulatorInitialize ())
	{
		Error ("Cannot initialize the simulator (main)", "", "");
		exit (-1);
	}
	
	glutMainLoop ();
	
	return (0);
}
#endif
