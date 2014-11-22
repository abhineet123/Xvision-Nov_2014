// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVWINDOWX_H_
#define _XVWINDOWX_H_
/*
 * $Author: burschka $
 * $Log: XVWindowX.h,v $
 * Revision 1.1.1.1  2008/01/30 18:44:05  burschka
 *
 *
 * Revision 1.45  2005/06/17 17:05:26  cvsuser
 * Donald: added setTitle function and corrected a constness issue
 *
 * Revision 1.44  2005/02/22 02:35:53  cvsuser
 * Donald: Trying to make XVision work under gcc 3.4
 *
 * Revision 1.43  2004/11/19 17:50:13  cvsuser
 * *** empty log message ***
 *
 * Revision 1.42  2004/11/18 20:26:18  cvsuser
 * selectPoint can now be used to just get the mouse-click position by
 * setting the draw_flag to false. It is set default to true for backward
 * compatibility
 *
 * D
 *
 * Revision 1.41  2004/07/19 04:10:14  cvsuser
 * Cleanup for correct search of X11 headers ...
 *
 * Darius
 *
 * Revision 1.40  2004/02/25 00:39:18  vsbook
 * Donald: minor changes to get riddle of some compiler warnings
 *
 * Revision 1.39  2004/02/18 03:11:17  vsbook
 * Changes done for the Tool tracking code
 *                   by Maneesh 02/17/2004
 *
 * Revision 1.38  2002/11/28 20:55:23  xvis
 * Donald: removed most gcc3.2 warnings :P
 *
 * Revision 1.37  2002/08/02 20:06:13  xvis
 * Donald: make things compatible with icc
 *
 * Revision 1.36  2002/07/02 19:15:55  xvis
 * Flip function for CopySubImage() added and fix the flip bug in
 * CopyImage()
 *
 * Darius
 *
 * Revision 1.35  2002/02/21 16:38:02  xvis
 * added base documentation
 * -jeremy
 *
 * Revision 1.34  2002/02/20 01:29:20  xvis
 * Expose is passed to the user as well (if requested)
 *
 * darius
 *
 * Revision 1.33  2002/02/20 01:23:54  xvis
 * event_mask is now part of the XVWindowX class
 *
 * Darius
 *
 * Revision 1.32  2002/02/11 17:59:14  xvis
 * Donald: re-implement ClearWindow using XFillRectangle
 *
 * Revision 1.31  2002/02/08 01:55:19  xvis
 * Donald: fixed a bug when calling addColor from XVDrawWindow::XVDrawWindow()
 *
 * Revision 1.30  2002/01/31 20:48:42  xvis
 * Donald: added a XVDrawWindow class to be an additional interface layer
 *
 * Revision 1.29  2002/01/26 22:11:39  xvis
 * Donald: Eliminate most annoying warnings.
 *
 * Revision 1.28  2002/01/24 21:31:42  xvis
 * Donald: added XVRemoteWindowX class and removed a bug in XVDrawWindow.cc
 *
 * Revision 1.27  2001/07/25 01:12:59  xvis
 * SLANG::7.24.01.  Fixed a memory leak problem with XVDrawColor
 *
 * Revision 1.26  2001/07/15 07:16:45  xvis
 * SLANG::7.15.01.  Added size parameters to XVWindow..X constructors
 *
 * Revision 1.24  2001/05/15 03:29:08  xvis
 * fixed a misplaced #include <config.h>
 *
 * Revision 1.23  2001/05/12 21:47:56  xvis
 * SLANG::5.12.01.  Was forgetting to free GC's in XVDrawWindow.  Caused a memory leak. :(
 *
 * Revision 1.22  2001/05/04 16:08:19  xvis
 * SLANG::5.4.01. Added selectAngledRect function.
 *
 * Revision 1.21  2001/04/30 17:08:42  xvis
 * SLANG::4.30.01.  Added a correct getDisplayedImage function (now gets from the screen).
 *
 * Revision 1.20  2001/04/29 17:22:15  xvis
 * SLANG::4.29.01.  Fixed a few bugs in XVInteractWindowX.
 *
 * Revision 1.19  2001/04/26 02:18:05  xvis
 * SLANG::4.25.01.  Added operator[] to XVStateWindowX
 *
 * Revision 1.18  2001/04/25 19:52:53  xvis
 * SLANG::4.25.01.  Fixed final bugs in XVThreadedWindowX.  Everything should work now.
 *
 * Revision 1.14  2001/04/24 12:41:15  xvis
 * SLANG::4.24.01.  Added threaded selection stuff.  still buggy...
 *
 * Revision 1.13  2001/04/18 08:59:40  xvis
 * SLANG::4.18.01. Added a selectSizedRect() function.
 *
 * Revision 1.12  2001/04/11 17:02:13  xvis
 * SLANG::4.11.01.  Made a bunch of changes.  Mostly to SSD and tracking stuff.
 *
 * Revision 1.11  2001/03/12 05:43:59  xvis
 * XClearWindow function added to clear the window content
 *
 * Darius
 *
 * Revision 1.10  2001/03/08 18:56:28  xvis
 * Restored the original version of getXDrawable now - fixing the
 * flickering problem.
 *
 * Revision 1.9  2001/03/08 17:58:42  xvis
 * The bugs I was chasing yesterday were due to opening 2 separate
 * connections to the X server and then trying to manipulate the same
 * objects through two separate connections.  One of the consequences of
 * trying to use 2 separate X-related libraries.  The fix was to add this
 * method to XVWindowX<...> and modify the graphics library.
 *
 *   Display* getXDisplay(void)
 *
 * Revision 1.8  2001/03/07 19:30:58  xvis
 * Added a method to support the Haskell Hackers (lets you see the XWindow (or backing buffer) inside the XVWindow
 *
 * Revision 1.7  2001/03/05 11:08:54  xvis
 * SLANG::3.5.1.  Added setCOPY and setXOR functions to XVDrawable.
 *
 * Revision 1.6  2001/03/01 10:10:12  xvis
 * SLANG::3.1.1.  removed templates from XVInteractive and XVDrawable.
 *
 * Revision 1.5  2001/03/01 09:57:34  xvis
 * SLANG::3.1.1.  Added an include for XVimageRGB
 *
 * Revision 1.4  2001/02/21 15:02:55  xvis
 * SLANG::2.21.01.  Added a currentImage member to XVWindow
 *
 * Revision 1.3  2001/02/21 12:32:28  xvis
 * SLANG::2.21.01.  Added virtuals to functions in XVWindowX
 *
 * Revision 1.2  2001/02/21 12:19:39  xvis
 * SLANG::2.21.01.  added using statements to XVDrawWindowX and XVInteractWindowX.
 *
 * Revision 1.1  2001/02/21 12:07:50  xvis
 * SLANG::2.21.01.  Changed structure of Consoles stuff.  XVWindow is now an abstract window for all windows.
 *
 * Revision 1.16  2001/02/05 07:15:30  xvis
 * SLANG::2.5.01.  Added functionality for displaying and interactive selection.
 *
 * Revision 1.15  2001/02/02 04:45:39  xvis
 * SLANG::2.1.01.  Fixed double buffering problem, but now double buffering may
 * not work on RedHat 6.1.  (but at least it works on both 6.2 & 7!)  Replaced
 * the HAVE_LIBXDPMS with HAVE_LIBXEXT.  the Xdpms library apparently is wrapped
 * in the Xext library now.
 *
 * Revision 1.14  2000/10/31 20:18:54  xvis
 * Depth problem in XVWindow solved ???
 *
 * Revision 1.13  2000/10/24 22:27:01  xvis
 * *** empty log message ***
 *
 * Revision 1.12  2000/10/24 22:22:13  xvis
 * *** empty log message ***
 *
 * Revision 1.11  2000/09/13 15:04:16  xvis
 * slang.  Darius (and me) added checks in CopyImage & CopySubImage that wait for the XServer to say its ready to receive stuff to display in the window.  Used XNextEvent and Expose...9.13.00.
 *
 * Revision 1.10  2000/09/11 15:37:56  xvis
 * slang.  added millisecond sleep in map so that image is displayed. 9.11.00.
 *
 * Revision 1.9  2000/08/31 18:25:50  xvis
 * increased maximum possible size of a window. SVM
 *
 * Revision 1.8  2000/07/27 22:01:18  xvis
 * *** empty log message ***
 *
 * Revision 1.7  2000/07/27 18:26:15  xvis
 * fixed for multiple definitions of XVPixmap. SVM
 *
 * Revision 1.6  2000/07/27 15:22:01  xvis
 * shared_memory only active on local display, but remote displays
 * possible now
 * clean-up in double-buffering
 *
 * Revision 1.5  2000/07/12 13:54:57  hager
 * Fixed a number of bugs in Images etc. etc. etc
 *
 * Revision 1.4  2000/06/26 19:40:37  hager
 * Broke images into several ssubclasses and did various modifications to
 * make that work.
 *
 * Revision 1.3  2000/06/22 16:03:34  xvis
 * iterators added.
 *
 * Revision 1.9  2000/05/22 16:39:59  cips
 * clean-up in buffer locking. Pointer-handle still not included.
 * Warnings in XVWindow removed.
 *
 * Darius
 *
 * Revision 1.8  2000/04/06 15:18:50  cips
 * my version of the locking
 *
 * Revision 1.7  2000/03/14 03:50:46  cips
 * Greg's bugfixes in image conversion (uups, these were not only a few)
 *
 * Revision 1.6  2000/03/10 00:16:38  cips
 * Pixmap structure added. TODO: GC managment for operations.
 *
 * Revision 1.5  2000/02/29 02:41:47  cips
 * *** empty log message ***
 *
 * Revision 1.4  2000/02/15 23:45:35  cips
 * *** empty log message ***
 *
 * Revision 1.3  2000/02/11 19:41:33  cips
 * Real game as pong2
 *
 * Revision 1.2  2000/02/05 00:20:26  cips
 * Sun compatible now
 *
 * Revision 1.1.1.1  2000/02/04 18:27:50  cips
 *
 *
 */

