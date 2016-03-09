#include "map.hpp"


#define OCCUPANCY_GRID_WIDTH 	64
#define OCCUPANCY_GRID_HEIGHT 	64


// Global Variables
TList g_pFrontierList;
bool g_bValidPath = false;
float g_p_fltPathSearchTree [4 * MAX_NODES];

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
	
        gridWidth = OCCUPANCY_GRID_WIDTH;
        gridLength = OCCUPANCY_GRID_HEIGHT;
        areaBounds[0] = -fltTerrainSize / 2.0f;
        areaBounds[1] =  fltTerrainSize / 2.0f;
        areaBounds[2] = -fltTerrainSize / 2.0f;
        areaBounds[3] =  fltTerrainSize / 2.0f;
	increment = .01f;
	robotX = .0f;
	robotY = .0f;
	robotAlpha = M_PI / 2.0f;

	// Allocates memory for the occupancy map
	if ((occupancyGrid = new float[gridWidth * gridLength])== NULL)
		Error ("Cannot allocate memory to occupancy map (TMap::TMap)", "", "");

	// Initializes the occupancy grid
	for (i = 0; i < gridWidth * gridLength; i++)
		occupancyGrid[i] = .5f;

	// Allocates memory for the evidence map
	if ((evidenceGrid = new unsigned int[gridWidth * gridLength])== NULL)
		Error ("Cannot allocate memory to evidence map (TMap::TMap)", "", "");
	
	// Allocates memory for the frontier map
	if ((frontierGrid = new TFrontierCell[gridWidth * gridLength])== NULL)
		Error ("Cannot allocate memory to frontier map (TMap::TMap)", "", "");
		
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
	delete occupancyGrid;
    	delete evidenceGrid;
        delete frontierGrid;

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
        
	UpdateOccupancyGrid (X, Y, Z);
	
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
	
	// Initializes the occupancy grid
	for (i = 0; i < gridWidth * gridLength; i++)
		occupancyGrid[i] = .5f;
	
	robotX = .0f;
	robotY = .0f;
	robotAlpha = M_PI / 2.0f;
	
	return;
}



// ----------------------------------------------------------------------
// TMap::Rotate - rotates the map
//
// Inputs: fltDeltaAngle - the rotation angle around Z axis [deg]
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Rotate (float fltDeltaAlfa)
{
        register int i, j, io;
        int nNextLayer;
	float fltAlpha, fltBeta, fltDistance, fltDelta;

        nNextLayer = (currentLayer + 1) % layersNumber;
	for (i = 0; i < width * height; i++)
        {
                depthMap[nNextLayer][i] = UNKNOWN;
                image[nNextLayer][3 * i + 0] = image[nNextLayer][3 * i + 1] = image[nNextLayer][3 * i + 2] = 0;
        }
	
	fltDeltaAlfa *= M_PI / 180.0f;
	
	fltDelta = Ki * fltDeltaAlfa;
	fltDelta += (fltDelta >= .0f) ? .5f : -.5f;
	
	for (j = 0; j < height; j++)
	{
                for (i = 0; i < width; i++)
                {
                        if (depthMap[currentLayer][i + j * width] == UNKNOWN)
				continue;
				                        
			ConvertRect2Sphe (&fltAlpha, 
					  &fltBeta, 
					  &fltDistance, 
					  grid[currentLayer][i + j * width].x, 
					  grid[currentLayer][i + j * width].y, 
					  grid[currentLayer][i + j * width].z);
			
			io = ((io = i - (int) fltDelta) >= 0) ? io % width : width + io;
				
                        depthMap[nNextLayer][io + j * width] = fltDistance;
                        
                        image[nNextLayer][3 * (io + j * width) + 0] = image[currentLayer][3 * (i + j * width) + 0];
                        image[nNextLayer][3 * (io + j * width) + 1] = image[currentLayer][3 * (i + j * width) + 1];
                        image[nNextLayer][3 * (io + j * width) + 2] = image[currentLayer][3 * (i + j * width) + 2];

                        ConvertSphe2Rect (&grid[nNextLayer][io + j * width].x,
                                          &grid[nNextLayer][io + j * width].y,
                                          &grid[nNextLayer][io + j * width].z,
                                          fltAlpha + fltDeltaAlfa,
                                          fltBeta,
                                          fltDistance);
                }
        }
	
        currentLayer = nNextLayer;

	// Updates the robot occupancy map orientation
	robotAlpha -= fltDeltaAlfa;

	return;
}



// ----------------------------------------------------------------------
// TMap::Forward - moves the map
//
// Inputs: fltStep - the displacement along X axis [cm]
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Forward (float fltStep)
{
        register int i, j, io, jo;
        float fltNewAlpha, fltNewBeta, fltNewDistance;
        int nNextLayer;
        
        nNextLayer = (currentLayer + 1) % layersNumber;
        for (i = 0; i < width * height; i++)
        {
                depthMap[nNextLayer][i] = UNKNOWN;
                image[nNextLayer][3 * i + 0] = image[nNextLayer][3 * i + 1] = image[nNextLayer][3 * i + 2] = 0;
        }
                
        for (j = 0; j < height; j++)
        {
                for (i = 0; i < width; i++)
                {
			if (depthMap[currentLayer][i + j * width] == UNKNOWN)
				continue;
			
                        ConvertRect2Sphe (&fltNewAlpha, 
					  &fltNewBeta, 
					  &fltNewDistance, 
					  grid[currentLayer][i + j * width].x + fltStep, 
					  grid[currentLayer][i + j * width].y, 
					  grid[currentLayer][i + j * width].z);
                       
                        io = (int) (Ki * (M_PI - fltNewAlpha) + .5f);
                        jo = (int) (Kj * (fltNewBeta - betaMin) + .5f);
                        
                        if ((io < 0) || (io >= width) || (jo < 0) || (jo >= height))
                                continue;
                        
                        depthMap[nNextLayer][io + jo * width] = fltNewDistance;
			
                        image[nNextLayer][3 * (io + jo * width) + 0] = image[currentLayer][3 * (i + j * width) + 0];
                        image[nNextLayer][3 * (io + jo * width) + 1] = image[currentLayer][3 * (i + j * width) + 1];
                        image[nNextLayer][3 * (io + jo * width) + 2] = image[currentLayer][3 * (i + j * width) + 2];

                        grid[nNextLayer][io + jo * width].x = grid[currentLayer][i + j * width].x + fltStep;
                        grid[nNextLayer][io + jo * width].y = grid[currentLayer][i + j * width].y;
                        grid[nNextLayer][io + jo * width].z = grid[currentLayer][i + j * width].z;
                }
        }
        
        currentLayer = nNextLayer;
	
	// Updates the robot occupancy map position
        robotX -= fltStep * cos (robotAlpha);
	robotY -= fltStep * sin (robotAlpha);
	
	return;
}



