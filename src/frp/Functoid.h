# ifndef _FUNCTOID_H_
# define _FUNCTOID_H_

namespace frp {

// Reference class is generic, i.e. can be used other than for laziness purpose

template<class T> 
class Ref {
 private:
  T * ptr ;
  unsigned int * count ;
  bool owner ;

 protected:
  inline bool hasValue(void) const { return ptr ; }
  inline int numRef(void) const { return *count ; }

  void inc(void) { if( hasValue() ) (*count)++; }
  void dec(void) { if( hasValue()  && !--(*count) ) 
    { if(owner) { delete ptr ; ptr = 0 ; } delete count ; } }

 public:
  inline T* get(void) const { return ptr; }
  void set( T * p, bool own=false )
    {
      if( ptr != p ) {
        dec();
        ptr = p ;
        count = 0 ;
        owner = own ;
        if( hasValue() ) {
          count = new unsigned int(0);
          inc();
        }
      }
    }
  void copy( const Ref<T>& r ) 
    {
      if( &r != this ) {
        dec();
        ptr = r.ptr ;
        count = r.count ;
        owner = r.owner ;
        inc();
      }
    }

 public:
  Ref() : ptr(0) {}

  Ref( const T& x ) : ptr(0) { set( new T(x), true ); }
  Ref( T * px, bool own=false ) : ptr(0) { set( px, own ); }

  virtual ~Ref() { dec(); }

  Ref( const Ref<T>& r ) : ptr(0) { copy(r); }

  Ref<T>& detach(void)   // make this independent
    {
      if( hasValue() && numRef() > 1 ) {
        set( new T(*ptr), true );
      }
      return *this ;
    }
  
  Ref<T>& operator = ( const Ref<T>& r )
    { copy(r) ; return (*this); }

  Ref<T>& operator = ( const T& x ) { set( new T(x), true ); return *this ; }

  Ref<T>& operator = ( T * px ) { set( px, false ); return *this ; }

  operator T* (void) const { return get() ; }
  T* operator ->(void) const { return get() ; }
};

// Abstract root

template<class R>
class AbsFunctoid {
 public:
  typedef R Result ;

  virtual ~AbsFunctoid() {} ;
};

template<class N> class LambdaExp ;
template<class T> class ValueNode ;

// Functoids with no argument

template<class R>
class Functoid0 : public AbsFunctoid<R> {
 public:
  virtual R operator () (void) = 0 ;
};

template<class R>
class Fun0 : public Functoid0<R> {
 protected:
  Ref<Functoid0<R> > ref ;
 public:
  explicit Fun0( Functoid0<R>* pf ) : ref(pf, true) {}
  virtual R operator () (void) { return ref->operator()(); }
};

template<class R>
class ConstFun0 : public Functoid0<R> {
 protected:
  R f ;
 public:
  explicit ConstFun0( const R& fn ) : f(fn) {}
  virtual R operator () (void) { return f ; }
};

template<class F, class R>
class DirectFun0 : public Functoid0<R> {
 protected:
  F f ;
 public:
  explicit DirectFun0( const F& fn ) : f(fn) {}
  virtual R operator () (void) { return f() ; }
};

template<class F, class R>
class ImplicitFun0 : public Functoid0<R> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun0( const F& fn ) : f(fn) {}
  virtual R operator () (void) { f(r) ; return r ; }
};

template<class F, class R>
class ImpClassFun0 : public Functoid0<R> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImpClassFun0( const F& fn ) : f(fn) {}
  virtual R operator () (void) { (r.*f)() ; return r ; }
};

template<class R>
class CtorFun0 : public Functoid0<R> {
 public:
  virtual R operator () (void) { return R() ; }
};

template<class F, class R, class C>
class ClassFun0 : public Functoid0<R> {
 protected:
  F f ;
  C c ;
 public:
  explicit ClassFun0( const F& fn, const C& host ) : f(fn), c(host) {}
  virtual R operator () (void) { return (c.*f)() ; }
  C * operator -> (void) { return &c ; }
  const C * operator -> (void) const { return &c ; }
};

template<class F, class R, class C>
class ClassPtrFun0 : public Functoid0<R> {
 protected:
  F f ;
  C * c ;
 public:
  explicit ClassPtrFun0( const F& fn, C * host ) : f(fn), c(host) {}
  virtual R operator () (void) { return (c->*f)() ; }
  C * operator -> (void) { return c ; }
  const C * operator -> (void) const { return c ; }
};


// Functoids with one argument

template<class R, class A1> class Fun1 ;

template<class R, class A1>
class Functoid1 : public AbsFunctoid<R> {
 public:
  typedef A1 Argument1 ;
 public:
  virtual R operator () ( const A1& a1 ) = 0 ;
};

template<class F, class A1> class ExpNode1 ;

template<class R, class A1>
class Fun1 : public Functoid1<R,A1> {
 protected:
  Ref<Functoid1<R,A1> > ref ;
 public:
  explicit Fun1( Functoid1<R,A1>* pf ) : ref(pf, true) {}
  virtual R operator () ( const A1& a1 ) { return ref->operator()(a1); }

  template<class A> Fun1<R,A> compose1( Fun1<A1,A> f ) const;
  Fun0<R> bind1( const A1& a1 ) const;

  template<class A> Fun1<R,A> operator ()( Fun1<A1,A> f ) const
    { return compose1(f) ; }

  template<class N1> LambdaExp<ExpNode1<Fun1<R,A1>,N1> > 
    operator () ( const LambdaExp<N1>& e1 ) const;
};

