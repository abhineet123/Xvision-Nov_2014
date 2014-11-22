// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVINTERACTIVE_H_
#define _XVINTERACTIVE_H_

#include <XVDrawable.h>

/** class providing methods for selecting items/regions of XVInteractWindows */
class XVInteractive {

 public:

  virtual void selectPoint(XVPosition &, 
			   XVDrawColor c = DEFAULT_COLOR,
			   bool draw_flag=true) = 0;
  virtual void selectLine(XVPosition &, 
			  XVPosition &, 
			  XVDrawColor c = DEFAULT_COLOR) = 0;
  virtual void selectRectangle(XVImageGeneric &, 
			       XVDrawColor c = DEFAULT_COLOR) = 0;
  virtual void selectEllipse(XVImageGeneric &, 
			     XVDrawColor c = DEFAULT_COLOR) = 0;

  virtual void selectSizedRect(XVImageGeneric &, const XVSize &, 
			       XVDrawColor c = DEFAULT_COLOR) = 0;

  virtual void selectAngledRect(XVPosition &, XVSize &, double &, 
				XVDrawColor c = DEFAULT_COLOR) = 0;

  virtual void selectAngledRect(XVPosition &, XVPosition &, XVSize &, 
				double &,XVDrawColor c =DEFAULT_COLOR)=0;
  virtual ~XVInteractive(){};
};

#endif