#include <config.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#ifdef HAVE_LIBXEXT
#include <X11/extensions/Xdbe.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#ifdef HAVE_LIBXEXT
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include <XVImageRGB.h>
#include <XVWindow.h>

#include <XVDrawable.h>
#include <XVInteractive.h>

#include <pthread.h>
#include <map>
#include <string>
#include <X11/cursorfont.h>
#include <string>

using namespace std ;

#define  EXPOSE		 (1<<9)
#define  RESIZED         (1<<8)
#define  KEY_RELEASED    (1<<7)
#define  KEY_PRESSED     (1<<6)
#define  POINTER_MOVED   (1<<5)
#define  BUTTON_RELEASED (1<<4)
#define  BUTTON_PRESSED  (1<<3)
#define  MIDDLE_BUTTON   (1<<2)
#define  RIGHT_BUTTON    (1<<1)
#define  LEFT_BUTTON	 (1<<0)

#define  DRAW_COLORS	16
#define  BW_MAP		0
#define  DRAW_MAP	1
#define REMAP

#define  MAX_FONT_NUM	5

/**  base class for all windows used in X  */
template <class T>
class XVWindowX: virtual public XVWindow<T> {
 protected:
  using XVWindow<T>::width ;
  using XVWindow<T>::height ;
  using XVWindow<T>::posx ;
  using XVWindow<T>::posy ;
  using XVWindow<T>::windowTitle ;
  using XVWindow<T>::frames_buf ;

