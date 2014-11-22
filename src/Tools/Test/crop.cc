#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <config.h>
#include <XVImageIO.h>
#include <XVWindowX.h>
#include <XVAffineWarp.h>

int main(int argc, char * argv[]){

  if(argc != 6){ cout << "use: crop <image.ppm> <centerx> <centery> <width> <height>" << endl; exit(1); }

  XVImageRGB<XV_RGB> im;
  if(!XVReadImage(im, argv[1])){ cerr << "can't read " << argv[1] << endl; exit(1); }

  XVWindowX<XV_RGB> w(im, 10, 10, "Image Window");
  w.map();
  w.CopySubImage(im);
  w.swap_buffers();
  w.flush();

  float angle = 0; //atoi(argv[6]);
  float sheer = 0;
  int direc = 1;
  float scale = 1;
  int sdirec = 1;

  XVImageRGB<XV_RGB> rim(10, 10), shim(10, 10), sim(10, 10), aim(10, 10);

  XVWindowX<XV_RGB> rw(rim, 100, 600, "Rotate Window");
  rw.map();

  XVWindowX<XV_RGB> shw(shim, 300, 600, "Sheer Window");
  shw.map();

  XVWindowX<XV_RGB> sw(sim, 600, 600, "Scale Window");
  sw.map();

  XVWindowX<XV_RGB> aw(aim, 900, 600, "All Window");
  aw.map();
 
  while(1){
    
    rim = warpRect(im, XVPosition(atoi(argv[2]), atoi(argv[3])),
		   XVSize(atoi(argv[4]), atoi(argv[5])), angle);
            
    rw.CopySubImage(rim);
    rw.swap_buffers();
    rw.flush();

    shim = warpRect(im, XVPosition(atoi(argv[2]), atoi(argv[3])),
		    XVSize(atoi(argv[4]), atoi(argv[5])), sheer, 0, 0);
    
    shw.CopySubImage(shim);
    shw.swap_buffers();
    shw.flush();

    sim = warpRect(im, XVPosition(atoi(argv[2]), atoi(argv[3])),
		   XVSize(atoi(argv[4]), atoi(argv[5])), scale, scale);
    
    sw.CopySubImage(sim);
    sw.swap_buffers();
    sw.flush();

    aim = warpRect(im, XVPosition(atoi(argv[2]), atoi(argv[3])),
		   XVSize(atoi(argv[4]), atoi(argv[5])), angle, scale, scale, sheer);
    aw.CopySubImage(aim);
    aw.swap_buffers();
    aw.flush();

    angle += 0.1;
    if(sheer > 2 || sheer < -2) direc *= -1;
    sheer += (0.05 * direc);
    if(scale > 4 || scale < 0.4) sdirec *= -1;
    scale += sdirec * 0.1;
    //    usleep(100);
  }
};
