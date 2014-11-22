// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVPIXEL_H_
#define _XVPIXEL_H_

#include <sys/types.h>
#include <iostream>
#include <XVMacros.h>
#include <XVList.h>
#include <XVTools.h>
#include <cmath>

using namespace std ;

#define DEFAULT_BRIGHT_PIXEL 120
#define DEFAULT_DARK_PIXEL   20
#define NULL_HUE      (u_short)(0x1000)
#define NULL_BRIGHT   (u_short)(0x2000)
#define NULL_DARK     (u_short)(0x4000)

/** definitions of various pixel types */

// This form of typing is not perfect, but will do "for now"

typedef enum { XVImage_uchar  =  0,
	       XVImage_ushort =  1,
	       XVImage_uint   =  2,
	       XVImage_char   = 10,
	       XVImage_short  = 11,
	       XVImage_int    = 12,
	       XVImage_float  = 13,
	       XVImage_double = 14,
	       XVImage_RGB15  = 21,
	       XVImage_RGB16  = 22,
	       XVImage_RGB24  = 23,
	       XVImage_RGB32  = 24,
	       XVImage_GLRGBA32 = 25,
	       XVImage_TRGB24   = 26,
	       XVImage_YUV16  = 30,
	       XVImage_YUV24  = 31,
               XVImage_YUV422 = 33,
               XVImage_YBAND  = 41,
               XVImage_UBAND  = 42,
               XVImage_VBAND  = 43,
	       XVImage_UVBAND8  = 44,
               XVImage_UVBAND16 = 45,
               XVImage_HUE      = 51,
               XVImage_HSV24    = 52,
	       XVImage_YCbCr	= 53
	       } 
XVImage_Type;

inline bool XVImage_Signed_Type(XVImage_Type x) { return ((x/10) == 1); }
inline bool XVImage_RGB_Type(XVImage_Type x) { return (((int)(x/20)) == 1); }
inline bool XVImage_YUV_Type(XVImage_Type x) { return ((int)(x/30)) == 1; } 

// ambiguous with big and little endian :(

// RGB definitions

typedef struct{
  unsigned short b:5;
  unsigned short g:5;
  unsigned short r:5;
  unsigned short a:1;

  unsigned char R() const {return r << 3;};
  unsigned char G() const {return g << 3;};
  unsigned char B() const {return b << 3;};

  void setR(unsigned char v) {r = v >> 3;};
  void setG(unsigned char v) {g = v >> 3;};
  void setB(unsigned char v) {b = v >> 3;};


} XV_RGB15;

typedef struct
{
  unsigned short b:5;
  unsigned short g:6;
  unsigned short r:5;

  unsigned char R() const {return r << 3;};
  unsigned char G() const {return g << 2;};
  unsigned char B() const {return b << 3;};

  void setR(unsigned char v) {r = v >> 3;};
  void setG(unsigned char v) {g = v >> 2;};
  void setB(unsigned char v) {b = v >> 3;};

} XV_RGB16;

typedef struct
{
  unsigned char b;
  unsigned char g;
  unsigned char r;

  unsigned char R() const {return r;};
  unsigned char G() const {return g;};
  unsigned char B() const {return b;};

  void setR(unsigned char v) {r = v;}
  void setG(unsigned char v) {g = v;}
  void setB(unsigned char v) {b = v;}


} XV_RGB24;

typedef struct
{
  unsigned char r;
  unsigned char g;
  unsigned char b;

  unsigned char R() const {return r;};
  unsigned char G() const {return g;};
  unsigned char B() const {return b;};

  void setR(unsigned char v) {r = v;}
  void setG(unsigned char v) {g = v;}
  void setB(unsigned char v) {b = v;}


} XV_TRGB24;

typedef struct
{
  // Mon Sep 11 17:57:42 EDT 2000 -- Gingi
  // Changed byte order -- should be BGRA.
  unsigned char b;
  unsigned char g;
  unsigned char r;
  unsigned char a;


  unsigned char R() const {return r;};
  unsigned char G() const {return g;};
  unsigned char B() const {return b;};

  void setR(unsigned char v) {r = v;}
  void setG(unsigned char v) {g = v;}
  void setB(unsigned char v) {b = v;}

} XV_RGBA32;

/* added 27august2001 jcorso - reversed the byte order for display by gl*/
typedef struct
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;


  unsigned char R() const {return r;};
  unsigned char G() const {return g;};
  unsigned char B() const {return b;};

  void setR(unsigned char v) {r = v;}
  void setG(unsigned char v) {g = v;}
  void setB(unsigned char v) {b = v;}

} XV_GLRGBA32;

// test structure to verify the bitorder
typedef struct
{
  union
  {
    XV_RGB15 a;
    unsigned short val;
  }u;
} RGB15test;

// YUV definitions

typedef struct {
  
  unsigned short y:8;
  unsigned short u:4;
  unsigned short v:4;

  unsigned char Y() const {return y;};
  unsigned char U() const {return u;};
  unsigned char V() const {return v;};

  void setY(unsigned char a) {y = a;};
  void setU(unsigned char a) {u = a;};
  void setV(unsigned char a) {v = a;};

} XV_YUV16;

typedef struct
{
  unsigned char y;
  unsigned char u;
  unsigned char v;

  unsigned char Y() const {return y;};
  unsigned char U() const {return u;};
  unsigned char V() const {return v;};

  void setY(unsigned char a) {y = a;};
  void setU(unsigned char a) {u = a;};
  void setV(unsigned char a) {v = a;};

} XV_YUV24;

