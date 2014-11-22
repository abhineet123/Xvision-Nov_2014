// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGEITERATOR_H_
#define _XVIMAGEITERATOR_H_

#include <XVImageBase.h>
#include <XVException.h>

class XVIteratorException : public XVException { public: XVIteratorException() : XVException() {};
    XVIteratorException(char * err) : XVException(err) {} }; 

/**
 * XVImageIterator is a class which encapsulates the pointer logic for
 * iterating across images. 
 *
 * Definition of Iterators for Images. These Iterators are a way of 
 * encapsulating the pointer arithmetic logic required for moving across
 * an image using normal operations of ++, and easy to understand for
 * and while loops.
 *
 * For maximum performance pointers are faster, but the timings on 
 * these iterators using box filters, etc. have been about double 
 * the values of straight pointer arithmetic. 
 *
 * To iterate across an image img of type Tx:
 * XVImageIterator<Tx> imgIter(img);
 * 
 * for (; imgIter.end() == false ; ++imgIter) {
 *     code
 * }
 *
 */
template <class PIXTYPE, class PIXIMTYPE>
class XVIterator : public XVImageGeneric {

 protected:

  PIXTYPE * uleftptr; 
  PIXTYPE * ptr;
  PIXTYPE * endrowptr;
  PIXTYPE * bottomcrnr;

  int lineskip;
  int rowlength;

  virtual PIXTYPE * getDataPtr(XVImageBase<PIXIMTYPE> &) = 0;

 protected:
  void init_ptrs( const XVImageBase<PIXIMTYPE> & imref, 
		  int xsize, int ysize, int px = 0, int py = 0 ) {
    rowlength = imref.Width() + imref.Skip();
    uleftptr = this->getDataPtr(const_cast<XVImageBase<PIXIMTYPE> & >(imref)) + (rowlength * py) + px;
    ptr = uleftptr;
    endrowptr = ptr + xsize ;
    lineskip = rowlength - xsize ;
    bottomcrnr = uleftptr + (ysize * rowlength) - lineskip;
  }

public:
  
  inline XVIterator() : uleftptr(NULL), ptr(NULL), endrowptr(NULL), bottomcrnr(NULL) {}
  inline XVIterator(XVImageBase<PIXIMTYPE> & imref) : XVImageGeneric(imref) {}
  inline XVIterator(XVImageBase<PIXIMTYPE> & imref, int xsize, int ysize, 
		    const int px = 0, const int py = 0)
    : XVImageGeneric(XVSize(xsize, ysize), XVPosition((imref.PosX() + px), (imref.PosY() + py))){}

  /** 
   * Resets an iterator on the partial image.
   * It leaves the size of the window the same, moving it in the image
   * to a specified location (x,y) given by two parameters.
   */ 
  inline void reset () {
    ptr = uleftptr;
    endrowptr = ptr + width; 
  }
 
  /** 
   * Copy constructor.
   * Copies Iterators.
   */
  inline XVIterator(XVIterator<PIXTYPE, PIXIMTYPE> & iter)
    : XVImageGeneric(iter)  {

    uleftptr = iter.uleftptr;
    rowlength = iter.rowlength;
    ptr = iter.ptr;
    endrowptr = iter.endrowptr;
    bottomcrnr = iter.bottomcrnr;
    lineskip = iter.lineskip;
  }
  
  inline XVIterator(XVImageBase<PIXIMTYPE> & imref, const XVImageGeneric & xvg)
    : XVIterator<PIXTYPE, PIXIMTYPE>(imref, xvg.Width(), xvg.Height(), xvg.PosX(), xvg.PosY()) {
    
    ASSERT (xvg <= imref);
  }
  
  /** 
   * Subimage Constructor: XVSize and two integer defined subimage.
   * Will assert that your defined subimage is contained by the
   * image you wish to iterate across.
   */
  inline XVIterator(XVImageBase<PIXIMTYPE> & imref, const XVSize & xsize,
			 const XVPosition & xvp)
    : XVIterator<PIXTYPE, PIXIMTYPE>(imref, xsize.Width(), xsize.Height(), xvp.PosX(), xvp.PosY()){
    
    ASSERT ((*this)<= imref);
  }
  
  /** 
   * Current X Position.
   * Returns the current position of the iterator on the x-axis,
   * using the upper left corner as an origin.
   */
  inline int currposx() {
    ASSERT(ptr);
    return (((ptr - uleftptr) % rowlength));
  }
  
