#include <XVImageRGB.h>
#include <XVImageIO.h>
#include <XVImageIterator.h>
#include <config.h>
#include <XVWindowX.h>
#include <XVMpeg.h>

int main(int argc, char * argv[]){
  
  //  if(!XInitThreads()) { cout << "X not MT-safe!" << endl; exit(1); } 

  XVImageRGB<XV_RGB> wIM(0, 0);
  
  XVMpeg<XVImageRGB<XV_RGB> > vid(argv[1]);

  XVThreadedWindowX<XV_RGB> window(vid.current_frame(), 0, 0);
  window.map();

  XVImageGeneric g(100, 100, 10, 10);
  int i = 0;

  int h = window.drawRectangle(10, 10, 50, 50);

  while(++i < 30){

    window.CopySubImage(vid.next_frame_continuous());
    window.swap_buffers();
    window.flush();
  }

  XVImageRGB<XV_RGB> winIM = window.getDisplayedImage(20, 20, 50, 50);
  XVWritePPM(winIM, "tmpIM.ppm");

  XVInteractWindowX<XV_RGB> winInter(vid.current_frame(), 500, 0);
  winInter.map();
  winInter.CopySubImage(vid.current_frame());
  winInter.swap_buffers();
  winInter.flush();

  XVPosition p1, p2;
  XV_RGB blackPix = {255, 255, 255};
  winInter.selectLine(p1, p2, blackPix);
  XVImageGeneric reg;
  winInter.selectRectangle(reg);
  winInter.selectEllipse(reg);
  winInter.selectSizedRect(reg, XVSize(20, 20));

  window.erase(h);

  XVPosition pos1, pos2;
  XVImageGeneric region;
  window.selectRectangle(region);

  while(1){

    window.CopySubImage(vid.next_frame_continuous());
    window.swap_buffers();
    window.flush();

    if(window.selected()){
      cout << region.PosX() << " : " << region.PosY() << " : "
	   << region.Width() << " : " << region.Height() << endl;
      window.selectSizedRect(region, XVSize(30, 30));
    }
  }
};


