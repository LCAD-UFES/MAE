// Includes
#include "common.hpp"
#include "memviewer.hpp"

// Global Variables
GLint g_nMemViewerWindowId;
TModuleStatus g_eMemViewerStatus = UNINITIALIZED;
//GLint g_nOldX = 0, g_nOldY = 0;
GLubyte **g_pNeuronMemory = NULL;
GLfloat *g_fltNeuronAssociatedValues = NULL;
GLint g_nStoredPatternsNumber = 0;

// Integer module parameters
GLint g_nMemViewerWindowWidth = 128; // [pix]
GLint g_nMemViewerWindowHeight = 128; // [pix]
GLint g_nNeuronMemorySize = 1;
GLint g_nMemoryPatternSize = 1;

GLfloat g_fltMinX, g_fltMaxX, g_fltMinY, g_fltMaxY;

// Real module parameters
/*GLfloat g_fltFovy = 47.05f; // [deg]
GLfloat g_fltFar = 100.0f; // [mm]
GLfloat g_fltNear = .1f; // [mm]
GLfloat g_fltFocus = 400.0f; // [pix]*/



// ----------------------------------------------------------------------
// Gray2Binary - convert gray code to binary code of length n+1
//
// Inputs: Gray code Cg[0...n]
//
// Outputs: Binary code Cb[0...n]
// ----------------------------------------------------------------------

GLvoid Gray2Binary (GLubyte *Cg, GLubyte *Cb, GLint n)
{
	GLint j;
	
	// Copy the high-order bit
	*Cb = *Cg;
	            
	for (j = 0; j < n; j++)
	{
		// Do the appropriate XOR for the remaining bits
		Cb--; Cg--;
		*Cb= *(Cb+1)^*Cg;
	}
}



// ----------------------------------------------------------------------
// Binary2Gray - convert binary code to gray code of length n+1
//
// Inputs: Binary code Cb[0...n]
//
// Outputs: Gray code Cg[0...n]
// ----------------------------------------------------------------------

GLvoid Binary2Gray (GLubyte *Cb, GLubyte *Cg, GLint n)
{
	GLint j;

	 // Copy the high-order bit
	*Cg = *Cb;              
	for (j = 0; j < n; j++)
	{
		// Do the appropriate XOR for the remaining bits
		Cg--; Cb--;
		*Cg= *(Cb+1)^*Cb;
	}
}


// ----------------------------------------------------------------------
// CompareBinaryCodes - compares binary codes
//
// Inputs: 
//
// Outputs: Gray code Cg[0...n]
// ----------------------------------------------------------------------

/*GLvoid CompareBinaryCodes (GLubyte *Cb1, GLubyte *Cb2, GLint n)
{
	GLint j;

	 // Copy the high-order bit
	*Cg = *Cb;              
	for (j = 0; j < n; j++)
	{
		// Do the appropriate XOR for the remaining bits
		Cg--;
		Cb--;
		*Cg= *(Cb+1)^*Cb;
	}
}*/


// ----------------------------------------------------------------------
// MemViewerSetParameteri - sets a module integer parameter
//
// Inputs: nParameter - the parameter
//	   nValue - the new value
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerSetParameteri (GLint nParameter, GLint nValue)
{
         if (g_eMemViewerStatus == INITIALIZED)
                return (-1);
		
	switch (nParameter)
	{
		case MEM_VIEWER_NEURON_MEMORY_SIZE:
			g_nNeuronMemorySize = nValue;
			break;
		case MEM_VIEWER_MEMORY_PATTERN_SIZE:
			g_nMemoryPatternSize = nValue;
			break;
		default:
			WARNING("Invalid parameter (MemViewerSetParameteri)", "", "");
			return (-1);
	}
	
        return (0);
}



// ----------------------------------------------------------------------
// MemViewerCalculatePosition - 
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerCalculatePosition (GLfloat *x, GLfloat *y, GLubyte *pattern)
{
	//g_nMemoryPatternSize
        return (0);
}



