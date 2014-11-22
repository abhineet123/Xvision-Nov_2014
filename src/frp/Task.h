# ifndef _FRP_TASK_H_
# define _FRP_TASK_H_

# include "Behavior.h"
# include "Event.h"

namespace frp {

// Task classes, similar with Behavior classes
// TaskBase -- abstract base
// TaskRef -- pointer to TaskBase
// Task -- pointer to TaskRef

template<class T, class Y>
class TaskBase : public BasicCollectable {
 public:
  virtual BehaviorRef<T> behavior(void) const = 0 ;
  virtual BehaviorRef<Maybe<Y> > event(void) const = 0 ;
  void run( Time interval = 0, Time totaltime = -1, 
            int gc_frames = GenericBehavior::gc_def ) 
    { behavior()->run( interval, totaltime, gc_frames ); }
};

template<class T, class Y>
class TaskRef : public BasicCollectable {
 protected:
  TaskBase<T,Y> * module ;
 public:
  TaskRef( TaskBase<T,Y> * m ) : module(m) {}

  void gc_marking() { if( module ) module->gc_mark() ; }

  operator TaskBase<T,Y> * (void) { return module ; }
  operator const TaskBase<T,Y> * (void) const { return module ; }
  TaskBase<T,Y> * get(void) { return module ; }
  const TaskBase<T,Y> * get(void) const { return module ; }
  const TaskBase<T,Y> * operator ->(void) const { return module ; }
  TaskBase<T,Y> * operator ->(void) { return module ; }
};

template<class T, class Y>
class Task : public TaskBase<T,Y> {
 protected:
  TaskRef<T,Y> * ref ;
 public:
  Task() : ref(new TaskRef<T,Y>((TaskBase<T,Y> *)(0))) {} ;

  void gc_marking() { if( ref ) ref->gc_mark() ; }

  BehaviorRef<T> behavior(void) const { return (*ref)->behavior(); }
  BehaviorRef<Maybe<Y> > event(void) const { return (*ref)->event(); }

  Task<T,Y>& operator = ( TaskRef<T,Y> x ) { *ref = x ; return *this ; }
  TaskRef<T,Y> getRef(void) const { return new Task<T,Y>(*this) ; }
  operator TaskRef<T,Y> (void) const { return getRef(); }
  operator void * (void) const { return (void *)getRef(); }
};

// TaskNode is the implementation of simple task (behavior+event)

template<class T, class Y>
class TaskNode : public TaskBase<T,Y> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<Y> > * e ;
 public:
  TaskNode( BehaviorBase<T> * x, BehaviorBase<Maybe<Y> > * y ) : p(x), e(y) {}
  void gc_marking() { p->gc_mark() ; e->gc_mark() ; }
  virtual BehaviorRef<T> behavior(void) const { return p ; }
  virtual BehaviorRef<Maybe<Y> > event(void) const { return e ; }
};

// Task functions

template<class T, class Y>
inline TaskRef<T,Y> mkTask( BehaviorRef<T> x, BehaviorRef<Maybe<Y> > y ) {
  return new TaskNode<T,Y>(x.get(),y.get());
}
template<class T, class Y>
inline TaskRef<T,Y> mkTask( const Behavior<T>& x, BehaviorRef<Maybe<Y> > y ) {
  return mkTask(x.getRef(),y);
}
template<class T, class Y>
inline TaskRef<T,Y> mkTask( BehaviorRef<T> x, const Event<Y>& y ) {
  return mkTask(x,y.getRef());
}
template<class T, class Y>
inline TaskRef<T,Y> mkTask( const Behavior<T>& x, const Event<Y>& y ) {
  return mkTask(x.getRef(),y.getRef());
}

template<class Y1, class Y2>
Maybe<Y2> FRP_force_maybe_cast( Maybe<Y1> x ) {
  return x.hasValue() ? Maybe<Y2>(Y2()) : Maybe<Y2>() ;
}
template<class T, class Y1, class Y2>
TaskRef<T,Y1> castTE( TaskRef<T,Y2> x ) {
  return mkTask<T,Y1>( x->behavior(),
		       liftB(&FRP_force_maybe_cast<Y1,Y2>)(x->event()) );
}

