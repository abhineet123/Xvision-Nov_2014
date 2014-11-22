// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <config.h>

#ifdef HAVE_LIBX11

#include <stdio.h>
#include <stdlib.h>
#include <XVWindowX.h>

static char * color_names[] =
{
  (char*)"black",(char*)"white",(char*)"tourquise1",(char*)"green",(char*)"brown",(char*)"orange",
  (char*)"violet",(char*)"red",(char*)"dark green",(char*)"aquamarine",
  (char*)"purple",(char*)"pink",(char*)"yellow",(char*)"blue",(char*)"peru",(char*)"burlywood"
};

bool operator < (const XVDrawColor & c1, const XVDrawColor & c2) {
  return c1.colorName < c2.colorName;
};

bool operator == (const XVDrawColor & c1, const XVDrawColor & c2) {
  return c1.colorName == c2.colorName;
};

template <class T>
void XVWindowX<T>::setTitle( const char * title ) {
  if (title != NULL){
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)title,
		    strlen(title));
    windowTitle = title;
  }else{
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)"XVision(R)CIRL/TUM- Window",
		    strlen("XVision(R)CIRL/TUM- Window"));
    windowTitle = "XVision(R)CIRL- Window";
  }
}

template <class T>
void XVWindowX<T>::setImages(XVImageRGB<T> * frames, int count) {
  int i;

  own_buffers=false;
  assert(count<=MAX_NUM_IMAGES);
  shared_fields=new const T *[count];
  if(shared_memory)
  {
    cerr << "Warning: usage of XVWindowX::setImages() is deprecated." << endl
	 << "Be prepared to program crashes or abnormal behaviors." << endl ;
#ifdef HAVE_REMAP
  XShmSegmentInfo      *shminfo;

  for (i=0;i<count;i++) {
    shminfo= new XShmSegmentInfo;
    shm_images[i] = XShmCreateImage(dpy,
                                    DefaultVisual(dpy,DefaultScreen(dpy)),
                                    DefaultDepth(dpy,DefaultScreen(dpy)),
                                    ZPixmap, NULL, shminfo,width,height);
    shminfo->shmid = shmget(IPC_PRIVATE,
                            shm_images[i]->bytes_per_line*shm_images[i]->height,
                            IPC_CREAT | 0777);
    shminfo->shmaddr =
	    (char *) shmat(shminfo->shmid,frames[i].data(),SHM_REMAP|SHM_RND);
    shm_images[i]->data = shminfo->shmaddr;
    shared_fields[i]=(T*)shminfo->shmaddr;
    XShmAttach(dpy, shminfo);
    def_shminfo = shminfo;
    shmctl(shminfo->shmid,IPC_RMID,NULL);
    }
#endif
  }
  else{
    for (i=0;i<count;i++) {
      shm_images[i] = XCreateImage(dpy,
                                 DefaultVisual(dpy,DefaultScreen(dpy)),
                                 DefaultDepth(dpy,DefaultScreen(dpy)),
                                 ZPixmap, 0, (char*)frames[i].pixData(),
				 width,height,BitmapPad(dpy),0);
      shared_fields[i]=frames[i].pixData();
    }
    flip_image = XCreateImage(dpy,
                                 DefaultVisual(dpy,DefaultScreen(dpy)),
                                 DefaultDepth(dpy,DefaultScreen(dpy)),
                                 ZPixmap, 0, (char*)new T[width*height],
				 width,height,BitmapPad(dpy),0);
  }
  frames_buf=frames;
}

template <class T>
void XVWindowX<T>::CopyImage(int which,u_short s_flip)
{
  XImage   *im= shm_images[which];
  const T  *from;
  T	   *to;
  int i,j;

  if(!exposeEvent){
    XEvent xev;
    XMaskEvent(dpy,ExposureMask, & xev);
    expose();
  }

  flip=s_flip;
  if(flip)
  {
    if(!shared_memory) im=flip_image;
    for(j=0,from=frames_buf[which].pixData(),
	    to=((T *)im->data)+width;
	   j<height;j++,to+=2*frames_buf[which].SizeX())
      for(i=0;i<frames_buf[which].SizeX();i++) *(--to)=*from++;
  }
#ifdef HAVE_REMAP
  if(shared_memory)
    XShmPutImage(dpy,(back_flag)? back_buffer : window,
                        gc_clear,im,0,0,0,0,width,height,False);
  else
#endif
 {
  if(!flip)
  {
   if(im->data!=(char*)frames_buf[which].pixData()){
    if(!(width&1)){
     memcpy(im->data,frames_buf[which].pixData(),
                                        width*height*bitmap_pad/8);
    }else{
     from = frames_buf[which].pixData();
     for(i = 0, to = (T*)(im->data); i < frames_buf[which].Height();
        i++, to += im->bytes_per_line / sizeof(T),
        from += frames_buf[which].SizeX())
       memcpy(to,from,frames_buf[which].Width()*sizeof(T));
    }
   }
  }
  XPutImage(dpy,(back_flag)? back_buffer : window,
                   gc_clear,im,0,0,0,0,width,height);
 }
}


template <class T>
void XVWindowX<T>::get_colors(void)
{
  XColor xcolor,near_color;
  int i;

     // allocate draw colors
     for(i=0;i<DRAW_COLORS;i++)
     {
	XAllocNamedColor(dpy,DefaultColormap(dpy,DefaultScreen(dpy)),
			 string(XVDrawColor(color_names[i])).c_str(),&near_color,&xcolor);
	draw_color[i]=near_color.pixel;
     }
     draw_color[i++]=BlackPixel(dpy, DefaultScreen(dpy));
     draw_color[i++]=WhitePixel(dpy, DefaultScreen(dpy));
}

template <class T>
int XVWindowX<T>::get_font(char *name)
{
  if(num_fonts==MAX_FONT_NUM) return 0;
  if((font[num_fonts]=XLoadFont(dpy,name))==BadName) return 0;
  num_fonts++;
  return num_fonts-1;
}

template <class T>
int XVWindowX<T>::print_string(char *string,int x, int y,int col_index,
		  int font_index)
{
   if(font_index<0 || font_index>num_fonts) return 0;
   if (col_index<0 || col_index>DRAW_COLORS) return 0;
   XTextItem text={string,strlen(string),0,font[font_index]};
   XDrawText(dpy,(back_flag)? back_buffer : window,
                   gc_window[col_index],x,y,&text,1);
   return 1;
}

template <class T>
XVWindowX<T>::XVWindowX(const XVImageRGB<T> & image,int s_posx,int s_posy,
		      char * title, int in_event_mask,char *display,
		      int num_buffers, int double_buf):
             XVWindow<T>(image.Width(),image.Height())
{
  XSizeHints 		hints;
  Visual 		visual;
  XSetWindowAttributes 	xswa;
  XPixmapFormatValues   *format_ptr;
  int 			bg_pix,fg_pix;
  int			minor_version,major_version,n,i;

  // defaults
  exposeEvent = false;
  def_image=NULL;
  window=0;
  own_buffers=true;
  flip=0;
  shm_images=0 ;
  buffer_count=num_buffers;
  posx=s_posx; posy=s_posy;
  width=image.Width();
  height=image.Height();
  // open display
  if (!(dpy = XOpenDisplay(display)))
  {
     perror("Cannot open display\n");
     exit(1);
  }
  shared_memory=0;
#ifdef HAVE_REMAP
  if(!(shared_memory=(XDisplayName(display)[0]==':')))
     cerr << "shared memory display not available on remote" << endl;
  else
    shm_images=new XImage *[num_buffers];
#endif
  // screen depth
  format_ptr=XListPixmapFormats(dpy,&n);
  for(bitmap_pad=-1,i=0;i<n;i++)
  {
    if(format_ptr[i].depth==DefaultDepth(dpy,DefaultScreen(dpy)))
	       bitmap_pad= format_ptr[i].bits_per_pixel;
  }
  if(bitmap_pad<0)
  {
      cerr << "Could not match screen depth" << endl;
  }
  if((bitmap_pad+1)/8
            !=sizeof(T))
  {
    cerr << "screen depth "
         << bitmap_pad
	 << " does not match window depth " << sizeof(T)*8 << " ...exiting" << endl;
    exit(1);
  }
  if(format_ptr) XFree(format_ptr);

  fg_pix = BlackPixel(dpy, DefaultScreen(dpy));
  bg_pix = WhitePixel(dpy, DefaultScreen(dpy));
  xswa.backing_store = NotUseful;
  xswa.background_pixel = bg_pix;
  xswa.border_pixel = fg_pix;
  xswa.colormap = DefaultColormap(dpy,DefaultScreen(dpy));
  visual.visualid = CopyFromParent;
  window=XCreateWindow(dpy,
	RootWindow(dpy, DefaultScreen(dpy)),
	posx,posy,width,height,1,
	DefaultDepth(dpy,DefaultScreen(dpy)), InputOutput,&visual,
	CWBackingStore|CWBorderPixel|CWBackPixel|CWColormap,
	&xswa);
  hints.flags=USPosition|PMinSize|PMaxSize;
  hints.x=posx;
  hints.y=posy;
  hints.min_width=0;
  hints.min_height=0;
  hints.max_width=MAX_WIDTH;
  hints.max_height=MAX_HEIGHT;
  XSetNormalHints(dpy,window,&hints);
  if (title != NULL){
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)title,
		    strlen(title));
    windowTitle = title;
  }else{
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)"XVision(R) MVP(TUM)/CIRL- Window",
		    strlen("XVision(R) MVP(TUM)/CIRL- Window"));
    windowTitle = "XVision(R) MVP(TUM)/CIRL- Window";
  }
  event_mask = in_event_mask;
  XSelectInput(dpy,window,event_mask|ExposureMask);
  get_colors();
  xgcv.background = bg_pix;
  xgcv.function=GXcopy;
  xgcv.line_width=0;
  for(int i=0;i<DRAW_COLORS;i++)
  {
    xgcv.foreground = draw_color[i];
    gc_window[i]=XCreateGC(dpy,window,
    	GCFunction|GCForeground | GCBackground,&xgcv);

  }
  xgcv.foreground = bg_pix;
  gc_clear = XCreateGC(dpy,window,
		   GCFunction|GCForeground | GCBackground,&xgcv);
  XFlush(dpy);

  num_fonts=0;
  font=new Font[MAX_FONT_NUM];
  // double buffering available ??
  back_flag=0;
