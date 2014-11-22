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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindow.h"
#include "Bt8x8.h"
#include "XVImageBase.h"
#include "XVImageIterator.h"
#include "XVColorSeg.h"
#include <pthread.h>
#include <vector>

class SelectWindow;

static  SelectWindow          * window;
static  Video<XV_RGB>         * grabber;
static  XVImageGeneric        * region;

static struct timespec       delay;
static bool                  selected = FALSE;

static u_short               preferredHue;

class SelectWindow : public XVWindow<XV_RGB>{

protected:

public:

  SelectWindow(XVImageRGB<XV_RGB> * im, int xpos, int ypos) : XVWindow<XV_RGB>(im, xpos, ypos){}

  void drawRectangle(XVImageGeneric & rect){

    XDrawRectangle(dpy, window, (GC)gc_window[8], rect.PosX(), rect.PosY(), rect.Width(), rect.Height());
  };

  void grabPointer(){

    XGrabPointer(dpy, window, True, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
  };

  void unGrabPointer(){

    XUngrabPointer(dpy, CurrentTime);
  };

  // has to be static to be used as the start function
  // for a thread
  static void * selectRegion(void * voidThisP){

    XEvent xev;

    XVPosition ulc(-1, -1);
    bool mouseDown = FALSE;

    SelectWindow * pseudoThis = (SelectWindow *)voidThisP;

    while(1){
      
      if(XCheckMaskEvent(pseudoThis->dpy, ButtonPressMask, &xev) && !mouseDown){

	cout << "ULC: " << xev.xbutton.x << ", " << xev.xbutton.y << endl;
	cout << "000::000" << endl;
	ulc.reposition(xev.xbutton.x, xev.xbutton.y);
	mouseDown = TRUE;
      }else if(XCheckMaskEvent(pseudoThis->dpy, ButtonReleaseMask, &xev) && mouseDown){

	if(xev.xbutton.x > ulc.PosX() && xev.xbutton.y > ulc.PosY()
	   && ulc.PosX() != -1 && ulc.PosY() != -1){

	  region = new XVImageGeneric(XVSize(xev.xbutton.x - ulc.PosX(),
					     xev.xbutton.y - ulc.PosY()), 
				      ulc);

	  selected = TRUE;
	  pthread_exit(NULL);
	  return NULL;
	}
	ulc.reposition(-1, -1);
	mouseDown = FALSE;
      }else if(XCheckMaskEvent(pseudoThis->dpy, PointerMotionMask, &xev) && mouseDown){
	
	if(xev.xmotion.x > ulc.PosX() && xev.xmotion.y > ulc.PosY()
	   && ulc.PosX() != -1 && ulc.PosY() != -1){
	
	  cout << "\b\b\b\b\b\b\b\b" << (xev.xbutton.x - ulc.PosX()) 
	       << "::"               << (xev.xbutton.x - ulc.PosY()) << std::flush;

	  XDrawRectangle(pseudoThis->dpy, pseudoThis->window, (GC)pseudoThis->gc_window[7], ulc.PosX(), ulc.PosY(), 
			 (xev.xbutton.x - ulc.PosX()), (xev.xbutton.y - ulc.PosY()));
	  pseudoThis->flush();

	}else{
	  ulc.reposition(-1, -1);
	  mouseDown = FALSE;
	}
      }
    }
  }
};


bool isHueCheck(const void * voidPixelPointer){

  return ((*(u_short *)voidPixelPointer) > (preferredHue - 1) &&
	  (*(u_short *)voidPixelPointer) < (preferredHue + 1));
};

int main (int argc, char **argv) {

  int framenum=0, next_fr=0, n_buffers, i, j, rate=100,
    height=480, width=640, xloc=width/2, yloc=height/2, output=1, depth;

  XVImageRGB<XV_RGB> image (640, 480), target(width, height);
  XVImageScalar<float> im (640, 480), targ(width, height);

  if (!grabber) {
    grabber = new Bt8x8<XV_RGB>();
    grabber -> set_params ("B4I1N0");
  }
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  n_buffers=grabber->buffer_count();

  if(!XInitThreads()){ cerr << "X isn't MT-safe" << endl; exit(1); }

  //Set up the window
  window = new SelectWindow(&target, 0, 0);
  window -> setImages(&target, 1);
  window -> map();
  
  //Acquire image
  grabber -> initiate_acquire(framenum);
  grabber -> wait_for_completion(framenum);
  image = (grabber -> frame(framenum));
  
  //Swap and flush
  framenum = (framenum+1)%n_buffers;
  window -> CopySubImage(&image);
  window -> swap_buffers;
  window -> flush();
  
  // Now that the first image is displayed, we can grab the
  // pointer and start the selectRegion thread
  window->grabPointer();
  pthread_t ptData;

  int thread_id = pthread_create( &ptData, NULL, 
				  (&(SelectWindow::selectRegion)), window);

  // setup the wait for thread stuff
  struct timeval  now;
  gettimeofday(&now, NULL);

  delay.tv_sec = now.tv_sec + 1;
  delay.tv_nsec = now.tv_usec * 1000;

  bool done = 0;
  while(!done) {
    
    //Acquire image
    grabber -> initiate_acquire(framenum);
    grabber -> wait_for_completion(framenum);
    image = (grabber -> frame(framenum));
    
    //Swap and flush
    framenum = (framenum+1)%n_buffers;
    window -> CopySubImage(&image);
    window -> swap_buffers;
    window -> flush();

    if(selected){ done = 1; }
  }
  
  window->unGrabPointer();

  cout << endl << "ULC: " << region->PosX() << "," << region->PosY() << endl;
  cout << "W: " << region->Width() << ", H: " << region->Height() << endl;

  // find best hue of image and start tracking

  // crop image at region
  float bestHue = findBestHue(subimage(image, *region));

  if(bestHue < 2)
    preferredHue = 2;
  else if(bestHue > 358)
    preferredHue = 358;
  else
    preferredHue = (u_short)round(bestHue);
  cout << "HUE: " << preferredHue << endl;
  
  XVHueSeg<XV_RGB> hueSegmenter;

  XVImageScalar<u_short> segmentedImage;
  vector<XVImageGeneric> regions;

  while(1){

    grabber -> initiate_acquire(framenum);
    grabber -> wait_for_completion(framenum);
    image = (grabber -> frame(framenum));
    
    framenum = (framenum+1)%n_buffers;
    window -> CopySubImage(&image);
    window -> swap_buffers;
    window -> flush();

    hueSegmenter.segment(image, segmentedImage);

    hueSegmenter.regionGrow(segmentedImage, regions, & isHueCheck, 50, 2000, 2);

    int ulX, ulY, lrX, lrY;
    if(regions.size() > 0){
      ulX = regions[0].PosX();
      ulY = regions[0].PosY();
      lrX = regions[0].PosX() + regions[0].Width();
      lrY = regions[0].PosY() + regions[0].Height();
    }

    for(int i = 0; i < regions.size(); i++){
      window->drawRectangle(regions[i]);
    }
    window->flush();
  }
};
