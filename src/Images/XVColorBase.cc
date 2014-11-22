// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***
#include <config.h>
#include <XVColorImage.h>
#include <XVPixel.h>
#include <XVImageRGB.h>
#include <XVImageYUV.h>
#include <XVImageHSV.h>
#include <XVImageScalar.h>
#include <XVImageYUV422.h>
#include <XVImageYCbCr.h>

#ifdef HAVE_IPP 
#include <ippi.h>
#define CONVERT_OP_IMPL(IM_TYPE) \
template <class T> \
XVColorBase<T>::operator IM_TYPE() const { \
\
 IM_TYPE newIM(this->Width(),this->Height());\
 if(this->ImageType()==XVImage_RGB24 && newIM.ImageType()==XVImage_RGB32)\
 {\
  IppiSize roi={newIM.Width(),newIM.Height()};\
   const int dstOrder[4]={0,1,2,3};\
    ippiSwapChannels_8u_C3C4R((const Ipp8u*)this->data(),\
                               this->SizeX()*sizeof(XV_RGB24),\
                               (Ipp8u*)newIM.lock(),\
			       newIM.SizeX()*sizeof(XV_RGBA32),\
			                                  roi,dstOrder,0);\
    newIM.unlock();\
 }\
 else\
 {\
  if(newIM.ImageType() == this->ImageType()){ \
    newIM = dynamic_cast<IM_TYPE &>(const_cast<XVColorBase<T> & >(*this)); \
    return newIM; \
  } \
  newIM.resize(this->Width(), this->Height()); \
  XVImageIterator<T> fromIter(*this); \
  XVImageWIterator<IM_TYPE::PIXELTYPE>  toIter(newIM); \
\
  for(; !toIter.end(); ++toIter, ++fromIter){ \
    *toIter << *fromIter; \
  } \
 }\
 return newIM; \
};
#else
#define CONVERT_OP_IMPL(IM_TYPE) \
template <class T> \
XVColorBase<T>::operator IM_TYPE() const { \
  IM_TYPE newIM(0,0); \
  if(newIM.ImageType() == this->ImageType()){ \
    newIM = dynamic_cast<IM_TYPE &>(const_cast<XVColorBase<T> & >(*this)); \
    return newIM; \
  } \
  newIM.resize(this->Width(), this->Height()); \
  XVImageIterator<T> fromIter(*this); \
  XVImageWIterator<IM_TYPE::PIXELTYPE>  toIter(newIM); \
\
  for(; !toIter.end(); ++toIter, ++fromIter){ \
    *toIter << *fromIter; \
  } \
  return newIM; \
};
#endif

CONVERT_OP_IMPL(XVImageRGB<XV_RGB15>);  
CONVERT_OP_IMPL(XVImageRGB<XV_RGB16>);
CONVERT_OP_IMPL(XVImageRGB<XV_RGB24>);  
CONVERT_OP_IMPL(XVImageRGB<XV_TRGB24>);  
CONVERT_OP_IMPL(XVImageRGB<XV_RGBA32>);
CONVERT_OP_IMPL(XVImageRGB<XV_GLRGBA32>);
CONVERT_OP_IMPL(XVImageYUV<XV_YUV24>);  
CONVERT_OP_IMPL(XVImageHSV<XV_HSV24>);

CONVERT_OP_IMPL(XVImageScalar<u_char>);
CONVERT_OP_IMPL(XVImageScalar<char>);
CONVERT_OP_IMPL(XVImageScalar<u_short>);
CONVERT_OP_IMPL(XVImageScalar<short>);
CONVERT_OP_IMPL(XVImageScalar<u_int>);
CONVERT_OP_IMPL(XVImageScalar<int>);
CONVERT_OP_IMPL(XVImageScalar<float>);
CONVERT_OP_IMPL(XVImageScalar<double>);


template <class T>
XVColorBase<T>::operator XVImageYUV422() const {

  XVImageYUV422 newIM(0, 0);
  if(newIM.ImageType() == this->ImageType()){
    newIM = dynamic_cast<XVImageYUV422 & >(const_cast<XVColorBase<T> & >(*this)); 
    return newIM;
  }
  XVSize oldSize = (XVSize)(*this);
  (const_cast<XVColorBase<T> *>(this))->resize(2 * ((int)(this->Width() / 2)), this->Height());
  newIM.resize((int)(this->Width() / 2), this->Height());
  XVImageIterator<T> fromIter(*this);
  XVImageWIterator<XV_YUV422> toIter(newIM);
  T doublePix[2];
  for(; !toIter.end(); ++toIter, ++fromIter){
    doublePix[0] = *fromIter;
    ++fromIter;
    doublePix[1] = *fromIter;
    *toIter << doublePix;
  }
  (const_cast<XVColorBase<T> *>(this))->resize(oldSize);
  return newIM;
};
  
template <class T>
XVColorBase<T>::operator XVImageYCbCr() const {

  XVImageYCbCr newIM(0, 0);
  if(newIM.ImageType() == this->ImageType()){
    newIM = dynamic_cast<XVImageYCbCr & >(const_cast<XVColorBase<T> & >(*this)); 
    return newIM;
  }
  XVSize oldSize = (XVSize)(*this);
  (const_cast<XVColorBase<T> *>(this))->resize(2 * ((int)(this->Width() / 2)), this->Height());
  newIM.resize((int)(this->Width() / 2), this->Height());
  XVImageIterator<T> fromIter(*this);
  XVImageWIterator<XV_YCbCr> toIter(newIM);
  T doublePix[2];
  for(; !toIter.end(); ++toIter, ++fromIter){
    doublePix[0] = *fromIter;
    ++fromIter;
    doublePix[1] = *fromIter;
    *toIter << doublePix;
  }
  (const_cast<XVColorBase<T> *>(this))->resize(oldSize);
  return newIM;
};
  
template class XVColorBase<XV_RGB15>;
template class XVColorBase<XV_RGB16>;
template class XVColorBase<XV_RGB24>;
template class XVColorBase<XV_TRGB24>;
template class XVColorBase<XV_RGBA32>;
template class XVColorBase<XV_GLRGBA32>;
template class XVColorBase<XV_YUV24>;
template class XVColorBase<XV_HSV24>;
