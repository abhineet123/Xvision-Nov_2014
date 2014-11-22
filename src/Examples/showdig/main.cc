/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

/* Usage: showdig [parameter-string] [number-of-cameras] [save-files]
   these three parameters can be in any order.
   note that windows for multiple cameras may overlap each other when
   the program start (depends on your windows manager and its settings)
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include "XVDig1394.h"
#include "XVImageRGB.h"
#include "XVImageIO.h"
#include <signal.h>

static  int nCameras = 1 ;
static  XVWindowX<XV_RGB> * window[8];
static	XVVideo<XVImageRGB<XV_RGB> > * grabber[8] ;

void sighndl(int ws) {
  cerr << "sig called" << endl;
  for( int i = 0 ; i < nCameras ; i ++ ) {
    delete grabber[i] ;
  }
  exit(0);
}

int main (int argc, char **argv) {

  int framenum=0, n_buffers, i, j, rate=100 ;
  struct timeval time1, time2; 

  //const char *param="i1V1", * files=0;
  //const char *param="i1V1", * files=0;
  const char * param = "i1V1f7o3r150", * files = 0 ;
  //const char * param = "i1V1f7o3r300", * files = 0 ;
  //const char * param = "h200f2m2o2", * files = 0 ;//1280x960
  //const char * param = "h200m5o2", * files = 0 ;// 640x480
  //const char * param = "h200f1m2o2", * files = 0 ;// 800x600
  for( i = 1, j = 0 ; i < argc ; i ++ ) {
    if( strchr( argv[i], '%' ) ) { // save files
      if( j & 1 ) {
	cout << "excessive parameter for save files:" << argv[i] << endl ;
	return 1 ;
      }else {
	j |= 1 ;
	files = argv[i] ;
      }
    }else if( argv[i][0] >= '0' && argv[i][0] <= '9' ) { // number
      if( j & 2 ) {
	cout << "excessive parameter for number of cameras:" << argv[i] <<endl;
	return 1 ;
      }else {
	j |= 2 ;
	nCameras = atoi( argv[i] );
      }
    }else {  // parameters
      if( j & 4 ) {
	cout << "excessive device parameter:" << argv[i] <<endl;
	return 1 ;	
      }else {
	j |= 4 ;
	param = argv[i] ;
      }
    }
  }
  if( i < argc ) {
    cout << "excessive parameter: " << argv[i] << endl;
    return 1 ;
  }
  signal(SIGINT, sighndl);
  char *filename ;
  int namesize ; 
  if( files ) {
    filename = new char[namesize = strlen(files)+16] ;
  }

  //Set up the device
  for( j = 0 ; j < nCameras ; j ++ ) {
   XVDig1394<XVImageRGB<XV_RGB> > *temp_grab;
   try{  temp_grab = new XVDig1394<XVImageRGB<XV_RGB> >
      ( DC_DEVICE_NAME, param, DIG1394_NTH_CAMERA(j) );
   }
   catch(int n)
   {
     cerr << "Exception " << n << " received from init" << endl;
     return 1;
   }
   temp_grab->set_whitebalance_auto();
   //temp_grab->set_exposure_auto();
   //temp_grab->set_brightness_auto();
   temp_grab->set_shutter_auto();
   grabber[j]=temp_grab;
  }
  n_buffers=grabber[0]->buffer_count(); // assuming same buffer count

  //Set up the window
  for( j = 0 ; j <nCameras ; j ++ ) {
    window[j] = new XVWindowX<XV_RGB> (grabber[j]->frame(0));
    window[j] -> map();
    //window[j]->setImages(&(grabber[j]->frame(0)),n_buffers);
  }
  for( j = 0 ; j <nCameras ; j ++ ) {
    grabber[j]->initiate_acquire(framenum);
  }
  //Start timing
  gettimeofday (&time1, NULL);
  for( i = 0; ; i++ ) {
    //Acquire images
    for( j = 0 ; j <nCameras ; j ++ ) {
      grabber[j]-> initiate_acquire((framenum+1)%n_buffers);
    }
    for( j = 0 ; j <nCameras ; j ++ ) {
      grabber[j]-> wait_for_completion(framenum);
      window[j] -> CopySubImage(grabber[j]->frame(framenum));
      window[j] -> swap_buffers();
      window[j] -> flush();
      if( files ) {
	snprintf( filename, namesize, files, i, j );
	XVWriteImage( grabber[j]->frame(framenum), filename );
      }
    }
    framenum=(framenum+1)%n_buffers;
    if( i % rate == rate-1 ) {
      gettimeofday (&time2, NULL);
      cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
	 (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;
      gettimeofday (&time1, NULL);
    }
  }
  return 0;
}