typedef struct
{
  unsigned char u;
  unsigned char y0;
  unsigned char v;
  unsigned char y1;
 
   
  void setY(unsigned char a) {y0 = a;};
  void setY2(unsigned char a) {y1 = a;};
  void setU(unsigned char a) {u = a;};
  void setV(unsigned char a) {v = a;};

  unsigned char U() const {return u;};
  unsigned char V() const {return v;};
  unsigned char Y0() const {return y0;};
  unsigned char Y1() const {return y1;};
} XV_YUV422;

typedef struct
{
  unsigned char y0;
  unsigned char u;
  unsigned char y1;
  unsigned char v;

  unsigned char U() const {return u;};
  unsigned char V() const {return v;};
  unsigned char Y0() const {return y0;};
  unsigned char Y1() const {return y1;};
} XV_YCbCr;

typedef u_char XV_YBAND;
typedef u_char XV_UBAND;

typedef u_char XV_YBAND;
typedef u_char XV_UBAND;
typedef u_char XV_VBAND;

typedef struct {

  unsigned short u:4;
  unsigned short v:4;

} XV_UVBAND8;

typedef struct {

  u_char u;
  u_char v;

  u_char U() const { return u; };
  u_char V() const { return v; };

  void setU(u_char a) { u = a; };
  void setV(u_char a) { v = a; };

} XV_UVBAND16;

// HSV definitions

typedef u_short XV_HUE;

typedef struct {

  u_char  h;
  u_char  s;
  u_char  v;
  
  u_char H() const { return h; };
  u_char S() const { return s; };
  u_char V() const { return v; };

  void setH(u_char val){ h = val; };
  void setS(u_char val){ s = val; };
  void setV(u_char val){ v = val; };
} XV_HSV24;


/*macro used to convert a YUV pixel to RGB format
  from Bart Nabbe
  note from Donald: this should probably change to a table look-up
  method and share with XVDig1394.cc
*/

// old version, conflict with YUVtoRGB
/*

#define YUV2R(y, u, v) (y + ((v * 1434) / 2048))
#define YUV2G(y, u, v) (y - ((u * 406) / 2048) - ((v * 595) / 2048))
#define YUV2B(y, u, v) (y + ((u * 2078) / 2048))

#define RGB2Y(r, g, b) ((0.257 * r) + (0.504 * g) + (0.098 * b) + 16)
#define RGB2U(r, g, b) ((0.439 * r) - (0.368 * g) - (0.071 * b) + 128)
#define RGB2V(r, g, b) (-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128)

*/

/* new version
   YUVtoRGB=[ 1 0       1.1402
              1 -0.3948 -0.5808
              1 2.0326  0 ]

   RGBtoYUV=[ 0.299   0.587     0.114
              -0.1471 -0.288804 0.4359
              0.6148  -0.5148   -0.1]    

   U and V should be offset by 128
*/          
                             
  
#define YUV2R(y, u, v) (y + (v-128)*9339/8192 )
#define YUV2G(y, u, v) (y - (u-128)*3228/8192 - (v-128)*4760/8192  )
#define YUV2B(y, u, v) (y + (u-128)*16646/8192 ) 

#define RGB2Y(r, g, b) ( r*2449/8192 + g*4809/8192 + b*934/8192 )
#define RGB2U(r, g, b) ( 128-r*1205/8192 - g*2366/8192 + b*3571/8192 )
#define RGB2V(r, g, b) ( 128+r*5036/8192 - g*4217/8192 - g*819 /8192  )


template <class T>
float RGBtoHue(T pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL, int darkPixel = DEFAULT_DARK_PIXEL){
  
  // normalizing r, g, b
  //T max = {255, 255, 255};

  float normalR = (float)pixel.R() / 255;
  float normalG = (float)pixel.G() / 255;
  float normalB = (float)pixel.B() / 255;

  int whichmax;
  float mx, mn;
  float hue=0,sat,delta;

  // find the max and min of r, g, and b
  if (normalR > normalG) {
    whichmax = 1;
    mx = normalR;
    mn = normalG;
  }
  else {
    whichmax = 2;
    mn = normalR;
    mx = normalG;
  }
  
  if (normalB < mn) {
    mn = normalB;
  }
  
  if (normalB > mx) {
    whichmax = 3;
    mx = normalB;
  }
  
  delta = (float)(mx - mn);
  
  // compute saturation
  if((mx + mn) > 1.0){
    sat = delta / (mx + mn);
  }else{
    sat = delta / (2 - mx - mn);
  }

  short NULL_RETVAL = 0;
  if((delta < 1e-9) || (sat < 0.005)) { NULL_RETVAL |= NULL_HUE; }
  if((mx * 255) < darkPixel)          { NULL_RETVAL |= NULL_DARK; }
  if((mn * 255) > brightPixel)        { NULL_RETVAL |= NULL_BRIGHT; }
  if(NULL_RETVAL) return (float)NULL_RETVAL;
  
  switch (whichmax) {
  case 1:
    hue = (normalG - normalB) / delta;
    break;
  case 2:
    hue = 2.0 + (normalB - normalR) / delta;
    break;
  case 3:
    hue = 4.0 + (normalR - normalG) / delta;
  }

  hue *= 60.0;
  if (hue < 0.0)
    hue += 360.0;

  return hue;
};

