#include <config.h>
#include "PipeModules.h"
#include <XVImageScalar.h>
#include <XVImageRGB.h>
#include "PipeImage.h"
#include <XVImageIterator.h>

#include <XVWindow.h>
#include <Mpeg.h>
#include "SSD.h"

main()

{

  MPEG src("mpeg_file.mpg");
  PipeVideoSource<XV_RGBA32> srcP(&src);

  PipeModuleLiftImplicit1< XVImageScalar<int>, XVImageBase<XV_RGBA32> > CtoBW(testRGB);
  Pipe<XVImageScalar<int> > IsrcP = CtoBW << srcP();

  XVImageScalar<int> temp = IsrcP->next_value();
  temp = IsrcP->next_value();
  temp = IsrcP->next_value();

  XVPosition p(temp.Width()/2+1,temp.Height()/2+1);
  SSD<int,XVPosition> s(subimage(temp,g),XVImageGeneric(XVSize(100,100),p),IsrcP);

  for (int i = 1;i<20;i++) {
    cout << i << (s.update()) << endl;
  }

}