template<class T>
inline TaskRef<T,void> liftT( BehaviorRef<T> x ) {
  return mkTask(x,neverE<void>());
}
template<class T>
inline TaskRef<T,void> liftT( const Behavior<T>& x ) {
  return liftT(x.getRef());
}

// TaskBehaviorNode and TaskEventNode are used for TaskPair 

template<class T, class Y>
class TaskBehaviorModule : public BehaviorModule<T> {
 protected:
  bool alive ;
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<Y> > * e ;
  TaskBase<T,Y> * t ;
 public:
  TaskBehaviorModule( BehaviorBase<T>* x, BehaviorBase<Maybe<Y> >* y, 
		      TaskBase<T,Y>* z ) : 
    alive(true), p(x), e(y), t(z) {}
  void gc_marking() { p->gc_mark() ; if( alive ) e->gc_mark() ; t->gc_mark(); }
  void compute(void) {
    if( alive ) {
      e->update( last );
      if( e->get() ) {
	alive = false ;
	p = t->behavior().get() ;
	e = 0 ;
      }
    }
    p->update( last );
    value = p->get() ;
  }
};

template<class T, class Y>
class TaskEventModule : public BehaviorModule<Maybe<Y> > {
 protected:
  bool alive ;
  BehaviorBase<Maybe<Y> > * e ;
  TaskBase<T,Y> * t ;
 public:
  TaskEventModule( BehaviorBase<Maybe<Y> >* y, TaskBase<T,Y>* z ) :
    alive(true), e(y), t(z) {}
  void gc_marking() { e->gc_mark() ; t->gc_mark() ; }
  void compute(void) {
    if( alive ) {
      e->update( last );
      if( e->get() ) {
	alive = false ;
	e = t->event().get() ;
      }
    }
    e->update( last ) ;
    value = e->get() ;
  }
};

// TaskPair is used for sequenced tasks

template<class T, class Y>
class TaskPair : public TaskBase<T,Y> {
 protected:
  TaskBase<T,Y> * t ;
  TaskBase<T,Y> * n ;
 public:
  TaskPair( TaskBase<T,Y> * x, TaskBase<T,Y>* y ) : t(x), n(y) {}
  void gc_marking() { t->gc_mark() ; n->gc_mark() ; }
  
  BehaviorRef<T> behavior(void) const 
    { return new TaskBehaviorModule<T,Y>(t->behavior().get(),
					 t->event().get(),n); }
  BehaviorRef<Maybe<Y> > event(void) const 
    { return new TaskEventModule<T,Y>(t->event().get(),n); }
};

template<class T, class Y>
inline TaskRef<T,Y> operator >> ( TaskRef<T,Y> x, TaskRef<T,Y> y ) {
  return new TaskPair<T,Y>(x,y);
}
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( TaskRef<T,Y> x, const Task<T,Y>& y ) {
  return x >> y.getRef() ;
}

template<class T, class Y1, class Y2>
inline TaskRef<T,Y2> operator >> ( TaskRef<T,Y1> x, TaskRef<T,Y2> y ) {
  return new TaskPair<T,Y2>( castTE<T,Y2,Y1>(x), y );
}
template<class T, class Y1, class Y2>
inline TaskRef<T,Y2> operator >> ( TaskRef<T,Y1> x, const Task<T,Y2>& y ) {
  return x >> y.getRef();
}

template<class T1, class T2, class Y>
inline TaskRef<T2,Y> operator || ( TaskRef<T1,Y> x, TaskRef<T2,Y> y ) {
  return mkTask<T2,Y>( ( x->behavior(), y->behavior() ), 
		       ( x->event() || y->event() ) );
}

# define TillT TillB
# define ThenT ThenB
# define ThenConstT ThenConstB

template<class T, class Y>
inline TaskRef<T,Y> tillT( TaskRef<T,Y> x, BehaviorRef<Maybe<Y> > y ) {
  return mkTask<T,Y>( x->behavior(), ( x->event() || y ) );
}

template<class T, class Y>
inline TaskRef<T,Y> 
operator TillT( TaskRef<T,Y> x, BehaviorRef<Maybe<Y> > y ) {
  return tillT(x,y);
}

