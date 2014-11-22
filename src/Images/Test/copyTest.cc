#include <config.h>
#include <XVWindowX.h>
#include <XVImageRGB.h>
#include <XVImageScalar.h>
#include <XVImageIO.h>
int main(int argc, char * argv[]){

  cout << "before imgRGB inst" << endl;
  XVImageRGB<XV_RGB> imgRGB;
  XVReadImage(imgRGB, argv[1]);
//    imgRGB.setSubImage(100, 100, 150, 150);

  XVImageRGB<XV_RGB> rgbIM;
  float fx, fy;
  sscanf(argv[2], "%f", &fx);
  sscanf(argv[3], "%f", &fy);
  cout << fx << " : " << fy << endl;
  resample(fx, fy, imgRGB, rgbIM);

  XVImageScalar<int> scIM;
  RGBtoScalar(rgbIM, scIM);
  XVImageScalar<int> smIM;
  //  resample(0.5, 0.5, scIM, smIM);
//    smIM = scIM;
    XVImageRGB<XV_RGB> resIM;
//    ScalartoRGB(smIM, resIM);

  resIM = rgbIM;
  XVWindowX<XV_RGB> win(resIM, 10, 10);
  win.map();
  win.CopySubImage(resIM);
  
  win.swap_buffers();
  win.flush();

  sleep(3);

}
