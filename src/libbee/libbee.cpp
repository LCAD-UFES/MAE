/*! \file
* \brief Bumblebee2 capture library
*/

/*
*********************************************************************************
* Module : Bumblebee2 capture library for MAE applications			*
* version: 1.0									*
*    date: 17/02/2011								*
*      By: Jorcy de Oliveira Neto						*
*********************************************************************************
*/

/********************************************************************************
*
* Original Example Application - simplestereo
* Copyright:	(C) 2006,2007,2008 Don Murray donm@ptgrey.com
*
* Description:
*
*    Get an image set from a Bumblebee or Bumblebee2 via DMA transfer
*    using libdc1394 and process it with the Triclops stereo
*    library. Based loosely on 'grab_gray_image' from libdc1394 examples.
*
*********************************************************************************/

/********************************** WARNING:***********************************************
* For Bumblebee2 camera usage, one must have full permission to use the firewire devices
* The following commands must be propted in the /dev/ as root :
*
* chmod 777 fw0
* chmod 777 fw1
* ln -s fw0 video1394 
*******************************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "include/libbee.hpp"

//
// Macro to check, report on, and handle libdc1394 error codes.
//

#define _HANDLE_LIBDC1394_ERROR(function,error,camera) \
{ \
    if ( error != DC1394_SUCCESS ) \
    { \
        printf( " libdc1394 error in %s: %d\n", function, error ); \
        libbee_cleanup_and_exit(camera); \
    } \
} \

//
// Macro to check, report on, and handle Triclops API error codes.
//
#define _HANDLE_TRICLOPS_ERROR(function,error,camera,triclops) \
{ \
   if( error != TriclopsErrorOk ) \
   { \
	printf( " Triclops error : Error %d reported in %s.\n",error,function); \
	triclopsDestroyContext(triclops); \
	libbee_cleanup_and_exit(camera); \
   } \
} \

// Left or right positions
#define	LEFT 0
#define	RIGHT 1

// Rectified Images Width/Height
#define RECTIFIED_IMAGE_WIDTH	640
#define RECTIFIED_IMAGE_HEIGHT	480

// Global stereo camera context (PGRStereoCamera and dc1394camera)
PGRStereoCamera_t stereoCamera;
dc1394camera_t* 	camera;
// Global Triclops SDK context and error
TriclopsContext triclops;
TriclopsError 	e;

//=============================================================================
// a simple function to write a .pgm file
int
libbee_writePgm( char* 	szFilename,
	 unsigned char* pucBuffer,
	 int		width,
	 int		height )
{
	FILE* stream;
	stream = fopen( szFilename, "wb" );
	if( stream == NULL)
	{
		perror( "Can't open image file" );
		return 1;
	}

	fprintf( stream, "P5\n%u %u 255\n", width, height );
	fwrite( pucBuffer, width, height, stream );
	fclose( stream );
	return 0;
}

//=============================================================================
// a simple function to write a .ppm file
int
libbee_writePpm( char* 	szFilename,
	 unsigned char* pucBuffer,
	 int		width,
	 int		height )
{
	FILE* stream;
	stream = fopen( szFilename, "wb" );
	if( stream == NULL)
	{
		perror( "Can't open image file" );
		return 1;
	}

	fprintf( stream, "P6\n%u %u 255\n", width, height );
	fwrite( pucBuffer, 3*width, height, stream );
	fclose( stream );
	return 0;
}


//=============================================================================
// libbee_cleanup_camera
// This is called for destroying the existing connections
// to the 1394 drivers
void
libbee_cleanup_camera( dc1394camera_t* camera )
{
	dc1394_capture_stop( camera );
	dc1394_video_set_transmission( camera, DC1394_OFF );
	dc1394_camera_free( camera );
}

//=============================================================================
// libbee_cleanup_camera
// This is called when the program exits and destroys the existing connections
// to the 1394 drivers
void
libbee_cleanup_and_exit( dc1394camera_t* camera )
{
	dc1394_capture_stop( camera );
	dc1394_video_set_transmission( camera, DC1394_OFF );
	dc1394_camera_free( camera );
	exit( 0 );
}

//======================================================================================================================
//Initialize global Bumblebee2 camera context both the Stereo Camera and Triclops context
//
void	libbbee_initialize_camera_context(void)
{
	dc1394error_t 		err;
	dc1394_t* 		d;
	dc1394camera_list_t* 	list;
	unsigned int 		nThisCam;

	// Find cameras on the 1394 buses
	d = dc1394_new ();

	// Enumerate cameras connected to the PC
	err = dc1394_camera_enumerate (d, &list);
	if ( err != DC1394_SUCCESS )
	{
		fprintf( stderr, "Unable to look for cameras\n\n"
		"Please check \n"
		"  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' "
		"are loaded \n"
		"  - if you have read/write access to /dev/raw1394\n\n");

		return;
	}

	if (list->num == 0)
	{
		fprintf( stderr, "No cameras found!\n");
		return;
	}

	// Identify cameras. Use the first stereo camera that is found
	for ( nThisCam = 0; nThisCam < list->num; nThisCam++ )
	{
        	camera = dc1394_camera_new(d, list->ids[nThisCam].guid);

		if(!camera)
		{
			printf("Failed to initialize camera with guid %ld", list->ids[nThisCam].guid);
			continue;
		}
		
		if ( isStereoCamera(camera))
			break;

		dc1394_camera_free(camera);		
	}

	if ( nThisCam == list->num )
	{
		printf( "No stereo cameras were detected\n" );
		return;
	}

	// Free memory used by the camera list
	dc1394_camera_free_list (list);

	// query information about this stereo camera
	err = queryStereoCamera( camera, &stereoCamera );
	_HANDLE_LIBDC1394_ERROR("queryStereoCamera",err,camera);

	if ( stereoCamera.nBytesPerPixel != 2 )
	{
		// can't handle XB3 3 bytes per pixel
		fprintf( stderr,"Example has not been updated to work with XB3 in 3 camera mode yet!\n" );
		libbee_cleanup_and_exit( stereoCamera.camera );
	}

	// set the capture mode
	err = setStereoVideoCapture( &stereoCamera );
	_HANDLE_LIBDC1394_ERROR("setStereoVideoCapture",err,stereoCamera.camera);

	// have the camera start sending us data
	err = startTransmission( &stereoCamera );
	_HANDLE_LIBDC1394_ERROR("startTransmission",err,stereoCamera.camera);

	e = getTriclopsContextFromCamera( &stereoCamera, &triclops );
	_HANDLE_TRICLOPS_ERROR("getTriclopsContextFromCamera",e,camera,triclops);

	e = triclopsSetResolution( triclops,RECTIFIED_IMAGE_HEIGHT,RECTIFIED_IMAGE_WIDTH );
	_HANDLE_TRICLOPS_ERROR("triclopsSetResolution",e,camera,triclops);

	/*   TriRectQlty_FAST,
   	TriRectQlty_STANDARD,
   	TriRectQlty_ENHANCED_1,
   	TriRectQlty_ENHANCED_2
	*/
	e = triclopsSetRectImgQuality(triclops, TriRectQlty_FAST);
	_HANDLE_TRICLOPS_ERROR("triclopsSetRectImgQuality",e,camera,triclops);
}

