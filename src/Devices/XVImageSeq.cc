#include <stdio.h>

#ifndef JPEG_LIB
#define JPEG_LIB
#endif

#include <config.h>
#include <XVImageIO.h>
#include <XVImageSeq.h>
#include <XVException.h>

template <class IMTYPE>
XVImageSeq<IMTYPE>::XVImageSeq(const char * file,
			       int initFrame ,
			       int lastFrame ,
			       int nbufs)
 : imageIndex(initFrame), stopIndex(lastFrame) ,
   initIndex(initFrame) {

  int len = strlen(file) + 1 ;
  format = new char[len] ;
  strcpy( format, file );
  filename = new char[len+32] ;  // allows %d expanding space
  sprintf(filename, format, imageIndex);
  IMTYPE tmpIM;
  goodBit = 1 == XVReadImage(tmpIM, filename);
  this->init_map(tmpIM, nbufs);
  --imageIndex;
};

template <class IMTYPE>
XVImageSeq<IMTYPE>::XVImageSeq(char * fp, char * fs,
			       int initFrame ,
			       int lastFrame ,
			       int nbufs)
 : imageIndex(initFrame), stopIndex(lastFrame) ,
   initIndex(initFrame) {

  int len = strlen(fp) + strlen(fs) + 4 ; // length of "%d." is 3
  format = new char[len] ;
  sprintf( format, "%s%s%s", fp, "%d.", fs );
  filename = new char[len+32] ;  // allows %d expanding space
  sprintf(filename, format, imageIndex);
  IMTYPE tmpIM;
  goodBit = 1 == XVReadImage(tmpIM, filename);
  this->init_map(tmpIM, nbufs);
  --imageIndex;
};

// this constructor is for the case that the image sequence is
//  binary and its size is not included in the file
template <class IMTYPE>
XVImageSeq<IMTYPE>::XVImageSeq(const char * file, int w, int h,
                               char* foo,  // disambiguite with first constr
			       int initFrame ,
			       int lastFrame ,
			       int nbufs)
 : imageIndex(initFrame), stopIndex(lastFrame) ,
   initIndex(initFrame) {

  int len = strlen(file) + 1 ;
  format = new char[len] ;
  strcpy( format, file );
  filename = new char[len+32] ;  // allows %d expanding space
  sprintf(filename, format, imageIndex);
  IMTYPE tmpIM(w,h);
  goodBit = 1 == XVReadImage(tmpIM, filename);
  this->init_map(tmpIM, nbufs);
  --imageIndex;
};

template <class IMTYPE>
int XVImageSeq<IMTYPE>::initiate_acquire(int buffernum) {

  if(stopIndex != -1 && imageIndex > stopIndex) {
    goodBit = 0 ;
    //throw XVException("End of Image Sequence Reached!");
  }else {
    sprintf(filename, format, ++imageIndex);
    goodBit = 1 == XVReadImage(image_buffers[buffernum], filename);
    return 1;
  }
};


template <class IMTYPE>
int XVImageSeq<IMTYPE>::seek(int seek_to_frame) {
  // seek to desired frame - 1 because initiate_acquire
  if (seek_to_frame < initIndex) {
    imageIndex = initIndex-1;
  }else if (seek_to_frame > stopIndex) {
    imageIndex = stopIndex-1;
  }else {
    imageIndex = seek_to_frame-1;
  }
  return imageIndex+1 ;
}

template <class IMTYPE>
const char * XVImageSeq<IMTYPE>::next_image_name() const {
  return filename ;
}

template class XVImageSeq<XVImageRGB<XV_RGB15> >;
template class XVImageSeq<XVImageRGB<XV_RGB16> >;
template class XVImageSeq<XVImageRGB<XV_RGB24> >;
template class XVImageSeq<XVImageRGB<XV_RGBA32> >;
template class XVImageSeq<XVImageRGB<XV_GLRGBA32> >;

template class XVImageSeq<XVImageScalar<u_char> >;
template class XVImageSeq<XVImageScalar<char> >;
template class XVImageSeq<XVImageScalar<u_short> >;
template class XVImageSeq<XVImageScalar<short> >;
template class XVImageSeq<XVImageScalar<u_int> >;
template class XVImageSeq<XVImageScalar<int> >;
template class XVImageSeq<XVImageScalar<float> >;
template class XVImageSeq<XVImageScalar<double> >;
