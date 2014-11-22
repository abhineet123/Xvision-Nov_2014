//# include <stdio.h>

# ifndef _FRP_BEHAVIOR_H_
# define _FRP_BEHAVIOR_H_

# include <iostream>
# include <vector>
# include <list>
# include <utility>
# include <cmath>

# include "Functoid.h"
# include "Lambda.h"
# include "NoCollectable.h"

namespace frp {

// Time stuff

typedef double Time ;
typedef unsigned int Sample ;

inline Time toTime( double seconds ) {
  return seconds ;
}
inline double toSecond( Time t ) {
  return t ;
}

void timeReset(void);
Time time(void);

// Helper functions

template<class T1, class T2>
inline T1 implicit_cast( const T2& x ) {
  return x ;
}

template<class T>
inline T derived_cast_fun( T x ) {
  return x ;
}
template<class T>
inline T derived_cast_fun( ... ) {
  return 0 ;
}
template<class T1, class T2>
inline T1 derived_cast( T2 x ) {
  return derived_cast_fun<T1>( x );
}
template<class T>
inline void try_gc_mark( T* x ) {
  if( BasicCollectable * p = derived_cast<BasicCollectable*,T*>(x) ) {
    p->gc_mark();
  }
}

// Interface of generic (not of certain type) behavior

class GenericBehavior : public BasicCollectable {
 protected: 
  const static Sample beginning = 0 ;
  
  typedef std::vector<GenericBehavior*> Waitings ;

  static Waitings todo ;
  static Sample todo_now ;
  
 public: 
  enum { gc_def = 1000 } ;
  
  virtual ~GenericBehavior() {} ;
  virtual bool update( Sample now ) = 0 ; // update this and all dependencies 
  virtual void next(void) = 0 ;      // advance to next sample
  virtual void compute(void) = 0 ;   // do local computation
  virtual void attach( GenericBehavior* b ) = 0 ; // add to dependency
  virtual void detach( GenericBehavior* b ) = 0 ; // remove from dependency
 
  static void add_todo( GenericBehavior* b, Sample now ) ;
  static void clean_up() ;
  void run( Time interval = 0, Time totaltime = -1, int gc_frames = gc_def );
};

// Interface of behavior of T

template<class T>
class BehaviorBase : public GenericBehavior {
 public:
  virtual const T& get(void) const = 0 ;   // get current behavior
  const T& getNext(void) { next() ; return get() ; }
};

// Module that does the real yet general work

template<class T> 
class BehaviorModule : public BehaviorBase<T> {
  typedef std::list<GenericBehavior*> Dependencies ;

  Dependencies minors ;

 protected: 
  Sample last ;
  bool flipping ;
  bool updated ;
  T value ;

 public:
  BehaviorModule() : last(beginning), flipping(false), updated(true) {}
  BehaviorModule( const T& init ) : 
    last(beginning), flipping(false), updated(true), value(init) {}
  virtual ~BehaviorModule() {}

  virtual void gc_more_marking() {}
  void gc_marking() { // derived from BasicCollectable, for garbage collection
    try_gc_mark( &value );
    for( Dependencies::iterator i = minors.begin() ; i != minors.end() ; i++ ){
      (*i)->gc_mark();
    }
    gc_more_marking();
  }
  bool update( Sample now ) {
    if( flipping ) {
      return false ;
    }
    if( now == last ) {
      return true ;
    }
    flipping = true ;
    updated = true ;
    for( Dependencies::iterator i = minors.begin(); i != minors.end(); i++ ){
      updated &= (*i)->update(now);
    }
    if( updated ) {
      last = now ;
      compute();
    }
    flipping = false ;
    return updated ;
  }
  void next(void) { update( last+1 ); clean_up(); }
  void compute(void) {}
  void attach( GenericBehavior* b ) { minors.push_back(b) ; }
  void detach( GenericBehavior* b ) {
    for( Dependencies::iterator i = minors.begin(); i != minors.end(); i++ ) {
      if( (*i) == b ) {
	minors.erase(i);
	break ;
      }
    }
  }
  const T& get(void) const { return value; } 
};

// Envelope class
template<class T>
class BehaviorRef : public BasicCollectable {
 protected:
  BehaviorBase<T> * module ;
 public:
  enum { gc_def = GenericBehavior::gc_def };

  BehaviorRef( BehaviorBase<T> * m ) : module(m) {}
  BehaviorRef( const T& init ) : module(new BehaviorModule<T>(init)) {}

  void gc_marking() { if( module ) module->gc_mark(); }

  /*
  operator BehaviorBase<T> * (void) { return module ; }
  operator const BehaviorBase<T> * (void) const { return module ; }
  */
  const BehaviorBase<T> * get(void) const { return module ; }
  BehaviorBase<T> * get(void) { return module ; }
  const BehaviorBase<T> * operator ->(void) const { return module ; }
  BehaviorBase<T> * operator ->(void) { return module ; }

  void run( Time interval = 0, Time totaltime = -1, int gc_frames = gc_def )
    { module->run( interval, totaltime, gc_frames ); }

