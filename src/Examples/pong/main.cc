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
#include "config.h"

#ifdef HAVE_BTTV
#include "Bt8x8.h"
#endif

//#include "Buffering.h"
#include "VideoManager.h"
#include "PongWindow.h"
#include "XVColSelector.h"

//FrameBuffer<u_short> 			*buf_manager;
VideoManager<Bt8x8<u_short> > 			*buf_manager;
static Video<u_short>  			*grabber;
static PongWindow<u_short>		*window;
static XVColSelector<u_char,u_short>   	*selector;
static XVImageBase<u_short>        
                    *buffers,image(500,400,NULL,"XVision-Pong V0.2");
static XVImageBase<u_char> *segmented_image=NULL;

pthread_t      work_thread;

// simple pong
typedef struct {int x;int y;} Ball;
static          Ball         ball={20,20};

// simple ball movement computation (pool?) on screen+ bouncing from
// colored objects
static void compute_ball(Blob* &region,int count)
{
   static int vel_x=10,vel_y=6;
   Blob       *reg=region;
   int i,not_found=1;

   // check for bouncing from the window frame
   if(abs(ball.x)<BALL_SIZE/2 || 
      (abs(ball.x-image.Width()) <BALL_SIZE/2))
        vel_x=-vel_x,not_found=0;
   if(abs(ball.y)<BALL_SIZE/2 || 
      (abs(ball.y-image.Height()) <BALL_SIZE/2))
        vel_y=-vel_y,not_found=0;
   // check for bouncing from colored regions
   for(i=0;not_found && i<count;i++,reg++)
   {
     if((abs(ball.x-reg->sx)<BALL_SIZE/2 || 
      (abs(ball.x-reg->ex) <BALL_SIZE/2)) && 
        (ball.y>reg->sy && ball.y<reg->ey)) vel_x=-vel_x,not_found=0;
     if((abs(ball.y-reg->sy)<BALL_SIZE/2 || 
        (abs(ball.y-reg->ey) <BALL_SIZE/2)) && 
        (ball.x>reg->sx && ball.x<reg->ex)) vel_y=-vel_y,not_found=0;
   }
   ball.x+=vel_x;
   ball.y+=vel_y;
}

void *task(void *dummy)
{
  int	  cur_frame,next_frame;
  struct timeval time1,time2;
  Blob   *regions=NULL;
  int	 i,reg_count;

  // ask for current grabbed frame
  cur_frame=buf_manager->request_frame(0);
  for(;;)
  {
    // time measurment routine start
    gettimeofday(&time1,NULL);
    for(i=0;i<100;i++)
    {
      // queue next available frame
      next_frame=buf_manager->request_frame(1);
      // wait for frame
      buf_manager->wait_for_completion(cur_frame); 
      // processing
      segmented_image=selector->segment_Image(&buffers[cur_frame],
      						segmented_image);
      selector->find_relevant(segmented_image,regions,reg_count,2);
      compute_ball(regions,reg_count);
      // copy image into window
      window->CopyImage(cur_frame,1);
      // release the current buffer
      buf_manager->release_buffer(cur_frame);
      window->ShowReg(regions, reg_count);
      window->DrawBall(ball.x,ball.y,BALL_SIZE);
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
  selector=new XVColSelector<u_char,u_short>(16,32);
  // open window
  window=new PongWindow<u_short>(&image,40,40,0,NULL,4);
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
