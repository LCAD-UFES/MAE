#include "gtsdbTools.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <set>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <string.h>

#define sprintf_s snprintf

bool readPPM( const char* fileName, RgbaValue** ppImg, int& sx, int& sy, bool switchRB, std::vector<std::string>* pComments )
{
	FILE * fp = fopen(fileName, "rb");
	if( !fp )
		return false;

	char format[16];
	fscanf(fp, "%s\n", format);

	// Kommentar-Zeilen berlesen
	char tmpCharBuf[256];

	fpos_t position;
	fgetpos (fp, &position);

	while ( true )
	{
		(void)fgets( tmpCharBuf, 255, fp );

		if ( 10 == tmpCharBuf[0] ) continue; // LF

		const int cnt = strncmp( tmpCharBuf, "#", 1 );
		if (0 != cnt)
		{
			// Keine Kommentarzeile, Zeiger wieder zur�cksetzen
			fsetpos(fp, &position);
			break;
		}
		else
		{
			// Kommentarzeile
			if (pComments) // ggf. behalten
			{
				if ( strlen(tmpCharBuf) > 2 )
				{
					std::string comment( tmpCharBuf + 2, strlen(tmpCharBuf)-3 );
					pComments->push_back( comment );
				}
			}
			fgetpos (fp, &position);
		}
	}

	int maxVal;

	const int nParamRead1 = fscanf( fp, "%d %d\n", &sx, &sy );
	const int nParamRead2 = fscanf( fp, "%d\n", &maxVal );

	if ( (nParamRead1 != 2) || (nParamRead2 != 1) )
	{
		fclose(fp);
		return false;
	}

	unsigned char * pTmpBuffer = (unsigned char*)malloc(sx*sy*3*sizeof(unsigned char));

	delete *ppImg;
	*ppImg = new RgbaValue[sx*sy];

	fseek (fp, -(sx*sy*3*sizeof(unsigned char)), SEEK_END);

	const int readcount = fread(pTmpBuffer, sx*sy*3*sizeof(unsigned char), 1, fp);
	if (1 !=  readcount)
	{
		free( pTmpBuffer );
		fclose(fp);
		return false;
	}

	const int szImg = sx * sy;
	for( int i= 0; i < szImg; ++i )
	{
		(*ppImg)[i].m_r = switchRB ? pTmpBuffer[3*i+2] : pTmpBuffer[3*i];
		(*ppImg)[i].m_g = pTmpBuffer[3*i+1];
		(*ppImg)[i].m_b = switchRB ? pTmpBuffer[3*i] : pTmpBuffer[3*i+2];
		(*ppImg)[i].m_a = 255;
	}

	free( pTmpBuffer );
	fclose(fp);

	return true;
}

double getJaccardCoefficient( int leftCol, int topRow, int rightCol, int bottomRow, int gtLeftCol, int gtTopRow, int gtRightCol, int gtBottomRow )
{
	double jaccCoeff = 0.;

	if ( !(	leftCol	> gtRightCol ||
			rightCol < gtLeftCol ||
			topRow > gtBottomRow ||
			bottomRow < gtTopRow	) 
		)
	{
		int interLeftCol = std::max<int>( leftCol, gtLeftCol );
		int interTopRow = std::max<int>( topRow, gtTopRow );
		int interRightCol = std::min<int>( rightCol, gtRightCol );
		int	interBottomRow = std::min<int>( bottomRow, gtBottomRow );

		const double areaIntersection = ( abs( interRightCol-interLeftCol )+1 ) * ( abs( interBottomRow-interTopRow ) +1);
		const double lhRoiSize = ( abs( rightCol-leftCol )+1 ) * ( abs( bottomRow-topRow ) +1);
		const double rhRoiSize = ( abs( gtRightCol-gtLeftCol )+1 ) * ( abs( gtBottomRow-gtTopRow ) +1);

		jaccCoeff = areaIntersection / ( lhRoiSize + rhRoiSize - areaIntersection );
	}
	return jaccCoeff;
};

