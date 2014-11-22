/* 
 *  PongWindow.h
 */

#ifndef __PONGWINDOW_H
#define __PONGWINDOW_H

#include "XVColorSeg.h"
#include "XVImageRGB.h"
#include "XVWindowX.h"
/* jcorso 20010131 #include "XVColSelector.h" */

#define SCALE(x)      ((x))
#define BALL_SIZE    SCALE(20)
#define BRICK_SIZEX  SCALE(50)
#define BRICK_SIZEY  SCALE(20)


typedef struct tagBrick {
  int x;
  int y; 
  char* color;
  int value;
  int valid;
} Brick;
typedef Brick* tBrick;


/**
 * PongWindow
 *  subclass of XVInteractWindowX that handles the ball and the game!
 */
template <class T>
class PongWindow:public XVInteractWindowX<T>
{
 protected:
  using XVInteractWindowX<T>::flip ;
  using XVInteractWindowX<T>::width ;

 public:
  using XVInteractWindowX<T>::fillRectangle ;
  using XVInteractWindowX<T>::fillEllipse ;
  using XVInteractWindowX<T>::drawRectangle ;

  public:

    void drawBall(int x,int y,int size);
    void drawBricks(Brick *brick,int num_bricks);

   void showReg(int x,int y); 

   PongWindow(XVImageRGB<T> *im,int x,int y) : XVInteractWindowX<T>(*im,x,y,"Pong2",0,NULL,2,1,GXcopy){};

};


#endif
