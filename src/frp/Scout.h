# ifndef _FRP_SCOUT_H_
# define _FRP_SCOUT_H_

# include "Behavior.h"

namespace frp {

class ScoutRobot {
  int id ;
  bool inited ;
 public:
  ScoutRobot( int idRobot );
  ~ScoutRobot() ;
  void init(void);     // throw ScoutNoRobotException ;
  int try_init(void);  // return nono-zero for success
  void uninit(void);
};

extern ScoutRobot theScoutRobot ;

class ScoutNoRobotException {} ;

struct ScoutVel {  // wheel control on velocity
  double left ;
  double right ;
  ScoutVel() : left(0), right(0) {}
  ScoutVel( double v ) : left(v), right(v) {}
  ScoutVel( double l, double r ) : left(l), right(r) {}
  double getLeft(void) const { return left ; }
  double getRight(void) const { return right ; }

  ScoutVel operator + ( const ScoutVel& x ) const 
    { return ScoutVel(left+x.left,right+x.right); }
  ScoutVel operator - ( const ScoutVel& x ) const 
    { return ScoutVel(left-x.left,right-x.right); }
  ScoutVel operator - (void) const 
    { return ScoutVel(-left,-right); }
  ScoutVel operator * ( const ScoutVel& x ) const 
    { return ScoutVel(left*x.left,right*x.right); }
  ScoutVel operator / ( const ScoutVel& x ) const 
    { return ScoutVel(left/x.left,right/x.right); }
  ScoutVel operator * ( double x ) const 
    { return ScoutVel(left*x,right*x); }
  ScoutVel operator / ( double x ) const 
    { return ScoutVel(left/x,right/x); }
};

extern Lift1Combinator<int,ScoutVel,int (*)(const ScoutVel&)> scoutDrive ;
extern BehaviorRef<ScoutVel> scoutStop ;
extern Lift1Combinator<ScoutVel,double,ScoutVel (*)(double)> scoutForward ;

BehaviorRef<double> getScoutSonar(int n);

} // namespace frp 

# endif // _FRP_SCOUT_H_
