// Includes
#include "map.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "viewer.hpp"
#include "terrain.hpp"

// Global Variables
int g_nViewerWindowId;
int g_nViewerViewMode= EXTERNAL_VIEW;
bool g_bAutoSwapping = false;
TMap *g_pMap;
TCamera *g_pViewerExternalCamera;
TModuleStatus g_eViewerStatus = UNINITIALIZED;
bool g_bShowFloorPlane = false;
int g_nOldX = 0, g_nOldY = 0;

// Integer module parameters
GLint g_nViewerWindowWidth = 512; // [pix]
GLint g_nViewerWindowHeight = 512; // [pix]
GLint g_nMapWidth = 1024; // [pix]
GLint g_nMapHeight = 512; // [pix]
GLint g_nImageWidth = 256; // [pix]
GLint g_nImageHeight = 256; // [pix]
GLfloat g_fltAspectRatio = (GLfloat) g_nImageWidth / (GLfloat) g_nImageHeight; // []
GLint g_nMapLayersNumber = 2;
GLint g_nMapEspheresNumber = 10;

// Real module parameters
GLfloat g_fltFovy = 47.05f; // [deg]
GLfloat g_fltFar = 100.0f; // [cm]
GLfloat g_fltNear = .1f; // [cm]
GLfloat g_fltFocus = 400.0f; // [pix]
GLfloat g_fltFloor = 10.0f; // [cm]
GLfloat g_fltViewerRobotRadius = 15.0f; // [cm]
GLfloat g_fltViewerTerrainSize = 250.0f; // [cm]

// ----------------------------------------------------------------------
// ViewerSetParameteri - sets a module integer parameter
//
// Inputs: nParameter - the parameter
//	   nValue - the new value
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerSetParameteri (int nParameter, int nValue)
{
         if (g_eViewerStatus == INITIALIZED)
                return (-1);
		
	switch (nParameter)
	{
		case VIEWER_IMAGE_WIDTH:
			g_nImageWidth = nValue;
			g_fltAspectRatio = (GLfloat) g_nImageWidth / (GLfloat) g_nImageHeight; // []
			break;
		case VIEWER_IMAGE_HEIGHT:
			g_nImageHeight = nValue;
			g_fltAspectRatio = (GLfloat) g_nImageWidth / (GLfloat) g_nImageHeight; // []
			break;
		case VIEWER_MAP_WIDTH:
			g_nMapWidth = nValue;
			break;	
		case VIEWER_MAP_HEIGHT:
			g_nMapHeight = nValue;
			break;
		case VIEWER_WINDOW_WIDTH:
			g_nViewerWindowWidth = nValue;
			break;	
		case VIEWER_WINDOW_HEIGHT:
			g_nViewerWindowHeight = nValue;
			break;
		case VIEWER_MAP_LAYERS_NUMBER:
			g_nMapLayersNumber = nValue;
			break;
		default:
			WARNING("Invalid parameter (ViewerSetParameteri)", "", "");
			return (-1);
	}
	
        return (0);
}



// ----------------------------------------------------------------------
// ViewerSetParameterf - sets a module real parameter
//
// Inputs: nParameter - the parameter
//	   fltValue - the new value
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerSetParameterf (int nParameter, float fltValue)
{
        if (g_eViewerStatus == INITIALIZED)
                return (-1);
		
	switch (nParameter)
	{
		case VIEWER_FOVY:
			g_fltFovy = fltValue;
			break;
		case VIEWER_FAR:
			g_fltFar = fltValue;
			break;
		case VIEWER_NEAR:
			g_fltNear = fltValue;
			break;	
		case VIEWER_FOCUS:
			g_fltFocus = fltValue;
			break;
		case VIEWER_FLOOR:
			g_fltFloor = fltValue;
			break;
		case VIEWER_ROBOT_RADIUS:
			g_fltViewerRobotRadius = fltValue;
			break;
		case VIEWER_TERRAIN_SIZE:
			g_fltViewerTerrainSize = fltValue;
			break;
		default:
			WARNING("Invalid parameter (ViewerSetParameterf)", "", "");
			return (-1);
	}
	
        return (0);
}