// switchT

template<class T, class Y>
class TaskSwitchBehaviorModule : public BehaviorModule<T> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<TaskRef<T,Y> > > * n ;
 public:
  TaskSwitchBehaviorModule( BehaviorBase<T> * x, 
			    BehaviorBase<Maybe<TaskRef<T,Y> > > * y )
    : p(x), n(y) {}
  void gc_marking() { p->gc_mark() ; n->gc_mark() ; }
  void compute(void) {
    n->update( last );
    if( n->get() ) {
      p = n->get().getValue()->behavior().get() ;
    }
    p->update( last );
    value = p->get() ;
  }
};

template<class T, class Y>
class TaskSwitchEventModule : public BehaviorModule<Maybe<Y> > {
 protected:
  BehaviorBase<Maybe<Y> > * e ;
  BehaviorBase<Maybe<TaskRef<T,Y> > > * n ;
 public:
  TaskSwitchEventModule( BehaviorBase<Maybe<Y> > * x, 
			 BehaviorBase<Maybe<TaskRef<T,Y> > > * y )
    : e(x), n(y) {}
  void gc_marking() { e->gc_mark() ; n->gc_mark() ; }
  void compute(void) {
    n->update( last );
    if( n->get() ) {
      e = n->get().getValue()->event().get() ;
    }
    e->update( last );
    value = e->get() ;
  }
};

template<class T, class Y>
class TaskSwitch : public TaskBase<T,Y> {
 protected:
  TaskBase<T,Y> * t ;
  BehaviorBase<Maybe<TaskRef<T,Y> > >* e ;
 public:
  TaskSwitch( TaskBase<T,Y> * x,  BehaviorBase<Maybe<TaskRef<T,Y> > >* y )
    : t(x), e(y) {}
  void gc_marking() { t->gc_mark() ; e->gc_mark() ; }
  BehaviorRef<T> behavior(void) const
    { return new TaskSwitchBehaviorModule<T,Y>( t->behavior().get(), e ); }
  BehaviorRef<Maybe<Y> > event(void) const
    { return new TaskSwitchEventModule<T,Y>( t->event().get(), e ); }
};

template<class T, class Y>
TaskRef<T,Y> switchT( TaskRef<T,Y> t, BehaviorRef<Maybe<TaskRef<T,Y> > > n ) {
  return new TaskSwitch<T,Y>( t.get(), n.get() );
}
template<class T, class Y>
TaskRef<T,Y> switchT( const Task<T,Y>& t, BehaviorRef<Maybe<TaskRef<T,Y> > > n ) {
  return switchT( t.getRef(), n );
}
template<class T, class Y>
TaskRef<T,Y> switchT( TaskRef<T,Y> t, const Behavior<Maybe<TaskRef<T,Y> > >& n ) {
  return switchT( t, n.getRef() );
}
template<class T, class Y>
TaskRef<T,Y> switchT( const Task<T,Y>& t, const Behavior<Maybe<TaskRef<T,Y> > >& n ) {
  return switchT( t.getRef(), n.getRef() );
}

// TaskLoop

template<class T, class Y, class Fn>
class TaskLoopModule : public BehaviorModule<T> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<Y> > * e, * n ;
  Fn fn ;
 public:
  TaskLoopModule( BehaviorBase<T>* x, BehaviorBase<Maybe<Y> >* y, 
		  BehaviorBase<Maybe<Y> >* z, const Fn& f ) : 
    p(x), e(y), n(z), fn(f) { attach(n); }
  void gc_marking() { p->gc_mark() ; e->gc_mark() ; n->gc_mark() ; }
  void compute(void) {
    Maybe<Y> y ;
    TaskBase<T,Y> * t ;
    bool once = true ;
    e->update( last );
    while( ( y = e->get() ) || once && ( once = false, y = n->get() ) ) {
      t = fn( y.getValue() );
      p = t->behavior().get() ;
      e = t->event().get() ;
      e->update( last ) ;
    }
    p->update(last) ;
    value = p->get() ;
  }
};

