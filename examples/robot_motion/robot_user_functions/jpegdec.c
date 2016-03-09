#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"

unsigned char * read_jpeg_file (FILE *infile, int *width, int *height, int *components)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buffer;		/* Output row buffer */
  JSAMPROW buf_row;
  unsigned char *buf;
  int row_stride;		/* physical row width in output buffer */
  int x, y;

  /* Step 1: allocate and initialize JPEG decompression object */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);

  /* Step 4: set parameters for decompression */

  /* In this example, we do nothing here. */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;

  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
  buf = malloc(row_stride * cinfo.output_height);
  if (!buffer || !buf)
  {
  	fprintf(stderr, "Error in read_jpeg_file: insufficient memory.\n");
	exit(1);
  }

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  *width = cinfo.output_width;
  *height = cinfo.output_height;
  *components = cinfo.output_components;

  for (y = 0; y < cinfo.output_height; y++)
  {
	(void) jpeg_read_scanlines(&cinfo, buffer, 1);
	buf_row = buffer[0];
	for (x = 0; x < row_stride; x++)
		buf[x + (y * row_stride)] = buf_row[x];
  }
  	  
  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);

  /* Step 8: Release JPEG decompression object */

  jpeg_destroy_decompress(&cinfo);

  return buf;

}
