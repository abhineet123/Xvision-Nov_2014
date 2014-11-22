// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <string.h>
#include "XVImageBase.h"
#include "XVImageIterator.h"
#include "XVMacros.h"

XVSize XVSize_AVI_full(720,480);
XVSize XVSize_NTSC_full(640,480);
XVSize XVSize_NTSC_half(320,240);
XVSize XVSize_NTSC_quarter(160,120);

// Flag to be used for slow memcpy-based image duplication
//#define SLOW_COPY

template <class T>
inline void XVImageBase<T>::setSubImage(const XVSize & xsize, const XVPosition & xvp) {
  XVImageBase<T>::setSubImage(xvp.PosX(),xvp.PosY(),
			      xsize.Width(),xsize.Height());
}

template <class T>
inline void XVImageBase<T>::setSubImage(int x,int y,int set_width,int set_height)
{
  ASSERT(x>=0 && y>=0 && x+set_width<=pixmap->SizeX() &&
	 y+set_height<=pixmap->SizeY() && set_width>0 && set_height>0);
  posx=x; posy=y;
  width=set_width;height=set_height;
  lineskip=pixmap->SizeX()-width;
  win_addr=pixmap->buffer_addr+posy*pixmap->SizeX()+posx;
}

template <class T>
inline void XVImageBase<T>::setSubImage(const XVImageGeneric &w)
{
  ASSERT(w.PosX()>=0 && w.PosY()>=0 && w.PosX()+w.Width()<=pixmap->SizeX() &&
	 w.PosY()+w.Height()<=pixmap->SizeY());
  posx=w.PosX(); posy=w.PosY();
  width=w.Width();height=w.Height();
  lineskip=pixmap->SizeX()-width;
  win_addr=pixmap->buffer_addr+posy*pixmap->SizeX()+posx;
}

template <class T> 
void XVImageBase<T>::resize(const XVSize &xsize)
{
  resize(xsize.Width(),xsize.Height());
}

template <class T> 
void XVImageBase<T>::resize(int set_width,int set_height)
{
  if (pixmap) {
    ASSERT(lock());
    if(set_width!=width || set_height!=height)
      {
	if(posx+set_width>pixmap->SizeX() ||
	   posy+set_height>pixmap->SizeY())
	  {
	    pixmap->remove(this);
	    if(pixmap->list_length()==0)
	      {
		ASSERT(unlock());
		delete pixmap;
	      }
	    pixmap=new XVPixmap<T>(set_width,set_height);
	    win_addr=pixmap->buffer_addr+posy*pixmap->SizeX()+posx;
	    ASSERT(lock());
	    posx=0,posy=0;
	    pixmap->append(this);
	  }
	width=set_width,height=set_height;
	lineskip=pixmap->SizeX()-width;
      }
  }
  else {
    pixmap=new XVPixmap<T>(set_width,set_height);
    win_addr=pixmap->buffer_addr+posy*pixmap->SizeX()+posx;
    ASSERT(this->lock());
    posx=0,posy=0;
    pixmap->append(this);
    width=set_width,height=set_height;
    lineskip=pixmap->SizeX()-width;
  }
  ASSERT(unlock());
}

template <class T> 
XVImageBase<T>::XVImageBase(XVPixmap<T> * pix) 
  : XVNode(), XVImageGeneric((XVSize &) * pix) {

  lineskip=0;
  write_perm=0;
  win_addr= pix->buffer_addr;
  this->pixmap = pix;
  pix->append(this);
}

template <class T>
XVImageBase<T>::XVImageBase(const XVImageBase<T> & k) 
  : XVNode(), XVImageGeneric(k) {

  pixmap = k.pixmap;
  if(pixmap) pixmap->append(this);
  lineskip = k.lineskip;
  win_addr = k.win_addr;
  write_perm = 0;
}

// This should be pointer swinging as an option -- GDH

template <class T>
XVImageBase<T> & XVImageBase<T>::operator = (const XVImageBase<T>& k)
{

  ASSERT(k.pixmap != NULL);
#ifndef SLOW_COPY

  // delete the current pixmap registration
  if(pixmap != k.pixmap) {
    if (pixmap != NULL) {
      pixmap->remove(this);
      if(pixmap->empty()) delete pixmap;
    }
    pixmap = k.pixmap;
    // ... but we still need to register as new pixmap user
    pixmap->append(this);
    write_perm = 0;
  }

  posx = k.posx, posy = k.posy, width = k.width, height = k.height;
  lineskip = k.lineskip, pixmap = k.pixmap;
  win_addr = k.win_addr;

#else // SLOW_COPY

  if(pixmap == k.pixmap){ // same pixmap involved?
     pixmap->remove(this);
     if(pixmap->empty()) delete pixmap;
  }
  if(pixmap && (pixmap->SizeX() != k.width || pixmap->SizeY() != k.height)) {
     pixmap->remove(this);
     if(pixmap->empty()) delete pixmap;
  }
  posx = 0, posy = 0;
  // pixmap stays if the right size was detected
  if(!pixmap){
    pixmap = new XVPixmap<T>(k.width, k.height);
    pixmap->append(this);
  }
  lineskip = 0;
  width = k.width;
  height = k.height;
  win_addr = pixmap->buffer_addr;
  if(!k.lineskip){ // just a simple memcpy if k-image=pixmap
  
    T * to = lock();
    ASSERT(to);
    memcpy(to, k.win_addr, width * height * sizeof(T));
    unlock();

  }else{

    int i;
    T * from;
    T * to = lock();
    ASSERT(to);
    for(i = 0, from = k.win_addr; i < k.height;
	i++, to += width, from += k.pixmap->Width()) 
       memcpy((char *)to,(char *)from,width*sizeof(T));
    unlock();
  }
#endif // SLOW_COPY
  return *this;
}