template <class T>
XV_HSV24 RGBtoHSV(T pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL,
		  int darkPixel = DEFAULT_DARK_PIXEL){

  // normalizing r, g, b
  //T max = {255, 255, 255};

  float normalR = (float)pixel.R() / 255;
  float normalG = (float)pixel.G() / 255;
  float normalB = (float)pixel.B() / 255;

  int whichmax;
  float mx, mn;
  float hue=0,sat,val;

  // find the max and min of r, g, and b
  if (normalR > normalG) {
    whichmax = 1;
    mx = normalR;
    mn = normalG;
  }
  else {
    whichmax = 2;
    mn = normalR;
    mx = normalG;
  }
  
  if (normalB < mn) {
    mn = normalB;
  }
  
  if (normalB > mx) {
    whichmax = 3;
    mx = normalB;
  }
 
  val = mx;
  float delta = mx - mn;
  sat = (mx != 0) ? ((float)delta) / mx : 0.0;
  
  if((delta < 1e-9) || (sat < 0.005) || (((mx * 255) < darkPixel) || ((mn * 255) > brightPixel)))
    {
      XV_HSV24 pv; pv.h = (u_char)NULL_HUE; pv.s = 0; pv.v = 0;
    }
  
  switch (whichmax) {
  case 1:
    hue = (normalG - normalB) / delta;
    break;
  case 2:
    hue = 2.0 + (normalB - normalR) / delta;
    break;
  case 3:
    hue = 4.0 + (normalR - normalG) / delta;
  }

  hue *= 60.0;
  if (hue < 0.0)
    hue += 360.0;

  XV_HSV24 pix; 
  pix.h = (u_char)((hue / 0.694) + 0.5); 
  pix.s = (u_char)(sat * 255);
  pix.v = (u_char)(val * 255);

  return pix;
}; 

template <class T>
T HSVtoRGB(const XV_HSV24 & pixel){

  if(pixel.s == 0){
    T pv; pv.setR(pixel.v); pv.setG(pixel.v); pv.setB(pixel.v);
    return pv;
  }else{
    double h, s, v, f, p, q, t;
    int i;
    
    h = pixel.h * 0.024;
    s = pixel.s / 255;
    v = pixel.v / 255;
    i = (int)floor(h);
    f = h - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));
    T rp;
    switch(i){
    case 0: 
      rp.setR(pixel.v); 
      rp.setG((u_char)(t * 255)); 
      rp.setB((u_char)(p * 255));
      break;
    case 1: 
      rp.setR((u_char)(q * 255)); 
      rp.setG(pixel.v); 
      rp.setB((u_char)(p * 255));
      break;
    case 2: 
      rp.setR((u_char)(p * 255)); 
      rp.setG(pixel.v); 
      rp.setB((u_char)(t * 255));
      break;
    case 3: 
      rp.setR((u_char)(p * 255)); 
      rp.setG((u_char)(q * 255)); 
      rp.setB(pixel.v);
      break;
    case 4: 
      rp.setR((u_char)(t * 255)); 
      rp.setG((u_char)(p * 255)); 
      rp.setB(pixel.v);
      break;
    case 5: 
      rp.setR(pixel.v); 
      rp.setG((u_char)(p * 255)); 
      rp.setB((u_char)(q * 255));
      break;
    };
    return rp;
  }
};

XV_YUV24 HSVtoYUV(XV_HSV24 p);

float YUVtoHue(XV_YUV24 pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL, 
	       int darkPixel = DEFAULT_DARK_PIXEL);

float YUV422toHue(XV_YUV422 pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL, 
		  int darkPixel = DEFAULT_DARK_PIXEL);

float YCbCrtoHue(XV_YCbCr pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL, 
		  int darkPixel = DEFAULT_DARK_PIXEL);

XV_HSV24 YUVtoHSV(XV_YUV24 pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL,
		  int darkPixel = DEFAULT_DARK_PIXEL);

XV_HSV24 YUV422toHSV(XV_YUV422 pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL,
		     int darkPixel = DEFAULT_DARK_PIXEL);
    
XV_HSV24 YCbCrtoHSV(XV_YCbCr pixel, int brightPixel = DEFAULT_BRIGHT_PIXEL,
		     int darkPixel = DEFAULT_DARK_PIXEL);
    
// conversion functions

inline void operator <<  (XV_RGB15 & pout, const XV_RGB15 & pin) {

  pout = const_cast<XV_RGB15 &>(pin);
};

inline void operator <<  (XV_RGB15 & pout, const XV_RGB16 & pin) {

  pout.r = pin.r;
  pout.g = pin.g >> 1;
  pout.b = pin.b;
};

inline void operator <<  (XV_RGB15 & pout, const XV_RGB24 & pin) {
  
  pout.r = pin.r >> 3;
  pout.g = pin.g >> 3;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB15 & pout, const XV_TRGB24 & pin) {
  
  pout.r = pin.r >> 3;
  pout.g = pin.g >> 3;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB15 & pout, const XV_RGBA32 & pin) {

  pout.r = pin.r >> 3;
  pout.g = pin.g >> 3;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB15 & pout, const XV_GLRGBA32 & pin) {

  pout.r = pin.r >> 3;
  pout.g = pin.g >> 3;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB15 & pout, const XV_YUV16 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v)) >> 3;
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v)) >> 3;
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB15 & pout, const XV_YUV24 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v)) >> 3;
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v)) >> 3;
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB15 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v)) >> 3;
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB15 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v)) >> 3;
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_RGB15 & pin) {

  pout.r = pin.r;
  pout.g = pin.g << 1;
  pout.b = pin.b;
};

inline void operator <<  (XV_RGB16 & pout, const XV_RGB16 & pin) {

  pout = const_cast<XV_RGB16 &>(pin);
};

