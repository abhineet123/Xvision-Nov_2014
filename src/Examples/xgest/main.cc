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
#include "Bt8x8.h"
#endif

#include "Buffering.h"
#include "HCIWindow.h"
#include "HCISelector.h"

#define GEST_SCALE	1
#define NUM_OBJECTS	2
#define OBJECT_SIZE	80

FrameBuffer<u_short> 				*buf_manager;
static Video<u_short>  				*grabber;
static HCIWindow<u_short>    			*window,*templ_win,
						*cur_grab_win;
static HCISelector<u_char,u_short,double>   	*selector;
static XVImage<u_short> 			*buffers,
		image(SCALE(640),SCALE(480),NULL,
				"XVision-Manipulation [CIPS@JHU]"),
		dummy_win(SCALE(112/GEST_SCALE),SCALE(112/GEST_SCALE),NULL,"templates");
static XVImage<u_char> *segmented_image=NULL;
XVImage<double>
reduced_target(SCALE(112/GEST_SCALE),SCALE(112/GEST_SCALE)),
  			target(SCALE(112),SCALE(112));

static         int img_number=0;
pthread_t      work_thread;
static        HCIObject       object[1];

// Gesture section

#define NUM_POS		3   // number of teaching positions on screen
#define NUM_PER_POS	1   // templates per position
#define NUM_GESTURES	3   // number of teached gestures

static int set_positions[NUM_POS][2]={SCALE(100),SCALE(100),
		SCALE(400),SCALE(100),SCALE(200),SCALE(100)};
				        //100,300,400,300,200,300};
static char  *gesture_name[NUM_GESTURES]={"fist","open palm",
					  "pointing finger"};
					  
Gesture<double,double> 
		*gesture_templates[NUM_GESTURES*NUM_POS*NUM_PER_POS];

