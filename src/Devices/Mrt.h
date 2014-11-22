#ifndef __xvmrt_h
#define __xvmrt_h
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

#include "bt819.h"
#include "mrt_cs.h"
#include "Video.h"

#define MRT_DEVICE_NAME	"/dev/mrt0"
#define MRT_MAX_GBUFFERS	5

enum{MRT_Composite1=0,MRT_SVIDEO=BT_COMP};

template <class PIXTYPE>
class Mrt:public Video<PIXTYPE>
{
   int		fd;
   int		grab_format;
   int		next_frame;
   PIXTYPE   	  *mm_buf[MRT_MAX_GBUFFERS];
   PIXTYPE	  *field;
   void		grab_init(void);
   int		set_input(int norm,int channel);
  public:
   		Mrt(const char *dev_name,XVSize size);
		~Mrt();
   int		open(const char *dev_name);
   void		close(void);
   int		set_params(char *paramstring);
   int		grab_queue(int frame);
   int		grab_wait(int frame);
};

#endif
