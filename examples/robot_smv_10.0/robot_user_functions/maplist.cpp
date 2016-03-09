#include "maplist.hpp"

TMapList::TMapList(int nLength, int nWidth, int nHeight, float fltBetaMin,
		   float fltBetaMax, int nLayersNumber, float fltNear, float fltFar,
		   float fltTerrainSize)
{
	for (int i = 0; i < nLength; i++)
	{
		TMap *map = new TMap(nWidth, nHeight, fltBetaMin, fltBetaMax, nLayersNumber,
				     fltNear, fltFar, fltTerrainSize);
		maps.push_back(map);
	}
}

TMapList::~TMapList()
{
	while(maps.size() > 0)
	{
		TMap *map = maps.back();
		maps.pop_back();
		delete map;
	}
}

void
TMapList::Draw()
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->Draw();
	}
}

void
TMapList::ShowImage()
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->ShowImage();
	}
}

void
TMapList::ShowDepthMap()
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->ShowDepthMap();
	}
}

void
TMapList::GetNextLayer()
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->GetNextLayer();
	}
}

void
TMapList::AddPosition(float fltX, float fltY, float fltZ)
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->SetPosition (map->position.x + fltX,
				  map->position.y + fltY,
				  map->position.z + fltZ);
	}
}

bool
TMapList::Load(int nIndex, char *strFileName)
{
	return maps[nIndex]->Load(strFileName);
}

bool
TMapList::Save(int nIndex, char *strFileName)
{
	return maps[nIndex]->Save(strFileName);
}

bool
TMapList::SaveImage(int nIndex, char *strFileName)
{
	return maps[nIndex]->SaveImage(strFileName);
}

bool
TMapList::SavePointCloud(int nIndex, char *strFileName)
{
	return maps[nIndex]->SavePointCloud(strFileName);
}

void
TMapList::Update(int nIndex, float X, float Y, float Z, GLubyte red, GLubyte green,
		 GLubyte blue)
{
	maps[nIndex]->Update(X, Y, Z, red, green, blue);
}

void
TMapList::Erase()
{
	for (MapIterator i = maps.begin(), n = maps.end(); i != n; i++)
	{
		TMap *map = (*i);
		map->Erase();
	}
}
