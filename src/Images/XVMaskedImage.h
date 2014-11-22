// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVMASKEDIMAGE_H_
#define _XVMASKEDIMAGE_H_

#include <sys/types.h>
#include <iostream>
#include <fstream>

#include <XVList.h>
#include <XVTools.h>
#include <XVImageBase.h>
#include <XVPixel.h>
#include <XVImageScalar.h>

/*
template <class u_char> class XVImageScalar;
template <class IMTYPE> class XVImageRGB;
*/


/** 
 * Defines a masked image class 
 * This class is templated on the actual underlying image while the
 *  8-bit mask is carried along inside this class (inherited).
 * Some operators are overloaded to act only where the mask is != 0
 * Thus, this mask takes the convention that 0 is "off"
 */
template <class IMTYPE>
class XVMaskedImage : public XVImageScalar<u_char> {

protected:
public:

  IMTYPE *realimage; // the entire image
  IMTYPE *maskimage; // vidimage intersected with the mask

public:

  XVMaskedImage(int s_width, int s_height, 
                XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap = NULL);

  XVMaskedImage(const XVSize & s, 
                XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap = NULL);

  XVMaskedImage(XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap);

  XVMaskedImage();

  IMTYPE&  intersect(); /* perform the intersection into maskimage */
  void     reassign_real(IMTYPE *new_real) ;  // reassign the realimg

  
  using XVImageScalar< u_char >::lock;
  using XVImageScalar< u_char >::unlock;
  using XVImageScalar< u_char >::data;
  using XVImageScalar< u_char >::Width;
  using XVImageScalar< u_char >::width;
  using XVImageScalar< u_char >::Height;

};

#endif
