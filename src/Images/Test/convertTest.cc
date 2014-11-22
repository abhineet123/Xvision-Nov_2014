#include <iostream>
#include <unistd.h>

#include <config.h>
#include <XVImageRGB.h>
#include <XVImageYUV.h>
#include <XVImageIO.h>

#include <XVWindowX.h>

XVImageYUV<XV_YUV24> toYUV(const XVImageYUV<XV_YUV24> & im) {
  return im;
};

XVImageRGB<XV_RGB> toRGB(const XVImageRGB<XV_RGB> & im) {
  return im;
};

XVImageRGB<XV_RGB15> toRGB15(const XVImageRGB<XV_RGB> & im) {
  return im;
};


int main(int argc, char * argv[]){

  if(argc < 2){ cout << "use: convertTest <rgb image>" << endl; exit(1); }
  
  XVImageRGB<XV_RGB> rgbIM;

  XVReadImage(rgbIM, argv[1]);

  XVImageRGB<XV_RGB15> rgb15 = toRGB15(rgbIM);

  XVImageYUV<XV_YUV24> yuvIM = toYUV(rgb15);

  XVImageRGB<XV_RGB> newIM = toRGB(yuvIM);

  XVWindowX<XV_RGB> win(newIM, 10, 10);
  win.map();
  
  win.CopySubImage(newIM);
  win.swap_buffers();
  win.flush();

  sleep(10);
}
