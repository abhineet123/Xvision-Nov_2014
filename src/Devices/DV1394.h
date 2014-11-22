#ifndef __xvdv1394_h
#define __xvdv1394_h
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

Support for DV avi formats 
It is based on dvgrab from Arnie Schirmacher
*/

#include <sys/types.h>
#include <linux/videodev.h>
#include "DVFrame.h"
#include "XVVideo.h"
extern "C"
{
#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>
#include <libdv/bitstream.h>
#include <libdv/dct.h>
#include <libdv/idct_248.h>
#include <libdv/quant.h>
#include <libdv/weighting.h>
#include <libdv/vlc.h>
#include <libdv/parse.h>
#include <libdv/place.h>
#include <libdv/ycrcb_to_rgb32.h>
}


#define DV_DEVICE_NAME	"/dev/raw1394"
#define DV_DEF_NUMFRAMES 4
#define DV_DEF_INPUT     0
#define DV_PORT_SIZE     16

template <class PIXTYPE>
class DV1394:public XVVideo<PIXTYPE>
{
protected:
  using XVVideo<PIXTYPE>::image_buffers ;
  using XVVideo<PIXTYPE>::parse_param ;
  using XVVideo<PIXTYPE>::size ;

   struct raw1394_portinfo	portinfo[DV_PORT_SIZE];
   int				card;
   int				numcards;
   int				channel;
   DVFrame			*frame_ring[DV_DEF_NUMFRAMES];
   void				decode_frame(DVFrame *frame,
       					     int dest_image_index);
  public:
   		DV1394(const char *dev_name=DV_DEVICE_NAME,
		      const char *parm_string=NULL);
   virtual	~DV1394();
   // Video_h compatibility functions
   int          open(const char *dev_name);
   void		close(void);
   int		set_params(char *param_string);
   int		initiate_acquire(int frame);
   int		wait_for_completion(int frame);
};

#endif
