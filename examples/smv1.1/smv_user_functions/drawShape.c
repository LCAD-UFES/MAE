#include "drawShape.h"


/*
+-----------------------------------------------------------------------
|
/ computeNorm - compute the vector norm
/ Input:	WORLD_POINT vector -  the vector
/ Output: 	float - the vector norm
|
+-----------------------------------------------------------------------
*/

float
computeNorm (WORLD_POINT vector)
{
	return (sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z));
}



/*
+-----------------------------------------------------------------------
|
/ computeDotProduct - compute the scalar product between two vectors
/ Input:	WORLD_POINT vectorA - first vector
/		WORLD_POINT vectorB - second vector
/ Output: 	float - the dot prodcut beetween vectorA and vectorB.
|
+-----------------------------------------------------------------------
*/

float
computeDotProduct (WORLD_POINT vectorA, WORLD_POINT vectorB)
{
	return (vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z);
}



/*
+-----------------------------------------------------------------------
|
/ computeAngleBetweenVectors - compute the angle beetween two vectors
/ Input:	WORLD_POINT vectorA - first vector
/		WORLD_POINT vectorB - second vector
/ Output: 	float - the angle beetween vectorA and vectorB in degree.
|
+-----------------------------------------------------------------------
*/

float
computeAngleBetweenVectors (WORLD_POINT vectorA, WORLD_POINT vectorB)
{
	return (180.0 * acos(computeDotProduct (vectorA, vectorB) / (computeNorm (vectorA) * computeNorm (vectorB))) / pi);
}



/*
+-----------------------------------------------------------------------
|
| ShapeMapInitialize - Iniatilize a map to construct volume shape
| Input:	shape_map - A Map to plot world points.
|		maxSize - maxSize of map vector 
| Output:	void
|
+-----------------------------------------------------------------------
*/

void
ShapeMapInitialize (SHAPE_MAP *shape_map, int maxSize)
{
	int i, j;
	
	shape_map->maxSize		= maxSize;
	shape_map->totalPoints 		= 0;
	shape_map->map_point 		= (MAP_POINT *)malloc(sizeof(MAP_POINT) * maxSize);

	for (i = 0; i < maxSize; i++)
	{
		shape_map->map_point[i].point.x 	= 0.0;
		shape_map->map_point[i].point.y 	= 0.0;
		shape_map->map_point[i].point.z 	= 0.0;
		shape_map->map_point[i].status 		= TURN_OFF;
	}
}



/*
+-----------------------------------------------------------------------
|
| nodeListInitialize - Iniatilize the list that will contain shape nodes
| Input:	NODE_LIST node_list
| Output:	void
|
+-----------------------------------------------------------------------
*/
void
nodeListInitialize (NODE_LIST *node_list)
{
	node_list->maxSize = 0;
	node_list->nodesVector = NULL;
}



/*
+-----------------------------------------------------------------------
|
| elementListInitialize - Iniatilize the list that will contain shape elements
| Input:	ELEMENT_LIST element_list
| Output:	void
|
+-----------------------------------------------------------------------
*/
void
elementListInitialize (ELEMENT_LIST *element_list)
{
	element_list->maxSize = 0;
	element_list->elementsVector = NULL;
}/*
+-----------------------------------------------------------------------
|
| ShapeMapDispose - Dispose memory associated to the shape_map
| Input:	shape_map - A Map to plot world points.
| Output:	void
|
+-----------------------------------------------------------------------
*/
void
ShapeMapDispose (SHAPE_MAP *shape_map)
{
	shape_map->totalPoints = 0;
	if (shape_map->map_point != NULL)
		free(shape_map->map_point);
	shape_map->map_point = NULL;
}

