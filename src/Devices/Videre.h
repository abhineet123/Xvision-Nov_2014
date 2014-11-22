#ifndef __xvvidere_h
#define __xvvidere_h
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
#include "svsclass.h"
#include "XVVideo.h"
#include "XVImageScalar.h"
#include "XVImageRGB.h"

#define DIG_DEF_NUMFRAMES 4
#define DIG_DEF_INPUT     0
#define VIDERE_DEVICE_NAME    "head.ini"

#define XVVID_RIGHT	  0
#define XVVID_LEFT	  1

template <class T>
class XV_Videre:public XVVideo<T>
{
  protected:
   using XVVideo<T>::image_buffers;
   using XVVideo<T>::init_map;
   T image_buffers_right;
   XVImageScalar<u_char>  *scalar_image[2];
   svsVideoImages *sourceObject;
   svsStereoImage   *s_image;
   svsStoredImages       *file_image;
   svsImageParams   *params;
  public:
   		XV_Videre(const char *dev_name=VIDERE_DEVICE_NAME,
		      const char *parm_string=NULL);
		~XV_Videre();
   XVImageScalar<u_char>
   		**get_stereo(void);
   inline T & get_color_right(void);
   inline T & get_color_left(void);
   XVSize	size(void)       {return XVSize(params->width,params->height);};
   int          open(const char *dev_name=VIDERE_DEVICE_NAME);
   void		close(void);
   int		initiate_acquire(int frame);
   int		wait_for_completion(int frame);
   int		set_params(char *param_string=NULL) {return 1;};
   void         set_stoc(bool flag)
                 {sourceObject->SetProcMode(flag?PROC_MODE_DISPARITY:
		                                 PROC_MODE_RECTIFIED);}
   svsVideoImages* get_sourceObject(T mode){return sourceObject;}
};

#include "Videre.icc"
#endif