 protected:

   Display		*dpy;
   Window		window;
   const T			**shared_fields;
   bool			own_buffers;
   XImage	        **shm_images;
   XImage		*flip_image;
   XImage		*def_image;
#ifdef HAVE_LIBXEXT
   XShmSegmentInfo      *def_shminfo;   // shared memory segment
#endif
   int			event_mask;
   int		        buffer_count;
   int			bitmap_pad;	// screen depth
   u_short		flip;

   XGCValues		xgcv;           // settings for graphics context
   unsigned long	bw_color[256];
   unsigned long	draw_color[DRAW_COLORS+2];
   GC			gc_window[DRAW_COLORS];  //graphics context
   GC			gc_clear;
   void			get_colors(void);
   u_short		back_flag;	// double buffering avail?
   u_short		shared_memory;
   int			num_fonts;
   Font			*font;
#ifdef HAVE_LIBXEXT
   XdbeBackBuffer       back_buffer;   // the back buffer if double-buffering
   XdbeSwapInfo		swap_info;     // info used when swapping buffers
#else
   int			back_buffer;
#endif
   
   bool                 exposeEvent;
   
  public:

   /** display window on screen*/
   virtual void		map(void) { if(window)
			      { XMapWindow(dpy,window); XFlush(dpy); } };

   /** erase window and descendants from screen */
   virtual void		unmap(void) { if(window)
                              { XUnmapWindow(dpy,window); XFlush(dpy); } };
   
