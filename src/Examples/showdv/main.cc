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

/* Usage: showdv [save-files] */

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include "XVAffineWarp.h"
#include "DV1394.h"
#include "XVImageRGB.h"
#include "XVImageIO.h"
#include <signal.h>

static  XVWindowX<XV_RGB>      *window;
static  XVVideo<XVImageRGB<XV_RGB24> >         *grabber;

void sighndl(int ws) {
  cerr << "sig called" << endl;
  delete grabber ;
  exit(0);
}

int main (int argc, char **argv) {

  int framenum=0, n_buffers, i, j, rate=100 ;
  struct timeval time1, time2; 

  const char * files = 0 ;
  for( i = 1, j = 0 ; i < argc ; i ++ ) {
    if( strchr( argv[i], '%' ) ) { // save files
      if( j & 1 ) {
	cout << "excessive parameter for save files:" << argv[i] << endl ;
	return 1 ;
      }else {
	j |= 1 ;
	files = argv[i] ;
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
  grabber = new DV1394<XVImageRGB<XV_RGB24> >( DV_DEVICE_NAME );
  n_buffers=grabber->buffer_count(); // assuming same buffer count

  //Set up the window
  window = new XVWindowX<XV_RGB> (grabber->frame(0));
  window -> map();

  grabber->initiate_acquire(framenum);

  //Start timing
  gettimeofday (&time1, NULL);
  for( i = 0; ; i++ ) {
    //Acquire images
    grabber-> initiate_acquire((framenum+1)%n_buffers);
    grabber-> wait_for_completion(framenum);
    window-> CopySubImage(grabber->frame(framenum));
    window-> swap_buffers();
    window-> flush();
    if( files ) {
      snprintf( filename, namesize, files, i, j );
      XVWriteImage( grabber->frame(framenum), filename );
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