 template<class D>
  BehaviorRef<D> cast(void) const { return lift(&implicit_cast<D,T>)(*this); }
  template<class D>
  operator BehaviorRef<D> (void) const { return cast<D>(); }

};

template<class T>
inline BehaviorRef<T> constB( const T& init ) {
  return BehaviorRef<T>(init);
}

// Behavior class for variables

template<class T>
class Behavior : public BehaviorBase<T> {
 protected:
  BehaviorRef<T> *ref ;
 public:
  Behavior() : ref(new BehaviorRef<T>((BehaviorBase<T> *)(0))) {} ;
  Behavior( const T& init ) : ref(new BehaviorRef<T>(init)) {}
  Behavior( BehaviorRef<T> init ) : ref(new BehaviorRef<T>(init)) {}

  void gc_marking() { if( ref ) ref->gc_mark() ; }

  bool update( Sample now ) { return (*ref)->update( now ) ; }
  void next(void) { (*ref)->next() ; }
  void compute(void) { (*ref)->compute() ; }
  void attach( GenericBehavior *b ) { (*ref)->attach(b); }
  void detach( GenericBehavior *b ) { (*ref)->detach(b); }
  const T& get(void) const { return (*ref)->get(); }

  Behavior<T>& operator = ( BehaviorRef<T> x ) { *ref = x ; return *this ; }
  Behavior<T>& operator = ( const Behavior<T>& x ) 
    { *ref = x.getRef() ; return *this ; }  /* this is controversy */
  Behavior<T>& operator = ( const T& x ) 
    { *ref = BehaviorRef<T>(x) ; return *this ; }
  BehaviorRef<T> getRef(void) const { return new Behavior<T>(*this) ; }
  operator BehaviorRef<T> (void) const { return getRef(); }
  template<class D>
  operator BehaviorRef<D> (void) const { return getRef(); }
};

// Type definitions for combinators

template<class T> 
struct BehaviorCombinator {
  typedef T Value ;
  typedef BehaviorRef<T> Result ;
};

// 'Pipeline' function

template<class T>
struct CombinatorTraits {             // default
  typedef typename T::Result Result ;
};

template<class T1, class T2>
inline typename CombinatorTraits<T1>::Result 
operator <<=( const T1& x1, const T2& x2 ) {
  return x1(x2);
}

// Delay 

template<class T>
class DelayModule : public BehaviorModule<T> {
 protected:
  BehaviorBase<T>* p ;
  T prefetch ;
 public:
  DelayModule( const T& init, BehaviorBase<T>* x ) : p(x), prefetch(init) 
    { attach(p); }
  void gc_more_marking() { try_gc_mark( &prefetch ); }
  bool update( Sample now ) {
    if( now != last ) {
      value = prefetch ;
      last = now ;
      updated = false ;
    }else {
      updated = true ;
    }
    updated |= p->update( now );
    if( ! updated ) {
      add_todo( this, now );
    }else {
      prefetch = p->get() ;
    }
    return true ;
  }
};

template<class T>
class DelayCombinator : public BehaviorCombinator<T> {
 protected:
  const T& first ;
 public:
  DelayCombinator( const T& init ) : first(init) {}
  BehaviorRef<T> operator() ( BehaviorRef<T> x ) const
    { return new DelayModule<T>( first, x.get() ); }
  BehaviorRef<T> operator() ( const Behavior<T>& x ) const
    { return this->operator ()( x.getRef() ); }

