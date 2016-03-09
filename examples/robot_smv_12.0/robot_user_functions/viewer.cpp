// Includes

#include "cconfig.h"
#include "crobot_smv.h"
#include "maplist.hpp"
#include "view.hpp"
#include "common.hpp"
#include "viewer.hpp"
#include "stereo_api.h"

// Global Variables
int g_nViewerWindowId;
int g_nViewerViewMode= EXTERNAL_VIEW;
bool g_bAutoSwapping = false;
TMapList *g_pMaps;
TView *g_pExternalView;
TModuleStatus g_eViewerStatus = UNINITIALIZED;
bool g_bShowFloorPlane = false;
int g_nOldX = 0, g_nOldY = 0;
double g_p_dblRightCameraOrientation[] = {0.0, 0.0 , 0.0, 0.0, 0.0, 0.0};
double g_p_dblLeftCameraOrientation[] = {0.0, 100.0 , 0.0, 0.0, 0.0, 0.0};

// Integer module parameters
GLint g_nViewerWindowWidth = 512; // [pix]
GLint g_nViewerWindowHeight = 512; // [pix]
GLint g_nMapWidth = 2560; // [pix]
GLint g_nMapHeight = 960; // [pix]
GLint g_nImageWidth = 256; // [pix]
GLint g_nImageHeight = 256; // [pix]
GLfloat g_fltAspectRatio = (GLfloat) g_nImageWidth / (GLfloat) g_nImageHeight; // []
GLint g_nMapLayersNumber = 2;

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
// ViewerSetParameterv - sets a module vector parameter
//
// Inputs: nParameter - the parameter
//	   pVector - the new vector
//	   nLength - the vector length
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerSetParameterv (int nParameter, void *pVector, int nLength)
{
        int i;
	double *p_dblVector = NULL;

	if (g_eViewerStatus == INITIALIZED)
                return (-1);

	if (pVector == NULL)
	{
		WARNING("Null vector (ViewerSetParameterv)", "", "");
		return (-1);
	}

	switch (nParameter)
	{
		case VIEWER_LEFT_CAMERA_ORIENTATION:
			for (i = 0, p_dblVector = (double *) pVector; i < nLength; i++)
				g_p_dblLeftCameraOrientation[i] = p_dblVector[i];
			break;
		case VIEWER_RIGHT_CAMERA_ORIENTATION:
			for (i = 0, p_dblVector = (double *) pVector; i < nLength; i++)
				g_p_dblRightCameraOrientation[i] = p_dblVector[i];
			break;
		default:
			WARNING("Invalid parameter (ViewerSetParameterv)", "", "");
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
  			g_pExternalView->ConfigureVisualizationVolume ();
			glColor3f(.0f, 1.0f, .0f);
			ViewerDrawFOV ();
			//glColor3f(1.0f, .0f, .0f);
			//glRotatef (g_p_dblLeftCameraOrientation[5], 1.0f,  .0f,  .0f);
			//glRotatef (g_p_dblLeftCameraOrientation[3],  .0f, 1.0f,  .0f);
			//glRotatef (g_p_dblLeftCameraOrientation[4],  .0f,  .0f, 1.0f);
			//glTranslatef (-g_p_dblLeftCameraOrientation[2], -g_p_dblLeftCameraOrientation[0], -g_p_dblLeftCameraOrientation[1]);
			//ViewerDrawFOV ();
			//if (g_bShowFloorPlane)
			//	ViewerDrawCutPlane ();
			g_pMaps->Draw ();
                        break;
                case WORLD_MAP_VIEW:
                        g_pMaps->ShowImage ();
                        break;
                case DEPTH_MAP_VIEW:
                        g_pMaps->ShowDepthMap();
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
		g_nOldX = x - g_pExternalView->imagewidth / 2;
		g_nOldY = g_pExternalView->imageheight / 2 - y;
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
       	g_pExternalView->ResizeImage (nWidth, nHeight);

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

	nCurrentX = x - g_pExternalView->imagewidth / 2;
	nCurrentY = g_pExternalView->imageheight / 2 - y;

	nDeltaX = nCurrentX - g_nOldX;
	nDeltaY = nCurrentY - g_nOldY;

	g_nOldX = nCurrentX;
	g_nOldY = nCurrentY;

	fltDeltaAlpha = M_PI * (float) nDeltaX / (float) g_pExternalView->imagewidth;
	fltDeltaBeta = M_PI * (float) nDeltaY / (float) g_pExternalView->imageheight;

	g_pExternalView->RotateAroundFixationPoint(fltDeltaAlpha, fltDeltaBeta);

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
                        g_pMaps->GetNextLayer ();
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

double g_factor = 1.0;

void ViewerKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		// Rotates the external view left around Z axis
		case '4':
                        g_pExternalView->RotateAroundFixationPoint(-CAMERA_DELTA_ANGLE, .0f);
			break;
		// Rotates the external view right around Z axis
		case '6':
                        g_pExternalView->RotateAroundFixationPoint(CAMERA_DELTA_ANGLE, .0f);
			break;
		// Rotates the external view down from XY plane
		case '2':
                        g_pExternalView->RotateAroundFixationPoint( .0f, -CAMERA_DELTA_ANGLE);
			break;
		case '8':
                         g_pExternalView->RotateAroundFixationPoint( .0f, CAMERA_DELTA_ANGLE);
			break;
		// Zoom out the external camera view
		case '-':
                        g_pExternalView->Zoom(CAMERA_ZOOM_STEP);
			break;
		// Zoom in the external camera view
		case '+':
                        g_pExternalView->Zoom(1.0f / CAMERA_ZOOM_STEP);
			break;
		case 'P':
                        g_pMaps->AddPosition(10, 0, 0);
			break;
		case 'p':
                        g_pMaps->AddPosition(-10, 0, 0);
			break;
		case 'O':
                        g_pMaps->AddPosition(0, 10, 0);
			break;
		case 'o':
                        g_pMaps->AddPosition(0, -10, 0);
			break;
		case 'I':
                        g_pMaps->AddPosition(0, 0, 10);
			break;
		case 'i':
                        g_pMaps->AddPosition(0, 0, -10);
			break;
                // Loads the map
                case 'L':
                case 'l':
                        if (!g_pMaps->Load (0, MAP_FILE_NAME))
                                Error ("Cannot load map from file '", MAP_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map loaded from '", MAP_FILE_NAME, "'");
                        break;
                // Dumps the map
                case 'D':
                        if (!g_pMaps->SavePointCloud (0, "cloud.txt"))
                                Error ("Cannot save map to file '", "cloud.txt", "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map saved to '", "cloud.txt", "'");
                        break;
                case 'd':
                        if (!g_pMaps->Save (0, MAP_FILE_NAME))
                                Error ("Cannot save map to file '", MAP_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map saved to '", MAP_FILE_NAME, "'");
                        break;
                // Saves the map image
                case 'S':
                case 's':
                        if (!g_pMaps->SaveImage (0, IMAGE_FILE_NAME))
                                Error ("Cannot save image to file '", IMAGE_FILE_NAME, "' (ViewerKeyboard)");
                        else
                                MESSAGE("Map image saved to '", IMAGE_FILE_NAME, "'");
                        break;
                // Alternates to the external view mode
                case 'R':
                case 'r':
                        g_nViewerViewMode = EXTERNAL_VIEW;
                        MESSAGE("External view mode", "", "");
                        break;
                // Save the current map viewer coordinates
                case 'v':
			{
				float fltX, fltY, fltZ;

				g_pMaps->GetPosition (&fltX, &fltY, &fltZ);
				config_set_float("world.x0", fltX);
				config_set_float("world.y0", fltY);
				config_set_float("world.z0", fltZ);

				g_pExternalView->GetPosition (&fltX, &fltY, &fltZ);
				config_set_float("viewer.x0", fltX);
				config_set_float("viewer.y0", fltY);
				config_set_float("viewer.z0", fltZ);
				
				robot_smv_save_config();
			}
                        break;
                // Restore the current map viewer coordinates
                case 'V':
			{
				float fltX, fltY, fltZ;

				robot_smv_load_config();

				fltX = config_get_float("world.x0");
				fltY = config_get_float("world.y0");
				fltZ = config_get_float("world.z0");
				g_pMaps->SetPosition (fltX, fltY, fltZ);

				fltX = config_get_float("viewer.x0");
				fltY = config_get_float("viewer.y0");
				fltZ = config_get_float("viewer.z0");
				g_pExternalView->SetPosition (fltX, fltY, fltZ);
				
				robot_smv_display();
			}
                        break;
		case 'f':
			g_factor *= 0.5;
                        break;
		case 'F':
			g_factor *= 2.0;
                        break;
		case 'h':
			robot_smv_rotate_x(g_factor * M_PI/18.0);
                break;
		case 'H':
			robot_smv_rotate_x(-g_factor * M_PI/18.0);
                break;
		case 'j':
			robot_smv_rotate_y(g_factor * M_PI/18.0);
                        break;
		case 'J':
			robot_smv_rotate_y(-g_factor * M_PI/18.0);
                        break;
		case 'k':
			robot_smv_rotate_z(g_factor * M_PI/18.0);
                        break;
		case 'K':
			robot_smv_rotate_z(-g_factor * M_PI/18.0);
                        break;
		case 'b':
			robot_smv_translate_x(g_factor*10);
                        break;
		case 'B':
			robot_smv_translate_x(-g_factor*10);
                        break;
		case 'n':
			robot_smv_translate_y(g_factor*10);
                        break;
		case 'N':
			robot_smv_translate_y(-g_factor*10);
                        break;
		case 'm':
			robot_smv_translate_z(g_factor*10);
                        break;
		case 'M':
			robot_smv_translate_z(-g_factor*10);
                        break;
		case 'z':
			robot_smv_center();
                        break;
		case 'Z':
			robot_smv_straight();
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
			break;
                // Rotates the map right
		case GLUT_KEY_RIGHT:
			break;
                // Moves the map backward
		case GLUT_KEY_DOWN:
			break;
                // Moves the map forward
		case GLUT_KEY_UP:
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

int ViewerInitialize (int nMapsLength)
{
        if (g_eViewerStatus == INITIALIZED)
                return (-1);
        g_eViewerStatus = INITIALIZED;

        // Creates the map
	g_pMaps = new TMapList(nMapsLength, g_nMapWidth, g_nMapHeight, -g_fltFovy / (2.0f * g_fltAspectRatio), g_fltFovy / (2.0f * g_fltAspectRatio), g_nMapLayersNumber, g_fltNear, g_fltFar, g_fltViewerTerrainSize);
	g_pExternalView = new TView (VIEWER_DEFAULT_FOVY, VIEWER_DEFAULT_ASPECT_RATIO, VIEWER_DEFAULT_NEAR, VIEWER_DEFAULT_FAR);
	//g_pExternalView = new TView (g_fltFovy, g_fltAspectRatio, g_fltNear, g_fltFar);
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

	delete g_pMaps;
	delete g_pExternalView;

        return (0);
}



// ----------------------------------------------------------------------
// ViewerUpdateMap -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

int ViewerUpdateMap (int nIndex, float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue)
{
        if (g_eViewerStatus == UNINITIALIZED)
                return (-1);

        g_pMaps->Update(nIndex, X, Y, Z, red, green, blue);

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

        g_pMaps->Erase ();

	glutPostWindowRedisplay (g_nViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
// ViewerRedisplay -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void ViewerRedisplay (void)
{
	glutPostWindowRedisplay (g_nViewerWindowId);
}
