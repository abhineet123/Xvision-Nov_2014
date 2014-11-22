// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef __XVIMAGEHSV_H_
#define __XVIMAGEHSV_H_

#include <sys/types.h>
#include <assert.h>

#include <iostream>
#include <XVList.h>
#include <XVTools.h>
#include <XVImageBase.h>
#include <XVImageScalar.h>
#include <XVColorImage.h>

using namespace std ;

/** class for HSV images */
template <class T>
class XVImageHSV : virtual public XVColorBase<T> {
  
 public:

  XVImageHSV(int width, int height, XVPixmap<T> * pixmap = NULL) :
    XVColorBase<T>(width, height, pixmap) {}
  XVImageHSV(XVSize s, XVPixmap<T> * p ) :
    XVColorBase<T>(s.Width(), s.Height(), p) {}

  XVImageHSV(XVPixmap<T> * pixmap) : XVColorBase<T>(pixmap) {}
  XVImageHSV() : XVColorBase<T>() {}
};

#endif
