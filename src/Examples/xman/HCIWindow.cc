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
#include "XVBlobs.h"

#define Sqr(a) ((a)*(a))

template class HCIWindow<XV_RGB>;

template <class T>
void HCIWindow<T>::ShowTeach(int x,int y)
{
   Drawable win=(back_flag)? back_buffer : window; 
  if(flip)
   XDrawRectangle(dpy,win,gc_window[7],width-x-112,y,112,
   	112);
  else
   XDrawRectangle(dpy,win,gc_window[7],x,y,112,112);
}

template <class T>
void HCIWindow<T>::ShowReg(vector<XVRectangleBlob> &regions,
    		int x,int y,
		float angle)
{
   int i;
   int cur_len,length;
   Drawable win=(back_flag)? back_buffer : window; 
   XVRectangleBlob *found_reg;


   length=-1;
   for(vector<XVRectangleBlob>::iterator region=regions.begin();
       		region!=regions.end();region++)
   {
        cur_len=Sqr(region->Width())+
		Sqr(region->Height());
	if(cur_len>length) length=cur_len,found_reg=&*region;
   }
   if(length>Sqr(60))
   {
    if(flip)
    {
     XDrawRectangle(dpy,win,(GC)gc_window[12],
                width-found_reg->PosX()-found_reg->Width()-x,
		found_reg->PosY()+y,
       		found_reg->Width(), found_reg->Height());
     //XDrawLine(dpy,win,gc_window[12],width-sx,sy,width-ex,ey);
    }
    else
    {
     XDrawRectangle(dpy,win,(GC)gc_window[12],
                found_reg->PosX()+x,found_reg->PosY()+y,
       		found_reg->Width(),found_reg->Height());
     //XDrawLine(dpy,win,gc_window[12],sx,sy,ex,ey);
    }
  }
}

template <class T>
void HCIWindow<T>::draw_object(HCIObject *object,int catched)
{
   Drawable draw=(back_flag)? back_buffer : window; 

  switch(object->type)
  {
     case RECT_OBJECT:
      XFillRectangle(dpy,draw,gc_window[object->color+catched],
           width-object->posx-object->size1,
	   object->posy,
	   object->size1,object->size2);
      break;
     case CIRC_OBJECT:
      XFillArc(dpy,draw,gc_window[object->color+catched],
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
HCIWindow<T>::HCIWindow(XVImageRGB<T> &image,int posx,int posy,
             char *title,int event_mask,char *display,int num_buf,
	     int double_buf):
   XVWindowX<T>(image,posx,posy,title,event_mask,display,num_buf,double_buf)
{
}
