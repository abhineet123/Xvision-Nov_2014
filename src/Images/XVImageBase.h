// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGEBASE_H_
#define _XVIMAGEBASE_H_

#include <sys/types.h>
#include <iostream>

#include <XVException.h>
#include <XVMacros.h>
#include <XVList.h>
#include <XVTools.h>
#include <XVPixel.h>
#include <XVGeometry.h>

using namespace std ;

/** exception class for XVImages */
class XVImageException : public XVException { public:
  XVImageException() : XVException((char*)"Image Exception") {}
  XVImageException(char * err) : XVException(err) {} };


/** 
 * A section of data. XVSize is the base class for anything that is 
 * a section of some data, that is a width and a height, or some number
 * of rows and columns. This is the base class for XVImageGeneric (from
 * which we derive XVImageBase and its class hierarchy), Video, and Matrix.
 */
class XVSize {

protected:

  int width;
  int height;

public:

  XVSize(const int cw, const int rh) { width = cw; height = rh;}
  XVSize() { width = 0; height = 0; }

  virtual ~XVSize() {}

  /** 
   * The Copy Constructor uses an item copy. 
   */
  XVSize(const XVSize & xsize) {
    width = xsize.width;
    height = xsize.height;
  }

  int Width() const {return width;}
  int Height() const {return height;}
  int Rows() const {return height;}
  int Columns() const {return width;}

  /** Resizes the Size class using an XVSize. */
  inline virtual void resize(const XVSize & xsize) {
    height = xsize.height;
    width = xsize.width;
  }

  /** Resizes the Size class, taking a width  and a height. */
  inline virtual void resize(int set_width, int set_height) {
    height = set_height;
    width = set_width;
  }

  /**Assignment operator. Works just like resize(XVSize &). */
  inline XVSize & operator = (const XVSize &xs) {
    height = xs.height;
    width = xs.width;
    return *this;
  }

  inline XVSize & operator += (const XVSize & s) {
    this->width += s.width;  this->height += s.height; return *this;
  };

  inline XVSize & operator -= (const XVSize & s) {
    this->width -= s.width;  this->height -= s.height; return *this;
  };

  inline XVSize & operator *= (int val) {
    this->width *= val; this->height *= val; return *this;
  };
};  


/**
 * The comparison operators for XVSize
 */

#define MAKE_COMPARE_OP_XVSIZE(OP) \
inline bool operator OP (const XVSize & s1, const XVSize & s2) { \
  return (s1.Width() OP s2.Width()) && (s1.Height() OP s2.Height()); \
}; 

MAKE_COMPARE_OP_XVSIZE(==);
MAKE_COMPARE_OP_XVSIZE(<=);
MAKE_COMPARE_OP_XVSIZE(>=);
MAKE_COMPARE_OP_XVSIZE(<);
MAKE_COMPARE_OP_XVSIZE(>);

/**
 * The binary operators for XVSize
 */

#define MAKE_XVSIZE_OP_XVSIZE(OP) \
inline XVSize operator OP (const XVSize & s1, const XVSize & s2) { \
  return XVSize(s1.Width() OP s2.Width(), s1.Height() OP s2.Height()); \
};

MAKE_XVSIZE_OP_XVSIZE(+);
MAKE_XVSIZE_OP_XVSIZE(-);

/**
 * The binary operators XVSize and int
 */

#define MAKE_XVSIZE_OP_INT(OP) \
inline XVSize operator OP (const XVSize & s1, const XVSize & s2) { \
  return XVSize(s1.Width() OP s2.Width(), s1.Height() OP s2.Height()); \
}; 

MAKE_XVSIZE_OP_INT(*);
MAKE_XVSIZE_OP_INT(/);

/**
 * The binary operators for XVPosition
 */
typedef XV2Vec<int> XVPosition;

template <class T> class XVImageBase;

/** 
 * Data class for the physical image data.
 * Pixmap is designed to hold the actual data of images.
 * All images contain pixmap pointers. All Image-derived
 * classes use pixmaps for data storage.
 */
template <class T>
class XVPixmap : public XVSize, public XVList {

  // all representations of XVImage need to be friends of Pixmap
  // to access and modify buffer specifications

  friend class XVImageBase<T>;
  
protected:
  
  /**
   * Each pixmap has the feature that it can
   * be locked, and only modified by one XVImage...
   * at a time.
   */
  int		locked;
  