#ifdef HAVE_LIBXEXT
  if (double_buf &&
      XdbeQueryExtension (dpy, &major_version, &minor_version) )
  {
    Drawable                        root_win;
    XdbeScreenVisualInfo            *dbeVisInfo;
    int 			    i,num_screens;
    int                              maxPerf=-1,fastestVis,
	                            fastestVisDepth;


    num_screens=1;
    root_win=RootWindow(dpy, DefaultScreen(dpy));
    if((dbeVisInfo =XdbeGetVisualInfo(dpy,&root_win,&num_screens)))
    {
      for (i=0; i<dbeVisInfo->count; i++)
      {
        if (dbeVisInfo->visinfo[i].perflevel > maxPerf)
	{
	  maxPerf = dbeVisInfo->visinfo[i].perflevel;
	  fastestVis = dbeVisInfo->visinfo[i].visual;
	  fastestVisDepth=dbeVisInfo->visinfo[i].depth;
	}
      }
      if(maxPerf>-1)
      {
        back_buffer=
		XdbeAllocateBackBufferName(dpy,window,XdbeBackground);
	swap_info.swap_window=window;
	swap_info.swap_action=XdbeUndefined;
	back_flag=1;
	cerr << "got double buffer" << endl;
      }
    }
  }
#endif
}

template <class T>
XVWindowX<T>::XVWindowX(int w, int h,int s_posx,int s_posy,
		      char * title, int in_event_mask,char *display,
		      int num_buffers, int double_buf):
             XVWindow<T>(w, h)
{
  XSizeHints 		hints;
  Visual 		visual;
  XSetWindowAttributes 	xswa;
  XPixmapFormatValues   *format_ptr;
  int 			bg_pix,fg_pix;
  int			minor_version,major_version,n,i;

  // defaults
  exposeEvent = false;
  def_image=NULL;
  window=0;
  own_buffers=true;
  shm_images=0 ;
  flip=0;
  buffer_count=num_buffers;
  posx=s_posx; posy=s_posy;
  width=w;
  height=h;
  // open display
  if (!(dpy = XOpenDisplay(display)))
  {
     perror("Cannot open display\n");
     exit(1);
  }
  shared_memory=0;
#ifdef HAVE_LIBXEXT
  if(!(shared_memory=(XDisplayName(display)[0]==':')))
     cerr << "shared memory display not available on remote" << endl;
  else
    shm_images=new XImage *[num_buffers];
#endif
  // screen depth
  format_ptr=XListPixmapFormats(dpy,&n);
  for(bitmap_pad=-1,i=0;i<n;i++)
  {
    if(format_ptr[i].depth==DefaultDepth(dpy,DefaultScreen(dpy)))
	       bitmap_pad= format_ptr[i].bits_per_pixel;
  }
  if(bitmap_pad<0)
  {
      cerr << "Could not match screen depth" << endl;
  }
  if((bitmap_pad+1)/8
            !=sizeof(T))
  {
    cerr << "screen depth "
         << bitmap_pad
	 << " does not match window depth " << sizeof(T)*8 << " ...exiting" << endl;
    exit(1);
  }
  if(format_ptr) XFree(format_ptr);

  fg_pix = BlackPixel(dpy, DefaultScreen(dpy));
  bg_pix = WhitePixel(dpy, DefaultScreen(dpy));
  xswa.backing_store = NotUseful;
  xswa.background_pixel = bg_pix;
  xswa.border_pixel = fg_pix;
  xswa.colormap = DefaultColormap(dpy,DefaultScreen(dpy));
  visual.visualid = CopyFromParent;
  window=XCreateWindow(dpy,
	RootWindow(dpy, DefaultScreen(dpy)),
	posx,posy,width,height,1,
	DefaultDepth(dpy,DefaultScreen(dpy)), InputOutput,&visual,
	CWBackingStore|CWBorderPixel|CWBackPixel|CWColormap,
	&xswa);
  hints.flags=USPosition|PMinSize|PMaxSize;
  hints.x=posx;
  hints.y=posy;
  hints.min_width=0;
  hints.min_height=0;
  hints.max_width=MAX_WIDTH;
  hints.max_height=MAX_HEIGHT;
  XSetNormalHints(dpy,window,&hints);
  if (title != NULL){
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)title,
		    strlen(title));
    windowTitle = title;
  }else{
    XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		    PropModeReplace, (unsigned char *)"XVision(R) MVP(TUM)/CIRL- Window",
		    strlen("XVision(R) MVP(TUM)/CIRL- Window"));
    windowTitle = "XVision(R) MVP(TUM)/CIRL- Window";
  }
  event_mask = in_event_mask;
  XSelectInput(dpy,window,event_mask|ExposureMask);
  get_colors();
  xgcv.background = bg_pix;
  xgcv.function=GXcopy;
  xgcv.line_width=0;
  for(int i=0;i<DRAW_COLORS;i++)
  {
    xgcv.foreground = draw_color[i];
    gc_window[i]=XCreateGC(dpy,window,
    	GCFunction|GCForeground | GCBackground,&xgcv);

  }
  xgcv.foreground = bg_pix;
  gc_clear = XCreateGC(dpy,window,
		   GCFunction|GCForeground | GCBackground,&xgcv);
  XFlush(dpy);

  num_fonts=0;
  font=new Font[MAX_FONT_NUM];
  // double buffering available ??
  back_flag=0;
#ifdef HAVE_LIBXEXT
  if (double_buf &&
      XdbeQueryExtension (dpy, &major_version, &minor_version) )
  {
    Drawable                        root_win;
    XdbeScreenVisualInfo            *dbeVisInfo;
    int 			    i,num_screens;
    int                              maxPerf=-1,fastestVis,
	                            fastestVisDepth;


    num_screens=1;
    root_win=RootWindow(dpy, DefaultScreen(dpy));
    if((dbeVisInfo =XdbeGetVisualInfo(dpy,&root_win,&num_screens)))
    {
      for (i=0; i<dbeVisInfo->count; i++)
      {
        if (dbeVisInfo->visinfo[i].perflevel > maxPerf)
	{
	  maxPerf = dbeVisInfo->visinfo[i].perflevel;
	  fastestVis = dbeVisInfo->visinfo[i].visual;
	  fastestVisDepth=dbeVisInfo->visinfo[i].depth;
	}
      }
      if(maxPerf>-1)
      {
        back_buffer=
		XdbeAllocateBackBufferName(dpy,window,XdbeBackground);
	swap_info.swap_window=window;
	swap_info.swap_action=XdbeUndefined;
	back_flag=1;
	cerr << "got double buffer" << endl;
      }
    }
    XdbeFreeVisualInfo(dbeVisInfo);
  }
#endif
}

template <class T>
XVWindowX<T>::~XVWindowX(void)
{
  if(shm_images && own_buffers) delete[] shm_images;
  if(window)
  {
    XDestroyWindow(dpy,window);
    XFlush(dpy);
  }

  for(int i = 0; i < DRAW_COLORS; ++i) {
    if(gc_window[i]) XFreeGC(dpy, gc_window[i]);
  }

  if(gc_clear) XFreeGC(dpy, gc_clear);
  if(def_image) XDestroyImage(def_image);
  delete[] font ;
}

template <class T>
void XVWindowX<T>::resize( const XVSize& size )
{
  if(window)
  {
    //{
    //  XEvent xev;
     // while(XCheckMaskEvent(dpy,ExposureMask,&xev));
    //}
    XResizeWindow(dpy,window,width=size.Width(),
    			     height=size.Height());
    if (windowTitle != NULL)
      XChangeProperty(dpy,window,XA_WM_NAME, XA_STRING,8,
		      PropModeReplace, (unsigned char *)windowTitle,
		      strlen(windowTitle));
    XFlush(dpy);
    exposeEvent=false;
  }
}

