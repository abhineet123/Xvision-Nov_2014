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

#include <math.h>
#include <values.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"
#include "Gesture.h"

#ifdef HAVE_BTTV
#include "XVBt8x8.h"
#endif
#include "Videre.h"

#include "HCIWindow.h"
#include "HCISelector.h"
#include "XVColorSeg.h"
#include "XVBlobFeature.h"

#define GEST_SCALE	1
#define NUM_OBJECTS	2
#define OBJECT_SIZE	(112/2)

static XVVideo<XVImageRGB<XV_RGB> >		*grabber;
static HCIWindow<XV_RGB >		*window,*templ_win,
						*cur_grab_win;
static XVImageRGB<XV_RGB> 			*buffers,
		image(640,480,NULL),
		dummy_win(112,112,NULL);
static XVImageScalar<u_short> segmented_image(640,480);
XVImageScalar<double> reduced_target(2*OBJECT_SIZE,2*OBJECT_SIZE), target(2*OBJECT_SIZE,2*OBJECT_SIZE);

static         int img_number=0;
pthread_t      work_thread;
static    HCIObject  object[1]=
  {CIRC_OBJECT,150,100,OBJECT_SIZE,OBJECT_SIZE,5,0,0};


// Gesture section
#define Sqr(a) ((a)*(a))

#define NUM_POS		3   // number of teaching positions on screen
#define NUM_PER_POS	1   // templates per position
#define NUM_GESTURES	2   // number of teached gestures

static int set_positions[NUM_POS][2]={100,100,
		400,100,200,100};
				        //100,300,400,300,200,300};
static char  *gesture_name[NUM_GESTURES]={"fist","open palm"};
			
					  
Gesture<double,double> 
		*gesture_templates[NUM_GESTURES*NUM_POS*NUM_PER_POS];

typedef XVBlobFeature<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > BLOB; 

void reduce_resolution(XVImageBase<double>&source, int factorx, 
                  int factory, XVImageBase<double>& target)
{

    target.resize(source.Height()/factory,source.Width()/factorx);

    int i,j,k,l;
    int dfactor = factorx * factory;
    double *tcount; 
    const double *counter = source.data(), *rowc;

     tcount=target.lock();
     assert(tcount!=NULL);
     memset(tcount,0,sizeof(XV_RGB)*target.Height()*target.Width());

     for(k=0;k<target.Height();k++) 
         for(l=0;l<target.Width();l++,tcount++)
      {
         for (i=0;i<factory;i++) 
            for(j=0,rowc=counter + l*factorx+(i+k*factory)*source.Width();
                                              j<factorx;j++)
 	                *tcount+=*rowc++;
		        *tcount/=dfactor;
       }
       assert(target.unlock());
}

