# ifndef _FRP_EVENT_H_
# define _FRP_EVENT_H_

# include "Behavior.h"

namespace frp {

// Maybe class

template<class T>
class Maybe : public BasicCollectable {
 protected:
  T * value ;
  char data[sizeof(T)];
 public:
  void gc_marking() { try_gc_mark( value ); }
  bool hasValue(void) const { return value ; }
  T& getValue(void) { return *value ; }
  const T& getValue(void) const { return *value ; }
  operator bool (void) const { return hasValue(); }
  bool operator ! (void) const { return !hasValue(); }
  T& operator *(void) { return getValue(); }
  const T& operator *(void) const { return getValue(); }

  Maybe() : value(0) {}
  Maybe( const T& x ) : value(new((void*)data) T(x)) {}
  template<class D>
  Maybe( const D& x ) : value(new((void*)data) T(x)) {}
  
  Maybe( bool f, const T& x ) : value(f?new((void*)data) T(x):0) {}
  template<class D>
  Maybe( bool f, const D& x ) : value(f?new((void*)data) T(x):0) {}
  
  
  Maybe( const Maybe<T>& x ) : 
    value(x.hasValue()?new((void*)data) T(x.getValue()):0) {} 
  template<class D>
  Maybe( const Maybe<D>& x ) : 
    value(x.hasValue()?new((void*)data) T(x.getValue()):0) {} 
  
  ~Maybe() { if( value ) value->~T() ; }

  Maybe<T>& operator = ( const Maybe<T>& x )
    { 
      if( &x != this ) { 
	if( x.hasValue() ) {
	  if( value ) {
	    *value = x.getValue() ;
	  }else {
	    value = new((void*)data) T(x.getValue()) ;
	  }
	}else {
	  if( value ) {
	    value->~T() ;
	    value = 0 ;
	  }
	}
      }
      return *this ;
    }
  template<class D>
  Maybe<T>& operator = ( const Maybe<D>& x )
    {       
      if( x.hasValue() ) {
	if( value ) {
	  *value = x.getValue() ;
	}else {
	  value = new((void*)data) T(x.getValue()) ;
	}
      }else {
	if( value ) {
	  value->~T() ;
	    value = 0 ;
	}
      }
      return *this ;
    }
};

template<>
class Maybe<void> {
 protected:
  bool value ;
 public:
  bool hasValue(void) const { return value ; }
  void getValue(void) const {}
  operator bool (void) const { return hasValue(); }
  bool operator ! (void) const { return !hasValue(); }
  void operator * (void) const {}

  Maybe() : value(0) {}
  Maybe( bool f ) : value(f) {}

  template<class D>
  Maybe( const Maybe<D>& x ) : value(x.hasValue()) {} 

  template<class D>
  Maybe<void>& operator = ( const Maybe<D>& x )
    { value = x.hasValue() ; return *this ; }

