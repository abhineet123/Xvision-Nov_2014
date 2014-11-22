// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGESEQ_H_
#define _XVIMAGESEQ_H_

#include <XVVideo.h>
#include <stdlib.h>
#include <XVImageRGB.h>
#include <XVImageIO.h>
#include <cstring>

template <class IMTYPE>
class XVImageSeq : public XVVideo<IMTYPE> {
protected:
  using XVVideo<IMTYPE>::image_buffers ;
private:
  XVImageSeq( const XVImageSeq& x ) {} // prevent copy-construction
  XVImageSeq& operator = ( const XVImageSeq& x ) {} // prevent assignment

protected:
  
  char * format ;
  char * filename ;
  int initIndex;
  int imageIndex;
  int stopIndex;

  bool goodBit ;

 public:

  XVImageSeq(const char *file, int initFrame = 0, int lastFrame = -1,int nbufs = 4);
  XVImageSeq(char * fp, char * fs, int initFrame = 0, int lastFrame = -1,int nbufs = 4);
  XVImageSeq(const char *file, int w, int h,char* foo,int initFrame = 0, int lastFrame = -1,int nbufs = 4);
  ~XVImageSeq() { delete[] format ; delete[] filename ; }

  virtual int  initiate_acquire(int buffernum);
  virtual int  wait_for_completion(int buffernum){ return 0; };

  int current_frame_index(){return imageIndex;};
  int seek(int seek_to_frame);
  int seek_to_begin() {return seek(initIndex);};
  int seek_to_end() {return seek(stopIndex);};
  int seek_to_prev() {return seek(imageIndex-1);};
  int set_params(char * ps){ return 0; };
  int stop_frame_index(){return stopIndex;};

  bool good() { return goodBit ; }
  const char * next_image_name() const ; // last loaded image name
};

#endif
