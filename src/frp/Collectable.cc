# include "Collectable.h"

# include <set>

namespace frp {
	
namespace {

class GarbageCollector {
 protected:
  typedef std::set<BasicCollectable*> Objects ;
  Objects heap, stack, removed ;
  void * last ;
  size_t length ;
  
  void commit();
  
 public:
  GarbageCollector();
  ~GarbageCollector();

  void* alloc( std::size_t size ) throw(std::bad_alloc);
  void free( void* ptr ) throw();
  BasicCollectable* enroll( BasicCollectable* ptr );
  void unroll( BasicCollectable* ptr );
  void collect() ;
};

GarbageCollector::GarbageCollector() : last(0), length(0) {
}

GarbageCollector::~GarbageCollector() {
  //cerr << "stack: " << stack.size() << " ; heap: " << heap.size() << endl;
}

void GarbageCollector::commit() {
  for( Objects::iterator i = removed.begin() ; i != removed.end() ; i ++ ) {
    heap.erase( *i );
  }
  removed.clear() ;
}

void* GarbageCollector::alloc( std::size_t size ) throw(std::bad_alloc) {
  try{
    void * ptr = new char[size];
    last = ptr ;
    length = size ;
    heap.insert( (BasicCollectable*)ptr );
    return ptr ;
  }catch( std::bad_alloc ) {
    last = 0 ;
    length = 0 ;
    throw ;
  }
}

void GarbageCollector::free( void* ptr ) throw() {
  removed.insert( (BasicCollectable*)ptr );
  if( last == ptr ) {
    last = 0 ;
    length = 0 ;
  }
  delete [] (char*)ptr ;
}

BasicCollectable* GarbageCollector::enroll( BasicCollectable* ptr ) {
  if( last && (char*)ptr >= (char*)last && (char*)ptr < (char*)last+length ) {
    return (BasicCollectable*) last ;
  }else {
    stack.insert( ptr );
    return 0 ;
  }
}

void GarbageCollector::unroll( BasicCollectable* ptr ) {
  stack.erase( ptr );
}

void GarbageCollector::collect(void) {
  Objects::iterator obj ;
  BasicCollectable * ptr ;
  commit();
  for( obj = stack.begin() ; obj != stack.end() ; ++ obj ) {
    (*obj)->gc_clear();
  }
  for( obj = heap.begin() ; obj != heap.end() ; ++ obj ) {
    (*obj)->gc_clear();
  }
  for( obj = stack.begin() ; obj != stack.end() ; ++ obj ) {
    (*obj)->gc_mark();
  }
  for( ptr = 0, obj = heap.begin() ; obj != heap.end() ; ptr = *obj, ++ obj ) {
    if( ptr && ! ptr->gc_check() ) {
      delete ptr ;
    }
  }
  if( ptr && ! ptr->gc_check() ) {
    delete ptr ;
  }
  commit();
}

GarbageCollector theCollector;

} // unnamed namespace

void collectGarbage(void) {
  theCollector.collect();
}

void* BasicCollectable::operator new( std::size_t size ) throw(std::bad_alloc){
  return theCollector.alloc(size);
}
void BasicCollectable::operator delete( void* ptr ) throw() {
  return theCollector.free(ptr);
}
BasicCollectable::BasicCollectable() {
  gc_host = theCollector.enroll(this) ;
}
BasicCollectable::BasicCollectable( const BasicCollectable& c ) {
  gc_host = theCollector.enroll(this) ;
}
BasicCollectable::BasicCollectable( BasicCollectable * host ) {
  if( ! ( gc_host = host ) ) {
    theCollector.enroll(this) ;
  }
}

BasicCollectable::~BasicCollectable() {
  if( ! gc_host ) { 
    theCollector.unroll(this);
  }
}

bool BasicCollectable::gc_check() { 
  return gc_marked ; 
}

void BasicCollectable::gc_mark() { 
  if( !gc_marked ) { 
    gc_marked = true ;
    if( gc_host ) {
      gc_host->gc_marked = true ;
    }
    gc_marking() ; 
  } 
}

void BasicCollectable::gc_clear() {
  gc_marked = false ;
}

void Collectable::gc_add( BasicCollectable * host ) {
  Collectable * ptr = dynamic_cast<Collectable *>(host) ;
  gc_prev = gc_next = gc_child = 0 ;
  if( ptr ) {
    if( ptr->gc_child ) {
      gc_next = ptr->gc_child ;
      ptr->gc_child->gc_prev = this ;
    }
    ptr->gc_child = this ;
  }
}

void Collectable::gc_remove( BasicCollectable * host ) {
  Collectable * ptr = dynamic_cast<Collectable *>(host) ;
  if( gc_prev ) {
    gc_prev->gc_next = gc_next ;
  }
  if( gc_next ) {
    gc_next->gc_prev = gc_prev ;
  }
  if( ptr && ptr->gc_child == this ) {
    if( gc_prev ) {
      ptr->gc_child = gc_prev ;
    }else {
      ptr->gc_child = gc_next ;
    }
  }
  gc_prev = gc_next = gc_child = 0 ;
}

Collectable::Collectable() : BasicCollectable() {
  gc_add( gc_host );
}
Collectable::Collectable( const Collectable& c ) : BasicCollectable(c) {
  gc_add( gc_host );
}
Collectable::Collectable( BasicCollectable * host ) : BasicCollectable(host) {
  gc_add( gc_host );
}

Collectable::~Collectable() {
  gc_remove( gc_host );
}

void Collectable::gc_marking() {
  if( gc_child ) {
    gc_child->gc_mark() ;
  }
  if( gc_next ) {
    gc_next->gc_mark() ;
  }
  gc_custom_marking(); 
}

} // namespace frp

