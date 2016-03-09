#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>
#include <string.h>
#include <stdlib.h>

#include "gtsdbTools.h"

/**
Your detector should be a C++ function with 7 arguments: The full image, its width and height, and the 4 return		*
values (the detected ROIs as vectors of the left column, the right column, the top row, and		*
the bottom row: detectorFunc( fullImage, width, height, &leftCols, &rightCols, &topRows, &bottomRows )			*
*/
void testDetectorFunc(const RgbaValue* fullImg, int width, int height, 
	std::vector<double>* leftCols, std::vector<double>* rightCols, std::vector<double>* topRows, std::vector<double>* bottomRows)
{
	// Change this file name if you want to evaluate your submission text file
	char *submissionFile = const_cast<char *> ("ex.txt");

	static int fileNo = 301;
	static std::vector<double> submissionLeftCols, submissionRightCols,
		submissionTopRows, submissionBottomRows;
	static std::vector<int> submissionFileNos;

	if ( submissionLeftCols.empty() )
	{
		std::ifstream fIn(submissionFile);

		if (! fIn.is_open())
		{
			std::cout << submissionFile << " not found." << std::endl;
			return;
		}

		while (! fIn.eof())
		{
			int inFileNo, leftCol, rightCol, topRow, bottomRow;
			char sLine[2000];
			fIn >> sLine;
			sscanf(sLine, "%d.ppm;%d;%d;%d;%d", &inFileNo, &leftCol, &topRow, &rightCol, &bottomRow);

			submissionFileNos.push_back(inFileNo);
			submissionLeftCols.push_back(leftCol);
			submissionRightCols.push_back(rightCol);
			submissionTopRows.push_back(topRow);
			submissionBottomRows.push_back(bottomRow);
		}
	}

	for (int i = 0; i < submissionFileNos.size(); ++i)
	{
		if ( submissionFileNos[i] == fileNo)
		{
			leftCols->push_back(submissionLeftCols[i]);
			rightCols->push_back(submissionRightCols[i]);
			topRows->push_back(submissionTopRows[i]);
			bottomRows->push_back(submissionBottomRows[i]);
		}
	}

	++fileNo;
}

int main(int argc, char *argv[])
{
	double jaccard = 0.6;

	if (argc == 2)
		jaccard = atof(argv[1]);

	//to use this function you will have to adjust the following parameters
	std::string benchmarkPath = "./";											//path you extracted the dataset to
	Category category = prohibitory;																										//'prohibitory', 'mandatory', 'danger'
	void (*detectorFunc)(const RgbaValue*, int, int, std::vector<double>*, std::vector<double>*, std::vector<double>*, std::vector<double>*) = &testDetectorFunc;	//the traffic sign category your submission is aimed at 'prohibitory', 'mandatory', or 'danger' 
	bool verbose = true;

	TSD_testMyDetector( benchmarkPath, category, detectorFunc, verbose, jaccard );

	return 0;
}