  Maybe<void>& operator = ( bool x )
    { value = x ; return *this ; }
};

template<class T>
inline bool operator ==( const Maybe<T>& x, const Maybe<T>& y ) {
  return x ? ( y ? x.getValue()==y.getValue() : false ) 
           : ( y ? false : true ) ;
}
inline bool operator ==( const Maybe<void>& x, const Maybe<void>& y ) {
  return (bool) x == (bool) y ;
}

template<class T>
inline Maybe<T> operator ||( const Maybe<T>& x, const Maybe<T>& y ) {
  return x ? x : y ;
}

template<class T1, class T2>
inline Maybe<void> operator ||( const Maybe<T1>& x, const Maybe<T2>& x2 ) {
  return (bool)x || (bool)x2;
}

template<class T1, class T2>
inline Maybe<T1> operator &&( const Maybe<T1>& x, const Maybe<T2> y ) {
  return y ? x : Maybe<T1>() ;
}

// liftE

template<class T>
struct LiftMaybeTraits {
  typedef T Arg ;
  typedef Maybe<T> Result ;
};

template<class T>
inline typename LiftMaybeTraits<T>::Result liftMaybe( T f ) {
  return typename LiftMaybeTraits<T>::Result(f);
}

template<class T>
inline typename LiftTraits<typename LiftMaybeTraits<T>::Result>::Result 
liftE( T f ) {
  return lift(liftMaybe(f));
}

template<class Tout, class Fn>
class LiftMaybe0 {
 protected:
  Fn fn ;
 public:
  LiftMaybe0( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( Maybe<void> x )
    { if(x) return Maybe<Tout>(fn()) ; else return Maybe<Tout>() ; }
};
template<class Tout, class Fn> 
struct LiftTraits<LiftMaybe0<Tout,Fn> > {
  typedef LiftMaybe0<Tout,Fn> Arg ;
  typedef Arg Agent ;
  typedef Lift1Combinator<Maybe<Tout>,Maybe<void>,Arg> Result ;
};

template<class Tout, class Tin1, class Fn>
class LiftMaybe1 {
 protected:
  Fn fn ;
 public:
  LiftMaybe1( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<Tin1>& x1 )
    { if( x1 ) return Maybe<Tout>(fn(*x1)) ; else return Maybe<Tout>() ; }
};
template<class Tout, class Fn>
class LiftMaybe1<Tout,void,Fn> {
 protected:
  Fn fn ;
 public:
  LiftMaybe1( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<void>& x1 ) 
    { if( x1 ) return Maybe<Tout>(fn()) ; else return Maybe<Tout>() ; }
};
template<class Tout, class Tin1, class Fn> 
struct LiftTraits<LiftMaybe1<Tout,Tin1,Fn> > {
  typedef LiftMaybe1<Tout,Tin1,Fn> Arg ;
  typedef Arg Agent ;
  typedef Lift1Combinator<Maybe<Tout>,Maybe<Tin1>,Arg> Result ;
};

template<class Tout, class Tin1, class Tin2, class Fn>
class LiftMaybe2 {
 protected:
  Fn fn ;
 public:
  LiftMaybe2( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<Tin1>& x1, const Maybe<Tin2>& x2 ) 
    { if( x1&&x2 ) return Maybe<Tout>(fn(*x1,*x2)); return Maybe<Tout>(); }
};
template<class Tout, class Tin1, class Fn>
class LiftMaybe2<Tout,Tin1,void,Fn> {
 protected:
  Fn fn ;
 public:
  LiftMaybe2( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<Tin1>& x1, const Maybe<void>& x2 )
    { if( x1&&x2 ) return Maybe<Tout>(fn(*x1)); return Maybe<Tout>(); }
};
template<class Tout, class Tin2, class Fn>
class LiftMaybe2<Tout,void,Tin2,Fn> {
 protected:
  Fn fn ;
 public:
  LiftMaybe2( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<void>& x1, const Maybe<Tin2>& x2 )
    { if( x1&&x2 ) return Maybe<Tout>(fn(*x2)); return Maybe<Tout>(); }
};
template<class Tout, class Fn>
class LiftMaybe2<Tout,void,void,Fn> {
 protected:
  Fn fn ;
 public:
  LiftMaybe2( const Fn& f ) : fn(f) {}
  Maybe<Tout> operator () ( const Maybe<void>& x1, const Maybe<void>& x2 )
    { if( x1&&x2 ) return Maybe<Tout>(fn()); return Maybe<Tout>(); }
};
template<class Tout, class Tin1, class Tin2, class Fn> 
struct LiftTraits<LiftMaybe2<Tout,Tin1,Tin2,Fn> > {
  typedef LiftMaybe2<Tout,Tin1,Tin2,Fn> Arg ;
  typedef Arg Agent ;
  typedef Lift2Combinator<Maybe<Tout>,Maybe<Tin1>,Maybe<Tin2>,Arg> Result ;
};

template<class Tout>
struct LiftMaybeTraits<Fun0<Tout> > {
  typedef Fun0<Tout> Arg ;
  typedef LiftMaybe0<Tout,Arg> Result ;
};
template<class Tout>
struct LiftMaybeTraits<Tout (*)(void)> {
  typedef Tout (*Arg)(void) ;
  typedef LiftMaybe0<Tout,Arg> Result ;
};
template<class Tout, class Tin1>
struct LiftMaybeTraits<Fun1<Tout,Tin1> > {
  typedef Fun1<Tout,Tin1> Arg ;
  typedef LiftMaybe1<Tout,Tin1,Arg> Result ;
};
template<class Tout, class Tin1>
struct LiftMaybeTraits<Tout (*)(Tin1)> {
  typedef Tout (*Arg)(Tin1) ;
  typedef LiftMaybe1<Tout,Tin1,Arg> Result ;
};
template<class Tout, class Tin1>
struct LiftMaybeTraits<Tout (*)(const Tin1&)> {
  typedef Tout (*Arg)(const Tin1&) ;
  typedef LiftMaybe1<Tout,Tin1,Arg> Result ;
};
template<class Tout, class Tin1, class Tin2>
struct LiftMaybeTraits<Fun2<Tout,Tin1,Tin2> > {
  typedef Fun2<Tout,Tin1,Tin2> Arg ;
  typedef LiftMaybe2<Tout,Tin1,Tin2,Arg> Result ;
};
template<class Tout, class Tin1, class Tin2>
struct LiftMaybeTraits<Tout (*)(Tin1,Tin2)> {
  typedef Tout (*Arg)(Tin1,Tin2) ;
  typedef LiftMaybe2<Tout,Tin1,Tin2,Arg> Result ;
};
template<class Tout, class Tin1, class Tin2>
struct LiftMaybeTraits<Tout (*)(const Tin1&,const Tin2&)> {
  typedef Tout (*Arg)(const Tin1&,const Tin2&) ;
  typedef LiftMaybe2<Tout,Tin1,Tin2,Arg> Result ;
};


// Event<T> -- just Behavior<Maybe<T> >

template<class T>
class Event : public Behavior<Maybe<T> > {
 public:
  Event() {}
  Event( const Maybe<T>& init ) : Behavior<Maybe<T> >(init) {}