// ----------------------------------------------------------------------
// TMap::Translate - translates the map
//
// Inputs: (fltStepX, fltStepY) - the displacement along X an Y axis, respectively ([cm], [cm])
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::Translate (float fltStepX, float fltStepY)
{
        register int i, j, io, jo;
        float fltNewAlpha, fltNewBeta, fltNewDistance;
        int nNextLayer;
        
        nNextLayer = (currentLayer + 1) % layersNumber;
        for (i = 0; i < width * height; i++)
        {
                depthMap[nNextLayer][i] = UNKNOWN;
                image[nNextLayer][3 * i + 0] = image[nNextLayer][3 * i + 1] = image[nNextLayer][3 * i + 2] = 0;
        }
                
        for (j = 0; j < height; j++)
        {
                for (i = 0; i < width; i++)
                {
			if (depthMap[currentLayer][i + j * width] == UNKNOWN)
				continue;
			
                        ConvertRect2Sphe (&fltNewAlpha, 
					  &fltNewBeta, 
					  &fltNewDistance, 
					  grid[currentLayer][i + j * width].x + fltStepX, 
					  grid[currentLayer][i + j * width].y + fltStepY, 
					  grid[currentLayer][i + j * width].z);
                        
                        io = (int) (Ki * (M_PI - fltNewAlpha) + .5f);
                        jo = (int) (Kj * (fltNewBeta - betaMin) + .5f);
                        
                        if ((io < 0) || (io >= width) || (jo < 0) || (jo >= height))
                                continue;
                        
                        depthMap[nNextLayer][io + jo * width] = fltNewDistance;
			
                        image[nNextLayer][3 * (io + jo * width) + 0] = image[currentLayer][3 * (i + j * width) + 0];
                        image[nNextLayer][3 * (io + jo * width) + 1] = image[currentLayer][3 * (i + j * width) + 1];
                        image[nNextLayer][3 * (io + jo * width) + 2] = image[currentLayer][3 * (i + j * width) + 2];

                        grid[nNextLayer][io + jo * width].x = grid[currentLayer][i + j * width].x + fltStepX;
                        grid[nNextLayer][io + jo * width].y = grid[currentLayer][i + j * width].y + fltStepY;
                        grid[nNextLayer][io + jo * width].z = grid[currentLayer][i + j * width].z;
                }
        }
        
        currentLayer = nNextLayer;

	// Updates the robot occupancy map position
	robotX -= fltStepX * cos (robotAlpha) - fltStepY * sin (robotAlpha);
	robotY -= fltStepX * sin (robotAlpha) + fltStepY * cos (robotAlpha);
        
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
                        glVertex3f (grid[currentLayer][i].x, grid[currentLayer][i].y, grid[currentLayer][i].z);
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
// TMap::ShowOccupancyMap - shows the occupancy map in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::ShowOccupancyMap (void)
{
        int nNodeIndex;
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (areaBounds[0], areaBounds[1], areaBounds[2], areaBounds[3], .0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, imagePlaneTex);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, gridWidth, gridLength, 0, GL_LUMINANCE, GL_FLOAT, (void *) occupancyGrid);
	glBegin (GL_QUADS);
	glTexCoord2f ( .0f,  .0f); glVertex3f (areaBounds[0], areaBounds[2], -.1f);
	glTexCoord2f ( .0f, 1.0f); glVertex3f (areaBounds[0], areaBounds[3], -.1f);
	glTexCoord2f (1.0f, 1.0f); glVertex3f (areaBounds[1], areaBounds[3], -.1f);
	glTexCoord2f (1.0f,  .0f); glVertex3f (areaBounds[1], areaBounds[2], -.1f);
	glEnd ();
	glDisable (GL_TEXTURE_2D);
	
	if (g_bValidPath)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable (GL_LINE_STIPPLE);
		glLineStipple (1, 0x0F0F);
		glBegin(GL_LINE_STRIP);
		for (nNodeIndex = 0; nNodeIndex < MAX_NODES; nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3])
 		{
			glVertex2f(g_p_fltPathSearchTree[4 * nNodeIndex + 0], g_p_fltPathSearchTree[4 * nNodeIndex + 1]);
			if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE) break;
		}
   		glEnd();
		glDisable (GL_LINE_STIPPLE);

		glPointSize(5);
		glBegin(GL_POINTS); 
		for (nNodeIndex = 0; nNodeIndex < MAX_NODES; nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3])
 		{
			glVertex2f(g_p_fltPathSearchTree[4 * nNodeIndex + 0], g_p_fltPathSearchTree[4 * nNodeIndex + 1]);
			if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE) break;
		}
		glEnd ();
   		glPointSize(2);
	}
		
	glTranslatef (robotX, robotY, .0f);
	glRotatef (180.0f / M_PI * robotAlpha, .0f, .0f, 1.0f);
	
        return;
}




