
#include <XVPixel.h>

float YUVtoHue(XV_YUV24 pixel, int brightPixel, 
	       int darkPixel){
  
  if (pixel.Y() > brightPixel || pixel.Y() < darkPixel)
    return NULL_HUE;
  return (M_PI / 180) * (atan2((float)pixel.U(), (float) pixel.V()));
};


float YUV422toHue(XV_YUV422 pixel, int brightPixel, 
	       int darkPixel){
  
  if (pixel.Y0() > brightPixel || pixel.Y0() < darkPixel)
    return NULL_HUE;
  return (M_PI / 180) * (atan2((float)pixel.U(), (float) pixel.V()));
};

float YCbCrtoHue(XV_YCbCr pixel, int brightPixel, 
	       int darkPixel){
  
  if (pixel.Y0() > brightPixel || pixel.Y0() < darkPixel)
    return NULL_HUE;
  return (M_PI / 180) * (atan2((float)pixel.U(), (float) pixel.V()));
};

XV_HSV24 YUVtoHSV(XV_YUV24 pixel, int brightPixel,
		  int darkPixel){
  
  if (pixel.Y() > brightPixel || pixel.Y() < darkPixel){
    XV_HSV24 rp;
    rp.h = (u_char)NULL_HUE;
    rp.s = 0; rp.v = 0;
    return rp;
  }
  XV_HSV24 rp;
  rp.h = (u_char)((0.01745) * (atan2((float)pixel.U(), (float) pixel.V())));
  rp.s = pixel.Y();
  rp.v = pixel.V() > (pixel.U() * 1.45) ? pixel.V() : (u_char)(pixel.U() * 1.45);
  return rp;
};  

XV_HSV24 YUV422toHSV(XV_YUV422 pixel, int brightPixel,
		     int darkPixel){
  
  if (pixel.Y0() > brightPixel || pixel.Y0() < darkPixel){
    XV_HSV24 rp;
    rp.h = (u_char)NULL_HUE;
    rp.s = 0; rp.v = 0;
    return rp;
  }
  XV_HSV24 rp;
  rp.h = (u_char)((0.01745) * (atan2((float)pixel.U(), (float) pixel.V())));
  rp.s = pixel.Y0();
  rp.v = pixel.V() > (pixel.U() * 1.45) ? pixel.V() : (u_int)(pixel.U() * 1.45);
  return rp;
};  

XV_HSV24 YCbCrtoHSV(XV_YCbCr pixel, int brightPixel,
		     int darkPixel){
  
  if (pixel.Y0() > brightPixel || pixel.Y0() < darkPixel){
    XV_HSV24 rp;
    rp.h = (u_char)NULL_HUE;
    rp.s = 0; rp.v = 0;
    return rp;
  }
  XV_HSV24 rp;
  rp.h = (u_char)((0.01745) * (atan2((float)pixel.U(), (float) pixel.V())));
  rp.s = pixel.Y0();
  rp.v = pixel.V() > (pixel.U() * 1.45) ? pixel.V() : (u_int)(pixel.U() * 1.45);
  return rp;
};  

XV_YUV24 HSVtoYUV(XV_HSV24 p){

  XV_YUV24 rp;  rp.y = p.s;
  rp.u = (u_char)(tan(p.h * 180 / M_PI) * (p.v / 1.45));
  rp.v = (u_char)(p.v / 1.45);
  return rp;
};
