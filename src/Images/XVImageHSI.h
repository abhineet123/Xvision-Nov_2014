#ifndef __xvimagehsi_h
#define __xvimagehsi_h
#include <sys/types.h>
#include <assert.h>
#include <iostream>
#include "List.h"
#include "Tools.h"
#include "XVImageBase.h"
#include "XVImageScalar.h"
#include "XVImageHSI.icc"

using namespace std ;

/** class for HSI images */
template <class T>
class XVImageHSI: public XVImageBase<T> {

public:
  XVImageHSI (int width, int height, XVPixmap<T> *pixmap=NULL,
		char *title=NULL): XVImageBase<T> (width, height, pixmap, title) {} 
  XVImageHSI (XVPixmap<T> *pixmap): XVImageBase<T> (pixmap) {}
  XVImageHSI(): XVImageBase<T>() {}

};
#endif