   /* resize window */
   virtual void		resize( const XVSize& );
   virtual void         resize( int w, int h ) { resize( XVSize(w,h) ); }

   int			check_events(int *ret_field);

   int			get_font(char *name);

   int			print_string(char *string,int x, int y,
                                       int col_index=0,
				       int font_index=0);

   /** if double-buffering, then swap the back buffer w/ the front */
   virtual void		swap_buffers(void)
#ifdef HAVE_LIBXEXT 
				{ if(back_flag)
   				    XdbeSwapBuffers(dpy,&swap_info,1); };
#else	
				{};
#endif

   virtual void         setTitle( const char *title = 0 );

   virtual void		setImages(XVImageRGB<T> *, int count);
   virtual void	        CopyImage(int which,u_short flip=0);

   virtual void 	ClearWindow(void) {
#ifdef HAVE_LIBXEXT
     if( back_flag ) {
       XFillRectangle( dpy, back_buffer, gc_clear, 0, 0, width, height );
     }else 
#endif
       XClearWindow(dpy,window);
   }


   virtual void		CopySubImage(const XVImageRGB<T> &,bool flip=0);

   virtual void         flush(void){XSync(dpy,0);};

   void                 expose(void) { exposeEvent = true; };

   const T		** get_shared_fields(void) const {
     			   return (const T**)shared_fields;
   };

   /** construct XVWindowX with XVImageRGB parameter */
   XVWindowX(const XVImageRGB<T> &, 
	     int posx = 0,
	     int posy = 0, 
	     char * title = NULL, 
	     int event_mask=0,
	     char * display = NULL,
	     int num_buf = 2,
	     int double_buf = 0);

   /** construct XVWindowX with specified width and height */
   XVWindowX(int w, int h,
	     int posx = 0,
	     int posy = 0, 
	     char * title = NULL, 
	     int event_mask=0,
	     char * display = NULL,
	     int num_buf = 2,
	     int double_buf = 0);
   
   virtual ~XVWindowX(void);

   XVImageRGB<T> getDisplayedImage(int px = 0, int py = 0, 
				   int w = -1, int h = -1);

   /** This is a hook for use by the Haskell people to let them use
       their existing graphics library on this window.
       The idea is that they copy an image over, do some graphics
       on this drawable and then swap buffers. */
   Display *		getXDisplay(void) { return dpy; };
   Drawable		getXDrawable(void) { 
#ifdef HAVE_LIBXEXT
     return back_flag ? back_buffer : window; 
#else
     return window;
#endif
   };
};

class CannotAllocColorException : public XVException { 
 public:
  CannotAllocColorException() : XVException((char*)"CannotAllocColorException: Could Not Allocate the specified color") {}
  CannotAllocColorException(char * err) : XVException(err){}
};


