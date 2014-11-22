// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVREMOTEWINDOWX_H_
#define _XVREMOTEWINDOWX_H_

#include <XVWindowX.h>
#include <pthread.h>
#include <list>
#include <queue>

// The following function is both on the manpage and in libpthread.a ,
// but it is not in pthread.h , so I have to put the prototype here.
extern "C" 
int pthread_mutexattr_setkind_np( pthread_mutexattr_t *attr, int kind );


/**
 The purpose of this XVRemoteWindowX class is to provide an additional
 layer for drawing on a remote X display. A separated thread is used
 to communicate with the X server and drawing requests are discarded
 if new ones come before they are submitted.
 CAUTION: Not all functions are buffered. The only functions buffered
 for sure are ClearWindow, copySubImage, drawXxxx/fillXxxx and check_events. 
 Other requests may be directly passed to the X server. Since another 
 thread has been started to handle X requests, take your own risk using 
 unbuffered functions! -- I won't do that other than map() if I were you.
*/

template<class T>
class XVRemoteWindowX : virtual public XVDrawWindow<T> {
 protected:
  using XVDrawWindow<T>::width ;
  using XVDrawWindow<T>::height ;
  using XVDrawWindow<T>::posx ;
  using XVDrawWindow<T>::posy ;

 protected:
  void			signal_worker();
  struct Drawing {
    enum Type { SwapBuffer, ClearWindow, DrawImage, 
		DrawPoint, DrawLine, DrawRectangle, 
		FillRectangle, DrawEllipse, FillEllipse, DrawString 
    } type ;
    XVDrawColor color ;
    int x1, y1, x2, y2 ;
    char *data ;

    Drawing( Type t, const XVDrawColor& c ) : type(t), color(c), data(0) {}
  };
  typedef std::list<Drawing> Drawings ;
  typedef typename Drawings::iterator OneDrawing ;

  struct Requests {
    bool flush ;
    XVImageRGB<T> image ;
    Drawings drawings ;

    Requests();
    ~Requests();
    void clear(void);
  } *next ;

  struct Event {
    enum { num_args = 2 };
    int type, args[num_args] ;
  };
  typedef std::queue<Event> Events ;

  struct Data {
    XVDrawWindowX<T>* worker ;
    pthread_mutex_t access_last, access_next ;
    Requests *last, *current, *next, *pre_last; 
    pthread_cond_t 	worker_signal;
    pthread_mutex_t       signal_mutex;
    Events events ;
    bool die ;              // ending request
    
    Data( XVDrawWindowX<T>* w );
    ~Data();

    void lock_last() { pthread_mutex_lock( &access_last ); }
    void unlock_last() { pthread_mutex_unlock( &access_last ); }
    void lock_next() { pthread_mutex_lock( &access_next ); }
    void unlock_next() { pthread_mutex_unlock( &access_next ); }
  } * data ;

  static void * supervising( void * obj );

  pthread_t supervisor ;
  XVDrawWindowX<T> * worker ;
  bool to_free ;
  bool supervised ;
  float factorX, factorY ;

  /** get the size and position information from worker */
  void fetch_size(); 

 private:
  void init() ;

 public:
  /** If you call this constructor, make sure the object xwin will not 
      be destroyed before this XVRemoteWindowX object. */
  XVRemoteWindowX( XVDrawWindowX<T>& xwin, 
		   float scaleX = 1.0, float scaleY = 1.0 );
  XVRemoteWindowX( const XVImageRGB<T> & im, 
		   int px = 0, int py = 0, char * title = NULL,
		   int event_mask = 0, char * disp = NULL, 
		   int num_buf = 2, int double_buf = 1, 
		   int function = GXcopy );
  XVRemoteWindowX( const XVImageRGB<T> & im, 
		   float scaleX, float scaleY,
		   int px = 0, int py = 0, char * title = NULL,
		   int event_mask = 0, char * disp = NULL, 
		   int num_buf = 2, int double_buf = 1, 
		   int function = GXcopy );
  XVRemoteWindowX( int w, int h, 
		   int px = 0, int py = 0, char * title = NULL,
		   int event_mask = 0, char * disp = NULL, 
		   int num_buf = 2, int double_buf = 1, 
		   int function = GXcopy );
  XVRemoteWindowX( int w, int h, 
		   float scaleX, float scaleY,
		   int px = 0, int py = 0, char * title = NULL,
		   int event_mask = 0, char * disp = NULL, 
		   int num_buf = 2, int double_buf = 1, 
		   int function = GXcopy );

  virtual ~XVRemoteWindowX();

  virtual void map(void) { worker->map() ; }
  virtual void unmap(void) { worker->unmap(); }

  virtual void setTitle( const char* title ) { worker->setTitle( title ); }
  virtual void resize( const XVSize& size ) 
    { worker->resize(size); fetch_size(); }

  virtual void setImages( XVImageRGB<T> * frames, int count );
  virtual void CopyImage( int which, u_short flip = 0 );
  virtual void ClearWindow(void);
  virtual void CopySubImage( const XVImageRGB<T>& image, bool flip=0 );

  virtual void swap_buffers(void);
  virtual void flush(void);

  virtual XVImageRGB<T> getDisplayedImage( int px, int py, int w, int h );

  /** This function is to get the image (not including the drawings)
      that last sent to the X server */
  virtual XVImageRGB<T> getLastImage(void);

  /** This function is to get the image (not including the drawings)
      that was displaying when events (returned by check_events) occurs */
  virtual XVImageRGB<T> getEventsImage(void);

  virtual int check_events( int *ret_filed );

  virtual int drawPoint( int x, int y, XVDrawColor c = DEFAULT_COLOR );
  virtual int drawLine( int x1, int y1, int x2, int y2, 
			XVDrawColor c = DEFAULT_COLOR,int line_width=0 );
  virtual int drawRectangle( int x, int y, int w, int h,
			     XVDrawColor c = DEFAULT_COLOR );
  virtual int fillRectangle( int x, int y, int w, int h,
			     XVDrawColor c = DEFAULT_COLOR );
  virtual int drawEllipse( int x, int y, int w, int h,
			   XVDrawColor c = DEFAULT_COLOR );
  virtual int fillEllipse( int x, int y, int w, int h,
			   XVDrawColor c = DEFAULT_COLOR );
  virtual int drawString( int x, int y, char * string, int length,
			  XVDrawColor c = DEFAULT_COLOR );

  virtual void addColor( XVDrawColor c ) { worker->addColor(c); }
  virtual void setXOR() { worker->setXOR(); }
  virtual void setCOPY() { worker->setCOPY(); }

  /** These are hooks for use by the Haskell people. 
      For detailed information, see comments in XVWindows.h
      X requests go through these hooks are NOT buffered ! */
  virtual Display * getXDisplay(void) { return worker->getXDisplay(); }
  virtual Drawable getXDrawable(void) { return worker->getXDrawable(); }  

  // Due to the bad design of XVWindow (deriving from XVImageBase),
  // member functions of XVSize, etc has to be overridden here.
  // This won't work if base class pointer/reference is used, so
  // fetch_size() is used as a backup
  int Width() const { return worker->Width(); }
  int Height() const { return worker->Height(); }
  int Rows() const { return worker->Rows(); }
  int Columns() const { return worker->Columns(); }
  const int& PosX() const { return worker->PosX(); }
  const int& PosY() const { return worker->PosY(); }
  const int& x() const { return worker->x(); }
  const int& y() const { return worker->y(); }
  // the list above is far from complete, and please add entries if used
};

#endif //_XVREMOTEWINDOWX_H_