/*
 ----------------------------------------------------------------------
 GetShapeGeometricCenter - Retorna o centro geometrico do TMap
 
 Entradas: map - TMap a ser calculado o centro geometrico
		       center_x - Retorno da coordenada x do centro geometrico
		       center_y - Retorno da coordenada y do centro geometrico
		       center_z - Retorno da coordenada z do centro geometrico

 Saida: WORLD_POINT - geometric center.
 ----------------------------------------------------------------------
*/
WORLD_POINT
GetShapeGeometricCenter(SHAPE_MAP shape_map)
{
	int i;
	WORLD_POINT maxAxisValues, minAxisValues;
	int flagPrimeiroPontoValido;
	WORLD_POINT geometricCenter;

	geometricCenter.x = 0.0;
	geometricCenter.y = 0.0;
	geometricCenter.z = 0.0;
	flagPrimeiroPontoValido = 1;
	
	for (i = 0; i < shape_map.totalPoints; i++)
	{
		if (shape_map.map_point[i].status == TURN_ON)
		{
			if (flagPrimeiroPontoValido)
			{
				flagPrimeiroPontoValido = 0;
				maxAxisValues.x = shape_map.map_point[0].point.x;
				maxAxisValues.y = shape_map.map_point[0].point.y;
				maxAxisValues.z = shape_map.map_point[0].point.z;
				minAxisValues.x = shape_map.map_point[0].point.x;
				minAxisValues.y = shape_map.map_point[0].point.y;
				minAxisValues.z = shape_map.map_point[0].point.z;
			}
			else
			{
				if (maxAxisValues.x < shape_map.map_point[i].point.x)
					maxAxisValues.x = shape_map.map_point[i].point.x;
				if (maxAxisValues.y < shape_map.map_point[i].point.y)
					maxAxisValues.y = shape_map.map_point[i].point.y;
				if (maxAxisValues.z < shape_map.map_point[i].point.z)
					maxAxisValues.z = shape_map.map_point[i].point.z;
				if (minAxisValues.x > shape_map.map_point[i].point.x)
					minAxisValues.x = shape_map.map_point[i].point.x;
				if (minAxisValues.y > shape_map.map_point[i].point.y)
					minAxisValues.y = shape_map.map_point[i].point.y;
				if (minAxisValues.z > shape_map.map_point[i].point.z)
					minAxisValues.z = shape_map.map_point[i].point.z;
			}
		}
	}
	

	geometricCenter.x  = (maxAxisValues.x + minAxisValues.x) / 2.0;
	geometricCenter.y  = (maxAxisValues.y + minAxisValues.y) / 2.0;
	geometricCenter.z  = (maxAxisValues.z + minAxisValues.z) / 2.0;
	
	return geometricCenter;
}

/*
------------------------------------------------------------------------------------------
memorizeWorldPoint - memorize a World Point to plot the Shape to calculate Volume.
Input	- No inputs.
Output	- No outputs.
------------------------------------------------------------------------------------------
*/

void
memorizeWorldPoint (WORLD_POINT world_point, SHAPE_MAP *shape_map)
{
	if (shape_map == NULL)
		ShapeMapInitialize (shape_map, MAX_POINTS);
	
	if ( shape_map->totalPoints >= shape_map->maxSize )
		Erro (	"Points can't be memorize, because doesn't have space to do this.",
			"Please, save the points, remake the application upsizing MAX_SIZE,", 
			"and then, load the points again.");

	shape_map->map_point[shape_map->totalPoints].point.x = world_point.x;
	shape_map->map_point[shape_map->totalPoints].point.y = world_point.y;
	shape_map->map_point[shape_map->totalPoints].point.z = world_point.z;
	shape_map->map_point[shape_map->totalPoints].status = 
		(shape_map->map_point[shape_map->totalPoints].status == TURN_ON) ? TURN_OFF : TURN_ON;
	shape_map->totalPoints++;
}



/*
------------------------------------------------------------------------------------------
generateBoundary - this function will interpolate the points to generate boundary limits.
Input	- FILE outputFile	- file where the points will be generated.
	  int totalPoints	- total points generated needed to interpolate lines or splines
Output	- Nothing
------------------------------------------------------------------------------------------
*/