/** subclass of XVWindowX that has basic shape drawing functionality */
template <class PIXEL>
class XVDrawWindowX : public XVDrawWindow<PIXEL>, 
                      public XVWindowX<PIXEL> {
  
  typedef std::map<string, GC>::iterator MI;
 protected:
  using XVWindowX<PIXEL>::width ;
  using XVWindowX<PIXEL>::height ;
  using XVWindowX<PIXEL>::posx ;
  using XVWindowX<PIXEL>::posy ;
  using XVWindowX<PIXEL>::dpy ;
  using XVWindowX<PIXEL>::gc_window ;
  using XVWindowX<PIXEL>::back_flag ;
  using XVWindowX<PIXEL>::back_buffer ;
  using XVWindowX<PIXEL>::window ;

 public:
  using XVWindowX<PIXEL>::map;
  using XVWindowX<PIXEL>::unmap;
  using XVWindowX<PIXEL>::resize;
  using XVWindowX<PIXEL>::setImages;
  using XVWindowX<PIXEL>::CopyImage;
  using XVWindowX<PIXEL>::CopySubImage;
  using XVWindowX<PIXEL>::swap_buffers;
  using XVWindowX<PIXEL>::flush;

 protected:
  
  std::map<string, GC> GCmap;
  int function;

  void setFunction(int);
  
 public:
  
  XVDrawWindowX(const XVImageRGB<PIXEL > & im, int px = 0, int py = 0,
		char * title = NULL,
		int event_mask = 0, char * disp = NULL, 
		int num_buf = 2, int double_buf = 1, int function = GXcopy);

  XVDrawWindowX(int w, int h, int px = 0, int py = 0,
		char * title = NULL,
		int event_mask = 0, char * disp = NULL, 
		int num_buf = 2, int double_buf = 1, int function = GXcopy);

  ~XVDrawWindowX();

  void setXOR(){ function = GXxor; setFunction(function); }
  void setCOPY(){ function = GXcopy; setFunction(function); }
  
  virtual void addColor(XVDrawColor);
  
  virtual int drawPoint(int x, int y, 
			XVDrawColor c = DEFAULT_COLOR);
  virtual int drawLine(int x1, int y1, int x2, int y2, 
		       XVDrawColor c = DEFAULT_COLOR, int line_width=0);
  virtual int drawRectangle(int x, int y, int w, int h, 
			    XVDrawColor c = DEFAULT_COLOR);
  virtual int drawEllipse(int x, int y, int w, int h, 
			  XVDrawColor c = DEFAULT_COLOR);
  virtual int fillRectangle(int x, int y, int w, int h, 
			    XVDrawColor c = DEFAULT_COLOR);
  virtual int fillEllipse(int x, int y, int w, int h, 
			  XVDrawColor c = DEFAULT_COLOR);
  virtual int drawString(int x, int y, char * str, int length, 
			 XVDrawColor c = DEFAULT_COLOR);
  
};


/** base class for objects that are drawable in any XVDrawWindowX */
class XVDrawObjectX {

 public:

  Display * disp;
  Window win;
  GC     context;

  XVDrawObjectX(Display * d, Window w, GC g) : disp(d), win(w), context(g) {}
  void reset(Display * d, Window w, GC g) {
    disp = d;  win = w; context = g;
  };
  
  virtual ~XVDrawObjectX() {}
};

/** point object for XVDrawWindowX */
class XVDrawPointX : public virtual XVDrawPoint, public XVDrawObjectX {

 public:

  XVDrawPointX(Display * d, Window w, GC g, 
	       XVPosition p, XVDrawColor c = DEFAULT_COLOR) :
    XVDrawPoint(p, c), XVDrawObjectX(d, w, g) {}

  virtual ~XVDrawPointX () {}

  virtual void draw() {
    XDrawPoint(disp, win, context,
	       this->point.PosX(), this->point.PosY());
  }
};


/** line object for XVDrawWindowX */
class XVDrawLineX : public virtual XVDrawLine, public XVDrawObjectX {

 public:

  XVDrawLineX(Display * d, Window w, GC g, 
	      XVPosition p1, XVPosition p2, XVDrawColor c = DEFAULT_COLOR) :
    XVDrawLine(p1, p2, c), XVDrawObjectX(d, w, g) {}

  virtual ~XVDrawLineX () {}

