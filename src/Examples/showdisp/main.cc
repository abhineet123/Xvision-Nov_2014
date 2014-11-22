#include "config.h"
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <XVRemoteWindowX.h>
#include <XVImageIO.h>
#include "StereoVidere.h"

//#define REMOVE_FLOOR
#define VIDEO   //show grayscale image


typedef StereoVidere<XVImageRGB<XV_RGB>, XVImageScalar<short> > VID;
typedef XVRemoteWindowX<XV_RGB>                                     WIN;

static VID* vid;

void sighndl(int ws) {
  cerr << "SIGINT called" << endl;
  if(vid) delete vid;
  exit(0);
}

void sighndl2(int ws) {
  cerr << "SIGSEGV called" << endl;
  if(vid) delete vid;
  exit(0);
}

static void
calc_display(XVImageScalar<short> &disp_image,
                                XVImageRGB<XV_RGB> &im)
{
  int size=disp_image.Width()*disp_image.Height();
  int   shift=2;

  XVImageIterator<short>  rptr(disp_image);
  XV_RGB*  wptr=im.lock() ;
  unsigned char value;

  for(int count=0;count<size;++rptr,wptr++,count++)
  {
     value= *rptr >0? (*rptr)>>shift : 0;
     wptr->setR(value);
     wptr->setB(value);
     wptr->setG(value);
  }
  im.unlock();
}

int main(int argc, char* argv[]) {
  signal(SIGINT, sighndl);
  signal(SIGSEGV, sighndl2);


  char * config = getenv("VIDERE_CONFIG");

  if(config) {
    vid = new VID(config);
  }
  else {
    vid = new VID();
  }
  
  XVImageScalar<short> *disp_image, *win_image;
  const short *r_ptr;

  vid->set_stoc(true);
  vid->next_frame_continuous();
  disp_image = vid->calc_stereo();
#ifdef VIDEO
  XVImageScalar<u_char> **images;
  XVWindowX<XV_RGB>   video(vid->frame(0).Width(),vid->frame(0).Height(),
			      disp_image->Width(),0);
  video.map();
#endif 
  XVImageRGB<XV_RGB>     im(disp_image->Width(),disp_image->Height());
  WIN 			 win(im);
  win.map();
  struct timeval start_time, end_time;
  while(1) {
    gettimeofday(&start_time, NULL);
    
    for(int i = 0; i < 50; i++) {
      vid->wait_for_completion(0);
#ifdef VIDEO
      images=vid->get_warped();
      video.CopySubImage(*(images[XVVID_LEFT]));
      //XVWritePPM(*images[XVVID_LEFT],(char*)"left_image.ppm");
      //XVWritePPM(*images[XVVID_RIGHT],(char*)"right_image.ppm");
      //FILE *fptr=fopen("disp_image.ppm","w");
      //r_ptr=disp_image->data();
      //for(int k=0;k<disp_image->Width()*disp_image->Height();k++)
      //   fprintf(fptr,"%d ",*r_ptr++);
      //fclose(fptr);
      video.swap_buffers();
      video.flush();
#endif

      disp_image = vid->calc_stereo();
      if(!disp_image) continue;
      calc_display(*disp_image,im);
      win.CopySubImage(im);
      win.swap_buffers();
      win.flush();
    }

    gettimeofday(&end_time, NULL);

    cout << 50 / ((end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) * 1e-6)
	 << " [Hz]"<< endl;
  }

  delete vid;
  return 1;
}