// ----------------------------------------------------------------------
// TMap::ShowEvidenceMap - shows the evidence map in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::ShowEvidenceMap (void)
{
        int nNodeIndex;
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (areaBounds[0], areaBounds[1], areaBounds[2], areaBounds[3], .0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, imagePlaneTex);
#ifndef WINDOWS
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, gridWidth, gridLength, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (void *) evidenceGrid);
#else
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, gridWidth, gridLength, 0, GL_RGBA, GL_UNSIGNED_INT, (void *) evidenceGrid);
#endif
	glBegin (GL_QUADS);
	glTexCoord2f ( .0f,  .0f); glVertex3f (areaBounds[0], areaBounds[2], -.1f);
	glTexCoord2f ( .0f, 1.0f); glVertex3f (areaBounds[0], areaBounds[3], -.1f);
	glTexCoord2f (1.0f, 1.0f); glVertex3f (areaBounds[1], areaBounds[3], -.1f);
	glTexCoord2f (1.0f,  .0f); glVertex3f (areaBounds[1], areaBounds[2], -.1f);
	glEnd ();
	glDisable (GL_TEXTURE_2D);
	
	if (g_bValidPath && g_p_fltPathSearchTree[3] != NULL_NODE)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable (GL_LINE_STIPPLE);
		glLineStipple (1, 0x0F0F);
		glBegin(GL_LINE_STRIP);
		for (nNodeIndex = 0; nNodeIndex < MAX_NODES; nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3])
 		{
			glVertex2f(g_p_fltPathSearchTree[4 * nNodeIndex + 0], g_p_fltPathSearchTree[4 * nNodeIndex + 1]);
			if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE) break;
		}
   		glEnd();
		glDisable (GL_LINE_STIPPLE);

		glPointSize(5);
		glBegin(GL_POINTS); 
		for (nNodeIndex = 0; nNodeIndex < MAX_NODES; nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3])
 		{
			glVertex2f(g_p_fltPathSearchTree[4 * nNodeIndex + 0], g_p_fltPathSearchTree[4 * nNodeIndex + 1]);
			if (g_p_fltPathSearchTree[4 * nNodeIndex + 3] == NULL_NODE) break;
		}
		glEnd ();
   		glPointSize(2);
	}

	glTranslatef (robotX, robotY, .0f);
	glRotatef (180.0f / M_PI * robotAlpha, .0f, .0f, 1.0f);

        return;
}


// ----------------------------------------------------------------------
// TMap::GetFuzzyInput -
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::GetFuzzyInputs (double *p_dblFuzzyInputs, int nInputsNumber, float fltMinAlpha, float fltMaxAlpha, float fltFloor)
{
	int i, j, k;
	static int *p_nPointsPerQuadrant = NULL;
	float fltDepth;
	int nStartX, nEndX, nQuadrantSize;

	fltMinAlpha *= M_PI / 180.0f;
	fltMaxAlpha *= M_PI / 180.0f;
	
	// Calculates the effective map bounds
        nStartX = (int) (Ki * (M_PI - fltMaxAlpha) + .5f);
        nEndX = (int) (Ki * (M_PI - fltMinAlpha) + .5f);

	// Calculates the quadrants size
	nQuadrantSize = (nEndX - nStartX)/ nInputsNumber;

	if (p_nPointsPerQuadrant == NULL)
		p_nPointsPerQuadrant = new int[nInputsNumber];

	// Initializes the data vector and the total of points
	for (k = 0; k < nInputsNumber; k++)
        {
                p_nPointsPerQuadrant[k] = 0;
		p_dblFuzzyInputs[k] = 0.0;
        }

	// Calculates the avarage distance within each quadrant
	for (j = 0; j < height; j++)
	{
		for (i = nStartX; i < nEndX; i++)
		{
			k = (i - nStartX) / nQuadrantSize;	// Quadrant
			fltDepth = depthMap[currentLayer][i + j * width];

			if ((fltDepth == UNKNOWN) || (grid[currentLayer][i + j * width].z < fltFloor))
				continue;
			
			if (fltDepth > far)
				fltDepth = far;
			
			if (fltDepth < near)
				fltDepth = near;

			p_dblFuzzyInputs[k] += fltDepth;
			p_nPointsPerQuadrant[k]++;
		}
	}

	// Normalizes the results
	for (k = 0; k < nInputsNumber; k++)
	{
		if (p_nPointsPerQuadrant[k])
			p_dblFuzzyInputs[k] /= (double) p_nPointsPerQuadrant[k];
		else
			p_dblFuzzyInputs[k] = (double) UNKNOWN;
	}

        return;
}



