/*! \file
* \brief Stereo system class module.
*/

#include "stereo_system.hpp"



/*! 
*********************************************************************************
* \brief Creates a stereo pair object.
* \pre None.
* \post A stereo system object created.
* \return None.
*********************************************************************************
*/

TStereoSystem::TStereoSystem (void)
{
	F.SetSize(3,3);
	el.SetSize(3,1);
	er.SetSize(3,1);
}



/*! 
*********************************************************************************
* \brief Destroys a stereo pair object.
* \pre A stereo system object created.
* \post A stereo system object destroyed.
* \return None.
*********************************************************************************
*/

TStereoSystem::~TStereoSystem (void)
{
}



/*! 
*********************************************************************************
* \brief Calculates the fundamental matrix and the left and right cameras epipoles.
* \pre A stereo system object created.
* \post The fundamental matrix and the epipoles computed.
* \return None.
*********************************************************************************
*/

bool TStereoSystem::CalculateFundamentalMatrix (void)
{
	/*!
	*********************************************************************************
	* Puts the right and left camera projection matrixes on the following form 
	* P = K[I|0] and P' = K'[R|t], respectively. The original camera matrixes 
	* are P = K[R|t] and P' = K'[R'|t']. The relationship between the left and 
	* right camera frame coordinates is given by the equation X' = RX + t.
	* The new values of R and t are related to the old ones by the equation 
	* R := R'~R and t := t' - (R'~R)t, where ~M is the transpose of matrix M
	* and the symbol := denotes the assignment operator.
	*********************************************************************************
	*/
	
	TMatrix R(3,3);
	R = leftCamera.R * ~rightCamera.R;	// R := R'~R
	cout << "The new rotation matrix (R):\n" << R << endl;
	
	TMatrix t(3,1);
	t = leftCamera.t - R * rightCamera.t;	// t := t'- (R'~R)t
	cout << "The new translation vector (t):\n" << t << endl;

	/*!
	*********************************************************************************
	* Calculates the fundamental matrix using the equation F = [P'C]x P'P+,
	* where M+ is the pseudo-inverse of matrix M and [V]x is the skew-symetric
	* matrix of vector V. Furthermore, one can compute the fundamental matrix 
	* using the formula F = [K't]x K'R!K, where !M is the inverse of matrix M.
	*********************************************************************************
	*/

	F = (leftCamera.K * t).SkewSymetric () * leftCamera.K * R * !rightCamera.K;	// F = [K't]x K'R!K
	cout << "The fundamental matrix (F):\n" << F << endl;

	/*!
	*********************************************************************************
	* Calculates the left and right epipoles ruled by the equations e = K~Rt 
	* and e' = K't.
	*********************************************************************************
	*/

	er = rightCamera.K * ~R * t;	// e = K~Rt
	cout << "Right camera epipole (e):\n" << er << endl;

	el = leftCamera.K * t;		// e' = K't
	cout << "Left camera epipole (e'):\n" << el << endl;
	
	/*!
	*********************************************************************************
	* Sets the world coordinates frame as the right camera coordinates frame
	*********************************************************************************
	*/

	rightCamera.R.Unit(3);
	rightCamera.t.Null(3,1);
	rightCamera.P = rightCamera.K * (rightCamera.R | rightCamera.t);
	cout << "Right camera projection matrix (P)\n" << rightCamera.P << endl;

	leftCamera.R = R;
	leftCamera.t = t;
	leftCamera.P = leftCamera.K * (leftCamera.R | leftCamera.t);
	cout << "Left camera projection matrix (P')\n" << leftCamera.P << endl;

	/*!
	*********************************************************************************
	* Updates the left and right camera's orientation constants
	*********************************************************************************
	*/
	leftCamera.constants.Tx = t(0,0);
	leftCamera.constants.Ty = t(1,0);
	leftCamera.constants.Tz = t(2,0);
	leftCamera.constants.Rx -= rightCamera.constants.Rx;
	leftCamera.constants.Ry -= rightCamera.constants.Ry;
	leftCamera.constants.Rz -= rightCamera.constants.Rz;
	rightCamera.constants.Tx = rightCamera.constants.Ty = rightCamera.constants.Tz = 0.0;
	rightCamera.constants.Rx = rightCamera.constants.Ry = rightCamera.constants.Rz = 0.0;

	return (true);
}



/*! 
*********************************************************************************
* \brief Calculates the left epipolar line l'.
* Calculates the left epipolar line l' associated with the right image point x 
* using equation l' = Fx. Every point x' on line l' satisfies the condition ~x'Fx = 0.
* \param[in] p_dblRightImagePoint The right image point x given in homogeneous coordinates.
* \pre 
* \post 
* \return The left epipolar line l': Ax + By + C = 0 associated with the right image point x.
*********************************************************************************
*/

