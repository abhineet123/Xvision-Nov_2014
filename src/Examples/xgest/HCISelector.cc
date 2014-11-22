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

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <cmath>

#include "prototypes.h"
#include "HCISelector.h"

using namespace std ;

#define Sqr(a) ((a)*(a))

template class HCISelector<u_char,u_short,double>;

template <class T,class Y,class Z>
XVImage<Z> *HCISelector<T,Y,Z>::segment_distance(XVImage<Y> *image,
                                      XVImage<Z> *output_image=NULL)
{
  Z		      *i_ptr;
  const Y             *pict=(image->data());
  int           x=image->PosX(),y=image->PosY();
  int           i,j;      
      
                  
  if(!output_image)        
      output_image=new XVImage<Z>(image->Width(),image->Height());
  else 
  {    
     if(output_image->Width()!=image->Width() ||
        output_image->Height()!=image->Height())
     {
	delete [] output_image;
        output_image=new XVImage<Z>(image->Width(),image->Height());
     }
  }
  ASSERT(i_ptr=output_image->lock());
  // clear index image
  memset(output_image->lock(),0,
                image->Height()*image->Width()*sizeof(T));

  for(i=0;i<image->Height();i++,pict+=image->Skip())
   for(j=0;j<image->Width();j++) *i_ptr++=360.0-distance[*pict++];
  ASSERT(output_image->unlock());
  return output_image;
}

template <class T,class Y,class Z>
float HCISelector<T,Y,Z>::get_rotation(XVImage<T> *image,
		Blob *region,T preferred)
{
   const T	*image_ptr;
   int		i,row,column;
   int		hsize=region->ex-region->sx,
   	        vsize=region->ey-region->sy;
   int		sum_xy=0,sum_xx=0,sum_yy=0,sum_x=0,sum_y=0,count=0;
   int		cxx,cxy,cyy;
   double	l1;

   for(image_ptr=image->data(),row=0;row<=vsize;row++,
   			image_ptr+=image->Skip())
      for(column=0;column<=hsize;column++,image_ptr++)
      {
        if(*image_ptr==preferred)
	{
	  sum_x+=column;sum_y+=row;sum_xy+=row*column;
	  sum_xx+=column*column;sum_yy+=row*row;
	  count++;
	}
      }
   if(!count) return 0;
   sum_x/=count;
   sum_y/=count;
   sum_xx/=count;
   sum_yy/=count;
   sum_xy/=count;
   cxx=sum_xx-sum_x*sum_x;
   cyy=sum_yy-sum_y*sum_y;
   cxy=sum_xy-sum_y*sum_x;
   l1=(cxx+cyy)/2-sqrt(Sqr((cxx+cyy)/2)+Sqr(cxy)-cxx*cyy);
   return atan2(l1-cxx,cxy);
}

template <class T,class Y,class Z>
int HCISelector<T,Y,Z>::select_gesture(Blob *region,
			int count,int& index, HCIObject& object)
{
   int i;
   int cur_len,length;

   for(i=0,index=-1,length=0;i<count;i++)
   {
     if(region[i].valid>0)
     {
        cur_len=Sqr(region[i].ex-region[i].sx)+
                Sqr(region[i].ey-region[i].sy);
        if(cur_len>length) length=cur_len,index=i;
     }
   }
   // found significant blob?
   if(index>-1 && length>Sqr(SCALE(40)))
   {
     // did it change?
     if(abs(object.length-length)>Sqr(SCALE(90)))
     {
       object.moving=SCALE(25),object.length=length;
       return GESTURE_CHANGED;
     }
     else if (object.moving)
     {
       object.moving --;
       if(!object.moving) return GESTURE_STOPPED;
       return GESTURE_CHANGED;
     }
     return GESTURE_UNCHANGED;
   }
   index=-1;
   return NO_GESTURE;	// no gesture found
}

template <class T,class Y,class Z>
HCISelector<T,Y,Z>::HCISelector(int num_bits,
		int num_sectors,
              int dark_pix,int bright_pix,float preferred):
	      XVColSelector<T,Y>(num_bits,num_sectors,
	      		    dark_pix,bright_pix)
{
   unsigned char r,g,b;
   int mask1,mask2;

   distance=new float[1<<num_bits];
   // create lookup-table
   if(num_bits==16)
   for(r=0;r<(1<<5);r++)
   {
    mask1=r<<11;
    for(g=0;g<(1<<6);g++)
    {
     mask2=mask1|(g<<5);
     for(b=0;b<(1<<5);b++)
      if((r+(g>>1)+b)/3>2 && (r+(g>>1)+b)/3<40)
       distance[mask2|b]=
        8*(compute_hue(r,g>>1,b)-preferred);
      else
       distance[mask2|b]=preferred+360.0;
    }
   }
   else
     for(r=0;r<(1<<5);r++)
     {
      mask1=r<<10;
      for(g=0;g<(1<<5);g++)
      {
       mask2=mask1|(g<<5);
       for(b=0;b<(1<<5);b++)
         if((r+g+b)/3>2 && (r+g+b)/3<40)
           distance[mask2|b]=
             8*(compute_hue(r,g,b)-preferred);
	 else
           distance[mask2|b]=preferred+360.0;

      }
   }
}