void
generateBoundary (FILE *outputFile, int totalPoints)
{
	int i, j;
	int pointsToGenerateSpline;
	int startPoint, endPoint;
	int totalSplines;
	int totalLines;
	int lastObject;

	if (totalPoints < MIN_POINTS_TO_GENERATE_PLANE)
	{
		Erro ("Could not generate a plane.","","");
		return;
	}
	else if (totalPoints >= (MIN_LINES_TO_GENERATE_PLANE * (MIN_POINTS_TO_GENERATE_SPLINE - 1)))
	{
		totalSplines = MIN_LINES_TO_GENERATE_PLANE;
//		totalSplines = (totalPoints / MIN_POINTS_TO_GENERATE_SPLINE) + 1;
		pointsToGenerateSpline = (int) (((float)(totalPoints + totalSplines)) / (totalSplines * 1.0) + 0.5); /* Needed because it is necessary repeat points */

		startPoint = 1;
		for (i=1;i <= totalSplines;i++)
		{
			fprintf(outputFile, SPLINE_COMMAND, i);

			endPoint = i * (pointsToGenerateSpline - 1) + 1;
			for (j=startPoint; j <= endPoint || j <= totalPoints; j++)
				fprintf(outputFile, OBJECT, j);
			startPoint = endPoint;
			fprintf(outputFile, LAST_OBJECT, 1);

		}
		
		fprintf(outputFile, LINE_LOOP_COMMAND, totalSplines + 1);
		for (i=1;i < totalSplines;i++)
			fprintf(outputFile, OBJECT, i);
		fprintf(outputFile, LAST_OBJECT, totalSplines);
		
		lastObject = totalSplines++;
	}
	else
	{
		totalLines = totalPoints - 1;
		
		for (i=1;i <= totalLines;i++)
			fprintf(outputFile, LINE_COMMAND,i, i, i+1);
		fprintf(outputFile, LINE_COMMAND, totalPoints, totalPoints, 1);
		totalLines++;
		
		fprintf(outputFile, LINE_LOOP_COMMAND, totalLines + 1);
		for (i=1;i < totalLines;i++)
			fprintf(outputFile, OBJECT, i);
		fprintf(outputFile, LAST_OBJECT, totalLines);
		lastObject = totalLines + 1;
		
	}

	fprintf(outputFile,"Ruled Surface(%d) = {%d};\n", lastObject + 1, lastObject);
	fprintf(outputFile,"Physical Surface(%d) = {%d};", lastObject + 2, lastObject + 1);
}

/*
------------------------------------------------------------------------------------------
generatePoints - generate points to .geo file needed by gmsh to generate mesh file
Input	- SHAPE_MAP shape_map	- Map used by shape.
	  FILE outputFile	- file where the points will be generated.
	  int breakLineFactor	- Factor that will be divide a segment line to calculate clscale
Output	- int - points generated.
------------------------------------------------------------------------------------------
*/

int
generatePoints (SHAPE_MAP *shape_map, FILE *outputFile, int breakLineFactor)
{
	int i, indexPoint = 0;
	float  point_clscale;
	int nextIndex;
	char strAux[20];
	
	for (i=0;i < shape_map->totalPoints;i++)
	{
		if (shape_map->map_point[i].status == TURN_ON)
		{
			indexPoint++;
			nextIndex = (indexPoint == shape_map->totalPoints) ? 0 : indexPoint;
			point_clscale = calculate_euclidean_distance (shape_map->map_point[i].point, shape_map->map_point[nextIndex].point) / breakLineFactor;
			fprintf(outputFile,"Point(%d) = {%.5f,\t%.5f,\t%.5f,\t%.5f};\n", indexPoint, shape_map->map_point[i].point.x,
				shape_map->map_point[i].point.y, shape_map->map_point[i].point.z, point_clscale);
		}
	}
	return indexPoint;
}

/*
------------------------------------------------------------------------------------------
generateGeoFile - generate .geo file needed by gmsh to generate mesh file
Input	- SHAPE_MAP *shape_map 	- Map used by shape.
	  int breakLineFactor	- Factor that will be divide a segment line to calculate clscale
Output	- No outputs.
------------------------------------------------------------------------------------------
*/

