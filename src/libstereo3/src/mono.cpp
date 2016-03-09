#include "mono.h"
#include <stdlib.h>

Mono::Mono()
{
	m_Intrinsics = (double**) malloc(3*sizeof(double*));
	for(int i=0; i<3; i++)
		m_Intrinsics[i] = (double*) malloc(3*sizeof(double));

	m_DistCoefs = (double*) malloc(5*sizeof(double));
}
Mono::~Mono()
{
	for (int i=0; i<3; i++)
		free(m_Intrinsics[i]);

	free(m_Intrinsics);
	free(m_DistCoefs);
}

void Mono::setIntrinsics (double fX, double fY, double pX, double pY)
{
	m_Intrinsics[0][0] = fX;
	m_Intrinsics[0][1] = 0.0;
	m_Intrinsics[0][2] = pX;
	m_Intrinsics[1][0] = 0.0;
	m_Intrinsics[1][1] = fY;
	m_Intrinsics[1][2] = pY;
	m_Intrinsics[2][0] = 0.0;
	m_Intrinsics[2][1] = 0.0;
	m_Intrinsics[2][2] = 1.0;
}

void Mono::setDistortion (double k1, double k2, double p1, double p2)
{
	m_DistCoefs[0] = k1;
	m_DistCoefs[1] = k2;
	m_DistCoefs[2] = p1;
	m_DistCoefs[3] = p2;
	m_DistCoefs[4] = 0.0;
}