// ----------------------------------------------------------------------
// ViewerDrawFOV - draws the FOV in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerDrawFOV (void)
{
	float fltFovWidth, fltFovHeight;
	
	fltFovWidth = (float) g_nImageWidth / 2.0f * g_fltFar / g_fltFocus;
	fltFovHeight = (float) g_nImageHeight / 2.0f * g_fltFar / g_fltFocus;

	// Field of view
	glColor3f(.0f, 1.0f, .0f);
	glBegin(GL_LINE_LOOP);
	{		
		glVertex3f(g_fltFar, -fltFovWidth, -fltFovHeight);
		glVertex3f(.0f, .0f, .0f);
		glVertex3f(g_fltFar, -fltFovWidth,  fltFovHeight);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(g_fltFar, fltFovWidth, -fltFovHeight);
		glVertex3f(.0f, .0f, .0f);
		glVertex3f(g_fltFar, fltFovWidth,  fltFovHeight);
	}
	glEnd();	

	glBegin(GL_LINES);
	{
		glVertex3f(g_fltFar, -fltFovWidth, -fltFovHeight);
		glVertex3f(g_fltFar,  fltFovWidth, -fltFovHeight);
		glVertex3f(g_fltFar, -fltFovWidth,  fltFovHeight);
		glVertex3f(g_fltFar,  fltFovWidth,  fltFovHeight);
	}
	glEnd();

	return;
}



// ----------------------------------------------------------------------
// ViewerDrawCutPlane - draws the FOV in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerDrawCutPlane (void)
{
	glColor4f (1.0f, 1.0f, 1.0f, .3f);
	glBegin(GL_POLYGON);
	{
		glVertex3f(-g_fltFar, -g_fltFar, g_fltFloor);
		glVertex3f(g_fltFar,  -g_fltFar, g_fltFloor);
		glVertex3f(g_fltFar, g_fltFar, g_fltFloor);
		glVertex3f(-g_fltFar,  g_fltFar, g_fltFloor);
	}
	glEnd();

	return;
}



// ----------------------------------------------------------------------
// ViewerDrawCutPlane - draws the FOV in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerDraw2DFOV (void)
{
	float fltFovWidth;
	
	fltFovWidth = (float) g_nImageWidth / 2.0f * g_fltFar / g_fltFocus;

	// Field of view
	glColor3f(.0f, 1.0f, .0f);
	glBegin(GL_LINE_LOOP);
	{		
		glVertex2f (.0f, .0f);
		glVertex2f (g_fltFar,  fltFovWidth);
		glVertex2f (g_fltFar, -fltFovWidth);
	}
	glEnd();

	return;
}



// ----------------------------------------------------------------------
// ViewerDrawCircle - draws a circle in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerDrawCircle(float fltRadius)
{
	float fltAngle;
	
	glColor3f(1.0f, .0f, .0f);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	glBegin(GL_LINE_LOOP);
   	for (fltAngle = 0; fltAngle <= 2.0f * M_PI; fltAngle += M_PI/100.0f)
		glVertex2f(cos(fltAngle) * fltRadius, sin(fltAngle) * fltRadius);
   	glEnd();
	glDisable (GL_LINE_STIPPLE);

	return;
}



// ----------------------------------------------------------------------
// ViewerDisplay - draws the scene in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerDisplay (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (g_nViewerViewMode)
        {
                case EXTERNAL_VIEW:
			 // Configures the OpenGL visualization volume
   			g_pViewerExternalCamera->ConfigureVisualizationVolume ();
			ViewerDrawFOV ();
			if (g_bShowFloorPlane)
				ViewerDrawCutPlane ();
			g_pMap->Draw ();
                        break;
                case WORLD_MAP_VIEW:
                        g_pMap->ShowImage ();
                        break;
                case DEPTH_MAP_VIEW:
                        g_pMap->ShowDepthMap();
                        break; 
		case OCCUPANCY_MAP_VIEW:
                        g_pMap->ShowOccupancyMap();
			ViewerDraw2DFOV ();
 			ViewerDrawCircle (g_fltViewerRobotRadius);
                       break;
		case EVIDENCE_MAP_VIEW:
                        g_pMap->ShowEvidenceMap();
 			ViewerDraw2DFOV ();
                        glColor3f (1.0f, .0f, .0f);
			glPointSize(5);
			glBegin(GL_POINTS); glVertex2f (.0f, .0f); glEnd ();
   			glPointSize(2);
                     break;
                default:
                         WARNING("Invalid view mode (ViewerDisplay)", "", "");
        }

	glutSwapBuffers();	// Swap buffers
	glFlush();

	return;
}



