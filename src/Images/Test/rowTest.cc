#include <config.h>
#include <XVMacros.h>
#include <XVImageRGB.h>
#include <XVImageIterator.h>
#include <XVImageIO.h>
#include <XVWindowX.h>
#include <iostream>


int main(int argc, char * argv[]){

  cout << "sanity check" << endl;

  if(argc != 4){
    cout << "use: rowTest <image> <begin row> <end row>" << endl;  
    exit(0);
  }

  XVImageRGB<XV_RGB> im;
  XVReadImage(im,argv[1]);
  
  XVWindowX<XV_RGB> win(im, 10, 10);
  win.map();

  win.CopySubImage(im);
  win.swap_buffers();
  win.flush();

  im.setSubImage(10, 10, 50, 50);

  XVRowWIterator<XV_RGB> riter(im, atoi(argv[2]));
  
  XV_RGB z;  z.r = 255; z.g = 0; z.b = 0;

  int r = atoi(argv[2]);

  for(; r < im.Height(); ++r){

    riter.reset(r);

    for(; !riter.end(); ++riter){

      *riter = z;
    }
  }

  im.setToPixmap();

  win.CopySubImage(im);
  win.swap_buffers();
  win.flush();

  sleep(4);
};

