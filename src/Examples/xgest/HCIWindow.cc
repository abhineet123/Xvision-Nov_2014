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
#include "HCIWindow.h"

#define Sqr(a) ((a)*(a))

template class HCIWindow<u_char>;
template class HCIWindow<u_short>;
template class HCIWindow<u_long>;

template <class T>
void HCIWindow<T>::ShowTeach(int x,int y)
{
#ifdef HAVE_LIBXDPMS
   Drawable win=(back_flag)? back_buffer : window; 
#else
   Drawable win=window;
#endif
  if(flip)
   XDrawRectangle(dpy,win,gc_window[7],width-x-SCALE(112),y,SCALE(112),
   	SCALE(112));
  else
   XDrawRectangle(dpy,win,gc_window[7],x,y,SCALE(112),SCALE(112));
}

template <class T>
void HCIWindow<T>::ShowReg(Blob * region,int count,int x,int y,
		float angle)
{
   int i;
   int cur_len,length,index;
#ifdef HAVE_LIBXDPMS
   Drawable win=(back_flag)? back_buffer : window; 
#else
   Drawable win=window;
#endif

   for(i=0,index=-1,length=0;i<count;i++)
   {
     if(region[i].valid>0)
     {
        cur_len=Sqr(region[i].ex-region[i].sx)+
		Sqr(region[i].ey-region[i].sy);
	if(cur_len>length) length=cur_len,index=i;
     }
   }
   if(index>-1 && length>Sqr(SCALE(60)))
   {
    int sx,sy,ex,ey;

    length=(int)sqrt(length)/2;
    ex=sx=x+(region[index].ex+region[index].sx)/2;
    ey=sy=y+(region[index].ey+region[index].sy)/2;
    sx+=(int)(0.8*length*sin(angle));
    sy+=(int)(0.8*length*cos(angle));
    ex-=(int)(0.8*length*sin(angle));
    ey-=(int)(0.8*length*cos(angle));
    if(flip)
    {
     XDrawRectangle(dpy,win,(GC)gc_window[0],
                width-region[index].ex-x,region[index].sy+y,
       		region[index].ex-region[index].sx,
       		region[index].ey-region[index].sy);
     XDrawLine(dpy,win,gc_window[0],width-sx,sy,width-ex,ey);
    }
    else
    {
     XDrawRectangle(dpy,win,(GC)gc_window[0],
                region[index].sx+x,region[index].sy+y,
       		region[index].ex-region[index].sx,
       		region[index].ey-region[index].sy);
     XDrawLine(dpy,win,gc_window[0],sx,sy,ex,ey);
    }
  }
  else
    index=-1;
}

template <class T>
void HCIWindow<T>::draw_object(HCIObject *object)
{
#ifdef HAVE_LIBXDPMS
   Drawable draw=(back_flag)? back_buffer : window; 
#else
   Drawable draw=window;
#endif

  switch(object->type)
  {
     case RECT_OBJECT:
      XFillRectangle(dpy,draw,gc_window[object->color],
           width-object->posx-object->size1,
	   object->posy,
	   object->size1,object->size2);
      break;
     case CIRC_OBJECT:
      XFillArc(dpy,draw,gc_window[object->color],
               width-object->posx-object->size1,
	       object->posy,
	       object->size1,
	       object->size2,0,360*64);
      break;
     default:
      break;
  }
  XDrawRectangle(dpy,draw,gc_window[7],
                width-object->posx-3*object->size1/2,
      		object->posy-object->size2/2,
                2*object->size1, 2*object->size2);
}

template <class T>
HCIWindow<T>::HCIWindow(XVImage<T> *image,int posx,int posy,int event_mask,
             char *display,int num_buf,int double_buf):
   XVWindow(image,posx,posy,event_mask,display,num_buf,double_buf)
{
}