// ----------------------------------------------------------------------
// ViewerMouse - mouse handler
//
// Inputs: button - mouse button
//         state - button state
//         (x, y)- mouse window position
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerMouse (int button, int state, int x, int y)
{
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
	{
		g_nOldX = x - g_pViewerExternalCamera->imagewidth / 2;
		g_nOldY = g_pViewerExternalCamera->imageheight / 2 - y;
	}
	
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
	{

	}

        return;
}



// ----------------------------------------------------------------------
// ViewerReshape - window reshape handler
//
// Inputs: nWidth X nHeight - the new window dimentions
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerReshape (int nWidth, int nHeight)
{
	// Configures the viewport
       	g_pViewerExternalCamera->ResizeImage (nWidth, nHeight);
		
	glutPostRedisplay ();
        
	return;
}



// ----------------------------------------------------------------------
// ViewerMotion - mouse motion handler
//
// Inputs: (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerMotion (int x, int y)
{
	int nDeltaX, nDeltaY, nCurrentX, nCurrentY;
	float fltDeltaAlpha, fltDeltaBeta;
	
	nCurrentX = x - g_pViewerExternalCamera->imagewidth / 2;
	nCurrentY = g_pViewerExternalCamera->imageheight / 2 - y;
	
	nDeltaX = nCurrentX - g_nOldX;
	nDeltaY = nCurrentY - g_nOldY;
	
	g_nOldX = nCurrentX;
	g_nOldY = nCurrentY;
		
	fltDeltaAlpha = M_PI * (float) nDeltaX / (float) g_pViewerExternalCamera->imagewidth;
	fltDeltaBeta = M_PI * (float) nDeltaY / (float) g_pViewerExternalCamera->imageheight;
	
	g_pViewerExternalCamera->RotateAroundFixationPoint(fltDeltaAlpha, fltDeltaBeta);
	
	glutPostRedisplay();
	
	return;
}



// ----------------------------------------------------------------------
// ViewerTimer - window time event handler
//
// Inputs: nTimeEvent - the event ID
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerTimer (int value)
{
	switch (value)
	{
                case VIEWER_SWAP_EVENT:
                        g_pMap->GetNextLayer ();
                        if (g_bAutoSwapping)
                                glutTimerFunc (100, ViewerTimer, VIEWER_SWAP_EVENT);
                        break;
                default:
                        WARNING("Invalid time event (ViewerTimer)", "", "");
        }

	glutPostRedisplay ();
	
        return;
}



