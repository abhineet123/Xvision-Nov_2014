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
#include <XVImageIO.h>
#include <XVImageBase.h>
#include <XVImageScalar.h>
#include <XVImageRGB.h>
#include "PongSelector.h"

using namespace std ;

template class PongSelector<XV_RGB,u_short>;

#define THRESH 5.0
#define HIT_THRESH 15


/*
 * check_ball
 *
 * IN: 
 * OUT:
 * DO:  check to see if the ball hits a *hittable* region
 */
template <class T, class Y>
int PongSelector<T,Y>::check_ball(XVImageRGB<T> *image,int x, int y, 
                                int ball_size, bool (*pf) (const Y) )
{
  int 			i,j;
  int			ret_val=0;
  const T		*i_ptr;
  static XVImageScalar<Y> segmented_image;

  x-=(int)(1.1*ball_size/2);
  y-=(int)(1.1*ball_size/2);
  if(x<0) x=0;
  if(y<0) y=0;
  if(x+ball_size>=image->SizeX()) 
  		x=image->SizeX()-ball_size-1;
  if(y+ball_size>=image->SizeY()) 
  		y=image->SizeY()-ball_size-1;
  if(y<100) return 0;  // no hits in upper space allowed

  int w = image->Width();
  int h = image->Height();

  image->setSubImage(x,y,ball_size,ball_size);
  segment(*image,segmented_image);

  /* leaving the code like this will result in random outliers that
   * satisfy the condition causing a bounce...in other words, the code
   * is not very powerful in relation to random noise
   */

  XVImageIterator<Y> iter(segmented_image);
  XVPosition cen;
  int count=1;
  findCentroid(segmented_image,cen,count,pf);
  //printf("%d ----- %d\n",count,HIT_THRESH);
  if (count > HIT_THRESH) {
    for(; !iter.end(); ++iter){
      if(pf(*iter)){
        //printf("passed -> %d\n",*iter);
        if(abs(iter.currposy()-image->Height()/2)<abs(iter.currposx()-image->Width()/2)) {
          ret_val|=(iter.currposy()-image->Height()/2)>0? DOWN_HIT: UP_HIT;
//XVImageRGB<T> im;
//ScalartoRGB(segmented_image,im);
//XVWritePPM(im,"seg.ppm");
//XVWritePPM(*image,"rgb.ppm");
//exit(0);
        } else {
          ret_val|=(iter.currposx()-image->Width()/2)>0? RIGHT_HIT: LEFT_HIT;
        }
      }
    }
  }

  image->setSubImage(0,0,w,h);

/*
  i_ptr=segmented_image->data();
  for(i=0;i<segmented_image->Height();i++,
  			i_ptr+=segmented_image->Skip())
   for(j=0;j<segmented_image->Width();j++)
     if(*i_ptr++==preferred)
     {
        if(abs(i-image->Height()/2)<abs(j-image->Width()/2)) 
						// vertical hit?
	  ret_val|=(i-image->Height()/2)>0? DOWN_HIT: UP_HIT;
	else
	  ret_val|=(j-image->Width()/2)>0? RIGHT_HIT: LEFT_HIT;
     }

*/

  return ret_val;
}


/*  CONSTRUCTOR
 */
template <class T, class Y>
PongSelector<T,Y>::PongSelector(int num_sectors, int dark_pix,int bright_pix)
	          :XVHueSeg<T,Y>(num_sectors,dark_pix,bright_pix) {
}