  Fun1<BehaviorRef<T>,BehaviorRef<T> > fun(void) const
    { return Fun1<BehaviorRef<T>,BehaviorRef<T> >
        (new DirectFun1<DelayCombinator<T>,BehaviorRef<T>,
	 BehaviorRef<T> >(*this)) ; }
};

template<class T>
inline DelayCombinator<T> delayB( const T& x ) {
  return DelayCombinator<T>(x) ;
}

template<class T>
class Delay1Module : public BehaviorModule<T> {
 protected:
  BehaviorBase<T>* p ;
  T prefetch ;
  bool inited ;
 public:
  Delay1Module( BehaviorBase<T>* x ) : p(x), inited(false) { attach(p); }
  void gc_more_marking() { try_gc_mark( &prefetch ); }
  bool update( Sample now ) { 
    if( ! inited ) {
      updated = p->update( now );
      if( updated ) {
	value = prefetch = p->get() ;
	inited = true ;
	return true ;
      }else {
	return false ;
      }
    }else {
      if( now != last ) {
	value = prefetch ;
	last = now ;
	updated = false ;
      }else {
	updated = true ;
      }
      updated |= p->update( now );
      if( ! updated ) {
	add_todo( this, now );
      }else {
	prefetch = p->get() ;
      }
      return true ;
    }
  }
};

template<class T>
inline BehaviorRef<T> delay1B( BehaviorRef<T> x ) {
  return new Delay1Module<T>(x.get()) ;
}
template<class T>
inline BehaviorRef<T> delay1B( const Behavior<T>& x ) {
  return delay1B( x.getRef() );
}

// Lift functions to behavior level

template<class T> 
struct LiftTraits {
  typedef T Arg ;
  typedef T Agent ;
  typedef Behavior<T> Result ;
};

template<class T>
inline typename LiftTraits<T>::Result lift( T f ) {
  return typename LiftTraits<T>::Result
    ( typename LiftTraits<T>::Agent(f) );
}
template<class T>
inline typename LiftTraits<T>::Result liftB( T f ) {
  return lift(f);
}

template<class T, class D> 
struct LiftTraits2 : public LiftTraits<T> {};

template<class T, class D>
inline typename LiftTraits2<T,D>::Result lift( T f, D d ) {
  return typename LiftTraits2<T,D>::Result
    ( typename LiftTraits2<T,D>::Agent(f,d) );
}
template<class T, class D>
inline typename LiftTraits2<T,D>::Result liftB( T f, D d ) {
  return lift(f,d);
}

template<class T1, class T2>
inline BehaviorRef<T1> cast( BehaviorRef<T2> x ) {
  return lift(&implicit_cast<T1,T2>)(x) ;
}
template<class T1, class T2>
inline BehaviorRef<T1> cast( const Behavior<T2>& x ) {
  return lift(&implicit_cast<T1,T2>)(x) ;
}

// Lift details

template<class Tout, class Fn>
class Lift0Module : public BehaviorModule<Tout> {
 protected:
  Fn fn ;
 public: 
  Lift0Module( const Fn& f ) : fn(f) {}
  void compute(void) { value = fn() ; }
};

template<class Tout, class Fn=Tout (*)(void)>
class Lift0Combinator : public BehaviorCombinator<Tout> {
 protected:
  Fn fn ;
 public:
  Lift0Combinator( const Fn& f ) : fn(f) {}
  BehaviorRef<Tout> operator() (void) const 
    { return new Lift0Module<Tout,Fn>(fn); }
  operator BehaviorRef<Tout> (void) const { return (*this)() ; }

  Fun0<BehaviorRef<Tout> > fun(void) const
    { return Fun0<BehaviorRef<Tout> >
        (new DirectFun0<Lift0Combinator<Tout,Fn>, BehaviorRef<Tout> >(*this)) ; }
};

template<class Tout, class Tin1, class Fn>
class Lift1Module : public BehaviorModule<Tout> {
 protected:
  Fn fn ;
  BehaviorBase<Tin1>* p1 ;
 public: 
  Lift1Module( const Fn& f, BehaviorBase<Tin1>* x1 ) : fn(f), p1(x1)  
    { attach(p1); }
  void compute(void) { value = fn( p1->get() ) ; }
};

template<class Tout, class Tin1, class Fn=Tout (*)(Tin1)>
class Lift1Combinator : public BehaviorCombinator<Tout> {
 protected:
  Fn fn ;
 public:
  typedef BehaviorRef<Tin1> Arg1 ;
  Lift1Combinator( const Fn& f ) : fn(f) {}
  BehaviorRef<Tout> operator() ( BehaviorRef<Tin1> x1 ) const 
    { return new Lift1Module<Tout,Tin1,Fn>( fn, x1.get() ); }

  Fun1<BehaviorRef<Tout>, BehaviorRef<Tin1> > fun(void) const
    { return Fun1<BehaviorRef<Tout>, BehaviorRef<Tin1> >
        (new DirectFun1<Lift1Combinator<Tout,Tin1,Fn>, BehaviorRef<Tout>,
	 BehaviorRef<Tin1> >(*this)) ; }

  Fun0<BehaviorRef<Tout> > bind1( BehaviorRef<Tin1> x1 ) const
   { return fun().bind1(x1); }
};

template<class Tout, class Tin1, class Tin2, class Fn>
class Lift2Module : public BehaviorModule<Tout> {
 protected:
  Fn fn ;
  BehaviorBase<Tin1>* p1 ;
  BehaviorBase<Tin2>* p2 ;
 public: 
  Lift2Module( const Fn& f, BehaviorBase<Tin1>* x1, BehaviorBase<Tin2>* x2 ) :
    fn(f), p1(x1), p2(x2) { attach(p1); attach(p2); }
  void compute(void) { value = fn( p1->get(), p2->get() ) ; }
};

template<class Tout, class Tin1, class Tin2, class Fn=Tout (*)(Tin1,Tin2)>
class Lift2Combinator : public BehaviorCombinator<Tout> {
 protected:
  Fn fn ;
 public:
  typedef BehaviorRef<Tin1> Arg1 ;
  typedef BehaviorRef<Tin2> Arg2 ;
  Lift2Combinator( const Fn& f ) : fn(f) {}
  BehaviorRef<Tout> operator() ( BehaviorRef<Tin1> x1, 
				 BehaviorRef<Tin2> x2 ) const 
    { return new Lift2Module<Tout,Tin1,Tin2,Fn>( fn, x1.get(), x2.get() );}
  BehaviorRef<Tout> operator() ( std::pair<BehaviorRef<Tin1>, 
					 BehaviorRef<Tin2> > x ) const 
    { return new Lift2Module<Tout,Tin1,Tin2,Fn>( fn, x.first.get(), 
						 x.second.get() );}