// ----------------------------------------------------------------------
// TMap::UpdateOccupancyGrid - updates the occupancy grid
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::UpdateOccupancyGrid (float X, float Y, float Z)
{
	int io, jo, ir, jr, i, j;
	float xo ,yo, m;
	
	if ((Z < -16.0f) || (Z > .0f))
		return;
	
	// Rotation (inverse)
	xo = X * cos (robotAlpha) - Y * sin (robotAlpha) + robotX;
	yo = X * sin (robotAlpha) + Y * cos (robotAlpha) + robotY;
	
	io = (int) ((float) gridWidth  * (xo - areaBounds[0]) / (areaBounds[1] - areaBounds[0]) + .5f);
	jo = (int) ((float) gridLength * (yo - areaBounds[2]) / (areaBounds[3] - areaBounds[2]) + .5f);

	ir = (int) ((float) gridWidth  * (robotX - areaBounds[0]) / (areaBounds[1] - areaBounds[0]) + .5f);
	jr = (int) ((float) gridLength * (robotY - areaBounds[2]) / (areaBounds[3] - areaBounds[2]) + .5f);

	if ((io < 0) || (io >= gridWidth) || (jo < 0) || (jo >= gridLength))
        	return;
	
	if ((occupancyGrid[io + jo * gridWidth] += increment) > 1.0f)
		occupancyGrid[io + jo * gridWidth] = 1.0f;
	
	if  ((io != ir) && fabs (m = (float) (jo - jr) / (float) (io - ir)) < 1.0f)
	{
		if (io < ir)
		{
			for (i = ir, j = jr; i > io; i--, j = (int) (m * (float) (i - ir) + (float) jr + .5f))
				if ((occupancyGrid[i + j * gridWidth] -= increment) < .0f) { occupancyGrid[i + j * gridWidth] = .0f; }
		}
		else
		{
			for (i = ir, j = jr; i < io; i++, j = (int) (m * (float) (i - ir) + (float) jr + .5f))
				if ((occupancyGrid[i + j * gridWidth] -= increment) < .0f) { occupancyGrid[i + j * gridWidth] = .0f; }
		}
	}
	else if  (jo != jr)
	{
		m = (float) (io - ir) / (float) (jo - jr);
		
		if (jo < jr)
		{
			for (i = ir, j = jr; j > jo; j--, i = (int) (m * (float) (j - jr) + (float) ir + .5f))
				if ((occupancyGrid[i + j * gridWidth] -= increment) < .0f) { occupancyGrid[i + j * gridWidth] = .0f; }
		}
		else
		{
			for (i = ir, j = jr; j < jo; j++, i = (int) (m * (float) (j - jr) + (float) ir + .5f))
				if ((occupancyGrid[i + j * gridWidth] -= increment) < .0f) { occupancyGrid[i + j * gridWidth] = .0f; }
		}
	}
}



// ----------------------------------------------------------------------
// TMap::GenerateOccupancyGrid - generates the occupancy grid
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::GenerateOccupancyGrid (void)
{
	int i;
	
	// Calculates the occupancy grid
	for (i = 0; i < gridWidth * gridLength; i++)
	{
		if (depthMap[currentLayer][i] != UNKNOWN)
			UpdateOccupancyGrid (grid[currentLayer][i].x, grid[currentLayer][i].y, grid[currentLayer][i].z);
	}
}




// ----------------------------------------------------------------------
// TMap::ExpandCell - expands an occupied cell
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::ExpandCell (int io, int jo, int nHorizontalMargin, int nVerticalMargin)
{
	int i, j;
	
	for (j = (jo - nVerticalMargin); j <= (jo + nVerticalMargin); j++)
	{
		for (i = (io - nHorizontalMargin); i <= (io + nHorizontalMargin); i++)
		{
			if ((i < 0) || (i >= gridWidth) || (j < 0) || (j >= gridLength))
				continue;
			
			if (((i == io) && (j == jo)) || (evidenceGrid[i + j * gridWidth] == OCCUPIED_CELL))
				continue;
							
			evidenceGrid[i + j * gridWidth]	= WARNING_CELL;
		}
	}
}



// ----------------------------------------------------------------------
// TMap::HaveFreeAdjacentCell - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

bool TMap::HaveFreeAdjacentCell (int io, int jo)
{
	int i, j;
	
	// Expands the occupied cells
	for (j = jo - 1; j <= jo + 1; j++)
	{
		for (i = io - 1; i <= io + 1; i++)
		{
			if ((i < 0) || (i >= gridWidth) || (j < 0) || (j >= gridLength))
				continue;
			
			if (evidenceGrid[i + j * gridWidth] == FREE_CELL)
				return (true);
		}
	}
	
	return (false);
	
}



// ----------------------------------------------------------------------
// TMap::HaveCommonFreeAdjacentCells - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------
int g_p_nXPos[] = {-1, 0, 1, 1, 1, 0, -1 , -1};
int g_p_nYPos[] = {1, 1, 1, 0, -1, -1 , -1, 0};

bool TMap::HaveCommonFreeAdjacentCells (int io, int jo, int ko)
{
	int i, j, k, l;
	
	for (l = -(1 + ko % 2); l <= (1 + ko % 2); l++)
	{
		k = ((ko + l) & 0xFF) % 8;
		
		if (k == ko)
			continue;
			
		i = io + g_p_nXPos[k];
		j = jo + g_p_nYPos[k];
		
		if (evidenceGrid[i + j * gridWidth] == FREE_CELL)
			return (true);
	}
	
	return (false);
}	



// ----------------------------------------------------------------------
// TMap::LinkFrontierCells - 
//
// Inputs: none
//
// Outputs: the frontier size
// ----------------------------------------------------------------------

