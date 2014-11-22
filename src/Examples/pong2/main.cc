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
#include <stdio.h>

/* XVision2 Headers */
#include "config.h"
#ifdef HAVE_BTTV
#include "XVBt8x8.h"
#endif
#include "XVImageRGB.h"

/* pong2 headers */
#include "PongWindow.h"
#include "PongSelector.h"

static XVVideo< XVImageRGB<XV_RGB> >  	*grabber;
static PongWindow<XV_RGB>    		*window;
static PongSelector<XV_RGB,u_short>     *selector;
static XVImageRGB<XV_RGB> image(640,480);
float  paddle_hue;

int wait_per=-1;
pthread_t      work_thread;

// waiting period after collision
#define	WPER	15
#define NUM_BRICKS	30

// simple pong

static char *pcred = "red";
static char *pcblue = "blue";
static char *pcyellow = "yellow";

typedef struct tagBall {int x;int y;} Ball;

static  Ball         ball={SCALE(20),SCALE(420)};
static  Brick	     bricks[NUM_BRICKS],st_bricks[NUM_BRICKS]=
{
  {SCALE(20),SCALE(10),pcred,1,1},
  {SCALE(80),SCALE(10),pcred,1,1},
  {SCALE(140),SCALE(10),pcred,1,1},
  {SCALE(200),SCALE(10),pcred,1,1},
  {SCALE(260),SCALE(10),pcred,1,1},
  {SCALE(320),SCALE(10),pcred,1,1},
  {SCALE(380),SCALE(10),pcred,1,1},
  {SCALE(440),SCALE(10),pcred,1,1},
  {SCALE(500),SCALE(10),pcred,1,1},
  {SCALE(560),SCALE(10),pcred,1,1},
  {SCALE(20),SCALE(40),pcblue,2,1},
  {SCALE(80),SCALE(40),pcblue,2,1},
  {SCALE(140),SCALE(40),pcblue,2,1},
  {SCALE(200),SCALE(40),pcblue,2,1},
  {SCALE(260),SCALE(40),pcblue,2,1},
  {SCALE(320),SCALE(40),pcblue,2,1},
  {SCALE(380),SCALE(40),pcblue,2,1},
  {SCALE(440),SCALE(40),pcblue,2,1},
  {SCALE(500),SCALE(40),pcblue,2,1},
  {SCALE(560),SCALE(40),pcblue,2,1},
  {SCALE(20),SCALE(70),pcyellow,13,1},
  {SCALE(80),SCALE(70),pcyellow,13,1},
  {SCALE(140),SCALE(70),pcyellow,13,1},
  {SCALE(200),SCALE(70),pcyellow,13,1},
  {SCALE(260),SCALE(70),pcyellow,13,1},
  {SCALE(320),SCALE(70),pcyellow,13,1},
  {SCALE(380),SCALE(70),pcyellow,13,1},
  {SCALE(440),SCALE(70),pcyellow,13,1},
  {SCALE(500),SCALE(70),pcyellow,13,1},
  {SCALE(560),SCALE(70),pcyellow,13,1}
};

static		int num_bricks=NUM_BRICKS;
static 		int vel_x=SCALE(8),vel_y=SCALE(-5);
static		int score=0;
static		int balls=5;


#define HUE_FOO 2
/**
  * isHueCheck
  */
bool isHueCheck(const u_short p)
{
  return ((p) > (paddle_hue - HUE_FOO) &&
          (p) < (paddle_hue + HUE_FOO) );
}



// simple ball movement computation (pool?) on screen+ bouncing from
// colored objects
static void compute_ball(int hit_value)
{
   static int collision=0;
   int i;

 if(collision) 
   collision--;

 if(ball.y<SCALE(90))
 {
   // check for bouncing from the brick
   for (i=0;i<NUM_BRICKS;i++)
   {
     if(!bricks[i].valid) continue;

     if ( abs(ball.x-bricks[i].x-BRICK_SIZEX/2)<(BALL_SIZE+BRICK_SIZEX)/2 &&
          abs(ball.y-bricks[i].y-BRICK_SIZEY/2)<(BALL_SIZE+BRICK_SIZEY)/2) {

       bricks[i].valid=0;
       score+=bricks[i].value;
       num_bricks--;
       if(abs(ball.x-bricks[i].x-BRICK_SIZEX/2)<BRICK_SIZEX/2-3)
         vel_y=-vel_y;
       else
         vel_x=-vel_x;
     }
   }
 }

   // check for bouncing from the window frame
   if(ball.x<BALL_SIZE/2 || (abs(ball.x-image.Width()) <BALL_SIZE/2))
     vel_x=-vel_x,collision=WPER;

   if(ball.y<BALL_SIZE/2 || (abs(ball.y-image.Height()) <BALL_SIZE/2))
     vel_y=-vel_y,collision=WPER;

   if(balls && abs(ball.y-image.Height()) <BALL_SIZE/2) 
     balls--;

   if((!balls || (!num_bricks)) && wait_per<0) wait_per=90;

   if(!collision)
   {

     if(vel_y>0 && (hit_value & DOWN_HIT)) 
       vel_y=-vel_y,collision=WPER;
     //else if (hit_value & UP_HIT) 
       //vel_y=-vel_y,collision=WPER;
     else if(vel_x>0 && (hit_value & RIGHT_HIT)) 
       vel_x=-vel_x,collision=WPER;
     else if (hit_value & LEFT_HIT) 
       vel_x=-vel_x,collision=WPER;
   }

   if(balls && num_bricks)
   {
     ball.x+=vel_x;
     ball.y+=vel_y;
   }
}