template<class R, class A1>
class ConstFun1 : public Functoid1<R,A1> {
  typedef Functoid1<R,A1> Base ;
 protected:
  R f ;
 public:
  explicit ConstFun1( const R& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { return f ; }
};

template<class R>
class ConstFun1<R,void> : public ConstFun0<R> {
 public:
  explicit ConstFun1( const R& fn ) : ConstFun0<R>(fn) {}
};

template<class F, class R, class A1>
class DirectFun1 : public Functoid1<R,A1> {
 protected:
  F f ;
 public:
  explicit DirectFun1( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { return f(a1) ; }
};

template<class F, class R, class A1>
class ImplicitFun1a : public Functoid1<R,A1> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun1a( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { f(r,a1) ; return r ; }
};

template<class F, class R, class A1>
class ImplicitFun1b : public Functoid1<R,A1> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun1b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { f(a1,r) ; return r ; }
};

template<class F, class R>
class ImpTransFun1 : public Functoid1<R,R> {
 protected:
  F f ;
 public:
  explicit ImpTransFun1( const F& fn ) : f(fn) {}
  virtual R operator () ( const R& a1 ) { R r = a1; f(r) ; return r ; }
};

template<class F, class R, class A1>
class ImpClassFun1 : public Functoid1<R,A1> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImpClassFun1( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { (r.*f)(a1) ; return r ; }
};

template<class R, class A1>
class CtorFun1 : public Functoid1<R,A1> {
 public:
  virtual R operator () ( const A1& a1 ) { return R(a1) ; }
};

template<class F, class R, class A1>
class MemberFun1 : public Functoid1<R,A1> {
 protected:
  F f ;
 public:
  explicit MemberFun1( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { return (a1.*f)() ; }
};

template<class F, class R, class A1>
class DataMemFun : public Functoid1<R,A1> {
 protected:
  F f ;
 public:
  explicit DataMemFun( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1 ) { return a1.*f ; }
};

template<class F, class R, class A1, class C>
class ClassFun1 : public Functoid1<R,A1> {
 protected:
  F f ;
  C c ;
 public:
  explicit ClassFun1( const F& fn, const C& host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1 ) { return (c.*f)(a1) ; }
  C * operator -> (void) { return &c ; }
  const C * operator -> (void) const { return &c ; }
};

template<class F, class R, class A1, class C>
class ClassPtrFun1 : public Functoid1<R,A1> {
 protected:
  F f ;
  C * c ;
 public:
  explicit ClassPtrFun1( const F& fn, C * host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1 ) { return (c->*f)(a1) ; }
  C * operator -> (void) { return c ; }
  const C * operator -> (void) const { return c ; }
};


// Functoids with two arguments

template<class R, class A1, class A2>
class Functoid2 : public AbsFunctoid<R> {
 public:
  typedef A1 Argument1 ;
  typedef A2 Argument2 ;
 public:
  virtual R operator () ( const A1& a1, const A2& a2 ) = 0 ;
};

template<class F, class A1, class A2> class ExpNode2 ;

template<class R, class A1, class A2>
class Fun2 : public Functoid2<R,A1,A2> {
 protected:
  Ref<Functoid2<R,A1,A2> > ref ;
 public:
  explicit Fun2( Functoid2<R,A1,A2>* pf ) : ref(pf, true) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { return ref->operator()(a1,a2); }  

  Fun1<R,A2> bind1( const A1& a1 ) const ;
  Fun1<R,A1> bind2( const A2& a2 ) const ;
  Fun1<R,A2> operator () ( const A1& a1 ) const 
    { return bind1(a1) ; }

  template<class N1> LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,ValueNode<A2> > >
    operator () ( const LambdaExp<N1>& e1, const A2& x2 ) const;
  template<class N2> LambdaExp<ExpNode2<Fun2<R,A1,A2>,ValueNode<A1>,N2 > >
    operator () ( const A1& x1, const LambdaExp<N2>& e2 ) const;
  template<class N1, class N2> LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,N2> >
    operator () ( const LambdaExp<N1>& e1, const LambdaExp<N2>& e2 ) const;
};

template<class R, class A1, class A2>
class ConstFun2 : public Functoid2<R,A1,A2> {
 protected:
  R f ;
 public:
  explicit ConstFun2( const R& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) { return f ; }
};

template<class F, class R, class A1, class A2>
class DirectFun2 : public Functoid2<R,A1,A2> {
 protected:
  F f ;
 public:
  explicit DirectFun2( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) { return f(a1,a2) ; }
};

template<class F, class R, class A1, class A2>
class ImplicitFun2a : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun2a( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { f(r,a1,a2) ; return r ; }
};

template<class F, class R, class A1, class A2>
class ImplicitFun2b : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun2b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { f(a1,a2,r) ; return r ; }
};

template<class F, class R, class A2>
class ImpTransFun2a : public Functoid2<R,R,A2> {
 protected:
  F f ;
 public:
  explicit ImpTransFun2a( const F& fn ) : f(fn) {}
  virtual R operator () ( const R& a1, const A2& a2 ) 
    { R r=a1 ; f(r,a2) ; return r ; }
};

template<class F, class R, class A1>
class ImpTransFun2b : public Functoid2<R,A1,R> {
 protected:
  F f ;
 public:
  explicit ImpTransFun2b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const R& a2 ) 
    { R r=a2 ; f(a1,r) ; return r ; }
};