  Fun2<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2> > 
  fun(void) const
    { return Fun2<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2> >
        (new DirectFun2<Lift2Combinator<Tout,Tin1,Tin2,Fn>, BehaviorRef<Tout>,
	 BehaviorRef<Tin1>, BehaviorRef<Tin2> >(*this)) ; }

  Fun1<BehaviorRef<Tout>, BehaviorRef<Tin2> > 
  bind1( BehaviorRef<Tin1> x1 ) const
   { return fun().bind1(x1); }
  Fun1<BehaviorRef<Tout>, BehaviorRef<Tin1> > 
  bind2( BehaviorRef<Tin2> x2 ) const
   { return fun().bind2(x2); }

  Fun1<BehaviorRef<Tout>, BehaviorRef<Tin2> > 
  operator () ( BehaviorRef<Tin1> x1 ) const
   { return bind1(x1); }
};

template<class Tout, class Tin1, class Tin2, class Tin3, class Fn>
class Lift3Module : public BehaviorModule<Tout> {
 protected:
  Fn fn ;
  BehaviorBase<Tin1>* p1 ;
  BehaviorBase<Tin2>* p2 ;
  BehaviorBase<Tin3>* p3 ;
 public: 
  Lift3Module( const Fn& f, BehaviorBase<Tin1>* x1, 
	       BehaviorBase<Tin2>* x2, BehaviorBase<Tin3>* x3 ) :
    fn(f), p1(x1), p2(x2), p3(x3) { attach(p1); attach(p2); attach(p3); }
  void compute(void) { value = fn( p1->get(), p2->get(), p3->get() ) ; }
};

template<class Tout, class Tin1, class Tin2, class Tin3, 
  class Fn=Tout (*)(Tin1,Tin2,Tin3)>
class Lift3Combinator : public BehaviorCombinator<Tout> {
 protected:
  Fn fn ;
 public:
  typedef BehaviorRef<Tin1> Arg1 ;
  typedef BehaviorRef<Tin2> Arg2 ;
  typedef BehaviorRef<Tin3> Arg3 ;
  Lift3Combinator( const Fn& f ) : fn(f) {}
  BehaviorRef<Tout> operator() ( BehaviorRef<Tin1> x1, 
				 BehaviorRef<Tin2> x2,
				 BehaviorRef<Tin3> x3 ) const 
    { return new Lift3Module<Tout,Tin1,Tin2,Tin3,Fn>( fn, x1.get(), 
						      x2.get(), x3.get() );}
  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>, 
       BehaviorRef<Tin3> > 
  fun(void) const
    { return Fun3<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>,
                  BehaviorRef<Tin3> >
        (new DirectFun3<Lift3Combinator<Tout,Tin1,Tin2,Tin3,Fn>, 
         BehaviorRef<Tout>, BehaviorRef<Tin1>, 
         BehaviorRef<Tin2>, BehaviorRef<Tin3> >(*this)) ; }

  Fun2<BehaviorRef<Tout>, BehaviorRef<Tin2>, BehaviorRef<Tin3> > 
  bind1( BehaviorRef<Tin1> x1 ) const
   { return fun().bind1(x1); }
  Fun2<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin3> > 
  bind2( BehaviorRef<Tin2> x2 ) const
   { return fun().bind2(x2); }
  Fun2<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2> > 
  bind3( BehaviorRef<Tin3> x3 ) const
   { return fun().bind3(x3); }

  Fun2<BehaviorRef<Tout>, BehaviorRef<Tin2>, BehaviorRef<Tin3> > 
  operator () ( BehaviorRef<Tin1> x1 ) const
   { return bind1(x1); }
};

template<class Tout, class Tin1, class Tin2, class Tin3, class Tin4, class Fn>
class Lift4Module : public BehaviorModule<Tout> {
 protected:
  Fn fn ;
  BehaviorBase<Tin1>* p1 ;
  BehaviorBase<Tin2>* p2 ;
  BehaviorBase<Tin3>* p3 ;
  BehaviorBase<Tin4>* p4 ;
 public: 
  Lift4Module( const Fn& f, BehaviorBase<Tin1>* x1, BehaviorBase<Tin2>* x2, 
	       BehaviorBase<Tin3>* x3, BehaviorBase<Tin4>* x4 ) :
    fn(f), p1(x1), p2(x2), p3(x3), p4(x4) 
    { attach(p1); attach(p2); attach(p3); attach(p4); }
  void compute(void) 
    { value = fn( p1->get(), p2->get(), p3->get(), p4->get() ) ; }
};

