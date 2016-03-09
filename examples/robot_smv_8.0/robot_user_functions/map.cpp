#include "map.hpp"



// Global Variables


// ----------------------------------------------------------------------
// TMap::TMap - creates an empty map object
//
// Inputs: nWidth X nHeight - the map grid dimentions
//         [fltBetaMin, fltBetaMax] - the vertical angle range [deg]
//
// Outputs: none
// ----------------------------------------------------------------------

TMap::TMap (int nWidth, int nHeight, float fltBetaMin, float fltBetaMax, int nLayersNumber, float fltNear, float fltFar, float fltTerrainSize)
{

	int i, j;

	fltBetaMin *= M_PI / 180.0f;
	fltBetaMax *= M_PI / 180.0f;

	// Sets the map attributes

	width = nWidth;
	height = nHeight;

	betaMin = fltBetaMin;
	betaMax = fltBetaMax;

	near = fltNear;
	far = fltFar;

	Ki = (float) (nWidth - 1) / (2.0f * M_PI);
	Kj = (float) (nHeight - 1) / (fltBetaMax - fltBetaMin);

	currentLayer = 0;
	layersNumber = nLayersNumber;
	imagePlaneTex = 0;

	position.x = 0;
	position.y = 0;
	position.z = 0;

  // Allocates memory for the grids vector
  if ((grid = new TPoint *[layersNumber]) == NULL)
		Error ("Cannot allocate memory to map grid (TMap::TMap)", "", "");

	// Allocates memory for the map's grid
	if ((*grid = new TPoint[layersNumber * width * height]) == NULL)
		Error ("Cannot allocate memory to map grid (TMap::TMap)", "", "");

        for (i = 0; i < layersNumber; i++)
                grid[i] = &(grid[0][i * width * height]);

	// Allocates memory for the images vector
	if ((image = new GLubyte *[layersNumber]) == NULL)
		Error ("Cannot allocate memory to map image (TMap::TMap)", "", "");

	// Allocates memory for the map's image
	if ((*image = new GLubyte[layersNumber * 3 * width * height]) == NULL)
		Error ("Cannot allocate memory to map image (TMap::TMap)", "", "");

        for (i = 0; i < layersNumber; i++)
                image[i] = &(image[0][i * 3 * width * height]);

        // Allocates memory for the depth map vector
	if ((depthMap = new float *[layersNumber]) == NULL)
		Error ("Cannot allocate memory to detph map vector (TMap::TMap)", "", "");

        // Allocates memory for the map's image
	if ((*depthMap = new float[layersNumber * width * height])== NULL)
		Error ("Cannot allocate memory to depth map (TMap::TMap)", "", "");

        for (i = 0; i < layersNumber; i++)
                depthMap[i] = &(depthMap[0][i * width * height]);

	// Initializes the map
	for (j = 0; j < height; j++)
	{
                for (i = 0; i < width; i++)
                {
                        depthMap[currentLayer][i + j * width] = UNKNOWN;
			image[currentLayer][3 * (i + j * width) + 0] = image[currentLayer][3 * (i + j * width) + 1] = image[currentLayer][3 * (i + j * width) + 2] = 0;

			/*depthMap[currentLayer][i + j * width] = 20.0f;
                        ConvertSphe2Rect (&grid[currentLayer][i + j * width].x,
                                          &grid[currentLayer][i + j * width].y,
                                          &grid[currentLayer][i + j * width].z,
                                          GetAlpha (i),
                                          GetBeta (j),
                                          depthMap[currentLayer][i + j * width]);
			image[currentLayer][3 * (i + j * width) + 0] = (GLubyte) (127.5f + 127.5f * COS(5.0f * GetAlpha (i)));
                        image[currentLayer][3 * (i + j * width) + 1] = (GLubyte) (127.5f + 127.5f * COS(5.0f * GetAlpha (i)));
                        image[currentLayer][3 * (i + j * width) + 2] = (GLubyte) (127.5f + 127.5f * COS(5.0f * GetAlpha (i)));*/
                }
        }
	
        return;
}



// ----------------------------------------------------------------------
// TMap::~TMap - destroys the map struct, the grid vector and the image vector
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

TMap::~TMap (void)
{

	delete *grid;
	delete grid;
	delete *image;
	delete image;
	delete *depthMap;
	delete depthMap;

        return;
}

