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
#include "PongWindow.h"


template <class T>
void PongWindow<T>::drawBricks(Brick *brick,int count)
{
  int i;
  
 if(flip)
  for(i=0;i<count;i++)
  {
    if(!brick[i].valid) continue;
    fillRectangle(width-brick[i].x-SCALE(50),brick[i].y, BRICK_SIZEX,BRICK_SIZEY,brick[i].color); 
  }
 else
  for(i=0;i<count;i++)
  {
    if(!brick[i].valid) continue;
    fillRectangle(brick[i].x,brick[i].y,BRICK_SIZEX,BRICK_SIZEY,brick[i].color);
  }
 
}

template <class T>
void PongWindow<T>::drawBall(int x,int y,int size)
{
   fillEllipse(x,y,size,size,"red");
}




 
/*
 * draws the region are the ball 
 */
template <class T>
void PongWindow<T>::showReg(int x, int y)
{
   drawRectangle(x,y,BALL_SIZE,BALL_SIZE);
}


template class PongWindow<XV_RGB>;
