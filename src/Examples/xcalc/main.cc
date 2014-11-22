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
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"

#ifdef HAVE_BTTV
#include "Bt8x8.h"
#endif

#include "Buffering.h"
#include "HCIWindow.h"
#include "HCISelector.h"

#define NUM_OBJECTS	2
#define OBJECT_SIZE	SCALE(80)

extern char *x[16];

FrameBuffer<u_short> 				*buf_manager;
static Video<u_short>  				*grabber;
static HCIWindow<u_short>    			*window;
static HCISelector<u_char,u_short,float>   	*selector;
static XVImage<u_short> *buffers,
    image(SCALE(640),SCALE(480),NULL,"XVision Calculator [CIPS@JHU]");
static XVImage<u_char> *segmented_image=NULL;
HCIObject       object[16];

static         int img_number=0;
pthread_t      work_thread;

//calculator specific

  static double   stack[3]={0};
  static int	  op_stack[2];
  static int	  stack_cnt=0,op_count=0;

double operation_handling(int key_index)
{
  switch(op_count)
  {
   case 0: // no outstanding ops
    switch(key_index)
    {
      default:
        op_stack[0]=key_index;
        op_count++;
      case KEY_EQUAL:
        return stack[0];
    }
   case 1: // one outstanding op
    switch(key_index)
    {
      case KEY_MUL:
       if(op_stack[0]!=KEY_MUL)
       {
         op_stack[op_count]=key_index;
         op_count++;
	 return stack[1];
       }
       if(stack_cnt==1)
         stack[0]=0.0;
       else
        {
	   stack_cnt--;
	   stack[0]*=stack[1];
        }
       op_stack[0]=key_index;
       op_count=1;
       return stack[0];
      case KEY_EQUAL:
      case KEY_NEG:
      case KEY_PLUS:
       switch(op_stack[0])
       {
         case KEY_MUL:
          if(stack_cnt!=1)
          {
	    stack_cnt--;
	    stack[0]*=stack[1];
          }
	  break;
         case KEY_PLUS:
          if(stack_cnt!=1)
          {
	    stack_cnt--;
	    stack[0]+=stack[1];
          }
	  break;
         case KEY_NEG:
          if(stack_cnt!=1)
          {
	    stack_cnt--;
	    stack[0]-=stack[1];
          }
	  else
	    stack[0]=-stack[0];
	  break;
        }
	if(key_index==KEY_EQUAL) op_count=0;
	else
	{
	 op_stack[0]=key_index;
	 op_count=1;
	}
	return stack[0];
      }
      break;
    case 2:
     switch(key_index)
     {
      case KEY_MUL:
        stack[1]*=stack[2];
	stack_cnt=2;
        return stack[1];
      case KEY_EQUAL:
      case KEY_PLUS:
      case KEY_NEG:
       if(op_stack[0]==KEY_PLUS)
        stack[0]+=stack[1]*stack[2];
       else
        stack[0]+=stack[1]*stack[2];
       if(key_index!=KEY_EQUAL)
       {
         op_stack[0]=key_index;
	 stack_cnt=1;
       }
       else
         stack_cnt=0;
       return  stack[0];
     }
   }
}