// ----------------------------------------------------------------------
// TView::SetPosition - sets the object position
//
// Inputs: (fltX, fltY, fltZ) - the new view position coordinates
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::SetPosition (float fltX, float fltY, float fltZ)
{
	position.x = fltX;
	position.y = fltY;
	position.z = fltZ;
		
	return;
}

// ----------------------------------------------------------------------
// TMap::Load - loads a map from file
//
// Inputs: strFileName - the map file name
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TMap::Load (char *strFileName)
{
        FILE *pFile = NULL;
        int i;

        // Opens the map file
        if ((pFile = fopen (strFileName, "r")) == NULL)
        {
                Error ("Cannot open file '", strFileName, "' to read (TMap::Load).\n");
                return (false);
        }

        // Reads the map struct
        if (fread (this, sizeof (TMap), 1, pFile) != 1)
        {
                Error ("While writing file '", strFileName, "' (TMap::Load)");
                return (false);
        }
        
        // Allocates memory for the grids vector
        if ((grid = new TPoint *[layersNumber]) == NULL)
	{
		Error ("Cannot allocate memory to map grid (TMap::Load)", "", "");
		return (false);
	}

	 // Allocates memory for the map's grid
	if ((*grid = new TPoint[layersNumber * width * height]) == NULL)
	{
		Error ("Cannot allocate memory to map grid (TMap::Load)", "", "");
		return (false);
	}

	for (i = 0; i < layersNumber; i++)
                grid[i] = &(grid[0][i * width * height]);
                
	// Reads the map grid
        if (fread (grid[0], sizeof (TPoint), layersNumber * width * height, pFile) != (unsigned int) (layersNumber * width * height))
        {
                Error ("While reading file '", strFileName, "' (TMap::Load)");
                return (false);
        }

        // Allocates memory for the images vector
        if ((image = new GLubyte *[layersNumber]) == NULL)
	{
		Error ("Cannot allocate memory to map image (TMap::Load)", "", "");
		return (false);
	}

	// Allocates memory for the map's image
	if ((*image = new GLubyte[layersNumber * 3 * width * height]) == NULL)	{
		Error ("Cannot allocate memory to map image (TMap::Load)", "", "");
		return (false);
	}

	for (i = 0; i < layersNumber; i++)
                image[i] = &(image[0][i * 3 * width * height]);
      
	// Reads the map image
        if (fread (image[0], sizeof (GLubyte), 3 * width * height, pFile) != (unsigned int)(layersNumber * 3 * width * height))
        {
                Error ("While reading file '", strFileName, "' (TMap::Load)");
                return (false);
        }
     
        // Closes the map file
        if (fclose (pFile))
        {
                Error ("Cannot close file '", strFileName, "' (TMap::Load)");
                return (false);
        }
        
        return (true);
}



// ----------------------------------------------------------------------
// TMap::Save - saves a map in file
//
// Inputs: strFileName - the map file name
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TMap::Save (char *strFileName)
{
        FILE *pFile = NULL;
 
        if ((pFile = fopen (strFileName, "w")) == NULL)
        {
                Error ("Cannot open file '", strFileName, "' to write (TMap::Save)");
                return (false);
        }

        if (fwrite (this, sizeof (TMap), 1, pFile) != 1)
        {
                Error ("While writing file '", strFileName, "' (TMap::Save)");
                return (false);
        }
        
        if (fwrite (grid[0], sizeof (TPoint), layersNumber * width * height, pFile) != (unsigned int)(layersNumber * width * height))
        {
                Error ("While writing file '", strFileName, "' (TMap::Save)");
                return (false);
        }

        if (fwrite (image[0], sizeof (GLubyte), layersNumber * 3 * width * height, pFile) != (unsigned int)(layersNumber * 3 * width * height))
        {
                Error ("While writing file '", strFileName, "' (TMap::Save)");
                return (false);
        }
     
        if (fclose (pFile))
        {
                Error ("Cannot close file '", strFileName, "' (TMap::Save)");
                return (false);
        }

        return (true);
}