  /**
   * a pointer to the image that locked the pixmap
   */
  XVImageBase<T> * master_image;
  
  /**
   * the actual data.
   */
  T * buffer_addr;
  
  /**
   * flag, if class should free the data after destruction
   * of the last client XVImage (default [yes]=1)
   */
  int own_flag;
  /** 
   * locks the pixmap, so that it can only be modified
   * by one image. Two different Images cannot have a lock
   * Always called from Images, using XVImageBase::lock() 
   * or any of XVImageBase's child classes. 
   */
  inline int lockPixmap(XVImageBase<T> * who){
    if(locked && who != master_image) return 0;
/*      if(this->contains(who)){ */
      master_image = who;
      return ++locked;
/*      }  */
/*      return 0; */
  }; 

  /** 
   * unlocks the pixmap.  Doing so removes the
   * master image from contolling this pixmap.
   * now any image pointing to this pixmap can
   * modify it.
   */
  inline bool unlockPixmap(XVImageBase<T> * who) {
    if(!locked || who != master_image) return false;
    --locked;
    if(!locked) master_image = NULL;
    return true;
  };

  inline int getNumLocks(XVImageBase<T> * who) { 
    return who == master_image ? locked : 0;
  };
  
public:  

  /**
   * an enumerated value which refers to the the type (pixel)
   * of the image.
   */
  XVImage_Type image_type;
  
  /** 
   * Constructor. Takes an optional width and a height, as well as
   * an optional source of the data buffer.
   */
  XVPixmap(int width = 640,int height = 480, T * s_buffer = NULL,
		  bool own_flag=true);
  
  /** 
   * Constructor. Takes a geometry (XVSize) and an optional buffer. 
   */
  XVPixmap(const XVSize & xsize, T * s_buffer = NULL, 
		  bool own_flag=true);

  int SizeX() const { return this->Width(); }
  int SizeY() const { return this->Height(); }

  ~XVPixmap();
};

/** 
 * Very basic definition (non-templated) of an image.
 * XVImageGeneric specifies the dimension aspects of a image,
 * but due to its a lack of templatization, it is suitable as 
 * the definition of a "window." If you need to use the data from
 * image, use XVImageBase.
 */
class XVImageGeneric: public XVSize, public XVPosition {

public:
  
  XVImageGeneric(const XVSize &xsize, const XVPosition &xvp)
    : XVSize(xsize), XVPosition(xvp) {}
  
  XVImageGeneric (const XVSize &xsize)
    : XVSize(xsize), XVPosition(0,0) {}
  
  XVImageGeneric(const int cw, const int rh, const int px = 0, const int py = 0)
    : XVSize(cw,rh), XVPosition(px,py) {}
  
  XVImageGeneric() : XVSize(), XVPosition() {}
  
  XVImageGeneric(const XVImageGeneric &xvg)
    : XVSize(xvg.width,xvg.height), XVPosition(xvg.posx, xvg.posy) {}
  
  /**
   * Checks if this region is completely bounded by another.
   * This region is within the outer region if this region's
   * boundary is within or on the boundary of the outer region.
   */
  inline bool within(const XVImageGeneric & outer) const {
    return (outer.PosX() < this->PosX()) && (outer.PosY() < this->PosY()) &&
      (outer.Width() + outer.PosX() > this->PosX() + this->Width()) &&
      (outer.Height() + outer.PosY() > this->PosY() + this->Height());
  };

  /**
   * checks if a point is contained by this region.
   * The point is contained by the region if the 
   * the point is inside or on the boundary of the 
   * region.
   */
  inline bool contains(const XVPosition & pos) const {
    return (pos.PosX() >= this->PosX() && pos.PosY() >= this->PosY() &&
	    pos.PosX() < this->PosX() + this->Width() &&
	    pos.PosY() < this->PosY() + this->Height());
  };      
};

#define MAKE_COMPARE_OP_XVIMAGEGEN(OP) \
inline bool operator OP (const XVImageGeneric & xv1, const XVImageGeneric& xv2) { \
  return ((XVSize)xv1) OP ((XVSize)xv2) && ((XVPosition)xv1) OP ((XVPosition)xv2); \
}; 

MAKE_COMPARE_OP_XVIMAGEGEN(==);
MAKE_COMPARE_OP_XVIMAGEGEN(!=);