#define XV_BASE_CONSTRUCT \
  pixmap=!s_pixm? new XVPixmap<T>(width,height) : s_pixm;\
  pixmap->append(this);\
  lineskip=pixmap->SizeX()-width;\
  write_perm=0;\
  win_addr=pixmap->buffer_addr+posy*pixmap->SizeX()+posx;\
  ASSERT(width<=pixmap->SizeX() && height<=pixmap->SizeY())
  
template <class T> 
XVImageBase<T>::XVImageBase(const int set_width,const int set_height,
			    XVPixmap<T> *s_pixm)
  : XVNode(), XVImageGeneric(set_width, set_height) 
{
  XV_BASE_CONSTRUCT;
}

template <class T>
XVImageBase<T>::XVImageBase(const XVSize &xsize, XVPixmap<T> *s_pixm) : XVNode(), XVImageGeneric(xsize) {
  XV_BASE_CONSTRUCT;
}

template <class T>
XVImageBase<T>::XVImageBase(const XVImageGeneric &xvg, XVPixmap<T> *s_pixm) : XVNode(), XVImageGeneric(xvg) {
  XV_BASE_CONSTRUCT;
}

template <class T> 
XVImageBase<T>::XVImageBase() : XVNode() , XVImageGeneric()
{
  pixmap=NULL;
  win_addr=NULL;
  write_perm=0;
}

template <class T> 
XVImageBase<T>::~XVImageBase()
{
  if(pixmap)
  {
    pixmap->remove(this);
    if(!pixmap->list_length()) delete pixmap;
  }
}

#define XV_PIXMAP_CONSTRUCT \
locked=0;\
own_flag=set_own;\
image_type = figure_out_type( T() );\
buffer_addr=n_address? n_address : new T[width*height]

template <class T>
XVPixmap<T>::XVPixmap(int set_width,int set_height, T * n_address,
		      bool set_own)
  : XVSize(set_width,set_height)
{
  XV_PIXMAP_CONSTRUCT;
}

template <class T>
XVPixmap<T>::XVPixmap(const XVSize &xsize, T * n_address, bool set_own)
  : XVSize(xsize)
{
  XV_PIXMAP_CONSTRUCT;
}
  

template <class T>
XVPixmap<T>::~XVPixmap(){
  if(buffer_addr && own_flag) delete [] buffer_addr;
}

template <class T>
void XVImageBase<T>::remap(T * addr,bool set_own) {
  if(pixmap->list_length() == 1) {
    if(pixmap->buffer_addr && pixmap->own_flag) 
	    		delete [] pixmap->buffer_addr; 
    pixmap->buffer_addr = addr;
    pixmap->own_flag=set_own;
  }else{
     pixmap->remove(this);
     pixmap = 
       new XVPixmap<T>(pixmap->SizeX(),pixmap->SizeY(),addr,set_own);
     pixmap->append(this);
  }
  win_addr = pixmap->buffer_addr + posy * pixmap->height + posx;
}


template class XVPixmap<u_char>;
template class XVPixmap<u_short>;
template class XVPixmap<u_int>;
template class XVPixmap<u_long>;
template class XVPixmap<char>;
template class XVPixmap<short>;
template class XVPixmap<int>;
template class XVPixmap<long>;
template class XVPixmap<float>;
template class XVPixmap<double>;

template class XVPixmap<XV_RGB16>;
template class XVPixmap<XV_RGB24>;
template class XVPixmap<XV_TRGB24>;
template class XVPixmap<XV_RGBA32>;
template class XVPixmap<XV_GLRGBA32>;
template class XVPixmap<XV_RGB15>;

template class XVPixmap<XV_YUV24>;
template class XVPixmap<XV_YUV422>;
template class XVPixmap<XV_YCbCr>;
template class XVPixmap<XV_UVBAND16>;

template class XVPixmap<XV_HSV24>;

template class XVImageBase<bool>;
template class XVImageBase<u_char>;
template class XVImageBase<u_short>;
template class XVImageBase<u_int>;
template class XVImageBase<u_long>;
template class XVImageBase<char>;
template class XVImageBase<short>;
template class XVImageBase<int>;
template class XVImageBase<long>;
template class XVImageBase<float>;
template class XVImageBase<double>;

template class XVImageBase<XV_RGB15>;
template class XVImageBase<XV_RGB16>;
template class XVImageBase<XV_RGB24>;
template class XVImageBase<XV_TRGB24>;
template class XVImageBase<XV_RGBA32>;
template class XVImageBase<XV_GLRGBA32>;

template class XVImageBase<XV_HSV24>;

template class XVImageBase<XV_YUV16>;
template class XVImageBase<XV_YUV24>;
template class XVImageBase<XV_YUV422>;
template class XVImageBase<XV_YCbCr>;

