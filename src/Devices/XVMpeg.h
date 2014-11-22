// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVMPEG_H_
#define _XVMPEG_H_

#define DEF_MPEG_NUM   4

#include <stdio.h>

#include <XVVideo.h>

template <class IMTYPE>
class XVMpeg : public XVVideo<IMTYPE> {
protected:
  using XVVideo<IMTYPE>::size ;
  using XVVideo<IMTYPE>::init_map ;
public:
  using XVVideo<IMTYPE>::frame ;

private:
  FILE *fd;

public:

  XVMpeg (const char *fname = "mpeg_file.mpg",
	  const char *parm_string=NULL);

  virtual int open(const char * filename);
  void close();
  virtual int  initiate_acquire(int buffernum);
  virtual int  wait_for_completion(int buffernum);

  int set_params(char *params) {return 1;};
};

#endif
