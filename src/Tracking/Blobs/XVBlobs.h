// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVBLOBS_H_
#define _XVBLOBS_H_

#include <XVImageBase.h>

class XVBlob{

 public:

  XVBlob(){}
  
  virtual bool contains(const XVPosition &) = 0;
  virtual bool within(const XVBlob &) = 0;

};

class XVRectangleBlob : public XVImageGeneric, public XVBlob {

 public:

  using XVImageGeneric::posx;
  using XVImageGeneric::posy;
  using XVImageGeneric::width;
  using XVImageGeneric::height;

  XVRectangleBlob(const XVImageGeneric & img) : XVImageGeneric(img), XVBlob() {}

  XVRectangleBlob(const XVSize &xsize, const XVPosition &xvp)
    : XVImageGeneric(xsize, xvp), XVBlob() {}
   
  XVRectangleBlob (const XVSize &xsize)
    : XVImageGeneric(xsize, XVPosition(0,0)), XVBlob() {}
  
  XVRectangleBlob(const int cw, const int rh, const int px = 0, const int py = 0)
    : XVImageGeneric(cw,rh, px,py), XVBlob() {}
  
  XVRectangleBlob() : XVImageGeneric(), XVBlob() {}
  
  XVRectangleBlob(const XVRectangleBlob &xvg)
    : XVImageGeneric(xvg), XVBlob() {}

  inline bool contains(const XVPosition & pos){ 
    return XVImageGeneric::contains(pos);
  };

  inline bool within(const XVRectangleBlob & box){
    return XVImageGeneric::within(box);
  };

  inline bool within(const XVBlob & box){ return false; }
};

#endif
