#include "camera.hpp"



// ----------------------------------------------------------------------
// TCamera::TCamera - creates a camera object
//
// Inputs: fltAperture - the camera aperture
//         fltAspectRatio - the camera image aspect ratio
//         [fltNear, fltFar]  - the depth range
//
// Outputs: none
// ----------------------------------------------------------------------

TCamera::TCamera (float fltAperture, float fltAspectRatio, float fltNear, float fltFar)
{
	// Sets the camera attributes
	aperture = fltAperture;
	aspectratio = fltAspectRatio;
	near  = fltNear;
	far = fltFar;
	
        // Sets the default camera orientation
        position.x = -100.0f;
        position.y = .0f;
        position.z = .0f;

        fixationpoint.x = .0f;
        fixationpoint.y = .0f;
        fixationpoint.z = .0f;

        viewupdirection[0] = .0f;
        viewupdirection[1] = .0f;
        viewupdirection[2] = 1.0f;
        
	return;
}



// ----------------------------------------------------------------------
// TCamera::~TCamera - destroys the camera struct
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

TCamera::~TCamera (void)
{
	return;
}



// ----------------------------------------------------------------------
// TCamera::GetFocalLength - gets the camera focal length
//
// Inputs: none
//
// Outputs: the focal distance in pixels
// ----------------------------------------------------------------------

float TCamera::GetFocalLength (void)
{
	return (focallength);
}



// ----------------------------------------------------------------------
// TCamera::ResizeImage - resizes the camera image accordly to the new viewport dimentions
//
// Inputs: nWindowWidth - the new window width
//         nWindowHeight - the new window height
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::ResizeImage (int nWindowWidth, int nWindowHeight)
{
        // Tests if the camera image aspect ratio is less or equal the viewport aspect ratio
        if (aspectratio >= ((float) nWindowWidth / (float) nWindowHeight))
        {
                imagewidth = nWindowWidth;
                imageheight = (int) ((float) nWindowWidth / aspectratio + .5f);
        }
	else
	{
                imagewidth = (int) (aspectratio * (float) nWindowHeight + .5f);
                imageheight = nWindowHeight;
        }

        if (aspectratio != ((float) nWindowWidth / (float) nWindowHeight))
        {
                glutReshapeWindow (imagewidth, imageheight);
                return;
        }
        
        // Configures the camera viewport
       	glViewport (0, 0, (GLsizei) imageheight, (GLsizei) imagewidth);

        // Calculates the new focal distance
	focallength = (float) (imagewidth / 2) / TAN(M_PI / 180.0f  * aperture / 2.0f);
	
	return;
}



// ----------------------------------------------------------------------
// TCamera::ConfigureVisualizationVolume - configures the OpenGL visualization
// volume based on the camera pinhole perspective projection model
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::ConfigureVisualizationVolume (void)
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	gluPerspective (aperture, aspectratio, near, far);

        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	gluLookAt (position.x, position.y, position.z,
                   fixationpoint.x, fixationpoint.y, fixationpoint.z,
                   viewupdirection[0], viewupdirection[1], viewupdirection[2]);

        return;
}



// ----------------------------------------------------------------------
// TCamera::Zoom - changes the camera zoom
//
// Inputs: fltZoom - a zoom factor
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::Zoom (float fltZoom)
{
        float fltAlpha, fltBeta, fltDistance;

        ConvertRect2Sphe (&fltAlpha, &fltBeta, &fltDistance, position.x, position.y, position.z);

        fltDistance *= fltZoom;

	ConvertSphe2Rect (&position.x, &position.y, &position.z, fltAlpha, fltBeta, fltDistance);

	return;
}



// ----------------------------------------------------------------------
// TCameraRotateAroundFixationPoint - rotates the camera around the fixation point
//
// Inputs: fltDeltaAlpha - the angle increment around Z axis
//         fltDeltaBeta - the angle increment from XY plane
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::RotateAroundFixationPoint (float fltDeltaAlpha, float fltDeltaBeta)
{
        float fltAlpha, fltBeta, fltDistance;
        
        ConvertRect2Sphe (&fltAlpha, &fltBeta, &fltDistance, position.x, position.y, position.z);

        fltAlpha += fltDeltaAlpha;
        fltBeta += fltDeltaBeta;

        //if (fltBeta > M_PI / 3.0f)
        //        fltBeta = M_PI / 3.0f;
        //else if (fltBeta < -M_PI / 3.0f)
        //        fltBeta = -M_PI / 3.0f;
                
	ConvertSphe2Rect (&position.x, &position.y, &position.z, fltAlpha, fltBeta, fltDistance);

	return;
}



// ----------------------------------------------------------------------
// TCameraRotate - rotates the camera around the fixation point
//
// Inputs: fltDeltaAlpha - the angle increment around Z axis
//         fltDeltaBeta - the angle increment from XY plane
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::Rotate (float fltDeltaAlpha, float fltDeltaBeta)
{
        float fltAlpha, fltBeta, fltDistance;
        
        ConvertRect2Sphe (&fltAlpha, &fltBeta, &fltDistance, fixationpoint.x, fixationpoint.y, fixationpoint.z);

        fltAlpha += fltDeltaAlpha;
        fltBeta += fltDeltaBeta;
	ConvertSphe2Rect (&fixationpoint.x, &fixationpoint.y, &fixationpoint.z, fltAlpha, fltBeta, fltDistance);

	return;
}



// ----------------------------------------------------------------------
// TCamera::SetPosition - sets the camera position
//
// Inputs: (fltX, fltY, fltZ) - the new camera position coordinates
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::SetPosition (float fltX, float fltY, float fltZ)
{
	position.x = fltX;
	position.y = fltY;
	position.z = fltZ;
		
	return;
}



// ----------------------------------------------------------------------
// TCamera::SetFixationPoint - sets the camera fixation point
//
// Inputs: (fltX, fltY, fltZ) - the new fixation point coordinates
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::SetFixationPoint (float fltX, float fltY, float fltZ)
{
	fixationpoint.x = fltX;
	fixationpoint.y = fltY;
	fixationpoint.z = fltZ;

	return;
}



// ----------------------------------------------------------------------
// TCamera::SetViewUpDirection - sets the camera view up direction
//
// Inputs: p_fltViewUpDirection - the camera view up direction
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::SetViewUpDirection (float *p_fltViewUpDirection)
{
        int i;
        
        for (i = 0; i < 3; i++)
                viewupdirection[i] = p_fltViewUpDirection[i];

	return;
}



// ----------------------------------------------------------------------
// TCamera::Draw - draws the camera in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TCamera::Draw (void)
{
	glPushMatrix ();
	
	glColor3f (.5f, .5f, .5f);
	glTranslatef (position.x, position.y, position.z);
	glRotatef (-90.0f, .0f, 1.0f, .0f);
	glutSolidCone (2.0, 4.0, 20, 20);
	
	glPopMatrix ();

	return;
}