template <class T>
int XVWindowX<T>::check_events(int *ret_field)
{
  XEvent xev;

  if(XCheckWindowEvent(dpy,window,event_mask,
  	&xev))
  {
    switch(xev.type)
      {
      case Expose:
	ret_field[0]=ret_field[1]=EXPOSE;
	return EXPOSE;
      case ButtonPress:
	ret_field[0] = xev.xbutton.x;
	ret_field[1] = xev.xbutton.y;
	switch(xev.xbutton.button)
	  {
	  case Button1:
	    return BUTTON_PRESSED|LEFT_BUTTON;
	  case Button2:
	    return BUTTON_PRESSED|MIDDLE_BUTTON;
	  case Button3:
	    return BUTTON_PRESSED|RIGHT_BUTTON;
	  default:
	    return BUTTON_PRESSED;
	}
      case ButtonRelease:
	ret_field[0] = xev.xbutton.x;
	ret_field[1] = xev.xbutton.y;
	switch(xev.xbutton.button)
	  {
	  case Button1:
	    return BUTTON_RELEASED|LEFT_BUTTON;
	  case Button2:
	    return BUTTON_RELEASED|MIDDLE_BUTTON;
	  case Button3:
	    return BUTTON_RELEASED|RIGHT_BUTTON;
	  default:
	    return BUTTON_RELEASED;
	}
      case MotionNotify:
	ret_field[0] = xev.xmotion.x;
	ret_field[1] = xev.xmotion.y;
	return POINTER_MOVED |
	  ( xev.xmotion.state&Button1Mask ? LEFT_BUTTON : 0 ) |
	  ( xev.xmotion.state&Button2Mask ? MIDDLE_BUTTON : 0 ) |
	  ( xev.xmotion.state&Button3Mask ? RIGHT_BUTTON : 0 ) ;
      case KeyPress:
	ret_field[0] = XLookupKeysym( &xev.xkey, xev.xkey.state&ShiftMask?1:0);
	ret_field[1] = xev.xkey.keycode ;
	return KEY_PRESSED ;
      case KeyRelease:
	ret_field[0] = XLookupKeysym( &xev.xkey, xev.xkey.state&ShiftMask?1:0);
	ret_field[1] = xev.xkey.keycode ;
	return KEY_RELEASED ;
      case ResizeRequest:
        if(xev.xresizerequest.width!=width ||
	   xev.xresizerequest.height!=height)
	  {
	    ret_field[0]=xev.xresizerequest.width;
	    ret_field[1]=xev.xresizerequest.height;
	    return RESIZED;
	  }
      default:
	cerr << "event "<< xev.type << " discarded" << endl;
       return 0 ;
    }
  }
  return 0 ;
}

template <class T>
XVImageRGB<T> XVWindowX<T>::getDisplayedImage(int px, int py, int w, int h) {

  if(w == -1) w = this->width;
  if(h == -1) h = this->height;

  u_int allplanes = 0;
  allplanes = ~allplanes;
  XImage * tmpXIM = XGetImage(dpy, window, px, py, w, h, allplanes, ZPixmap);
  XVImageRGB<T> dispIM(w, h);
  memcpy(dispIM.lock(), tmpXIM->data, sizeof(T)*w*h);
  dispIM.unlock();
  XDestroyImage( tmpXIM );
  return dispIM;
};

template <class T>
void XVWindowX<T>::CopySubImage(const XVImageRGB<T> & image,bool s_flip) {
  int i,j;
  const T *from=image.data();
  static const T* old_from=NULL;
  T *to;


#ifdef HAVE_REMAP
  if( ((height!=image.Height()) ||(width!=image.Width())) && def_image ) {
    if( shared_memory ) {
      XShmDetach(dpy,def_shminfo);
      shmdt(def_shminfo->shmaddr);
      shmctl(def_shminfo->shmid, IPC_RMID, 0);
    }
    XDestroyImage(def_image);
    def_image=NULL;
  }

#endif
  if( !def_image ) {
    if(back_flag) XdbeDeallocateBackBufferName(dpy,back_buffer);
    if( (height!=image.Height()) ||(width!=image.Width()) ) {
      height=image.Height();
      width=image.Width();
      XResizeWindow(dpy,window,width,height);
      exposeEvent=false;
    }
    old_from=from;
#ifdef HAVE_REMAP
      if(back_flag) back_buffer=
                XdbeAllocateBackBufferName(dpy,window,XdbeBackground);
#endif
      if(shared_memory)
      {
#ifdef HAVE_REMAP
    	XShmSegmentInfo      *shminfo;
        shminfo = new XShmSegmentInfo;
        def_image= XShmCreateImage(dpy, DefaultVisual(dpy,DefaultScreen(dpy)),
                                 DefaultDepth(dpy,DefaultScreen(dpy)),
                                 ZPixmap,NULL,shminfo,image.Width(),
				 image.Height());
        shminfo->shmid = shmget(IPC_PRIVATE,
                           def_image->bytes_per_line*def_image->height,
		           IPC_CREAT | 0777);
        shminfo->shmaddr = (char *) shmat(shminfo->shmid,0,0);
        def_image->data = shminfo->shmaddr;
        XShmAttach(dpy, shminfo);
	def_shminfo = shminfo;
        shmctl(shminfo->shmid,IPC_RMID,NULL);
        //delete shminfo; //don't delete it - shared memory won't work otherwise
#endif
      }
      else
      {
	// Donald+Darius: stupid Xserver allocates word aligned buffers
	//              : this is our answer :-)
	char* memory = (char *)
	  malloc((width*sizeof(T)/4+((width*sizeof(T)%4)!=0))*4*height);
        def_image=XCreateImage(dpy,
			       DefaultVisual(dpy,DefaultScreen(dpy)),
			       DefaultDepth(dpy,DefaultScreen(dpy)),
			       ZPixmap, 0, memory,
			       width,height,BitmapPad(dpy),0);
      }
  }
  if(!(flip=s_flip))
  {
    if(!(image.Skip()) && !(width*sizeof(T)%4))
    {
       memcpy(def_image->data,from,
	   	image.Height()*image.Width()*sizeof(T));
    }
    else
    {
       for(i=0,to=(T*)(def_image->data);i<image.Height();
         i++,to=(T*)((char*)to+def_image->bytes_per_line),from+=image.SizeX())
	     memcpy(to,from,image.Width()*sizeof(T));
    }
  }
  else
  {
        for(j=0,to=(T *)(def_image->data)+
	       def_image->bytes_per_line/sizeof(T);
	     j<height;j++, to=(T*)((char*)to+
	        def_image->bytes_per_line)+
	        def_image->bytes_per_line/sizeof(T))
	        for(i=0;i<image.Width();i++) *(--to)=*from++;
  }

  if(!exposeEvent){
    XEvent xev;
    XMaskEvent(dpy,ExposureMask, & xev);
    expose();
  }

  if(shared_memory)
  {
#ifdef HAVE_REMAP
     XShmPutImage(dpy,(back_flag)? back_buffer : window,
		  gc_clear,def_image,0,0,0,0,width,height,False);
#else
     XPutImage(dpy,(back_flag)? back_buffer : window,
		      	gc_clear,def_image,0,0,0,0,width,height);
#endif
  }
  else
  {
    XPutImage(dpy,(back_flag)? back_buffer : window,
                   gc_clear,def_image,0,0,0,0,width,height);
  }
}

#ifdef NEVER
template class XVWindowX<u_char>;
template class XVWindowX<u_short>;
template class XVWindowX<u_long>;
#endif
template class XVWindowX<XV_RGBA32>;
template class XVWindowX<XV_RGB24>;
template class XVWindowX<XV_RGB16>;
template class XVWindowX<XV_RGB15>;
#endif

#include <X11/cursorfont.h>

template <class PIX>
XVDrawWindowX<PIX>::XVDrawWindowX(const XVImageRGB<PIX > & im, int px, int py ,
				  char * title, int event_mask, char * disp,
				int num_buf, int double_buf, int f) :
  XVWindowX<PIX>(im, px, py, title, event_mask, disp, num_buf, double_buf),
  XVWindow<PIX>(im.Width(),im.Height()) {

  function = f;

  for(int i = 0; i < DRAW_COLORS; ++i){

    XGCValues vals;
    XGetGCValues(dpy, gc_window[i], GCForeground | GCBackground, & vals);
    vals.function = function;
    vals.line_width=2;
    GCmap[color_names[i]] = XCreateGC(dpy, back_flag ? back_buffer : window,
				      GCFunction | GCForeground | GCBackground|GCLineWidth, & vals);
  }
};

template <class PIX>
XVDrawWindowX<PIX>::XVDrawWindowX(int w, int h, int px, int py,
				  char * title, int event_mask, char * disp,
				int num_buf, int double_buf, int f) :
  XVWindowX<PIX>(w, h, px, py, title, event_mask, disp, num_buf, double_buf),
  XVWindow<PIX>(w,h) {

  function = f;
  for(int i = 0; i < DRAW_COLORS; ++i){

    XGCValues vals;
    XGetGCValues(dpy, gc_window[i], GCForeground | GCBackground, & vals);
    vals.function = function;
    vals.line_width=2;
    GCmap[color_names[i]] = XCreateGC(dpy, back_flag ? back_buffer : window,
				      GCLineWidth|GCFunction | GCForeground | GCBackground, & vals);
  }
};

template <class PIX>
XVDrawWindowX<PIX>::~XVDrawWindowX() {

  MI GCmapIter = GCmap.begin();
  for(; GCmapIter != GCmap.end(); ++GCmapIter) {
    if(GCmapIter->second) XFreeGC(dpy, GCmapIter->second);
  }
  GCmap.clear();
};