int TMap::LinkFrontierCells (int io, int jo, unsigned int nFrontierType, int *p_nFrontierCentroidX, int *p_nFrontierCentroidY)
{
	int i, j, k;
	int xo, yo;
	int nFrontierLength;
	
	if ((frontierGrid[io + jo * gridWidth].left != UNLINKED) || (frontierGrid[io + jo * gridWidth].right != UNLINKED))
		return (0);
		
	xo = io; yo = jo;
	
	*p_nFrontierCentroidX = io;
	*p_nFrontierCentroidY = jo;
	nFrontierLength = 1;

	while (frontierGrid[io + jo * gridWidth].left == UNLINKED)
	{
			
		for (k = 1; k < 8; k += 2)
		{
			i = io + g_p_nXPos[k];
			j = jo + g_p_nYPos[k];

			if ((i < 0) || (i >= gridWidth) || (j < 0) || (j >= gridLength))
				continue;

			if (evidenceGrid[i + j * gridWidth] != nFrontierType)
				continue;

			if (!HaveCommonFreeAdjacentCells (io, jo, k))
				continue;
			
			if ((frontierGrid[io + jo * gridWidth].left == UNLINKED) &&
			    (frontierGrid[i + j * gridWidth].right == UNLINKED) &&
			    (frontierGrid[io + jo * gridWidth].right != k))
			{
				frontierGrid[io + jo * gridWidth].left = k;
				frontierGrid[i + j * gridWidth].right = (k + 4) % 8;
				io = i; jo = j;
				(*p_nFrontierCentroidX) += io;
				(*p_nFrontierCentroidY) += jo;
				nFrontierLength++;
				break;
			}
		}
		
		if (k >= 8) break;
	};
	
	io = xo; jo = yo;

	while (frontierGrid[io + jo * gridWidth].right == UNLINKED)
	{
			
		for (k = 1; k < 8; k += 2)
		{
			i = io + g_p_nXPos[k];
			j = jo + g_p_nYPos[k];

			if ((i < 0) || (i >= gridWidth) || (j < 0) || (j >= gridLength))
				continue;

			if (evidenceGrid[i + j * gridWidth] != nFrontierType)
				continue;

			if (!HaveCommonFreeAdjacentCells (io, jo, k))
				continue;
			
			if ((frontierGrid[io + jo * gridWidth].right == UNLINKED) &&
			    (frontierGrid[i + j * gridWidth].left == UNLINKED) &&
			    (frontierGrid[io + jo * gridWidth].left != k))
			{
				frontierGrid[io + jo * gridWidth].right = k;
				frontierGrid[i + j * gridWidth].left = (k + 4) % 8;
				io = i; jo = j;
				(*p_nFrontierCentroidX) += io;
				(*p_nFrontierCentroidY) += jo;
				nFrontierLength++;
				break;
			}
		}
		
		if (k >= 8) break;
	};
	
	*p_nFrontierCentroidX = (int) ((float) *p_nFrontierCentroidX / (float) nFrontierLength + .5f);
	*p_nFrontierCentroidY = (int) ((float) *p_nFrontierCentroidY / (float) nFrontierLength + .5f);
	
	return (nFrontierLength);
}



// ----------------------------------------------------------------------
// TMap::GroupFrontierCells - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::GroupFrontierCells (unsigned int nFrontierType)
{
	int i, j;
	TFrontier *pFrontier = NULL;
	int nFrontierLength, nFrontierCentroidX, nFrontierCentroidY;
	
	for (i = 0; i < gridWidth * gridLength; i++)
		if (evidenceGrid[i] == nFrontierType)
			frontierGrid[i].right = frontierGrid[i].left = UNLINKED;

	for (j = 0; j < gridLength; j++)
	{
		for (i = 0; i < gridWidth; i++)
		{
			if (evidenceGrid[i + j * gridWidth] == nFrontierType)
			{
				if ((nFrontierLength = LinkFrontierCells (i, j, nFrontierType, &nFrontierCentroidX, &nFrontierCentroidY)) >= 1)
				{
					pFrontier = new TFrontier;
					pFrontier->type = nFrontierType;
					pFrontier->centroidX = nFrontierCentroidX;
					pFrontier->centroidY = nFrontierCentroidY;
					pFrontier->length = nFrontierLength;
					
					cout << "nFrontierType = " << nFrontierType << endl;
					cout << "nFrontierCentroidX = " << nFrontierCentroidX << endl;
					cout << "nFrontierCentroidY = " << nFrontierCentroidY << endl;
					cout << "nFrontierLength = " << nFrontierLength << endl;

					g_pFrontierList.Append ((void *) pFrontier);				
				}
			}
		}
	}
}



// ----------------------------------------------------------------------
// TMap::UpdateEvidenceGrid - generates the evidence grid
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::UpdateEvidenceGrid (float fltRobotRadius)
{
	int i, j;
	int nHorizontalMargin, nVerticalMargin;
	
	// Calculates the obstacles margin
	nHorizontalMargin = (int) (fltRobotRadius * (float) gridWidth / (areaBounds[1] - areaBounds[0]) + .5f);
	nHorizontalMargin = 2 * (nHorizontalMargin / 2) + 1;
	
	nVerticalMargin = (int) (fltRobotRadius * (float) gridLength / (areaBounds[3] - areaBounds[2]) + .5f);
	nVerticalMargin = 2 * (nVerticalMargin / 2) + 1;
	
	// Empties the current frontier list
	g_pFrontierList.Empty ();
	
	// Initializes the frontier grid
	for (i = 0; i < gridWidth * gridLength; i++)
		frontierGrid[i].right = frontierGrid[i].left = UNLINKED; 
	
	// Classifies the occupancy map cells
	for (i = 0; i < gridWidth * gridLength; i++)
	{
		if (occupancyGrid[i] >= .75f)
			evidenceGrid[i] = OCCUPIED_CELL;
		else if (occupancyGrid[i] <= .25f)
			evidenceGrid[i] = FREE_CELL;
		else
			evidenceGrid[i] = UNKNOWN_CELL;
	}
	
	// Expands the occupied cells into warning
	for (j = 0; j < gridLength; j++)
	{
		for (i = 0; i < gridWidth; i++)
		{
			if (evidenceGrid[i + j * gridWidth] == OCCUPIED_CELL)
				ExpandCell (i, j, nHorizontalMargin, nVerticalMargin);
		}
	}

	// Finds the unknown frontiers cells
	for (j = 0; j < gridLength; j++)
	{
		for (i = 0; i < gridWidth; i++)
		{
			if (evidenceGrid[i + j * gridWidth] == UNKNOWN_CELL &&
			    HaveFreeAdjacentCell (i, j))
				evidenceGrid[i + j * gridWidth] = UNKNOWN_FRONTIER_CELL;
		}
	}
	GroupFrontierCells (UNKNOWN_FRONTIER_CELL);
	
	// Finds the warning frontiers cells
	for (j = 0; j < gridLength; j++)
	{
		for (i = 0; i < gridWidth; i++)
		{
			if (evidenceGrid[i + j * gridWidth] == WARNING_CELL &&
			    HaveFreeAdjacentCell (i, j))
				evidenceGrid[i + j * gridWidth] = WARNING_FRONTIER_CELL;
		}
	}
	GroupFrontierCells (WARNING_FRONTIER_CELL);
}