template<class F, class R, class A1, class A2>
class ImpClassFun2 : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImpClassFun2( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { (r.*f)(a1,a2) ; return r ; }
};

template<class R, class A1, class A2>
class CtorFun2 : public Functoid2<R,A1,A2> {
 public:
  virtual R operator () ( const A1& a1, const A2& a2 ) { return R(a1,a2) ; }
};

template<class F, class R, class A1, class A2>
class MemberFun2a : public Functoid2<R,A1,A2> {
 protected:
  F f ;
 public:
  explicit MemberFun2a( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) { return (a1.*f)(a2); }
};

template<class F, class R, class A1, class A2>
class MemberFun2b : public Functoid2<R,A1,A2> {
 protected:
  F f ;
 public:
  explicit MemberFun2b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) { return (a2.*f)(a1); }
};

template<class F, class R, class A1, class A2, class C>
class ClassFun2 : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  C c ;
 public:
  explicit ClassFun2( const F& fn, const C& host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { return (c.*f)(a1,a2) ; }
  C * operator -> (void) { return &c ; }
  const C * operator -> (void) const { return &c ; }
};

template<class F, class R, class A1, class A2, class C>
class ClassPtrFun2 : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  C * c ;
 public:
  explicit ClassPtrFun2( const F& fn, C * host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1, const A2& a2 ) 
    { return (c->*f)(a1,a2) ; }
  C * operator -> (void) { return c ; }
  const C * operator -> (void) const { return c ; }
};


// Functoids with three arguments

template<class R, class A1, class A2, class A3>
class Functoid3 : public AbsFunctoid<R> {
 public:
  typedef A1 Argument1 ;
  typedef A2 Argument2 ;
  typedef A3 Argument3 ;
 public:
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) = 0 ;
};

template<class R, class A1, class A2, class A3>
class Fun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  Ref<Functoid3<R,A1,A2,A3> > ref ;
 public:
  explicit Fun3( Functoid3<R,A1,A2,A3>* pf ) : ref(pf, true) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return ref->operator()(a1,a2,a3); }  

  Fun2<R,A2,A3> bind1( const A1& a1 ) const;
  Fun2<R,A1,A3> bind2( const A2& a2 ) const;
  Fun2<R,A1,A2> bind3( const A3& a3 ) const;
  Fun2<R,A2,A3> operator () ( const A1& a1 ) const
    { return bind1(a1); }
  Fun1<R,A3> operator () ( const A1& a1, const A2& a2 ) const
    { return bind1(a1).bind1(a2); }
};

template<class R, class A1, class A2, class A3>
class ConstFun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  R f ;
 public:
  explicit ConstFun3( const R& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& x3 ) 
    { return f ; }
};

template<class F, class R, class A1, class A2, class A3>
class DirectFun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
 public:
  explicit DirectFun3( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return f(a1,a2,a3) ; }
};

template<class F, class R, class A1, class A2, class A3>
class ImplicitFun3a : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun3a( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { f(r,a1,a2,a3) ; return r ; }
};

template<class F, class R, class A1, class A2, class A3>
class ImplicitFun3b : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImplicitFun3b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { f(a1,a2,a3,r) ; return r ; }
};

template<class F, class R, class A1, class A2>
class ImpTransFun3a : public Functoid3<R,R,A1,A2> {
 protected:
  F f ;
 public:
  explicit ImpTransFun3a( const F& fn ) : f(fn) {}
  virtual R operator () ( const R& r, const A1& a1, const A2& a2 ) 
    { f(r,a1,a2) ; return r ; }
};

template<class F, class R, class A1, class A2>
class ImpTransFun3b : public Functoid3<R,A1,A2,R> {
 protected:
  F f ;
 public:
  explicit ImpTransFun3b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const R& r ) 
    { f(a1,a2,r) ; return r ; }
};

template<class F, class R, class A1, class A2, class A3>
class ImpClassFun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  R r ;
 public:
  explicit ImpClassFun3( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { (r.*f)(a1,a2,a3) ; return r ; }
};

template<class R, class A1, class A2, class A3>
class CtorFun3 : public Functoid3<R,A1,A2,A3> {
 public:
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return R(a1,a2,a3) ; }
};

template<class F, class R, class A1, class A2, class A3>
class MemberFun3a : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
 public:
  explicit MemberFun3a( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return (a1.*f)(a2,a3); }
};

template<class F, class R, class A1, class A2, class A3>
class MemberFun3b : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
 public:
  explicit MemberFun3b( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return (a3.*f)(a1,a2); }
};

template<class F, class R, class A1, class A2, class A3,class C>
class ClassFun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  C c ;
 public:
  explicit ClassFun3( const F& fn, const C& host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return (c.*f)(a1,a2,a3) ; }
  C * operator -> (void) { return &c ; }
  const C * operator -> (void) const { return &c ; }
};

template<class F, class R, class A1, class A2, class A3, class C>
class ClassPtrFun3 : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  C * c ;
 public:
  explicit ClassPtrFun3( const F& fn, C * host ) : f(fn), c(host) {}
  virtual R operator () ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return (c->*f)(a1,a2,a3) ; }
  C * operator -> (void) { return c ; }
  const C * operator -> (void) const { return c ; }
};

// Functoids with four arguments

