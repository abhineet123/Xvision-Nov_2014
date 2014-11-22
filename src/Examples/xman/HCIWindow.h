#ifndef __hciwindow_h
#define __hciwindow_h

#include "XVWindowX.h"
#include "XVBlobs.h"
#include "prototypes.h"
#include <vector>

//#include "XVColSelector.h"

#define BALL_SIZE 20

template <class T>
class HCIWindow:public XVWindowX<T>
{
 protected:
  using XVWindowX<T>::dpy ;
  using XVWindowX<T>::back_flag ;
  using XVWindowX<T>::back_buffer ;
  using XVWindowX<T>::window ;
  using XVWindowX<T>::flip ;
  using XVWindowX<T>::gc_window ;
  using XVWindowX<T>::width ;

  public:
    void DrawBall(int x,int y,int size);
    void ShowTeach(int x,int y);
    void ShowReg(vector<XVRectangleBlob> &regions,int x,int y,
	 float angle);
    	 HCIWindow(XVImageRGB<T> &image,int posx=0,int
	     posy=0,char *title=NULL,int event_mask=0,
	     char *display=NULL,int num_buf=2,
	     int double_buf=0);
    Display *get_display(void) {return dpy;};
    void    draw_object(HCIObject *object,int catched);
};
#endif