  /** 
   * Current Y Position.
   * Returns the current position of the iterator on the y-axis,
   * using the upper left corner as an origin.
   */
  inline int currposy() {
    ASSERT(ptr);
    return ((ptr - uleftptr) / rowlength);
  }

  /** 
   * Prefix Increment.
   * Moves the iterator to the next row position in the iterator using prefix syntax,
   * wrapping to the next row when necessary.
   * For Example: in a 2 by 3 iterator the sequence of coordinates sequenced by
   * iterator calls would be:
   *     (0,0) --> (1,0) --> (0,1) --> (1,1) --> (0,2) --> (1,2).
   */
  inline XVIterator<PIXTYPE, PIXIMTYPE> & operator ++ () {
    ASSERT(ptr);
    if (++ptr == endrowptr) {
      ptr+=lineskip, endrowptr += (rowlength);
    }
    return *this;
  }
  
  /** 
   * Postfix Increment.
   * Postfix operation involves two copies (creating an old state
   * version and executing the by-value return).
   */  	
  // I disabled this function since its design is so flawed that it 
  // cannot pass compiling anyway. -- Donald
  /*
  inline XVIterator<PIXTYPE, PIXIMTYPE> operator ++ (int) { //postfix increment
    ASSERT(ptr);
    XVIterator<PIXTYPE, PIXIMTYPE> copthis(*this);
    if (++ptr == endrowptr) {
      ptr+=lineskip, endrowptr += (rowlength);
    }
    ASSERT(ptr >= uleftptr && ptr <= bottomcrnr);
    return copthis;
  }
  */ 
  /**
   * Increment by a constant value.
   */
  inline XVIterator<PIXTYPE, PIXIMTYPE> & operator += (const int val) {
    ASSERT(ptr);
    ptr += val +  (val % width) * lineskip;
    ASSERT(ptr >= uleftptr && ptr <= bottomcrnr);
    return *this;
  };
  
  /** 
   * Are two iterators equal?.
   * Equality for iterators --> do they point to the same
   * position in the pixmap?
   */
  inline bool operator == (XVIterator<PIXTYPE, PIXIMTYPE> & iter) {
    ASSERT(ptr);
    return (ptr == iter.ptr);
  }
  
  /** 
   * Less than operator.
   * (i2 < i3) returns true if i2 points to a position in the pixmap
   * that is before i3s. In debugging, the function asserts that the 
   * two iterators point to the same image.
   */
  inline bool operator < (XVIterator<PIXTYPE, PIXIMTYPE> & iter) {
    ASSERT(ptr);
    ASSERT (uleftptr == iter.uleftptr);
    return (ptr < iter.ptr);
  }

  /** 
   * Less than operator.
   * (i2 > i3) returns true if i2 points to a position after i3s.
   * In debugging, the function asserts that both iterators i2 & 
   * i3 refer to the same image. 
   */
  inline bool operator > (XVIterator<PIXTYPE, PIXIMTYPE> & iter) {
    ASSERT(ptr);
    ASSERT (uleftptr == iter.uleftptr);
    return (ptr > iter.ptr);
  }
  
  /** 
   * Dereference for Read Iterators.
   * Returns a const reference to the actual entry in the pixmap
   * that the iterator points to.
   */
  inline PIXTYPE & operator * () {
    ASSERT(ptr);
    return *ptr;
  }

  /** 
   * Pointer to Member for Read Iterators.
   * Returns current iterating pointer, as const.
   */
  inline PIXTYPE * operator->() {
    ASSERT(ptr);
    return ptr;
  }  
  
  /** 
   * At or after the end of the iterator?.
   * Returns true if the ptr is past the end of the set size 
   * area of the image. 
   */
  inline bool end() {
    ASSERT(ptr);
    return (ptr >= bottomcrnr);
  }
  
  /** 
   * At or before the end of the iterator?
   * returns true if the ptr is located before the upper left
   * corner of the image.
   */
  inline bool beginning() {
    ASSERT(ptr);
    return (ptr <= uleftptr);
  }

  /** 
   * Move: Move the iterator to a position in its defined window.
   * Moves the iterator by an amount (x,y) given as two parameters.
   * For example to move to the beginning of the image, the call would be:
   *        iter.move (-iter.currposx(),-iter.currposy());
   */
  inline void moveBy(signed int diffxpos, signed int diffypos) { 
    ASSERT(ptr);
    ptr += (rowlength)*diffypos + diffxpos; 
    endrowptr = endrowptr + (diffypos*rowlength);
    ASSERT(ptr >= uleftptr && ptr <= bottomcrnr);
  }
  
