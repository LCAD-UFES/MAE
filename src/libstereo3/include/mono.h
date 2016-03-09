#include <stdexcept>
#include <exception>

#ifdef _POSIX_VERSION
#include <stdlib.h>
#endif

#ifndef MonoCamera_h
#define MonoCamera_h 1

class Mono
{
   public:
	Mono();

	~Mono();

	void setIntrinsics(double fX, double fY, double pX, double pY);

	void setDistortion(double k1, double k2, double p1, double p2);

	double** getIntrinsics(void){ return m_Intrinsics; }

	double* getDistortion(void){ return m_DistCoefs; }

	private:
		double** m_Intrinsics;
		double* m_DistCoefs;
};
#endif
