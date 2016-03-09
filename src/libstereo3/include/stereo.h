#include <stdexcept>
#include <exception>
#include "mono.h"

#ifndef StereoCamera_h
#define StereoCamera_h 1

class Stereo
{
   public:
	Stereo();

	~Stereo();

	void setRotation(double r00, double r01, double r02, double r10, double r11, double r12, double r20, double r21, double r22);

	void setTranslation(double t1, double t2, double t3);

	double** getRotation(void){ return m_Rotation; }

	double* getTranslation(void){ return m_Translation; }

	Mono* getLeft(void){ return m_Left; }

	Mono* getRight(void){ return m_Right; }

	private:
		double** m_Rotation;
		double* m_Translation;
		Mono* m_Left;
		Mono* m_Right;
};
#endif
