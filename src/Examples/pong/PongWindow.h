#include "XVWindow.h"
#include "XVColSelector.h"

#define BALL_SIZE 20

template <class T>
class PongWindow:public XVWindow<T>
{
  public:
    void DrawBall(int x,int y,int size);
    void ShowReg(Blob * &region,int count);
    	 PongWindow(XVImageBase<T> *image,int posx=0,int
	     posy=0,int event_mask=0, char *display=NULL,int num_buf=2);
};
