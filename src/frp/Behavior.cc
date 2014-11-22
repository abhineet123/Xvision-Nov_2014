# include "Behavior.h"

# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
# include <math.h>

namespace frp {

// Time stuff

namespace {

Time getTime(void) {
  struct timeval t ;
  gettimeofday( &t, 0 );
  return toTime( t.tv_sec + t.tv_usec * 1e-6 ) ;
}

Time startTime = 0 ;

} // unnamed namespace

void timeReset(void) {
  startTime = getTime() ;
}

Time time(void) { 
  if( !startTime ) { 
    timeReset(); 
    return 0 ;
  }else {
    return getTime() - startTime ;
  }
}

namespace {

class Timer {
 protected:
  double nexttime ;
  double interval ;
  double endtime ; 
  bool forever ;
 public:
  Timer( double total, double inter );  // total < 0 -> forever
  bool next(void);
};

Timer::Timer( double total, double inter ) : nexttime(getTime()),
  interval(inter), endtime(nexttime+total), forever(total<0) {}

bool Timer::next(void) {
  double now = getTime() ;
  if( forever || now < endtime ) {
    struct timeval t ;
    fd_set zero ;
    FD_ZERO( &zero );
    double period = nexttime - now ;
    nexttime += interval ;
    if( !forever && nexttime > endtime ) {
      nexttime = endtime ;
    }
    if( period > 0 ) {
      t.tv_sec = (int)floor(period) ;
      t.tv_usec = (int)( (period - t.tv_sec)*1e6 + .5 ) ;
      select( 0, &zero, &zero, &zero, &t );
    }
    return true ;
  }else {
    return false ;
  }
}

} // unnamed namespace


// Behavior

void GenericBehavior::run( Time interval, Time totaltime, int gc_frames ) {
  Timer t( toSecond(totaltime), toSecond(interval) );
  int count = gc_frames ;
  while( t.next() ) { 
    next() ;
    if( gc_frames >= 0 && -- count <= 0 ) {
      count = gc_frames ;
      collectGarbage() ;
    }
  }
}

GenericBehavior::Waitings GenericBehavior::todo ;
Sample GenericBehavior::todo_now ;

void GenericBehavior::add_todo( GenericBehavior* b, Sample now ) {
  todo.push_back(b);
  todo_now = now ;
}

void GenericBehavior::clean_up() {
  for( Waitings::iterator i = todo.begin() ; i != todo.end() ; i ++ ) {
    (*i)->update( todo_now );
  }
  todo.clear();
}

BehaviorRef<Time> timeB = lift(time)() ;
BehaviorRef<Time> difTimeB = differential(timeB) ;

namespace {

class SampleModule : public BehaviorModule<Sample> {
public:
  void compute(void) { value = last ; }
};

SampleModule sample_module ;

} // unamed namespace 

BehaviorRef<Sample> sampleB( &sample_module );

} // namespace frp
