# include <stdio.h>
# include "Behavior.h"
# include "Event.h"
# include "Task.h"

# include <iostream>

using namespace std ;
using namespace frp ;

int next(void) {
  static int n = 0 ;
  return n ++ ;
}
int inc( int x ) {
  return x+1 ;
}
int add( int x, int y ) {
  return x+y ;
}
int add3( int x, int y, int z ) {
  return x+y+z ;
}
int add4( int x, int y, int z, int w ) {
  return x+y+z+w ;
}

bool test( int x ) {
  return x >= 0 ;
}

double one(void) {
  return 1.5 ;
}

TaskRef<int,double> triple( double x ) {
  return mkTask( constB((int)(x*3)), neverE<double>() ) ;
}

struct A {
  int a ;
  A( int a ) : a(a) {}
  int f() { return a++ ; }
  int g( float x ) { return (int)x + a++ ; }
  int k( int x, int y ) { return x+y+a++ ; }
};

BehaviorRef<int> fire( int n, Behavior<int> s ) {
  Behavior<int> x ;
  // x = ( delayB(1) <<= x ) + ( delayB(0) <<= delayB(0) <<= x ) ;
  x = ( delayB(0) <<= x ) + (n-1) ;
  return x ;
}


int mainx() {
  Behavior<int> y ;
  Behavior<int> x ;
  Behavior<int> z(3) ;
  Task<int,void> t ;
  Behavior<int> s = 10 ;

  LambdaVar<int>::X w ;
  //z = whenE( x >= 5 );
  x = (delayB(0)( x )) + (delayB(0)( delayB(1)( x ) )) ;
  //y = liftB(add4)(x)(x)(x)(x);
  //y = stepB( 0.0f )( castE<float,int>( constE(1) ) );
  //y = x TillB ( snapshotE( whenE( x >= 5 ), x ) 
  //              ThenB ( lambda( w, ctorF1<BehaviorRef<int>,int>()( w*2 )*z ) ) );
  t = switchT( mkTask(x,whileE(x==13)), 
	       whenE(x>=14) ThenConstB liftT(constB(10)) )
		 >> liftT(constB(20)) ;
  // t = mkTask(x,splitE( liftE(one)(whileE(x>=5)), whileE(x<=8) )) >> triple ;
  // t = ( mkTask(x,whenE(x>=5)) TillT whenE(x>=3) ) >> liftT(constB(1));
  //liftT(constB<int>(1)) ;
  y = t.behavior();
  //y = x TillB ( whenE( x >= 5 ) ThenConstB 1 ) 
  //         || ( whenE( x >= 3 ) ThenConstB (x+2) );

  // y = derivative( asin( sin( &integral<double> <<= x ) ) );
  // z = ( mkTask( y, whenE( y >= 10 ) ) TillB whenE( y >= 21 ) ) <<= x ;
  //z = ( mkTask( constB(1), whenE( y >= 3 ) ) >>  mkTask( y, whenE( y >= 10 ) ) ) <<= x ;
  // z = liftT(y) <<= x ;
  // z = ( y || x ) ;

  //Event<int> a ;
  //Event<float> b ;
  //b = liftE(inc) <<= a ;

  for( int i = 0 ; i < 10 ; i ++ ) {
    //cout << z.get() << "\t" << x.get() << endl;
    cout << y.getNext() << endl ;
  }
  /*
  Behavior<int> x, y ;
  x = fire() ;
  y = switchB ( x, snapshotE( whenE(delayB(0)(y)>4), y )  
  ThenB liftF(&fire).bind2(s) );
  
  Task<int,void> t ;
  t = mkTask( (x+100), whileE( x>=4 && x<8 || x>=16 ) ) 
    >> ( mkTask( (x+200), whileE( x>=8 && x<12 || x>=20 ) ) 
    >> ( mkTask( (x+300), whileE( x>=12 && x<16 ) ) >> t ) );
  
  Task<int,TaskT> t ;
  Event<TaskT> tasknet ;
  tasknet = whenE( x > 4 ) ThenConstB
   ( (TaskT)( mkTask( (x+100), whileE(x>7) ) >> t.getRef() ) );
  t = mkTaskLoopCast( x.getRef(), tasknet.getRef() );
  y = t.behavior() ;
  for( int i = 0 ; i < 25 ; i ++ ) {
    cout << y.getNext() << endl ;
  }
  */
  return 0 ;
}

int main() {
  mainx();
  collectGarbage();
  collectGarbage();
}