// ----------------------------------------------------------------------
// TMap::GetNearestFrontier - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

bool  TMap::GetNearestFrontier (unsigned int nFrontierType, int nMinimalFrontierLength, float *p_fltNearestFrontierX, float *p_fltNearestFrontierY)
{
	TNode *pAux = NULL;
	TFrontier *pFrontier = NULL;
	float fltNearestFrontierDistance = FLT_MAX, fltFrontierDistance;
	float fltFrontierX = .0f, fltFrontierY = .0f;
	
	for (pAux = g_pFrontierList.head; pAux->next != NULL; pAux = pAux->next)
	{
		pFrontier = (TFrontier *) pAux->info;
		
		Grid2Floor (pFrontier->centroidX, pFrontier->centroidY, &fltFrontierX, &fltFrontierY);
				
		if ((pFrontier->type != nFrontierType) || (pFrontier->length < nMinimalFrontierLength))
			continue;
		
		fltFrontierDistance = (fltFrontierX - robotX) * (fltFrontierX - robotX) + (fltFrontierY - robotY) * (fltFrontierY - robotY);
		
		if (fltFrontierDistance < fltNearestFrontierDistance)
		{
			fltNearestFrontierDistance = fltFrontierDistance;
			*p_fltNearestFrontierX = fltFrontierX;
			*p_fltNearestFrontierY = fltFrontierY;
		}
	}
	
	return (fltNearestFrontierDistance != FLT_MAX);
}


// ----------------------------------------------------------------------
//  TMap::ScanLine - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

bool TMap::ScanLine (int nStartX, int nStartY, int nEndX, int nEndY, int *p_nReachedX, int *p_nReachedY, unsigned int nProhibitedCellType)
{
	float fltAngCoef;

	*p_nReachedX = nEndX;
	*p_nReachedY = nEndY;
	
	if  ((nEndX != nStartX) && fabs (fltAngCoef = (float) (nEndY - nStartY) / (float) (nEndX - nStartX)) < 1.0f)
	{
		if (nEndX < nStartX)
		{
			for (*p_nReachedX = nStartX - 1; *p_nReachedX > nEndX; (*p_nReachedX)--)
			{
				*p_nReachedY = (int) (fltAngCoef * (float) (*p_nReachedX - nStartX) + (float) nStartY + .5f);
				if ((evidenceGrid[*p_nReachedX + *p_nReachedY * gridWidth] & nProhibitedCellType) & INDEX_MASK)
					return (true);
			}
		}
		else
		{
			for (*p_nReachedX = nStartX + 1; *p_nReachedX < nEndX; (*p_nReachedX)++)
			{
				*p_nReachedY = (int) (fltAngCoef * (float) (*p_nReachedX - nStartX) + (float) nStartY + .5f);
				if ((evidenceGrid[*p_nReachedX + *p_nReachedY * gridWidth] & nProhibitedCellType) & INDEX_MASK)
					return (true);
			}
		}
		
		*p_nReachedY = (int) (fltAngCoef * (float) (*p_nReachedX - nStartX) + (float) nStartY + .5f);
	}
	else if  (nEndY != nStartY)
	{
		fltAngCoef = (float) (nEndX - nStartX) / (float) (nEndY - nStartY);

		if (nEndY < nStartY)
		{
			for (*p_nReachedY = nStartY - 1; *p_nReachedY > nEndY; (*p_nReachedY)--)
			{
				*p_nReachedX = (int) (fltAngCoef * (float) (*p_nReachedY - nStartY) + (float) nStartX + .5f);
				if ((evidenceGrid[*p_nReachedX + *p_nReachedY * gridWidth] & nProhibitedCellType) & INDEX_MASK)
					return (true);
			}
		}
		else
		{					
			for (*p_nReachedY = nStartY + 1; *p_nReachedY < nEndY; (*p_nReachedY)++)
			{
				*p_nReachedX = (int) (fltAngCoef * (float) (*p_nReachedY - nStartY) + (float) nStartX + .5f);
				if ((evidenceGrid[*p_nReachedX + *p_nReachedY * gridWidth] & nProhibitedCellType) & INDEX_MASK)
					return (true);
			}
		}
		
		*p_nReachedX = (int) (fltAngCoef * (float) (*p_nReachedY - nStartY) + (float) nStartX + .5f);
	}	

	
	return (false);
}



// ----------------------------------------------------------------------
//  TMap::ScanFrontier - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