  virtual void draw() {
    XDrawLine(disp, win, context, 
	      this->point1.PosX(), this->point1.PosY(),
	      this->point2.PosX(), this->point2.PosY());
  }
};


/** rectangle object for XVDrawWindowX */
class XVDrawRectX : public virtual XVDrawRect, public XVDrawObjectX {

 public:

  XVDrawRectX(Display * d, Window w, GC g, XVImageGeneric r, 
	      bool f = false, XVDrawColor c = DEFAULT_COLOR) :
    XVDrawRect(r, f, c), XVDrawObjectX(d, w, g) {}

  virtual ~XVDrawRectX () {}

  virtual void draw() {
    if(this->fill) {
      XFillRectangle(disp, win, context,
		     this->region.PosX(), this->region.PosY(),
		     this->region.Width(), this->region.Height());
    }else{
      XDrawRectangle(disp, win, context,
		     this->region.PosX(), this->region.PosY(),
		     this->region.Width(), this->region.Height());
    }
  }
};


/** ellipse object for XVDrawWindowX */
class XVDrawEllipseX : public virtual XVDrawEllipse, public XVDrawObjectX {

 public:

  XVDrawEllipseX(Display * d, Window w, GC g, 
		 XVImageGeneric r, bool f = false, XVDrawColor c = DEFAULT_COLOR) :
    XVDrawEllipse(r, f, c), XVDrawObjectX(d, w, g) {}
  
  virtual ~XVDrawEllipseX () {}

  virtual void draw() {
    if(this->fill){
      XFillArc(disp, win, context, 
	       this->region.PosX(), this->region.PosY(),
	       this->region.Width(), this->region.Height(), 0, 360*64);
    }else{
      XDrawArc(disp, win, context,
	       this->region.PosX(), this->region.PosY(),
	       this->region.Width(), this->region.Height(), 0, 360*64);
    }
  }	    
};


/** string object for XVDrawWindowX */
class XVDrawStringX : public virtual XVDrawString, public XVDrawObjectX {

 public:

  XVDrawStringX(Display * d, Window w, GC g, 
		XVPosition p, char * s, int l, XVDrawColor c = DEFAULT_COLOR) :
    XVDrawString(p, s, l, c), XVDrawObjectX(d, w, g) {}

  virtual ~XVDrawStringX () {}

  virtual void draw() {
    XDrawString(disp, win, context, this->pos.PosX(), this->pos.PosY(),
		this->string, this->length);
  };
};

#include <vector>

/** subclass of XVDrawWindowX that allows user to obtain state of window
    at a moment in time -> has access to all objects drawn in window, etc. 
*/
template <class PIXEL>
class XVStateWindowX : public XVDrawWindowX<PIXEL> {
 protected:
  using XVDrawWindowX<PIXEL>::dpy ;
  using XVDrawWindowX<PIXEL>::gc_window ;
  using XVDrawWindowX<PIXEL>::back_flag ;
  using XVDrawWindowX<PIXEL>::back_buffer ;
  using XVDrawWindowX<PIXEL>::window ;
  using XVDrawWindowX<PIXEL>::GCmap ;

 protected:

  typedef vector<XVDrawObject *>::iterator OBJ_ITER;
  vector<XVDrawObject *> drawObjects;

 public:

  XVStateWindowX(const XVImageRGB<PIXEL > & im, int px = 0, int py = 0,
		 char * title = NULL,
		 int event_mask = 0, char * disp = NULL, 
		 int num_buf = 2, int double_buf = 1, int function = GXcopy) :
  XVDrawWindowX<PIXEL >(im, px, py, title, event_mask, disp, num_buf,
			double_buf, function),
  XVWindow<PIXEL>(im.Width(),im.Height()) {}

  XVStateWindowX(int w, int h, int px = 0, int py = 0,
		 char * title = NULL,
		 int event_mask = 0, char * disp = NULL, 
		 int num_buf = 2, int double_buf = 1, int function = GXcopy) :
  XVDrawWindowX<PIXEL >(w, h, px, py, title, event_mask, disp, num_buf,
			double_buf, function),
  XVWindow<PIXEL>(w,h) {}
  