#define MAKE_WITHIN_OP_XVIMAGEGEN(OP, INVOP) \
inline bool operator OP (const XVImageGeneric & g1, const XVImageGeneric & g2) { \
  return (g1.PosX() INVOP g2.PosX()) && (g1.PosY() INVOP g2.PosY()) && \
	 ((g1.Width() + g1.PosX()) OP (g2.PosX() + g2.Width()) && \
         ((g1.Height() + g1.PosY()) OP g2.PosY() + g2.Height())); \
};

MAKE_WITHIN_OP_XVIMAGEGEN(>=, <=);
MAKE_WITHIN_OP_XVIMAGEGEN(<=, >=);
MAKE_WITHIN_OP_XVIMAGEGEN(<, >);
MAKE_WITHIN_OP_XVIMAGEGEN(>, <);

#include <XVImageIterator.h>

/** 
 * Base templated class for images.
 * 
 * XVImageBase contains the basics of an image which is defined as a full 
 * or partial part of a pixmap upon which various operations can be performed.
 * Base contains the basic operations, such as equality, output, 
 * and dimensioning. 
 * A basic "subimage" is defined in XVImageGeneric, but only XVImageBase
 * is templated, thus making XVImageBase ideal as the base class when
 * any actions on the image data must take place.
 * <br>The locking mechanism requires that locks be released on a pixmap
 * before it will allow another image/iterators set to lock. That being
 * said, all write iterators lock on construction and unlock on
 * destruction. All write iterators based on an image must be destroyed
 * in order for all locks to release and the way opened for a new image
 * to lock.<br>
 * Locking Functions:<br>
 * XVPixmap::lock_pixmap().<br>
 * XVImageBase::lock(), and all derived classes.<br>
 * XVImageYUV::uv_lock().<br>
 * XVImageWIterator::XVImageWIterator().<br>
 * Unlocking Functions:<br>
 * XVPixmap::unlock_pixmap().<br>
 * XVImageBase::unlock(), and all derived classes.<br>
 * XVImageYUV::uv_unlock().<br>
 * XVImageWIterator::~XVImageWIterator().<br>
 * Sanity Check Functions:<br>
 * XVPixmap::get_num_locks().<br>
 */
template <class T>
class XVImageBase : public XVNode, public XVImageGeneric {  

 public:
  
  typedef T PIXELTYPE;
  
  friend class XVImageIterator<T>;
  
 protected:
  
  /**
   * Pointer. The address of the image in the pixmap.
   */
  T * win_addr;
  
  /**
   * The pixmap containing this image
   */
  XVPixmap<T> * pixmap;
  
  /**
   * The number of memory elements between 
   * each leftmost pixel of a given row
   */
  int lineskip;
  
  /**
   * Whether this image can modify the
   * underlying pixmap.
   */
  int write_perm;
  
public:
  
  /** 
   * Returns the width of the pixmap. 
   */
  inline int SizeX(void) const {return pixmap->width;};
  
  /** 
   * Returns the height of the pixmap. 
   */
  inline int SizeY(void) const {return pixmap->height;};
 
  /** 
   * Const Pointer.
   * Returns the beginning of the image in the pixmap (const).
   */
  const T * data() const { return win_addr; };
  
  /** 
   * Const Pointer. 
   * Returns the beginning of the underlying pixmap (const).
   */
  const T * pixData() const { return pixmap->buffer_addr; };

  /**
   * Integer.
   * Returns the lineskip value.
   */
  int Skip() const { return lineskip; }

  /** 
   * Returns the image data type.
   * Returns an XVImage_Type (i.e. is XV_RGB16). See
   * XVImage_Type.
   */
  virtual XVImage_Type ImageType() const {return pixmap->image_type;};
  
  /** 
   * Locks the pixmap for this image, until you call unlock.
   * Returns a pointer to the beginning of the image space in the
   * pixmap if it successfully locks, otherwise returns NULL.
   * See the general XVImageBase documentation for a description
   * of the locking mechanism.
   */
  T * lock(void) {
    write_perm = pixmap->lockPixmap(this);
    return write_perm ? win_addr : NULL; 
  };
  
  /** 
   * Unlocks the pixmap, making it available to be locked.
   */
  bool unlock (void) {    
    bool ret = pixmap->unlockPixmap(this);
    write_perm = pixmap->getNumLocks(this);
    return ret;
  };
  
  /** 
   * Sets the "window" that the image affects within the pixmap.
   * Takes integer parameters.
   */
  
