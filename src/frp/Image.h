# ifndef _FRP_IMAGE_H_
# define _FRP_IMAGE_H_

# include "Behavior.h"
# include "Event.h"

# include "XVVideo.h"
# include "XVWindowX.h"
# include "XVRemoteWindowX.h"
# include "XVImageRGB.h"

namespace frp {

// Video grabber classes, wrapper of XVVideo

template<class T>
class VideoModule : public BehaviorModule<T> {
 protected:
  XVVideo<T> * v ;
 public:
  VideoModule( XVVideo<T>* vin ) : v(vin) {}
  void compute(void) { value = v->next_frame_continuous() ; }
};

template<class T>
inline BehaviorRef<T> video( XVVideo<T>& v ) {
  return new VideoModule<T>(&v);
}

template<class T, template<class I> class V>
inline BehaviorRef<T> video() {
  return new VideoModule<T>( new V<T> );
}
template<class T, template<class I> class V, class D1>
inline BehaviorRef<T> video( const D1& d1 ) {
  return new VideoModule<T>( new V<T>(d1) );
}
template<class T, template<class I> class V, class D1, class D2>
inline BehaviorRef<T> video( const D1& d1, const D2& d2 ) {
  return new VideoModule<T>( new V<T>(d1,d2) );
}

// Extented class for XVDrawWindowX

template<class T>
class EventWindowX : public XVRemoteWindowX<T> {
 protected:
  Maybe<XVPosition> last_lbp, last_rbp, last_mbp ;
  Maybe<XID> last_key ;
  virtual bool check(void) {
    int r, x[2];
    if( r = check_events(x) ) {
      if( r & BUTTON_PRESSED ) {
	if( r & LEFT_BUTTON ) {
	  last_lbp = Maybe<XVPosition>(XVPosition(x[0],x[1])) ;
	}else if( r & MIDDLE_BUTTON ) {
	  last_mbp = Maybe<XVPosition>(XVPosition(x[0],x[1])) ;
	}else if( r & RIGHT_BUTTON ) {
	  last_rbp = Maybe<XVPosition>(XVPosition(x[0],x[1])) ;
	}
      }else if( r & KEY_PRESSED ) {
	last_key = Maybe<XID>( x[0] );
      }
      return true ;
    }else {
      return false ;
    }
  }
 public:
  EventWindowX( const XVImageRGB<T> & im ) : 
    XVRemoteWindowX<T>(im,0,0,"XVision2/FRP - ", 
		       ExposureMask | KeyPressMask | ButtonPressMask ) {}
  ~EventWindowX() {}

  virtual Maybe<XVPosition> lbp(void) { 
    Maybe<XVPosition> result ; 
    if( !last_lbp ) check() ; 
    result = last_lbp ;
    last_lbp = Maybe<XVPosition>() ;
    return result ;
  }
  virtual Maybe<XVPosition> rbp(void) { 
    Maybe<XVPosition> result ; 
    if( !last_rbp ) check() ; 
    result = last_rbp ;
    last_rbp = Maybe<XVPosition>() ;
    return result ;
  }
  virtual Maybe<XVPosition> mbp(void) { 
    Maybe<XVPosition> result ; 
    if( !last_mbp ) check() ; 
    result = last_mbp ;
    last_mbp = Maybe<XVPosition>() ;
    return result ;
  }
  virtual Maybe<char> key(void) {
    Maybe<char> result ;
    if( !last_key ) check() ;
    result = last_key ;
    last_key = Maybe<unsigned int>() ;
    return result ;
  }
  virtual Maybe<XID> keyx(void) {
    Maybe<XID> result ;
    if( !last_key ) check() ;
    result = last_key ;
    last_key = Maybe<XID>() ;
    return result ;
  }
};

// Helper class for types used by DisplayModule and Display

struct ShowClasses {
  struct Showable {
    virtual void show( XVDrawable& ) const = 0 ;
    virtual ~Showable() {};
  };
  template<class T>
  class ShowModule : public BehaviorModule<Showable *> {
   protected:
    BehaviorBase<T> * p ;

    struct ShowableObj : public Showable {
      const T* t ;
      void show( XVDrawable& d ) const { t->show(d); }
    } object ;
   public:
    ShowModule( BehaviorBase<T> * x ) : p(x) { attach(p) ; value = &object ; }
    void compute(void) { object.t = &(p->get()) ; }
  };

  template<class T>
  class ShowStatic : public BehaviorModule<Showable *> {
   protected:
    struct ShowableObj : public Showable {
      const T* t ;
      void show( XVDrawable& d ) const { t->show(d); }
    } object ;
   public:
    ShowStatic( T* t ) { value = &object ; object.t = t ; }
  };

