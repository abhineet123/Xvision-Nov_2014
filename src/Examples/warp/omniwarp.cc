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

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include "XVDig1394.h"
#include "XVImageBase.h"
#include <signal.h>

static  XVWindow<XV_RGB>      *window;
static  XVVideo<XVImageRGB<XV_RGB> >        *grabber;

#define DEG2RAD(x) 	     ((x)*M_PI/180.0)
#define ANG_RESOLUTION       2   // 1/ANG_RESOLUTION degrees

void sighndl(int ws) {
  cerr << "sig called" << endl;
  if(grabber) delete grabber;
  exit(0);
}

XVImageRGB<XV_RGB>& unwarp(XVImageRGB<XV_RGB> &source,int sphere_flag)
{
   int           i,j,dx,dy,end_val;
   double        y,
                 mirror_geo[sphere_flag?90*ANG_RESOLUTION:
			                56*ANG_RESOLUTION];
   const double  b=0.001,sqrt_b=sqrt(b);
   const double  resolution=DEG2RAD(1.0/ANG_RESOLUTION);
   const int	 cal_y=-19,cal_x=-1;
   const int     width=source.Width();
   const int     corr_x=width/2+cal_x,corr_y=source.Height()/2+cal_y;

   
   // pre-calculate lookup-table for radial sampling of the original image
   //  it allows arbitrary mirror geometries at no additional costs
   if(sphere_flag)
   {
     end_val=90*ANG_RESOLUTION;
     for(i=0;i<end_val;i++)
     {
        // subtract the (maximum angle)/resolution
        y=DEG2RAD((i-75*ANG_RESOLUTION+1)/ANG_RESOLUTION);
        mirror_geo[i]=acos((2*sqrt_b-(1+b)*sin(y))/((1+b)-2*sqrt_b*sin(y)))*120;
     }
   }
   else
   {
     double pix_res=tan(DEG2RAD(41))/(41.0*ANG_RESOLUTION);
     end_val=56*ANG_RESOLUTION;
     for(i=0;i<end_val;i++)
     {
        y=atan((i-41*ANG_RESOLUTION)*pix_res);
	mirror_geo[i]=acos((2*sqrt_b-(1+b)*sin(y))/((1+b)-2*sqrt_b*sin(y)))*120;
     }
   }

   static XVImageRGB<XV_RGB> dest_im(360*ANG_RESOLUTION,end_val);
   const  XV_RGB             *r_ptr=source.data();
   XV_RGB                    *w_ptr=dest_im.lock();
   // this is the original implementation, before optimization
   //  Darius
   //for(i=0;i<180;i++)
   //  for(j=0;j<720;j++)
   //  {
   //    x=DEG2RAD(j/2.0);
   //    fi=mirror_geo[i];
   //    dx=fi*cos(x)-fi*sin(x);
   //    dy=fi*sin(x)+fi*cos(x);
   //    *w_ptr++=r_ptr[(int)((-dy+height2-16))*width-(int)dx+width/2];
   //  }
   //
   //  sine and cosine start at 0 rad :-)
   double  cos_ang=1.0,sin_ang=0.0,fi,temp;
   const double	sin_delta=sin(resolution),cos_delta=cos(resolution);
   for(i=0;i<end_val;i++)
   {
     fi=mirror_geo[i];
     for(j=0;j<360*ANG_RESOLUTION;j++)
     {
       dx=(int)( fi*cos_ang-fi*sin_ang);
       dy=(int)( fi*sin_ang+fi*cos_ang);
       *w_ptr++=r_ptr[(-dy+corr_y)*width-dx+corr_x];
       // simple circle optimization
       temp    = cos_ang*cos_delta-sin_ang*sin_delta;
       sin_ang = sin_ang*cos_delta+cos_ang*sin_delta;
       cos_ang = temp;
     }
   }
   dest_im.unlock();
   return dest_im;
}

int main (int argc, char **argv) {

  int framenum=0, next_fr=0, n_buffers, i, j, rate=100,
    output=1, depth;
  struct timeval time1, time2; 

  //signal(SIGINT, sighndl);

  if (!grabber) {
    grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0"); //full-size
    //grabber = new Dig1394<XV_RGB>(DC_DEVICE_NAME,"S2"); //half-size
  }
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  n_buffers=grabber->buffer_count();
  XVImageRGB<XV_RGB> image(grabber->frame(0));

  //Set up the window
  //if (output == 1 || output == 3) {
  window = new XVWindowX<XV_RGB> ((grabber->frame(0)));
  window -> map();
  //}  

  grabber -> initiate_acquire(framenum);
  for (;;) {
    //Start timing
    gettimeofday (&time1, NULL);
    for (i=0; i<rate; i++) {
      //Acquire image
      grabber -> initiate_acquire((framenum+1)%n_buffers);
      grabber -> wait_for_completion(framenum);
      image=unwarp(grabber->frame(framenum),0);
      window->CopySubImage(image);
      window -> swap_buffers();
      window -> flush();
      framenum=(framenum+1)%n_buffers;
    }
    gettimeofday (&time2, NULL);
    cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
    (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;

  }
  return 0;
}