  inline void moveBy(const XVPosition & diffPos){
    ASSERT(ptr);
    ptr += (rowlength)*diffPos.PosY() + diffPos.PosX();
    endrowptr = endrowptr + (diffPos.PosY()*rowlength);
    ASSERT(ptr >= uleftptr && ptr <= bottomcrnr);
  }

  /** 
   * Move: Move the iterator to a position in its defined window.
   * Moves the iterator to a position (x,y) defined by an XVPosition.
   * To move to the beginning of the image, use 
   * XVIterator<T> xvi(...)
   * {  
   *    // code
   *    xvi.move(xvi);
   * }
   * because the position an iterator represents is its upperleft-corner.
   */
  inline void move(const XVPosition & xvp) { 
    ASSERT(ptr);
    ASSERT(this->contains(xvp));
    ptr = uleftptr + (xvp.PosY() * rowlength) + xvp.PosX();
  }

  inline void move(int x, int y) { 
    ASSERT(ptr);
    ASSERT(this->contains(XVPosition(x, y)));
    ptr = uleftptr + (y * rowlength) + x;
  }
};

template <class PIXTYPE>
class XVImageIterator : virtual public XVIterator<const PIXTYPE, PIXTYPE> {
 protected:
  using XVIterator<const PIXTYPE, PIXTYPE>::uleftptr ;
  using XVIterator<const PIXTYPE, PIXTYPE>::ptr ;
  using XVIterator<const PIXTYPE, PIXTYPE>::endrowptr ;
  using XVIterator<const PIXTYPE, PIXTYPE>::bottomcrnr ;
  using XVIterator<const PIXTYPE, PIXTYPE>::lineskip ;
  using XVIterator<const PIXTYPE, PIXTYPE>::rowlength ;

 protected:

  virtual const PIXTYPE * getDataPtr(XVImageBase<PIXTYPE> & im) { return im.data(); }; 

 public:

  inline XVImageIterator(const XVImageBase<PIXTYPE> & imref) : XVIterator<const PIXTYPE, PIXTYPE>(const_cast<XVImageBase<PIXTYPE> & >(imref)) {
    uleftptr = this->getDataPtr(const_cast<XVImageBase<PIXTYPE> & >(imref));
    ptr = uleftptr;
    endrowptr = ptr + imref.Width();               
    lineskip = imref.Skip();
    rowlength = imref.Width() + lineskip;          
    bottomcrnr = uleftptr + (imref.Height() * rowlength) - lineskip;
  }
  

 public:
  inline XVImageIterator(const XVImageBase<PIXTYPE> & imref, int xsize, int ysize, 
			 const int px = 0, const int py = 0)
    : XVIterator<const PIXTYPE, PIXTYPE>(const_cast<XVImageBase<PIXTYPE> & >(imref), xsize, ysize, px, py) {
    init_ptrs( imref, xsize, ysize, px, py );
  }

  inline XVImageIterator(const XVImageBase<PIXTYPE> & imref, const XVImageGeneric & xvg) 
    : XVIterator<const PIXTYPE, PIXTYPE>(const_cast<XVImageBase<PIXTYPE> & >(imref), xvg.Width(), xvg.Height(), xvg.PosX(), xvg.PosY()) {
    init_ptrs( imref, xvg.Width(), xvg.Height(), xvg.PosX(), xvg.PosY() ); 
  }

  inline XVImageIterator(XVImageIterator<PIXTYPE> & iter) 
    : XVIterator<const PIXTYPE, PIXTYPE>(iter) {}

  inline XVImageIterator(const XVImageBase<PIXTYPE> & imref, const XVSize & xsize, const XVPosition & xvp)
    : XVIterator<const PIXTYPE, PIXTYPE>(const_cast<XVImageBase<PIXTYPE> & >(imref), xsize.Width(), xsize.Height(), xvp.PosX(), xvp.PosY()) {
    init_ptrs( imref, xsize.Width(), xsize.Height(), xvp.PosX(), xvp.PosY() );
  }
};

template <class PIXTYPE>
class XVImageWIterator : virtual public XVIterator<PIXTYPE, PIXTYPE> {
 protected:
  using XVIterator<PIXTYPE, PIXTYPE>::uleftptr ;
  using XVIterator<PIXTYPE, PIXTYPE>::ptr ;
  using XVIterator<PIXTYPE, PIXTYPE>::endrowptr ;
  using XVIterator<PIXTYPE, PIXTYPE>::bottomcrnr ;
  using XVIterator<PIXTYPE, PIXTYPE>::lineskip ;
  using XVIterator<PIXTYPE, PIXTYPE>::rowlength ;

