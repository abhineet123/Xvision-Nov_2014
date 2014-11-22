#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Mrt.h"
#include <XVMacros.h>

using namespace std ;

extern int debug;
template class Mrt<u_short>;

static u_char
   bt819[2][27]=
   //NTSC
   {{0,0x58,0,0x00,0x16,0xE0,0xA8,0x80,0x02,0xAA,
     0x0,0x20,0xD8,0xFE,0xB4,0,0,0,6,0x60,0x00,0,0,0x70,0x68,0x5D,0xA0},
    //PAL
    {0,0x58,0,0x23,0x40,0x40,0x48,0x10,0x1D,0x3C,
     0x0,0x20,0xD8,0xFE,0xB4,0,0,0,6,0x60,0,1,0,0x70,0x7F,0x72,0x82}};

static int K[256];                     // Table for YCrCb-RGB conversion
static int Rb[256];                    // Table for YCrCb-RGB conversion
static int Gb[256];                    // Table for YCrCb-RGB conversion
static int Gc[256];                    // Table for YCrCb-RGB conversion
static int Bb[256];                    // Table for YCrCb-RGB conversion

static u_short CprR16[280*2+256];   // Table for 16bit   red conversion
static u_short CprG16[280*2+256];   // Table for 16bit green conversion
static u_short CprB16[280*2+256];   // Table for 16bit  blue conversion


#define K1 1.164f
#define R2 1.596f
#define K2 18.624f
#define G2 0.813f
#define G3 0.391f
#define B2 2.018f

//   The formulas are implemented in lookuptables as follows:
//     R=Cpr[ K[Y] + Rb[Cr] ]
//     G=Cpr[ K[Y] + Gb[Cr] + Gc[Cb] ]
//     B=Cpr[ K[Y] + Bb[Cb] ]
//
//Cpr converts from int to BYTE, setting negative values to 0 and values
//greater than 255 to 255. Also does gamma-correction.
//
//*********************************************************************/
static void
fillLookupTables(float gamma)
{
  u_int i;
  u_char gam;

  for (i=0; i<256; i++)
  {
    K[i]=(int)((float)i*K1-K2)+280;
    Rb[i]=(int)(((float)i-128.0)*R2);
    Gb[i]=(int)((128.0-(float)i)*G2);
    Bb[i]=(int)(((float)i-128.0)*B2);
    Gc[i]=(int)((128.0-(float)i)*G3);
     gam = (u_char) ((gamma==1.0f) ? (u_char)i :
        ((i==0) ? 0 : (u_char)(255*pow(i/255.0,1.0/gamma)+0.5)));
     /* RRRR RGGG GGGB BBBB */
     CprR16[i+280] = (u_short) (((u_short)(gam&0xF8U))<<8);
     CprG16[i+280] = (u_short) (((u_short)(gam&0xFCU))<<3);
     CprB16[i+280] = (u_short) (((u_short)(gam&0xF8U))>>3);
  }
  for (i=0; i<280; i++)
  {
    /* RRRR RGGG GGGB BBBB */
    CprR16[i] = 0;
    CprG16[i] = 0;
    CprB16[i] = 0;
    CprR16[i+280+256] = (0xF8U)<<8;
    CprG16[i+280+256] = (0xFCU)<<3;
    CprB16[i+280+256] = (0xF8U)>>3;
  }
}

template <class T>
void Mrt<T>::grab_init(void)
{
   ioctl(fd,MRT_HOME_CURSOR,&grab_format);
   ioctl(fd,MRT_CAPTURE,&grab_format);
}

template <class T>
int Mrt<T>::grab_wait(int i_frame)
{
   u_long status;
   u_short *w_ptr,value;
   u_char *c_val1,*c_val2,*r_ptr;
   int	  i,j,col_lim;
   static u_char *buffer=NULL;

   if(!buffer) buffer=
      new u_char[size.Height()*size.Width()*2];
   i=0;
   ioctl(fd,MRT_VSYNC,&grab_format);
#ifdef GRAYSCALE
   col_lim=size.Width()/2;  //grayscale
#else
   col_lim=size.Width();
#endif
   for(w_ptr=mm_buf[i_frame],j=0;j<size.Height();j++) 
   {
     ioctl(fd,MRT_NEXT_LINE,&grab_format);
     for(i=0;i<col_lim;i++) *w_ptr++=*field;
   }
   // start new grab
   grab_init();
#ifndef GRAYSCALE
     for(r_ptr=(u_char*)buffer,w_ptr=(ushort*)mm_buf[i_frame],j=0;
      j<size.Width()*size.Height()/2;j++)
     {
       *w_ptr++=CprB16[K[r_ptr[1]]+Bb[r_ptr[0]]]|
              CprG16[K[r_ptr[1]]+Gb[r_ptr[2]]+Gc[r_ptr[0]]]|
	      CprR16[K[r_ptr[1]]+Rb[r_ptr[2]]];
       *w_ptr++=CprB16[K[r_ptr[3]]+Bb[r_ptr[0]]]|
              CprG16[K[r_ptr[3]]+Gb[r_ptr[2]]+Gc[r_ptr[0]]]|
	      CprR16[K[r_ptr[3]]+Rb[r_ptr[2]]];
       r_ptr+=4;
     }
#else
     for(r_ptr=(u_char*)buffer,w_ptr=(ushort*)mm_buf[i_frame],j=0;
      j<size.Width()*size.Height();j++,r_ptr++)
     {
       value=(*r_ptr>>3);
       *w_ptr++=(value<<11) | (value<<6) |value;
     }
#endif
     assert(frame(i_frame).unlock());
     return 1;
}

