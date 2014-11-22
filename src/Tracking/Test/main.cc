#include <config.h>
#include "PipeModules.h"
#include <XVImageScalar.h>
#include <XVImageRGB.h>
#include "PipeImage.h"
#include <XVImageIterator.h>

#include <XVWindow.h>
#include "../../Devices/Mpeg.h"
#include "SSD.h"

template <class IN, class OUT>
XVImageScalar<OUT> testRGB (const XVImageRGB<IN> &image,XVImageScalar<OUT> &targ) {
  return RGBtoScalar(image,targ);
}


main()
{

  MPEG<XV_RGBA32> src("mpeg_file.mpg");
  PipeVideoSource<XV_RGBA32> srcP(src);

  PipeModuleLiftImplicit1< XVImageScalar<int>, XVImageRGB<XV_RGBA32> > CtoBW(testRGB);
  Pipe<XVImageScalar<int> > IsrcP = CtoBW <<= srcP();
  //  Pipe<XVImageScalar<int> > IsrcP = liftP(testRGB) << srcP();

  XVImageScalar<int> temp = IsrcP->next_value();
  temp = IsrcP->next_value();
  temp = IsrcP->next_value();

  XVPosition p(temp.Width()/2+1,temp.Height()/2+1);
  SSD<XVImageScalar<int>,XVPosition> s(subimage(temp,XVImageGeneric(XVSize(40,40),p)),p,IsrcP);


  for (int i = 1;i<20;i++) {
    cout << i << " " << (s.update()) << endl;
  }

}

