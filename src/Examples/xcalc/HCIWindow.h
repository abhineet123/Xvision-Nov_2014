#ifndef __hciwindow_h
#define __hciwindow_h

#include "XVWindow.h"
#include "prototypes.h"
#include "XVColSelector.h"


#define RECT_OFFSET_X		145
#define RECT_OFFSET_Y		80
#define KEY_SIZE		60
#define KEY_DISTANCE		80

template <class T>
class HCIWindow:public XVWindow<T>
{
  public:
    void DrawBall(int x,int y,int size);
    void DrawCalc(double sum,char active);
    void ShowReg(Blob * region,int count,int x,int y,float angle);
    	 HCIWindow(XVImage<T> *image,int posx=0,int
	     posy=0,int event_mask=0, char *display=NULL,int num_buf=2,
	     int double_buf=0);
    Display *get_display(void) {return dpy;};
    void    draw_object(HCIObject *object);
#ifdef HAVE_LIBXDPMS
    Window get_window(void)  {return back_flag? back_buffer:window;};
#else
    Window get_window(void)  {return window;};
#endif
    GC	   *context(void) {return gc_window; };
};
#endif
