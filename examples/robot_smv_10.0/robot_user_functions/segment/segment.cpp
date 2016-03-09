/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include "segment.hpp"
#include <cstdio>
#include <cstdlib>
#include "pnmfile.h"
#include "image.h"
#include "misc.h"
#include "segment-image.h"

char *segment_image (float sigma, float k, int min_size, char *in_image, int width, int height)
{
  printf("loading input image.\n");
  
  segment::image<segment::rgb> *input = segment::charPtrToImage(in_image, width, height);

  printf("processing\n");
  int num_ccs; 

  char *seg = segment::segment_image(input, sigma, k, min_size, &num_ccs); 
  
  printf("got %d components\n", num_ccs);
  printf("done! uff...thats hard work.\n");

  delete input;
  
  return seg;
}
