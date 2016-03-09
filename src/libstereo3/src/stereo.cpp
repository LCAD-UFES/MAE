#include "stereo.h"
#include <stdlib.h>

Stereo::Stereo()
{
	m_Rotation = (double**) malloc(3*sizeof(double*));
	for(int i=0; i<3; i++)
		m_Rotation[i] = (double*) malloc(3*sizeof(double));

	m_Translation = (double*) malloc(5*sizeof(double));

	m_Left = new Mono();
	m_Right = new Mono();
}
Stereo::~Stereo()
{
	for (int i=0; i<3; i++)
		free(m_Rotation[i]);

	free(m_Rotation);
	free(m_Translation);
}

void Stereo::setRotation (double r00, double r01, double r02, double r10, double r11, double r12, double r20, double r21, double r22)
{
	m_Rotation[0][0] = r00;
	m_Rotation[0][1] = r01;
	m_Rotation[0][2] = r02;
	m_Rotation[1][0] = r10;
	m_Rotation[1][1] = r11;
	m_Rotation[1][2] = r12;
	m_Rotation[2][0] = r20;
	m_Rotation[2][1] = r21;
	m_Rotation[2][2] = r22;
}

void Stereo::setTranslation (double t1, double t2, double t3)
{
	m_Translation[0] = t1;
	m_Translation[1] = t2;
	m_Translation[2] = t3;
}
