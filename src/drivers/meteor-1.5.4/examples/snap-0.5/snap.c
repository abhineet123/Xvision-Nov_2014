#include <sys/fcntl.h>
#include <sys/mman.h>
#include "ioctl_meteor.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

/*
 * Usage: snap   [-d device] [-r rows] [-c cols] [-o outfile]
 *                [-R retries] [-s sourcenum] [-f ofmt] [-m]
 *       
 *
 * Extensively modified from "single.c" distributed with the meteor-1.0
 * driver package by Jim Bray, and probably originating with Lowe and
 * Tinguely's FreeBSD Meteor driver docs.
 *
 * Version 0.5 by Pat Flynn (flynn@eecs.wsu.edu)
 *
 * device defaults to DEF_DEVICE below
 * rows defaults to DEF_ROWS below
 * cols defaults to DEF_COLS below
 * outfile defaults to the standard output stream.
 * retries defaults to DEF_RETRIES above.  If set to a nonzero value,
 *         the  fifo and dma error counts are checked after acquisition.  If
 *         the counts increase, an error is assumed to have occurred and the
 *         acquisition is repeated up to the specified number of times.
 *         If retries is set to a nonzero value, no error checking is performed
 *         and the digitized image is assumed to be OK.
 *
 * sourcenum defaults to DEF_SOURCE above.  If [-s sourcenum] is specified,
 *         the source is picked out of the array above.
 * 
 * ofmt    defaults to DEF_OFMT above.  If [-f ofmt] is specified, the output
 *         format is picked out of the array above.
 *
 * -m:     if specified, causes the output image to be written in PGM format
 *         (think "m == monochrome").  By default, output images are written
 *         in PPM format.
 *
 */


extern int errno;

#define DEF_ROWS 256
#define DEF_COLS 240
#define DEF_DEVICE "/dev/mmetfgrab0"
#define DEF_RETRIES 5
#define DEF_SOURCE METEOR_INPUT_DEV0
#define DEF_OFMT METEOR_GEO_RGB24
#define DEF_IFMT METEOR_FMT_NTSC
 
int input_src[] = { METEOR_INPUT_DEV0, METEOR_INPUT_DEV1, METEOR_INPUT_DEV2,
                     METEOR_INPUT_DEV3, METEOR_INPUT_DEV_SVIDEO,
                     METEOR_INPUT_DEV_RGB };


int output_fmt[] =  { METEOR_GEO_RGB16, METEOR_GEO_RGB24, METEOR_GEO_YUV_PACKED,
                METEOR_GEO_YUV_PLANAR };


void set_meteor_params(int fd, struct meteor_geomet *geo,int *src,int *ofmt);
int get_buf_size(int rows,int cols,int ofmt);
int get_meteor_error_counts(int fd);

 
#define USAGE "Usage: %s [-d device] [-r rows] [-c cols] [-o outfile] [-R retries] [-s sourcenum] [-f ofmt] [-m]\n"

/*--------------------------------------------------------------------------
  A waiting loop.  This loop delays aproximately "i" microseconds.  E.g. to
  delay for 1/10 of a second, pass it a value of 100000.  Caviets:
  probably highly O/S dependant.  Some UNIX flavors don't have "gettimeofday"
  others might measure in units other than microseconds.  Also, please do
  not use this for delays longer than 1 sec, use something like "sleep"
  instead.  Also note, due to context switching, this is really sloppy,
  it is not a precise timer!
 *-------------------------------------------------------------------------*/
void fnWait(long i)
{
  struct timeval timeback;
  struct timezone zoneback;
  int dummy;
  long curtime,initime,elapsedtime,cutouttime,sectime;

  dummy = gettimeofday(&timeback,&zoneback);
  initime = timeback.tv_usec;
  sectime = timeback.tv_sec;
  if ((initime+i) < 1000000)
    {
      do
	{
	  dummy = gettimeofday(&timeback,&zoneback);
	  curtime = timeback.tv_usec;
	  elapsedtime = curtime-initime;
	} while((elapsedtime < i) && (timeback.tv_sec == sectime));
    }
  else
    {
      cutouttime = initime+i-1000000;
      do
	{
	  dummy = gettimeofday(&timeback,&zoneback);
	  curtime = timeback.tv_usec;
	} while(((timeback.tv_sec == sectime) || (curtime < cutouttime)) && timeback.tv_sec < (sectime + 2));
    }
}