template <class PIX>
inline void XVDrawWindowX<PIX>::setFunction(int f){

  for(MI iter = GCmap.begin(); iter != GCmap.end(); ++iter){
    XGCValues vals;
    XGetGCValues(dpy, iter->second, GCForeground | GCBackground, & vals);
    vals.function = f;
    if(GCmap[iter->first]) { XFreeGC(dpy, GCmap[iter->first]); }
    GCmap[iter->first] = XCreateGC(dpy, back_flag ? back_buffer : window,
				   GCFunction | GCForeground | GCBackground, & vals);
  }
};

template <class PIX>
inline void XVDrawWindowX<PIX>::addColor(XVDrawColor colorName){

  if(GCmap.find(colorName) != GCmap.end()) return;

  XColor xcolor, nearcolor;

  Status ret;

  ret = XAllocNamedColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
			 string(colorName).c_str(), & nearcolor, & xcolor);

  if(!ret) throw CannotAllocColorException();

  XGCValues vals;

  vals.background = WhitePixel(dpy, DefaultScreen(dpy));
  vals.function   = function;
  vals.foreground = nearcolor.pixel;

  GCmap[colorName] = XCreateGC(dpy, back_flag ? back_buffer : window,
			       GCFunction | GCForeground | GCBackground, & vals);
};

