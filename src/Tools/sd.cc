#include "X11/Xlib.h"
#include <stdio.h>
#include <stdlib.h>

int main(){

  Display * d = XOpenDisplay(NULL);
  if(d != NULL){
    int p;
    int bitmapPad = -1;
    XPixmapFormatValues * formats = XListPixmapFormats(d, & p);
    for(int i = 0; i < p; i++){
      
      if(formats[i].depth == DefaultDepth(d, DefaultScreen(d)))	
	bitmapPad = formats[i].bits_per_pixel;
    }
    printf("%d\n", bitmapPad);
    exit(0);
  }
  printf("%d\n", -1);
  exit(0);
}