  void reposition(const XVPosition & xvp) {
    posx = xvp.PosX();
    posy = xvp.PosY();
    win_addr = pixmap->buffer_addr + posy * pixmap->SizeX() + posx;
  }
  
  void reposition(const int px, const int py) {
    posx = px;
    posy = py;
    win_addr = pixmap->buffer_addr + posy * pixmap->SizeX() + posx;
  }
  
  /** 
   * Sets a new position for the image data.
   * Used primarily for grabbing a frame from a framebuffer.
   */
  void remap(T * addr,bool own_flag=true);
  
  /** 
   * Returns a pointer to the beginning of row r in the image. 
   */
  const T * operator [] (int r) const { return win_addr + r * pixmap->SizeX(); };
  
  /** 
   * Resizes the image based on new width and height for the image.
   * Takes integer parameters.
   */
  void virtual resize(const int set_width, const int set_height);
  
  /** 
   * Resizes the image based on the XVSize class.
   * For more information, see XVSize.
   */
  void virtual resize(const XVSize & xsize);
  
  /** 
   * Sets the window based on the XVSize and XVPosition classes.
   * For more information see XVSize and XVPosition.
   */
  void virtual setSubImage(const XVSize &, const XVPosition &);
  void virtual setSubImage(int, int, int, int);
  void virtual setSubImage(const XVImageGeneric &);


  /**
   * Sets the image back to the entire pixmap
   */
  inline virtual void setToPixmap(){ 
    setSubImage(0, 0, pixmap->SizeX(), pixmap->SizeY()); 
  }

  /** 
   * Constructs based on user parameters.
   * If pixmap is NULL, it creates a new one the same size as
   * the image.
   * 
   * PARAMTERS:
   *
   * int width  -  the width of the image
   * int height -  the height of the image
   * XVPixmap<T> * pixmap  - the pixmap to set the image to (defaults to NULL)
   */
  XVImageBase(const int, const int, XVPixmap<T> * pixmap = NULL);
  
  /** 
   * Constructs based on an XVSize class, 
   * other parameters as above. 
   *
   * PARAMETERS:
   *
   * XVSize & size  - reference to size of the new image
   * XVPixmap<T> *  - pointer to the pixmap to set this image to
   */
  XVImageBase(const XVSize &, XVPixmap<T> * pixmap = NULL);
  
  /** 
   * Constucts an image based on the pixmap passed in
   * and defined by the region of interest.
   * PARAMETERS:
   *
   * XVImageGeneric roi   -  the region of interest to set this image to
   * XVPixmap<T> * pixmap -  the pixmap to set this image to refer to
   */
  XVImageBase(const XVImageGeneric &, XVPixmap<T> * pixmap = NULL);
  
  /** 
   * Copy constructor. 
   */
  XVImageBase(const XVImageBase<T> &im);
  
  /** 
   * Constructor to cover the passed pixmap. 
   * The entire passed pixmap falls into the scope of the new
   * image.
   */
  XVImageBase(XVPixmap<T> * pixmap);
  XVImageBase();
  
  ~XVImageBase();

  /** 
   * WARNING: Please Read the Detailed Description. 
   * If SLOW_COPY is defined (not default), the method of assignment is a
   * pixel by pixel copy. While very safe, this takes time, and
   * is accordingly a slow copy. If SLOW_COPY is not defined (default)
   * the method of assignment is via "pointer-swinging": the pointer to 
   * Image A's pixmap is assigned and registered as the pixmap pointer 
   * for Image B, and the Image parameters (height, width, position, etc.)
   * are then copied. While much faster than the SLOW_COPY method, this may
   * may not be what you are expecting. After assignment, both images A and
   * B will modify the same data!
   */
  XVImageBase<T> & operator = (const XVImageBase<T> & im);

  /**
   * WARNING:  The following operator functions that
   * perform binary operations on images are to be 
   * used with caution.  The results one gets from
   * these functions may not be expected.  This is 
   * due to the limited range of the type values
   * of the pixels (u_char, int, XV_RGB15, etc).
   * If result of an addition or other operation
   * results in the pixel being outside the range
   * of the type, then the result is overlapped (started
   * from zero again).  This may not be the desired
   * result.
   */  
  // I don't know what does the above comments refer to -- Donald

};

#include "XVImageBase.icc"
#include "XVImageIterator.h"

#endif