template <class PIX>
inline int XVDrawWindowX<PIX>::drawPoint(int x, int y,
					 XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);

  XVDrawPointX p(dpy, back_flag ? back_buffer : window, GCmap[c],
		 XVPosition(x, y), c);
  p.draw();
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::drawLine(int x1, int y1, int x2, int y2,
					XVDrawColor c, int line_width){

  XGCValues  temp_gc;
  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XGetGCValues(dpy,GCmap[c],GCLineWidth,&temp_gc);
  if(line_width)
  {
    temp_gc.line_width=line_width;
    XChangeGC(dpy,GCmap[c],GCLineWidth,&temp_gc);
  }

  XVDrawLineX l(dpy, back_flag ? back_buffer : window, GCmap[c],
		XVPosition(x1, y1), XVPosition(x2, y2), c);

  l.draw();
  if(line_width)
  {
    temp_gc.line_width=0;
    XChangeGC(dpy,GCmap[c],GCLineWidth,&temp_gc);
  }
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::drawRectangle(int x, int y, int w, int h,
					     XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawRectX r(dpy, back_flag ? back_buffer : window, GCmap[c],
		     XVImageGeneric(w, h, x, y), false, c);
  r.draw();
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::fillRectangle(int x, int y, int w, int h,
					     XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawRectX r(dpy, back_flag ? back_buffer : window, GCmap[c],
		     XVImageGeneric(w, h, x, y), true, c);
  r.draw();
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::drawEllipse(int x, int y, int w, int h,
					   XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);

  XVDrawEllipseX e(dpy, back_flag ? back_buffer : window, GCmap[c],
		   XVImageGeneric(w, h, x, y), false, c);
  e.draw();
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::fillEllipse(int x, int y, int w, int h,
					   XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);

  XVDrawEllipseX e(dpy, back_flag ? back_buffer : window, GCmap[c],
		   XVImageGeneric(w, h, x, y), true, c);
  e.draw();
  return true;
};

template <class PIX>
inline int XVDrawWindowX<PIX>::drawString(int x, int y,
					  char * str, int length,
					  XVDrawColor c){

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);

  XVDrawStringX s(dpy, back_flag ? back_buffer : window, GCmap[c],
		  XVPosition(x, y), str, length, c);
  s.draw();
  return true;
};

template <class PIX>
inline void XVStateWindowX<PIX>::CopySubImage(const XVImageRGB<PIX > & image){

  this->XVDrawWindowX<PIX >::CopySubImage(image);
  for(OBJ_ITER oiter = drawObjects.begin();
      oiter != drawObjects.end(); ++oiter){
    (dynamic_cast<XVDrawObjectX *>(*oiter))->reset(dpy,
     back_flag ? back_buffer : window, GCmap[(*oiter)->color]);
    (*oiter)->draw();
  }
};

template <class PIX>
inline void XVStateWindowX<PIX>::CopyImage(int which, u_short s_flip) {

  this->XVDrawWindowX<PIX >::CopyImage(which, s_flip);
  for(OBJ_ITER oiter = drawObjects.begin();
      oiter != drawObjects.end(); ++oiter){
    (*oiter)->draw();
    (dynamic_cast<XVDrawObjectX *>(*oiter))->reset(dpy,
     back_flag ? back_buffer : window, GCmap[(*oiter)->color]);
  }
};

template <class PIX>
inline void XVStateWindowX<PIX>::erase(int i) {
  OBJ_ITER iter = drawObjects.begin();
  for(int j = 0; j < i && iter != drawObjects.end(); ++j, ++iter){}
  if(*iter) delete *iter;
  drawObjects.erase(iter);
}

template <class PIX>
inline void XVStateWindowX<PIX>::clear() {
  OBJ_ITER iter = drawObjects.begin();
  for(; iter != drawObjects.end(); ++iter){
    if(*iter) delete *iter;
  }
  drawObjects.clear();
}

template <class PIX>
int
XVStateWindowX<PIX>::drawPoint(int x, int y, XVDrawColor c) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawPointX * p
    = new XVDrawPointX(this->dpy,
		       this->back_flag ? this->back_buffer : this->window,
		       this->GCmap[c],
		       XVPosition(x, y), c);
  drawObjects.push_back(p);
  p->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::drawLine(int x1, int y1, int x2, int y2,
			      XVDrawColor c,int line_width ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);

  XGCValues temp_gc;
  XGetGCValues(dpy,GCmap[c],GCLineWidth,&temp_gc);
  if(line_width!=temp_gc.line_width)
  {
    temp_gc.line_width=line_width;
    XChangeGC(dpy,GCmap[c],GCLineWidth,&temp_gc);
  }

  XVDrawLineX * l
    = new XVDrawLineX(this->dpy,
		      this->back_flag ? this->back_buffer : this->window,
		      this->GCmap[c],
		      XVPosition(x1, y1),
		      XVPosition(x2, y2), c);
  drawObjects.push_back(l);
  l->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::drawRectangle(int x, int y, int w, int h,
				   XVDrawColor c ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawRectX * r
    = new XVDrawRectX(this->dpy,
		      this->back_flag ? this->back_buffer : this->window,
		      this->GCmap[c], XVImageGeneric(w, h, x, y), false, c);
  drawObjects.push_back(r);
  r->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::fillRectangle(int x, int y, int w, int h,
				   XVDrawColor c ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawRectX * r
    = new XVDrawRectX(this->dpy,
		      this->back_flag ? this->back_buffer : this->window,
		      this->GCmap[c], XVImageGeneric(w, h, x, y), true, c);
  drawObjects.push_back(r);
  r->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::drawEllipse(int x, int y, int w, int h,
				 XVDrawColor c ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawEllipseX * e
    = new XVDrawEllipseX(this->dpy,
			 this->back_flag ? this->back_buffer : this->window,
			 this->GCmap[c],
			 XVImageGeneric(w, h, x, y), false, c);
  drawObjects.push_back(e);
  e->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::fillEllipse(int x, int y, int w, int h,
				 XVDrawColor c ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawEllipseX * e
    = new XVDrawEllipseX(this->dpy,
			 this->back_flag ? this->back_buffer : this->window,
			 this->GCmap[c], XVImageGeneric(w, h, x, y), true, c);
  drawObjects.push_back(e);
  e->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
int
XVStateWindowX<PIX>::drawString(int x, int y, char * str, int length,
				XVDrawColor c ) {

  if(GCmap.find(c) == GCmap.end()) this->addColor(c);
  XVDrawStringX * s
    = new XVDrawStringX(this->dpy,
			this->back_flag ? this->back_buffer : this->window,
			this->GCmap[c], XVPosition(x, y), str, length, c);
  drawObjects.push_back(s);
  s->draw();
  return drawObjects.size() - 1;
};

template <class PIX>
XVInteractWindowX<PIX>::XVInteractWindowX(const XVImageRGB<PIX > & im,
					  int px , int py ,
					  char * title ,
					  int event_mask ,
					  char * disp ,
					  int num_buf ,
					  int double_buf,
					  int function ) :
  XVDrawWindowX<PIX>(im, px, py, title, event_mask,
		     disp, num_buf, double_buf, function),
  XVWindow<PIX>(im.Width(),im.Height()) {

  crossHair = XCreateFontCursor(dpy, XC_crosshair);
  arrow     = XCreateFontCursor(dpy, XC_arrow);
}

template <class PIX>
XVInteractWindowX<PIX>::XVInteractWindowX(int w, int h,
					  int px, int py,
					  char * title,
					  int event_mask,
					  char * disp,
					  int num_buf,
					  int double_buf,
					  int function ) :
  XVDrawWindowX<PIX>(w, h, px, py, title, event_mask,
		     disp, num_buf, double_buf, function),
  XVWindow<PIX>(w,h) {

  crossHair = XCreateFontCursor(dpy, XC_crosshair);
  arrow     = XCreateFontCursor(dpy, XC_arrow);
}

template <class PIX>
XVInteractWindowX<PIX>::~XVInteractWindowX() {

  if(crossHair) { XFreeCursor(dpy, crossHair); }
  if(arrow)     { XFreeCursor(dpy, arrow);     }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectPoint(XVPosition & point,
				    XVDrawColor c,bool draw_flag ){

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  point.reposition(-1, -1);

  if(draw_flag) XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, this->window, ButtonPressMask   |
	                                ButtonReleaseMask |
                                        ButtonMotionMask);

   do{
    if(XCheckMaskEvent(this->dpy, ButtonPressMask, & xev)){

      point.reposition(xev.xbutton.x, xev.xbutton.y);
      if(draw_flag)
      {
        this->XVDrawWindowX<PIX >::drawPoint(point.PosX(), point.PosY(), c);
        this->XVWindowX<PIX>::swap_buffers();
        this->XVWindowX<PIX>::flush();
        XUndefineCursor(this->dpy, this->window);
      }
      /** reset the input mask for this window **/
      XSelectInput(this->dpy,this->window,attr.all_event_masks);
      return;
    }
  }while(draw_flag);
};


template <class PIX>
inline void
XVInteractWindowX<PIX>::selectLine(XVPosition & p1, XVPosition & p2, XVDrawColor c ){

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  ButtonMotionMask);

  p1.reposition(-1, -1);
  p2.reposition(-1, -1);
  bool mouseDown = false;

  while(1){

    if(XCheckMaskEvent(this->dpy, ButtonPressMask, &xev) && !mouseDown){

      p1.reposition(xev.xbutton.x, xev.xbutton.y);
      p2.reposition(xev.xbutton.x, xev.xbutton.y);
      mouseDown = true;
    }else if(XCheckMaskEvent(this->dpy, ButtonReleaseMask, &xev) && mouseDown){

      p2.reposition(xev.xbutton.x, xev.xbutton.y);
      XUndefineCursor(this->dpy, this->window);
      /** reset the input mask for this window **/
      XSelectInput(this->dpy,this->window,attr.all_event_masks);
      return;
    }else if(XCheckMaskEvent(this->dpy, ButtonMotionMask, &xev) && mouseDown){

      this->XVDrawWindowX<PIX >::drawLine(p1.PosX(), p1.PosY(),
					  p2.PosX(), p2.PosY(), c);
      p2.reposition(xev.xbutton.x, xev.xbutton.y);
      this->XVDrawWindowX<PIX>::drawLine(p1.PosX(), p1.PosY(), xev.xbutton.x, xev.xbutton.y, c);
      this->XVWindowX<PIX>::swap_buffers();
      this->XVWindowX<PIX>::flush();
    }
  }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectRectangle(XVImageGeneric & region,
					XVDrawColor c ){

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);
  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  ButtonMotionMask);

  region.reposition(-1, -1); region.resize(-1, -1);
  bool mouseDown = false;
  int tmpWidth, tmpHeight;

  while(1){

    if(XCheckMaskEvent(dpy, ButtonPressMask, &xev) && !mouseDown){

      region = XVImageGeneric(0, 0, xev.xbutton.x, xev.xbutton.y);
      this->XVDrawWindowX<PIX >::drawRectangle(region.PosX(), region.PosY(),
					       region.Width(), region.Height(), c);
      mouseDown = true;

    }else if(XCheckMaskEvent(dpy, ButtonReleaseMask, &xev) && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region.PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region.PosY()) > 0 &&
	 region.PosX() != -1 && region.PosY() != -1){

	region.resize(tmpWidth, tmpHeight);
	XUndefineCursor(this->dpy, this->window);
	/** reset the input mask for this window **/
        XSelectInput(this->dpy,this->window,attr.all_event_masks);
	return;
      }
      region = XVImageGeneric(-1, -1, -1, -1);
      mouseDown = false;
    }else if(XCheckMaskEvent(dpy, PointerMotionMask, &xev) && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region.PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region.PosY()) > 0 &&
	 region.PosX() != -1 && region.PosY() != -1){

	this->XVDrawWindowX<PIX >::drawRectangle(region.PosX(), region.PosY(),
						 region.Width(), region.Height(), c);
	region.resize(tmpWidth, tmpHeight);
	this->XVDrawWindowX<PIX >::drawRectangle(region.PosX(), region.PosY(),
						 region.Width(), region.Height(),
						 c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    }
  }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectEllipse(XVImageGeneric & region,
				      XVDrawColor c ){

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  ButtonMotionMask);

  region.reposition(-1, -1);
  region.resize(-1, -1);

  bool mouseDown = false;
  int tmpWidth, tmpHeight;

  while(1){

    if(XCheckMaskEvent(this->dpy, ButtonPressMask, &xev) && !mouseDown){

      region.reposition(xev.xbutton.x, xev.xbutton.y);
      mouseDown = true;
    }else if(XCheckMaskEvent(this->dpy, ButtonReleaseMask, &xev) && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region.PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region.PosY()) > 0 &&
	 region.PosX() != -1 && region.PosY() != -1){

	region.resize(tmpWidth, tmpHeight);
	XUndefineCursor(this->dpy, this->window);
	/** reset the input mask for this window **/
        XSelectInput(this->dpy,this->window,attr.all_event_masks);
	return;
      }
      region = XVImageGeneric(-1, -1, -1, -1);
      mouseDown = false;
    }else if(XCheckMaskEvent(this->dpy, PointerMotionMask, &xev) && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region.PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region.PosY()) > 0 &&
	 region.PosX() != -1 && region.PosY() != -1){

	this->XVDrawWindowX<PIX >::drawEllipse(region.PosX(), region.PosY(),
					       region.Width(), region.Height(), c);
	region.resize(tmpWidth, tmpHeight);
	this->XVDrawWindowX<PIX >::drawEllipse(region.PosX(), region.PosY(),
					       tmpWidth, tmpHeight, c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    }
  }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectSizedRect(XVImageGeneric & box,
					const XVSize & size,
					XVDrawColor c ){

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  PointerMotionMask);

  int first = true;
  XVPosition half(size.Width() / 2, size.Height() / 2);
  XVPosition currPos(-1, -1);
  box.resize(size); box.reposition(currPos);

  while(1){

    if(XCheckMaskEvent(dpy, PointerMotionMask, &xev)){

      if(!first){
	this->XVDrawWindowX<PIX >::drawRectangle(box.PosX(), box.PosY(),
						 box.Width(), box.Height(), c);
      }else{ first = false; }
      box.reposition(XVPosition(xev.xmotion.x, xev.xmotion.y) - half);
      this->XVDrawWindowX<PIX >::drawRectangle(box.PosX(), box.PosY(),
					       box.Width(), box.Height(), c);
      this->XVWindowX<PIX>::swap_buffers();
      this->XVWindowX<PIX>::flush();

    }else if(XCheckMaskEvent(dpy, ButtonPressMask, &xev)){

      box.reposition(XVPosition(xev.xbutton.x, xev.xbutton.y) - half);
      XUndefineCursor(this->dpy, this->window);
      /** reset the input mask for this window **/
      XSelectInput(this->dpy,this->window,attr.all_event_masks);
      return;
    }
  }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectAngledRect(XVPosition & ulcPos, XVSize & size,
					 double & angle,
					 XVDrawColor c ) {

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  PointerMotionMask);

  XV2Vec<double> ulc, urc, lrc, llc;

  bool topLineSelected = false;
  bool mouseDown = false;

  while(1){

    if(XCheckMaskEvent(dpy, PointerMotionMask, &xev)){
      if(mouseDown && !topLineSelected) {
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      } else if(topLineSelected) {
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(llc.PosX()),
					   (int)rint(llc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(urc.PosX()),
					   (int)rint(urc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(llc.PosX()),
					   (int)rint(llc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	XV2Vec<double> currentPos(xev.xbutton.x, xev.xbutton.y);
	XV2Vec<double> topVec = urc - ulc;
	XV2Vec<double> tmpVec = currentPos - ulc;
	XV2Vec<double> projVec = ((topVec * tmpVec) * topVec)
	  / (topVec.length() * topVec.length());
	XV2Vec<double> heightVec = currentPos - (projVec + ulc);
	llc = ulc + heightVec;
	lrc = llc + topVec;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(llc.PosX()),
					   (int)rint(llc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(urc.PosX()),
					   (int)rint(urc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(llc.PosX()),
					   (int)rint(llc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    } else if(XCheckMaskEvent(dpy, ButtonPressMask, &xev)) {
      mouseDown = true;
      if(!topLineSelected) {
	ulc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	urc = ulc;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    } else if(XCheckMaskEvent(dpy, ButtonReleaseMask, &xev)) {
      if(mouseDown && !topLineSelected) {
	mouseDown = false;
	topLineSelected = true;
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	lrc = urc;
	llc = ulc;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()));
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }else if(topLineSelected) {
	XV2Vec<double> diffVec = ulc - llc;
	if(diffVec.x() < 0){
	  angle = -acos((XV2Vec<double>(0, -1) * diffVec)
					   / diffVec.length());
	}else{
	  angle = acos((XV2Vec<double>(0, -1) * diffVec)
		       / diffVec.length());
	}
	size = XVSize((int)rint((urc - ulc).length()), (int)rint(diffVec.length()));
	ulcPos = (XVPosition)ulc;
	XUndefineCursor(this->dpy, this->window);
        /** reset the input mask for this window **/
        XSelectInput(this->dpy,this->window,attr.all_event_masks);
	return;
      }
    }
  }
};

template <class PIX>
inline void
XVInteractWindowX<PIX>::selectAngledRect(XVPosition & ulcPos, XVPosition & urcPos, XVSize & size,
					 double & angle,
					 XVDrawColor c ) {

  XEvent xev;
  XWindowAttributes attr;

  XGetWindowAttributes(this->dpy,this->window,&attr);

  XDefineCursor(this->dpy, this->window, crossHair);
  XSelectInput(this->dpy, window, ButtonPressMask |
	                          ButtonReleaseMask |
                                  PointerMotionMask);

  XV2Vec<double> ulc, urc, lrc, llc;

  bool topLineSelected = false;
  bool mouseDown = false;

  while(1){

    if(XCheckMaskEvent(dpy, PointerMotionMask, &xev)){
      if(mouseDown && !topLineSelected) {
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      } else if(topLineSelected) {
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(llc.PosX()),
					   (int)rint(llc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(urc.PosX()),
					   (int)rint(urc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(llc.PosX()),
					   (int)rint(llc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	XV2Vec<double> currentPos(xev.xbutton.x, xev.xbutton.y);
	XV2Vec<double> topVec = urc - ulc;
	XV2Vec<double> tmpVec = currentPos - ulc;
	XV2Vec<double> projVec = ((topVec * tmpVec) * topVec)
	  / (topVec.length() * topVec.length());
	XV2Vec<double> heightVec = currentPos - (projVec + ulc);
	llc = ulc + heightVec;
	lrc = llc + topVec;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(llc.PosX()),
					   (int)rint(llc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(urc.PosX()),
					   (int)rint(urc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVDrawWindowX<PIX>::drawLine((int)rint(llc.PosX()),
					   (int)rint(llc.PosY()),
					   (int)rint(lrc.PosX()),
					   (int)rint(lrc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    } else if(XCheckMaskEvent(dpy, ButtonPressMask, &xev)) {
      mouseDown = true;
      if(!topLineSelected) {
	ulc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	urc = ulc;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()), c);
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }
    } else if(XCheckMaskEvent(dpy, ButtonReleaseMask, &xev)) {
      if(mouseDown && !topLineSelected) {
	mouseDown = false;
	topLineSelected = true;
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	lrc = urc;
	llc = ulc;
	this->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					   (int)rint(ulc.PosY()),
					   (int)rint(urc.PosX()),
					   (int)rint(urc.PosY()));
	this->XVWindowX<PIX>::swap_buffers();
	this->XVWindowX<PIX>::flush();
      }else if(topLineSelected) {
	XV2Vec<double> diffVec = ulc - llc;
	if(diffVec.x() < 0){
	  angle = -acos((XV2Vec<double>(0, -1) * diffVec)
					   / diffVec.length());
	}else{
	  angle = acos((XV2Vec<double>(0, -1) * diffVec)
		       / diffVec.length());
	}
	size = XVSize((int)rint((urc - ulc).length()), (int)rint(diffVec.length()));
	ulcPos = (XVPosition)ulc;
	urcPos = (XVPosition)urc;
	//	cout<<"selected rectangle: "<<ulc<<","<<llc<<","<<urc<<","<<lrc<<endl;
	XUndefineCursor(this->dpy, this->window);
        /** reset the input mask for this window **/
        XSelectInput(this->dpy,this->window,attr.all_event_masks);
	return;
      }
    }
  }
};


#include <pthread.h>

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectPointThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *) params)[0];
  XVPosition * point = (XVPosition *) ((VOIDP *) params)[1];
  XVDrawColor * c = (XVDrawColor *) ((VOIDP *) params)[2];

  pthread_mutex_lock(& pThis->selectMutex);
  XEvent xev;
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask   |
	                                  ButtonReleaseMask |
	                                  ButtonMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  point->reposition(-1, -1);

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bool bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask, & xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(bpev){

      point->reposition(xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_lock(& pThis->selectMutex);
      pThis->XVDrawWindowX<PIX >::drawPoint(point->PosX(),
					    point->PosY(),
					    *c);
      pthread_mutex_unlock(& pThis->selectMutex);
      pThis->swap_buffers();
      pThis->flush();
      pthread_mutex_lock(& pThis->selectMutex);
      XUndefineCursor(pThis->dpy, pThis->window);
      pThis->currentlySelecting = false;
      pThis->hasBeenSelected = true;
      pthread_mutex_unlock(& pThis->selectMutex);
      return NULL;
    }
  }
};

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectLineThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *)params)[0];
  XVPosition * p1 = (XVPosition *) ((VOIDP *) params)[1];
  XVPosition * p2 = (XVPosition *) ((VOIDP *) params)[2];
  XVDrawColor  * c  = (XVDrawColor  *) ((VOIDP *) params)[3];

  XEvent xev;

  pthread_mutex_lock(& pThis->selectMutex);
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask   |
	                                  ButtonReleaseMask |
	                                  ButtonMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  p1->reposition(-1, -1);
  p2->reposition(-1, -1);
  bool mouseDown = false;

  int lhandle;

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bool bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask, &xev);
    bool brev = XCheckMaskEvent(pThis->dpy, ButtonReleaseMask, &xev);
    bool pmev = XCheckMaskEvent(pThis->dpy, PointerMotionMask, &xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(bpev && !mouseDown){

      p1->reposition(xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_lock(& pThis->selectMutex);
      lhandle = pThis->XVStateWindowX<PIX>::drawLine(p1->PosX(),
						     p1->PosY(),
						     xev.xbutton.x,
						     xev.xbutton.y,
						     *c);
      pthread_mutex_unlock(& pThis->selectMutex);
      mouseDown = true;
    }else if(brev && mouseDown){

      p2->reposition(xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_lock(& pThis->selectMutex);
      XUndefineCursor(pThis->dpy, pThis->window);
      pThis->erase(lhandle);
      pThis->currentlySelecting = false;
      pThis->hasBeenSelected = true;
      pthread_mutex_unlock(& pThis->selectMutex);
       return NULL;
    }else if(pmev && mouseDown){

      pthread_mutex_lock(& pThis->selectMutex);
      pThis->XVDrawWindowX<PIX >::drawLine(p1->PosX(),
					   p1->PosY(),
					   p2->PosX(),
					   p2->PosY(), *c);
      pthread_mutex_unlock(& pThis->selectMutex);
      p2->reposition(xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_lock(& pThis->selectMutex);
      static_cast<XVDrawLine &>((*pThis)[lhandle]).point2.reposition(xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_unlock(& pThis->selectMutex);
      pThis->swap_buffers();
      pThis->flush();
    }
  }
};

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectRectThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *) params)[0];
  XVImageGeneric * region = (XVImageGeneric *) ((VOIDP *) params)[1];
  XVDrawColor      * c      = (XVDrawColor *) ((VOIDP *) params)[2];

  XEvent xev;

  pthread_mutex_lock(& pThis->selectMutex);
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask   |
	                                  ButtonReleaseMask |
	                                  ButtonMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  region->reposition(-1, -1); region->resize(-1, -1);
  bool mouseDown = false;
  bool bpev      = false;
  bool brev      = false;
  bool pmev      = false;
  int tmpWidth, tmpHeight;

  int rhandle;

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask, &xev);
    brev = XCheckMaskEvent(pThis->dpy, ButtonReleaseMask, &xev);
    pmev = XCheckMaskEvent(pThis->dpy, PointerMotionMask, &xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(bpev && !mouseDown){

      *region = XVImageGeneric(0, 0, xev.xbutton.x, xev.xbutton.y);
      pthread_mutex_lock(& pThis->selectMutex);
      rhandle = pThis->XVStateWindowX<PIX >::drawRectangle(region->PosX(),
							   region->PosY(),
							   region->Width(),
							   region->Height(), *c);
      pthread_mutex_unlock(& pThis->selectMutex);
      mouseDown = true;

    }else if(brev && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region->PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region->PosY()) > 0 &&
	 region->PosX() != -1 && region->PosY() != -1){

	region->resize(tmpWidth, tmpHeight);
	pthread_mutex_lock( & pThis->selectMutex);
	XUndefineCursor(pThis->dpy, pThis->window);
	pThis->erase(rhandle);
	pThis->currentlySelecting = false;
	pThis->hasBeenSelected = true;
	pthread_mutex_unlock( & pThis->selectMutex);
	return NULL;
      }
      *region = XVImageGeneric(-1, -1, -1, -1);
      mouseDown = false;
    }else if(pmev && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region->PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region->PosY()) > 0 &&
	 region->PosX() != -1 && region->PosY() != -1){

  	pthread_mutex_lock(& pThis->selectMutex);
  	pThis->XVDrawWindowX<PIX >::drawRectangle(region->PosX(),
						  region->PosY(),
						  region->Width(),
						  region->Height(), *c);
  	pthread_mutex_unlock(& pThis->selectMutex);
	region->resize(tmpWidth, tmpHeight);
	pthread_mutex_lock(& pThis->selectMutex);
	static_cast<XVDrawRect &>((*pThis)[rhandle]).region.resize(tmpWidth, tmpHeight);
	pthread_mutex_unlock(& pThis->selectMutex);
      }
    }
  }
};

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectEllipseThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *) params)[0];
  XVImageGeneric * region = (XVImageGeneric *) ((VOIDP *) params)[1];
  XVDrawColor * c = (XVDrawColor *) ((VOIDP *) params)[2];

  XEvent xev;

  pthread_mutex_lock(& pThis->selectMutex);
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask   |
	                                  ButtonReleaseMask |
	                                  ButtonMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;

  region->resize(-1, -1); region->reposition(-1, -1);
  bool mouseDown = false;
  int tmpWidth, tmpHeight;

  int ehandle;

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bool bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask, &xev);
    bool brev = XCheckMaskEvent(pThis->dpy, ButtonReleaseMask, &xev);
    bool pmev = XCheckMaskEvent(pThis->dpy, PointerMotionMask, &xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(bpev && !mouseDown){

      region->reposition(xev.xbutton.x, xev.xbutton.y);
      region->resize(0, 0);
      pthread_mutex_lock(& pThis->selectMutex);
      ehandle = pThis->XVStateWindowX<PIX >::drawEllipse(region->PosX(),
							 region->PosY(),
							 region->Width(),
							 region->Height(), *c);
      pthread_mutex_unlock(& pThis->selectMutex);
      mouseDown = true;
    }else if(brev && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region->PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region->PosY()) > 0 &&
	 region->PosX() != -1 && region->PosY() != -1){

	region->resize(tmpWidth, tmpHeight);
	pthread_mutex_lock(& pThis->selectMutex);
	XUndefineCursor(pThis->dpy, pThis->window);
	pThis->erase(ehandle);
	pThis->currentlySelecting = false;
	pThis->hasBeenSelected = true;
	pthread_mutex_unlock(& pThis->selectMutex);
	return NULL;
      }

      *region = XVImageGeneric(-1, -1, -1, -1);
      mouseDown = false;
    }else if(pmev && mouseDown){

      if((tmpWidth  = xev.xmotion.x - region->PosX()) > 0 &&
	 (tmpHeight = xev.xmotion.y - region->PosY()) > 0 &&
	 region->PosX() != -1 && region->PosY() != -1){

	pthread_mutex_lock(& pThis->selectMutex);
	pThis->XVDrawWindowX<PIX >::drawEllipse(region->PosX(), region->PosY(),
						region->Width(), region->Height(), *c);
	pthread_mutex_unlock(& pThis->selectMutex);
	region->resize(tmpWidth, tmpHeight);
	pthread_mutex_lock(& pThis->selectMutex);
	static_cast<XVDrawEllipse &>((*pThis)[ehandle]).region.resize(tmpWidth, tmpHeight);
	pthread_mutex_unlock(& pThis->selectMutex);
	pThis->swap_buffers();
	pThis->flush();
      }
    }
  }
};

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectSizedRectThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *) params)[0];
  XVImageGeneric * box = (XVImageGeneric *) ((VOIDP *) params)[1];
  XVSize * size = (XVSize *) ((VOIDP *) params)[2];
  XVDrawColor * c = (XVDrawColor *) ((VOIDP *) params)[3];

  XEvent xev;

  pthread_mutex_lock(& pThis->selectMutex);
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask   |
	                                  ButtonReleaseMask |
	                                  PointerMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  int first = true;
  XVPosition half(size->Width() / 2, size->Height() / 2);
  XVPosition currPos(-1, -1);
  box->resize(*size); box->reposition(currPos);

  int srhandle;

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bool pmev = XCheckMaskEvent(pThis->dpy, PointerMotionMask, &xev);
    bool bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask, &xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(pmev){

      if(!first){
	pthread_mutex_lock(& pThis->selectMutex);
	pThis->XVDrawWindowX<PIX >::drawRectangle(box->PosX(), box->PosY(),
  						  box->Width(), box->Height(),
  						  *c);
	pthread_mutex_unlock(& pThis->selectMutex);
	box->reposition(XVPosition(xev.xmotion.x, xev.xmotion.y) - half);
	pthread_mutex_lock(& pThis->selectMutex);
	static_cast<XVDrawRect &>((*pThis)[srhandle]).region.reposition((XVPosition)*box);
	pthread_mutex_unlock(& pThis->selectMutex);
      }else{

	box->reposition(XVPosition(xev.xmotion.x, xev.xmotion.y) - half);
	pthread_mutex_lock(& pThis->selectMutex);
	srhandle = pThis->XVStateWindowX<PIX >::
	  drawRectangle(box->PosX(), box->PosY(),
			box->Width(), box->Height(), *c);
	pthread_mutex_unlock(& pThis->selectMutex);
	first = false;
      }
      pThis->swap_buffers();
      pThis->flush();
    }else if(bpev){
      box->reposition(XVPosition(xev.xbutton.x, xev.xbutton.y) - half);
      pthread_mutex_lock(& pThis->selectMutex);
      XUndefineCursor(pThis->dpy, pThis->window);
      pThis->erase(srhandle);
      pThis->currentlySelecting = false;
      pThis->hasBeenSelected = true;
      pthread_mutex_unlock(& pThis->selectMutex);
      return NULL;
    }
  }
};