template <class T>
int   Mrt<T>::set_input(int norm,int channel)
{
   int scale,delay;

   assert(norm>=NORM_NTSC && norm<=NORM_PAL);
   bt819[norm][BT_CONTROL]&=~BT_COMP;
   bt819[norm][BT_CONTROL]|=channel;
   bt819[norm][BT_HACTIVE_LO]=size.Width()&0xFF;
   scale=0x10000-(640*512/size.Width()-512)&0x1FFF;
   bt819[norm][BT_HSCALE_LO]=(scale) &0xFF;
   bt819[norm][BT_HSCALE_HI]=(scale)>>8;
   scale=0x10000-(480*512/size.Height()-512)&0x1FFF;
   bt819[norm][BT_VSCALE_LO]=scale&0xFF;
   bt819[norm][BT_VSCALE_HI]|=(scale>>8)&0x1F;
   scale=625;
   delay=(int)bt819[BT_HDELAY_LO]|(((int)bt819[BT_CROP]&0xC)<<6);
   bt819[norm][BT_CROP]=((scale>>4)&0x30) | ((size.Width()>>8) &0x3)|
                  ((delay*size.Width()/640)>>6)&0xC;
   bt819[norm][BT_HDELAY_LO]=(delay*size.Width()/640)&0xFE;
   bt819[norm][BT_VACTIVE_LO]=scale&0xFF;
   ioctl(fd,MRT_SET_BT819,bt819[norm]);
#ifdef GRAYSCALE
   grab_format=FRAME_BIT | MONO_BIT;
#else
   grab_format=FRAME_BIT ;
#endif
   return 1;
}

template <class T>
int Mrt<T>::grab_queue(int i_frame)
{
  static int initial=1;

  if(fd<0) return 0;
  if(i_frame <0 || i_frame>n_buffers)
  {
    cerr << "invalid frame number" << endl;
    return 0;
  }
  assert(frame(i_frame).lock());
  next_frame=i_frame;
  if(initial) {initial=0;grab_init();}
  return 1;
}

template <class T>
int Mrt<T>::set_params(char *paramstring)
{
   int i;
   int norm=MRT_Composite1,input=NORM_NTSC;
   XVParser     parse_result;  

   while(parse_param(paramstring,parse_result)>0) 
   switch(parse_result.c)
   {
       case 'N':  // norm selection
          norm=parse_result.val;
          break;
       case 'I':  // input selection
          input=parse_result.val;
          break;
       case 'B': // number of buffers
          n_buffers=parse_result.val;
          break;
       default:
          cerr << parse_result.c << "=" << parse_result.val
               << " is not supported by Mrt (skipping)" << endl;
          break;
   }
   set_input(norm,input);
   assert(n_buffers>0 && n_buffers<=MRT_MAX_GBUFFERS);
   //allocate buffers
   for(i=0;i<n_buffers;i++) mm_buf[i]=
     		new T[size.Width()*size.Height()];
   remap(mm_buf,n_buffers);
   return 1;
}

template <class T>
void Mrt<T>::close(void)
{
  if(fd>=0) ::close(fd);
  fd=-1;
}


template <class T>
Mrt<T>::open(const char *dev_name)
{
  int i;

  if(fd>-1) close();
  if((fd=::open(dev_name,O_RDWR))==-1)
  {
    cerr << "couldn't open device " << dev_name << endl;
    exit(1);
  }
  // mmap buffer into user space
  field=(T *)mmap
     (NULL,0x2000,PROT_READ|PROT_WRITE,
                                  MAP_FILE|MAP_PRIVATE,fd,0);
  if(field==(T *)~0)
  {
    cerr << "couldn't mmap buffers" << endl;
    exit(1);
  }
  //adjust to the start of framebuffer
  field+=0x1900/sizeof(*field);
  fillLookupTables(1.0);
  set_params("B4N0I0");
}

template <class T>
Mrt<T>::Mrt(const char *dev_name,XVSize size_in):
         Video<T>(dev_name,size_in)
{
  fd=-1;
  open(dev_name);
}

template <class T>
Mrt<T>::~Mrt()
{
  if(fd>-1) ::close(fd);
}