bool TMap::ScanFrontier (int nFrontierX, int nFrontierY, int nCurrentStartX, int nCurrentStartY, int *p_nNewStartX, int *p_nNewStartY, int nSide)
{
	int k;
	
	// Scan the frontier until reach the frontier edge or 
	// detects an unknown or an ocuppied cell on the line linking 
	// the current frontier cell and the current start cell
	for (;;)
	{			
		// Gets the adjacent cell
		k = (nSide == SCAN_LEFT)  ? 
		     frontierGrid[nFrontierX + nFrontierY * gridWidth].left : 
		     frontierGrid[nFrontierX + nFrontierY * gridWidth].right;
		
		// If there's no adjacent cell, returns the current frontier cell
		if (k == UNLINKED) 
		{
			*p_nNewStartX = nFrontierX;
			*p_nNewStartY = nFrontierY;
			break;
		}
	
		// Goes to the adjacent cell
		nFrontierX += g_p_nXPos[k];
		nFrontierY += g_p_nYPos[k];
		
		cout << "Frontier cell = (" << nFrontierX << ", " << nFrontierY << "), k = " << k << endl;
		
		// ----------------------------------------------------------------------
		// Scans the line linking the current frontier cell 
		// and the current start cell looking for unknown or ocuppied cells
		// ----------------------------------------------------------------------
		if (ScanLine (nFrontierX, nFrontierY, nCurrentStartX, nCurrentStartY, p_nNewStartX, p_nNewStartY, WARNING_CELL | OCCUPIED_CELL))
		{
			nFrontierX -= g_p_nXPos[k];
			nFrontierY -= g_p_nYPos[k];
			
			// ----------------------------------------------------------------------
			// If the current start cell is in the same frontier,
			// returns the current frontier cell
			// ----------------------------------------------------------------------
			if (!ScanLine (nFrontierX, nFrontierY, nCurrentStartX, nCurrentStartY, p_nNewStartX, p_nNewStartY, FREE_CELL) ||
			    !ScanLine (nFrontierX, nFrontierY, nCurrentStartX, nCurrentStartY, p_nNewStartX, p_nNewStartY, WARNING_FRONTIER_CELL))
			{
				*p_nNewStartX = nFrontierX;
				*p_nNewStartY = nFrontierY;
			}
			break;
		}
	}
	
	cout << "New start cell = (" << *p_nNewStartX << ", " << *p_nNewStartY << ")" << endl;

	// Tests if the start cell keeps the same
	return (((*p_nNewStartX) != nCurrentStartX) || ((*p_nNewStartY) != nCurrentStartY));
}



// ----------------------------------------------------------------------
//  TMap::GetPath - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

float TMap::GetPath (float fltStartX, float fltStartY, float fltEndX, float fltEndY, int nNodeIndex)
{
	int nStartX, nStartY, nEndX, nEndY, nReachedX, nReachedY, nNewStartX, nNewStartY, nLeftNodeIndex, nRightNodeIndex;
	float fltNewStartX, fltNewStartY, fltPathLength, fltLeftPathLength, fltRightPathLength;
	
	Floor2Grid (fltStartX, fltStartY, &nStartX, &nStartY);
	Floor2Grid (fltEndX, fltEndY, &nEndX, &nEndY);

	//----------------------------------------------------------------------
	// Tests if the accumulated path depth is smallest than 
	// the maximal path depth
	//----------------------------------------------------------------------
	if (nNodeIndex < MAX_NODES)
	{
		// Create the path
		g_p_fltPathSearchTree[4 * nNodeIndex + 0] = fltStartX;
		g_p_fltPathSearchTree[4 * nNodeIndex + 1] = fltStartY;
		cout << "--------------------------------------------------" << endl;
		cout << "Start cell = (" << nStartX << ", " << nStartY << ")" << endl;
		cout << "End cell = (" << nEndX << ", " << nEndY << ")" << endl;

		//----------------------------------------------------------------------	
		// Scan the line linking the start cell and the end cell 
		// looking for unknown or ocuppied cells
		//----------------------------------------------------------------------	
		if (ScanLine (nStartX, nStartY, nEndX, nEndY, &nReachedX, &nReachedY, WARNING_CELL | OCCUPIED_CELL))
		{
			cout << "Occupied cell = (" << nReachedX << ", " << nReachedY << ")" << endl;
			
			ScanLine (nReachedX, nReachedY, nStartX, nStartY, &nReachedX, &nReachedY, WARNING_FRONTIER_CELL);

			cout << "Reached cell = (" << nReachedX << ", " << nReachedY << ")" << endl;

			// Scan the left adjacent frontier cells
			nLeftNodeIndex = (ScanFrontier (nReachedX, nReachedY, nStartX, nStartY, &nNewStartX, &nNewStartY, SCAN_LEFT)) ?
					2 * nNodeIndex + 1 : MAX_NODES;
			Grid2Floor (nNewStartX, nNewStartY, &fltNewStartX, &fltNewStartY);
			fltLeftPathLength = GetPath (fltNewStartX, fltNewStartY, fltEndX, fltEndY, nLeftNodeIndex) +
					    FloorDistance (fltStartX, fltStartY, fltNewStartX, fltNewStartY);
	
			// Scan the right adjacent frontier cells
			nRightNodeIndex = (ScanFrontier (nReachedX, nReachedY, nStartX, nStartY, &nNewStartX, &nNewStartY, SCAN_RIGHT)) ?
					2 * nNodeIndex + 2 : MAX_NODES;
			Grid2Floor (nNewStartX, nNewStartY, &fltNewStartX, &fltNewStartY);
			fltRightPathLength = GetPath (fltNewStartX, fltNewStartY, fltEndX, fltEndY, nRightNodeIndex) + 
					     FloorDistance (fltStartX, fltStartY, fltNewStartX, fltNewStartY);
								    		
			// Choose the smallest path
			if (fltLeftPathLength < fltRightPathLength)
			{
				cout << "Left small" << endl;
				g_p_fltPathSearchTree[4 * nNodeIndex + 2] = fltPathLength = fltLeftPathLength;
				g_p_fltPathSearchTree[4 * nNodeIndex + 3] = LEFT_NODE;

			}
			else
			{
				cout << "Right small" << endl;
				g_p_fltPathSearchTree[4 * nNodeIndex + 2] = fltPathLength = fltRightPathLength;
				g_p_fltPathSearchTree[4 * nNodeIndex + 3] = RIGHT_NODE;
			}
		}
		else
		{
			//----------------------------------------------------------------------	
			// If the reached cell is the end cell, 
			// return the current path concatenated with the end cell
			//----------------------------------------------------------------------			
			if ((nLeftNodeIndex = 2 * nNodeIndex + 1) < MAX_NODES)
			{
				cout << "End cell = (" << nEndX << ", " << nEndY << ")" << endl;
				g_p_fltPathSearchTree[4 * nNodeIndex + 2] = fltPathLength = FloorDistance (fltStartX, fltStartY, fltEndX, fltEndY);
				g_p_fltPathSearchTree[4 * nNodeIndex + 3] = LEFT_NODE;

				g_p_fltPathSearchTree[4 * nLeftNodeIndex + 0] = fltEndX;
				g_p_fltPathSearchTree[4 * nLeftNodeIndex + 1] = fltEndY;
				g_p_fltPathSearchTree[4 * nLeftNodeIndex + 2] = .0f;
				g_p_fltPathSearchTree[4 * nLeftNodeIndex + 3] = NULL_NODE;
			}
			else
			{
				g_p_fltPathSearchTree[4 * nNodeIndex + 2] = fltPathLength = MAX_PATH_LENGTH;
				g_p_fltPathSearchTree[4 * nNodeIndex + 3] = NULL_NODE;
			}
		}
	}
	else
		fltPathLength = MAX_PATH_LENGTH;
	
	return (fltPathLength);
}
// ----------------------------------------------------------------------
//  TMap::GetShortestPath - 
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TMap::GetShortestPath (float fltGoalX, float fltGoalY)
{
	int nStartX, nStartY, nGoalX, nGoalY, nNodeIndex = 0;
	float fltPathLength, fltStartX, fltStartY;
	
	cout << "Robot position = (" << robotX << ", " << robotY <<")" << endl;
	cout << "Goal position = (" << fltGoalX << ", " << fltGoalY <<")"<< endl;
	
	fltStartX = robotX;
	fltStartY = robotY;
	
	Floor2Grid (fltStartX, fltStartY, &nStartX, &nStartY);
	if ((evidenceGrid[nStartX + nStartY * gridWidth] == WARNING_CELL) || (evidenceGrid[nStartX + nStartY * gridWidth] == OCCUPIED_CELL))
	{
		g_p_fltPathSearchTree[4 * nNodeIndex + 0] = fltStartX;
		g_p_fltPathSearchTree[4 * nNodeIndex + 1] = fltStartY;
		g_p_fltPathSearchTree[4 * nNodeIndex + 2] = .0f;
		g_p_fltPathSearchTree[4 * nNodeIndex + 3] = LEFT_NODE;
		nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3];
		
		Floor2Grid (fltGoalX, fltGoalY, &nGoalX, &nGoalY);
		ScanLine (nStartX, nStartY, nGoalX, nGoalY, &nStartX, &nStartY, WARNING_FRONTIER_CELL);
		Grid2Floor (nStartX, nStartY, &fltStartX, &fltStartY);

	}
	
	fltPathLength = GetPath (fltStartX, fltStartY, fltGoalX, fltGoalY, nNodeIndex);
	
	for (nNodeIndex = 0, cout << "["; nNodeIndex < MAX_NODES && g_p_fltPathSearchTree[4 * nNodeIndex + 3] != NULL_NODE; nNodeIndex = 2 * nNodeIndex + (int) g_p_fltPathSearchTree[4 * nNodeIndex + 3])
		cout << "(" << g_p_fltPathSearchTree[4 * nNodeIndex + 0] << ", " << g_p_fltPathSearchTree[4 * nNodeIndex + 1] <<") ,";
	
	if (nNodeIndex < MAX_NODES)
		cout << "(" << g_p_fltPathSearchTree[4 * nNodeIndex + 0] << ", " << g_p_fltPathSearchTree[4 * nNodeIndex + 1] <<")]" << endl;

	g_bValidPath = true;
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



