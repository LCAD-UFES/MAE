#ifndef __MODEL_3D_H
#define __MODEL_3D_H

#include <map>
using std::map;

#include <vector>
using std::vector;

#include <GL/glut.h>

#include "point_cloud.hpp"
using mae::smv::PointCloud;

#include "working_area.hpp"
using mae::smv::WorkingArea;

#ifdef __cplusplus
extern "C"
{
#endif

#include "cylinder_list.h"

#ifdef __cplusplus
}
#endif

namespace mae
{
	namespace smv
	{
		class Model3D;
	}
}

/*
Seção de Constantes
*/

#define NUM_CAMERA_PAIRS 3

#define MODEL_3D_X_AXIS 0
#define MODEL_3D_Y_AXIS 1
#define MODEL_3D_Z_AXIS 2

/*
Seção de Tipos de Dados
*/

class mae::smv::Model3D
{
	/*
	Inner Class Section
	*/

	public: class Face
	{
		/*
		Attribute Section
		*/

		private: vector<double> rightWorldPoints;

		private: vector<GLubyte> red, green, blue;

		private: vector<double> eulerAngles, displacement, massCenter;

		private: CYLINDER_LIST *cylinders;
		
		private: map<int, CYLINDER> transformed;

		/*
		Constructor Section
		*/

		public: Face();

		/*
		Method Section
		*/

		private: void computeMassCenter();
		
		private: void computeTransformed();

		private: string stateString();

		private: void transformCylinder(CYLINDER& cylinder);

		public: void copyCylinders(CYLINDER_LIST* list, int step);

		public: void extractPointCloud(vector<double>& points, vector<GLubyte> red, vector<GLubyte> green, vector<GLubyte> blue);

		public: CYLINDER& transformedCylinder(int index);
		
		public: CYLINDER& highestCylinder();

		public: CYLINDER& lowestCylinder();
		
		public: void removeCylinder(int index);

		public: void center();

		public: void rotate(int axis, double value);

		public: void straight();

		public: void translate(int axis, double value);
		
		public: string toString();

		/*
		Input / Output Section
		*/

		void loadConfig(string basename);

		void saveConfig(string basename);

		/*
		Property Section
		*/

		public: CYLINDER_LIST* getCylinders();

		public: CYLINDER& getCylinders(int index);

		public: void setCylinders(CYLINDER_LIST* cylinders);
		
		public: int getCylindersSize();

		public: void setPointCloud(PointCloud& cloud);

		public: int getSize();

		public: void setSize(int size);
	};

	/*
	Attribute Section
	*/

	private: static const double INVALID_VOLUME = -1.0;

	private: vector<Face> faces;

	private: WorkingArea workingArea;

	private: double solidVolume;

	private: double stereoVolume;

	private: double length, height, width;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	public: Model3D();

	/*
	Method Section
	*/

	private: void computeSolidVolume();

	private: void computeStereoVolume();

	public: void center(int index);

	public: void rotate(int index, int axis, double value);

	public: void straight(int index);

	public: void translate(int index, int axis, double value);

	/*
	Property Section
	*/

	public: CYLINDER_LIST* getAllCylinders();

	public: Face& getFace(int index);

	public: double getLength();

	public: double getHeight();

	public: double getWidth();

	/**
	Returns this model's solid volume.

	@return This model's solid volume.
	*/
	public: double getSolidVolume();

	/**
	Returns this model's stereo volume.

	@return This model's stereo volume.
	*/
	public: double getStereoVolume();

	public: void setCylinders(int index, CYLINDER_LIST* cylinders);

	public: void setPointCloud(int index, PointCloud& cloud);
};

#endif
