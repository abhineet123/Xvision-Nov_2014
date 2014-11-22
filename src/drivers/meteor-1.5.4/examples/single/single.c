#define USE_MMAP
#include <stdio.h>
#include <sys/fcntl.h>
#ifdef USE_MMAP
#include <sys/mman.h>
#endif
#include "ioctl_meteor.h"

extern int errno;

int ROWS=576;
int COLS=768;
int SIZE;

void main(int argc, char **argv)
{
  struct meteor_counts cnt;
  struct meteor_geomet geo;
  char *buf,b[4],header[32],*p;
  char *mmbuf;
  int i,o,c;
  int j,k;
  char *DevName;
  static char DefaultName[]="/dev/mmetfgrab0";
  u_long cap;


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
  buf = malloc(SIZE);

  if ((i = open(DevName, O_RDONLY)) < 0) {
    perror("open failed");
    printf("device %s\n", DevName);
    exit(1);
  }

  /* set up the capture type and size */
  geo.rows = ROWS;
  geo.columns = COLS;
  geo.frames = 1;
  geo.oformat = METEOR_GEO_RGB24; /*YUV_PLANAR ;*/

  if (ioctl(i, METEORSETGEO, &geo) < 0) {
    perror("ioctl SetGeometry failed");
    exit(1);
  }

  c = METEOR_FMT_PAL;

  if (ioctl(i, METEORSFMT, &c) < 0) {
    perror("ioctl SetFormat failed");
    exit(1);
  }

  c = METEOR_INPUT_DEV_RCA;

  if (ioctl(i, METEORSINPUT, &c) < 0) {
    perror("ioctl Setinput failed");
    exit(1);
  }

  sleep(1);
#ifdef USE_MMAP
  mmbuf=(char *)mmap((caddr_t)0, SIZE, PROT_READ,
		     MAP_FILE|MAP_PRIVATE, i, (off_t)0);
  if ( mmbuf == (char *)-1 ) {
    perror("mmap failed");
    exit(1);
  }
  /* capture one frame */
  c = METEOR_CAP_SINGLE ;
  if (ioctl(i, METEORCAPTUR, &c)) {
    perror("ioctl SingleCapture failed");
    exit(1);
  }
#else
  if ((c=read(i, &buf[0], SIZE)) < SIZE) {
    perror("read failed");
    exit(1);
  }
#endif
  if (ioctl(i, METEORGCAPT, &cap)) {
      perror("ioctl GetCapt failed");
      exit(1);
  }
  fprintf(stderr, "Capture control: 0x%lx\n", cap);

  /* get error counts */
  if (ioctl(i, METEORGCOUNT, &cnt)) {
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

  close(i);

  fprintf(stderr, "Writing image\n");
#if 0
  if ((o = open("rgb8.pgm", O_WRONLY | O_CREAT, 0644)) < 0) {
    perror("ppm open failed");
    exit(1);
  }

  /* make PPM header and save to file */
  sprintf(header, "P5\n%d %d\n255\n", COLS, ROWS);
  write (o, &header[0], strlen(header));
  /* save the data to PPM file */

  write(o, &buf[0], COLS*ROWS);

  close(o);

#elif 1
  if ((o = open("rgb24.ppm", O_WRONLY | O_CREAT, 0644)) < 0) {
    perror("ppm open failed");
    exit(1);
  }

  /* make PPM header and save to file */
  sprintf(header, "P6\n%d %d\n255\n", COLS, ROWS);
  write (o, &header[0], strlen(header));
  /* save the data to PPM file */

  for (j=0; j<ROWS; j++) {
      for (k=0; k<COLS; k++) {
	  write(o, buf, 3);
	  buf+=4;
      }
  }

  close(o);

#else
  {
      int i;
      char *s[3] = {"r.pgm", "g.pgm", "b.pgm"};
      for (i=0; i<3; i++) {
	  if ((o = open(s[i], O_WRONLY | O_CREAT, 0644)) < 0) {
	      perror("pgm open failed");
	      exit(1);
	  }

	  /* make PGM header and save to file */
	  sprintf(header, "P5\n%d %d\n255\n", COLS, ROWS);
	  write (o, &header[0], strlen(header));
	  /* save the data to PPM file */
      
	  write(o, &buf[i*COLS*ROWS], COLS*ROWS);
	  
	  close(o);
      }
  }
#endif

  exit(0);
}