void
generateGeoFile (SHAPE_MAP *shape_map, int breakLineFactor)
{
	int i,j;
	FILE *outputFile;
	int indexSpline = 0, indexLineLoop = 0, indexSurface = 0;
	int pointsToGenerateSpline = 0;
	int totalPoints;
	int firstPointSpline;
	int startPoint, endPoint;
	
	
	if ((outputFile = fopen (GMSH_GEO_FILENAME, "w")) == NULL)
	{
		Erro ("Could not open file stockPileData.geo","","");
	}

	/* printing points... */
	totalPoints = generatePoints (shape_map, outputFile, breakLineFactor);
	generateBoundary (outputFile, totalPoints);
	fclose (outputFile);
}



/*
------------------------------------------------------------------------------------------
drawShapeBoudary - plot the shape boundary
Input	- SHAPE_MAP shape_map 	- the shape map points
Output	- No outputs.
------------------------------------------------------------------------------------------
*/
void
drawShapeBoundary (SHAPE_MAP shape_map)
{
	int i;
	
	glBegin(GL_LINE_LOOP);
		for (i=0;i < shape_map.totalPoints; i++)
		{	
			glVertex3f(shape_map.map_point[i].point.x, shape_map.map_point[i].point.y, -shape_map.map_point[i].point.z);  
		}
	glEnd();
	glBegin(GL_LINE_LOOP);
		for (i=0;i < shape_map.totalPoints; i++)
		{	
			glVertex3f(shape_map.map_point[i].point.x, shape_map.map_point[i].point.y, -shape_map.map_point[i].point.z - stackWidth);  
		}
	glEnd();
}



/*
------------------------------------------------------------------------------------------
drawShapeNodes - plot the shape nodes.
Input	- SHAPE_MAP shape_map 	- the shape map points
Output	- No outputs.
------------------------------------------------------------------------------------------
*/
void
drawShapeNodes (NODE_LIST node_list)
{
	int i;
	glBegin(GL_POINTS);
		for (i=0;i < node_list.maxSize; i++)
		{	
			glVertex3f(node_list.nodesVector[i].x, node_list.nodesVector[i].y, -node_list.nodesVector[i].z);  
		}
	glEnd();
}



void
drawShapeWindoText (char *string) 
{  
  	GLint shapeHeight;

	glPushMatrix();
        // Posição no universo onde o texto será colocado          
	shapeHeight = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
        glRasterPos2f(-shapeHeight, shapeHeight - (shapeHeight * 0.08)); 
	
        // Exibe caracter a caracter
        while(*string)
             glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10,*string++); 
	glPopMatrix();
}

/*
------------------------------------------------------------------------------------------
drawShapeElements - plot the shape elements
Input	- TRIANGLE *elementsVector - the shape map points
Output	- No outputs.
------------------------------------------------------------------------------------------
*/
void
drawShapeElements (ELEMENT_LIST element_list, NODE_LIST node_list)
{
	int i;
	
	for (i=0;i < element_list.maxSize; i++)
	{	
		glBegin(GL_LINE_LOOP);
			
			glColor3f(1.0, 1.0, 1.0);
			
			glVertex3f(node_list.nodesVector[element_list.elementsVector[i].indexA].x, 
				   node_list.nodesVector[element_list.elementsVector[i].indexA].y, 
				   -node_list.nodesVector[element_list.elementsVector[i].indexA].z);
				   
			glVertex3f(node_list.nodesVector[element_list.elementsVector[i].indexB].x, 
				   node_list.nodesVector[element_list.elementsVector[i].indexB].y, 
				   -node_list.nodesVector[element_list.elementsVector[i].indexB].z);
				   
			glVertex3f(node_list.nodesVector[element_list.elementsVector[i].indexC].x, 
				   node_list.nodesVector[element_list.elementsVector[i].indexC].y, 
				   -node_list.nodesVector[element_list.elementsVector[i].indexC].z);
		glEnd();
	}
}



