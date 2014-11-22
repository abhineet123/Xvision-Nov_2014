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

#include "config.h"
#include "PongWindow.h"

template PongWindow<u_short>;

template <class T>
void PongWindow<T>::DrawBall(int x,int y,int size)
{
#ifdef HAVE_LIBXDPMS
   Drawable win=(back_flag)? back_buffer : window; 
#else
   Drawable win=window;
#endif
  if(flip)
   XFillArc(dpy,win,(GC)gc_window[7],width-x,y,size,size,0,64*360);
  else
   XFillArc(dpy,win,(GC)gc_window[7],x,y,size,size,0,64*360);
}

template <class T>
void PongWindow<T>::ShowReg(Blob * &region,int count)
{
   int i;
#ifdef HAVE_LIBXDPMS
   Drawable win=(back_flag)? back_buffer : window; 
#else
   Drawable win=window;
#endif

   if(flip)
    for(i=0;i<count;i++)
    {
     if(region[i].valid>0)
     {
       XDrawRectangle(dpy,win,(GC)gc_window[0],
                width-region[i].ex,region[i].sy,
       		region[i].ex-region[i].sx,
       		region[i].ey-region[i].sy);
     }
    }
   else
    for(i=0;i<count;i++)
    {
     if(region[i].valid>0)
     {
       XDrawRectangle(dpy,win,(GC)gc_window[0],
                region[i].sx,region[i].sy,
       		region[i].ex-region[i].sx,
       		region[i].ey-region[i].sy);
     }
    }
}

template <class T>
PongWindow<T>::PongWindow(XVImageBase *image,int posx,int posy,
             int event_mask,
             char *display,int num_buf):
	    XVWindow<T>(image,posx,posy,event_mask,display,num_buf)
{
}