double calculator(int key_index,int catched,int skip_frames)
{
  static int	  neg=0;
  static double	  pow,sum=0.0;
  static int 	  mode=0,initial=1;
  
      if(catched)
      {
        switch(mode)
	{
	  case 0: // enter number
	    if(catched==2)
	    {
	      if(initial) {initial=0;sum=0.0;}
	      sum*=10.0;
	      sum=neg? sum-atof(x[key_index]):sum+atof(x[key_index]);
	    }
	    else
	    {
	      switch(key_index)
	      {
	        case KEY_DPOINT:
		  pow=.1;
		  mode=1;
		  break;
	        case KEY_NEG:
		  sum=-sum;
		  neg^=1;
		  break;
		default:
		  stack[stack_cnt++]=sum;
		  //sum=operation_handling(key_index);
                  sum=0.0;
                  initial=1;
		  break;
	      }
	    }
	    break;
	  case 1:  // .xx digits
	    if(catched==2)
	    { 
	     sum=neg? sum-atof(x[key_index])*pow:
	              sum+atof(x[key_index])*pow;
	     pow*=.1;}
	    else
	     switch(key_index)
	     {
	       case KEY_NEG:
	         sum=-sum;
	         neg^=1;
		 break;
	       case KEY_DPOINT:
	         break;
	       default:
		 stack[stack_cnt++]=sum;
	         sum=operation_handling(key_index);
		 initial=1;
		 neg=0;
		 mode=0;
	         break;
	     }
	    break;
	}
      }
      window->DrawCalc(sum,skip_frames? x[key_index][0]: 255);
}

void *task(void *dummy)
{
  int	  cur_frame,next_frame;
  struct timeval time1,time2;
  Blob   *regions=NULL;
  int	 width_corr=buffers[0].Width()-SCALE(KEY_SIZE);
  int	 found,j,i,reg_count,n,sig_blob;
  int	 key_index;
  int	 catched=0,skip_frames;

#if SCALE(4)==4
  window->get_font("-*-courier-medium-r-normal-*-34-*-*-*-*-*-*-*");
#else
  window->get_font("-*-courier-medium-r-normal-*-16-*-*-*-*-*-*-*");
#endif
  // ask for current grabbed frame
  cur_frame=buf_manager->request_frame(0);
  for(skip_frames=0;;)
  {
    // time measurment routine start
    gettimeofday(&time1,NULL);
    for(i=0;i<100;i++)
    {
      // queue next available frame
      next_frame=buf_manager->request_frame(1);
      // wait for frame
      buf_manager->wait_frame(cur_frame); 
      // copy image into window
      window->CopyImage(cur_frame,1);
      if(skip_frames) skip_frames--;
      else
      {
      for(i=0,found=0;!found && i<4;i++)
        for(j=0;!found && j<4;j++)
      {
        buffers[cur_frame].set_subimage(
          width_corr-SCALE(KEY_DISTANCE*j+RECT_OFFSET_X),
	  SCALE(KEY_DISTANCE*i+RECT_OFFSET_Y),
	  SCALE(KEY_SIZE),SCALE(KEY_SIZE));
        segmented_image=selector->segment_image(&buffers[cur_frame],
                segmented_image);
        selector->find_relevant(segmented_image,regions,reg_count,2);
        switch(selector->select_gesture(regions,reg_count,sig_blob,
                                         object[i*4+j]))
        {
           case GESTURE_STOPPED:
	     if(sig_blob>0)
	     {
	       {
	         key_index=i*4+j;
		 skip_frames=20;
		 catched=((i<3 && j<3) || (!j && i==3))? 2: 1;
	       }
	     }
             break;
           default:
             break;
        }
	if(sig_blob>=0) found=1;
        //if(sig_blob>=0)
        //{
         // window->ShowReg(&(regions[sig_blob]),1,
	//		width_corr-KEY_DISTANCE*j-RECT_OFFSET_X,
	//		KEY_DISTANCE*i+RECT_OFFSET_Y,0);
        //}
      }
      }
      calculator(key_index,catched,skip_frames);
      catched=0;
      // release the current buffer
      buf_manager->release_buffer(cur_frame);
      // handle objects
      // double buffer? then swap buffers
      window->swap_buffers();
      // refresh display
      window->flush();
      // queued frame is the next expected
      cur_frame=next_frame;
    }
    gettimeofday(&time2,NULL);
    cerr << "rate: " <<1e2/(time2.tv_sec-time1.tv_sec+
   	(time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;
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
  selector=new HCISelector<u_char,u_short,float>(16,32,16,240,0);
  // open window
  window=new HCIWindow<u_short>(&image,40,40,0,NULL,4,1);
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