template <class PIX, class INPUT>
void *
XVThreadedWindowX<PIX, INPUT>::selectAngledRectThreadFunc(void * params) {

  XVThreadedWindowX<PIX, INPUT> * pThis
    = (XVThreadedWindowX<PIX, INPUT> *) ((VOIDP *) params)[0];
  XVPosition * ulcPos = (XVPosition *) ((VOIDP *) params)[1];
  XVSize * size = (XVSize *) ((VOIDP *) params)[2];
  double * angle = (double *) ((VOIDP *) params)[3];
  XVDrawColor * c = (XVDrawColor *) ((VOIDP *) params)[4];

  XEvent xev;

  pthread_mutex_lock(& pThis->selectMutex);
  XDefineCursor(pThis->dpy, pThis->window, pThis->crossHair);
  XSelectInput(pThis->dpy, pThis->window, ButtonPressMask |
	                                  ButtonReleaseMask |
                                          PointerMotionMask);
  pthread_mutex_unlock(& pThis->selectMutex);

  XV2Vec<double> ulc, urc, lrc, llc;

  bool topLineSelected = false;
  bool mouseDown = false;

  int topH, leftH, rightH, bottomH;

  while(1){

    pthread_mutex_lock(& pThis->selectMutex);
    bool pmev = XCheckMaskEvent(pThis->dpy, PointerMotionMask, &xev);
    bool bpev = XCheckMaskEvent(pThis->dpy, ButtonPressMask,   &xev);
    bool brev = XCheckMaskEvent(pThis->dpy, ButtonReleaseMask, &xev);
    pthread_mutex_unlock(& pThis->selectMutex);

    if(pmev){
      if(mouseDown && !topLineSelected) {
	pthread_mutex_lock(& pThis->selectMutex);
	pThis->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					    (int)rint(ulc.PosY()),
					    (int)rint(urc.PosX()),
					    (int)rint(urc.PosY()), *c);
	pthread_mutex_unlock(& pThis->selectMutex);
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	pthread_mutex_lock(& pThis->selectMutex);
	static_cast<XVDrawLine &>((*pThis)[topH]).point2.reposition((int)rint(urc.PosX()), (int)rint(urc.PosY()));
	pthread_mutex_unlock(& pThis->selectMutex);
      } else if(topLineSelected) {
	pthread_mutex_lock(& pThis->selectMutex);
	pThis->XVDrawWindowX<PIX>::drawLine((int)rint(ulc.PosX()),
					    (int)rint(ulc.PosY()),
					    (int)rint(llc.PosX()),
					    (int)rint(llc.PosY()), *c);
	pThis->XVDrawWindowX<PIX>::drawLine((int)rint(urc.PosX()),
					    (int)rint(urc.PosY()),
					    (int)rint(lrc.PosX()),
					    (int)rint(lrc.PosY()), *c);
	pThis->XVDrawWindowX<PIX>::drawLine((int)rint(llc.PosX()),
					    (int)rint(llc.PosY()),
					    (int)rint(lrc.PosX()),
					    (int)rint(lrc.PosY()), *c);
	pthread_mutex_unlock(& pThis->selectMutex);
	XV2Vec<double> currentPos(xev.xbutton.x, xev.xbutton.y);
	XV2Vec<double> topVec = urc - ulc;
	XV2Vec<double> tmpVec = currentPos - ulc;
	XV2Vec<double> projVec = ((topVec * tmpVec) * topVec)
	  / (topVec.length() * topVec.length());
	XV2Vec<double> heightVec = currentPos - (projVec + ulc);
	llc = ulc + heightVec;
	lrc = llc + topVec;
	pthread_mutex_lock(& pThis->selectMutex);
	static_cast<XVDrawLine &>((*pThis)[leftH]).point2.reposition((XVPosition)llc);
	static_cast<XVDrawLine &>((*pThis)[rightH]).point2.reposition((XVPosition)lrc);
	static_cast<XVDrawLine &>((*pThis)[bottomH]).point1.reposition((XVPosition)llc);
	static_cast<XVDrawLine &>((*pThis)[bottomH]).point2.reposition((XVPosition)lrc);
	pthread_mutex_unlock(& pThis->selectMutex);
      }
      pThis->swap_buffers();
      pThis->flush();
    } else if(bpev) {
      mouseDown = true;
      if(!topLineSelected) {
	ulc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	urc = ulc;
	pthread_mutex_lock(& pThis->selectMutex);
	topH = pThis->XVDrawable::drawLine(ulc, urc);
	pthread_mutex_unlock(& pThis->selectMutex);
	pThis->swap_buffers();
	pThis->flush();
      }
    } else if(brev) {
      if(mouseDown && !topLineSelected) {
	topLineSelected = true;
	urc = XV2Vec<double>(xev.xbutton.x, xev.xbutton.y);
	lrc = urc;
	llc = ulc;
	pthread_mutex_lock(& pThis->selectMutex);
	topH = pThis->XVDrawable::drawLine(ulc, urc);
	leftH = pThis->XVDrawable::drawLine(ulc, llc);
	rightH = pThis->XVDrawable::drawLine(urc, lrc);
	bottomH = pThis->XVDrawable::drawLine(llc, lrc);
	pthread_mutex_unlock(& pThis->selectMutex);
	pThis->swap_buffers();
	pThis->flush();
      }else if(topLineSelected) {
	XV2Vec<double> diffVec = ulc - llc;
	pthread_mutex_lock(& pThis->selectMutex);
	if(diffVec.x() < 0){
	  *angle = -acos((XV2Vec<double>(0, -1) * diffVec)
					   / diffVec.length());
	}else{
	  *angle = acos((XV2Vec<double>(0, -1) * diffVec)
		       / diffVec.length());
	}
	*size = XVSize((int)rint((urc - ulc).length()), (int)rint(diffVec.length()));
	*ulcPos = (XVPosition) ulc;
	pThis->erase(topH);
	pThis->erase(leftH);
	pThis->erase(rightH);
	pThis->erase(bottomH);
	XUndefineCursor(pThis->dpy, pThis->window);
	pThis->currentlySelecting = false;
	pThis->hasBeenSelected = true;
	pthread_mutex_unlock(& pThis->selectMutex);
	return NULL;
      }
      mouseDown = false;
    }
  }
};