int main(int argc,char *argv[])
{
  struct meteor_geomet geo;
  char *mmbuf;
  int c,i,o;
  char *DevName = DEF_DEVICE;
  int rows = DEF_ROWS, cols = DEF_COLS;
  int a;
  extern char *optarg;
  char *argv0 = argv[0];
  char tmp[128];
  FILE *fp = stdout;
  char *outfile=NULL;
  char success=0;
  int retries=DEF_RETRIES;
  int tries=0;
  int cnt_total;
  int src = DEF_SOURCE;
  int ofmt = DEF_OFMT; 
  int ifmt = DEF_IFMT;
  char mono=0;
  int size;
  char *buf;

  /* parse command line */
  while ((a=getopt(argc,argv,"d:r:c:o:mR:f:s:")) != -1) {
   switch(a) {
    case 'd': DevName = optarg; break;
    case 'r': rows = atoi(optarg); break;
    case 'c': cols = atoi(optarg); break;
    case 'o': outfile = optarg; break;
    case 'm': mono=1; break;
    case 'R': retries=atoi(optarg); break;
    case 's': src = input_src[atoi(optarg)]; break;
    case 'f': ofmt = output_fmt[atoi(optarg)]; break;
    default: /*  huh? */
    case '?': {
     fprintf(stderr,"Usage: %s [-d device] [-r rows] [-c cols]\n",argv0);
     exit(-1);
     }
    }
   }

  /* open the digitizer */
  if ((i = open(DevName, O_RDONLY)) < 0) {
    sprintf(tmp,"open(%s)",DevName);
    perror(tmp);
    exit(-1);
  }

  /* set up the capture type and size */
  geo.rows = rows;
  geo.columns = cols;
  geo.frames = 1;
  geo.oformat = ofmt;

  set_meteor_params(i,&geo,&src,&ifmt);
  fnWait(200000);

  /*
   * determine mmap() buffer size (not necessarily the same as the output image
   * buffer size)
   */
  size = get_buf_size(rows,cols,ofmt);

  if ((mmbuf=(char *)mmap((caddr_t)0, size, PROT_READ,
		     MAP_FILE|MAP_PRIVATE, i, (off_t)0)) == (char *)-1) {
   perror("mmap failed");
   exit(-1);
   }

  /*
     If checking for errors, get a baseline error count.
     This seems to be reset to 0 every time the board is initialized, so
     we could skip this.  Just to be safe...
   */
  if (retries)  cnt_total=get_meteor_error_counts(i);
   
  /* capture one frame */
  c = METEOR_CAP_SINGLE ;
  if (ioctl(i, METEORCAPTUR, &c)) {
   perror("ioctl(METEORCAPTUR)");
   exit(-1);
   }

  while(retries && (tries<retries) && !success) {
   int cnt_now=get_meteor_error_counts(i);
   if (cnt_now == cnt_total) { 
    success =1;   /* yippee! */
    }
   else {
    tries ++;
    /*fprintf(stderr,"[retry]");*/
    cnt_total = cnt_now;
    c = METEOR_CAP_SINGLE ;
    if (ioctl(i, METEORCAPTUR, &c)) {
     perror("ioctl(METEORCAPTUR)");
     exit(-1);
     }
    }
   } /* while */
   
  if (retries) {
   if (success) { /*fprintf(stderr," [success]\n") */ ; }
   else {
    fprintf(stderr,"Error: failed to capture frame (FIFO or DMA errors, %d attempts)\n",retries);
    exit(-1);
    };
   }

  close(i);
  
  /*
   * take the image data from the memory-mapped area and copy it into an
   * output image buffer.
   */
  if (mono) {
   unsigned char *ptr,*mptr=mmbuf;
   buf = (char *)malloc(rows*cols);
   ptr=buf;
   switch(ofmt) {
    case METEOR_GEO_RGB16: {
     /* 5 bits each of red, green and blue. */
     /* assumption: equally weight the red, green, and blue intensities. */
     for(i=0;i<rows*cols;i++) {
      unsigned short pixel = mptr[0] + 256*mptr[1];
      unsigned char gray=(((pixel&0x1f)+((pixel>>5)&0x1f)+((pixel>>10)&0x1f))/3)<<3;
      *ptr++ = gray;
      mptr += 2;
      };
     break;
     }
    case METEOR_GEO_RGB24: {
     for(i=0;i<rows*cols;i++) {
      /* assumption: equally weight the red, green, and blue intensities. */
      *ptr++ = (mptr[0] + mptr[1] + mptr[2])/3;
      mptr+=4;
      };
     break;
     }
    case  METEOR_GEO_YUV_PACKED: {
     /* this is a weird one.  According to Lowe and Tinguely's meteor.doc,
        the Y, U, and V bits are interleaved in a 16-bit word:
        u0 y0 v0 y1 u1 y2 v1 y3 u2 y4 v2 y5 u3 y6 v3 y7
        But in real life, the luminance seems to reside in the second half
        of the 16-bit word.
     */
     for(i=0;i<rows*cols;i++) {
      *ptr++ = mptr[1]; /* XXX DOES NOT AGREE WITH meteor.doc !!!! */
      mptr += 2;
      };
     break;
     }
    case METEOR_GEO_YUV_PLANAR: { 
     /* easy */
     bcopy(mptr,ptr,rows*cols);
     break;
     }
    default: {
     fprintf(stderr,"Bogus output format 0x%x\n",ofmt);
     exit(-1);
     };
    }; /* switch */
   } /* if */
  else { /* color! */
   unsigned char *ptr,*mptr=mmbuf;
   ptr = buf = (unsigned char *)malloc(rows*cols*3);
   switch(ofmt) {
    case METEOR_GEO_RGB16: {
     /* mask & shift color components into place */
     for(i=0;i<rows*cols;i++) {
      unsigned short pixel = mptr[0] + 256*mptr[1];
      *ptr++ = (pixel&0x7c00)>>7; /* RED  */
      *ptr++ = (pixel&0x03e0)>>2; /* GREEN */
      *ptr++ = (pixel&0x001f)<<3;  /* BLUE */
      mptr += 2;
      };
     break;
     }
    case METEOR_GEO_RGB24: {
     for(i=0;i<rows*cols;i++) {
      *(ptr+2)=*mptr++;
      *(ptr+1)=*mptr++;
      *ptr=*mptr++;
      ptr+=3;
      mptr++;
      };
     break;
     }
    /* XXX eventually support other formats */
    default: {
     fprintf(stderr,"Sorry, output format 0x%x is not supported for color.\n",
             ofmt);
     exit(-1);
     };
    }
   } /* else */

  /* if necessary, open the output file. */
  if (outfile) {
   fp = fopen(outfile,"w");
   if (!fp) {
    sprintf(tmp,"fopen(%s)",outfile);
    perror(tmp);
    exit(-1);
    };
   };

  /* make PPM or PGM header and save to file */
  fprintf(fp,"P%c\n%d %d\n255\n",(mono?'5':'6'),cols,rows);
  if (rows*cols != fwrite (buf,(mono?1:3)*sizeof(char),rows*cols,fp)) {
   perror("fwrite()");
   exit(-1);
   };

  fclose(fp);
  exit(0);
}