void *task(void *dummy)
{
  int	  init_wait=25;
  struct timeval time1,time2;
  int	 i,reg_count;
  char	 sbuf[30];

#if SCALE(4)==4  
  window->get_font("-*-times-bold-i-normal-*-50-*-*-*-*-*-*-*");
#else
  window->get_font("-*-times-bold-i-normal-*-16-*-*-*-*-*-*-*");
#endif

  for(;;)
  {
    // time measurment routine start
    gettimeofday(&time1,NULL);
    for(i=0;i<100;i++)
    {

      /* get an image */
      image = grabber->next_frame_continuous();
      window->CopySubImage(image);

      window->drawBall(ball.x,ball.y,BALL_SIZE);
      window->drawBricks(bricks,NUM_BRICKS);

      sprintf(sbuf,"score: %03d",score);
      window->print_string(sbuf,SCALE(40),SCALE(440),0);
      sprintf(sbuf,"balls: %d",balls);
      window->print_string(sbuf,SCALE(450),SCALE(440),0);
      if(!balls)
      {
        window->print_string("Game over!!",SCALE(300),SCALE(300),6);
      }

      // processing
      if(init_wait>0) 
	  init_wait--;
      else
          compute_ball(selector->check_ball(&image,
                                ball.x,ball.y,BALL_SIZE,&isHueCheck));

      if(wait_per>0) {
        wait_per--; 
        if(!wait_per) {
          init_wait=250;balls=5;score=0;
          memcpy(bricks,st_bricks,NUM_BRICKS*sizeof(Brick));
          num_bricks=NUM_BRICKS;
        }
      }

      if(!num_bricks)
        window->print_string("You won!!",SCALE(300),SCALE(300),7);

      window->swap_buffers();
      window->flush();
    }

    gettimeofday(&time2,NULL);
    cerr << "rate: " <<1e2/(time2.tv_sec-time1.tv_sec+
   	(time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;
  }
  return NULL;
}




/**
  *  train_system
  *
  *  train the system to learn what type of paddle the person is using...
  */
void train_system()
{
  char sbuf[30];
  double td = 0;
  double wait = 3.5;
  struct timeval t1,t2;

  XVImageScalar<u_short> segimg;
  XVHueSeg<XV_RGB,u_short> seg;
  vector<XVRectangleBlob> regions;
  XVRectangleBlob box;

  gettimeofday(&t1,NULL); 
  while (td<wait) {
    image = grabber->next_frame_continuous();

    window->CopySubImage(image);

    window->get_font("-*-times-bold-i-normal-*-24-*-*-*-*-*-*-*");
    sprintf(sbuf,"%3lf",wait-td);
    window->print_string(sbuf,SCALE(300),SCALE(150),1);

    window->get_font("-*-times-bold-i-normal-*-50-*-*-*-*-*-*-*");
    sprintf(sbuf,"Training Stage - Ready Object For Training",wait-td);
    window->print_string(sbuf,SCALE(155),SCALE(30),6);

    window->swap_buffers();
    window->flush();

    gettimeofday(&t2,NULL);
    td = t2.tv_sec - t1.tv_sec + (t2.tv_usec-t1.tv_usec)*1e-6; 
  }
  image = grabber->next_frame_continuous();
  window->CopySubImage(image);

  window->get_font("-*-times-bold-i-normal-*-24-*-*-*-*-*-*-*");
  sprintf(sbuf,"Click On Object 1 time",wait-td);
  window->print_string(sbuf,SCALE(300),SCALE(150),1);
  window->swap_buffers();
  window->flush();

  XVPosition p;
  window->selectPoint(p,"white");
  paddle_hue = selector->findBestHue(subimage(image,p.PosX()-12,p.PosY()-12,24,24));
  cout << "paddle_hue -> (" << p.PosX() << "," << p.PosY() << ") " << paddle_hue << endl;
}


/**
  *  main routine!
  */
int main(int argc,char **argv)
{

  /* initialize the grabber */
  grabber=new XVBt8x8< XVImageRGB<XV_RGB> >();
  grabber->set_params("B2I1N0");
  if(!grabber)
  {
    cerr << "no framegrabber found, check your installation" << endl;
    exit(1);
  }

  /* set up the game parameters */
  memcpy(bricks,st_bricks,NUM_BRICKS*sizeof(Brick));

  /* set-up the grabber and open a window */
  image = grabber->next_frame_continuous();
  window = new PongWindow<XV_RGB>(&image,40,40);
  window->map();

  /* init color selection */
  selector=new PongSelector<XV_RGB,u_short>(180,75,175);

  /* get the paddle hue first to train the system */
  train_system();

  /* create the running thread and send it off to the races */
  /* when using pthread, the program halts after X seconds */
  //pthread_create(&work_thread,NULL,&task,NULL);
  //select(0,NULL,NULL,NULL,NULL);
  task(NULL);

  return 0;
}