template<class R, class A1, class A2, class A3, class A4>
class Functoid4 : public AbsFunctoid<R> {
 public:
  typedef A1 Argument1 ;
  typedef A2 Argument2 ;
  typedef A3 Argument3 ;
  typedef A4 Argument4 ;
 public:
  virtual R operator () ( const A1& a1, const A2& a2, 
			  const A3& a3, const A4& a4 ) = 0 ;
};

template<class R, class A1, class A2, class A3, class A4>
class Fun4 : public Functoid4<R,A1,A2,A3,A4> {
 protected:
  Ref<Functoid4<R,A1,A2,A3,A4> > ref ;
 public:
  explicit Fun4( Functoid4<R,A1,A2,A3,A4>* pf ) : ref(pf, true) {}
  virtual R operator () ( const A1& a1, const A2& a2, 
			  const A3& a3, const A4& a4 ) 
    { return ref->operator()(a1,a2,a3,a4); }  

  Fun3<R,A2,A3,A4> bind1( const A1& a1 ) const;
  Fun3<R,A1,A3,A4> bind2( const A2& a2 ) const;
  Fun3<R,A1,A2,A4> bind3( const A3& a3 ) const;
  Fun3<R,A1,A2,A3> bind4( const A4& a4 ) const;
  Fun3<R,A2,A3,A4> operator () ( const A1& a1 ) const
    { return bind1(a1); }
  Fun2<R,A3,A4> operator () ( const A1& a1, const A2& a2 ) const
    { return bind1(a1).bind1(a2); }
  Fun1<R,A4> operator () ( const A1& a1, const A2& a2, const A3& a3 ) const
    { return bind1(a1).bind1(a2).bind1(a3); }
};

template<class R, class A1, class A2, class A3, class A4>
class ConstFun4 : public Functoid4<R,A1,A2,A3,A4> {
 protected:
  R f ;
 public:
  explicit ConstFun4( const R& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, 
			  const A3& x3, const A4& a4 ) 
    { return f ; }
};

template<class F, class R, class A1, class A2, class A3, class A4>
class DirectFun4 : public Functoid4<R,A1,A2,A3,A4> {
 protected:
  F f ;
 public:
  explicit DirectFun4( const F& fn ) : f(fn) {}
  virtual R operator () ( const A1& a1, const A2& a2, 
			  const A3& a3, const A4& a4 ) 
    { return f(a1,a2,a3,a4) ; }
};

// The unified liftF

struct UndefinedType {};

template<class F> 
struct FunTraits {
  typedef UndefinedType Arg ;
  typedef UndefinedType Agent ;
  typedef UndefinedType Result ;
};
template<class F, class C>
struct FunTraits2 {
  typedef UndefinedType Arg ;
  typedef UndefinedType Agent ;
  typedef UndefinedType Result ;
};
template<class R> 
struct FunTraits<R (*)()> {
  typedef R (*Arg)() ;
  typedef DirectFun0<Arg,R> Agent ;
  typedef Fun0<R> Result ;
};
template<class R> 
struct FunTraits<void (*)(R&)> {
  typedef void (*Arg)(R&) ;
  typedef ImplicitFun0<Arg,R> Agent ;
  typedef Fun0<R> Result ;
};
template<class R> 
struct FunTraits<void (R::*)()> {
  typedef void (R::*Arg)() ;
  typedef ImpClassFun0<Arg,R> Agent ;
  typedef Fun0<R> Result ;
};
template<class R, class C>
struct FunTraits2<R (C::*)(), C> {
  typedef R (C::*Arg)() ;
  typedef ClassFun0<Arg,R,C> Agent ;
  typedef Fun0<R> Result ;
};
template<class R, class C>
struct FunTraits2<R (C::*)() const, C> {
  typedef R (C::*Arg)() const;
  typedef ClassFun0<Arg,R,C> Agent ;
  typedef Fun0<R> Result ;
};
template<class R, class C>
struct FunTraits2<R (C::*)(), C*> {
  typedef R (C::*Arg)() ;
  typedef ClassPtrFun0<Arg,R,C> Agent ;
  typedef Fun0<R> Result ;
};
template<class R, class C>
struct FunTraits2<R (C::*)() const, C*> {
  typedef R (C::*Arg)() const;
  typedef ClassPtrFun0<Arg,R,C> Agent ;
  typedef Fun0<R> Result ;
};