template <class T, class I>
inline void
XVThreadedWindowX<T, I>::selectPoint(XVPosition & p,
				  XVDrawColor c,bool draw_flag ){

  if(!draw_flag) XVThreadedWindowException((char*)"Not implemented");
  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectPointThreadFuncColor = c;
  static VOIDP selectPointThreadFuncParams[3];
  selectPointThreadFuncParams[0] = (VOIDP) this;
  selectPointThreadFuncParams[1] = (VOIDP) & p;
  selectPointThreadFuncParams[2] = (VOIDP) & selectPointThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<T, I>::selectPointThreadFunc,
		 (void *) selectPointThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<T>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class T, class I>
inline  void
XVThreadedWindowX<T, I>::selectLine(XVPosition & p1,
				    XVPosition & p2,
				    XVDrawColor c ){

  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectLineThreadFuncColor = c;
  static VOIDP selectLineThreadFuncParams[4];

  selectLineThreadFuncParams[0] = (VOIDP) this;
  selectLineThreadFuncParams[1] = (VOIDP) & p1;
  selectLineThreadFuncParams[2] = (VOIDP) & p2;
  selectLineThreadFuncParams[3] = (VOIDP) & selectLineThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<T, I>::selectLineThreadFunc,
		 (void *) selectLineThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<T>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class T, class I>
inline void
XVThreadedWindowX<T, I>::selectRectangle(XVImageGeneric & g,
					 XVDrawColor c ){

  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectRectangleThreadFuncColor = c;
  static VOIDP selectRectangleThreadFuncParams[3];

  selectRectangleThreadFuncParams[0] = (VOIDP) this;
  selectRectangleThreadFuncParams[1] = (VOIDP) & g;
  selectRectangleThreadFuncParams[2] = (VOIDP) & selectRectangleThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<T, I>::selectRectThreadFunc,
		 (void *) selectRectangleThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<T>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class T, class I>
inline void
XVThreadedWindowX<T, I>::selectEllipse(XVImageGeneric & g,
				       XVDrawColor c ){

  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectEllipseThreadFuncColor = c;
  static VOIDP selectEllipseThreadFuncParams[3];

  selectEllipseThreadFuncParams[0] = (VOIDP) this;
  selectEllipseThreadFuncParams[1] = (VOIDP) & g;
  selectEllipseThreadFuncParams[2] = (VOIDP) & selectEllipseThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<T, I>::selectEllipseThreadFunc,
		 (void *) selectEllipseThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<T>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class T, class I>
inline void
XVThreadedWindowX<T, I>::selectSizedRect(XVImageGeneric & g, const XVSize & s,
					 XVDrawColor c ){

  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectSizedRectThreadFuncColor = c;
  static XVSize      selectSizedRectThreadFuncSize = s;
  static VOIDP selectSizedRectThreadFuncParams[4];

  selectSizedRectThreadFuncParams[0] = (VOIDP) this;
  selectSizedRectThreadFuncParams[1] = (VOIDP) & g;
  selectSizedRectThreadFuncParams[2] = (VOIDP) & selectSizedRectThreadFuncSize;
  selectSizedRectThreadFuncParams[3] = (VOIDP) & selectSizedRectThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<T, I>::selectSizedRectThreadFunc,
		 (void *) selectSizedRectThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<T>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class PIX, class INPUT>
inline void
XVThreadedWindowX<PIX, INPUT>::selectAngledRect(XVPosition & ulcPos, XVSize & size,
					 double & angle,
					 XVDrawColor c ) {

  hasBeenSelected = false;
  if(currentlySelecting)
    throw XVThreadedWindowException((char*)"Already Selecting");
  currentlySelecting = true;

  static XVDrawColor selectAngledRectThreadFuncColor = c;
  static VOIDP       selectAngledRectThreadFuncParams[5];

  selectAngledRectThreadFuncParams[0] = (VOIDP) this;
  selectAngledRectThreadFuncParams[1] = (VOIDP) & ulcPos;
  selectAngledRectThreadFuncParams[2] = (VOIDP) & size;
  selectAngledRectThreadFuncParams[3] = (VOIDP) & angle;
  selectAngledRectThreadFuncParams[4] = (VOIDP) & selectAngledRectThreadFuncColor;

  pthread_t selectionThread;
  pthread_create(& selectionThread, NULL,
		 & XVThreadedWindowX<PIX, INPUT>::selectAngledRectThreadFunc,
		 (void *) selectAngledRectThreadFuncParams);

  while(!hasBeenSelected) {

    this->CopySubImage((XVImageRGB<PIX>)(vid->next_frame_continuous()));
    this->swap_buffers();
    this->flush();
  }
};

template <class T, class I>
void XVThreadedWindowX<T, I>::CopySubImage(const XVImageRGB<T> & image) {
  pthread_mutex_lock( & selectMutex );
  this->XVStateWindowX<T>::CopySubImage(image);
  pthread_mutex_unlock( & selectMutex );
};

template <class T, class I>
void XVThreadedWindowX<T, I>::CopyImage(int which, u_short s_flip){
  pthread_mutex_lock( & selectMutex );
  this->XVStateWindowX<T>::CopyImage(which, s_flip);
  pthread_mutex_unlock( & selectMutex );
};

template <class T, class I>
void XVThreadedWindowX<T, I>::swap_buffers() {
  pthread_mutex_lock( & selectMutex );
  this->XVWindowX<T>::swap_buffers();
  pthread_mutex_unlock( & selectMutex );
};

template <class T, class I>
void XVThreadedWindowX<T, I>::flush() {
  pthread_mutex_lock( & selectMutex );
  this->XVWindowX<T>::flush();
  pthread_mutex_unlock( & selectMutex );
};

template class XVDrawWindowX<XV_RGB15>;
template class XVDrawWindowX<XV_RGB16>;
template class XVDrawWindowX<XV_RGB24>;
template class XVDrawWindowX<XV_RGBA32>;

template class XVStateWindowX<XV_RGB15>;
template class XVStateWindowX<XV_RGB16>;
template class XVStateWindowX<XV_RGB24>;
template class XVStateWindowX<XV_RGBA32>;

template class XVInteractWindowX<XV_RGB15>;
template class XVInteractWindowX<XV_RGB16>;
template class XVInteractWindowX<XV_RGB24>;
template class XVInteractWindowX<XV_RGBA32>;

#define _MANINST_THREADED_WINDOW_(_VID_TYPE_) \
template class XVThreadedWindowX<XV_RGB15, _VID_TYPE_ >; \
template class XVThreadedWindowX<XV_RGB16, _VID_TYPE_ >; \
template class XVThreadedWindowX<XV_RGB24, _VID_TYPE_ >; \
template class XVThreadedWindowX<XV_RGBA32, _VID_TYPE_ >;

#ifdef THREADEDWINDOW_ALLRGB

_MANINST_THREADED_WINDOW_(XVImageRGB<XV_RGB15 >);
_MANINST_THREADED_WINDOW_(XVImageRGB<XV_RGB24 >);

#endif

_MANINST_THREADED_WINDOW_(XVImageRGB<XV_RGB16 >);
_MANINST_THREADED_WINDOW_(XVImageRGB<XV_RGBA32 >);

#ifdef THREADEDWINDOW_SCALAR

_MANINST_THREADED_WINDOW_(XVImageScalar<u_char >);
_MANINST_THREADED_WINDOW_(XVImageScalar<u_short >);
_MANINST_THREADED_WINDOW_(XVImageScalar<u_int >);
_MANINST_THREADED_WINDOW_(XVImageScalar<char >);
_MANINST_THREADED_WINDOW_(XVImageScalar<short >);
_MANINST_THREADED_WINDOW_(XVImageScalar<int >);
_MANINST_THREADED_WINDOW_(XVImageScalar<float >);
_MANINST_THREADED_WINDOW_(XVImageScalar<double >);

#endif

void XV_temporary_fix_for_the_ipp_bug_with_pthread(void)
{ pthread_attr_t attr; pthread_attr_init(&attr); }