// ----------------------------------------------------------------------
// TMap::SavePointCloud - saves a map in file
//
// Inputs: strFileName - the map file name
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TMap::SavePointCloud (char *strFileName)
{
        FILE *pFile = NULL;
 
        if ((pFile = fopen (strFileName, "w")) == NULL)
        {
                Error ("Cannot open file '", strFileName, "' to write (TMap::SavePointCloud)");
                return (false);
        }
        
        for (int i = 0; i < width * height; i++)
            if (depthMap[currentLayer][i] != UNKNOWN)
               fprintf (pFile, "%e %e %e %d %d %d\n", grid[currentLayer][i].x, grid[currentLayer][i].y, grid[currentLayer][i].z,
                                                      image[currentLayer][3*i + 0], image[currentLayer][3*i + 1], image[currentLayer][3*i + 2]);
     
        if (fclose (pFile))
        {
                Error ("Cannot close file '", strFileName, "' (TMap::SavePointCloud)");
                return (false);
        }

        return (true);
}



// ----------------------------------------------------------------------
// TMap::SaveImage - saves the map image in file
//
// Inputs: strFileName - the image file name
//
// Outputs: true if OK, false otherwise
// ----------------------------------------------------------------------

bool TMap::SaveImage (char *strFileName)
{
        FILE *pFile = NULL;
        int i, j;

        if ((pFile = fopen (strFileName, "w")) == NULL)
        {
                Error ("Cannot open file '", strFileName, "' to write (TMap::SaveImage)");
                return (false);
        }
     
	fprintf (pFile, "P3\n");
	fprintf (pFile, "# Map image\n");
	fprintf (pFile, "%d %d\n255\n", width, height);

	for (j = 0; j < height; j++)
		for (i = 0; i < width; i++)
			fprintf (pFile, "%d %d %d\n",
                        image[currentLayer][3 * (i + (height - 1 - j) * width) + 0],
                        image[currentLayer][3 * (i + (height - 1 - j) * width) + 1],
                        image[currentLayer][3 * (i + (height - 1 - j) * width) + 2]);

        if (fclose (pFile))
        {
                Error ("Cannot close file '", strFileName, "' (TMap::SaveImage)");
                return (false);
        }
     
        return (true);
}



// ----------------------------------------------------------------------
// TMap::Update - updates the map
//
// Inputs: (X, Y, Z) - the coordinates of the world point to be stored in map
//         (red, green, blue) - the point color in RGB color system
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Update (float X, float Y, float Z, GLubyte red, GLubyte green, GLubyte blue)
{
	int i, j;
        float fltAlpha, fltBeta, fltDistance;
                        
        ConvertRect2Sphe (&fltAlpha, &fltBeta, &fltDistance, X, Y, Z);
     
	i = (int) (Ki * (M_PI - fltAlpha) + .5f);
	j = (int) (Kj * (fltBeta - betaMin) + .5f);
	
	if ((i < 0) || (i >= width) || (j < 0) || (j >= height))
                return;
      
        depthMap[currentLayer][i + j * width] = fltDistance;

        grid[currentLayer][i + j * width].x = X;
        grid[currentLayer][i + j * width].y = Y;
        grid[currentLayer][i + j * width].z = Z;
                    
        image[currentLayer][3 * (i + j * width) + 0] = red;
        image[currentLayer][3 * (i + j * width) + 1] = green;
        image[currentLayer][3 * (i + j * width) + 2] = blue;

	return;
}



// ----------------------------------------------------------------------
// TMap::Erase - erases the map
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Erase (void)
{
	int i;
                      
        // Erases the map
        for (i = 0; i < width * height; i++)
	{
        	depthMap[currentLayer][i] = UNKNOWN;

        	image[currentLayer][3 * i + 0] = image[currentLayer][3 * i + 1] = image[currentLayer][3 * i + 2] = 0;
	}	

	return;
}



// ----------------------------------------------------------------------
// TMap::GetNextLayer - gets the next map layer
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::GetNextLayer (void)
{
        currentLayer = (currentLayer + 1) % layersNumber;
	return;
}



// ----------------------------------------------------------------------
// TMap::Draw - draws the map in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Draw (void)
{
	int i;

        glBegin(GL_POINTS);
        for (i = 0; i < height * width; i++)
	{
                if (depthMap[currentLayer][i] != UNKNOWN)
                {
                        glColor3ub (image[currentLayer][3 * i + 0], image[currentLayer][3 * i + 1], image[currentLayer][3 * i + 2]);
                        glVertex3f (grid[currentLayer][i].x + this->position.x, 
                                    grid[currentLayer][i].y + this->position.y, 
                                    grid[currentLayer][i].z + this->position.z);
                 }
        }
        glEnd ();

        return;
}