template<class T, class Y, class Fn>
class TaskLoopNode : public TaskBase<T,Y> {
 protected:
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<Y> > * e, * n ;
  Fn fn ;
 public:
  TaskLoopNode( BehaviorBase<T> * x, BehaviorBase<Maybe<Y> > * y, 
		BehaviorBase<Maybe<Y> >* z, const Fn& f ) : 
    p(x), e(y), n(z), fn(f) {}
  void gc_marking() { p->gc_mark() ; e->gc_mark() ; n->gc_mark() ; }
  virtual BehaviorRef<T> behavior(void) const 
    { return new TaskLoopModule<T,Y,Fn>(p,e,n,fn) ; }
  virtual BehaviorRef<Maybe<Y> > event(void) const { return neverE<Y>() ; }
};

// TaskLoop functions

template<class T, class Y, class Fn>
inline TaskRef<T,Y> 
mkTaskLoop( TaskRef<T,Y> x, BehaviorRef<Maybe<Y> > y, const Fn& fn ) {
  return new TaskLoopNode<T,Y,Fn>(x->behavior().get(),x->event().get(),
				  y.get(),fn) ;
}
template<class T, class Y, class Fn>
inline TaskRef<T,Y> 
mkTaskLoop( const Task<T,Y>& x, BehaviorRef<Maybe<Y> > y, const Fn& fn ) {
  return mkTaskLoop<T,Y,Fn>(x.getRef(),y,fn) ;
}
template<class T, class Y, class Fn>
inline TaskRef<T,Y> 
mkTaskLoop( TaskRef<T,Y> x, const Event<Y>& y, const Fn& fn ) {
  return mkTaskLoop<T,Y,Fn>(x,y.getRef(),fn) ;
}
template<class T, class Y, class Fn>
inline TaskRef<T,Y> 
mkTaskLoop( const Task<T,Y>& x, const Event<Y>& y, const Fn& fn ) {
  return mkTaskLoop<T,Y,Fn>(x.getRef(),y.getRef(),fn) ;
}

// TaskT<T> is to emulate TaskRef<T,TaskT<T> >

template<class T>
class TaskT {
 protected:
  void * ptr ;
 public:
  TaskT( TaskRef<T,TaskT<T> > x ) : ptr( x ) {}
  TaskT( const Task<T,TaskT<T> >& x ) : ptr( x.getRef() ) {}
  void gc_marking() { ((TaskBase<T,TaskT<T> >*)ptr)->gc_mark() ; }
  operator TaskBase<T,TaskT<T> > * () const 
    { return reinterpret_cast<TaskBase<T,TaskT<T> >*>(ptr) ; }
  operator TaskRef<T,TaskT<T> > () const 
    { return (TaskBase<T,TaskT<T> >*)ptr ; }
};

template<class T>
TaskBase<T,TaskT<T> > * FRP_TaskLoopCaster( TaskT<T> p ) {
  return (TaskBase<T,TaskT<T> > *)p ;
}
template<class T>
inline TaskRef<T,TaskT<T> > 
mkTaskLoopCast( TaskRef<T,TaskT<T> > x, BehaviorRef<Maybe<TaskT<T> > > y ){
  return mkTaskLoop<T,TaskT<T>,TaskBase<T,TaskT<T> > * (*)(TaskT<T> )>
    (x,y,&FRP_TaskLoopCaster<T>) ;
}
template<class T>
inline TaskRef<T,TaskT<T> > 
mkTaskLoopCast( const Task<T,TaskT<T> >& x, BehaviorRef<Maybe<TaskT<T> > > y ){
  return mkTaskLoopCast<T>(x.getRef(),y);
}
template<class T>
inline TaskRef<T,TaskT<T> > 
mkTaskLoopCast( TaskRef<T,TaskT<T> > x, const Event<TaskT<T> >& y ){
  return mkTaskLoopCast<T>(x,y.getRef());
}
template<class T>
inline TaskRef<T,TaskT<T> > 
mkTaskLoopCast( const Task<T,TaskT<T> >& x, const Event<TaskT<T> >& y ){
  return mkTaskLoopCast<T>(x.getRef(),y.getRef());
}
 
// Task >> Task (*)(Y)