  void CopySubImage(const XVImageRGB<PIXEL > &);
  void CopyImage(int, u_short);
    
  inline virtual int drawPoint(int x, int y, 
			       XVDrawColor c = DEFAULT_COLOR);
  inline virtual int drawLine(int x1, int y1, int x2, int y2, 
			      XVDrawColor c = DEFAULT_COLOR, int line_width=0);
  inline virtual int drawRectangle(int x, int y, int w, int h, 
				   XVDrawColor c = DEFAULT_COLOR);
  inline virtual int drawEllipse(int x, int y, int w, int h, 
				 XVDrawColor c = DEFAULT_COLOR);
  inline virtual int fillRectangle(int x, int y, int w, int h, 
				   XVDrawColor c = DEFAULT_COLOR);
  inline virtual int fillEllipse(int x, int y, int w, int h, 
				 XVDrawColor c = DEFAULT_COLOR);
  inline virtual int drawString(int x, int y, char * str, int length, 
				XVDrawColor c = DEFAULT_COLOR);
  
  inline virtual void erase(int i);
  inline virtual void clear();
  inline virtual XVDrawObject & getDrawObj(int i) { return (*drawObjects[i]); }
  inline virtual vector<XVDrawObject *> getAll() { return drawObjects; }

  inline virtual XVDrawObject & operator [] (int i) { return (*drawObjects[i]); }
};  

/** subclass of XVDrawWindowX that allows for user interaction (selection of
    objects)
 */
template <class PIXEL>
class XVInteractWindowX : public XVInteractive, public XVDrawWindowX<PIXEL> {
 protected:
  using XVDrawWindowX<PIXEL>::dpy ;
  using XVDrawWindowX<PIXEL>::window ;

 public:
  using XVDrawWindowX<PIXEL>::map;
  using XVDrawWindowX<PIXEL>::unmap;
  using XVDrawWindowX<PIXEL>::resize;
  using XVDrawWindowX<PIXEL>::setImages;
  using XVDrawWindowX<PIXEL>::CopyImage;
  using XVDrawWindowX<PIXEL>::CopySubImage;
  using XVDrawWindowX<PIXEL>::swap_buffers;
  using XVDrawWindowX<PIXEL>::flush;

  using XVDrawWindowX<PIXEL>::addColor;

 protected:
  
  Cursor crossHair, arrow;
  
 public:
  
  XVInteractWindowX(const XVImageRGB<PIXEL > & im, int px = 0, int py = 0, 
		    char * title = NULL,
		    int event_mask = 0, char * disp = NULL,
		    int num_buf = 2, int double_buf = 1, int function = GXxor);

  XVInteractWindowX(int w, int h, int px = 0, int py = 0, 
		    char * title = NULL,
		    int event_mask = 0, char * disp = NULL,
		    int num_buf = 2, int double_buf = 1, int function = GXxor);

  ~XVInteractWindowX();
  
  virtual void selectPoint(XVPosition &, XVDrawColor c = DEFAULT_COLOR,bool draw_flag=true);
  virtual void selectLine(XVPosition &, XVPosition &, XVDrawColor c = DEFAULT_COLOR);
  virtual void selectRectangle(XVImageGeneric &, 
			       XVDrawColor c = DEFAULT_COLOR);
  virtual void selectEllipse(XVImageGeneric &, 
			     XVDrawColor c = DEFAULT_COLOR);

  virtual void selectSizedRect(XVImageGeneric &,
			       const XVSize &, 
			       XVDrawColor c = DEFAULT_COLOR);

  virtual void selectAngledRect(XVPosition &, XVSize &, double &,
				XVDrawColor c = DEFAULT_COLOR);
    
  virtual void selectAngledRect(XVPosition &, XVPosition &, XVSize &, 
				double &, XVDrawColor c = DEFAULT_COLOR);

};


#include <XVException.h>