 typedef std::list<BehaviorBase<Showable*>* > ShowList ;
};

// Display window class, wrapper of XVDrawWindowX

template<class T>
class DisplayModule : public BehaviorModule<XVImageRGB<T> > , 
                      protected ShowClasses {
 protected:
  EventWindowX<T> * w ;
  BehaviorBase<XVImageRGB<T> > * p ;
  ShowList l ;
 public:
  DisplayModule( EventWindowX<T> * v, BehaviorBase<XVImageRGB<T> > * x, 
		 ShowList y ) : 
    w(v), p(x), l(y) { 
    attach(p); 
    for( ShowList::iterator i = l.begin() ; i != l.end() ; i ++ ) {
      attach(*i) ;
    }
  }
  void compute(void) { 
    w->CopySubImage(p->get());
    for( ShowList::iterator i = l.begin() ; i != l.end() ; i ++ ) {
      (*i)->get()->show( *w );
    }
    w->swap_buffers();
    w->flush();
    value = w->getEventsImage() ;
  }
};

// display combinator 

template<class T>
class WindowDisplay : public BehaviorCombinator<XVImageRGB<T> >, 
                      protected ShowClasses {
 protected:
  EventWindowX<T> * w ;
  ShowList l ;

  BehaviorRef<Maybe<XVPosition> > lbp_r, rbp_r, mbp_r ;
  BehaviorRef<Maybe<char> >  key_r ;
 
  const static int defX = 320, defY = 240 ;
 public:
  WindowDisplay() : w(new EventWindowX<T>(XVImageRGB<T>(defX,defY))), 
    lbp_r(0), rbp_r(0), mbp_r(0), key_r(0) { w->map();}
  BehaviorRef<XVImageRGB<T> > operator() ( BehaviorRef<XVImageRGB<T> > x ) 
    const { return new DisplayModule<T>(w,x.get(),l);}

  template<class D>
  WindowDisplay<T>& operator << ( BehaviorRef<D> x ) 
    { l.push_back( new ShowModule<D>(x.get()) ); return *this ; }
  template<class D>
  WindowDisplay<T>& operator << ( const Behavior<D>& x ) 
    { return (*this) << x.getRef() ; }
  template<class D>
  WindowDisplay<T>& operator << ( Behavior<D>& x ) 
    { return (*this) << x.getRef() ; }

  template<class D>
  WindowDisplay<T>& operator << ( D& x ) 
    { l.push_back( new ShowStatic<D>(&x) ); return *this ; }

  BehaviorRef<Maybe<XVPosition> > lbp(void) 
    { if( ! lbp_r.get() ) lbp_r = lift(&EventWindowX<T>::lbp,w)() ; 
    return lbp_r ; }
  BehaviorRef<Maybe<XVPosition> > rbp(void) 
    { if( ! rbp_r.get() ) rbp_r = lift(&EventWindowX<T>::rbp,w)() ; 
    return rbp_r ; }
  BehaviorRef<Maybe<XVPosition> > mbp(void) 
    { if( ! mbp_r.get() ) mbp_r = lift(&EventWindowX<T>::mbp,w)() ; 
    return mbp_r ; }
  BehaviorRef<Maybe<char> > key(void)
    { if( ! key_r.get() ) key_r = lift(&EventWindowX<T>::key,w)() ; 
    return key_r ; }
};

template<class T>
inline WindowDisplay<T> display(void) {
  return WindowDisplay<T>() ;
}

// lift pixel function to image function

template<class T, class Fn>
class LiftPixel0Module : public BehaviorModule<XVImageRGB<T> > {
 protected:
  Fn fn ;
  XVSize size ;
 public:
  LiftPixel0Module( const Fn& f, const XVSize& s ) : fn(f), size(s) {}
  void compute(void) {
    value.resize(size);
    for( int i = 0 ; i < size.Height() ; i ++ ) {
      T * ptr = const_cast<T*>(value[i]) ;
      for( int j = 0 ; j < size.Width() ; j ++ ) {
	ptr[j] = fn(j,i) ;
      }
    }
  }
};

template<class T, class Fn>
class LiftPixel0Combinator : public BehaviorCombinator<XVImageRGB<T> > {
 protected:
  Fn fn ;
  XVSize size ;
 public:
  LiftPixel0Combinator( const Fn& f, const XVSize& s ) : fn(f), size(s) {}
  BehaviorRef<XVImageRGB<T> > operator() (void) const 
    { return new LiftPixel0Module<T,Fn>(fn,size); }
};

template<class T>
struct LiftTraits2<T (*)(int,int),XVSize> {
  typedef T (*Arg)(int,int) ;
  typedef LiftPixel0Combinator<T,Arg> Result ;
  typedef Result Agent ;
};

// Image functions

template<class T1, class T2, RGB_Projection proj>
XVImageScalar<T2> FRP_RGBtoScalar( const XVImageRGB<T1> & image ) {
  XVImageScalar<T2> targ ;
  return RGBtoScalar( image, targ, proj );
}

template<class T1, class T2, RGB_Projection proj>
BehaviorRef<XVImageScalar<T2> > RGBtoScalar( BehaviorRef<XVImageRGB<T1> > x ) {
  return lift(&FRP_RGBtoScalar<T1,T2,proj>)(x) ;
}
template<class T1, class T2, RGB_Projection proj>
BehaviorRef<XVImageScalar<T2> > RGBtoScalar( const Behavior<XVImageRGB<T1> >& x ) {
  return RGBtoScalar<T1,T2>(x.getRef()) ;
}
template<class T1, class T2>
BehaviorRef<XVImageScalar<T2> > RGBtoScalar( BehaviorRef<XVImageRGB<T1> > x ) {
  return lift(&FRP_RGBtoScalar<T1,T2,XV_RGB_INTENSITY>)(x) ;
}
template<class T1, class T2>
BehaviorRef<XVImageScalar<T2> > RGBtoScalar( const Behavior<XVImageRGB<T1> >& x ) {
  return RGBtoScalar<T1,T2>(x.getRef()) ;
}

template <class T1, class T2>
XVImageRGB<T2> FRP_ScalartoRGB( const XVImageScalar<T1> & image ) {
  XVImageRGB<T2> targ ;
  return ScalartoRGB( image, targ );
}

template<class T1, class T2>
BehaviorRef<XVImageRGB<T2> > ScalartoRGB( BehaviorRef<XVImageScalar<T1> > x ) {
  return lift(&FRP_ScalartoRGB<T1,T2>)(x) ;
}
template<class T1, class T2>
BehaviorRef<XVImageRGB<T2> > ScalartoRGB( const Behavior<XVImageScalar<T1> >& x ) {
  return ScalartoRGB<T1,T2>(x.getRef()) ;
}

} // namespace frp

# endif // _FRP_IMAGE_H_