template<class T, class Y, class Fn>
class TaskSeqCache : public BasicCollectable {
 protected:
  bool cached ;
  Fn fn ;
  TaskRef<T,Y> t ;
 public:
  TaskSeqCache( Fn f ) : cached(false), fn(f), t(0) {}
  void gc_marking() { if( cached ) t.gc_mark() ; }
  TaskRef<T,Y> operator() ( const Y& y ) 
    { if( !cached ) { cached = true ; t = fn(y) ; } return t ; }
};

template<class T, class Y, class Fn>
class TaskSeqBehaviorModule : public BehaviorModule<T> {
 protected:
  bool alive ;
  BehaviorBase<T> * p ;
  BehaviorBase<Maybe<Y> > * e ;
  TaskSeqCache<T,Y,Fn> * fn ;
 public:
  TaskSeqBehaviorModule( BehaviorBase<T>* x, BehaviorBase<Maybe<Y> >* y,
			 TaskSeqCache<T,Y,Fn>* z )
    : alive(true), p(x), e(y), fn(z) {}
  void gc_marking() { p->gc_mark() ; if( alive ) e->gc_mark() ; fn->gc_mark() ;}
  void compute(void) {
    if( alive ) {
      e->update( last );
      if( e->get() ) {
	alive = false ;
	p = (*fn)(e->get().getValue())->behavior().get() ;
      }
    }
    p->update( last );
    value = p->get() ;
  }
};

template<class T, class Y, class Fn>
class TaskSeqEventModule : public BehaviorModule<Maybe<Y> > {
 protected:
  bool alive ;
  BehaviorBase<Maybe<Y> > * e ;
  TaskSeqCache<T,Y,Fn> * fn ;
 public:
  TaskSeqEventModule( BehaviorBase<Maybe<Y> >* y, TaskSeqCache<T,Y,Fn>* z )
    : alive(true), e(y), fn(z) {}
  void gc_marking() { e->gc_mark() ; fn->gc_mark() ; }
  void compute(void) {
    if( alive ) {
      e->update( last );
      if( e->get() ) {
	alive = false ;
	e = (*fn)(e->get().getValue())->event().get();
      }
    }
    e->update( last );
    value = e->get() ;
  }
};

template<class T, class Y, class Fn>
class TaskSeq : public TaskBase<T,Y> {
 protected:
  TaskBase<T,Y> * t ;
  TaskSeqCache<T,Y,Fn> * fn ;
 public:
  TaskSeq( TaskBase<T,Y> * x, Fn f ) : t(x), fn(new TaskSeqCache<T,Y,Fn>(f)) {}
  void gc_marking() { t->gc_mark() ; fn->gc_mark() ; }
  
  BehaviorRef<T> behavior(void) const
    { return new TaskSeqBehaviorModule<T,Y,Fn>( t->behavior().get(),
						t->event().get(), fn ); }
  BehaviorRef<Maybe<Y> > event(void) const
    { return new TaskSeqEventModule<T,Y,Fn>( t->event().get(), fn ); }
};

template<class T, class Y>
inline TaskRef<T,Y> operator >> ( TaskRef<T,Y> x, TaskRef<T,Y> (*fn)( Y ) ) {
  return new TaskSeq<T,Y,TaskRef<T,Y> (*)(Y)>( x.get(), fn );
};
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( const Task<T,Y>& x, TaskRef<T,Y> (*fn)( Y ) ) {
  return x.getRef() >> fn ;
};
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( TaskRef<T,Y> x, TaskRef<T,Y> (*fn)( const Y& ) ) {
  return new TaskSeq<T,Y,TaskRef<T,Y> (*)(const Y&)>( x.get(), fn );
};
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( const Task<T,Y>& x, TaskRef<T,Y> (*fn)( const Y& ) ) {
  return x.getRef() >> fn ;
};
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( TaskRef<T,Y> x, Fun1<TaskRef<T,Y>,Y> fn ) {
  return new TaskSeq<T,Y,Fun1<TaskRef<T,Y>,Y> >( x.get(), fn );
};
template<class T, class Y>
inline TaskRef<T,Y> operator >> ( const Task<T,Y>& x, Fun1<TaskRef<T,Y>,Y> fn ) {
  return x.getRef() >> fn ;
};

} // namepsace frp

# endif // _FRP_TASK_H_
