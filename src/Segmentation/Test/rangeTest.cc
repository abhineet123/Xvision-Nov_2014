#include <config.h>
#include <XVColorSeg.h>
#include <XVImageRGB.h>
#include <XVWindowX.h>
#include <XVImageIO.h>

#include <unistd.h>

int main(int argc, char * argv[]){

  if(argc < 3){ cout << "try: rangeTest <image file> <range> [darkpix (0-255)] [lightpix (0-255)]\n" << endl; exit(1); }

  XVImageRGB<XV_RGB> img;
  XVReadImage(img, argv[1]);

  XVInteractWindowX<XV_RGB> win(img, 100, 100);
  win.map();
  win.CopySubImage(img);
  win.swap_buffers();
  win.flush();

  XVHueRangeSeg<XV_RGB, u_short> * segmenter;

  XVImageGeneric rect;
  win.selectRectangle(rect, "red");

  XVImageRGB<XV_RGB> tmpIM = subimage(img, rect);

  if(argc == 4){ 
    segmenter = new XVHueRangeSeg<XV_RGB, u_short>(tmpIM, DEFAULT_HUERANGE_WIDTH, DEFAULT_SECTOR_NUM, 
						   atoi(argv[3]), DEFAULT_BRIGHT_PIXEL);
  }else if(argc == 5){
    segmenter = new XVHueRangeSeg<XV_RGB, u_short>(tmpIM, DEFAULT_HUERANGE_WIDTH, DEFAULT_SECTOR_NUM, 
						   atoi(argv[3]), atoi(argv[4]));
  }else{
    segmenter = new XVHueRangeSeg<XV_RGB, u_short>(tmpIM);
  }

  XVRectangleBlob blob;
  
  segmenter->findBoundingBox(img, blob);

  cout << blob.PosX() << " : " << blob.PosY() << " : " << blob.Width() << " : " << blob.Height() << endl;

  win.setCOPY();
  win.XVDrawable::drawRectangle(blob, "red");
  win.swap_buffers();
  win.flush();
  sleep(2);
};
