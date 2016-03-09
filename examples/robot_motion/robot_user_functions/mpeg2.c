/*
 * Copyright (C) 2003      Regis Duchesne <hpreg@zoy.org>
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mpeg2.h"
#include "mpeg2convert.h"

#define BUFFER_SIZE 4096


void save_rgb_frame (mpeg2_rgb_frame_t * rgb_frame)
{
    char filename[30];
    FILE * rgbfile;

    sprintf (filename, "frame%6d.rgb", rgb_frame->num);
    rgbfile = fopen (filename, "wb");
    if (!rgbfile)
    {
	fprintf (stderr, "Could not open file \"%s\".\n", filename);
	exit (1);
    }
    fwrite (rgb_frame->buf, 3 * rgb_frame->width, rgb_frame->height, rgbfile);
    fclose (rgbfile);
}


int open_mpeg_file (mpeg2_rgb_frame_t * rgb_frame)
{
    rgb_frame->mpeg2file = fopen (rgb_frame->mpeg2filename, "rb");
    if (!rgb_frame->mpeg2file)
    {
	fprintf (stderr, "Could not open file \"%s\".\n",
			  rgb_frame->mpeg2filename);
	exit (1);
    }

    rgb_frame->file_buffer = (uint8_t *) malloc (BUFFER_SIZE);
    if (!rgb_frame->file_buffer)
    {
	fprintf (stderr, "Could not allocate a file buffer.\n");
	exit (1);
    }

    rgb_frame->decoder = mpeg2_init ();
    if (!rgb_frame->decoder)
    {
	fprintf (stderr, "Could not allocate a decoder object.\n");
	exit (1);
    }
    return 1;
}

int close_mpeg_file (mpeg2_rgb_frame_t * rgb_frame)
{
    mpeg2_close (rgb_frame->decoder);
    free (rgb_frame->file_buffer);
    fclose (rgb_frame->mpeg2file);
    return 1;
}


int read_mpeg_frame (mpeg2_rgb_frame_t * rgb_frame)
{
    const mpeg2_info_t * info;
    mpeg2_state_t state;
    size_t size;
    static int frame_num = 0;

    info = mpeg2_info (rgb_frame->decoder);
    size = (size_t)-1;
    do {
	state = mpeg2_parse (rgb_frame->decoder);
	switch (state)
	{
	case STATE_BUFFER:
	    size = fread (rgb_frame->file_buffer, 1, BUFFER_SIZE,
	    		  rgb_frame->mpeg2file);
	    mpeg2_buffer (rgb_frame->decoder, rgb_frame->file_buffer, 
	    		  rgb_frame->file_buffer + size);
	    break;
	case STATE_SEQUENCE:
	    mpeg2_convert (rgb_frame->decoder, mpeg2convert_rgb24, NULL);
	    break;
	case STATE_SLICE:
	case STATE_END:
	case STATE_INVALID_END:
	    if (info->display_fbuf)
	    {
	        rgb_frame->width = info->sequence->width;
		rgb_frame->height = info->sequence->height;
		rgb_frame->period = info->sequence->frame_period;
		rgb_frame->buf = info->display_fbuf->buf[0];
		if (info->discard_fbuf)
		    rgb_frame->previous = info->discard_fbuf->buf[0];
		rgb_frame->num = ++frame_num;
		return frame_num;
	    }
	default:
	    break;
	}
    } while (size);
    
    return 0;
}
