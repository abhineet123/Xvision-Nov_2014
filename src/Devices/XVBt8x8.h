// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVBT8X8_H_
#define _XVBT8X8_H_

#include <sys/types.h>
#include <videodev.h>
#include <XVVideo.h>

#define DEVICE_NAME	"/dev/video0"
#define BT_DEF_NUMFRAMES 8
#define BT_DEF_INPUT     0
#define BT_DEF_NORM      NORM_NTSC

#ifndef XV_VIDEO_ENUM_COMPOSITE
#define XV_VIDEO_ENUM_COMPOSITE
enum{Composite1=1,SVIDEO};

struct STRTAB {
    long nr;
    char *str;
};
#endif

template <class IMTYPE>
class XVBt8x8 : public XVVideo<IMTYPE> {
  protected:
   using XVVideo<IMTYPE>::size ;
   using XVVideo<IMTYPE>::n_buffers ;
   using XVVideo<IMTYPE>::parse_param ;

  private:
   int		fd;
   struct 	video_capability  capability;
   struct 	video_channel     *channels;
   struct 	STRTAB 		  *inputs;
   struct 	video_audio       audio;
   struct 	video_tuner       *tuner;
   struct 	video_picture     pict;   
   int		brightness;
   int		contrast;
   int		hue;
   struct 	video_mbuf        gb_buffers;
   typename IMTYPE::PIXELTYPE 	  *mm_buf[BT_DEF_NUMFRAMES];
   struct 	video_mmap 	  *frames;
   int		set_format;
   int		set_input(int norm,int channel);
  public:
   		XVBt8x8(const char *dev_name=DEVICE_NAME,
		      const char *parm_string=NULL);
   		XVBt8x8(const XVSize &, const char *dev_name=DEVICE_NAME,
		      const char *parm_string=NULL);
   virtual	~XVBt8x8();
   // Video_h compatibility functions
   int          open(const char *dev_name,const char *parm_string=NULL);
   void		close(void);
   int		set_params(char *param_string);
   int		initiate_acquire(int frame);
   int		wait_for_completion(int frame);
   using XVVideo<IMTYPE>::frame ;
};

#endif