double *TStereoSystem::GetLeftEpipolarLine (double *p_dblRightImagePoint)
{
	static double p_dblLineConstants[3];

	try 
	{
		TMatrix xr(3,1);
		xr(0,0) = p_dblRightImagePoint[0];
		xr(1,0) = p_dblRightImagePoint[1];
		xr(2,0) = 1.0;

		TMatrix ll(3,1);
		ll = F * xr;	// l' = Fx

		p_dblLineConstants[0] = ll(0,0);
		p_dblLineConstants[1] = ll(1,0);
		p_dblLineConstants[2] = ll(2,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (p_dblLineConstants);
}



/*! 
*********************************************************************************
* \brief Calculates the right epipolar line l.
* Calculates the right epipolar line l associated with the left image point x'
* using equation l = ~Fx'. Every point x on line l satisfies the condition ~x'Fx = 0.
* \param[in] p_dblLeftImagePoint the left image point x' given in homogeneous coordinates.
* \pre 
* \post 
* \return The right epipolar line l: Ax + By + C = 0 associated with the left image point x'.
*********************************************************************************
*/

double *TStereoSystem::GetRightEpipolarLine (double *p_dblLeftImagePoint)
{
	static double p_dblLineConstants[3];

	try 
	{
		TMatrix xl(3,1);
		xl(0,0) = p_dblLeftImagePoint[0];
		xl(1,0) = p_dblLeftImagePoint[1];
		xl(2,0) = 1.0;

		TMatrix lr(3,1);
		lr = ~F * xl;	// l = ~Fx'

		p_dblLineConstants[0] = lr(0,0);
		p_dblLineConstants[1] = lr(1,0);
		p_dblLineConstants[2] = lr(2,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (p_dblLineConstants);
}



/*! 
*********************************************************************************
* \brief Gets the left epipole e'.
* \pre 
* \post 
* \return The left epipole e' if OK.
*********************************************************************************
*/

double *TStereoSystem::GetLeftEpipole (void)
{
	static double p_dblEpipole[2];

	try 
	{
		p_dblEpipole[0] = el(0,0);
		p_dblEpipole[1] = el(1,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (p_dblEpipole);
}



/*! 
*********************************************************************************
* \brief Gets the right epipole e.
* \pre 
* \post 
* \return The right epipole e if OK.
*********************************************************************************
*/

double *TStereoSystem::GetRightEpipole (void)
{
	static double p_dblEpipole[2];

	try 
	{
		p_dblEpipole[0] = er(0,0);
		p_dblEpipole[1] = er(1,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (p_dblEpipole);
}



/*! 
*********************************************************************************
* \brief Calculates the slope of the right epipolar line l.
* Calculates the slope of the right epipolar line l associated with the right image point x 
* using equation l = e ^ x = [e]x x, where the operator ^ denotes the cross product.
* \param[in] p_dblRightImagePoint The right image point x given in homogeneous coordinates.
* \pre 
* \post 
* \return The right epipolar line slope.
*********************************************************************************
*/

double TStereoSystem::GetRightEpipolarLineSlope (double *p_dblRightImagePoint)
{
	double dblLineSlope;

	try 
	{
		TMatrix xr(2,1);
		xr(0,0) = p_dblRightImagePoint[0];
		xr(1,0) = p_dblRightImagePoint[1];
		
		TMatrix lr(3,1);
		lr = er.SkewSymetric() * xr;	// l = [e]x x

		dblLineSlope = atan (-lr(0,0)/lr(1,0));
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (dblLineSlope);
}



/*! 
*********************************************************************************
* \brief Calculates the slope of the left epipolar line l'.
* Calculates the slope of the left epipolar line l' associated with the left image point x'
* using equation l' = e' ^ x' = [e']x x'.
* \param[in] p_dblLeftImagePoint The left image point x' given in homogeneous coordinates.
* \pre 
* \post 
* \return The left epipolar line slope.
*********************************************************************************
*/

double TStereoSystem::GetLeftEpipolarLineSlope (double *p_dblLeftImagePoint)
{
	double dblLineSlope;

	try 
	{
		TMatrix xl(2,1);
		xl(0,0) = p_dblLeftImagePoint[0];
		xl(1,0) = p_dblLeftImagePoint[1];

		TMatrix ll(3,1);
		ll = el.SkewSymetric() * xl;	// l' = [e']x x'

		dblLineSlope = atan (-ll(0,0)/ll(1,0));
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (dblLineSlope);
}



/*! 
*********************************************************************************
* \brief Calculates the left infinity point x' corresponding to the an right point x.
* \param[ref] p_dblLeftPoint The left image point x' given in homogeneous coordinates.
* \param[in] p_dblRightPoint The right image point x given in homogeneous coordinates.
* \pre 
* \post 
* \return None.
*********************************************************************************
*/

void TStereoSystem::GetLeftInfinityPoint (double *p_dblLeftPoint, double *p_dblRightPoint)
{
	try 
	{
		TMatrix xr(3,1);
		xr(0,0) = p_dblRightPoint[0];
		xr(1,0) = p_dblRightPoint[1];
		xr(2,0) = 1.0;

		TMatrix A(3,4);
		A = rightCamera.P;
		A.SetSize(3,3);

		TMatrix B(3,4);
		B = leftCamera.P;
		B.SetSize(3,3);

		TMatrix xl(3,1);
		xl = B * !A * xr;
		cout << "Left infinity point:\n " << xl << endl;

		p_dblLeftPoint[0] = xl(0,0) / xl(2,0);
		p_dblLeftPoint[1] = xl(1,0) / xl(2,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return;
}



/*! 
*********************************************************************************
* \brief Calculates the right infinity point x corresponding to the an left point x'.
* \param[ref] p_dblRightPoint The right image point x given in homogeneous coordinates.
* \param[in] p_dblLeftPoint The left image point x' given in homogeneous coordinates.
* \pre 
* \post 
* \return None.
*********************************************************************************
*/

void TStereoSystem::GetRightInfinityPoint (double *p_dblRightPoint, double *p_dblLeftPoint)
{
	try 
	{
		TMatrix xl(3,1);
		xl(0,0) = p_dblLeftPoint[0];
		xl(1,0) = p_dblLeftPoint[1];
		xl(2,0) = 1.0;

		TMatrix A(3,4);
		A = leftCamera.P;
		A.SetSize(3,3);

		TMatrix B(3,4);
		B = rightCamera.P;
		B.SetSize(3,3);

		TMatrix xr(3,1);
		xr = B * !A * xl;
		cout << "Right infinity point:\n " << xr << endl;
		
		p_dblRightPoint[0] = xr(0,0) / xr(2,0);
		p_dblRightPoint[1] = xr(1,0) / xr(2,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return;
}



/*! 
*********************************************************************************
* \brief Maps the image coordinates to the world coordinates.
* \param[in] p_dblRightImagePoint The right image point.
* \param[in] p_dblLeftImagePoint The left image point.
* \pre The stereo module initialized, the cameras calibrated and the epipolar geometry assembled.
* \post None.
* \return A pointer to he world point if OK, NULL otherwise.
*********************************************************************************
*/

double *TStereoSystem::Image2WorldPoint (double *p_dblRightImagePoint, double *p_dblLeftImagePoint)
{
	static double p_dblWorldPoint[3];

	try
	{	
		/*!
		*********************************************************************************
		* Mounts the 3x1 right image point in homogeneous coordinates.
		*********************************************************************************
		*/
		TMatrix xr(3,1);
		xr(0,0) = p_dblRightImagePoint[0]; xr(1,0) = p_dblRightImagePoint[1]; xr(2,0) = 1.0;
		//cout << "Right image point (x):\n" << xr << endl;
		
		/*!
		*********************************************************************************
		* Mounts the 3x1 left image point in homogeneous coordinates.
		*********************************************************************************
		*/
		TMatrix xl(3,1);
		xl(0,0) = p_dblLeftImagePoint[0];  xl(1,0) = p_dblLeftImagePoint[1];  xl(2,0) = 1.0;
		//cout << "Left image point (x'):\n" << xl << endl;

		TMatrix Ar(3,4);
		Ar = xr.SkewSymetric() * rightCamera.P; // [x]x (PX) = 0
		Ar.SetSize(2,4);	// Discards the third LD equation

		TMatrix Al(3,4);
		Al = xl.SkewSymetric() * leftCamera.P;	// [x']x (P'X) = 0
		Al.SetSize(2,4);	// Discards the third LD equation

		/*!
		*********************************************************************************
		* Mounts the 4x4 matrix A with two pair of equations extracted from the right and
		* left image points, respectively.
		*********************************************************************************
		*/
		TMatrix A(4,4);
		A = ~(~Ar | ~Al);	// Concatenates the four equations
		//cout << "Linear homogeneous system (A):\n" << A << endl;
		
		/*!
		*********************************************************************************
		* Transforms the homogeneous system AX = 0 into the inhomogeneous system MX' = b.
		*********************************************************************************
		*/
		TMatrix b(4,1);
		TMatrix M(4,3);
		b = -A.Col(3);
		//for (int i = 0; i < 4; i++) { b(i,0) = -A(i,3); }	// b = [-A(:,3)]
		A.SetSize(4,3);
		M = A;	// M = [A(:,0) | A(:,1) | A(:,2)]
		
		/*!
		*********************************************************************************
		* Solve the inhomogeneous system MX' = b using the least-squares minimization method 
		* (pseudo-inverse approach).
		*********************************************************************************
		*/
		TMatrix X(3,1);
		X = M.PseudoInv() * b;	// X' = M+ * b
		//cout << "World point (X):\n" << X << endl;

		p_dblWorldPoint[0] = X(0,0);
		p_dblWorldPoint[1] = X(1,0);
		p_dblWorldPoint[2] = X(2,0);
	}
	catch (TMatrixError e)
	{
		throw TModuleError(e.what());
	}
	catch (...)
	{
		throw TModuleError("Unknown error type.");
	}

	return (p_dblWorldPoint);
}
