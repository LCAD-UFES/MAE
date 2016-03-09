#ifndef GTSDB_TOOLS_H_INCLUDED
#define GTSDB_TOOLS_H_INCLUDED

#include <string>
#include <vector>

// Traffic sign categories relevant for the upcoming competition phase
enum Category
{
	prohibitory,	// prohibitory signs, circular with a red border
	mandatory,		// mandatory signs, circular on blue ground
	danger,			// danger signs, triangular with a red border
	other,			// all signs not belonging to one of the above categories
};

// Ground truth data for one traffic sign 
struct GTData
{
	int			fileNo;			// number of the image file the traffic sign can be found in
	int			leftCol;		// left column of the region of interest (ROI) of the traffic sign
	int			topRow;			// upper row of the region of interest (ROI) of the traffic sign
	int			rightCol;		// right column of the region of interest (ROI) of the traffic sign
	int			bottomRow;		// lower row of the region of interest (ROI) of the traffic sign
	//std::string	className;		// a string describing the class of the traffic sign (e.g. "speed limit 30")
	int			classID;		// an integer describing the class of the traffic sign (e.g. "speed limit 30"), refer to the ReadMe.txt in your download package for a disambiguation
	Category	category;		// category of the traffic sign (see above)
	int 		found;
};

// union representing one RGB pixel
union RgbaValue
{
	RgbaValue () : m_r(0), m_g(0), m_b(0), m_a(255){}
	RgbaValue (unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : m_r(r), m_g(g), m_b(b), m_a(a){}

	struct
	{
		unsigned char m_b;		// pixel's blue component
		unsigned char m_g;		// pixel's green component
		unsigned char m_r;		// pixel's red component
		unsigned char m_a;		// pixel's transparency component (unused)
	};
};

// This function reads a .ppm image file.
//
// fileName		full name of the file to be read
// ppImg		(inout) if *ppImg is set to 0, the memory for the new image is allocated and the image data is copied to that pointer
//				if *ppImg is not set to 0, the function assumes that the memory has already been allocated
// sx, sy		(out) width and height of the image
// switchRB		(leave this one on default)
// pComments	(leave this one on default)
//
// return	true, iff the file was read successfully
bool readPPM( const char* fileName, RgbaValue** ppImg, int& sx, int& sy, bool switchRB = false, std::vector<std::string>* pComments = 0 );

// This function reads the ground truth data from the file in your download package
//
// aGTFile		the full name of the ground truth file "..../gt.txt"
//
// return		a vector of GTData-structs, each one representing a traffic sign in the dataset
std::vector<GTData> TSD_readGTData( const char* aGTFile );

// This function presents all the images one by one to your detector and compares its results with the ground truth.
//
// benchmarkPath	full path of the directory you extracted the benchmark path to
// category			the category your detector works on (is he specialized to prohibitory, mandatory or danger?)
// detectorFunc		function pointer to your detector function (see the example in main.cpp for details on implementation)
// vebose			states whether or not every detection or miss should be printed to the default output
void TSD_testMyDetector( const std::string & benchmarkPath, Category category, 
	void (*detectorFunc)(const RgbaValue*, int, int ,std::vector<double>*, std::vector<double>*, std::vector<double>*, std::vector<double>*), bool verbose, double jaccard );

#endif //GTSDB_TOOLS_H_INCLUDED