// ----------------------------------------------------------------------
// ViewerKeyboard - keyboard handler
//
// Inputs: key - the keyboard button
//         (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		// Rotates the external camera left around Z axis
		case '4':
                        g_pViewerExternalCamera->RotateAroundFixationPoint(-CAMERA_DELTA_ANGLE, .0f);
			break;
		// Rotates the external camera right around Z axis
		case '6':
                        g_pViewerExternalCamera->RotateAroundFixationPoint(CAMERA_DELTA_ANGLE, .0f);
			break;
		// Rotates the external camera down from XY plane
		case '2':
                        g_pViewerExternalCamera->RotateAroundFixationPoint( .0f, -CAMERA_DELTA_ANGLE);
			break;
		case '8':
                         g_pViewerExternalCamera->RotateAroundFixationPoint( .0f, CAMERA_DELTA_ANGLE);
			break;
		// Zoom out the external camera
		case '-':
                        g_pViewerExternalCamera->Zoom(CAMERA_ZOOM_STEP);
			break;
		// Zoom in the external camera
		case '+':
                        g_pViewerExternalCamera->Zoom(1.0f / CAMERA_ZOOM_STEP);
			break;
                // Prints the external camera focal distance
		/*case 'F':
                case 'f':
                        cout << "Camera focal length = " << g_pViewerExternalCamera->GetFocalLength() << endl;
			break;*/
                // Loads the map
                case 'L':
                case 'l':
                        if (!g_pMap->Load (MAP_FILE_NAME))
                                Error ("Cannot load map from file '", MAP_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map loaded from '", MAP_FILE_NAME, "'");
                        break;
                // Dumps the map
                case 'D':
                case 'd':
                        if (!g_pMap->Save (MAP_FILE_NAME))
                                Error ("Cannot save map to file '", MAP_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map saved to '", MAP_FILE_NAME, "'");
                        break;
                // Saves the map image
                case 'S':
                case 's':
                        if (!g_pMap->SaveImage (IMAGE_FILE_NAME))
                                Error ("Cannot save image to file '", IMAGE_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map image saved to '", IMAGE_FILE_NAME, "'");
                        break;
                // Alternates to the external view mode
                case 'E':
                case 'e':
                        g_nViewerViewMode = EXTERNAL_VIEW;
                        MESSAGE("External view mode", "", "");
                        break;
                // Alternates to the world map view mode
                case 'W':
                case 'w':
                        g_nViewerViewMode = WORLD_MAP_VIEW;
                        MESSAGE("World map view mode", "", "");
                        break;
                // Alternates to the depth map view mode
                case 'R':
                case 'r':
                        g_nViewerViewMode = DEPTH_MAP_VIEW;
                        MESSAGE("World depth map view mode", "", "");
                        break; 
		// Alternates to the occupancy map view mode
                case 'O':
                case 'o':
                        g_nViewerViewMode = OCCUPANCY_MAP_VIEW;
                        MESSAGE("World occupancy map view mode", "", "");
                        break;
		// Alternates to the evidence map view mode
                case 'F':
                case 'f':
			g_pMap->UpdateEvidenceGrid (2.5f * g_fltViewerRobotRadius);
                        g_nViewerViewMode = EVIDENCE_MAP_VIEW;
                        MESSAGE("World evidence map view mode", "", "");
                        break;
                // Disables auto swapping
                case 'T':
                        g_bAutoSwapping = false;
                        break;
                // Enables auto swapping
                case 't':
                        glutTimerFunc (100, ViewerTimer, VIEWER_SWAP_EVENT);
                        g_bAutoSwapping = true;
                        break;
		// Raises the floor plane
                case 'C':
                        g_fltFloor += .5f;
                        break;
		// Lowers the floor plane
                case 'c':
                        g_fltFloor -= .5f;
                        break;
		// View or hide the floor plane 
		case 'X':
		case 'x':
                        g_bShowFloorPlane = !g_bShowFloorPlane;
                        break;
		// Print robot odometry
		case 'H':
		case 'h':
			cout << "Viewer:" << endl;
			cout << "Robot position = (" << g_pMap->robotX << ", " << g_pMap->robotY << ")" << endl;
			cout << "Robot orientation = " << g_pMap->robotAlpha << endl;
			break;
		// Gets the shortest path to a goal position
		case 'P':
		case 'p':
			float fltNearestFrontierX, fltNearestFrontierY;
			g_pMap->UpdateEvidenceGrid (g_fltViewerRobotRadius);
			g_nViewerViewMode = EVIDENCE_MAP_VIEW;
                        MESSAGE("World evidence map view mode", "", "");
			g_pMap->GetNearestFrontier (UNKNOWN_FRONTIER_CELL, 5, &fltNearestFrontierX, &fltNearestFrontierY);
                       	g_pMap->GetShortestPath (fltNearestFrontierX, fltNearestFrontierY);
                        break;
		// Quits viewer
		case 'Q':
		case 'q':
                        ViewerQuit ();
			return;
                default:
                        WARNING("Invalid key pressed '", key, "' (ViewerKeyboard).");
	}

	glutPostRedisplay ();
	
	return;
}



// ----------------------------------------------------------------------
// ViewerSpecial - keyboard special key handler
//
// Inputs: key - the keyboard button
//         (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerSpecial (int key, int x, int y)
{
        switch (key)
	{
                // Rotates the map left
		case GLUT_KEY_LEFT:
                        g_pMap->Rotate (-5.0f);
			break;
                // Rotates the map right
		case GLUT_KEY_RIGHT:
                        g_pMap->Rotate (5.0f);
			break;
                // Moves the map backward
		case GLUT_KEY_DOWN:
                        g_pMap->Forward (2.5f);
			break;
                // Moves the map forward
		case GLUT_KEY_UP:
                        g_pMap->Forward (-2.5f);
                        break;
                default:
                        WARNING("Invalid key pressed '", key, "' (ViewerSpecial).");
	}

	glutPostRedisplay ();

	return;
}



// ----------------------------------------------------------------------
// ViewerInitialize - initializes the viewer module
//
// Inputs: none
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerInitialize (void)
{
        if (g_eViewerStatus == INITIALIZED)
                return (-1);
        g_eViewerStatus = INITIALIZED;

        // Creates the map
	g_pMap = new TMap (g_nMapWidth, g_nMapHeight, -g_fltFovy / (2.0f * g_fltAspectRatio), g_fltFovy / (2.0f * g_fltAspectRatio), g_nMapLayersNumber, g_nMapEspheresNumber, g_fltNear, g_fltFar, g_fltViewerTerrainSize);
	g_pViewerExternalCamera = new TCamera (VIEWER_DEFAULT_FOVY, VIEWER_DEFAULT_ASPECT_RATIO, VIEWER_DEFAULT_NEAR, VIEWER_DEFAULT_FAR);
	
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize (g_nViewerWindowWidth, g_nViewerWindowHeight);
	glutInitWindowPosition (100, 100);

        if ((g_nViewerWindowId = glutCreateWindow (VIEWER_WINDOW_TITLE)) == 0)
        {
                Error("Cannot create map window (ViewerInitialize)", "", "");
                return (-1);
        }

	glClearColor (.5f, .67f, .94f, .0f);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT,GL_NICEST);
	glPointSize (2);


/*	GLfloat materialAmbient[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat materialShininess[] = {50.0};
	GLfloat lightAmbient[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat lightPosition[] = {0.0, 50.0, 0.0};

	glMaterialfv (GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv (GL_FRONT, GL_SHININESS, materialShininess);
	glLightfv (GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);*/

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glShadeModel(GL_FLAT);
	
	glutDisplayFunc (ViewerDisplay);
	glutReshapeFunc (ViewerReshape);
	glutMouseFunc (ViewerMouse);
	glutMotionFunc (ViewerMotion);
	glutKeyboardFunc (ViewerKeyboard);
	glutSpecialFunc (ViewerSpecial);
	
        return (0);
}



