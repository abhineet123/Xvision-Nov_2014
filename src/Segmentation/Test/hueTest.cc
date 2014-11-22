#include <config.h>
#include <XVColorSeg.h>
#include <XVImageRGB.h>
#include <XVWindowX.h>
#include <XVImageIO.h>

#include <unistd.h>

static u_char preferredHue = 0;
static u_char range = 2;

bool ISPREF(u_short val) {
  return abs(val - preferredHue) < (range / 2);
};

bool ISDARK(u_short val) {
  return val & NULL_DARK;
};

bool ISBRIGHT(u_short val) {
  return val & NULL_BRIGHT;
};

int main(int argc, char * argv[]){

  if(argc < 4){ cout << "try: hueTest <image file> <num of sectors> <range> [darkpix (0-255)] [lightpix (0-255)]\n" << endl; exit(1); }

  range = atoi(argv[3]);

  XVImageRGB<XV_RGB> img;
  XVReadImage(img, argv[1]);

  XVInteractWindowX<XV_RGB> win(img, 100, 100);
  win.map();
  win.CopySubImage(img);
  win.swap_buffers();
  win.flush();

  XVHueSeg<XV_RGB, u_short> * segmenter;

  if(argc == 5){ 
    segmenter = new XVHueSeg<XV_RGB, u_short>(atoi(argv[2]), atoi(argv[4]), DEFAULT_BRIGHT_PIXEL);
  }else if(argc == 6){
    segmenter = new XVHueSeg<XV_RGB, u_short>(atoi(argv[2]), atoi(argv[4]), atoi(argv[5]));
  }else{
    segmenter = new XVHueSeg<XV_RGB, u_short>(atoi(argv[2]));
  }

  XVImageGeneric rect;
  win.selectRectangle(rect, "red");

  XVImageRGB<XV_RGB> tmpIM = subimage(img, rect);
  preferredHue = (u_short)segmenter->findBestHue(tmpIM);

  cout << "Hue is: " << (int)preferredHue << endl;
  XVImageScalar<u_short> scIM(img.Width(), img.Height());

  segmenter->segment(img, scIM);

  XVRectangleBlob blob;
  
  segmenter->findBoundingBox(img, blob, &ISDARK);

  cout << blob.PosX() << " : " << blob.PosY() << " : " << blob.Width() << " : " << blob.Height() << endl;

  win.setCOPY();
  win.XVDrawable::drawRectangle(blob, "red");
  win.swap_buffers();
  win.flush();

  sleep(2);
};
