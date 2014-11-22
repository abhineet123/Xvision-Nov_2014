/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka 
    (JHU Computational Interaction and Robotics Laboratory (CIRL)
     formerly the CIPS lab)

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

/**
 * program arguments:  
 *
 * wait time:  how long the program waits before grabbing
 * the colors from the boxed region
 * 
 * width: the width of the boxed region to grab colors from
 * 
 * height: the height of the boxed region to grab colors from
 *
 * the program can be run with no arguments (default) or
 * with the following combination of arguments:
 *
 * color <wait time>
 * color <width> <height>
 * color <width> <height> <wait time>
 */

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindow.h"
#include "GeoTransform.h"
#include "Mpeg.h"
#include "Bt8x8.h"
#include "XVImageBase.h"
#include <XVColorSeg.h>
#include <vector>

static float preferredHue = 0;

inline bool isHueCheck(const void * voidPixelPointer){
  
    return ((*(u_short *)voidPixelPointer) > (preferredHue - 2) &&
  	  (*(u_short *)voidPixelPointer) < (preferredHue + 2));
};

template <class T>
class DrawWindow : public XVWindow<T>{

public:

  DrawWindow(XVImageBase<T> * im, int x, int y) : XVWindow<T>(im, x, y){}

  void drawRect(int x, int y, int w, int h){

    XDrawRectangle(dpy, window, gc_window[7], x, y, w, h);
  }
};

static  DrawWindow<XV_RGB>    *window;
static  Video<XV_RGB>         *grabber;


int main (int argc, char **argv) {

  int old_frame, framenum = 0, n_buffers, rate = 100;
  int WIDTH  = 640;
  int HEIGHT = 480;

  int boxWidth, boxHeight, waitTime;

  struct timeval time1, time2, startTime, stopTime;

  XVImageRGB<XV_RGB> IM (WIDTH, HEIGHT);

  switch(argc){

  case 2:
    
    waitTime  = atoi(argv[1]);
    boxWidth  = 30;
    boxHeight = 30;
    break;
  case 3:

    boxWidth  = atoi(argv[1]);
    boxHeight = atoi(argv[2]);
    waitTime  = 5;
    break;
  case 4:

    boxWidth  = atoi(argv[1]);
    boxHeight = atoi(argv[2]);
    waitTime  = atoi(argv[3]);
    break;
  default:

    boxWidth  = 30;
    boxHeight = 30;
    waitTime  = 5;
  };

  grabber = new Bt8x8<XV_RGB>();
  grabber -> set_params ("B4I1N0");
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }

  n_buffers=grabber->buffer_count();

  //Set up the window
  window = new DrawWindow<XV_RGB> (&IM, 0, 0);
  window -> setImages (&IM, 1);
  window -> map();
  
  grabber -> initiate_acquire(framenum);

  gettimeofday (&startTime, NULL);

  cout << endl 
       << "Place the color you want to track in the red square" 
       << endl 
       << "It will be grabbed in:" 
       << endl 
       << endl;

  do{

    //Acquire image
    old_frame=framenum;
    framenum = (framenum+1)%n_buffers;
    grabber -> initiate_acquire(framenum);
    grabber -> wait_for_completion(old_frame);
    IM = (grabber -> frame(old_frame));
    
    window -> CopyImage(0);
    window -> drawRect( WIDTH / 2 - boxWidth / 2, HEIGHT / 2 - boxHeight / 2, 
			boxWidth, boxHeight );

    window -> swap_buffers;
    window -> flush();

    gettimeofday(& stopTime, NULL);

    cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" 
	 << waitTime - (stopTime.tv_sec - startTime.tv_sec)
	 << " seconds" << flush;

  }while(stopTime.tv_sec - startTime.tv_sec < waitTime);

  preferredHue = findBestHue(subimage(IM, XVSize(boxWidth, boxHeight),
				      XVPosition(WIDTH / 2, HEIGHT / 2)));

  HUERANGE range = {(HUE)(preferredHue - 2), (HUE)(preferredHue + 2)};
  XVHueRangeSeg<XV_RGB> colorSeg(range);
  XVImageScalar<HUE> segIM;
  vector<XVImageGeneric> regions;
  int updateHueAtIteration = 10;

  cout << endl << "PH is: " << preferredHue << endl;
  
  //Start timing
  gettimeofday (&time1, NULL);
  
  int update = 0;
  for(int i = 1; ; ++i, ++update){

    old_frame=framenum;
    framenum = (framenum+1)%n_buffers;
    grabber -> initiate_acquire(framenum);
    grabber -> wait_for_completion(old_frame);
    IM = (grabber -> frame(old_frame));
    
    window -> CopyImage(0);

    colorSeg.segment(IM, segIM);
    colorSeg.regionGrow(segIM, regions, & isHueCheck, 100);

    if(update % updateHueAtIteration == 0){

      for(int k = 0; k < regions.size(); ++k){
	preferredHue += findBestHue(subimage(IM, regions[k]));
      }
      preferredHue /= regions.size();
      
    }

    for(int j = 0; j < regions.size(); ++j){

      window -> drawRect(regions[j].PosX(), regions[j].PosY(), 
			 regions[j].Width(),  regions[j].Height());
    }

    window -> swap_buffers;
    window -> flush();

    if(i % rate == 0){

      //Produce timing results
      gettimeofday (&time2, NULL);
      cout << "Rate: " << rate / (time2.tv_sec - time1.tv_sec +
				  (time2.tv_usec - time1.tv_usec) * 1e-6) 
	   << " [Hz]" << endl;

      i = 0;
      gettimeofday (&time1, NULL);
    }
  }

  return 0;
}