// ----------------------------------------------------------------------
// ViewerQuit - exits the viewer module
//
// Inputs: none
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerQuit (void)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);
        g_eViewerStatus = UNINITIALIZED;

        glutDestroyWindow (g_nViewerWindowId);

	delete g_pMap;
	delete g_pViewerExternalCamera;
	        
        return (0);
}



// ----------------------------------------------------------------------
// ViewerUpdateMap -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerUpdateMap (float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->Update(X, Y, Z, red, green, blue);

	//glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
// ViewerEraseMap -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerEraseMap (void)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->Erase ();

	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
// ViewerGetFuzzyInputs -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerGetFuzzyInputs (double *p_dblFuzzyInputs, int nInputsNumber, float fltMinAlpha, float fltMaxAlpha)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->GetFuzzyInputs (p_dblFuzzyInputs, nInputsNumber, fltMinAlpha, fltMaxAlpha, g_fltFloor);

        return (0);
}



// ----------------------------------------------------------------------
// ViewerTransformMap - applyes a transformation on the map
//
// Inputs: fltAngle - the rotation angle [deg]
//	   fltStep - the translation step [cm]
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerTransformMap (float fltAngle, float fltStep)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->Rotate (fltAngle);
        g_pMap->Forward (fltStep);
	
	//glutPostWindowRedisplay (g_nViewerWindowId);

	glutSetWindow (g_nViewerWindowId);
	ViewerDisplay ();
	
        return (0);
}



// ----------------------------------------------------------------------
// ViewerGenerateOccupancyMap - generates the occupancy map
//
// Inputs: None.
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerGenerateOccupancyMap (void)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->GenerateOccupancyGrid ();
	
	g_nViewerViewMode = OCCUPANCY_MAP_VIEW;
	MESSAGE("World occupancy map view mode", "", "");
	
	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
//  ViewerUpdateEvidenceMap - updates the evidence map
//
// Inputs: fltMinDistance - .
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerUpdateEvidenceMap (float fltMinDistance)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMap->UpdateEvidenceGrid (fltMinDistance);
	
	g_nViewerViewMode = EVIDENCE_MAP_VIEW;
	MESSAGE("World evidence map view mode", "", "");
	
	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
