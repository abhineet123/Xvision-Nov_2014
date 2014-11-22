#include "config.h"
#include "XVWindowX.h"


#define TIFF_LIB
#define JPEG_LIB

#include <XVImageIO.h>
#include <unistd.h>
#include "sys/types.h"

int main(int argc, char *argv[]){

  char * jpgfile = "jpgtest.jpg";
  char * tiffile = "tiftest.tif";
  char * ppmfile = "ppmtest.ppm";
  char * pgmfile = "pgmtest.pgm";
  char * grayJPGfile = "grayjpgtest.jpg";
  char * grayTIFfile = "graytiftest.tif";
  
  XVImageRGB<XV_RGB> jpg;
  XVImageRGB<XV_RGB> tif;
  XVImageRGB<XV_RGB> ppm;  
  XVImageScalar<u_char> pgm;
  XVImageScalar<u_char> grayJPG;
  XVImageScalar<u_char> grayTIF;

  XVWindowX<XV_RGB> *win;

  //*****************************
  // JPEG TEST
  //*****************************

  cout << "Testing color JPEG..." << flush;

  if(!XVReadImage(jpg, jpgfile)){ 
    cerr << "Can't read " << jpgfile << endl;
    exit(1); 
  }
  win = new XVWindowX<XV_RGB>(jpg, 300, 200);

  win->map();
  usleep(10000);
  win->CopySubImage(jpg);
  win->swap_buffers();
  win->flush();
  sleep(2);
  XVWriteImage(jpg, "jpgtestout.jpg");

  cout << "done." << endl;

  //*****************************
  // TIFF TEST
  //*****************************

  cout << "Testing color TIFF..." << flush;

  if(!XVReadImage(tif, tiffile)){ 
    cerr << "Can't read " << tiffile << endl;
    exit(1); 
  }

  win->CopySubImage(tif);
  win->swap_buffers();
  win->flush();
  sleep(2);
  XVWriteImage(tif, "tiftestout.tif");

  cout << "done." << endl;

  //*****************************
  // PPM TEST
  //*****************************

  cout << "Testing color PPM..." << flush;

  if(!XVReadImage(ppm, ppmfile)){ 
    cerr << "Can't read " << ppmfile << endl;
    exit(1); 
  }

  win->CopySubImage(ppm);
  win->swap_buffers();
  win->flush();
  sleep(2);
  XVWriteImage(ppm, "ppmtestout.ppm");

  cout << "done." << endl;

  //*****************************
  // gray JPG TEST
  //*****************************

  cout << "Testing gray JPEG..." << flush;

  if(!XVReadImage(grayJPG, grayJPGfile)){
    cerr << "Can't read " << grayJPGfile << endl;
    exit(1);
  }

  win->CopySubImage(grayJPG);
  win->swap_buffers();
  win->flush();
  sleep(2);

  XVWriteImage(grayJPG, "grayjpgtestout.jpg", "jpg");

  cout << "done." << endl;

  //*****************************
  // gray TIF TEST
  //*****************************

  cout << "Testing gray TIFF..." << flush;

  if(!XVReadImage(grayTIF, grayTIFfile)){
    cerr << "Can't read " << grayTIFfile << endl;
    exit(1);
  }

  win->CopySubImage(grayTIF);
  win->swap_buffers();
  win->flush();
  sleep(2);

  XVWriteImage(grayTIF, "graytiftestout.tif", "tif");

  cout << "done." << endl;

  //*****************************
  // PGM TEST
  //*****************************

  cout << "Testing gray PGM..." << flush;

  if(!XVReadImage(pgm, pgmfile)){ 
    cerr << "Can't read " << pgmfile << endl;
    exit(1); 
  }

  win->CopySubImage(pgm);
  win->swap_buffers();
  win->flush();
  sleep(2);
  
  XVWriteImage(pgm, "pgmtestout.pgm", "pgm");

  cout << "done." << endl;
}

