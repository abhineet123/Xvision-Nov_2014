# ifndef _FRP_COLLECTABLE_H_
# define _FRP_COLLECTABLE_H_

# include <memory>

namespace frp {
	
void collectGarbage(void);

/* Everything that is going to be garbage collected must be solely derived 
   from BasicCollectable. Since the garbage collector uses "mark-and-sweep",
   each piece of collectable object must be able to mark itself and all
   other collectable object it directly references (children).
*/

class BasicCollectable {
 private:
  bool gc_marked ;
 protected:
  BasicCollectable * gc_host ;
 public:
  BasicCollectable() ;
  BasicCollectable( const BasicCollectable& c ) ;
  explicit BasicCollectable( BasicCollectable * host ) ;
  virtual ~BasicCollectable() ;

  bool gc_check() ;
  void gc_mark() ;
  void gc_clear() ;

  void * operator new ( std::size_t size ) throw(std::bad_alloc);
  void operator delete( void* ptr ) throw() ;
  void * operator new ( std::size_t size, void * ptr ) throw() { return ptr ; }
  void operator delete( void* ptr, std::size_t size ) throw() {}

  virtual void gc_marking() {} // to call gc_mark() for all its children
};

/* Collectable implements the BasicCollectable interface.
   This is the base class which user-defined class can be derived from.
*/

class Collectable : public BasicCollectable {
 private:
  Collectable * gc_child, * gc_prev, * gc_next ;
  void gc_add( BasicCollectable * host );
  void gc_remove( BasicCollectable * hst );
 protected:
  void gc_marking() ;
 public:
  Collectable() ;
  Collectable( const Collectable& c ) ;
  explicit Collectable( BasicCollectable * host );
  ~Collectable() ;

  virtual void gc_custom_marking() {}
};

template<class T>
class CollectablePtr : public Collectable {
 protected:
  T * ptr ;

  void gc_custom_marking() { if(ptr) ptr->gc_mark() ; }
 public:
  CollectablePtr() : Collectable(), ptr(0) {}
  explicit CollectablePtr( BasicCollectable * host ) : 
    Collectable(host), ptr(0) {}
  CollectablePtr( T* p ) : Collectable(), ptr(p) {}
  CollectablePtr( T* p, BasicCollectable * host ) : Collectable(host), ptr(p){}
  CollectablePtr( const CollectablePtr<T>& p ) : Collectable(p), ptr(p.ptr) {}
  CollectablePtr( const CollectablePtr<T>& p, BasicCollectable * host ) : 
    Collectable(host), ptr(p.ptr) {}
  template<class P> 
  CollectablePtr( P* p ) : Collectable(), ptr(p) {}
  template<class P> 
  CollectablePtr( P* p, BasicCollectable * host ) : Collectable(), ptr(p.ptr){}
  template<class P>
  CollectablePtr( const CollectablePtr<P>& p ) : Collectable(p), ptr(p.ptr) {}
  template<class P>
  CollectablePtr( const CollectablePtr<P>& p, BasicCollectable * host ) : 
    Collectable(host), ptr(p.ptr) {}
  ~CollectablePtr() { ptr = 0 ; }

  T * operator ->() const { return ptr ; }
  T& operator *() const { return *ptr ; }
  const CollectablePtr<T>& operator = ( const CollectablePtr<T>& p )
    { ptr = p.ptr ; return *this ; }
  const CollectablePtr<T>& operator = ( T * p )
    { ptr = p ; return *this ; }
  template<class P>
  const CollectablePtr<T>& operator = ( const CollectablePtr<P>& p )
    { ptr = p.ptr ; return *this ; }
  template<class P>
  const CollectablePtr<T>& operator = ( P * p )
    { ptr = p ; return *this ; }
};

} // namespace frp

# endif // _FRP_COLLECTABLE_H_