  Event<T>& operator = ( BehaviorRef<Maybe<T> > x ) 
    { *ref = x ; return *this ; }

  virtual bool hasValue(void) const { return get().hasValue() ; }
  virtual T getValue(void) const { return get().getValue() ; }
};

// operator || for event merge

template<class T>
inline Maybe<T> FRP_maybe_or1( const Maybe<T>& x, const Maybe<T>& y ) {
  return x || y ;
}
template<class T1, class T2>
inline Maybe<void> FRP_maybe_or2( const Maybe<T1>& x, const Maybe<T2>& y ) {
  return x || y ;
}

template<class T>
inline BehaviorRef<Maybe<T> > 
operator || ( BehaviorRef<Maybe<T> > e1, BehaviorRef<Maybe<T> > e2 ) {
  return liftB(&FRP_maybe_or1<T>)( e1, e2 );
}
template<class T>
inline BehaviorRef<Maybe<T> > 
operator || ( const Event<T>& e1, BehaviorRef<Maybe<T> > e2 ) {
  return e1.getRef() || e2 ;
}
template<class T>
inline BehaviorRef<Maybe<T> > 
operator || ( BehaviorRef<Maybe<T> > e1, const Event<T>& e2 ) {
  return e1 || e2.getRef() ;
}
template<class T>
inline BehaviorRef<Maybe<T> > 
operator || ( const Event<T>& e1, const Event<T>& e2 ) {
  return e1.getRef() || e2.getRef() ;
}

template<class T1, class T2>
inline BehaviorRef<Maybe<void> > 
operator || ( BehaviorRef<Maybe<T1> > e1, BehaviorRef<Maybe<T2> > e2 ) {
  return liftB(&FRP_maybe_or2<T1,T2>)( e1, e2 );
}
template<class T1, class T2>
inline BehaviorRef<Maybe<void> > 
operator || ( const Event<T1>& e1, BehaviorRef<Maybe<T2> > e2 ) {
  return e1.getRef() || e2 ;
}
template<class T1, class T2>
inline BehaviorRef<Maybe<void> > 
operator || ( BehaviorRef<Maybe<T1> > e1, const Event<T1>& e2 ) {
  return e1 || e2.getRef() ;
}
template<class T1, class T2>
inline BehaviorRef<Maybe<void> > 
operator || ( const Event<T1>& e1, const Event<T2>& e2 ) {
  return e1.getRef() || e2.getRef() ;
}

// whileE and whenE

inline BehaviorRef<Maybe<void> > whileE( BehaviorRef<bool> x ) {
  return cast<Maybe<void>,bool>(x);
}

template<class Tout, class Tin>
inline typename LiftTraits<Fun1<Maybe<Tout>,Tin> >::Result 
whileByE( Fun1<Maybe<Tout>,Tin> f ) {
  return lift(f);
}
template<class Tout, class Tin>
inline typename LiftTraits<Maybe<Tout> (*)(Tin)>::Result 
whileByE( Maybe<Tout> (*f)(Tin) ) {
  return lift(f);
}

template<class T>
inline BehaviorRef<Maybe<T> > constE( const T& init ) {
  return constB<Maybe<T> >( Maybe<T>(init) );
}

template<class T>
inline BehaviorRef<Maybe<T> > neverE(void) {
  return constB<Maybe<T> >( Maybe<T>() );
}

inline bool operator_not_and( bool x, bool y ) {
  return ! x && y ;
}

inline BehaviorRef<Maybe<void> > whenE( BehaviorRef<bool> x ) {
  return cast<Maybe<void>,bool>(lift(operator_not_and)( delayB(false)(x), x ));
}

template<class T>
inline DelayCombinator<Maybe<T> > delayE() {
  return delayB<Maybe<T> >( Maybe<T>() );
}
template<class T>
inline DelayCombinator<Maybe<T> > delayE( const T& x ) {
  return delayB<Maybe<T> >( Maybe<T>(x) );
}

// splitE

template<class T1, class T2>
BehaviorRef<Maybe<T1> > splitE( BehaviorRef<Maybe<T1> > x, 
				BehaviorRef<Maybe<T2> > y ) {
  return x && !y ;
}
template<class T1, class T2>
BehaviorRef<Maybe<T1> > splitE( const Behavior<Maybe<T1> >& x, 
				BehaviorRef<Maybe<T2> > y ) {
  return splitE( x.getRef(), y );
}
template<class T1, class T2>
BehaviorRef<Maybe<T1> > splitE( BehaviorRef<Maybe<T1> > x, 
				const Behavior<Maybe<T2> >& y ) {
  return splitE( x, y.getRef() );
}
template<class T1, class T2>
BehaviorRef<Maybe<T1> > splitE( const Behavior<Maybe<T1> >& x, 
				const Behavior<Maybe<T2> >& y ) {
  return splitE( x.getRef(), y.getRef() );
}

// castE

template<class T1, class T2>
inline BehaviorRef<Maybe<T1> > castE( BehaviorRef<Maybe<T2> > x ) {
  return liftE(&implicit_cast<T1,T2>)(x) ;
}
template<class T1, class T2>
inline BehaviorRef<Maybe<T1> > castE( const Event<T2>& x ) {
  return liftE(&implicit_cast<T1,T2>)(x) ;
}

// prevE. NO recursive!

template<class T>
class PrevModule : public BehaviorModule<Maybe<T> > {
 protected:
  BehaviorBase<Maybe<T> >* p ;
  Maybe<T> prefetch ;
 public:
  PrevModule( BehaviorBase<Maybe<T> >* x ) : p(x) { attach(p); }
  void gc_more_marking() { try_gc_mark( &prefetch ); }
  void compute(void) { 
    if( p->get() ) {
      value = prefetch ;
      prefetch = p->get() ;
    }else {
      value = p->get() ;
    }
  }
};

template<class T>
inline BehaviorRef<Maybe<T> > prevE( BehaviorRef<Maybe<T> > x ) {
  return new PrevModule<T>(x.get());
}
template<class T>
inline BehaviorRef<Maybe<T> > prevE( const Event<T>& x ) {
  return prevE<T>(x.getRef());
}

// filterE

template<class F, class R>
class FilterFun : public Functoid1<Maybe<R>,Maybe<R> > {
 protected:
  F f ;
 public:
  FilterFun( const F& fn ) : f(fn) {}
  virtual Maybe<R> operator() ( const Maybe<R>& a ) 
    { return (bool)a && f(a.getValue()) ? a : Maybe<R>() ; }
};
template<class Tout, class Fn>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tout> > >::Result 
filterE( Fn f ) {
  return lift( Fun1<Maybe<Tout>,Maybe<Tout> >( new FilterFun<Fn,Tout>(f) ) );
}
template<class Tout>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tout> > >::Result 
filterE( Fun1<bool,Tout> f ) {
  return filterE<Tout,Fun1<bool,Tout> >(f);
}
template<class Tout>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tout> > >::Result 
filterE( bool (*f)(Tout)  ) {
  return filterE<Tout,bool (*)(Tout)>(f);
}

template<class F, class R, class A>
class FilterByFun : public Functoid1<Maybe<R>,Maybe<A> > {
 protected:
  F f ;
 public:
  FilterByFun( const F& fn ) : f(fn) {}
  virtual Maybe<R> operator() ( const Maybe<A>& a ) 
    { return a ? f( a.getValue() ) : Maybe<R>() ; }
};
template<class Tout, class Tin, class Fn>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tin> > >::Result 
filterByE( Fn f ) {
  return lift( Fun1<Maybe<Tout>,Maybe<Tin> >
	       ( new FilterByFun<Fn,Tout,Tin>(f) ) );
}
template<class Tout, class Tin>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tin> > >::Result 
filterByE( Fun1<Maybe<Tout>,Tin> f ) {
  return filterByE<Tout,Tin,Fun1<Maybe<Tout>,Tin> >(f) ;
}
template<class Tout, class Tin>
inline LiftTraits<Fun1<Maybe<Tout>,Maybe<Tin> > >::Result 
filterByE( Maybe<Tout> (*f)(Tin) ) {
  return filterByE<Tout,Tin,Maybe<Tout> (*)(Tin) >(f) ;
}

// onceE

template<class T>
class OnceModule : public BehaviorModule<Maybe<T> > {
 protected:
  BehaviorBase<Maybe<T> >* p ;
 public:
  OnceModule( BehaviorBase<Maybe<T> >* x ) : p(x) { attach(p); }
  void compute(void) {
    if( !value && (value = p->get()) ) {
      detach(p);
    }
  }
};

template<class T>
inline BehaviorRef<Maybe<T> > onceE( BehaviorRef<Maybe<T> > x ) {
  return new OnceModule<T>(x.get());
}
template<class T>
inline BehaviorRef<Maybe<T> > onceE( const Event<T>& x ) {
  return onceE<T>(x.getRef());
}

// stepB

template<class T>
class StepModule : public BehaviorModule<T> {
 protected:
  BehaviorBase<Maybe<T> >* p ;
 public:
  StepModule( const T& init, BehaviorBase<Maybe<T> >* x ) : 
    BehaviorModule<T>(init), p(x) { attach(p); }
  void compute(void) { 
    if( p->get() ) {
      value = p->get().getValue();
    }
  }
};

template<class T>
class StepCombinator : public BehaviorCombinator<T> {
 protected:
  const T& first ;
 public:
  StepCombinator( const T& init ) : first(init) {}
  BehaviorRef<T> operator() ( BehaviorRef<Maybe<T> > x ) const
    { return new StepModule<T>( first, x.get() ); }
  BehaviorRef<T> operator() ( const Event<T>& x ) const
    { return (*this)(x.getRef()); }
};

template<class T>
inline StepCombinator<T> stepB( const T& x ) {
  return StepCombinator<T>(x) ;
}

// snapshotE_

template<class Tout, class Tin>
class SnapshotModule : public BehaviorModule<Maybe<Tout> > {
 protected:
  BehaviorBase<Maybe<Tin> >* p ;
  BehaviorBase<Tout>* t ;
 public:
  SnapshotModule( BehaviorBase<Maybe<Tin> >* x, BehaviorBase<Tout>* y ) : 
    p(x), t(y) { attach(p) ; attach(t) ; }
  void compute(void) 
    { value = p->get() ? Maybe<Tout>(t->get()) : Maybe<Tout>() ; }
};

template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
snapshotE( BehaviorRef<Maybe<Tin> > x, BehaviorRef<Tout> y ) {
  return new SnapshotModule<Tout,Tin>(x.get(),y.get()) ;
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
snapshotE( BehaviorRef<Maybe<Tin> > x, const Behavior<Tout>& y ) {
  return snapshotE(x,y.getRef()) ;
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
snapshotE( const Event<Tin>& x, BehaviorRef<Tout> y ) {
  return snapshotE(x.getRef(),y) ;
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
snapshotE( const Event<Tin>& x, const Behavior<Tout>& y ) {
  return snapshotE(x.getRef(),y.getRef()) ;
}

template<class T>
inline BehaviorRef<Maybe<Time> > timeOfE( BehaviorRef<Maybe<T> > x ) {
  return snapshotE(x,timeB);
}
template<class T>
inline BehaviorRef<Maybe<Time> > timeOfE( const Event<T>& x ) {
  return snapshotE(x.getRef(),timeB);
}

template<class T>
inline BehaviorRef<Time> stepTimeOfE( BehaviorRef<Maybe<T> > x ) {
  return stepB<Time>(0)(timeOfE(x));
}
template<class T>
inline BehaviorRef<Time> stepTimeOfE( const Event<T>& x ) {
  return stepB<Time>(0)(timeOfE(x.getRef()));
}


// nowE and notNowE

class EventNowModule : public BehaviorModule<Maybe<void> > {
 protected:
  bool alive ;
 public:
  EventNowModule() : alive(true) {}
  void compute(void) { value = alive ; alive = false ; }
};

inline BehaviorRef<Maybe<void> > nowE(void) {
  return new EventNowModule ;
}

template<class T>
BehaviorRef<Maybe<T> > notNowE( BehaviorRef<Maybe<T> > x ) {
  return x && ! nowE() ;
}
template<class T>
BehaviorRef<Maybe<T> > notNowE( const Event<T>& x ) {
  return notNowE( x.getRef() ) ;
}

// switch and till

# define TillB *=
# define ThenB |=
# define ThenConstB &=

template<class T, class B>
inline T if_then_else_fun( const B& t, const T& x, const T& y ) {
  return t ? x : y ;
}
template<class T, class B>
inline BehaviorRef<T>
ifB( BehaviorRef<B> t, BehaviorRef<T> x, BehaviorRef<T> y ) {
  return liftB(&if_then_else_fun<T,B>)(t,x,y);
}
template<class T, class B>
inline BehaviorRef<T>
ifB( const Behavior<B>& t, BehaviorRef<T> x, BehaviorRef<T> y ) {
  return ifB(t.getRef(),x,y);
}
template<class T, class B>
inline BehaviorRef<T>
ifB( BehaviorRef<B> t, const Behavior<T>& x, BehaviorRef<T> y ) {
  return ifB(t,x.getRef(),y);
}
template<class T, class B>
inline BehaviorRef<T>
ifB( BehaviorRef<B> t, BehaviorRef<T> x, const Behavior<T>& y ) {
  return ifB(t,x,y.getRef());
}
template<class T, class B>
inline BehaviorRef<T>
ifB( const Behavior<B>& t, const Behavior<T>& x, BehaviorRef<T> y ) {
  return ifB(t.getRef(),x.getRef(),y);
}
template<class T, class B>
inline BehaviorRef<T>
ifB( BehaviorRef<B> t, const Behavior<T>& x, const Behavior<T>& y ) {
  return ifB(t,x.getRef(),y.getRef());
}
template<class T, class B>
inline BehaviorRef<T>
ifB( const Behavior<B>& t, BehaviorRef<T> x, const Behavior<T>& y ) {
  return ifB(t.getRef(),x,y.getRef());
}
template<class T, class B>
inline BehaviorRef<T>
ifB( const Behavior<B>& t, const Behavior<T>& x, const Behavior<T>& y ) {
  return ifB(t.getRef(),x.getRef(),y.getRef());
}


template<class T>
class Switch2Module : public BehaviorModule<T> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<BehaviorRef<T> > >* e ;
 public:
  Switch2Module( BehaviorBase<T>* x, 
		 BehaviorBase<Maybe<BehaviorRef<T> > >* y ) :
    p(x), e(y) { attach(e); }
  void gc_more_marking() { p->gc_mark(); }
  void compute(void) {
    if( e->get() ) {
      BehaviorRef<T> x = e->get().getValue() ;
      x->update( last );
      value = x->get();
    }else {
      p->update( last );
      value = p->get();
    }
  }
};

template<class T>
inline BehaviorRef<T> 
switch2B( BehaviorRef<T> x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return new Switch2Module<T>(x.get(),y.get());
}
template<class T>
inline BehaviorRef<T> 
switch2B( const Behavior<T>& x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return switch2B<T>(x.getRef(),y);
}
template<class T>
inline BehaviorRef<T> 
switch2B( BehaviorRef<T> x, const Event<BehaviorRef<T> >& y ) {
  return switch2B<T>(x,y.getRef());
}
template<class T>
inline BehaviorRef<T> 
switch2B( const Behavior<T>& x, const Event<BehaviorRef<T> >& y ) {
  return switch2B<T>(x.getRef(),y.getRef());
}

template<class T>
class SwitchModule : public BehaviorModule<T> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<BehaviorRef<T> > >* e ;
 public:
  SwitchModule( BehaviorBase<T>* x, BehaviorBase<Maybe<BehaviorRef<T> > >* y):
    p(x), e(y) { attach(e); }
  void gc_more_marking() { p->gc_mark() ; }
  void compute(void) {
    if( e->get() ) {
      BehaviorRef<T> x = e->get().getValue() ;
      p = x.get() ;
    }
    p->update( last );
    value = p->get();
  }
};

template<class T>
inline BehaviorRef<T> 
switchB( BehaviorRef<T> x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return new SwitchModule<T>(x.get(),y.get());
}
template<class T>
inline BehaviorRef<T> 
switchB( const Behavior<T>& x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return switchB(x.getRef(),y);
}
template<class T>
inline BehaviorRef<T> 
switchB( BehaviorRef<T> x, const Event<BehaviorRef<T> >& y ) {
  return switchB(x,y.getRef());
}
template<class T>
inline BehaviorRef<T> 
switchB( const Behavior<T>& x, const Event<BehaviorRef<T> >& y ) {
  return switchB(x.getRef(),y.getRef());
}

template<class T>
inline BehaviorRef<T> 
tillB( BehaviorRef<T> x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return switchB(x,onceE(y));
}
template<class T>
inline BehaviorRef<T> 
tillB( const Behavior<T>& x, BehaviorRef<Maybe<BehaviorRef<T> > > y ) {
  return tillB(x.getRef(),y);
}
template<class T>
inline BehaviorRef<T> 
tillB( BehaviorRef<T> x, const Event<BehaviorRef<T> >& y ) {
  return tillB(x,y.getRef());
}
template<class T>
inline BehaviorRef<T> 
tillB( const Behavior<T>& x, const Event<BehaviorRef<T> >& y ) {
  return tillB(x.getRef(),y.getRef());
}

template<class T>
inline BehaviorRef<T> 
operator TillB( BehaviorRef<T> x, BehaviorRef<Maybe<BehaviorRef<T> > > y){
  return tillB(x,y);
}
template<class T>
inline BehaviorRef<T> 
operator TillB( const Behavior<T>& x, BehaviorRef<Maybe<BehaviorRef<T> > > y){
  return tillB(x,y);
}
template<class T>
inline BehaviorRef<T> 
operator TillB( BehaviorRef<T> x, const Event<BehaviorRef<T> >& y ) {
  return tillB(x,y);
}
template<class T>
inline BehaviorRef<T> 
operator TillB( const Behavior<T>& x, const Event<BehaviorRef<T> >& y ) {
  return tillB(x,y);
}

template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( BehaviorRef<Maybe<Tin> > x, Fun1<Tout,Tin> f ) {
  return liftE(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( const Event<Tin>& x, Fun1<Tout,Tin> f ) {
  return liftE(f)(x.getRef());
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( BehaviorRef<Maybe<Tin> > x, Fun1<Tout,Tin> f ) {
  return thenB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( const Event<Tin>& x, Fun1<Tout,Tin> f ) {
  return thenB(x,f);
}

template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( BehaviorRef<Maybe<Tin> > x, Tout (*f)(Tin) ) {
  return liftE(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( const Event<Tin>& x, Tout (*f)(Tin) ) {
  return liftE(f)(x.getRef());
}
template<class Tout>
inline BehaviorRef<Maybe<Tout> >
thenB ( BehaviorRef<Maybe<void> > x, Tout (*f)(void) ) {
  return liftE(f)(x);
}
template<class Tout>
inline BehaviorRef<Maybe<Tout> >
thenB ( const Event<void>& x, Tout (*f)(void) ) {
  return liftE(f)(x.getRef());
}

template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( BehaviorRef<Maybe<Tin> > x, Tout (*f)(Tin) ) {
  return thenB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( const Event<Tin>& x, Tout (*f)(Tin) ) {
  return thenB(x,f);
}
template<class Tout>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( BehaviorRef<Maybe<void> > x, Tout (*f)() ) {
  return thenB(x,f);
}
template<class Tout>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( const Event<void>& x, Tout (*f)() ) {
  return thenB(x,f);
}

template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( BehaviorRef<Maybe<Tin> > x, Tout (*f)(const Tin&) ) {
  return liftE(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenB ( const Event<Tin>& x, Tout (*f)(const Tin&) ) {
  return liftE(f)(x.getRef());
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( BehaviorRef<Maybe<Tin> > x, Tout (*f)(const Tin&) ) {
  return thenB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenB ( const Event<Tin> x, Tout (*f)(const Tin&) ) {
  return thenB(x,f);
}

template<class Tout, class Tin>
inline typename LiftTraits<LiftMaybe1<Tout,Tin,ConstFun1<Tout,Tin> > >::Result
liftConstE( const Tout& x ) {
  typedef ConstFun1<Tout,Tin> Fn ;
  return lift(LiftMaybe1<Tout,Tin,Fn>(Fn(x)));
}

template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
thenConstB ( BehaviorRef<Maybe<Tin> > x, BehaviorRef<Tout> f ) {
  return liftConstE<BehaviorRef<Tout>,Tin>(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
thenConstB ( const Event<Tin>& x, BehaviorRef<Tout> f ) {
  return liftConstE<BehaviorRef<Tout>,Tin>(f)(x.getRef());
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
operator ThenConstB ( BehaviorRef<Maybe<Tin> > x, BehaviorRef<Tout> f ) {
  return thenConstB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
operator ThenConstB ( const Event<Tin>& x, BehaviorRef<Tout> f ) {
  return thenConstB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
thenConstB ( BehaviorRef<Maybe<Tin> > x, const Behavior<Tout>& f ) {
  return liftConstE<BehaviorRef<Tout>,Tin>(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
thenConstB ( const Event<Tin>& x, const Behavior<Tout>& f ) {
  return liftConstE<BehaviorRef<Tout>,Tin>(f)(x.getRef());
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
operator ThenConstB ( BehaviorRef<Maybe<Tin> > x, const Behavior<Tout>& f ) {
  return thenConstB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<BehaviorRef<Tout> > >
operator ThenConstB ( const Event<Tin>& x, const Behavior<Tout>& f ) {
  return thenConstB(x.getRef(),f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenConstB ( BehaviorRef<Maybe<Tin> > x, Tout f ) {
  return liftConstE<Tout,Tin>(f)(x);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
thenConstB ( const Event<Tin>& x, Tout f ) {
  return thenConstB(x.getRef(),f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenConstB ( BehaviorRef<Maybe<Tin> > x, Tout f ) {
  return thenConstB(x,f);
}
template<class Tout, class Tin>
inline BehaviorRef<Maybe<Tout> >
operator ThenConstB ( const Event<Tin>& x, Tout f ) {
  return thenConstB(x.getRef(),f);
}



} // namespace frp 

# endif // _FRP_EVENT_H_
