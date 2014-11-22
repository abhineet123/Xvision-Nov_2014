# ifndef _FRP_TRACK_H_
# define _FRP_TRACK_H_

# include "Behavior.h"

# include "XVFeature.h"
# include "XVWindowX.h"

namespace frp {

// Helper class with typedefs. Assuming:
// 1. FEATURE is derived from XVFeature.
// 2. IMAGE is derived from XVImageBase.

template<class FEATURE> 
struct FeatureTypes {
  typedef FEATURE F ;
  typedef typename FEATURE::IMAGE I ;
  typedef typename FEATURE::STATE S ;
  typedef typename FEATURE::IMAGE_INTERNAL II ;
  typedef typename I::PIXELTYPE P ;
  typedef XVFeature<I,II,S> B ;
  typedef const S& (B::*Step1)(const I&);
  typedef const S& (B::*Step2)(const I&,const S&);
  typedef ClassPtrFun1<Step1,S,I,F> Fn1 ;
  typedef ClassPtrFun2<Step2,S,I,S,F> Fn2 ;
  typedef Lift1Combinator<S,I,Fn1> Comb1 ;
  typedef Lift2Combinator<S,I,S,Fn2> Comb2 ;

  static inline Step1 step1(void) { return &B::step ; }
  static inline Step2 step2(void) { return &B::step ; }
};

// LoopTracker and Tracker, both wrapper of XVFeature

template<class FEATURE>
class LoopTracker : public FeatureTypes<FEATURE>::Comb1, 
                    protected FeatureTypes<FEATURE> {
 public:
  typedef I Image ;
  typedef S State ;

  LoopTracker() : Comb1(Fn1(step1(),new F())) {}
  LoopTracker( const F& f ) : Comb1(Fn1(step1(),new F(f))) {}
  FEATURE * operator -> () { return fn.operator->() ; }
  const FEATURE * operator -> () const { return fn.operator->() ; }
  void show( XVDrawable& x ) const { return const_cast<Fn1&>(fn)->show(x) ; }

  S interactiveInit( BehaviorRef<I> x ) {
    I image = x->getNext() ;
    XVInteractWindowX<P> win(image);
    (&win)->map();
    win.CopySubImage(image);
    win.swap_buffers();
    win.flush();
    S r = fn->interactiveInit(win,image);
    win.unmap();
    return r ;
  }
};

template<class FEATURE>
class Tracker : public FeatureTypes<FEATURE>::Comb2, 
                protected FeatureTypes<FEATURE> {
 public:
  typedef I Image ;
  typedef S State ;

  Tracker() : Comb2(Fn2(step2(),new F())) {}
  Tracker( const F& f ) : Comb2(Fn2(step2(),new F(f))) {}
  FEATURE * operator -> () { return fn.operator->() ; }
  const FEATURE * operator -> () const { return fn.operator->() ; }
  void show( XVDrawable& x ) const { return const_cast<Fn2&>(fn)->show(x) ; }

  S interactiveInit( BehaviorRef<I> x ) {
    I image = x->getNext() ;
    XVInteractWindowX<P> win(image);
    (&win)->map();
    win.CopySubImage(image);
    win.swap_buffers();
    win.flush();
    S r = fn->interactiveInit(win,image);
    win.unmap();
    return r ;
  }
};


} // namespace frp 

# endif // _FRP_TRACK_H_