inline void operator <<  (XV_RGB16 & pout, const XV_RGB24 & pin) {
  
  pout.r = pin.r >> 3;
  pout.g = pin.g >> 2;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_TRGB24 & pin) {
  
  pout.r = pin.r >> 3;
  pout.g = pin.g >> 2;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_RGBA32 & pin) {

  pout.r = pin.r >> 3;
  pout.g = pin.g >> 2;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_GLRGBA32 & pin) {

  pout.r = pin.r >> 3;
  pout.g = pin.g >> 2;
  pout.b = pin.b >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_YUV16 & pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v)) >> 3;
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v)) >> 2;
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB16 & pout, const XV_YUV24 & pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v)) >> 3;
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v)) >> 2;
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB16 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v)) >> 2;
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v)) >> 3;
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v)) >> 2;
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB16 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v)) >> 3;
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v)) >> 2;
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v)) >> 3;
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v)) >> 3;
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v)) >> 2;
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v)) >> 3;
};

inline void operator <<  (XV_RGB24 & pout, const XV_RGB15 & pin) {

  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_RGB24 & pout, const XV_RGB16 & pin) {
  
  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_RGB24 & pout, const XV_RGB24 & pin) {
  
  pout = const_cast<XV_RGB24 &>(pin);
};

inline void operator <<  (XV_RGB24 & pout, const XV_TRGB24 & pin) {

  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
  
};

inline void operator <<  (XV_RGB24 & pout, const XV_RGBA32 & pin) {

  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
};

inline void operator <<  (XV_RGB24 & pout, const XV_GLRGBA32 & pin) {

  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
};

inline void operator <<  (XV_RGB24 & pout, const XV_YUV16 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_RGB24 & pout, const XV_YUV24 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_RGB24 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_RGB24 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_TRGB24 & pout, const XV_RGB15 & pin) {

  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_TRGB24 & pout, const XV_RGB16 & pin) {
  
  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_TRGB24 & pout, const XV_RGB24 & pin) {
  
  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
};

inline void operator <<  (XV_TRGB24 & pout, const XV_TRGB24 & pin) {
  
  pout = const_cast<XV_TRGB24 &>(pin);
};

inline void operator <<  (XV_TRGB24 & pout, const XV_RGBA32 & pin) {

  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
};

inline void operator <<  (XV_TRGB24 & pout, const XV_GLRGBA32 & pin) {

  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
};

inline void operator <<  (XV_TRGB24 & pout, const XV_YUV16 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_TRGB24 & pout, const XV_YUV24 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_TRGB24 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_TRGB24 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_RGBA32 & pout, const XV_RGB15 & pin) {

  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_RGB15 & pin) {

  pout.r = pin.r << 3;
  pout.g = pin.g << 3;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_RGBA32 & pout, const XV_RGB16 & pin) {
  
  pout.r = pin.r << 3;
  pout.g = pin.g << 2;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_RGB16 & pin) {
  
  pout.r = pin.r << 3;
  pout.g = pin.g << 2;
  pout.b = pin.b << 3;
};

inline void operator <<  (XV_RGBA32 & pout, const XV_RGB24 & pin) {

  pout.r = pin.r; pout.g = pin.g; pout.b = pin.b;
};

inline void operator <<  (XV_RGBA32 & pout, const XV_TRGB24 & pin) {

  pout.r = pin.r; pout.g = pin.g; pout.b = pin.b;
};


inline void operator <<  (XV_RGBA32 & pout, const XV_RGBA32 & pin) {

  pout = const_cast<XV_RGBA32 &>(pin);
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_TRGB24 & pin) {
  pout.r = pin.r; pout.g = pin.g; pout.b = pin.b;

};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_RGB24 & pin) {
  pout.r = pin.r; pout.g = pin.g; pout.b = pin.b;

};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_GLRGBA32 & pin) {

  pout = const_cast<XV_GLRGBA32 &>(pin);
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_RGBA32 & pin) {
  
  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
  pout.a = pin.a;
};

inline void operator <<  (XV_RGBA32 & pout, const XV_GLRGBA32 & pin) {
  
  pout.r = pin.r;
  pout.g = pin.g;
  pout.b = pin.b;
  pout.a = pin.a;
};

inline void operator <<  (XV_RGBA32 & pout, const XV_YUV16 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_RGBA32 & pout, const XV_YUV24 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_YUV16 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_GLRGBA32 & pout, const XV_YUV24 &  pin) {

  pout.r = (u_char)(YUV2R(pin.y, pin.u, pin.v));
  pout.g = (u_char)(YUV2G(pin.y, pin.u, pin.v));
  pout.b = (u_char)(YUV2B(pin.y, pin.u, pin.v));
};