// ----------------------------------------------------------------------
// TMap::Grid2Floor -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TMap::Grid2Floor (int i, int j, float *p_fltX, float *p_fltY)
{
       *p_fltX = ((float) i + .5f) / (float) gridWidth  * (areaBounds[1] - areaBounds[0]) + areaBounds[0];
       *p_fltY = ((float) j + .5f) / (float) gridLength * (areaBounds[3] - areaBounds[2]) + areaBounds[2];
//       *p_fltX = (float) i / (float) gridWidth  * (areaBounds[1] - areaBounds[0]) + areaBounds[0];
//       *p_fltY = (float) j / (float) gridLength * (areaBounds[3] - areaBounds[2]) + areaBounds[2];
}



// ----------------------------------------------------------------------
// TMap::Floor2Grid -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TMap::Floor2Grid (float fltX, float fltY, int *i, int *j)
{
// 	*i =(int) ((float) gridWidth  * (fltX - areaBounds[0]) / (areaBounds[1] - areaBounds[0]) - .5f);
//	*j =(int) ((float) gridLength * (fltY - areaBounds[2]) / (areaBounds[3] - areaBounds[2]) - .5f);
 	*i =(int) ((float) gridWidth  * (fltX - areaBounds[0]) / (areaBounds[1] - areaBounds[0]));
	*j =(int) ((float) gridLength * (fltY - areaBounds[2]) / (areaBounds[3] - areaBounds[2]));
}



// ----------------------------------------------------------------------
// TMap::FloorDistance -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

float TMap::FloorDistance (int nStartX, int nStartY, int nEndX, int nEndY)
{
	float fltStartX, fltStartY, fltEndX, fltEndY;
	
	Grid2Floor (nStartX, nStartY, &fltStartX, &fltStartY);
	Grid2Floor (nEndX, nEndY, &fltEndX, &fltEndY);
	
	return (sqrt ((fltStartX - fltEndX) * (fltStartX - fltEndX) + (fltStartY - fltEndY) * (fltStartY - fltEndY)));
}

// ----------------------------------------------------------------------
// TMap::FloorDistance -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

float TMap::FloorDistance (float fltStartX, float fltStartY, float fltEndX, float fltEndY)
{
	return (sqrt ((fltStartX - fltEndX) * (fltStartX - fltEndX) + (fltStartY - fltEndY) * (fltStartY - fltEndY)));
}