// ----------------------------------------------------------------------
// MemViewerDisplay - draws the scene in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerDisplay ()
{
	int i;
	GLubyte *pattern = NULL;
	GLfloat associated_value;
	float x, y;
	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (.0f, 1.0f, .0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
        
	glBegin(GL_POINTS);
        for (i = 0; i < g_nStoredPatternsNumber; i++)
	{
		pattern = g_pNeuronMemory[i];
		associated_value = g_fltNeuronAssociatedValues[i];
		
		/*if (associated_value > .0f)
	                glColor3f (.0f,.5f*associated_value,.0f);
                else
			glColor3f (-.5f*associated_value,.0f,.0f);*/
				
		glColor3f (associated_value,associated_value,associated_value);
               	
		/*x = (float) i / (float) g_nStoredPatternsNumber;
		y = (float) i / (float) g_nStoredPatternsNumber;*/
		
		MemViewerCalculatePosition (&x, &y, pattern);
		glVertex2f ((x - g_fltMinX) / (g_fltMaxX - g_fltMinX), (y - g_fltMinY) / (g_fltMaxY - g_fltMinY));
        }
        glEnd ();

	glutSwapBuffers();	// Swap buffers
	glFlush();

	return;
}



// ----------------------------------------------------------------------
// MemViewerMouse - mouse handler
//
// Inputs: button - mouse button
//         state - button state
//         (x, y)- mouse window position
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerMouse (GLint button, GLint state, GLint x, GLint y)
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
// MemViewerReshape - window reshape handler
//
// Inputs: nWidth X nHeight - the new window dimentions
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerReshape (GLint nWidth, GLint nHeight)
{
	// Configures the viewport
	glViewport (0, 0, (GLsizei) nWidth, (GLsizei) nHeight);

	glutPostRedisplay ();
        
	return;
}



// ----------------------------------------------------------------------
// MemViewerMotion - mouse motion handler
//
// Inputs: (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerMotion (GLint x, GLint y)
{
	
	glutPostRedisplay();
	
	return;
}



// ----------------------------------------------------------------------
// MemViewerKeyboard - keyboard handler
//
// Inputs: key - the keyboard button
//         (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerKeyboard (GLubyte key, GLint x, GLint y)
{
	switch (key)
	{
		// Quits memory viewer module
		case 'Q':
		case 'q':
                        MemViewerQuit ();
			return;
                default:
                        WARNING("Invalid key pressed '", key, "' (MemViewerKeyboard).");
	}

	glutPostRedisplay ();
	
	return;
}



// ----------------------------------------------------------------------
// MemViewerSpecial - keyboard special key handler
//
// Inputs: key - the keyboard button
//         (x, y) - the mouse position
//
// Outputs: none
// ----------------------------------------------------------------------

GLvoid MemViewerSpecial (GLint key, GLint x, GLint y)
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
                        WARNING("Invalid key pressed '", key, "' (MemViewerSpecial).");
	}

	glutPostRedisplay ();

	return;
}


// ----------------------------------------------------------------------
// MemViewerInitialize - initializes the viewer module
//
// Inputs: none
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerInitialize ()
{
        if (g_eMemViewerStatus == INITIALIZED)
                return (-1);
        g_eMemViewerStatus = INITIALIZED;
	
	g_pNeuronMemory = new GLubyte *[g_nNeuronMemorySize];
	g_fltNeuronAssociatedValues = new GLfloat [g_nNeuronMemorySize];

	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize (g_nMemViewerWindowWidth, g_nMemViewerWindowHeight);
	glutInitWindowPosition (100, 100);

        if ((g_nMemViewerWindowId = glutCreateWindow (MEM_VIEWER_WINDOW_TITLE)) == 0)
        {
                Error("Cannot create memory viewer window (MemViewerInitialize)", "", "");
                return (-1);
        }

	glClearColor (.0f, .0f, .0f, .0f);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT,GL_NICEST);
	glPointSize (2);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glShadeModel(GL_FLAT);
	
	glutDisplayFunc (MemViewerDisplay);
	glutReshapeFunc (MemViewerReshape);
	glutMouseFunc (MemViewerMouse);
	glutMotionFunc (MemViewerMotion);
	glutKeyboardFunc (MemViewerKeyboard);
	glutSpecialFunc (MemViewerSpecial);

        return (0);
}



// ----------------------------------------------------------------------
// MemViewerQuit - exits the viewer module
//
// Inputs: none
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerQuit ()
{
        if (g_eMemViewerStatus == UNINITIALIZED)
                return (-1);
        g_eMemViewerStatus = UNINITIALIZED;

        glutDestroyWindow (g_nMemViewerWindowId);
	  
	delete g_pNeuronMemory;
	delete g_fltNeuronAssociatedValues;
     
        return (0);
}



// ----------------------------------------------------------------------
// MemViewerUpdateMemory -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerUpdateMemory (GLubyte *pattern, GLfloat associated_value)
{
        GLfloat x, y;
	
	if (g_eMemViewerStatus == UNINITIALIZED)
                return (-1);

	if (g_nStoredPatternsNumber >= g_nNeuronMemorySize)
	{
		Error("Full memory (MemViewerUpdateMemory)", "", "");
		return (-1);
	}
	
	g_pNeuronMemory[g_nStoredPatternsNumber] = pattern;
	g_fltNeuronAssociatedValues[g_nStoredPatternsNumber] = associated_value;
	
	MemViewerCalculatePosition (&x, &y, pattern);
	
	if (x < g_fltMinX)
		g_fltMinX = x;
	else if (x > g_fltMaxX)
		g_fltMaxX = x;
	
	if (y < g_fltMinY)
		g_fltMinY = y;
	else if (y > g_fltMaxY)
		g_fltMaxY = y;
	
	g_nStoredPatternsNumber++;
	
	//glutPostWindowRedisplay (g_nMemViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
// MemViewerClearMemory -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerClearMemory ()
{
        if (g_eMemViewerStatus == UNINITIALIZED)
                return (-1);
	
	g_fltMinX = g_fltMinY = FLT_MAX;
	g_fltMaxX = g_fltMaxY = FLT_MIN;
	
	g_nStoredPatternsNumber = 0;
	
	//glutPostWindowRedisplay (g_nMemViewerWindowId);

        return (0);
}



// ----------------------------------------------------------------------
// MemViewerRefresh -
//
// Inputs:
//
// Outputs: 0 if OK, -1 otherwise
// ----------------------------------------------------------------------

GLint MemViewerRefresh ()
{
        if (g_eMemViewerStatus == UNINITIALIZED)
                return (-1);
		
	glutPostWindowRedisplay (g_nMemViewerWindowId);

        return (0);
}
