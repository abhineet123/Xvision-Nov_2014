

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "XVWindow.h"
#include "XVImageRGB.h"

int main(int argc,char **argv)
{
  XVWindow<XV_RGB16> *window;
  XVImageRGB<XV_RGB16> image(100,100);
  XVImageIterator<XV_RGB16> iter(image);
  XV_RGB16 i=0;
  for (;iter.end()==false;++iter) *iter = i;
  // Initialization
  window=new XVWindow<XV_RGB16>(&image,40,40,0,NULL,4,1);
  // initialize XVImages for buffers available
  window->setImages(&image,1);
  // map window on screen
  window->map();

  // application part (loops etc.)
  //
  // DO your own stuff
  //
  //

  // copy image into window
  window->CopyImage(0);
  // double buffer? then swap buffers
  window->swap_buffers();
  // refresh display
  window->flush();
  sleep(100000);
  return 0;
}
