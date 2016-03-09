#include "target_model.h"


float g_fltAlpha = .0f, g_fltBeta = .0f, g_fltDistance = 50.0f;
int g_nTargetModelWindow = 0;
int g_nWindowWidth, g_nWindowHeight;

/*
********************************************************
* Function: ConvertSphe2Rect                           *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void ConvertSphe2Rect (float *p_fltX, float *p_fltY, float *p_fltZ, float fltAlfa, float fltBeta, float fltDistance)
{
        float aux;
        
        *p_fltZ = fltDistance * sin(fltBeta);
        aux = fltDistance * cos(fltBeta);
        *p_fltX = aux * cos(fltAlfa);
        *p_fltY = aux * sin(fltAlfa);
        
        return;
}


/*
********************************************************
* Function: GetTargetWindowCoordinates                 *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void GetTargetWindowCoordinates (int *x, int *y)
{
        GLdouble winx, winy, winz;
        GLdouble objx, objy, objz;
        GLdouble modelMatrix[16], projMatrix[16];
        GLint viewport[4];

        glutSetWindow (g_nTargetModelWindow);
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
        
        objx = 5.0f;
        objy = 5.0f;
        objz = -5.0f;
        
        gluProject(objx, objy, objz, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
               
        *x = (int) (winx + .5f);
        *y = (int) (winy + .5f);
        
        return;
}

/*
********************************************************
* Function: UpdateTargetModel                	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void UpdateTargetModel (float fltAlpha, float fltBeta, float fltDistance)
{
        g_fltAlpha = M_PI * fltAlpha / 180.0f;
        g_fltBeta = M_PI * fltBeta / 180.0f;
        g_fltDistance = fltDistance;
        
        glutSetWindow (g_nTargetModelWindow);
        TargetModelDisplay ();
        
        return;       
}
        


/*
********************************************************
* Function: TargetModelDisplay                	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void TargetModelDisplay (void)
{	
        float eyex, eyey, eyez;
        float centerx, centery, centerz;
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glViewport (0, 0, g_nWindowWidth, g_nWindowHeight);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0f, (GLfloat) g_nWindowWidth / (GLfloat) g_nWindowHeight, 1.0f, 100.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	ConvertSphe2Rect (&eyex, &eyey, &eyez, g_fltAlpha+M_PI/4.0f, g_fltBeta+M_PI/6.0f, g_fltDistance);

	centerx = centery = centerz = .0f;
	
	gluLookAt (eyex, eyey, -eyez, centerx, centery, centerz, .0f, 1.0f, .0f);
	
	glColor3f (.5f, .5f, .5f);
        glutSolidCube (10.0f);
        
        glutSwapBuffers ();
        glFlush();
        
        return;
}



/*
********************************************************
* Function: TargetModelReshape                	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void TargetModelReshape (int w, int h)
{
	g_nWindowWidth = w;
        g_nWindowHeight = h;
	
	return;
}



/*
********************************************************
* Function: GetTargetPattern                 	       *
* Description: 				 	       *
* Inputs: 					       *
* Output: none  				       *
********************************************************
*/

void GetTargetPattern (INPUT_DESC *input)
{	
	int p_nViewport[4]; 
	int nWidth, nHeight;
	static int nPreviusWidth = 0, nPreviusHeight = 0;
	static GLubyte *pScreenPixels = (GLubyte *) NULL;
	FILE *pImageFile = NULL;
	int i, j;
	
	glutSetWindow(g_nTargetModelWindow);

        glGetIntegerv (GL_VIEWPORT, p_nViewport);
	nWidth = p_nViewport[2];
	nHeight = p_nViewport[3];
	
	if ((nWidth != nPreviusWidth) || (nHeight != nPreviusHeight))
	{
		free (pScreenPixels);
		if ((pScreenPixels = (GLubyte *) malloc (3 * nWidth * nHeight * sizeof (GLubyte))) == (GLubyte *) NULL)
		{
			Erro ("Cannot allocate more memory", "", "");
			return;
		}
		nPreviusWidth = nWidth;
		nPreviusHeight = nHeight;
	}
	
	glReadBuffer (GL_FRONT);
	glEnable(GL_READ_BUFFER);
	glReadPixels(0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, pScreenPixels); 
	glDisable(GL_READ_BUFFER);
	
	for (j = 0; j < input->wh; j++)
	{
		for (i = 0; i < input->ww; i++)
		{
                        if ((i > nWidth) && (j > nHeight))
                                continue;
			input->image[3 * (i + j * input->tfw) + 0] = pScreenPixels[3 * (i + j * nWidth) + 0];
			input->image[3 * (i + j * input->tfw) + 1] = pScreenPixels[3 * (i + j * nWidth) + 1];
			input->image[3 * (i + j * input->tfw) + 2] = pScreenPixels[3 * (i + j * nWidth) + 2];
                }
        }	
        
        input->up2date = 0;

	return;
}


/*
********************************************************
* Function: TargetModelInitialize 		       *
* Description:  				       *
* Inputs: none  				       *
* Output:					       *
********************************************************
*/

void TargetModelInitialize (void)
{
        GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat mat_shininess[] = { 50.0f };
        GLfloat light_position[] = { 10.0f, 20.0f, 30.0f, 0.0f };

        glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glClearColor (.0f, .0f, .0f, .0f);

	glutInitWindowSize (256, 256);
	glutInitWindowPosition (0, 0);
	
        g_nTargetModelWindow = glutCreateWindow("Training Target");
        
        glClearColor (.0f, .0f, .0f, .0f);
        
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);
   
	glutDisplayFunc(TargetModelDisplay);
	glutReshapeFunc(TargetModelReshape);

	return;
}