template<class R, class A1>
struct FunTraits<R (*)(A1)> {
  typedef R (*Arg)(A1) ;
  typedef DirectFun1<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<R (*)(const A1&)> {
  typedef R (*Arg)(const A1&) ;
  typedef DirectFun1<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<void (*)(R&,A1)> {
  typedef void (*Arg)(R&,A1) ;
  typedef ImplicitFun1a<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<void (*)(R&,const A1&)> {
  typedef void (*Arg)(R&,const A1&) ;
  typedef ImplicitFun1a<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<void (*)(A1,R&)> {
  typedef void (*Arg)(A1,R&) ;
  typedef ImplicitFun1b<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<void (*)(const A1&,R&)> {
  typedef void (*Arg)(const A1&,R&) ;
  typedef ImplicitFun1b<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R>
struct FunTraits<R (*)(R&)> {
  typedef R (*Arg)(R&) ;
  typedef ImpTransFun1<Arg,R> Agent ;
  typedef Fun1<R,R> Result ;
};
template<class R, class A1>
struct FunTraits<void (R::*)(A1)> {
  typedef void (R::*Arg)(A1) ;
  typedef ImpClassFun1<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<void (R::*)(const A1&)> {
  typedef void (R::*Arg)(const A1&) ;
  typedef ImpClassFun1<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<R (A1::*)() const> {
  typedef R (A1::*Arg)() const ;
  typedef MemberFun1<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<R A1::*> {
  typedef R A1::*Arg ;
  typedef DataMemFun<Arg,R,A1> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(A1),C> {
  typedef R (C::*Arg)(A1) ;
  typedef ClassFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(A1) const,C> {
  typedef R (C::*Arg)(A1) const;
  typedef ClassFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(const A1&),C> {
  typedef R (C::*Arg)(const A1&) ;
  typedef ClassFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(const A1&) const,C> {
  typedef R (C::*Arg)(const A1&) const;
  typedef ClassFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(A1),C*> {
  typedef R (C::*Arg)(A1) ;
  typedef ClassPtrFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(A1) const,C*> {
  typedef R (C::*Arg)(A1) const;
  typedef ClassPtrFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(const A1&),C*> {
  typedef R (C::*Arg)(const A1&) ;
  typedef ClassPtrFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};
template<class R, class A1, class C>
struct FunTraits2<R (C::*)(const A1&) const,C*> {
  typedef R (C::*Arg)(const A1&) const;
  typedef ClassPtrFun1<Arg,R,A1,C> Agent ;
  typedef Fun1<R,A1> Result ;
};

template<class R, class A1, class A2>
struct FunTraits<R (*)(A1,A2)> {
  typedef R (*Arg)(A1,A2) ;
  typedef DirectFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<R (*)(const A1&,A2)> {
  typedef R (*Arg)(const A1&,A2) ;
  typedef DirectFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<R (*)(A1,const A2&)> {
  typedef R (*Arg)(A1,const A2&) ;
  typedef DirectFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<R (*)(const A1&,const A2&)> {
  typedef R (*Arg)(const A1&,const A2&) ;
  typedef DirectFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(R&,A1,A2)> {
  typedef void (*Arg)(R&,A1,A2) ;
  typedef ImplicitFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(R&,const A1&,A2)> {
  typedef void (*Arg)(R&,const A1&,A2) ;
  typedef ImplicitFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(R&,A1,const A2&)> {
  typedef void (*Arg)(R&,A1,const A2&) ;
  typedef ImplicitFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(R&,const A1&,const A2&)> {
  typedef void (*Arg)(R&,const A1&,const A2&) ;
  typedef ImplicitFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(A1,A2,R&)> {
  typedef void (*Arg)(A1,A2,R&) ;
  typedef ImplicitFun2b<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(const A1&,A2,R&)> {
  typedef void (*Arg)(const A1&,A2,R&) ;
  typedef ImplicitFun2b<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(A1,const A2&,R&)> {
  typedef void (*Arg)(A1,const A2&,R&) ;
  typedef ImplicitFun2b<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (*)(const A1&,const A2&,R&)> {
  typedef void (*Arg)(const A1&,const A2&,R&) ;
  typedef ImplicitFun2b<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1>
struct FunTraits<R (*)(R&,A1)> {
  typedef R (*Arg)(R&,A1) ;
  typedef ImpTransFun2a<Arg,R,A1> Agent ;
  typedef Fun2<R,R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<R (*)(R&,const A1&)> {
  typedef R (*Arg)(R&,const A1&) ;
  typedef ImpTransFun2a<Arg,R,A1> Agent ;
  typedef Fun2<R,R,A1> Result ;
};
template<class R, class A1>
struct FunTraits<R (*)(A1,R&)> {
  typedef R (*Arg)(A1,R&) ;
  typedef ImpTransFun2b<Arg,R,A1> Agent ;
  typedef Fun2<R,A1,R> Result ;
};
template<class R, class A1>
struct FunTraits<R (*)(const A1&,R&)> {
  typedef R (*Arg)(const A1&,R&) ;
  typedef ImpTransFun2b<Arg,R,A1> Agent ;
  typedef Fun2<R,A1,R> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (R::*)(A1,A2)> {
  typedef void (R::*Arg)(A1,A2) ;
  typedef ImpClassFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (R::*)(const A1&,A2)> {
  typedef void (R::*Arg)(const A1&,A2) ;
  typedef ImpClassFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (R::*)(A1,const A2&)> {
  typedef void (R::*Arg)(A1,const A2&) ;
  typedef ImpClassFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<void (R::*)(const A1&,const A2&)> {
  typedef void (R::*Arg)(const A1&,const A2&) ;
  typedef ImpClassFun2<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<R (A1::*)(A2) const> {
  typedef R (A1::*Arg)(A2) const ;
  typedef MemberFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2>
struct FunTraits<R (A1::*)(const A2&) const> {
  typedef R (A1::*Arg)(const A2&) const ;
  typedef MemberFun2a<Arg,R,A1,A2> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,A2),C> {
  typedef R (C::*Arg)(A1,A2) ;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,A2) const,C> {
  typedef R (C::*Arg)(A1,A2) const;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,A2),C> {
  typedef R (C::*Arg)(const A1&,A2) ;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,A2) const,C> {
  typedef R (C::*Arg)(const A1&,A2) const;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,const A2&),C> {
  typedef R (C::*Arg)(A1,const A2&) ;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,const A2&) const,C> {
  typedef R (C::*Arg)(A1,const A2&) const;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,const A2&),C> {
  typedef R (C::*Arg)(const A1&,const A2&) ;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,const A2&) const,C> {
  typedef R (C::*Arg)(const A1&,const A2&) const;
  typedef ClassFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,A2),C*> {
  typedef R (C::*Arg)(A1,A2) ;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,A2) const,C*> {
  typedef R (C::*Arg)(A1,A2) const;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,A2),C*> {
  typedef R (C::*Arg)(const A1&,A2) ;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,A2) const,C*> {
  typedef R (C::*Arg)(const A1&,A2) const;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,const A2&),C*> {
  typedef R (C::*Arg)(A1,const A2&) ;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(A1,const A2&) const,C*> {
  typedef R (C::*Arg)(A1,const A2&) const;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,const A2&),C*> {
  typedef R (C::*Arg)(const A1&,const A2&) ;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};
template<class R, class A1, class A2, class C>
struct FunTraits2<R (C::*)(const A1&,const A2&) const,C*> {
  typedef R (C::*Arg)(const A1&,const A2&) const;
  typedef ClassPtrFun2<Arg,R,A1,A2,C> Agent ;
  typedef Fun2<R,A1,A2> Result ;
};

template<class R, class A1, class A2, class A3>
struct FunTraits<R (*)(A1,A2,A3)> {
  typedef R (*Arg)(A1,A2,A3) ;
  typedef DirectFun3<Arg,R,A1,A2,A3> Agent ;
  typedef Fun3<R,A1,A2,A3> Result ;
};

template<class R, class A1, class A2, class A3, class A4>
struct FunTraits<R (*)(A1,A2,A3,A4)> {
  typedef R (*Arg)(A1,A2,A3,A4) ;
  typedef DirectFun4<Arg,R,A1,A2,A3,A4> Agent ;
  typedef Fun4<R,A1,A2,A3,A4> Result ;
};


template<class F>
typename FunTraits<F>::Result liftF( F f ) {
  return typename FunTraits<F>::Result
    (new typename FunTraits<F>::Agent(f)) ;
}
template<class F, class C>
typename FunTraits2<F,C>::Result liftF( F f, const C& c ) {
  return typename FunTraits2<F,C>::Result
    (new typename FunTraits2<F,C>::Agent(f,c)) ;
}

template<class R>
Fun0<R> liftConstF0( const R& f ) {
  return Fun0<R>(new ConstFun0<R>(f)) ;
}
template<class R, class A1>
Fun1<R,A1> liftConstF1( const R& f ) {
  return Fun1<R,A1>(new ConstFun1<R,A1>(f)) ;
}
template<class R, class A1, class A2>
Fun2<R,A1,A2> liftConstF2( const R& f ) {
  return Fun2<R,A1,A2>(new ConstFun2<R,A1,A2>(f)) ;
}
template<class R, class A1, class A2, class A3>
Fun3<R,A1,A2,A3> liftConstF3( const R& f ) {
  return Fun3<R,A1,A2,A3>(new ConstFun3<R,A1,A2,A3>(f)) ;
}

template<class R>
Fun0<R> ctorF0() {
  return Fun0<R>(new CtorFun0<R>()) ;
}
template<class R, class A1>
Fun1<R,A1> ctorF1() {
  return Fun1<R,A1>(new CtorFun1<R,A1>()) ;
}
template<class R, class A1, class A2>
Fun2<R,A1,A2> ctorF2() {
  return Fun2<R,A1,A2>(new CtorFun2<R,A1,A2>()) ;
}
template<class R, class A1, class A2, class A3>
Fun3<R,A1,A2,A3> ctorF3() {
  return Fun3<R,A1,A2,A3>(new CtorFun3<R,A1,A2,A3>()) ;
}

// Functoid Operations

template<class F1, class F2, class R, class A>
class Compose1Fun1 : public Functoid1<R,A> {
 protected:
  F1 f ;
  F2 g ;
 public:
  explicit Compose1Fun1( const F1& fn, const F2& gn ) : f(fn), g(gn) {}
  virtual R operator () ( const A& a ) { return f(g(a)); }
};

template<class F, class G, class R, class A>
Fun1<R,A> compose1f1( F f, G g ) {
  return Fun1<R,A>(new Compose1Fun1<F,G,R,A>(f,g));
}
template<class R, class A, class T>
Fun1<R,A> compose1f1( Fun1<R,T> f, Fun1<T,A> g ) {
  return compose1f1<Fun1<R,T>,Fun1<T,A>,R,A>(f,g);
}
template<class R, class A, class T>
Fun1<R,A> compose1f1( R (*f)(T), T (*g)(A) ) {
  return compose1f1<R (*)(T),T (*)(A),R,A>(f,g);
}

template<class R, class A1> template<class A>
Fun1<R,A> Fun1<R,A1>::compose1( Fun1<A1,A> f ) const {
  return compose1f1( *this, f ) ;
}


// argument binding

template<class F, class R, class A1>
class Bind1Fun1 : public Functoid0<R> {
 protected:
  F f ;
  A1 a ;
 public:
  explicit Bind1Fun1( const F& fn, const A1& x ) : f(fn), a(x) {}
  virtual R operator() (void) { return f(a); }
};

template<class F, class R, class A1>
Fun0<R> bind1f1( F f, const A1& a ) {
  return Fun0<R>(new Bind1Fun1<F,R,A1>(f,a));
}
template<class R, class A1>
Fun0<R> bind1f1( Fun1<R,A1> f, const A1& a ) {
  return bind1f1<Fun1<R,A1>,R,A1>(f,a) ;
}
template<class R, class A1>
Fun0<R> bind1f1( R (*f)(A1), const A1& a ) {
  return bind1f1<R (*)(A1),R,A1>(f,a);
}
template<class R, class A1>
Fun0<R> Fun1<R,A1>::bind1( const A1& a1 ) const {
  return bind1f1( *this, a1 );
}

template<class F, class R, class A1, class A2>
class Bind2Fun1 : public Functoid1<R,A2> {
 protected:
  F f ;
  A1 a1 ;
 public:
  explicit Bind2Fun1( const F& fn, const A1& x ) : f(fn), a1(x) {}
  virtual R operator() ( const A2& a2 ) { return f(a1,a2); }
};
template<class F, class R, class A1, class A2>
class Bind2Fun2 : public Functoid1<R,A1> {
 protected:
  F f ;
  A2 a2 ;
 public:
  explicit Bind2Fun2( const F& fn, const A2& x ) : f(fn), a2(x) {}
  virtual R operator() ( const A1& a1 ) { return f(a1,a2); }
};

template<class F, class R, class A1, class A2>
Fun1<R,A2> bind2f1( F f, const A1& a ) {
  return Fun1<R,A2>(new Bind2Fun1<F,R,A1,A2>(f,a));
}
template<class F, class R, class A1, class A2>
Fun1<R,A1> bind2f2( F f, const A2& a ) {
  return Fun1<R,A1>(new Bind2Fun2<F,R,A1,A2>(f,a));
}
template<class R, class A1, class A2>
Fun1<R,A2> bind2f1( Fun2<R,A1,A2> f, const A1& a ) {
  return bind2f1<Fun2<R,A1,A2>,R,A1,A2>(f,a) ;
}
template<class R, class A1, class A2>
Fun1<R,A1> bind2f2( Fun2<R,A1,A2> f, const A2& a ) {
  return bind2f2<Fun2<R,A1,A2>,R,A1,A2>(f,a) ;
}
template<class R, class A1, class A2>
Fun1<R,A2> bind2f1( R (*f)(A1,A2), const A1& a ) {
  return bind2f1<R (*)(A1,A2),R,A1,A2>(f,a);
}
template<class R, class A1, class A2>
Fun1<R,A1> bind2f2( R (*f)(A1,A2), const A2& a ) {
  return bind2f2<R (*)(A1,A2),R,A1,A2>(f,a);
}
template<class R, class A1, class A2>
Fun1<R,A2> Fun2<R,A1,A2>::bind1( const A1& a1 ) const {
  return bind2f1( *this, a1 );
}
template<class R, class A1, class A2>
Fun1<R,A1> Fun2<R,A1,A2>::bind2( const A2& a2 ) const {
  return bind2f2( *this, a2 );
}

template<class F, class R, class A1, class A2, class A3>
class Bind3Fun1 : public Functoid2<R,A2,A3> {
 protected:
  F f ;
  A1 a1 ;
 public:
  explicit Bind3Fun1( const F& fn, const A1& x1 ) : f(fn), a1(x1) {}
  virtual R operator() ( const A2& a2, const A3& a3 ) { return f(a1,a2,a3); }
};
template<class F, class R, class A1, class A2, class A3>
class Bind3Fun2 : public Functoid2<R,A1,A3> {
 protected:
  F f ;
  A2 a2 ;
 public:
  explicit Bind3Fun2( const F& fn, const A2& x2 ) : f(fn), a2(x2) {}
  virtual R operator() ( const A1& a1, const A3& a3 ) { return f(a1,a2,a3); }
};
template<class F, class R, class A1, class A2, class A3>
class Bind3Fun3 : public Functoid2<R,A1,A2> {
 protected:
  F f ;
  A3 a3 ;
 public:
  explicit Bind3Fun3( const F& fn, const A3& x3 ) : f(fn), a3(x3) {}
  virtual R operator() ( const A1& a1, const A2& a2 ) { return f(a1,a2,a3); }
};
template<class F, class R, class A1, class A2, class A3>
Fun2<R,A2,A3> bind3f1( F f, const A1& a ) {
  return Fun2<R,A2,A3>(new Bind3Fun1<F,R,A1,A2,A3>(f,a));
}
template<class F, class R, class A1, class A2, class A3>
Fun2<R,A1,A3> bind3f2( F f, const A2& a ) {
  return Fun2<R,A1,A3>(new Bind3Fun2<F,R,A1,A2,A3>(f,a));
}
template<class F, class R, class A1, class A2, class A3>
Fun2<R,A1,A2> bind3f3( F f, const A3& a ) {
  return Fun2<R,A1,A2>(new Bind3Fun3<F,R,A1,A2,A3>(f,a));
}
template<class R, class A1, class A2, class A3>
Fun2<R,A2,A3> bind3f1( Fun3<R,A1,A2,A3> f, const A1& a ) {
  return bind3f1<Fun3<R,A1,A2,A3>,R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A3> bind3f2( Fun3<R,A1,A2,A3> f, const A2& a ) {
  return bind3f2<Fun3<R,A1,A2,A3>,R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A2> bind3f3( Fun3<R,A1,A2,A3> f, const A3& a ) {
  return bind3f3<Fun3<R,A1,A2,A3>,R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A2,A3> bind3f1( R (*f)(A1,A2,A3), const A1& a ) {
  return bind3f1<R (*)(A1,A2,A3),R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A3> bind3f2( R (*f)(A1,A2,A3), const A2& a ) {
  return bind3f2<R (*)(A1,A2,A3),R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A2> bind3f3( R (*f)(A1,A2,A3), const A3& a ) {
  return bind3f3<R (*)(A1,A2,A3),R,A1,A2,A3>(f,a);
}
template<class R, class A1, class A2, class A3>
Fun2<R,A2,A3> Fun3<R,A1,A2,A3>::bind1( const A1& a1 ) const {
  return bind3f1( *this, a1 );
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A3> Fun3<R,A1,A2,A3>::bind2( const A2& a2 ) const {
  return bind3f2( *this, a2 );
}
template<class R, class A1, class A2, class A3>
Fun2<R,A1,A2> Fun3<R,A1,A2,A3>::bind3( const A3& a3 ) const {
  return bind3f3( *this, a3 );
}

template<class F, class R, class A1, class A2, class A3, class A4>
class Bind4Fun1 : public Functoid3<R,A2,A3,A4> {
 protected:
  F f ;
  A1 a1 ;
 public:
  explicit Bind4Fun1( const F& fn, const A1& x1 ) : f(fn), a1(x1) {}
  virtual R operator() ( const A2& a2, const A3& a3, const A4& a4 ) 
    { return f(a1,a2,a3,a4); }
};
template<class F, class R, class A1, class A2, class A3, class A4>
class Bind4Fun2 : public Functoid3<R,A1,A3,A4> {
 protected:
  F f ;
  A2 a2 ;
 public:
  explicit Bind4Fun2( const F& fn, const A2& x2 ) : f(fn), a2(x2) {}
  virtual R operator() ( const A1& a1, const A3& a3, const A4& a4 ) 
    { return f(a1,a2,a3,a4); }
};
template<class F, class R, class A1, class A2, class A3, class A4>
class Bind4Fun3 : public Functoid3<R,A1,A2,A4> {
 protected:
  F f ;
  A3 a3 ;
 public:
  explicit Bind4Fun3( const F& fn, const A3& x3 ) : f(fn), a3(x3) {}
  virtual R operator() ( const A1& a1, const A2& a2, const A4& a4 ) 
    { return f(a1,a2,a3,a4); }
};
template<class F, class R, class A1, class A2, class A3, class A4>
class Bind4Fun4 : public Functoid3<R,A1,A2,A3> {
 protected:
  F f ;
  A4 a4 ;
 public:
  explicit Bind4Fun4( const F& fn, const A4& x4 ) : f(fn), a4(x4) {}
  virtual R operator() ( const A1& a1, const A2& a2, const A3& a3 ) 
    { return f(a1,a2,a3,a4); }
};
template<class F, class R, class A1, class A2, class A3, class A4>
Fun3<R,A2,A3,A4> bind4f1( F f, const A1& a ) {
  return Fun3<R,A2,A3,A4>(new Bind4Fun1<F,R,A1,A2,A3,A4>(f,a));
}
template<class F, class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A3,A4> bind4f2( F f, const A2& a ) {
  return Fun3<R,A1,A3,A4>(new Bind4Fun2<F,R,A1,A2,A3,A4>(f,a));
}
template<class F, class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A4> bind4f3( F f, const A3& a ) {
  return Fun3<R,A1,A2,A4>(new Bind4Fun3<F,R,A1,A2,A3,A4>(f,a));
}
template<class F, class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A3> bind4f4( F f, const A4& a ) {
  return Fun3<R,A1,A2,A3>(new Bind4Fun4<F,R,A1,A2,A3,A4>(f,a));
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A2,A3,A4> bind4f1( Fun4<R,A1,A2,A3,A4> f, const A1& a ) {
  return bind4f1<Fun4<R,A1,A2,A3,A4>,R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A3,A4> bind4f2( Fun4<R,A1,A2,A3,A4> f, const A2& a ) {
  return bind4f2<Fun4<R,A1,A2,A3,A4>,R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A4> bind4f3( Fun4<R,A1,A2,A3,A4> f, const A3& a ) {
  return bind4f3<Fun4<R,A1,A2,A3,A4>,R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A3> bind4f4( Fun4<R,A1,A2,A3,A4> f, const A4& a ) {
  return bind4f4<Fun4<R,A1,A2,A3,A4>,R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A2,A3,A4> bind4f1( R (*f)(A1,A2,A3,A4), const A1& a ) {
  return bind4f1<R (*)(A1,A2,A3,A4),R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A3,A4> bind4f2( R (*f)(A1,A2,A3,A4), const A2& a ) {
  return bind4f2<R (*)(A1,A2,A3,A4),R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A4> bind4f3( R (*f)(A1,A2,A3,A4), const A3& a ) {
  return bind4f3<R (*)(A1,A2,A3,A4),R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A3> bind4f4( R (*f)(A1,A2,A3,A4), const A4& a ) {
  return bind4f4<R (*)(A1,A2,A3,A4),R,A1,A2,A3,A4>(f,a);
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A2,A3,A4> Fun4<R,A1,A2,A3,A4>::bind1( const A1& a1 ) const {
  return bind4f1( *this, a1 );
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A3,A4> Fun4<R,A1,A2,A3,A4>::bind2( const A2& a2 ) const {
  return bind4f1( *this, a2 );
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A4> Fun4<R,A1,A2,A3,A4>::bind3( const A3& a3 ) const {
  return bind4f3( *this, a3 );
}
template<class R, class A1, class A2, class A3, class A4>
Fun3<R,A1,A2,A3> Fun4<R,A1,A2,A3,A4>::bind4( const A4& a4 ) const {
  return bind4f4( *this, a4 );
}


} // namespace frp

# endif //_FUNCTOID_H_
