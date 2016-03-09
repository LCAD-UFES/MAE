#include "terrain.hpp"



// ----------------------------------------------------------------------
// LoadTexture -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

int LoadTexture (char *strFileName)
{
	FILE *texFile = NULL;
	int red, green, blue;
	GLuint texName;
	GLubyte *texImage = NULL;
	int nWidth, nHeight;
	int nMaxRange;
	char strHeader[256];
	char cGarbage = 0;
	
	if ((texFile = fopen (strFileName, "r")) == NULL)
	{
		Error ("Unable to open file '", strFileName, "' (Terrain::LoadTexture).");
		return (-1);
	}
	
	// Ignores the image format	
	fscanf (texFile, "%s", strHeader);	

	// Discards any comments in the begin of the image file
	while (cGarbage != '\n')
		cGarbage = fgetc (texFile);
	
	if ((cGarbage = fgetc (texFile)) == '#')
	{
		while (cGarbage != '\n')
			cGarbage = fgetc (texFile);
	}
	else
		fseek (texFile, -1, SEEK_SET);
	
	// Ignores the image dimentions
	fscanf (texFile, "%d %d\n", &(nWidth), &(nHeight));

	// Ignores the pixels value range
	fscanf (texFile, "%d\n", &(nMaxRange));
	
	if ((texImage = new GLubyte[4 * nWidth * nHeight]) == NULL)
	{
		Error ("Cannot allocate more memory (LoadTexture).", "", "");
		return (-1);
	}
	
	for (int j = 0; j < nHeight; j++)
	{
		for (int i = 0; i < nWidth; i++)
		{
			fscanf (texFile, "%d %d %d", &red, &green, &blue);
			texImage[4 * (i + j * nWidth) + 0] = (GLubyte) red;
			texImage[4 * (i + j * nWidth) + 1] = (GLubyte) green;
			texImage[4 * (i + j * nWidth) + 2] = (GLubyte) blue;
			texImage[4 * (i + j * nWidth) + 3] = 255;
		}
	}
	
	fclose (texFile);
	
	glGenTextures (1, &texName);
	glBindTexture (GL_TEXTURE_2D, texName);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0,GL_RGBA, GL_UNSIGNED_BYTE, texImage);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	delete texImage;
	
	return (texName);
}



// ----------------------------------------------------------------------
// TTerrain::TTerrain -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TTerrain::TTerrain (float fltTerrainSize)
{
	// Sets the attributes
	size = fltTerrainSize / 2.0f;
	
	texFloor = LoadTexture (TERRAIN_TEXTURE_FILE_NAME);
	texWall = LoadTexture (WALL_TEXTURE_FILE_NAME);

	return;
}



// ----------------------------------------------------------------------
// TTerrain::~TTerrain -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TTerrain::~TTerrain (void)
{
	return;
}



