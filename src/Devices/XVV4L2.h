// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVV4L2_H_
#define _XVV4L2_H_

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

#include <sys/types.h>
#include <asm/types.h>
//#include <linux/fs.h>
#include <linux/kernel.h>
//#include <linux/videodev.h>
//#define __user
#include <linux/videodev2.h>
#include <XVVideo.h>

#define DEVICE_NAME	"/dev/video0"

#ifndef XV_MAX_INPUT_V4L2
#define XV_MAX_INPUT_V4L2  4
#endif

#define V4L2_DEF_NUMFRAMES 8
#define V4L2_DEF_INPUT     0
#define V4L2_DEF_NORM      NORM_NTSC

#ifndef XV_VIDEO_ENUM_COMPOSITE
#define XV_VIDEO_ENUM_COMPOSITE
enum{Composite1=1,SVIDEO};

struct STRTAB {
    long nr;
    char *str;
};
#endif

template <class IMTYPE>
class XVV4L2 : public XVVideo<IMTYPE> {
  protected:
   using XVVideo<IMTYPE>::size ;
   using XVVideo<IMTYPE>::n_buffers ;
   using XVVideo<IMTYPE>::parse_param ;

  private:
   bool		raw_mode;
   int		fd;
   struct 	v4l2_input        inp[XV_MAX_INPUT_V4L2];
   struct       v4l2_buffer       vidbuf[V4L2_DEF_NUMFRAMES];
   struct       v4l2_capability   capability;
   v4l2_std_id 	  		  norm;
   struct 	v4l2_format 	  fmt;
   struct 	v4l2_requestbuffers req;


   int		ninputs;
   typename     IMTYPE::PIXELTYPE *mm_buf[V4L2_DEF_NUMFRAMES];
   int		set_format;
   int		set_input(int channel);
   int		set_params(char *param_string);
  public:
   		XVV4L2(char const *dev_name=DEVICE_NAME,
		       char const *parm_string=NULL);
   		XVV4L2(const XVSize &, char const *dev_name=DEVICE_NAME,
		      char const *parm_string=NULL);
   virtual	~XVV4L2();
   // Video_h compatibility functions
   int          open(const char *dev_name,const char *parm_string=NULL);
   void		close(void);
   int		initiate_acquire(int frame);
   int		wait_for_completion(int frame);
   int	get_acquisition_time(int i_frame, struct timeval &time);
   using XVVideo<IMTYPE>::frame ;
};

#endif