inline void operator <<  (XV_RGBA32 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_GLRGBA32 pout[2], const XV_YUV422 & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_RGBA32 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

inline void operator <<  (XV_GLRGBA32 pout[2], const XV_YCbCr & pin) {

  pout[0].r = (u_char)(YUV2R(pin.y0, pin.u, pin.v));
  pout[0].g = (u_char)(YUV2G(pin.y0, pin.u, pin.v));
  pout[0].b = (u_char)(YUV2B(pin.y0, pin.u, pin.v));
  pout[1].r = (u_char)(YUV2R(pin.y1, pin.u, pin.v));
  pout[1].g = (u_char)(YUV2G(pin.y1, pin.u, pin.v));
  pout[1].b = (u_char)(YUV2B(pin.y1, pin.u, pin.v));
};

#define HSV_TO_RGB(RGB_TYPE) \
inline void operator <<  (RGB_TYPE & pout, const XV_HSV24 & pin) { \
\
  RGB_TYPE tmp = HSVtoRGB<RGB_TYPE>(pin); \
  pout << tmp; \
}; 

HSV_TO_RGB(XV_RGB15);
HSV_TO_RGB(XV_RGB16);
HSV_TO_RGB(XV_RGB24);
HSV_TO_RGB(XV_TRGB24);
HSV_TO_RGB(XV_RGBA32);
HSV_TO_RGB(XV_GLRGBA32);

#define RGB_TO_YUV16(PIX_TYPE) \
inline void operator <<  (XV_YUV16 & pout, const PIX_TYPE & pin){ \
\
  pout.y = (u_char)(RGB2Y(pin.R(), pin.G(), pin.B())); \
  pout.u = (u_char)(RGB2U(pin.R(), pin.G(), pin.B())); \
  pout.v = (u_char)(RGB2V(pin.R(), pin.G(), pin.B())); \
};
 
RGB_TO_YUV16(XV_RGB15);
RGB_TO_YUV16(XV_RGB16);
RGB_TO_YUV16(XV_RGB24);
RGB_TO_YUV16(XV_TRGB24);
RGB_TO_YUV16(XV_RGBA32);
RGB_TO_YUV16(XV_GLRGBA32);

#define RGB_TO_YUV24(PIX_TYPE) \
inline void operator <<  (XV_YUV24 & pout, const PIX_TYPE & pin){ \
\
  pout.y = (u_char)(RGB2Y(pin.R(), pin.G(), pin.B())); \
  pout.u = (u_char)(RGB2U(pin.R(), pin.G(), pin.B())); \
  pout.v = (u_char)(RGB2V(pin.R(), pin.G(), pin.B())); \
};
 
RGB_TO_YUV24(XV_RGB15);
RGB_TO_YUV24(XV_RGB16);
RGB_TO_YUV24(XV_RGB24);
RGB_TO_YUV24(XV_TRGB24);
RGB_TO_YUV24(XV_RGBA32);
RGB_TO_YUV24(XV_GLRGBA32);

inline void operator <<  (XV_YUV24 & pout, const XV_HSV24 & pin){

  pout = HSVtoYUV(pin);
};

inline void operator <<  (XV_YUV24 pout[2], const XV_YUV422 & pin) {
  pout[0].y = pin.y0;
  pout[0].u = pin.u;
  pout[0].v = pin.v;
  pout[1].y = pin.y1;
  pout[1].u = pin.u;
  pout[1].v = pin.v;
};

inline void operator <<  (XV_YUV24 pout[2], const XV_YCbCr & pin) {
  pout[0].y = pin.y0;
  pout[0].u = pin.u;
  pout[0].v = pin.v;
  pout[1].y = pin.y1;
  pout[1].u = pin.u;
  pout[1].v = pin.v;
};

inline void operator <<  (XV_YUV24 & pout, const XV_YUV24 & pin) {
  pout = pin;
};

inline void operator <<  (XV_YUV422 & pout, const XV_YUV24 pin[2]) {
  pout.y0 = pin[0].y;
  pout.u  = pin[0].u;
  pout.y1 = pin[1].y;
  pout.v  = pin[1].v;
};

inline void operator <<  (XV_YCbCr & pout, const XV_YUV24 pin[2]) {
  pout.y0 = pin[0].y;
  pout.u  = pin[0].u;
  pout.y1 = pin[1].y;
  pout.v  = pin[1].v;
};

#define RGB_TO_YUV422(RGB_TYPE) \
inline void operator <<  (XV_YUV422 & pout, const RGB_TYPE pin[2]) { \
  XV_YUV24 tmp[2]; \
  tmp[0] << pin[0]; \
  tmp[1] << pin[1]; \
  pout << tmp; \
};

RGB_TO_YUV422(XV_RGB15);
RGB_TO_YUV422(XV_RGB16);
RGB_TO_YUV422(XV_RGB24);
RGB_TO_YUV422(XV_TRGB24);
RGB_TO_YUV422(XV_RGBA32);
RGB_TO_YUV422(XV_GLRGBA32);

#define RGB_TO_YCbCr(RGB_TYPE) \
inline void operator <<  (XV_YCbCr & pout, const RGB_TYPE pin[2]) { \
  pout.y0 = (u_char)(RGB2Y(pin[0].R(), pin[0].G(), pin[0].B())); \
  pout.u = (u_char)(RGB2U(pin[0].R(), pin[0].G(), pin[0].B())); \
  pout.v = (u_char)(RGB2V(pin[0].R(), pin[0].G(), pin[0].B())); \
  pout.y1 = (u_char)(RGB2Y(pin[1].R(), pin[1].G(), pin[1].B())); \
};

RGB_TO_YCbCr(XV_RGB15);
RGB_TO_YCbCr(XV_RGB16);
RGB_TO_YCbCr(XV_RGB24);
RGB_TO_YCbCr(XV_TRGB24);
RGB_TO_YCbCr(XV_RGBA32);
RGB_TO_YCbCr(XV_GLRGBA32);

inline void operator <<  (XV_YUV422 & pout, const XV_HSV24 pin[2]) {
  XV_YUV24 tmp[2]; tmp[0] << pin[0]; tmp[1] << pin[1];
  pout << tmp;
};

inline void operator <<  (XV_YCbCr & pout, const XV_HSV24 pin[2]) {
  XV_YUV24 tmp[2]; tmp[0] << pin[0]; tmp[1] << pin[1];
  pout << tmp;
};
inline void operator <<  (XV_YUV422 & pout, const XV_YUV422 & pin) {
  pout  = pin;
};

inline void operator <<  (XV_YUV422 & pout, const XV_YCbCr & pin) {
  pout.y0  = pin.y0;
  pout.y1  = pin.y1;
  pout.u  = pin.u;
  pout.v  = pin.v;
};

inline void operator <<  (XV_YCbCr & pout, const XV_YUV422 & pin) {
  pout.y0  = pin.y0;
  pout.y1  = pin.y1;
  pout.u  = pin.u;
  pout.v  = pin.v;
};

inline void operator <<  (XV_YCbCr & pout, const XV_YCbCr & pin) {
  pout  = pin;
};

#define RGB_TO_HSV(RGB_TYPE) \
inline void operator <<  (XV_HSV24 & pout, const RGB_TYPE & pin) { \
  pout = RGBtoHSV(pin); \
};

RGB_TO_HSV(XV_RGB15);
RGB_TO_HSV(XV_RGB16);
RGB_TO_HSV(XV_RGB24);
RGB_TO_HSV(XV_TRGB24);
RGB_TO_HSV(XV_RGBA32);
RGB_TO_HSV(XV_GLRGBA32);

inline void operator <<  (XV_HSV24 & pout, const XV_YUV24 & pin) {
  pout = YUVtoHSV(pin);
};

inline void operator <<  (XV_HSV24 & pout, const XV_YUV422 & pin) {
  pout = YUV422toHSV(pin);
};

inline void operator <<  (XV_HSV24 & pout, const XV_YCbCr & pin) {
  pout = YCbCrtoHSV(pin);
};

inline void operator <<  (XV_HSV24 & pout, const XV_HSV24 & pin) {
  pout = pin;
};

#define RGB_TO_SCALAR(RGB_TYPE, SCALAR_TYPE) \
inline void operator << (SCALAR_TYPE & scalar, const RGB_TYPE & rgb) { \
  scalar = (SCALAR_TYPE)((rgb.R() + rgb.G() + rgb.B()) / 3); \
};

#define SCALAR_TO_RGB(SCALAR_TYPE, RGB_TYPE) \
inline void operator << (RGB_TYPE & rgb, const SCALAR_TYPE & scalar) { \
  rgb.setR((u_char)scalar); \
  rgb.setG((u_char)scalar); \
  rgb.setB((u_char)scalar); \
};

#define _RGB_TO_SCALAR_ALL_SC_(_RGB_TYPE_) \
RGB_TO_SCALAR(_RGB_TYPE_, u_char); \
SCALAR_TO_RGB(u_char, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, char); \
SCALAR_TO_RGB(char, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, u_short); \
SCALAR_TO_RGB(u_short, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, short); \
SCALAR_TO_RGB(short, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, u_int); \
SCALAR_TO_RGB(u_int, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, int); \
SCALAR_TO_RGB(int, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, float); \
SCALAR_TO_RGB(float, _RGB_TYPE_); \
RGB_TO_SCALAR(_RGB_TYPE_, double); \
SCALAR_TO_RGB(double, _RGB_TYPE_);

_RGB_TO_SCALAR_ALL_SC_(XV_RGB15);
_RGB_TO_SCALAR_ALL_SC_(XV_RGB16);
_RGB_TO_SCALAR_ALL_SC_(XV_RGB24);
_RGB_TO_SCALAR_ALL_SC_(XV_TRGB24);
_RGB_TO_SCALAR_ALL_SC_(XV_RGBA32);
_RGB_TO_SCALAR_ALL_SC_(XV_GLRGBA32);

#define YUV_TO_SCALAR(YUV_TYPE, SCALAR_TYPE) \
inline void operator << (SCALAR_TYPE & scalar, const YUV_TYPE & yuv) { \
  scalar = (SCALAR_TYPE)(yuv.y); \
};

#define SCALAR_TO_YUV(SCALAR_TYPE, YUV_TYPE) \
inline void operator << (YUV_TYPE & yuv, const SCALAR_TYPE & scalar) { \
  yuv.setY((u_char)scalar); yuv.setU(0); yuv.setV(0); \
};

#define _YUV_TO_SCALAR_SC_(_YUV_TYPE_) \
YUV_TO_SCALAR(_YUV_TYPE_, u_char); \
SCALAR_TO_YUV(u_char, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, char); \
SCALAR_TO_YUV(char, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, u_short); \
SCALAR_TO_YUV(u_short, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, short); \
SCALAR_TO_YUV(short, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, u_int); \
SCALAR_TO_YUV(u_int, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, int); \
SCALAR_TO_YUV(int, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, float); \
SCALAR_TO_YUV(float, _YUV_TYPE_); \
YUV_TO_SCALAR(_YUV_TYPE_, double); \
SCALAR_TO_YUV(double, _YUV_TYPE_);

_YUV_TO_SCALAR_SC_(XV_YUV24);

#define HSV_TO_SCALAR(HSV_TYPE, SCALAR_TYPE) \
inline void operator << (SCALAR_TYPE & scalar, const HSV_TYPE & hsv) { \
  scalar = (SCALAR_TYPE)hsv.v; \
};

#define SCALAR_TO_HSV(SCALAR_TYPE, HSV_TYPE) \
inline void operator << (HSV_TYPE & hsv, const SCALAR_TYPE & scalar) { \
  hsv.v = (u_char)scalar; hsv.h = 0; hsv.s = 0; \
};

#define _HSV_TO_SCALAR_SC_(_HSV_TYPE_) \
HSV_TO_SCALAR(_HSV_TYPE_, u_char); \
SCALAR_TO_HSV(u_char, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, char); \
SCALAR_TO_HSV(char, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, u_short); \
SCALAR_TO_HSV(u_short, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, short); \
SCALAR_TO_HSV(short, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, u_int); \
SCALAR_TO_HSV(u_int, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, int); \
SCALAR_TO_HSV(int, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, float); \
SCALAR_TO_HSV(float, _HSV_TYPE_); \
HSV_TO_SCALAR(_HSV_TYPE_, double); \
SCALAR_TO_HSV(double, _HSV_TYPE_);

_HSV_TO_SCALAR_SC_(XV_HSV24);

#define YUV422_TO_SCALAR(SCALAR) \
inline void operator << (SCALAR sc[2], const XV_YUV422 & yuv) { \
  sc[0] = yuv.y0; sc[1] = yuv.y1; \
};

#define SCALAR_TO_YUV422(SCALAR) \
inline void operator << (XV_YUV422 & yuv, const SCALAR sc[2]) { \
  yuv.y0 = (u_char)sc[0]; yuv.y1 = (u_char)sc[1]; yuv.u = 0; yuv.v = 0; \
};

YUV422_TO_SCALAR(u_char);
SCALAR_TO_YUV422(u_char);
YUV422_TO_SCALAR(char);
SCALAR_TO_YUV422(char);
YUV422_TO_SCALAR(u_short);
SCALAR_TO_YUV422(u_short);
YUV422_TO_SCALAR(short);
SCALAR_TO_YUV422(short);
YUV422_TO_SCALAR(u_int);
SCALAR_TO_YUV422(u_int);
YUV422_TO_SCALAR(int);
SCALAR_TO_YUV422(int);
YUV422_TO_SCALAR(float);
SCALAR_TO_YUV422(float);
YUV422_TO_SCALAR(double);
SCALAR_TO_YUV422(double);

#define YCbCr_TO_SCALAR(SCALAR) \
inline void operator << (SCALAR sc[2], const XV_YCbCr & yuv) { \
  sc[0] = yuv.y0; sc[1] = yuv.y1; \
};

#define SCALAR_TO_YCbCr(SCALAR) \
inline void operator << (XV_YCbCr & yuv, const SCALAR sc[2]) { \
  yuv.y0 = (u_char)sc[0]; yuv.y1 = (u_char)sc[1]; yuv.u = 0; yuv.v = 0; \
};

YCbCr_TO_SCALAR(u_char);
SCALAR_TO_YCbCr(u_char);
YCbCr_TO_SCALAR(char);
SCALAR_TO_YCbCr(char);
YCbCr_TO_SCALAR(u_short);
SCALAR_TO_YCbCr(u_short);
YCbCr_TO_SCALAR(short);
SCALAR_TO_YCbCr(short);
YCbCr_TO_SCALAR(u_int);
SCALAR_TO_YCbCr(u_int);
YCbCr_TO_SCALAR(int);
SCALAR_TO_YCbCr(int);
YCbCr_TO_SCALAR(float);
SCALAR_TO_YCbCr(float);
YCbCr_TO_SCALAR(double);
SCALAR_TO_YCbCr(double);

// Type Determination Functions.  Overloaded to include all types.

inline XVImage_Type figure_out_type(XV_RGB16 x) { return XVImage_RGB16; }
inline XVImage_Type figure_out_type(XV_RGB15 x) { return XVImage_RGB15; }
inline XVImage_Type figure_out_type(XV_RGB24 x) { return XVImage_RGB24; }
inline XVImage_Type figure_out_type(XV_TRGB24 x) { return XVImage_TRGB24; }
inline XVImage_Type figure_out_type(XV_RGBA32 x){ return XVImage_RGB32; }
inline XVImage_Type figure_out_type(XV_GLRGBA32 x){ return XVImage_GLRGBA32; }
inline XVImage_Type figure_out_type(XV_YUV16 x) { return XVImage_YUV16; }
inline XVImage_Type figure_out_type(XV_YUV24 x) { return XVImage_YUV24; }
inline XVImage_Type figure_out_type(XV_YUV422 x){ return XVImage_YUV422;}
inline XVImage_Type figure_out_type(XV_UVBAND16 x) { return XVImage_UVBAND16; }
inline XVImage_Type figure_out_type(XV_HSV24 x) { return XVImage_HSV24; }
inline XVImage_Type figure_out_type(u_char x)   { return XVImage_char;  }
inline XVImage_Type figure_out_type(u_short x)  { return XVImage_ushort;}
inline XVImage_Type figure_out_type(u_int x)    { return XVImage_int;   }
inline XVImage_Type figure_out_type(u_long x)    { return XVImage_int; }
inline XVImage_Type figure_out_type(int x)    { return XVImage_int;   }
inline XVImage_Type figure_out_type(long x) { return XVImage_int; }
inline XVImage_Type figure_out_type(float x)    { return XVImage_float; }
inline XVImage_Type figure_out_type(double x)   { return XVImage_double;}
inline XVImage_Type figure_out_type(XV_YCbCr x){ return XVImage_YCbCr;}

#define PRINT_RGB_PIXEL(PIX) \
inline ostream &operator << (ostream & output, const PIX & RGB) { \
  output.width(3); \
  output<<'['; \
  output.width(3); \
  output<<static_cast<int>(RGB.r)<<','; \
  output.width(3); \
  output<<static_cast<int>(RGB.g)<<','; \
  output.width(3); \
  output<<static_cast<int>(RGB.b)<<']'; \
  return output; \
};

PRINT_RGB_PIXEL(XV_RGB15);
PRINT_RGB_PIXEL(XV_RGB16);
PRINT_RGB_PIXEL(XV_RGB24);
PRINT_RGB_PIXEL(XV_TRGB24);
PRINT_RGB_PIXEL(XV_RGBA32);

#define RGB_DOT_PROD(PIX) \
inline int dot(const PIX & p1, const PIX & p2){ \
  return (p1.r * p2.r + p1.g * p2.g + p1.b * p2.b); \
};

RGB_DOT_PROD(XV_RGB15);
RGB_DOT_PROD(XV_RGB16);
RGB_DOT_PROD(XV_RGB24);
RGB_DOT_PROD(XV_TRGB24);
RGB_DOT_PROD(XV_RGBA32);
RGB_DOT_PROD(XV_GLRGBA32);

#define RGB_CROSS_PROD(PIX) \
inline PIX cross(const PIX & p1, const PIX & p2){ \
  PIX prod = {(p1.g * p2.b - p1.b * p2.g), \
	    (p1.r * p2.b - p1.b * p2.r),  \
	    (p1.r * p2.g - p1.g * p2.r)}; \
  return prod; \
};

RGB_CROSS_PROD(XV_RGB15);
RGB_CROSS_PROD(XV_RGB16);
RGB_CROSS_PROD(XV_RGB24);
RGB_CROSS_PROD(XV_TRGB24);
RGB_CROSS_PROD(XV_RGBA32);
RGB_CROSS_PROD(XV_GLRGBA32);

#define RGB_LENGTH(PIX) \
inline double length(const PIX & p){ \
  return sqrt((double)sqr(p.r) + sqr(p.g) + sqr(p.b)); \
}; 

RGB_LENGTH(XV_RGB15);
RGB_LENGTH(XV_RGB16);
RGB_LENGTH(XV_RGB24);
RGB_LENGTH(XV_TRGB24);
RGB_LENGTH(XV_RGBA32);
RGB_LENGTH(XV_GLRGBA32);

#define MAKE_RGB_PIXEL_OP(OP, PIX) \
inline PIX operator OP (const PIX & p1, const PIX & p2){ \
  PIX result; \
  result.r = p1.r OP p2.r; result.g = p1.g OP p2.g; result.b = p1.b OP p2.b; \
  return result; \
}; 

#define MAKE_RGB_PIXEL_MODIFY_OP(OP, PIX) \
inline PIX operator OP (PIX & p1, const PIX & p2){ \
  p1.r OP p2.r;  p1.g OP p2.g; p1.b OP p2.b; \
  return p1; \
}; 

#define MAKE_RGB_PIXEL_CONST_SCALAR_OP(OP, PIX) \
inline PIX operator OP (const PIX & p1, int val) { \
  PIX result;  result.r = p1.r OP val;  result.g = p1.g OP val; result.b = p1.b OP val; \
 return result; \
};

#define MAKE_RGB_PIXEL_SCALAR_OP(OP, PIX) \
inline PIX & operator OP (PIX & p1, int val){ \
  p1.r OP val;  p1.g OP val;  p1.b OP val; \
  return p1; \
};

MAKE_RGB_PIXEL_OP(+, XV_RGB15);
MAKE_RGB_PIXEL_OP(-, XV_RGB15);
MAKE_RGB_PIXEL_OP(*, XV_RGB15);
MAKE_RGB_PIXEL_OP(/, XV_RGB15);
MAKE_RGB_PIXEL_OP(+, XV_RGB16);
MAKE_RGB_PIXEL_OP(-, XV_RGB16);
MAKE_RGB_PIXEL_OP(*, XV_RGB16);
MAKE_RGB_PIXEL_OP(/, XV_RGB16);
MAKE_RGB_PIXEL_OP(+, XV_RGB24);
MAKE_RGB_PIXEL_OP(-, XV_RGB24);
MAKE_RGB_PIXEL_OP(*, XV_RGB24);
MAKE_RGB_PIXEL_OP(/, XV_RGB24);
MAKE_RGB_PIXEL_OP(+, XV_TRGB24);
MAKE_RGB_PIXEL_OP(-, XV_TRGB24);
MAKE_RGB_PIXEL_OP(*, XV_TRGB24);
MAKE_RGB_PIXEL_OP(/, XV_TRGB24);
MAKE_RGB_PIXEL_OP(+, XV_RGBA32);
MAKE_RGB_PIXEL_OP(-, XV_RGBA32);
MAKE_RGB_PIXEL_OP(*, XV_RGBA32);
MAKE_RGB_PIXEL_OP(/, XV_RGBA32);
MAKE_RGB_PIXEL_OP(+, XV_GLRGBA32);
MAKE_RGB_PIXEL_OP(-, XV_GLRGBA32);
MAKE_RGB_PIXEL_OP(*, XV_GLRGBA32);
MAKE_RGB_PIXEL_OP(/, XV_GLRGBA32);

MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_RGB15);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_RGB15);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_RGB15);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_RGB15);
MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_RGB16);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_RGB16);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_RGB16);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_RGB16);
MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_RGB24);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_RGB24);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_RGB24);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_RGB24);
MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_TRGB24);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_TRGB24);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_TRGB24);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_TRGB24);
MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_RGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_RGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_RGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_RGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(+=, XV_GLRGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(-=, XV_GLRGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(*=, XV_GLRGBA32);
MAKE_RGB_PIXEL_MODIFY_OP(/=, XV_GLRGBA32);

MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_RGB15);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_RGB15);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_RGB15);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_RGB15);
MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_RGB16);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_RGB16);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_RGB16);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_RGB16);
MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_RGB24);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_RGB24);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_RGB24);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_RGB24);
MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_TRGB24);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_TRGB24);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_TRGB24);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_TRGB24);
MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_RGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_RGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_RGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_RGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(+=, XV_GLRGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(-=, XV_GLRGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(*=, XV_GLRGBA32);
MAKE_RGB_PIXEL_SCALAR_OP(/=, XV_GLRGBA32);

MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_RGB15);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_RGB15);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_RGB15);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_RGB15);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_RGB16);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_RGB16);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_RGB16);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_RGB16);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_RGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_RGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_RGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_RGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_TRGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_TRGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_TRGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_TRGB24);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_RGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_RGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_RGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_RGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(+, XV_GLRGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(-, XV_GLRGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(*, XV_GLRGBA32);
MAKE_RGB_PIXEL_CONST_SCALAR_OP(/, XV_GLRGBA32);

#endif
