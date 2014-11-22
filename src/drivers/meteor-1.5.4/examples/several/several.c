#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include "ioctl_meteor.h"

extern int errno;

int ROWS=576;
int COLS=768;
int SIZE;

void main(int argc, char **argv)
{
  struct meteor_counts cnt;
  struct meteor_geomet geo;
  struct meteor_frame_offset off;
  char *buf,b[4],str[128],*p;
  char *mmbuf;
  int meteor,o,c;
  int i,j,k,noframes=1;
  char *DevName;
  static char DefaultName[]="/dev/mmetfgrab0";
  u_long cap;


  if (--argc > 0) {
      noframes = atoi(*++argv);
  }

  if (--argc > 0) {
      COLS = atoi(*++argv);
      ROWS = atoi(*++argv);
      argc--;
  }
      
  if (--argc > 0)
    DevName = *++argv;
  else
    DevName = DefaultName;

  SIZE = ROWS*COLS*4;

  if ((meteor = open(DevName, O_RDONLY)) < 0) {
    perror("open failed");
    printf("device %s\n", DevName);
    exit(1);
  }

  /* set up the capture type and size */
  geo.rows = ROWS;
  geo.columns = COLS;
  geo.frames = noframes;
  geo.oformat = METEOR_GEO_RGB24;

  if (ioctl(meteor, METEORSETGEO, &geo) < 0) {
    perror("ioctl SetGeometry failed");
    exit(1);
  }

  if (ioctl(meteor, METEORGFROFF, &off) < 0) {
    perror("ioctl FrameOffset failed");
    exit(1);
  }

  c = METEOR_FMT_PAL;
  if (ioctl(meteor, METEORSFMT, &c) < 0) {
    perror("ioctl SetFormat failed");
    exit(1);
  }

  c = METEOR_INPUT_DEV_RCA;
  if (ioctl(meteor, METEORSINPUT, &c) < 0) {
    perror("ioctl Setinput failed");
    exit(1);
  }

  mmbuf=(char *)mmap((caddr_t)0, noframes*SIZE, PROT_READ,
		     MAP_FILE|MAP_SHARED, meteor, (off_t)0);
  if ( mmbuf == (char *)-1 ) {
    perror("mmap failed");
    exit(1);
  }

  /* capture frame */
  c = METEOR_CAP_CONT_ONCE;
  if (ioctl(meteor, METEORCAPTUR, &c)) {
    perror("ioctl SingleCapture failed");
    exit(1);
  }
  sleep(2);

  if (ioctl(meteor, METEORGCAPT, &cap)) {
      perror("ioctl GetCapt failed");
      exit(1);
  }
  fprintf(stderr, "Capture control: 0x%lx\n", cap);

  /* get error counts */
  if (ioctl(meteor, METEORGCOUNT, &cnt)) {
      perror("ioctl GetCount failed");
      exit(1);
  }
  fprintf(stderr, "Frames: %d\nEven:   %d\nOdd:    %d\n", 
	  cnt.frames_captured,
	  cnt.even_fields_captured,
	  cnt.odd_fields_captured);
  fprintf(stderr, "Even: %d\nOdd:  %d\n", 
	  cnt.even_fields_captured,
	  cnt.odd_fields_captured);
  fprintf(stderr, "Fifo errors: %d\n", cnt.fifo_errors);
  fprintf(stderr, "DMA errors:  %d\n", cnt.dma_errors);

  for (i=0; i<noframes; i++) {
      sprintf(str, "im.%03d.ppm", i);
      fprintf(stderr, "Writing image %d\n", i);
      if ((o = open(str, O_WRONLY | O_CREAT, 0644)) < 0) {
	  perror("ppm open failed");
	  exit(1);
      }

      /* make PPM header and save to file */
      sprintf(str, "P6\n%d %d\n255\n", COLS, ROWS);
      write (o, &str[0], strlen(str));
      /* save the data to PPM file */

      buf = mmbuf + off.frame_offset[i];
      for (j=0; j<ROWS; j++) {
	  for (k=0; k<COLS; k++) {
	      write(o, buf, 3);
	      buf+=4;
	  }
      }

      close(o);
  }

  close(meteor);
  exit(0);
}

