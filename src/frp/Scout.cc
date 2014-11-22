# include "Scout.h"
# include "Nclient.h"
# include <math.h>

namespace frp {

static const char * scoutName = "localhost" ;
static const int scoutPort = 7019 ;

static const int idScountRobot = 1 ;
static const int timeOut = 10 ;

static const double leftGain = 1.0 ;
static const double rightGain = 1.0 ;
static const double sonarGain = 1.0 ;

ScoutRobot theScoutRobot(idScountRobot);

ScoutRobot::ScoutRobot( int idRobot ) : id(idRobot), inited(false) {}

ScoutRobot::~ScoutRobot() { 
  uninit();
}

int ScoutRobot::try_init(void) {
  if( !inited ) {
    int r ;
    strcpy( SERVER_MACHINE_NAME, scoutName );
    SERV_TCP_PORT = scoutPort ;
    // create_robot( id );
    if( ( r = connect_robot(id) ) != 0 ) {
      conf_tm( timeOut );
      inited = true ;
    }
    return r ;
  }
}

void ScoutRobot::init(void) {
  if( ! try_init() ) {
    throw ScoutNoRobotException() ;
  }
}

void ScoutRobot::uninit(void) {
  if( inited ) {
    vm( 0, 0, 0 );
    disconnect_robot(id) ;
    inited = false ;
  }
}

static int scoutVM( const ScoutVel& v ) {
  theScoutRobot.init(); 
  return vm( (int)rint(v.right * rightGain), (int)rint(v.left * leftGain), 0 );
}

Lift1Combinator<int,ScoutVel,int (*)(const ScoutVel&)> scoutDrive(scoutVM);

static ScoutVel zeroVel ;

BehaviorRef<ScoutVel> scoutStop(zeroVel);

static ScoutVel velForward( double x ) {
  return ScoutVel(x,x);
}
Lift1Combinator<ScoutVel,double,ScoutVel (*)(double)> scoutForward(velForward);

static int scoutGS(void) {
  theScoutRobot.init(); 
  return gs();
}

BehaviorRef<int> scoutStates(new Lift0Module<int,int (*)(void)>(scoutGS));

class ScoutSonarState : public Functoid1<double,int> {
 protected:
  int f ;
 public:
  ScoutSonarState( int n ) : f(n) {}
  double operator ()( const int& dummy ) {
    return sonarGain*State[STATE_SONAR_0+f] ;
  }
};
 
BehaviorRef<double> getScoutSonar(int n) {
  return new Lift1Module<double,int,ScoutSonarState>
    (ScoutSonarState(n),scoutStates.get()) ;
}


} // namespace frp 