 template<class Tout, class Tin1, class Tin2, class Tin3, class Tin4,
  class Fn=Tout (*)(Tin1,Tin2,Tin3,Tin4)>
class Lift4Combinator : public BehaviorCombinator<Tout> {
 protected:
  Fn fn ;
 public:
  typedef BehaviorRef<Tin1> Arg1 ;
  typedef BehaviorRef<Tin2> Arg2 ;
  typedef BehaviorRef<Tin3> Arg3 ;
  typedef BehaviorRef<Tin4> Arg4 ;
  Lift4Combinator( const Fn& f ) : fn(f) {}
  BehaviorRef<Tout> operator() ( BehaviorRef<Tin1> x1, 
				 BehaviorRef<Tin2> x2,
				 BehaviorRef<Tin3> x3,
				 BehaviorRef<Tin4> x4 ) const 
    { return new Lift4Module<Tout,Tin1,Tin2,Tin3,Tin4,Fn>
      ( fn, x1.get(), x2.get(), x3.get(), x4.get() );}

  Fun4<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>, 
       BehaviorRef<Tin3>, BehaviorRef<Tin4> > 
  fun(void) const
    { return Fun4<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>,
                  BehaviorRef<Tin3>, BehaviorRef<Tin4> >
        (new DirectFun4<Lift4Combinator<Tout,Tin1,Tin2,Tin3,Tin4,Fn>, 
         BehaviorRef<Tout>, BehaviorRef<Tin1>, 
         BehaviorRef<Tin2>, BehaviorRef<Tin3>, BehaviorRef<Tin4> >(*this)) ; }

  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin2>, BehaviorRef<Tin3>, 
       BehaviorRef<Tin4> > 
  bind1( BehaviorRef<Tin1> x1 ) const
   { return fun().bind1(x1); }
  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin3>,
       BehaviorRef<Tin4> > 
  bind2( BehaviorRef<Tin2> x2 ) const
   { return fun().bind2(x2); }
  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>,
       BehaviorRef<Tin4> > 
  bind3( BehaviorRef<Tin3> x3 ) const
   { return fun().bind3(x3); }
  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin1>, BehaviorRef<Tin2>,
       BehaviorRef<Tin3> > 
  bind4( BehaviorRef<Tin4> x4 ) const
   { return fun().bind4(x4); }

