
#include <XVImageYUV422.h>
#include <XVImageYCbCr.h>

#define CONVERT_OP_IMPL(IM_TYPE) \
XVImageYUV422::operator IM_TYPE() const { \
\
  typedef IM_TYPE::PIXELTYPE PIXTYPE; \
\
  IM_TYPE newIM; \
  if(newIM.ImageType() == this->ImageType()) newIM = dynamic_cast<IM_TYPE &>(const_cast<XVImageYUV422 & >(*this)); return newIM; \
  newIM.resize(this->Width() * 2, this->Height()); \
  XVImageIterator<XV_YUV422> fromIter(*this); \
  XVImageWIterator<PIXTYPE >  toIter(newIM); \
\
  PIXTYPE pixels[2]; \
  for(; !fromIter.end(); toIter += 2, ++fromIter){ \
\
    pixels << *fromIter; \
    *toIter = pixels[0]; \
    ++toIter; \
    *toIter = pixels[1]; \
  } \
  return newIM; \
};

CONVERT_OP_IMPL(XVImageRGB<XV_RGB15>); 
CONVERT_OP_IMPL(XVImageRGB<XV_RGB16>);
CONVERT_OP_IMPL(XVImageRGB<XV_RGB24>); 
CONVERT_OP_IMPL(XVImageRGB<XV_TRGB24>); 
CONVERT_OP_IMPL(XVImageRGB<XV_RGBA32>);
CONVERT_OP_IMPL(XVImageRGB<XV_GLRGBA32>);
CONVERT_OP_IMPL(XVImageYUV<XV_YUV24>); 

CONVERT_OP_IMPL(XVImageScalar<u_char>);
CONVERT_OP_IMPL(XVImageScalar<char>);
CONVERT_OP_IMPL(XVImageScalar<u_short>);
CONVERT_OP_IMPL(XVImageScalar<short>);
CONVERT_OP_IMPL(XVImageScalar<u_int>);
CONVERT_OP_IMPL(XVImageScalar<int>);
CONVERT_OP_IMPL(XVImageScalar<float>);
CONVERT_OP_IMPL(XVImageScalar<double>);


#define CONVERT_OP_SINGLE_IMPL(IM_TYPE) \
XVImageYUV422::operator IM_TYPE() const { \
\
  typedef IM_TYPE::PIXELTYPE PIXTYPE; \
\
  IM_TYPE newIM; \
  if(newIM.ImageType() == this->ImageType()) newIM = dynamic_cast<IM_TYPE &>(const_cast<XVImageYUV422 & >(*this)); return newIM; \
  newIM.resize(this->Width(), this->Height()); \
  XVImageIterator<XV_YUV422> fromIter(*this); \
  XVImageWIterator<PIXTYPE >  toIter(newIM); \
\
  for(; !fromIter.end(); ++toIter, ++fromIter){ \
\
    *toIter << *fromIter; \
  } \
  return newIM; \
};

#include <XVImageHSV.h>

CONVERT_OP_SINGLE_IMPL(XVImageHSV<XV_HSV24>);
CONVERT_OP_SINGLE_IMPL(XVImageYUV422);
CONVERT_OP_SINGLE_IMPL(XVImageYCbCr);