/*
 * Given an open meteor device identified by the file descriptor fd,
 * load its parameter set.
 */
void set_meteor_params(int fd,struct meteor_geomet *geo,int *src,int *ifmt)
{
  if (ioctl(fd, METEORSETGEO, geo) < 0) {
    perror("ioctl(METEORSETGEO)");
    exit(1);
  }
  if (ioctl(fd, METEORSINPUT, src) < 0) {
    perror("ioctl(METEORSINPUT)");
    exit(1);
  }
  if (ioctl(fd, METEORSFMT, ifmt) < 0) {
    perror("ioctl(METEORSFMT)");
    exit(1);
  }
}

/*
 * determine the buffer size in device space for a given image size
 * and output format.
 */
int get_buf_size(int rows,int cols,int ofmt)
{
  switch(ofmt) {
   case METEOR_GEO_RGB24: return 4*rows*cols; break;
   case METEOR_GEO_RGB16:
   case METEOR_GEO_YUV_PACKED:
   case METEOR_GEO_YUV_PLANAR: return 2*rows*cols; break;
   default: {
    fprintf(stderr,"get_buf_size: bogus output format 0x%x\n",ofmt);
    exit(-1);
    };
   };
}

/*
 * sniff the error numbers out of the Meteor registers and return their sum.
 */
int get_meteor_error_counts(int fd)
{
 struct meteor_counts cnts;
 if (ioctl(fd, METEORGCOUNT, &cnts) < 0) {
  perror("ioctl(METEORGCOUNT)");
  exit(-1);
  }
 return cnts.fifo_errors + cnts.dma_errors;
}
