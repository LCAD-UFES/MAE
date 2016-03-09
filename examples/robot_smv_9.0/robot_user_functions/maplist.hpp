#ifndef _MAPLIST_H
#define _MAPLIST_H

#include <vector>
using namespace std;

#include "map.hpp"

typedef vector<TMap*> MapVector;

typedef vector<TMap*>::iterator MapIterator;

class TMapList
{
	private: MapVector maps;
	
	public: TMapList(int nLength, int nWidth, int nHeight, float fltBetaMin,
			 float fltBetaMax, int nLayersNumber, float fltNear, float fltFar,
			 float fltTerrainSize);
			 
	public: ~TMapList();
			
	public: void Draw();
	
	public: void ShowImage();
	
	public: void ShowDepthMap();
	
	public: void GetNextLayer();
	
	public: void AddPosition(float fltX, float fltY, float fltZ);
	
	public: bool Load(int nIndex, char *strFileName);
	
	public: bool Save(int nIndex, char *strFileName);
	
	public: bool SaveImage(int nIndex, char *strFileName);
	
	public: bool SavePointCloud(int nIndex, char *strFileName);
	
	public: void Update(int nIndex, float X, float Y, float Z, GLubyte red, GLubyte green,
			    GLubyte blue);

	public: void Erase();
};

#endif