// ----------------------------------------------------------------------
// TTerrain::DrawWall - draws a wall in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TTerrain::DrawWall (float fltWidth, float fltLength, float fltHeight)
{
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture (GL_TEXTURE_2D, texWall);

	glBegin(GL_QUADS);
		glTexCoord2f ( .0f,  .0f); 			glVertex3f (.0f, .0f, .0f);
		glTexCoord2f ( .0f, 1.0f); 			glVertex3f (.0f, .0f, fltHeight);
		glTexCoord2f (fltWidth/fltHeight/4.0f, 1.0f); 	glVertex3f (fltWidth, .0f, fltHeight);
		glTexCoord2f (fltWidth/fltHeight/4.0f,  .0f); 	glVertex3f (fltWidth, .0f, .0f);
	glEnd();
	
	glBegin(GL_QUADS);
		glTexCoord2f ( .0f,  .0f); 			glVertex3f (.0f, fltLength, .0f);
		glTexCoord2f ( .0f, 1.0f); 			glVertex3f (.0f, fltLength, fltHeight);
		glTexCoord2f (fltWidth/fltHeight/4.0f, 1.0f); 	glVertex3f (fltWidth, fltLength, fltHeight);
		glTexCoord2f (fltWidth/fltHeight/4.0f,  .0f); 	glVertex3f (fltWidth, fltLength, .0f);

	glEnd();
	
	glBegin(GL_QUADS);		
		glTexCoord2f ( .0f,  .0f); 			glVertex3f (.0f, .0f, .0f);
		glTexCoord2f ( .0f, 1.0f); 			glVertex3f (.0f, .0f, fltHeight);
		glTexCoord2f (fltLength/fltHeight/4.0f, 1.0f); 	glVertex3f (.0f, fltLength, fltHeight);
		glTexCoord2f (fltLength/fltHeight/4.0f,  .0f); 	glVertex3f (.0f, fltLength, .0f);
	glEnd();
	
	glBegin(GL_QUADS);		
		glTexCoord2f ( .0f,  .0f); 			glVertex3f (fltWidth, .0f, .0f);
		glTexCoord2f ( .0f, 1.0f); 			glVertex3f (fltWidth, .0f, fltHeight);
		glTexCoord2f (fltLength/fltHeight/4.0f, 1.0f); 	glVertex3f (fltWidth, fltLength, fltHeight);
		glTexCoord2f (fltLength/fltHeight/4.0f,  .0f); 	glVertex3f (fltWidth, fltLength, .0f);
	glEnd();
	glDisable (GL_TEXTURE_2D);
	
	glColor3f (.5f, .5f, .5f);
	glBegin(GL_POLYGON);
		glVertex3f (.0f, .0f, fltHeight);
		glVertex3f (.0f,  fltLength, fltHeight);
		glVertex3f (fltWidth, fltLength, fltHeight);
		glVertex3f (fltWidth, .0f, fltHeight);
	glEnd();
	
	return;
}



// ----------------------------------------------------------------------
// TTerrain::Draw - draws the terrain in an OpenGL window
//
// Inputs: none
//
// Outputs: none
// ----------------------------------------------------------------------

void TTerrain::Draw (void)
{
	glPushMatrix ();
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture (GL_TEXTURE_2D, texFloor);

	glBegin(GL_QUADS);
		glTexCoord2f ( .0f,  .0f); glVertex3f (-size, -size, .0f);
		glTexCoord2f ( .0f, 2.0f); glVertex3f (-size,  size, .0f);
		glTexCoord2f (2.0f, 2.0f); glVertex3f ( size,  size, .0f);
		glTexCoord2f (2.0f,  .0f); glVertex3f ( size, -size, .0f);
	glEnd();
	
	glDisable (GL_TEXTURE_2D);
	
	glTranslatef (-size, -size, .0f);
	DrawWall (10.0f, 2.0f * size, 50.0f);
	
	glTranslatef (2.0f * size - 10.0f, .0f, .0f);
	DrawWall (10.0f, 2.0f * size, 50.0f);
	
	glRotatef (90.0f, .0f, .0f, 1.0f);
	DrawWall (10.0f, 2.0f * size - 10.0f, 50.0f);
	
	glTranslatef (2.0f * size - 10.0f, .0f, .0f);
	DrawWall (10.0f, 2.0f * size - 10.0f, 50.0f);
	
	glTranslatef (-size / 2.0f, size / 2.0f - 10.0f, .0f);
	DrawWall (10.0f, size, 50.0f);
	
	glTranslatef (-size, .0f, .0f);
	DrawWall (10.0f, size, 50.0f);
	
	glRotatef (-90.0f, .0f, .0f, 1.0f);
	DrawWall (10.0f, size / 3.0f, 50.0f);
	
	glTranslatef (.0f, 2.0f * size / 3.0f + 10.0f, .0f);
	DrawWall (10.0f, size / 3.0f, 50.0f);
	
	glTranslatef (-size - 10.0f, -2.0f * size / 3.0f - 10.0f, .0f);
	DrawWall (10.0f, size / 3.0f, 50.0f);
	
	glTranslatef (.0f, 2.0f * size / 3.0f + 10.0f, .0f);
	DrawWall (10.0f, size / 3.0f, 50.0f);
	
	glPopMatrix();
	
	return;
}