static int select_gesture(vector<XVRectangleBlob> &regions,
			XVRectangleBlob &found_reg, HCIObject& object)
{
   int cur_len,length=-1;

   found_reg.resize(0,0);
   for(vector<XVRectangleBlob>::iterator region=regions.begin();
        region!=regions.end();region++)

   {
         cur_len=Sqr(region->Width())+
	              Sqr(region->Height());
	 if(cur_len>length) length=cur_len;found_reg=*region;

   }
   // found significant blob?
   if(length>Sqr(30))
   {
     // did it change?
     if(abs(object.length-length)>Sqr(90))
     {
       object.moving=15,object.length=length;
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
   return NO_GESTURE;	// no gesture found
}

bool check_hand(u_short p) { return abs(p - 20) < 20; }

void *task(void *dummy)
{
  XVHueSeg<XV_RGB, u_short>* seg = 
          new XVHueSeg<XV_RGB, u_short>(1000, 40, 120) ;
  int    speed_x=0,speed_y=0;
  int	  cur_frame;
  vector<XVRectangleBlob> regions;
  XVRectangleBlob	    found_reg;
  int	 i,n,catched=0;
  int	 gesture_num,template_num=0,temp_copy,gest_temp_num;
  double track_val,cur_val;
  int	 gest_timer=0,gest_index,best_match[NUM_GESTURES];
  char	 buffer[80];
  TrackerOutput y,tmp_y[NUM_GESTURES*NUM_POS*NUM_PER_POS];

#if SCALE(4)==4
  window->get_font("-*-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
#else
  window->get_font("-*-times-bold-r-normal-*-12-*-*-*-*-*-*-*");
#endif

  seg->setCheck(check_hand);
  // ask for current grabbed frame
  //grabber->initiate_acquire(cur_frame=0); gesture acquisition 
  XVImageGeneric roi; for(gesture_num=0,template_num=0,gest_temp_num=0;
      gesture_num<NUM_GESTURES;) {
      // queue next available frame
      //grabber->initiate_acquire(cur_frame^1); wait for frame
      grabber->wait_for_completion(cur_frame=0); 
      // copy image into window
      window->CopySubImage(grabber->frame(cur_frame),1);
      // release the current buffer

      sprintf(buffer,"Show gesture: \'%s\' template #: %d",
      		gesture_name[gesture_num],template_num+1);
      window->print_string(buffer,150,49,12);
      window->ShowTeach(set_positions[template_num%NUM_POS][0],
      			set_positions[template_num%NUM_POS][1]);
      //set subimage to active window

      roi.reposition(set_positions[template_num%NUM_POS][0],
	              set_positions[template_num%NUM_POS][1]);
      roi.resize(112, 112);
      XVImageRGB<XV_RGB> im_temp=subimage(grabber->frame(cur_frame),roi);
      RGBtoScalar(im_temp,target);
      //analyse image
      seg->segment(im_temp,segmented_image);
      seg->regionGrow(segmented_image,regions,NULL,90);
      temp_copy=template_num;
      switch(select_gesture(regions,found_reg, object[0]))
      {
	   case GESTURE_STOPPED:
	       //selector->segment_distance(&buffers[cur_frame],&target);
	       reduce_resolution(target,GEST_SCALE,GEST_SCALE,
		   reduced_target);
	       gesture_templates[gest_temp_num]=
	       	  new Gesture<double,double>(reduced_target);
	       template_num++;gest_temp_num++;
	       if(template_num==NUM_PER_POS*NUM_POS)
	       {
	         template_num=0,gesture_num++;
	       }
	       object[0].moving=0,object[0].length=0;
	     break;
	   default:
	     break;
      }
      if(found_reg.Width()>0)
	{
	  window->ShowReg(regions,
			set_positions[temp_copy%NUM_POS][0],
			set_positions[temp_copy%NUM_POS][1],0);
	}
      // double buffer? then swap buffers
      window->swap_buffers();
      // refresh display
      window->flush();
      // queued frame is the next expected
      //cur_frame^=1;
  }
  // regular operation
  template_num=0;
  for(;;)
  {
      // queue next available frame
      //grabber->initiate_acquire(cur_frame^1);
      // wait for frame
      grabber->wait_for_completion(cur_frame=0); 
      // copy image into window
      window->CopySubImage(grabber->frame(cur_frame),1);
      roi.reposition( object[0].posx-OBJECT_SIZE/2,
	              object[0].posy-OBJECT_SIZE/2);
      roi.resize(2*OBJECT_SIZE,2*OBJECT_SIZE);
      XVImageRGB<XV_RGB> im_temp=subimage(grabber->frame(cur_frame),roi);
      RGBtoScalar(im_temp,target);
      seg->segment(im_temp,segmented_image);
      seg->regionGrow(segmented_image,regions,NULL,90);

      //analyse image
      temp_copy=template_num;
      switch(select_gesture(regions,found_reg,object[0]))
      {
           case GESTURE_CHANGED:
             if(catched)
             {
               int tx,ty;

               tx=found_reg.Width()/2;
               ty =found_reg.Height()/2;
	       if(ty>tx) ty=tx;
	       tx+=found_reg.PosX()-OBJECT_SIZE;
               ty+=found_reg.PosY()-OBJECT_SIZE;
               if(tx!=0) speed_x+=(int)((tx-speed_x)*.6);
                 else    speed_x=0;
               if(ty!=0) speed_y+=(int)((ty-speed_y)*.6);
                 else    speed_y=0;
               object[0].posx+=speed_x;
               object[0].posy+=speed_y;
               if(object[0].posx<OBJECT_SIZE/2) 
                                         object[0].posx=OBJECT_SIZE/2;
               if(object[0].posy<OBJECT_SIZE/2) 
                                         object[0].posy=OBJECT_SIZE/2;
               if(object[0].posx+2*OBJECT_SIZE>image.Width()) 
                        object[0].posx=image.Width()-2*OBJECT_SIZE;
               if(object[0].posy+2*OBJECT_SIZE>image.Height()) 
	       		object[0].posy=image.Height()-2*OBJECT_SIZE;
	     }
	     break;
	   case GESTURE_STOPPED:
	       for(i=0;i<gest_temp_num;i++)
	       {
	         reduce_resolution(target,GEST_SCALE,GEST_SCALE,
						reduced_target);
	         tmp_y[i]=
		   gesture_templates[i]-> Match_value(reduced_target);
	       }
	      for(n=0,i=0;i<NUM_GESTURES;i++)
	      {
	        best_match[i]=n;
		for(int j=0;j<NUM_PER_POS*NUM_POS;j++,n++)
		{
		  cerr << tmp_y[n].objective_value_per_pixel << endl;
		  if(tmp_y[n].objective_value_per_pixel<
		     tmp_y[best_match[i]].objective_value_per_pixel)
		  {
		    best_match[i]=n;
		  }
		}
	      }
	      n=0;
	      for(i=0;i<NUM_GESTURES;i++)
	        if(tmp_y[best_match[i]].objective_value_per_pixel<
		   tmp_y[best_match[n]].objective_value_per_pixel) n=i;
	      track_val=MAXFLOAT;
	      for(i=0;i<NUM_GESTURES;i++)
	      {
	        if(i==n) continue;
		if(track_val>
		    tmp_y[best_match[i]].objective_value_per_pixel-
		    tmp_y[best_match[n]].objective_value_per_pixel)
		  track_val=
		    tmp_y[best_match[i]].objective_value_per_pixel-
		    tmp_y[best_match[n]].objective_value_per_pixel;
	      }
              catched=(n>NUM_GESTURES/2);
	      cerr << "match result " << track_val << " gesture: " 
		   << catched << endl;
	     break;
           case GESTURE_UNCHANGED:
             if(catched)
             {
               int tx,ty;
               tx=found_reg.Width()/2;
               ty=found_reg.Height()/2;
               if(ty>tx) ty=tx;
               tx+=found_reg.PosX()-OBJECT_SIZE;
               ty+=found_reg.PosY()-OBJECT_SIZE;

               if(tx!=0) speed_x+=(int)((tx-speed_x)*.6);
                 else    speed_x=0;
               if(ty!=0) speed_y+=(int)((ty-speed_y)*.6);
                 else    speed_y=0;
               object[0].posx+=speed_x;
               object[0].posy+=speed_y;
               if(object[0].posx<OBJECT_SIZE/2)
                                         object[0].posx=OBJECT_SIZE/2;
               if(object[0].posy<OBJECT_SIZE/2)
                                         object[0].posy=OBJECT_SIZE/2;
               if(object[0].posx+2*OBJECT_SIZE>image.Width())
                        object[0].posx=image.Width()-2*OBJECT_SIZE;
               if(object[0].posy+2*OBJECT_SIZE>image.Height())
                        object[0].posy=image.Height()-2*OBJECT_SIZE;
             }
	     break;
	   default:
	     break;
      }
      window->draw_object(&object[0],catched);
      // release the current buffer
      window->swap_buffers();
      // refresh display
      window->flush();
      // queued frame is the next expected
      cur_frame^=1;
  }
  return NULL;
}

int main(int argc,char **argv)
{

  // initialization
  grabber=NULL;
  // open input device
  grabber=new XVBt8x8<XVImageRGB<XV_RGB> > ("I1B2N0");
  if(!grabber)
  {
    cerr << "no framegrabber found, check your installation" << endl;
    exit(1);
  }
  // map available frames to user space
  buffers=&(grabber->frame(0));
  // open window
  window=new HCIWindow<XV_RGB >(image,40,40,NULL,0,NULL,4,1);
  //templ_win=new XVWindowX<XV_RGB> (112,112);
  //templ_win->map();
  // initialize XImages for buffers available
  window->setImages(buffers,grabber->buffer_count());
  // map window on screen
  window->map();
  // to show the idle cycles :)))
  task(NULL);
  // wait forever
  select(0,NULL,NULL,NULL,NULL);
  return 0;
}