std::vector<GTData> TSD_readGTData( const char* aGTFile )
{
	int pC[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 15, 16};
	int mC[] = {33, 34, 35, 36, 37, 38, 39, 40};
	int dC[] = {11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
	std::set<int> prohibitoryClassIds( pC, pC+12 );
	std::set<int> mandatoryClassIds( mC, mC+8 );
	std::set<int> dangerClassIds( dC, dC+15 );

	std::ifstream fID;
	fID.open( aGTFile );

	std::vector<GTData> rGTDataVec;
	if( fID.is_open() )
	{
		char line[100];
		while( !fID.eof() )
		{
			GTData data;
			
			fID.getline( line, 100 );
			sscanf( line, "%05d.ppm;%d;%d;%d;%d;%d", &(data.fileNo), &(data.leftCol), &(data.topRow), &(data.rightCol), &(data.bottomRow), &(data.classID) );

			if ( prohibitoryClassIds.find( data.classID ) != prohibitoryClassIds.end() )
				data.category = prohibitory;
			else if ( mandatoryClassIds.find( data.classID ) != mandatoryClassIds.end() )
				data.category = mandatory;
			else if ( dangerClassIds.find( data.classID ) != dangerClassIds.end() )
				data.category = danger;
			else
				data.category = other;

			rGTDataVec.push_back( data );
		}
	}
	else
		std::cout << "cannot open file";

	return rGTDataVec;
}



void TSD_testMyDetector( const std::string & benchmarkPath, Category category, 
	void (*detectorFunc)(const RgbaValue*, int width, int height, std::vector<double>*, std::vector<double>*, std::vector<double>*, std::vector<double>*), bool verbose, double jaccard )
{
	std::string file = "gt.txt";
	std::vector<GTData> gtReadData = TSD_readGTData( (benchmarkPath + file).c_str() );
	std::vector<GTData> gtData;

	gtData.reserve(gtReadData.size());
	for (int i = 0; i < gtReadData.size(); i++)
		if (gtReadData[i].category == category)
			gtData.push_back(gtReadData[i]);

	for (int i = 0; i < gtData.size(); i++)
		gtData[i].found = 0;

	int TP = 0;
	int FP = 0;
	int FN = 0;
	RgbaValue* pFullImage = 0;

	for ( int imgNum=301; imgNum <= 599; ++imgNum )
	{
		char currFileName [1000];
		sprintf_s( currFileName, 1000, "%s/%05d.ppm", benchmarkPath.c_str(), imgNum );

		int imgSx;
		int imgSy;
		readPPM( currFileName, &pFullImage, imgSx, imgSy );

		std::vector<double> leftCols;
		std::vector<double> rightCols;
		std::vector<double> topRows;
		std::vector<double> bottomRows;
		(*detectorFunc)( pFullImage, imgSx, imgSy, &leftCols, &rightCols, &topRows, &bottomRows );
		
		std::vector<double> gtLeftCols;
		std::vector<double> gtRightCols;
		std::vector<double> gtTopRows;
		std::vector<double> gtBottomRows;

		for (int i = 0; i < gtData.size(); i++)
		{
			if (gtData[i].fileNo == imgNum)
			{
				gtLeftCols.push_back (gtData[i].leftCol);
				gtRightCols.push_back (gtData[i].rightCol);
				gtTopRows.push_back (gtData[i].topRow);
				gtBottomRows.push_back (gtData[i].bottomRow);
			}
		}

		bool have_hit_or_miss = false;
		bool already_shown = false;
		bool have_hit = false;

		std::vector<bool> gtSignHit(gtLeftCols.size(), false);

		for(int roiIdx = 0; roiIdx < leftCols.size(); roiIdx++)
		{
			double maxJaccCoeff = jaccard;
			int maxGtRoiIdx = -1;
			for( int gtRoiIdx=0; gtRoiIdx<gtLeftCols.size(); ++ gtRoiIdx )
			{
				double jaccCoeff = getJaccardCoefficient( leftCols[roiIdx], topRows[roiIdx], rightCols[roiIdx], bottomRows[roiIdx], 
					gtLeftCols[gtRoiIdx], gtTopRows[gtRoiIdx], gtRightCols[gtRoiIdx], gtBottomRows[gtRoiIdx] );
				if( jaccCoeff > maxJaccCoeff )
				{
					maxJaccCoeff = jaccCoeff;
					maxGtRoiIdx = gtRoiIdx;
				} 
			}
			if( maxGtRoiIdx == -1 )
			{
				have_hit_or_miss = true;

				FP = FP + 1;

				//td::cout << "00" << imgNum << ".ppm;MISS;" << leftCols[roiIdx] << ";" << topRows[roiIdx]  << ";" << rightCols[roiIdx] << ";" << bottomRows[roiIdx] << std::endl;
			}
			else
			{
				have_hit_or_miss = true;
				have_hit = true;

				gtSignHit[maxGtRoiIdx] = true;

				//std::cout << "00" << imgNum << ".ppm;HIT;" << leftCols[roiIdx] << ";" << topRows[roiIdx] << ";" << rightCols[roiIdx] << ";" << bottomRows[roiIdx] << std::endl;
			}
		}

		for(int i = 0; i < gtSignHit.size(); i++)
		{
			if (gtSignHit[i] == false)
			{
				double lx = gtLeftCols[i];
				double rx = gtRightCols[i];
				double tr = gtTopRows[i];
				double br = gtBottomRows[i];

				std::cout << "00" << imgNum << ".ppm;" << lx << ";" << tr << ";" << rx << ";" << br << std::endl;
			}
		}

		const int sumHits = std::accumulate(gtSignHit.begin(), gtSignHit.end(), 0);
		TP = TP + sumHits;
		FN = FN + gtSignHit.size() - sumHits;
	}

	fprintf(stderr, "true positive = %d false positives = %d false negatives = %d\n", TP, FP, FN);
	fprintf(stderr, "Precision: %lf Recall: %lf\n", (double)TP / (double)(TP + FP), (double)TP / (double)(TP + FN));
}