  Fun3<BehaviorRef<Tout>, BehaviorRef<Tin2>, BehaviorRef<Tin3>, 
       BehaviorRef<Tin4> > 
  operator () ( BehaviorRef<Tin1> x1 ) const
   { return bind1(x1); }
};


// Type lookup table for <<=

template<class Tout, class Tin>
struct CombinatorTraits<Tout (*)(Tin)> {
  typedef Tout Result ;
};

template<class Tout, class Tin>
struct CombinatorTraits<Tout (*)(const Tin&)> {
  typedef Tout Result ;
};

// Type lookup table for lifting

template<class Tout> 
struct LiftTraits<Fun0<Tout> > {
  typedef Fun0<Tout> Arg ;
  typedef Arg Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout> 
struct LiftTraits<Tout (*)(void)> {
  typedef Tout (*Arg)(void) ;
  typedef Arg Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout, class Tin1> 
struct LiftTraits<Fun1<Tout,Tin1> > {
  typedef Fun1<Tout,Tin1> Arg ;
  typedef Arg Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1> 
struct LiftTraits<Tout (*)(Tin1)> {
  typedef Tout (*Arg)(Tin1) ;
  typedef Arg Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1> 
struct LiftTraits<Tout (*)(const Tin1&)> {
  typedef Tout (*Arg)(const Tin1&) ;
  typedef Arg Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2> 
struct LiftTraits<Fun2<Tout,Tin1,Tin2> > {
  typedef Fun2<Tout,Tin1,Tin2> Arg ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2> 
struct LiftTraits<Tout (*)(Tin1,Tin2)> {
  typedef Tout (*Arg)(Tin1,Tin2) ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2> 
struct LiftTraits<Tout (*)(Tin1,const Tin2&)> {
  typedef Tout (*Arg)(Tin1,const Tin2&) ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2> 
struct LiftTraits<Tout (*)(const Tin1&,Tin2)> {
  typedef Tout (*Arg)(const Tin1&,Tin2) ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2> 
struct LiftTraits<Tout (*)(const Tin1&,const Tin2&)> {
  typedef Tout (*Arg)(const Tin1&,const Tin2&) ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tclass>
struct LiftTraits<Tout (Tclass::*)(void)> {
  typedef Tout (Tclass::*Arg)(void) ;
  typedef MemberFun1<Arg,Tout,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tclass,Agent> Result ;
};

template<class Tout, class Tclass>
struct LiftTraits<Tout (Tclass::*)(void) const> {
  typedef Tout (Tclass::*Arg)(void) const ;
  typedef MemberFun1<Arg,Tout,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tclass,Agent> Result ;
};

template<class Tout, class Tclass, class Tin1>
struct LiftTraits<Tout (Tclass::*)(Tin1)> {
  typedef Tout (Tclass::*Arg)(Tin1) ;
  typedef MemberFun2a<Arg,Tout,Tclass,Tin1> Agent ;
  typedef Lift2Combinator<Tout,Tclass,Tin1,Agent> Result ;
};

template<class Tout, class Tclass, class Tin1>
struct LiftTraits<Tout (Tclass::*)(const Tin1&)> {
  typedef Tout (Tclass::*Arg)(const Tin1&) ;
  typedef MemberFun2a<Arg,Tout,Tclass,Tin1> Agent ;
  typedef Lift2Combinator<Tout,Tclass,Tin1,Agent> Result ;
};

template<class Tout, class Tclass, class Tin1>
struct LiftTraits<Tout (Tclass::*)(Tin1) const> {
  typedef Tout (Tclass::*Arg)(Tin1) const ;
  typedef MemberFun2a<Arg,Tout,Tclass,Tin1> Agent ;
  typedef Lift2Combinator<Tout,Tclass,Tin1,Agent> Result ;
};

template<class Tout, class Tclass, class Tin1>
struct LiftTraits<Tout (Tclass::*)(const Tin1&) const> {
  typedef Tout (Tclass::*Arg)(const Tin1&) const ;
  typedef MemberFun2a<Arg,Tout,Tclass,Tin1> Agent ;
  typedef Lift2Combinator<Tout,Tclass,Tin1,Agent> Result ;
};

template<class Tout, class Tclass>
struct LiftTraits<Tout Tclass::*> {
  typedef Tout Tclass::*Arg ; 
  typedef DataMemFun<Arg,Tout,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tclass,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tin3> 
struct LiftTraits<Fun3<Tout,Tin1,Tin2,Tin3> > {
  typedef Fun3<Tout,Tin1,Tin2,Tin3> Arg ;
  typedef Arg Agent ;
  typedef Lift3Combinator<Tout,Tin1,Tin2,Tin3,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tin3> 
struct LiftTraits<Tout (*)(Tin1,Tin2,Tin3) > {
  typedef Tout (*Arg)(Tin1,Tin2,Tin3) ;
  typedef Arg Agent ;
  typedef Lift3Combinator<Tout,Tin1,Tin2,Tin3,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tin3> 
struct LiftTraits<Tout (*)(const Tin1&,const Tin2&,const Tin3&) > {
  typedef Tout (*Arg)(const Tin1&,const Tin2&,const Tin3&) ;
  typedef Arg Agent ;
  typedef Lift3Combinator<Tout,Tin1,Tin2,Tin3,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tin3, class Tin4> 
struct LiftTraits<Fun4<Tout,Tin1,Tin2,Tin3,Tin4> > {
  typedef Fun4<Tout,Tin1,Tin2,Tin3,Tin4> Arg ;
  typedef Arg Agent ;
  typedef Lift4Combinator<Tout,Tin1,Tin2,Tin3,Tin4,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tin3, class Tin4> 
struct LiftTraits<Tout (*)(Tin1,Tin2,Tin3,Tin4) > {
  typedef Tout (*Arg)(Tin1,Tin2,Tin3,Tin4) ;
  typedef Arg Agent ;
  typedef Lift4Combinator<Tout,Tin1,Tin2,Tin3,Tin4,Agent> Result ;
};

template<class Tout, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(void),Tclass> {
  typedef Tout (Tclass::*Arg)(void) ;
  typedef ClassFun0<Arg,Tout,Tclass> Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(void) const,Tclass> {
  typedef Tout (Tclass::*Arg)(void) const ;
  typedef ClassFun0<Arg,Tout,Tclass> Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(void),Tclass *> {
  typedef Tout (Tclass::*Arg)(void) ;
  typedef ClassPtrFun0<Arg,Tout,Tclass> Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(void) const,Tclass *> {
  typedef Tout (Tclass::*Arg)(void) const ;
  typedef ClassPtrFun0<Arg,Tout,Tclass> Agent ;
  typedef Lift0Combinator<Tout,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(Tin1),Tclass> {
  typedef Tout (Tclass::*Arg)(Tin1) ;
  typedef ClassFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(const Tin1&),Tclass> {
  typedef Tout (Tclass::*Arg)(const Tin1&) ;
  typedef ClassFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(Tin1) const,Tclass> {
  typedef Tout (Tclass::*Arg)(Tin1) const;
  typedef ClassFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(const Tin1&) const,Tclass> {
  typedef Tout (Tclass::*Arg)(const Tin1&) const ;
  typedef ClassFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(Tin1),Tclass*> {
  typedef Tout (Tclass::*Arg)(Tin1) ;
  typedef ClassPtrFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(const Tin1&),Tclass*> {
  typedef Tout (Tclass::*Arg)(const Tin1&) ;
  typedef ClassPtrFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(Tin1) const,Tclass*> {
  typedef Tout (Tclass::*Arg)(Tin1) const;
  typedef ClassPtrFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tclass> 
struct LiftTraits2<Tout (Tclass::*)(const Tin1&) const,Tclass*> {
  typedef Tout (Tclass::*Arg)(const Tin1&) const ;
  typedef ClassPtrFun1<Arg,Tout,Tin1,Tclass> Agent ;
  typedef Lift1Combinator<Tout,Tin1,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tclass>
struct LiftTraits2<Tout (Tclass::*)(Tin1,Tin2),Tclass> {
  typedef Tout (Tclass::*Arg)(Tin1,Tin2) ;
  typedef ClassFun2<Arg,Tout,Tin1,Tin2,Tclass> Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tclass>
struct LiftTraits2<Tout (Tclass::*)(const Tin1&,const Tin2&),Tclass> {
  typedef Tout (Tclass::*Arg)(const Tin1&,const Tin2&) ;
  typedef ClassFun2<Arg,Tout,Tin1,Tin2,Tclass> Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tclass>
struct LiftTraits2<Tout (Tclass::*)(Tin1,Tin2),Tclass*> {
  typedef Tout (Tclass::*Arg)(Tin1,Tin2) ;
  typedef ClassPtrFun2<Arg,Tout,Tin1,Tin2,Tclass> Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};

template<class Tout, class Tin1, class Tin2, class Tclass>
struct LiftTraits2<Tout (Tclass::*)(const Tin1&,const Tin2&),Tclass*> {
  typedef Tout (Tclass::*Arg)(const Tin1&,const Tin2&) ;
  typedef ClassPtrFun2<Arg,Tout,Tin1,Tin2,Tclass> Agent ;
  typedef Lift2Combinator<Tout,Tin1,Tin2,Agent> Result ;
};


// Operator lifts

# define FRP_LiftUnaryOperator( op, name, opname ) \
template<class T> \
inline T operator_##name( T x ) { return op x ; } \
\
template<class T> \
inline BehaviorRef<T> operator op ( BehaviorRef<T> x ){\
  return lift(&operator_##name<T>)(x); \
} \
template<class T> \
struct ExpType1Traits<opname,BehaviorRef<T> > { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T> \
inline BehaviorRef<T> operator op ( const Behavior<T>& x ){\
  return lift(&operator_##name<T>)(x); \
} \
template<class T> \
struct ExpType1Traits<opname,Behavior<T> > { \
  typedef BehaviorRef<T> Result ; \
}; \


# define FRP_LiftBinaryOperator( op1, op2, name, opname, T ) \
template<class T1, class T2> \
inline T operator_##name( T1 x1, T2 x2 ) { return x1 op1 x2 op2; } \
\
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( BehaviorRef<T1> x1, BehaviorRef<T2> x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,BehaviorRef<T1>,BehaviorRef<T2> > { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( const Behavior<T1>& x1, BehaviorRef<T2> x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,Behavior<T1>,BehaviorRef<T2> > { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( BehaviorRef<T1> x1, const Behavior<T2>& x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,BehaviorRef<T1>,Behavior<T2> > { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( const Behavior<T1>& x1, const Behavior<T2>& x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,Behavior<T1>,Behavior<T2> > { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( BehaviorRef<T1> x1, T2 x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,BehaviorRef<T1>,T2> { \
  typedef BehaviorRef<T> Result ; \
}; \
template<class T1, class T2> \
inline BehaviorRef<T> operator op1 op2 ( const Behavior<T1>& x1, T2 x2 ){\
  return lift(&operator_##name<T1,T2>)(x1,x2); \
} \
template<class T1, class T2> \
struct ExpType2Traits<opname,Behavior<T1>,T2> { \
  typedef BehaviorRef<T> Result ; \
}; \


FRP_LiftUnaryOperator( !, not, OpNot );
FRP_LiftUnaryOperator( -, negative, OpNegative );

FRP_LiftBinaryOperator( +,, plus, OpPlus, T1 );
FRP_LiftBinaryOperator( -,, minus, OpMinus, T1 );
FRP_LiftBinaryOperator( *,, multiplies, OpMultiplies, T1 );
FRP_LiftBinaryOperator( /,, divides, OpDivides, T1 );
FRP_LiftBinaryOperator( >,, greater, OpGreater, bool );
FRP_LiftBinaryOperator( >=,, greater_equal, OpGreaterEqual, bool );
FRP_LiftBinaryOperator( <,, less, OpLess, bool );
FRP_LiftBinaryOperator( <=,, less_equal, OpLessEqual, bool );
FRP_LiftBinaryOperator( ==,, equal, OpEqual, bool );
FRP_LiftBinaryOperator( !=,, not_equal, OpNotEqual, bool );
FRP_LiftBinaryOperator( &&,, and, OpAnd, T1 );
FRP_LiftBinaryOperator( ||,, or, OpOr, T1 );
FRP_LiftBinaryOperator( ^,, xor, OpXor, T1 );
FRP_LiftBinaryOperator( <<,, left_shift, OpLeftShift, T1 );
FRP_LiftBinaryOperator( >>,, right_shift, OpRightShift, T1 );
# ifndef FRP_COMMA
# define FRP_COMMA ,
# endif
FRP_LiftBinaryOperator( FRP_COMMA ,, comma, OpComma, T2 );

// To counter those silly definitions of <=, >= , etc. in g++-3/stl_relops.h
# define FRP_LiftRelationOperatorOn( op, name, type ) \
inline BehaviorRef<bool> \
operator op ( BehaviorRef<type> x1, BehaviorRef<type> x2 ){ \
  return lift(&operator_##name<type,type>)(x1,x2); \
} \

# define FRP_LiftRelationOperator( op, name ) \
FRP_LiftRelationOperatorOn( op, name, char ) \
FRP_LiftRelationOperatorOn( op, name, signed char ) \
FRP_LiftRelationOperatorOn( op, name, unsigned char ) \
FRP_LiftRelationOperatorOn( op, name, int ) \
FRP_LiftRelationOperatorOn( op, name, unsigned int ) \
FRP_LiftRelationOperatorOn( op, name, short ) \
FRP_LiftRelationOperatorOn( op, name, unsigned short ) \
FRP_LiftRelationOperatorOn( op, name, long ) \
FRP_LiftRelationOperatorOn( op, name, unsigned long ) \
FRP_LiftRelationOperatorOn( op, name, float ) \
FRP_LiftRelationOperatorOn( op, name, double ) \

FRP_LiftRelationOperator( >, greater );
FRP_LiftRelationOperator( >=, greater_equal );
FRP_LiftRelationOperator( <, less );
FRP_LiftRelationOperator( <=, less_equal );
FRP_LiftRelationOperator( ==, equal );
FRP_LiftRelationOperator( !=, not_equal );


// Function lifts

# define FRP_LiftUnaryFunction( name, fn, Tout, Tin1 ) \
inline BehaviorRef<Tout> name ( BehaviorRef<Tin1> x1 ) { \
  return lift(&fn)(x1); \
} \
inline BehaviorRef<Tout> name ( const Behavior<Tin1>& x1 ) { \
  return lift(&fn)(x1); \
} \

# define FRP_LiftBinaryFunction( name, fn, Tout, Tin1, Tin2 ) \
inline BehaviorRef<Tout> name ( BehaviorRef<Tin1> x1, BehaviorRef<Tin2> x2 ) {\
  return lift(&fn)(x1,x2); \
} \
inline BehaviorRef<Tout> name ( BehaviorRef<Tin1> x1, const Behavior<Tin2> x2& ) {\
  return lift(&fn)(x1,x2); \
} \
inline BehaviorRef<Tout> name ( const Behavior<Tin1>& x1, BehaviorRef<Tin2> x2 ) {\
  return lift(&fn)(x1,x2); \
} \
inline BehaviorRef<Tout> name ( const Behavior<Tin1>& x1, const Behavior<Tin2>& x2 ) {\
  return lift(&fn)(x1,x2); \
} \

FRP_LiftUnaryFunction( sin, std::sin, double, double );
FRP_LiftUnaryFunction( cos, std::cos, double, double );
FRP_LiftUnaryFunction( tan, std::tan, double, double );
FRP_LiftUnaryFunction( asin, std::asin, double, double );
FRP_LiftUnaryFunction( acos, std::acos, double, double );
FRP_LiftUnaryFunction( atan, std::atan, double, double );

// Other utilities

extern BehaviorRef<Time> timeB ;
extern BehaviorRef<Sample> sampleB ;

// numberic operations

template<class T>
BehaviorRef<T> differential( BehaviorRef<T> x ) {
  return x - delayB<T>(0)(x) ;
}
template<class T>
BehaviorRef<T> differential( const Behavior<T>& x ) {
  return differential((BehaviorRef<T>)x) ;
}

template<class T>
BehaviorRef<T> accumulative( BehaviorRef<T> x ) {
  Behavior<T> sum ;
  sum = x + delayB<T>(0)(sum) ;
  return sum ;
}
template<class T>
BehaviorRef<T> accumulative( const Behavior<T>& x ) {
  return accumulative((BehaviorRef<T>)x) ;
}

extern BehaviorRef<Time> difTimeB ;

// Euler integral

template<class T>
BehaviorRef<T> integral( BehaviorRef<T> x ) {
  return accumulative( x*difTimeB );
}
template<class T, class D>
inline BehaviorRef<T> integral( const Behavior<D>& x ) {
  return integral<T>((BehaviorRef<T>)x) ;
}

// Derivative, primitive though

template<class T>
BehaviorRef<T> derivative( BehaviorRef<T> x ) {
  return differential(x) / difTimeB ;
}
template<class T>
BehaviorRef<T> derivative( const Behavior<T>& x ) {
  return derivative((BehaviorRef<T>)x) ;
}

// ostream wrapper

template<class T>
class StreamWrapper {
 protected:
  T * stream ;
 public:
  StreamWrapper() : stream(0) {}
  StreamWrapper( T& s ) : stream(&s) {}
  template<class D>
  StreamWrapper<T> operator << ( const D& d ) 
    { (*stream) << d ; return *this ; }
  template<class D>
  StreamWrapper<T> operator >> ( D& d ) 
    { (*stream) >> d ; return *this ;}
};

template<class T>
BehaviorRef<StreamWrapper<T> > liftS( T& s ) {
  return StreamWrapper<T>(s);
}

} // namespace frp

# endif // _FRP_BEHAVIOR_H_