//  ViewerMakePath - makes the path
//
// Inputs: None.
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerMakePath (void)
{
 	float fltNearestFrontierX, fltNearestFrontierY;
        
	if (g_eViewerStatus == UNINITIALIZED)
                return (-1);
	
	if (!g_pMap->GetNearestFrontier (UNKNOWN_FRONTIER_CELL, 1, &fltNearestFrontierX, &fltNearestFrontierY))
		return (-1);
        g_pMap->GetShortestPath (fltNearestFrontierX, fltNearestFrontierY);
	
	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
//  ViewerMakePath2Point - makes the path
//
// Inputs: None.
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerMakePath2Point (float fltX, float fltY)
{     
	if (g_eViewerStatus == UNINITIALIZED)
                return (-1);
	
        g_pMap->GetShortestPath (fltX, fltY);
	
	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
//  ViewerUpdateEvidenceMap - updates the evidence map
//
// Inputs: None.
//
// Outputs: 0 along the path, -1 when the path ends, -2 otherwise
// ----------------------------------------------------------------------

int ViewerGetPath (float *p_fltDistance, float *p_fltAngle)
{	
	static int nNodeIndex = 0;
	float fltStepX, fltStepY;
	float fltAlpha, fltBeta, fltDistance;
	
	if (g_eViewerStatus == UNINITIALIZED)
                return (-2);
			
	if (nNodeIndex == 0)
	{
		if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE)
		{
			nNodeIndex = 0;
			return (-2);
		}
		nNodeIndex = (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3];
	}
		
	fltStepX = g_p_fltPathSearchTree[4 * nNodeIndex + 0];
	fltStepY = g_p_fltPathSearchTree[4 * nNodeIndex + 1];

	ConvertRect2Sphe (&fltAlpha, &fltBeta, &fltDistance, fltStepX - g_pMap->robotX, fltStepY - g_pMap->robotY, .0f);
	
	fltAlpha = 180.0f / M_PI * (fltAlpha - g_pMap->robotAlpha);
	fltAlpha = fltAlpha - 360.0f * (float) ((int) fltAlpha / 360);
	
	if (fltAlpha >= 180.0f)
		fltAlpha -= 360.0f;
	
	if (fltAlpha <= -180.0f)
		fltAlpha += 360.0f;
	
	*p_fltAngle = fltAlpha;
	*p_fltDistance = fltDistance;
	
	cout << "Distance = " << *p_fltDistance << " [cm]" << endl;
	cout << "Angle = " << *p_fltAngle << " [deg]" << endl;
	
	if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE)
	{
		nNodeIndex = 0;
		return (-1);
	}
	
	nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3];
	
	if (nNodeIndex >= MAX_NODES)
	{
		nNodeIndex = 0;
		return (-2);
	}
	
        return (0);
}



#ifdef _TESTING_VIEWER
// ----------------------------------------------------------------------
// ViewerShowUsageMessage - shows the usage information
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerShowUsageMessage (void)
{
	cout << "Usage: ./viewer [option]\nOptions:\n";
        cout << "\t-h\t\tShow this help.\n";
        cout << "\t-v\t\tShow version information.\n";
	exit (-1);
}



// ----------------------------------------------------------------------
// ViewerParseArguments - parses the command line parameters
//
// Inputs: the default main function parameters
//
// Outputs: none
// ----------------------------------------------------------------------

void ViewerParseArguments (int argc, char **argv)
{
	if ((argc != 1) && (argc != 2))
		ViewerShowUsageMessage ();
	
	if (argc == 2)
	{
		if (strcmp ("-h", argv[1]) == 0)
			ViewerShowUsageMessage ();
		else if (strcmp ("-v", argv[1]) == 0)
		{
			cout << "Viewer version " << VIEWER_VERSION << endl;
			exit (0);
		}
		else
		{
			Error ("Invalid input argument (ViewerParseArguments)'", argv[1], "'");
			ViewerShowUsageMessage ();
		}
	}
			
	return;
}



// ----------------------------------------------------------------------
// main - main function
//
// Inputs: the default main function parameters
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int main (int argc, char **argv)
{
	// Parse the command line arguments
	ViewerParseArguments (argc, argv);
	
	glutInit (&argc, argv);
	
	if (ViewerInitialize ())
	{
		Error ("Cannot initialize the viewer (main)", "", "");
		exit (-1);
	}
	
	glutMainLoop ();
	
	return (0);
}
#endif