void *task(void *dummy)
{
  int	  cur_frame,next_frame;
  Blob   *regions=NULL;
  int	 i,reg_count,n,sig_blob;
  int	 catched;
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
  // ask for current grabbed frame
  cur_frame=buf_manager->request_frame(0);
  // gesture acquisition 
  for(gesture_num=0,template_num=0,gest_temp_num=0;
  		gesture_num<NUM_GESTURES;)
  {
      // queue next available frame
      next_frame=buf_manager->request_frame(1);
      // wait for frame
      buf_manager->wait_frame(cur_frame); 
      // copy image into window
      window->CopyImage(cur_frame,1);
      // release the current buffer

      sprintf(buffer,"Show gesture: \'%s\' template #: %d",
      		gesture_name[gesture_num],template_num+1);
      window->print_string(buffer,SCALE(150),SCALE(40),0);
      window->ShowTeach(set_positions[template_num%NUM_POS][0],
      			set_positions[template_num%NUM_POS][1]);
      //set subimage to active window
      buffers[cur_frame].set_subimage(
	       set_positions[template_num%NUM_POS][0],
	       set_positions[template_num%NUM_POS][1],SCALE(112),
	       SCALE(112));
      //analyse image
      segmented_image=selector->segment_image(&buffers[cur_frame],
	        segmented_image);
      selector->find_relevant(segmented_image,regions,reg_count,3);
      temp_copy=template_num;
      switch(selector->select_gesture(regions,reg_count,sig_blob,
					 object[0]))
      {
	   case GESTURE_STOPPED:
	       //selector->segment_distance(&buffers[cur_frame],&target);
               convert(buffers[cur_frame],target);
	       target.reduce_resolution(GEST_SCALE,GEST_SCALE,reduced_target);
	       gesture_templates[gest_temp_num++]=
	       	  new Gesture<double,double>(reduced_target);
	       template_num++;
	       if(template_num==NUM_PER_POS*NUM_POS)
	       {
	         template_num=0,gesture_num++;
	       }
	       object[0].moving=0,object[0].length=0;
	     break;
	   default:
	     break;
      }
      buf_manager->release_buffer(cur_frame);
      if(sig_blob>=0)
	{
	  window->ShowReg(&(regions[sig_blob]),1,
			set_positions[temp_copy%NUM_POS][0],
			set_positions[temp_copy%NUM_POS][1],0);
	}
      // double buffer? then swap buffers
      window->swap_buffers();
      // refresh display
      window->flush();
      // queued frame is the next expected
      cur_frame=next_frame;
  }
  // regular operation
  template_num=0;
  templ_win->map();
  cur_grab_win->map();
  for(;;)
  {
      // queue next available frame
      next_frame=buf_manager->request_frame(1);
      // wait for frame
      buf_manager->wait_frame(cur_frame); 
      // copy image into window
      window->CopyImage(cur_frame,1);

      window->ShowTeach(set_positions[template_num%NUM_POS][0],
      			set_positions[template_num%NUM_POS][1]);
      //set subimage to active window
      buffers[cur_frame].set_subimage(
	       set_positions[template_num%NUM_POS][0],
	       set_positions[template_num%NUM_POS][1],SCALE(112),
	       	SCALE(112));
      //analyse image
      segmented_image=selector->segment_image(&buffers[cur_frame],
	        segmented_image);
      selector->find_relevant(segmented_image,regions,reg_count,3);
      temp_copy=template_num;
      switch(selector->select_gesture(regions,reg_count,sig_blob,
					 object[0]))
      {
	   case GESTURE_STOPPED:
	       //selector->segment_distance(&buffers[cur_frame],&target);
               convert(buffers[cur_frame],target);
	       target.reduce_resolution(GEST_SCALE,GEST_SCALE,reduced_target);
	       for(i=0;i<gest_temp_num;i++)
	       {
	         tmp_y[i]=
		   gesture_templates[i]-> Match_value(reduced_target);
#ifdef NEVER
	         cerr<< "Gesture: " 
	           << gesture_name[i/(NUM_POS*NUM_PER_POS)]
	            << " with objective value "
		    << tmp_y[i].values <<
		    tmp_y[i].objective_value_per_pixel << endl;
#endif
	       }
	      for(n=0,i=0;i<NUM_GESTURES;i++)
	      {
		best_match[i]=n;
		for(int j=0;j<NUM_PER_POS*NUM_POS;j++,n++)
		{
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
	      //if(track_val>
	      //3*tmp_y[best_match[n]].objective_value_per_pixel/2)
	      {
               convert(reduced_target,dummy_win);
	       cur_grab_win->CopySubImage(&dummy_win);
	       cur_grab_win->swap_buffers();
	       cur_grab_win->flush();
               convert(gesture_templates[best_match[n]]->get_target_win(),
	       			dummy_win);
	       templ_win->CopySubImage(&dummy_win);
	       templ_win->swap_buffers();
	       templ_win->flush();
	       //templ_win->CopySubImage(
	        //      gesture_templates[catched].get_target_win());
	       gest_index=n;gest_timer=35;
	      }
	      object[0].moving=0,object[0].length=0;
	     break;
	   default:
	     break;
      }
      // release the current buffer
      buf_manager->release_buffer(cur_frame);
      if(sig_blob>=0)
	{
	  window->ShowReg(&(regions[sig_blob]),1,
			set_positions[temp_copy%NUM_POS][0],
			set_positions[temp_copy%NUM_POS][1],0);
	}
      if(gest_timer)
      {
	sprintf(buffer,"gesture %s # %d",gesture_name[gest_index],
				gest_index%(NUM_POS*NUM_PER_POS));
        window->print_string(buffer,SCALE(300),SCALE(40),0);
	//sprintf(buffer,"distance to others %f",track_val);
        //window->print_string(buffer,SCALE(350),SCALE(460),0);
	gest_timer--;
      }
      // double buffer? then swap buffers
      window->swap_buffers();
      // refresh display
      window->flush();
      // queued frame is the next expected
      cur_frame=next_frame;
  }
  return NULL;
}

int main(int argc,char **argv)
{

  // initialization
  grabber=NULL;
  // open input device
#ifdef HAVE_BTTV
  grabber=new Bt8x8<u_short>(image.Width(),image.Height());
  //grabber->set_input(BT848_IFORM_NTSC,SVIDEO);
  grabber->set_input(BT848_IFORM_NTSC,Composite1);
  //grabber->set_input(BT848_IFORM_PAL_BDGHI,Composite1);
#endif
  if(!grabber)
  {
    cerr << "no framegrabber found, check your installation" << endl;
    exit(1);
  }
  // map available frames to user space
  buffers=grabber->get_mmap();
  // init managment layer for frame control
  buf_manager= new FrameBuffer<u_short>(grabber);
  buf_manager->create_thread();
  // init color selection
  selector=new HCISelector<u_char,u_short,double>(16,32,8,170,2);
  // open window
  window=new HCIWindow<u_short>(&image,40,40,0,NULL,4,1);
  templ_win=new HCIWindow<u_short>(&dummy_win,700,40,0,NULL,4,1);
  cur_grab_win=new HCIWindow<u_short>(&dummy_win,700,200,0,NULL,4,1);
  // initialize XImages for buffers available
  window->setImages(buffers,grabber->get_num_buffers());
  // map window on screen
  window->map();
  // to show the idle cycles :)))
  pthread_create(&work_thread,NULL,&task,NULL);
  // wait forever
  select(0,NULL,NULL,NULL,NULL);
  return 0;
}