class XVThreadedWindowException : public XVException {
 public:
  XVThreadedWindowException() : XVException() {}
  XVThreadedWindowException(char * str) : XVException(str) {}
};

#include <XVVideo.h>

/** subclass of XVStateWindowX that allows window to be run in separate thread
 */
template <class PIXEL, class IN_IMAGE>
class XVThreadedWindowX : public XVInteractive, public XVStateWindowX<PIXEL> {
 protected:
  using XVStateWindowX<PIXEL>::dpy ;
  
 public:
  using XVStateWindowX<PIXEL>::map;
  using XVStateWindowX<PIXEL>::unmap;
  using XVStateWindowX<PIXEL>::resize;
  using XVStateWindowX<PIXEL>::setImages;

  using XVStateWindowX<PIXEL>::addColor;

 protected:

  XVVideo<IN_IMAGE > * vid;

  Cursor crossHair, arrow;
  
  typedef void * VOIDP;

  bool hasBeenSelected;
  bool currentlySelecting;
  pthread_mutex_t selectMutex;
  
  XVDrawObject * selectedObject;
  
 public:
  
  static void * selectPointThreadFunc(void *);
  static void * selectLineThreadFunc(void *);
  static void * selectRectThreadFunc(void *);
  static void * selectEllipseThreadFunc(void *);
  static void * selectSizedRectThreadFunc(void *);
  static void * selectAngledRectThreadFunc(void *);

  XVThreadedWindowX(XVVideo<IN_IMAGE > * v, 
		    const XVImageRGB<PIXEL > & im, 
		    int px = 0, int py = 0, 
		    char * title = NULL,
		    int event_mask = 0, char * disp = NULL,
		    int num_buf = 2, int double_buf = 1, 
		    int function = GXxor) :
    XVStateWindowX<PIXEL>(im, px, py, title, event_mask, disp, num_buf, 
			  double_buf, function), 
    XVWindow<PIXEL>(im.Width(),im.Height()), selectedObject(NULL), vid(v) {

    crossHair = XCreateFontCursor(dpy, XC_crosshair);
    arrow     = XCreateFontCursor(dpy, XC_arrow);

    pthread_mutex_init( & selectMutex, NULL ); 
    currentlySelecting = false;
  };

  XVThreadedWindowX(XVVideo<IN_IMAGE > * v, 
		    int w, int h, 
		    int px = 0, int py = 0, 
		    char * title = NULL,
		    int event_mask = 0, char * disp = NULL,
		    int num_buf = 2, int double_buf = 1, 
		    int function = GXxor) :
    XVStateWindowX<PIXEL>(w, h, px, py, title, event_mask, disp, num_buf, 
			  double_buf, function),
    XVWindow<PIXEL>(w,h), selectedObject(NULL), vid(v) {

    crossHair = XCreateFontCursor(dpy, XC_crosshair);
    arrow     = XCreateFontCursor(dpy, XC_arrow);

    pthread_mutex_init( & selectMutex, NULL ); 
    currentlySelecting = false;
  };
  
  void setVideoDevice(XVVideo<IN_IMAGE > * v) { vid = v; }

  virtual void CopySubImage(const XVImageRGB<PIXEL> &);
  virtual void CopyImage(int which, u_short);
  virtual void swap_buffers();
  virtual void flush();

  inline virtual void selectPoint(XVPosition &, XVDrawColor c = DEFAULT_COLOR,bool draw_flag=true);
  inline virtual void selectLine(XVPosition &, XVPosition &, 
				 XVDrawColor c = DEFAULT_COLOR);
  inline virtual void selectRectangle(XVImageGeneric &, 
				      XVDrawColor c = DEFAULT_COLOR);
  inline virtual void selectEllipse(XVImageGeneric &,
				    XVDrawColor c = DEFAULT_COLOR);

  inline virtual void selectSizedRect(XVImageGeneric &, const XVSize &, 
				      XVDrawColor c = DEFAULT_COLOR);

  inline virtual void selectAngledRect(XVPosition &, XVSize &, double &,
				       XVDrawColor c = DEFAULT_COLOR);

};
  
#endif
