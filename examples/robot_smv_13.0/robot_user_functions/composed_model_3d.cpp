/*
This file is part of SMV.

History:

* 2006.03.22 - Created by Helio Perroni Filho
*/

#include "composed_model_3d.hpp"
using mae::smv::ComposedModel3D;

#include "logging.hpp"
using mae::logging::trace;

#include "model_3d_io.hpp"

#include "primitives.hpp"
using mae::util::Primitives;

#include <math.h>

/*
Nested Class Section
*/

ComposedModel3D::ComposingModel3D::ComposingModel3D()
{
}

double ComposedModel3D::ComposingModel3D::getComposingSolidVolume()
{
	return 0;
}

double ComposedModel3D::ComposingModel3D::getComposingStereoVolume()
{
	return (getLength() - offset) * getWidth() * getHeight() / pow(10, 9);
}

void ComposedModel3D::ComposingModel3D::setOffset(double offset)
{
	this->offset = offset;
}

/*
Constructor Section
*/

ComposedModel3D::ComposedModel3D(): components(0)
{
	trace("ComposedModel3D::ComposedModel3D");
}

/*
Method Section
*/

Model3D& ComposedModel3D::newModel3D(double offset)
{
	int size = components.size() + 1;
	components.resize(size);

	ComposingModel3D& model3d = components.back();
	model_3d_load_config(model3d);
	model3d.setOffset(offset);
	return model3d;
}

void ComposedModel3D::clear()
{
	components.resize(0);
}

/*
Property Section
*/

Model3D& ComposedModel3D::getModel3D(unsigned int index)
{
	trace("ComposedModel3D::getModel3D", string() + index);
	
	if (components.size() <= index)
	{
		components.resize(index + 1);
		Model3D& model3d = components.at(index);
		model_3d_load_config(model3d);
		
		return model3d;
	}
	else
		return components.at(index);
}

double ComposedModel3D::getLength()
{
	double length = 0.0;
	for (vector<ComposingModel3D>::iterator i = components.begin(), n = components.end(); i != n; i++)
	{
		ComposingModel3D& model3d = (*i);
		length += model3d.getLength();
	}
	
	return length;
}

double ComposedModel3D::getWidth()
{
	double width = 0.0;
	for (vector<ComposingModel3D>::iterator i = components.begin(), n = components.end(); i != n; i++)
	{
		ComposingModel3D& model3d = (*i);
		width += model3d.getWidth();
	}
	
	return width / components.size();
}

double ComposedModel3D::getHeight()
{
	double height = 0.0;
	for (vector<ComposingModel3D>::iterator i = components.begin(), n = components.end(); i != n; i++)
	{
		ComposingModel3D& model3d = (*i);
		height += model3d.getHeight();
	}
	
	return height / components.size();
}

double ComposedModel3D::getSolidVolume()
{
	double solidVolume = 0.0;
	for (vector<ComposingModel3D>::iterator i = components.begin(), n = components.end(); i != n; i++)
	{
		ComposingModel3D& model3d = (*i);
		solidVolume += model3d.getSolidVolume();
	}
	
	return solidVolume;
}

double ComposedModel3D::getStereoVolume()
{
	double stereoVolume = 0.0;
	for (vector<ComposingModel3D>::iterator i = components.begin(), n = components.end(); i != n; i++)
	{
		ComposingModel3D& model3d = (*i);
		stereoVolume += model3d.getStereoVolume();
	}
	
	return stereoVolume;
}
