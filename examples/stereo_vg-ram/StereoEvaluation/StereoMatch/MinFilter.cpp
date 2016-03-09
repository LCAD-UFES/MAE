///////////////////////////////////////////////////////////////////////////
//
// NAME
//  MinFilter.cpp -- separable min/max filter
//
// DESCRIPTION
//  Performs a separable box filtering using efficient running sum code.
//
// DESIGN NOTES
//  First version - uses single variable for minimum:
//   potentially inefficent due to large strides in vertical direction
//
// SEE ALSO
//  MinFilter.h         longer description of the interface
//
// Copyright © Daniel Scharstein and Richard Szeliski, 2001.
// See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

#include "Image.h"
#include "Error.h"
#include "Convert.h"
#include "BoxFilter.h"
#include "MinFilter.h"

// new, faster version: only recompute minimum if old min value leaves current interval


