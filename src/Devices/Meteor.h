#ifndef __xvmeteor_h
#define __xvmeteor_h
/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <ioctl_meteor.h>
#include "XVVideo.h"

#define METEOR_DEVICE_NAME	"/dev/meteor0"
#define METEOR_DEF_NUMFRAMES 1
#define METEOR_DEF_NORM      METEOR_FMT_NTSC
#define METEOR_DEF_INPUT     METEOR_INPUT_DEV0

template <class IMTYPE>
class Meteor:public XVVideo<IMTYPE>
{
  protected:
   using XVVideo<IMTYPE>::size ;
   using XVVideo<IMTYPE>::n_buffers ;
   using XVVideo<IMTYPE>::parse_param ;
  public:
   using XVVideo<IMTYPE>::frame ;

  private:
   int		fd;
   struct 	meteor_frame_offset off;
   struct meteor_geomet geo;
   typename IMTYPE::PIXELTYPE       	  *mm_buf[METEOR_DEF_NUMFRAMES];
   int		set_input(int norm,int channel);
  public:
   		Meteor(const char *dev_name=METEOR_DEVICE_NAME,
		      const char *parm_string=NULL);
   virtual	~Meteor();
   // Video_h compatibility functions
   int          open(const char *dev_name,const char *parm_string=NULL);
   void		close(void);
   int		set_params(char *param_string);
   int		initiate_acquire(int frame);
   int		wait_for_completion(int frame);
};

#endif