// ----------------------------------------------------------------------
// TMap::ShowImage - shows the map image in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::ShowImage (void)
{
        glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (.0f, 1.0f, .0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, imagePlaneTex);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *) image[currentLayer]);
	glBegin (GL_QUADS);
	glTexCoord2f ( .0f,  .0f); glVertex2f ( .0f, .5f + 2.0f * betaMin / M_PI);
	glTexCoord2f ( .0f, 1.0f); glVertex2f ( .0f, .5f + 2.0f * betaMax / M_PI);
	glTexCoord2f (1.0f, 1.0f); glVertex2f (1.0f, .5f + 2.0f * betaMax / M_PI);
	glTexCoord2f (1.0f,  .0f); glVertex2f (1.0f, .5f + 2.0f * betaMin / M_PI);
	glEnd ();
	glDisable (GL_TEXTURE_2D);

        return;
}



// ----------------------------------------------------------------------
// TMap::ShowDepthMap - shows the depth map in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::ShowDepthMap (void)
{
	int i;
	static GLubyte *pDepthPlane = NULL;
	float fltDepth, fltFixationPointDepth;

        if (pDepthPlane == NULL)
        {
                if ((pDepthPlane = new GLubyte[3 * width * height]) == NULL)
                {
                        Error ("Cannot allocate more memory (TMap::ShowDepthMap)", "", "");
                        return;
                }
        }

	fltFixationPointDepth = depthMap[currentLayer][width / 2 + height / 2 * width];

        for (i = 0, fltDepth = depthMap[currentLayer][i]; i < width * height; i++, fltDepth = depthMap[currentLayer][i])
        {
		if (fltDepth == UNKNOWN)
		{
			pDepthPlane[3 * i + 2] = 255;
                        pDepthPlane[3 * i + 0] = pDepthPlane[3 * i + 1] = 0;
		}
		else if (fltDepth <= near)
		{
                        pDepthPlane[3 * i + 0] = 255;
			pDepthPlane[3 * i + 1] = pDepthPlane[3 * i + 2] = 0;			
		}
		else if (fltDepth >= far)
                {
                        pDepthPlane[3 * i + 1] = 255;
                        pDepthPlane[3 * i + 0] = pDepthPlane[3 * i + 2] = 0;
                }
		else if (fltDepth < fltFixationPointDepth)
                {
                        pDepthPlane[3 * i + 0] = (GLubyte) (255.0f * (fltFixationPointDepth - fltDepth) / (fltFixationPointDepth - near));
                        pDepthPlane[3 * i + 1] = pDepthPlane[3 * i + 2] = 0;
                }
                else
                {
                        pDepthPlane[3 * i + 1] = (GLubyte) (255.0f * (fltDepth - fltFixationPointDepth) / (far - fltFixationPointDepth));
                        pDepthPlane[3 * i + 0] = pDepthPlane[3 * i + 2] = 0;
                }
        }

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (.0f, 1.0f, .0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, imagePlaneTex);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *) pDepthPlane);
	glBegin (GL_QUADS);
	glTexCoord2f ( .0f,  .0f); glVertex2f ( .0f, .5f + 2.0f * betaMin / M_PI);
	glTexCoord2f ( .0f, 1.0f); glVertex2f ( .0f, .5f + 2.0f * betaMax / M_PI);
	glTexCoord2f (1.0f, 1.0f); glVertex2f (1.0f, .5f + 2.0f * betaMax / M_PI);
	glTexCoord2f (1.0f,  .0f); glVertex2f (1.0f, .5f + 2.0f * betaMin / M_PI);
	glEnd ();
	glDisable (GL_TEXTURE_2D);

        return;
}



// ----------------------------------------------------------------------
// TMap::GetAlpha - calculates the alpha angle around Z axis from horizontal matrix index
//
// Inputs: i - the horizontal matrix index
//
// Outputs: the angle around Z axis
// ----------------------------------------------------------------------

float TMap::GetAlpha (int i)
{
        return (M_PI - (float) i / Ki);
}



// ----------------------------------------------------------------------
// TMap::GetBeta - scalculates the beta angle from XY plane from horizontal matrix index
//
// Inputs: j - the horizontal matrix index
//
// Outputs: the angle from XY plane
// ----------------------------------------------------------------------

float TMap::GetBeta (int j)
{
        return ((float) j / Kj + betaMin);
}
