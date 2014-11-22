// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVDRAWABLE_H_
#define _XVDRAWABLE_H_

#include <XVImageBase.h>
#include <string.h>
#include <stdio.h>  

#include <string>

#define DEFAULT_COLOR (char*)"red"

/** color class for XVDrawObjects */
class XVDrawColor {

 protected:

  void getColorName(u_char r, u_char g, u_char b){
    sprintf(colorName, "rgb:%.2x/%.2x/%.2x", r, g, b);
  };

 public:

  char colorName[50];

  XVDrawColor(char * c)  { 
    memset(colorName, 0, 50);
    strcpy(colorName, c);
  };
  
#define _CONSTRUCT_FROM_PIXEL_(_XV_RGB_) \
  XVDrawColor(_XV_RGB_ v) { \
    getColorName(v.R(), v.G(), v.B()); \
  }
  
  _CONSTRUCT_FROM_PIXEL_(XV_RGB15);
  _CONSTRUCT_FROM_PIXEL_(XV_RGB16);
  _CONSTRUCT_FROM_PIXEL_(XV_RGB24);
  _CONSTRUCT_FROM_PIXEL_(XV_RGBA32);
  _CONSTRUCT_FROM_PIXEL_(XV_GLRGBA32);

  operator const string () { return colorName; }
};

// function defined in XVWindowX.cc

bool operator < (const XVDrawColor & c1, const XVDrawColor & c2);
bool operator == (const XVDrawColor & c1, const XVDrawColor & c2);

/** base class for drawable objects */
class XVDrawObject {

 public:
  
  XVDrawColor color;

  XVDrawObject(XVDrawColor c) : color(c) {}
  virtual ~XVDrawObject() {}
  
  virtual void draw() = 0;
};

/** class for drawing a point in XVWindows */
class XVDrawPoint : public XVDrawObject {

 public:

  XVPosition point;

  XVDrawPoint(XVPosition p, XVDrawColor c = DEFAULT_COLOR) : XVDrawObject(c), point(p) {}
  virtual ~XVDrawPoint() {}
};

/** class for drawing a line in XVWindows */
class XVDrawLine : public XVDrawObject {

 public:

  XVPosition point1;
  XVPosition point2;

  XVDrawLine(XVPosition p1, XVPosition p2, XVDrawColor c = DEFAULT_COLOR) : 
    XVDrawObject(c), point1(p1), point2(p2) {}
  virtual ~XVDrawLine() {}
};

/** class for drawing a rectangle in XVWindows */
class XVDrawRect : public XVDrawObject {
  
 public:

  bool fill;
  XVImageGeneric region;

  XVDrawRect(XVImageGeneric r, bool f, XVDrawColor c = DEFAULT_COLOR) : 
    XVDrawObject(c), fill(f), region(r) {}
  virtual ~XVDrawRect() {}
};

/** class for drawing an ellipse in XVWindows */
class XVDrawEllipse : public XVDrawObject {

 public:

  bool fill;
  XVImageGeneric region;

  XVDrawEllipse(XVImageGeneric r, bool f = false, XVDrawColor c = DEFAULT_COLOR) : 
    XVDrawObject(c), fill(f), region(r) {}
  virtual ~XVDrawEllipse() {}
};

/** class for drawing text in XVWindows */
class XVDrawString : public XVDrawObject {

 public:

  XVPosition pos;
  char * string;
  int length;

  XVDrawString(XVPosition p, char * s, int l, XVDrawColor c) : 
    XVDrawObject(c), pos(p), string(s), length(l) {}
  virtual ~XVDrawString() {}
};

/** provides methods for drawing/filling/coloring objects in XVDrawWindows */
class XVDrawable {

 public:

  inline int drawPoint(XVPosition p, 
		       XVDrawColor c = (char*)DEFAULT_COLOR){ 
    return this->drawPoint(p.PosX(), p.PosY()); 
  };
  
  virtual int drawPoint(int x, int y, 
			XVDrawColor c = (char*)DEFAULT_COLOR) = 0;

  inline int drawLine(XVPosition p1, XVPosition p2, 
	       XVDrawColor c = DEFAULT_COLOR){
    return this->drawLine(p1.PosX(), p1.PosY(), p2.PosX(), p2.PosY(), c);
  };
  
  virtual int drawLine(int x1, int y1, 
		       int x2, int y2, 
		       XVDrawColor c = (char*)DEFAULT_COLOR,int line_width=0) = 0;
  
  inline int drawRectangle(XVImageGeneric rect, 
			   XVDrawColor c = DEFAULT_COLOR){
    return this->drawRectangle(rect.PosX(), rect.PosY(), rect.Width(), rect.Height(), c);
  };

  inline int drawRectangle(XVPosition p, XVSize s, 
			   XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->drawRectangle(p.PosX(), p.PosY(), s.Width(), s.Height(), c);
  };

  virtual int drawRectangle(int x, int y, 
			    int w, int h, 
			    XVDrawColor c = (char*)DEFAULT_COLOR) = 0;
  
  inline int fillRectangle(XVImageGeneric rect, 
			   XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->fillRectangle(rect.PosX(), rect.PosY(), rect.Width(), rect.Height(), c);
  };

  inline int fillRectangle(XVPosition p, XVSize s, 
			   XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->fillRectangle(p.PosX(), p.PosY(), s.Width(), s.Height(), c);
  };
  
  virtual int fillRectangle(int x, int y, 
			    int w, int h, 
			    XVDrawColor c = (char*)DEFAULT_COLOR) = 0;
  
  inline int drawEllipse(XVImageGeneric rect, 
			 XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->drawEllipse(rect.PosX(), rect.PosY(), rect.Width(), rect.Height(), c);
  };

  inline int drawEllipse(XVPosition p, XVSize s, 
			 XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->drawEllipse(p.PosX(), p.PosY(), s.Width(), s.Height(), c);
  };
  
  virtual int drawEllipse(int x, int y, 
			  int w, int h, 
			  XVDrawColor c = (char*)DEFAULT_COLOR) = 0;
  
  inline int fillEllipse(XVImageGeneric rect, 
			 XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->fillEllipse(rect.PosX(), rect.PosY(), rect.Width(), rect.Height(), c);
  };
  
  inline int fillEllipse(XVPosition p, XVSize s, 
			 XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->fillEllipse(p.PosX(), p.PosY(), s.Width(), s.Height(), c);
  };
  
  virtual int fillEllipse(int x, int y, 
			  int w, int h, 
			  XVDrawColor c = (char*)DEFAULT_COLOR) = 0;
  
  inline int drawString(XVPosition p, char * string, 
			int length, XVDrawColor c = (char*)DEFAULT_COLOR){
    return this->drawString(p.PosX(), p.PosY(), string, length, c);
  };
  
  virtual int drawString(int x, int y, 
			 char * string, int length, 
			 XVDrawColor c = (char*)DEFAULT_COLOR) = 0;
  
  virtual void addColor(XVDrawColor) = 0;

  virtual void setXOR() = 0;
  virtual void setCOPY() = 0;
  virtual ~XVDrawable(){};
};

#endif
