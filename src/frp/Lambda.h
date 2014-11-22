# ifndef _LAMBDA_H_
# define _LAMBDA_H_

# include "Functoid.h"

namespace frp {

/* 
 Here we use 'typed' lambda variables, however, it's also possible
 to use 'uuntyped' lambda variables, which means the type of expression
 can not be determined until function appication (lambda variable 
 substitution). This would be more general in some way, but it would prevent
 lambda expression be a subtype of functoid, which doesn't fit our FRP
 needs.
*/

// LambdaExp<XxxxNode> is a lambda expression tree
// Except in the case of a single ValueNode, no LambdaExp head
// However, the single ValueNode only exists in the lambda variable 
// substitution phase, never in real LambdaFun

template<class N>
struct LambdaExp {
  typedef N Node ;
  typedef typename N::Result Result ;
  N node ;
  LambdaExp() {}  // for LambdaVar
  LambdaExp( const N& n ) : node(n) {}
};

 
// Leaf node of a single piece of data
template<class D>
struct ValueNode {
  typedef D Result ;
  D value ;
  ValueNode( const D& v ) : value(v) {}
};

// Leaf node of a single lambda variable

template<class D, int h>
struct LambdaNode {
  typedef D Result ;
  enum { id = h } ;
};

// Lambda varaibles, each has its unique type

template<class T>
struct LambdaVar {
  typedef LambdaExp<LambdaNode<T, 1> > A ;
  typedef LambdaExp<LambdaNode<T, 2> > B ;
  typedef LambdaExp<LambdaNode<T, 3> > C ;
  typedef LambdaExp<LambdaNode<T,24> > X ;
  typedef LambdaExp<LambdaNode<T,25> > Y ;
  typedef LambdaExp<LambdaNode<T,26> > Z ;
};

// Tree non-leaf Nodes

// Type traits

template<class F>
struct ExpTypeTraits {
  // set default for functoids
  typedef typename F::Result Result ;
};

template<class N, class L>
struct EvalExpNodeTraits {
  typedef N Result ;
};
template<class N, class L, class D>
inline typename EvalExpNodeTraits<N,L>::Result 
evalExpNode( const N& node, L var, const D& data ) {
  return node ;
}

template<class L>
struct EvalExpNodeTraits<L,L> {
  typedef ValueNode<typename L::Result> Result ;
};
template<class L, class D>
inline typename EvalExpNodeTraits<L,L>::Result
evalExpNode( const L& node, L var, const D& data ) {
  typedef typename EvalExpNodeTraits<L,L>::Result Result ;
  return Result(data) ;
}

// Node has 0 child -- actually never used

template<class F>
struct ExpType0Traits : public ExpTypeTraits<F> {};

template<class F>
struct ExpNode0 {
  typedef typename ExpType0Traits<F>::Result Result ;
  F func ;
  ExpNode0( const F& f ) : func(f) {}
};

template<class F>
struct MakeExpNode0Traits {
  typedef typename ExpNode0<F>::Result Result ;
};
template<class F>
inline typename MakeExpNode0Traits<F>::Result
makeExpNode0( const F& f ) {
  typedef typename MakeExpNode0Traits<F>::Result Result ;
  return Result(f()) ;
}

template<class F, class L>
struct EvalExpNodeTraits<ExpNode0<F>,L> {
  typedef typename MakeExpNode0Traits<F>::Result Result ;
};
template<class F, class L, class D>
inline typename EvalExpNodeTraits<ExpNode0<F>,L>::Result
evalExpNode( const ExpNode0<F>& node, L var, const D& data ) {
  return makeExpNode0( node.func );
}


// Node has 1 child -- unary functions

template<class F, class A1>
struct ExpType1Traits : public ExpTypeTraits<F> {};

template<class F, class A1>
struct ExpNode1 {
  typedef typename ExpType1Traits<F,typename A1::Result>::Result Result ;
  F func ;
  A1 arg1 ;
  ExpNode1( const F& f, const A1& a1 ) : func(f), arg1(a1) {}
};

template<class F, class A1>
struct MakeExpNode1Traits {
  typedef ExpNode1<F,A1> Result ;
};
template<class F, class A1>
inline typename MakeExpNode1Traits<F,A1>::Result
makeExpNode1( const F& f, const A1& a1 ) {
  typedef typename MakeExpNode1Traits<F,A1>::Result Result ;
  return Result(f,a1) ;
};

template<class F, class D1>
struct MakeExpNode1Traits<F,ValueNode<D1> > {
  typedef ValueNode<typename ExpType1Traits<F,D1>::Result> Result ;
};
template<class F, class D1>
inline typename MakeExpNode1Traits<F,ValueNode<D1> >::Result
makeExpNode1( const F& f, const ValueNode<D1>& a1 ) {
  typedef typename MakeExpNode1Traits<F,ValueNode<D1> >::Result Result ;
  return Result(const_cast<F&>(f)(a1.value));
}

template<class F, class A1, class L>
struct EvalExpNodeTraits<ExpNode1<F,A1>,L> {
  typedef typename MakeExpNode1Traits<F,typename EvalExpNodeTraits<A1,L>::Result>::Result Result ;
};
template<class F, class A1, class L, class D>
inline typename EvalExpNodeTraits<ExpNode1<F,A1>,L>::Result
evalExpNode( const ExpNode1<F,A1>& node, L var, const D& data ) {
  return makeExpNode1( node.func, evalExpNode( node.arg1, var, data ) ) ;
}

// Node has 2 childern -- binary functions

template<class F, class A1, class A2>
struct ExpType2Traits : public ExpTypeTraits<F> {};

template<class F, class A1, class A2>
struct ExpNode2 {
  typedef typename ExpType2Traits<F,typename A1::Result,typename A2::Result>::Result Result ;
  F func ;
  A1 arg1 ;
  A2 arg2 ;
  ExpNode2( const F& f, const A1& a1, const A2& a2 ) 
   : func(f), arg1(a1), arg2(a2) {}
};

template<class F, class A1, class A2>
struct MakeExpNode2Traits {
  typedef ExpNode2<F,A1,A2> Result ;
};
template<class F, class A1, class A2>
inline typename MakeExpNode2Traits<F,A1,A2>::Result
makeExpNode2( const F& f, const A1& a1, const A2& a2 ) {
  typedef typename MakeExpNode2Traits<F,A1,A2>::Result Result ;
  return Result(f,a1,a2) ;
};

template<class F, class D1, class D2>
struct MakeExpNode2Traits<F,ValueNode<D1>,ValueNode<D2> > {
  typedef ValueNode<typename ExpType2Traits<F,D1,D2>::Result> Result ;
};
template<class F, class D1, class D2>
inline typename MakeExpNode2Traits<F,ValueNode<D1>,ValueNode<D2> >::Result
makeExpNode2( const F& f, const ValueNode<D1>& a1, const ValueNode<D2>& a2 ) {
  typedef typename MakeExpNode2Traits<F,ValueNode<D1>,ValueNode<D2> >::Result Result ;
  return Result(const_cast<F&>(f)(a1.value,a2.value));
}

template<class F, class A1, class A2, class L>
struct EvalExpNodeTraits<ExpNode2<F,A1,A2>,L> {
  typedef typename MakeExpNode2Traits<F,typename EvalExpNodeTraits<A1,L>::Result,typename EvalExpNodeTraits<A2,L>::Result>::Result Result ;
};
template<class F, class A1, class A2, class L, class D>
inline typename EvalExpNodeTraits<ExpNode2<F,A1,A2>,L>::Result
evalExpNode( const ExpNode2<F,A1,A2>& node, L var, const D& data ) {
  return makeExpNode2( node.func, evalExpNode( node.arg1, var, data ),
                       evalExpNode( node.arg2, var, data ) ) ;
}

// Lambda support in functoid

template<class R, class A1> template<class N1>
LambdaExp<ExpNode1<Fun1<R,A1>,N1> >
Fun1<R,A1>::operator () ( const LambdaExp<N1>& e1 ) const {
  return LambdaExp<ExpNode1<Fun1<R,A1>,N1> >
         (ExpNode1<Fun1<R,A1>,N1>( *this, e1.node ));
}

template<class R, class A1, class A2> template<class N1>
LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,ValueNode<A2> > >
Fun2<R,A1,A2>::operator () ( const LambdaExp<N1>& e1, const A2& x2 ) const {
  return LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,ValueNode<A2> > >
         (ExpNode2<Fun2<R,A1,A2>,N1,ValueNode<A2> >
          ( *this, e1.node, ValueNode<A2>(x2) ));
}
template<class R, class A1, class A2> template<class N2>
LambdaExp<ExpNode2<Fun2<R,A1,A2>,ValueNode<A1>,N2 > >
Fun2<R,A1,A2>::operator () ( const A1& x1, const LambdaExp<N2>& e2 ) const {
  return LambdaExp<ExpNode2<Fun2<R,A1,A2>,ValueNode<A1>,N2 > >
         (ExpNode2<Fun2<R,A1,A2>,ValueNode<A1>,N2 >
          ( *this, ValueNode<A1>(x1), e2.node ));
}
template<class R, class A1, class A2> template<class N1, class N2>
LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,N2> >
Fun2<R,A1,A2>::operator () ( const LambdaExp<N1>& e1, const LambdaExp<N2>& e2 ) const {
  return LambdaExp<ExpNode2<Fun2<R,A1,A2>,N1,N2> >
         (ExpNode2<Fun2<R,A1,A2>,N1,N2>
          ( *this, e1.node, e2.node ));
}


// Lambda function

template<class N, class L>
class LambdaFun1 : public Functoid1<typename N::Result, typename L::Result> {
 protected:
  N node ;
  L var ;
 public:
  explicit LambdaFun1( L v, const N& n ) : node(n), var(v) {}
  virtual Result operator () ( const Argument1& x1 ) 
    { return evalExpNode( node, var, x1 ).value ; }
};

template<class N, class L>
inline Fun1<typename N::Result, typename L::Result>
lambda( LambdaExp<L> var, const LambdaExp<N>& exp ) {
  typedef typename N::Result R ;
  typedef typename L::Result A1 ;
  return Fun1<R,A1>(new LambdaFun1<N,L>(var.node,exp.node)) ;
}

template<class N, class L1, class L2>
class LambdaFun2 : public Functoid2<typename N::Result, typename L1::Result, typename L2::Result> {
 protected:
  N node ;
  L1 var1 ;
  L2 var2 ;
 public:
  explicit LambdaFun2( L1 v1, L2 v2, const N& n ) : 
    node(n), var1(v1), var2(v2) {}
  virtual Result operator () ( const Argument1& x1, const Argument2& x2 )
    { return evalExpNode( evalExpNode( node, var1, x1 ),
                          var2, x2 ).value ; }
};

template<class N, class L1, class L2>
inline Fun2<typename N::Result, typename L1::Result, typename L2::Result>
lambda( LambdaExp<L1> var1, LambdaExp<L2> var2, const LambdaExp<N>& exp ) {
  typedef typename N::Result R ;
  typedef typename L1::Result A1 ;
  typedef typename L2::Result A2 ;
  return Fun2<R,A1,A2>(new LambdaFun2<N,L1,L2>(var1.node,var2.node,exp.node)) ;
}


// Operator support

// Unary operator

# define DEF_FUN_OP_UNARY( op, name ) \
struct Op##name { \
  template<class A1> \
  typename ExpType1Traits<Op##name,A1>::Result \
  operator () ( const A1& a1 ) const \
    { return op a1; } \
}; \
\
template<class N1> \
inline LambdaExp<ExpNode1<Op##name,N1> > \
operator op ( const LambdaExp<N1>& e1 ) { \
  return LambdaExp<ExpNode1<Op##name,N1> > \
         (ExpNode1<Op##name,N1>( Op##name(), e1.node )) ; \
} \


DEF_FUN_OP_UNARY( -, Negative )
template<class T>
struct ExpType1Traits<OpNegative,T> {
  typedef T Result ;
};
DEF_FUN_OP_UNARY( !, Not )
template<class T>
struct ExpType1Traits<OpNot,T> {
  typedef bool Result ;
};

// Binary operator

# define DEF_FUN_OP_BINARY( op, op2, name ) \
struct Op##name { \
  template<class A1,class A2> \
  typename ExpType2Traits<Op##name,A1,A2>::Result \
  operator () ( const A1& a1, const A2& a2 ) const \
    { return a1 op a2 op2 ; } \
}; \
\
template<class N1, class A2> \
inline LambdaExp<ExpNode2<Op##name,N1,ValueNode<A2> > > \
operator op op2 ( const LambdaExp<N1>& e1, const A2& x2 ) { \
  return LambdaExp<ExpNode2<Op##name,N1,ValueNode<A2> > > \
         (ExpNode2<Op##name,N1,ValueNode<A2> > \
          ( Op##name(), e1.node, ValueNode<A2>(x2) )); \
} \
template<class A1, class N2> \
inline LambdaExp<ExpNode2<Op##name,ValueNode<A1>,N2 > > \
operator op op2 ( const A1& x1, const LambdaExp<N2>& e2 ) { \
  return LambdaExp<ExpNode2<Op##name,ValueNode<A1>,N2 > > \
         (ExpNode2<Op##name,ValueNode<A1>,N2 > \
          ( Op##name(), ValueNode<A1>(x1), e2.node )); \
} \
template<class N1, class N2> \
inline LambdaExp<ExpNode2<Op##name,N1,N2> > \
operator op op2 ( const LambdaExp<N1>& e1, const LambdaExp<N2>& e2 ) { \
  return LambdaExp<ExpNode2<Op##name,N1,N2> > \
         (ExpNode2<Op##name,N1,N2> \
          ( Op##name(), e1.node, e2.node )); \
} \


template<class T1, class T2> 
struct TypePromotionTraits { typedef T1 Result ; };  // default is left type

DEF_FUN_OP_BINARY( +, , Plus )
template<class T1, class T2>
struct ExpType2Traits<OpPlus,T1,T2> {
  typedef typename TypePromotionTraits<T1,T2>::Result Result ;
};
DEF_FUN_OP_BINARY( -, , Minus )
template<class T1, class T2>
struct ExpType2Traits<OpMinus,T1,T2> {
  typedef typename TypePromotionTraits<T1,T2>::Result Result ;
};
DEF_FUN_OP_BINARY( *, , Multiplies )
template<class T1, class T2>
struct ExpType2Traits<OpMultiplies,T1,T2> {
  typedef typename TypePromotionTraits<T1,T2>::Result Result ;
};
DEF_FUN_OP_BINARY( /, , Divides )
template<class T1, class T2>
struct ExpType2Traits<OpDivides,T1,T2> {
  typedef typename TypePromotionTraits<T1,T2>::Result Result ;
};
DEF_FUN_OP_BINARY( >, , Greater )
template<class T1, class T2>
struct ExpType2Traits<OpGreater,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( >=, , GreaterEqual )
template<class T1, class T2>
struct ExpType2Traits<OpGreaterEqual,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( <, , Less )
template<class T1, class T2>
struct ExpType2Traits<OpLess,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( <=, , LessEqual )
template<class T1, class T2>
struct ExpType2Traits<OpLessEqual,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( ==, , Equal )
template<class T1, class T2>
struct ExpType2Traits<OpEqual,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( !=, , NotEqual )
template<class T1, class T2>
struct ExpType2Traits<OpNotEqual,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( &&, , And )
template<class T1, class T2>
struct ExpType2Traits<OpAnd,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( ||, , Or )
template<class T1, class T2>
struct ExpType2Traits<OpOr,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( ^, , Xor )
template<class T1, class T2>
struct ExpType2Traits<OpXor,T1,T2> {
  typedef bool Result ;
};
DEF_FUN_OP_BINARY( <<, , LeftShift )
template<class T1, class T2>
struct ExpType2Traits<OpLeftShift,T1,T2> {
  typedef T1 Result ;
};
DEF_FUN_OP_BINARY( >>, , RightShift )
template<class T1, class T2>
struct ExpType2Traits<OpRightShift,T1,T2> {
  typedef T1 Result ;
};
# ifndef FRP_COMMA
# define FRP_COMMA ,
# endif
DEF_FUN_OP_BINARY( FRP_COMMA, , Comma )
template<class T1, class T2>
struct ExpType2Traits<OpComma,T1,T2> {
  typedef T2 Result ;
};


// Type promotion table 

template<class T>
struct TypePromotionTraits<T,T> { typedef T Result ; };
struct TypePromotionTraits<double,double> { typedef double Result ; };
struct TypePromotionTraits<double,float> { typedef double Result ; };
struct TypePromotionTraits<double,long> { typedef double Result ; };
struct TypePromotionTraits<double,int> { typedef double Result ; };
struct TypePromotionTraits<double,short> { typedef double Result ; };
struct TypePromotionTraits<double,char> { typedef double Result ; };
struct TypePromotionTraits<double,bool> { typedef double Result ; };
struct TypePromotionTraits<float,double> { typedef double Result ; };
struct TypePromotionTraits<float,float> { typedef float Result ; };
struct TypePromotionTraits<float,long> { typedef float Result ; };
struct TypePromotionTraits<float,int> { typedef float Result ; };
struct TypePromotionTraits<float,short> { typedef float Result ; };
struct TypePromotionTraits<float,char> { typedef float Result ; };
struct TypePromotionTraits<float,bool> { typedef float Result ; };
struct TypePromotionTraits<long,double> { typedef double Result ; };
struct TypePromotionTraits<long,float> { typedef float Result ; };
struct TypePromotionTraits<long,long> { typedef long Result ; };
struct TypePromotionTraits<long,int> { typedef long Result ; };
struct TypePromotionTraits<long,short> { typedef long Result ; };
struct TypePromotionTraits<long,char> { typedef long Result ; };
struct TypePromotionTraits<long,bool> { typedef long Result ; };
struct TypePromotionTraits<int,double> { typedef double Result ; };
struct TypePromotionTraits<int,float> { typedef float Result ; };
struct TypePromotionTraits<int,long> { typedef long Result ; };
struct TypePromotionTraits<int,int> { typedef int Result ; };
struct TypePromotionTraits<int,short> { typedef int Result ; };
struct TypePromotionTraits<int,char> { typedef int Result ; };
struct TypePromotionTraits<int,bool> { typedef int Result ; };
struct TypePromotionTraits<short,double> { typedef double Result ; };
struct TypePromotionTraits<short,float> { typedef float Result ; };
struct TypePromotionTraits<short,long> { typedef long Result ; };
struct TypePromotionTraits<short,int> { typedef int Result ; };
struct TypePromotionTraits<short,short> { typedef int Result ; };
struct TypePromotionTraits<short,char> { typedef int Result ; };
struct TypePromotionTraits<short,bool> { typedef int Result ; };
struct TypePromotionTraits<char,double> { typedef double Result ; };
struct TypePromotionTraits<char,float> { typedef float Result ; };
struct TypePromotionTraits<char,long> { typedef long Result ; };
struct TypePromotionTraits<char,int> { typedef int Result ; };
struct TypePromotionTraits<char,short> { typedef int Result ; };
struct TypePromotionTraits<char,char> { typedef int Result ; };
struct TypePromotionTraits<char,bool> { typedef int Result ; };
struct TypePromotionTraits<bool,double> { typedef double Result ; };
struct TypePromotionTraits<bool,float> { typedef float Result ; };
struct TypePromotionTraits<bool,long> { typedef long Result ; };
struct TypePromotionTraits<bool,int> { typedef int Result ; };
struct TypePromotionTraits<bool,short> { typedef int Result ; };
struct TypePromotionTraits<bool,char> { typedef int Result ; };
struct TypePromotionTraits<bool,bool> { typedef int Result ; };



} // namespace frp 

# endif //_LAMBDA_H_