/*
------------------------------------------------------------------------------------------
drawShape - plot the Shape in the Shape Map Window.
Input	- SHAPE_MAP shape_map 	- the shape map points
 	  int volumeWinID	- the Shape Map Window ID.
Output	- No outputs.
------------------------------------------------------------------------------------------
*/
void
drawShape (SHAPE_MAP shape_map, int volumeWinID)
{
	int i;
	char message[255];
	WORLD_POINT observerPoint;

	glutSetWindow(volumeWinID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glColor3f(1.0, 0.0, 0.0);
/* 	
	observerPoint = GetShapeGeometricCenter(shape_map);
	
	gluLookAt (observerPoint.x, observerPoint.y, observerPoint.z, 
		   0.0, 0.0, 0.0,
		   0.0, 1.0, 0.0);

 */   	if (node_list.nodesVector == NULL)
	{
		sprintf (message, "Points Ploted = %d\n", shape_map.totalPoints);
		drawShapeBoundary (shape_map);
		drawShapeWindoText(message);
	}
	else
	{
		sprintf (message, "Nodes = %d, Elements = %d, Break Line Factor = %d\nArea = %10.5f Volume = %10.5f", 
				  node_list.maxSize, element_list.maxSize, breakLineFactor, shapeArea, stockPileVolume);
		drawShapeWindoText(message);
		drawShapeNodes (node_list);
		drawShapeElements (element_list, node_list);
	}
	
	glutSwapBuffers();
	glFlush();
}



/*
------------------------------------------------------------------------------------------
computeTriangleArea - compute the triangle area formed by three points
Input	- WORLD_POINT A, B, C - Three world points.
Output	- the area of the triangle formed by the three points.
------------------------------------------------------------------------------------------
*/
float
computeTriangleArea (WORLD_POINT A, WORLD_POINT B, WORLD_POINT C)
{
	WORLD_POINT vectorAB;
	WORLD_POINT vectorAC;
	WORLD_POINT vectorS;
	
	vectorAB.x = B.x - A.x;
	vectorAB.y = B.y - A.y;
	vectorAB.z = B.z - A.z;
	
	vectorAC.x = C.x - A.x;
	vectorAC.y = C.y - A.y;
	vectorAC.z = C.z - A.z;
/*	
	vectorS  = (i,j,k) = 1.i + 1.j + 1.k
	vectorAB = (a,b,c) = a.i + b.j + c.k
	vectorAC = (d,e,f) = d.i + e.j + f.k
	O produto vetorial u x v será o vetor
	w = (x,y,z) = x.i + y.j + z.k , onde x, y e z são dados pelas relações acima, ou seja:
	x = b.f - c.e
	y = c.d - a.f
	z = a.e - b.d
*/
	vectorS.x = vectorAB.y * vectorAC.z - vectorAB.z * vectorAC.y;
	vectorS.y = vectorAB.z * vectorAC.x - vectorAB.x * vectorAC.z;
	vectorS.z = vectorAB.x * vectorAC.y - vectorAB.y * vectorAC.x;
	
//	return (sqrt((vectorS.x * vectorS.x) + (vectorS.y * vectorS.y) + (vectorS.z * vectorS.z)) * 0.5);
	return (computeNorm (vectorS) * 0.5);
}



/*
+-----------------------------------------------------------------------------------
|
| initializeNodesVector - Iniatilize the vector that will contain all nodes
| Input:	shape_map - Shape Map choosen by the user.
/		nodesVector - the Nodes Vector. A vector that will contain all nodes.
| Output:	void
|
+-----------------------------------------------------------------------------------
*/
void
initializeNodesVector (SHAPE_MAP shape_map, WORLD_POINT *nodesVector)
{
	int i;
	
	for (i = 0; i < shape_map.totalPoints; i++)
	{
		nodesVector[i].x = shape_map.map_point[i].point.x;
		nodesVector[i].y = shape_map.map_point[i].point.y;
		nodesVector[i].z = shape_map.map_point[i].point.z;
	}
}



/*
------------------------------------------------------------------------------------------
readMSHFormatFile - read the file generated by a software to calculate mesh points with
	the same format generated by gmsh.
Input	- Nothing.
Output	- Nothing.
------------------------------------------------------------------------------------------
*/
void
readMSHFormatFile (SHAPE_MAP shape_map)
{
	FILE *meshFile;
	char errorMessage[255];
	char token[255];
	int node_number;
	float x_coord, y_coord, z_coord;
	int number_of_elements;
	int elm_number, elm_type, reg_phys, reg_elem, number_of_nodes, firstIndex, secondIndex, thirdIndex;
	WORLD_POINT *nodesVector;
	TRIANGLE *elementsVector;
	
	
	if ((meshFile = fopen (GMSH_MSH_FILENAME, "r")) == NULL)
	{
		sprintf(errorMessage,"Could not open file %s", GMSH_MSH_FILENAME);
		Erro (errorMessage,"","");
		return;
	}

	fscanf(meshFile, "%s", token);
	fscanf(meshFile, "%d", &number_of_nodes);

	if (node_list.nodesVector != NULL)
		free(node_list.nodesVector);
		
	
	node_list.maxSize = number_of_nodes;
	node_list.nodesVector = (WORLD_POINT *)malloc(sizeof(WORLD_POINT) * number_of_nodes);

	initializeNodesVector (shape_map, node_list.nodesVector);
	
	for(fscanf(meshFile, "%d ", &node_number); node_number < number_of_nodes;fscanf(meshFile, "%d ", &node_number))
	{
		fscanf(meshFile, "%f %f %f\n", &x_coord, &y_coord, &z_coord);
		node_list.nodesVector[node_number - 1].x = x_coord;
		node_list.nodesVector[node_number - 1].y = y_coord;
		node_list.nodesVector[node_number - 1].z = z_coord;
	}

	/* Read the last node_number*/
	fscanf(meshFile, "%f %f %f\n", &x_coord, &y_coord, &z_coord);
	node_list.nodesVector[node_number - 1].x = x_coord;
	node_list.nodesVector[node_number - 1].y = y_coord;
	node_list.nodesVector[node_number - 1].z = z_coord;

	fscanf(meshFile, "%s", token);
	fscanf(meshFile, "%s", token);
	fscanf(meshFile, "%d", &number_of_elements);
	
	if (element_list.elementsVector != NULL)
		free(element_list.elementsVector);
	
	element_list.maxSize = number_of_elements;
	element_list.elementsVector = (TRIANGLE *)malloc(sizeof(TRIANGLE) * number_of_elements);

	for(fscanf(meshFile, "%d ", &elm_number); elm_number < number_of_elements;fscanf(meshFile, "%d ", &elm_number))
	{
		fscanf(meshFile, "%d %d %d %d %d %d %d\n", &elm_type, &reg_phys, &reg_elem, &number_of_nodes, &firstIndex, &secondIndex, &thirdIndex);
		element_list.elementsVector[elm_number - 1].indexA = firstIndex - 1;
		element_list.elementsVector[elm_number - 1].indexB = secondIndex - 1;
		element_list.elementsVector[elm_number - 1].indexC = thirdIndex - 1;
	}
	fscanf(meshFile, "%d %d %d %d %d %d %d\n", &elm_type, &reg_phys, &reg_elem, &number_of_nodes, &firstIndex, &secondIndex, &thirdIndex);
	element_list.elementsVector[elm_number - 1].indexA = firstIndex - 1;
	element_list.elementsVector[elm_number - 1].indexB = secondIndex - 1;
	element_list.elementsVector[elm_number - 1].indexC = thirdIndex - 1;
	fclose(meshFile);
}



/*
------------------------------------------------------------------------------------------
computeArea - calculate the area of shape that has been generated by gmsh.
Input	- Nothing.
Output	- float - the shape area.
------------------------------------------------------------------------------------------
*/
float
computeArea ()
{
	int i;
	float area;
	
	area = 0.0;
	for (i = 0;i < element_list.maxSize;i++)
		area += computeTriangleArea (node_list.nodesVector[element_list.elementsVector[i].indexA],
					     node_list.nodesVector[element_list.elementsVector[i].indexB],
					     node_list.nodesVector[element_list.elementsVector[i].indexC]);
	processStatus = AREA_CALCULATED;
	return area;
}



/*
------------------------------------------------------------------------------------------
calculateVolume - calculate the volume of the shape that has been generated by gmsh.
Input	- Nothing.
Output	- float - the shape volume.
------------------------------------------------------------------------------------------
*/
float
calculateVolume (float area, float width)
{
	processStatus = VOLUME_CALCULATED;
	return area * width;
}