 protected:
 
 // to save the image reference so we can unlock it later
  XVImageBase<PIXTYPE> & img ; 

  virtual PIXTYPE * getDataPtr(XVImageBase<PIXTYPE> & im) { return im.lock(); }; 

 public:

  inline XVImageWIterator(XVImageBase<PIXTYPE> & imref) : XVIterator<PIXTYPE, PIXTYPE>(imref), img(imref) {
    uleftptr = this->getDataPtr(imref);
    ptr = uleftptr;
    endrowptr = ptr + imref.Width();               
    lineskip = imref.Skip();
    rowlength = imref.Width() + lineskip;          
    bottomcrnr = uleftptr + (imref.Height() * rowlength) - lineskip;
  }


  inline XVImageWIterator(XVImageBase<PIXTYPE> & imref, int xsize, int ysize, 
			 const int px = 0, const int py = 0)
    : XVIterator<PIXTYPE, PIXTYPE>(imref, xsize, ysize, px, py), img(imref) {
    init_ptrs( imref, xsize, ysize, px, py );
  }

  inline XVImageWIterator(XVImageBase<PIXTYPE> & imref, const XVImageGeneric & xvg) 
    : XVIterator<PIXTYPE,PIXTYPE>(imref, xvg.Width(), xvg.Height(), xvg.PosX(), xvg.PosY()), img(imref) {     
    init_ptrs( imref, xvg.Width(), xvg.Height(), xvg.PosX(), xvg.PosY() ); 
  }

  inline XVImageWIterator(XVImageBase<PIXTYPE> & imref, const XVSize & xsize, const XVPosition & xvp)
    : XVIterator<PIXTYPE,PIXTYPE>(imref, xsize.Width(), xsize.Height(), xvp.PosX(), xvp.PosY()), img(imref) {    
    init_ptrs( imref, xsize.Width(), xsize.Height(), xvp.PosX(), xvp.PosY() ); 
  } 

  inline XVImageWIterator(XVImageWIterator<PIXTYPE> & iter) 
    : XVIterator<PIXTYPE, PIXTYPE>(iter), img(iter.img) {}

  inline void close(){ 
    img.unlock();
    uleftptr  = NULL; 
    ptr       = NULL;
    endrowptr = NULL;
    bottomcrnr = NULL;
  }
  ~XVImageWIterator() { close(); }
};


template <class PIXTYPE>
class XVRowIterator : virtual public XVImageIterator<PIXTYPE> {
 protected:
  using XVImageIterator<PIXTYPE>::uleftptr ;
  using XVImageIterator<PIXTYPE>::ptr ;
  using XVImageIterator<PIXTYPE>::endrowptr ;
  using XVImageIterator<PIXTYPE>::bottomcrnr ;
  using XVImageIterator<PIXTYPE>::lineskip ;
  using XVImageIterator<PIXTYPE>::rowlength ;

 protected:

  const PIXTYPE * beginrowptr;
  int row;
  int numRows;

 public:

  inline XVRowIterator(const XVImageBase<PIXTYPE> & imref, int r = 0) : XVImageIterator<PIXTYPE>(imref){
    
    ASSERT(r < imref.Height());
    row = r;
    numRows = imref.Height();
    beginrowptr = uleftptr + (rowlength * row);
    endrowptr   = beginrowptr + (rowlength - lineskip);
    ptr = beginrowptr;    
  }

  inline XVRowIterator<PIXTYPE> & operator ++ () {
    ++ptr;
    return *this;
  }
  
  inline XVRowIterator<PIXTYPE> operator ++ (int) {
    XVRowIterator<PIXTYPE> copthis(*this);
    ++ptr;
    return copthis;
  }
  
  inline XVRowIterator<PIXTYPE> & operator += (const int val) {
    ptr += val;
    return *this;
  }

  inline bool end() {
    return (ptr >= endrowptr);
  }
  
  inline bool beginning() {
    return (ptr <= beginrowptr);
  }

  inline XVRowIterator<PIXTYPE> & reset() {
    ptr = beginrowptr;
    return *this;
  }

  inline XVRowIterator<PIXTYPE> & reset(int r){
    //    ASSERT(r < numRows);
    row = r;
    beginrowptr = uleftptr + (rowlength * r);
    endrowptr   = beginrowptr + (rowlength - lineskip);
    ptr = beginrowptr;
    return *this;
  }
};

