/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#include "working_area.hpp"

#include <stdexcept>

#include "primitives.hpp"

using std::runtime_error;

using mae::util::Primitives;

using mae::smv::WorkingArea;

/*
Constructor Section
*/

WorkingArea::WorkingArea()
{
	corners[0] = 0;
	corners[1] = 0;
	corners[2] = 0;
	corners[3] = 0;
}

WorkingArea::WorkingArea(int x0, int y0, int xn, int yn)
{
	corners[0] = x0;
	corners[1] = y0;
	corners[2] = xn;
	corners[3] = yn;
}

/*
Operator Section
*/

int& WorkingArea::operator[](int index)
{
	if (0 <= index && index < 4)
		return corners[index];
	else
		throw runtime_error(string() + "Index [" + index + "] is off the valid range [0..3]");
}

/*
Method Section
*/

int WorkingArea::x0()
{
	return corners[0];
}

int WorkingArea::y0()
{
	return corners[1];
}

int WorkingArea::xn()
{
	return corners[2];
}

int WorkingArea::yn()
{
	return corners[3];
}

string WorkingArea::toString()
{
	return string() + "{" + corners[0] + ", " + corners[1] + ", " + corners[2] + ", " + corners[3] + "}";
}

/*
Property Section
*/

void WorkingArea::setCorners(int x0, int y0, int xn, int yn)
{
	corners[0] = x0;
	corners[1] = y0;
	corners[2] = xn;
	corners[3] = yn;
}