//=====================================
// Terminates the libbee camera context
//
void	libbbee_terminate_camera_context(void)
{
	// Cleanup the camera context
	libbee_cleanup_camera( camera );
}

//======================================================================================================================
// Captures a rectified image from Bumblebee2 camera, it must be selected wheter it is the leftmost or the rightmost one
//

TriclopsColorImage*	libbee_get_rectified_images(int left_or_right)
{

	// a TriclopsInput image
	TriclopsInput input;
	// size of buffer for all images at mono8
	unsigned int   nBufferSize = stereoCamera.nRows * stereoCamera.nCols * stereoCamera.nBytesPerPixel;
	// allocate a buffer to hold the de-interleaved images
	unsigned char* pucDeInterlacedBuffer = new unsigned char[ nBufferSize ];

	// Color Image Capture Buffers
	unsigned char* pucRGBBuffer 	= new unsigned char[ 3 * nBufferSize ];
	unsigned char* pucRedBuffer	= new unsigned char[ nBufferSize ];
	unsigned char* pucGreenBuffer	= new unsigned char[ nBufferSize ];
	unsigned char* pucBlueBuffer	= new unsigned char[ nBufferSize ];
	unsigned char* pucRightRGB	= NULL;
	unsigned char* pucLeftRGB	= NULL;
	unsigned char* pucCenterRGB	= NULL;	

	// B&W Image Capture Buffers
	unsigned char* pucRightMono	= NULL;
	unsigned char* pucLeftMono	= NULL;
	unsigned char* pucCenterMono	= NULL;

	dc1394_video_set_transmission(camera,DC1394_ON);

	if ( stereoCamera.bColor )
	{
		// get the images from the capture buffer and do all required processing
		// note: produces a TriclopsInput that can be used for stereo processing
		extractImagesColor( &stereoCamera,
			DC1394_BAYER_METHOD_NEAREST,
			pucDeInterlacedBuffer,
			pucRGBBuffer,
			pucRedBuffer,
			pucGreenBuffer,
			pucBlueBuffer,
			&pucRightRGB,
			&pucLeftRGB,
			&pucCenterRGB,
			&input );
	}
	else
	{
		// get the images from the capture buffer and do all required processing
		// note: produces a TriclopsInput that can be used for stereo processing
		extractImagesMono( &stereoCamera,
			pucDeInterlacedBuffer,
			&pucRightMono,
			&pucLeftMono,
			&pucCenterMono,
			&input );
	}
	
	// make sure we are in subpixel mode
	triclopsSetSubpixelInterpolation( triclops, 1 );
	_HANDLE_TRICLOPS_ERROR("triclopsSetSubpixelInterpolation",e,camera,triclops);

	// get the left or right rectified image	
	TriclopsColorImage 	*image;
	image = new(TriclopsColorImage);

	if(left_or_right == LEFT)
	{
		unsigned char * aux;
		aux = (unsigned char*)input.u.rgb.red;
		input.u.rgb.red = &aux[stereoCamera.nCols * stereoCamera.nRows];
		aux = (unsigned char*)input.u.rgb.green;
		input.u.rgb.green = &aux[stereoCamera.nCols * stereoCamera.nRows];
		aux = (unsigned char*)input.u.rgb.blue;
		input.u.rgb.blue = &aux[stereoCamera.nCols * stereoCamera.nRows];

		e = triclopsRectifyColorImage( triclops, TriCam_LEFT, &input,image );
		_HANDLE_TRICLOPS_ERROR("triclopsRectifyPackedColorImage",e,camera,triclops);
	}	
	else
	{
		e = triclopsRectifyColorImage( triclops, TriCam_RIGHT, &input, image );
		_HANDLE_TRICLOPS_ERROR("triclopsRectifyPackedColorImage",e,camera,triclops);
	}

	delete[] pucDeInterlacedBuffer;
	
	if ( pucRGBBuffer )
		delete(pucRGBBuffer);
	if ( pucRedBuffer )
		delete(pucRedBuffer);
	if ( pucGreenBuffer )
		delete(pucGreenBuffer);
	if ( pucBlueBuffer )
		delete(pucBlueBuffer);

	dc1394_video_set_transmission(camera,DC1394_OFF);
	//dc1394_video_set_one_shot(camera,DC1394_ON);
	
	return(image);
}

//=====================================================================
// frees the alloc'ed TriclopsImage in libbee_get_rectified_images
//
void	libbee_destroy_flycapture_image(TriclopsColorImage *image)
{
	if(image)
		delete(image);
}