template <class PIXTYPE>
class XVColIterator : virtual public XVImageIterator<PIXTYPE> {
 protected:
  using XVImageIterator<PIXTYPE>::uleftptr ;
  using XVImageIterator<PIXTYPE>::ptr ;
  using XVImageIterator<PIXTYPE>::endrowptr ;
  using XVImageIterator<PIXTYPE>::bottomcrnr ;
  using XVImageIterator<PIXTYPE>::lineskip ;
  using XVImageIterator<PIXTYPE>::rowlength ;

 protected:

  const PIXTYPE * beginrowptr;
  int column;
  int numCols;

 public:

  inline XVColIterator(const XVImageBase<PIXTYPE> & imref, int c = 0) : XVImageIterator<PIXTYPE>(imref){
    
    ASSERT(c < imref.Height());
    column = c;
    numCols = imref.Height();
    beginrowptr = uleftptr + column;
    endrowptr   = uleftptr + numCols*rowlength;
    ptr = beginrowptr;    
  }

  inline XVColIterator<PIXTYPE> & operator ++ () {
    ptr+=rowlength;
    return *this;
  }
  
  inline XVColIterator<PIXTYPE> operator ++ (int) {
    XVColIterator<PIXTYPE> copthis(*this);
    ptr+=rowlength;
    return copthis;
  }
  
  inline XVColIterator<PIXTYPE> & operator += (const int val) {
    ptr += val*rowlength;
    return *this;
  }

  inline bool end() {
    return (ptr >= endrowptr);
  }
  
  inline bool beginning() {
    return (ptr <= beginrowptr);
  }

  inline XVColIterator<PIXTYPE> & reset() {
    ptr = beginrowptr;
    return *this;
  }

  inline XVColIterator<PIXTYPE> & reset(int c){
    //    ASSERT(c < numCols);
    column = c;
    beginrowptr = uleftptr + column;
    endrowptr   = uleftptr + numCols*rowlength;
    ptr = beginrowptr;    
    return *this;
  }
};

template <class PIXTYPE>
class XVRowWIterator : virtual public XVImageWIterator<PIXTYPE> {
 protected:
  using XVImageWIterator<PIXTYPE>::uleftptr ;
  using XVImageWIterator<PIXTYPE>::ptr ;
  using XVImageWIterator<PIXTYPE>::endrowptr ;
  using XVImageWIterator<PIXTYPE>::bottomcrnr ;
  using XVImageWIterator<PIXTYPE>::lineskip ;
  using XVImageWIterator<PIXTYPE>::rowlength ;

 protected:

  PIXTYPE * beginrowptr;
  int row;
  int numRows;

 public:

  inline XVRowWIterator(XVImageBase<PIXTYPE> & imref, int r = 0) : XVImageWIterator<PIXTYPE>(imref){
    
    ASSERT(r < imref.Height());
    row = r;
    numRows = imref.Height();
    beginrowptr = uleftptr + (rowlength * row);
    endrowptr   = beginrowptr + (rowlength - lineskip);
    ptr = beginrowptr;    
  }

  inline XVRowWIterator<PIXTYPE> & operator ++ () {
    ASSERT(ptr);
    ++ptr;
    return *this;
  }
  
  inline XVRowWIterator<PIXTYPE> operator ++ (int) {
    ASSERT(ptr);
    XVRowIterator<PIXTYPE> copthis(*this);
    ++ptr;
    return copthis;
  }
  
  inline XVRowWIterator<PIXTYPE> & operator += (const int val) {
    ASSERT(ptr);
    ptr += val;
    return *this;
  }

  inline bool end() {
    ASSERT(ptr);
    return (ptr >= endrowptr);
  }
  
  inline bool beginning() {
    ASSERT(ptr);
    return (ptr <= beginrowptr);
  }

  inline XVRowWIterator<PIXTYPE> & reset() {
    ASSERT(ptr);
    ptr = beginrowptr;
    return *this;
  }

  inline XVRowWIterator<PIXTYPE> & reset(int r){
    ASSERT(ptr);
    ASSERT(r < numRows);
    row = r;
    beginrowptr = uleftptr + (rowlength * r);
    endrowptr   = beginrowptr + (rowlength - lineskip);
    ptr = beginrowptr;
    return *this;
  }

  inline int currentRow(){ ASSERT(ptr); return row; }
  inline int index(){ ASSERT(ptr); return ptr - beginrowptr; }
};

#endif
