// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***
// 
//  known parameters     rxxx  xxx-needs to be integer
//                        xxx/10 is the framerate
#ifndef _XVPv_H_
#define _XVPv_H_


#include <exception>
#include <sys/types.h>
#define _LINUX
#define _x86
#include <PvApi.h>
#include "config.h"
#include "XVImageRGB.h"
#include <XVVideo.h>

#define XVPV_MAX_CAMERAS        6
#define XVPV_NUM_FRAMES		4

#define FRAMESCOUNT 15

// camera's data
typedef struct
{
    unsigned long   UID;
    tPvHandle       Handle;
    tPvFrame        *pv_buffers;
}tCamera;

template <class IMTYPE>
class XVPv : public XVVideo<IMTYPE> {
 protected:
  using XVVideo<IMTYPE>::size ;
  using XVVideo<IMTYPE>::image_buffers ;
  using XVVideo<IMTYPE>::n_buffers ;
  using XVVideo<IMTYPE>::current_buffer ;
  using XVVideo<IMTYPE>::init_map ;
  using XVVideo<IMTYPE>::parse_param ;

   const char* TypeToString(tPvDatatype aType);
   tPvErr AttrList(tPvHandle Camera);
   tPvErr AttrType(tPvHandle Camera,const char* Label);
   tPvErr AttrRange(tPvHandle Camera,const char* Label);
   tPvErr AttrRead(tPvHandle Camera,const char* Label);
   tPvErr AttrWrite(tPvHandle Camera,const char* Label,const char* Value);
 private:
   tCamera		 CameraStruct;
   int			 buffer_index;
   unsigned long	 frame_size;
   bool			 verbose;
  public:
   using XVVideo<IMTYPE>::frame ;
   		XVPv(unsigned long u_id=0, char *param=NULL,
		     bool verbose=true);
   virtual	~XVPv();
   // Video_h compatibility functions
   int          open(const char *dev_name);
   void		close(void);
   IMTYPE 		& current_frame_continuous();

   virtual int		initiate_acquire(int frame);
   virtual int		wait_for_completion(int frame);
   virtual int    set_params(char*);
   int         get_buffer_index(){return buffer_index;}


};

#endif
