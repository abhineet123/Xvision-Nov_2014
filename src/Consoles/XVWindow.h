// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVWINDOW_H_
#define _XVWINDOW_H_

#include <XVImageBase.h>

#define  MAX_NUM_IMAGES 5
#define  MAX_WIDTH	1200
#define  MAX_HEIGHT	1000
#define  MIN_WIDTH	120
#define  MIN_HEIGHT	90


/** template class for windows */
template <class PIX>
class XVWindow : public XVImageBase<PIX> {
 protected:
  using XVImageBase<PIX>::width ;
  using XVImageBase<PIX>::height ;
  using XVImageBase<PIX>::posx ;
  using XVImageBase<PIX>::posy ;

 protected:

  const char * windowTitle;

  XVImageRGB<PIX> * frames_buf;
  int		        buffer_count;
  
 public:

  XVWindow() : XVImageBase<PIX>(), frames_buf(NULL) {}
  XVWindow(int w, int h) : XVImageBase<PIX>(w, h), frames_buf(NULL) {}

  virtual void map(void) = 0;
  virtual void unmap(void) = 0;
  
  virtual void resize( const XVSize& ) = 0;

  virtual void setTitle( const char *title = 0 ) {}
  virtual void setImages(XVImageRGB<PIX> * frames, int count) = 0;
  virtual void CopyImage(int, u_short) = 0;
  virtual void CopySubImage(const XVImageRGB<PIX> &,bool flip=0) = 0;

  virtual void swap_buffers(void) = 0;
  virtual void flush(void) = 0;

  virtual XVImageRGB<PIX> getDisplayedImage(int, int, int, int) = 0;
};

#include <XVDrawable.h>

/** template class for windows that allow drawing inside them */
template <class PIX>
class XVDrawWindow : virtual public XVWindow<PIX>, 
                     virtual public XVDrawable {
 protected:
  using XVWindow<PIX>::width ;
  using XVWindow<PIX>::height ;
  using XVWindow<PIX>::posx ;
  using XVWindow<PIX>::posy ;

 public:
  XVDrawWindow() : XVWindow<PIX>() {} 
};

#endif
