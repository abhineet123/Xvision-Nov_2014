#include <XVImageScalar.h>
#include <XVImageRGB.h>
#include <XVMaskedImage.h>


/** 
 * CONSTRUCTORS
 */
template <class IMTYPE>
XVMaskedImage<IMTYPE> :: XVMaskedImage(const int s_width, const int s_height, 
                         XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap) 
                      : XVImageScalar<u_char> (s_width,s_height)
{
  realimage = new IMTYPE (s_width,s_height,pixmap);
  maskimage = new IMTYPE (s_width,s_height);
}


template <class IMTYPE>
XVMaskedImage<IMTYPE> :: XVMaskedImage(const XVSize &s,
                         XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap) 
                      : XVImageScalar<u_char> (s.Width(),s.Height())
{
  realimage = new IMTYPE (s.Width(),s.Height(),pixmap);
  maskimage = new IMTYPE (s.Width(),s.Height());
}


template <class IMTYPE>
XVMaskedImage<IMTYPE> :: XVMaskedImage( 
                         XVPixmap<typename IMTYPE::PIXELTYPE> * pixmap) 
                      : XVImageScalar<u_char> ()
{
  realimage = new IMTYPE (pixmap);
  maskimage = new IMTYPE ();
}


template <class IMTYPE>
XVMaskedImage<IMTYPE> :: XVMaskedImage() 
                      : XVImageScalar<u_char> ()
{
  realimage = new IMTYPE ();
  maskimage = new IMTYPE ();
}



/**
 * intersect
 *
 * @do:  perform the intersection of the current realimage with 
 *       the mask
 */
//#define BINARY_OPTIMIZED_MASK  
#define MASK_USE_ITERATOR
template <class IMTYPE>
IMTYPE&  XVMaskedImage<IMTYPE> :: intersect() 
{

#ifdef MASK_USE_ITERATOR

  XVImageIterator<typename IMTYPE::PIXELTYPE> rit(*realimage);
  XVImageIterator<u_char> mit(*(reinterpret_cast< XVImageScalar<u_char> * > (this)));
  XVImageWIterator<typename IMTYPE::PIXELTYPE> wit(*maskimage);

  for(;wit.end()==false;++rit,++mit,++wit) {
  #ifdef BINARY_OPTIMIZED_MASK
    *wit = (*rit & *mit);
  #else
    if ( *mit != 0 ) 
      *wit = *rit;
    else 
      *wit = 0;
  #endif
  }

#else

  typename IMTYPE::PIXELTYPE * rit;
  typename IMTYPE::PIXELTYPE * wit;
  u_char* mit;
  int d = width*height;
  int i;

  rit = (typename IMTYPE::PIXELTYPE *)(realimage->data());
  mit = (u_char*)data();
  wit = maskimage->lock();
  
  for (i=0;i<d;i++,++wit,++rit,++mit) {
  #ifdef BINARY_OPTIMIZED_MASK
    *wit = (*rit & *mit);
  #else
    if ( *mit != 0 ) 
      *wit = *rit;
    else 
      *wit = 0;
  #endif
  }

  maskimage->unlock();

#endif

  return *maskimage;
}


/**
 *  reassign the real image pointer
 *   this does not free the original realimage
 */
template <class IMTYPE>
void  XVMaskedImage<IMTYPE> :: reassign_real(IMTYPE *new_real) 
{
  realimage = new_real;
}



template class XVMaskedImage< XVImageScalar < u_char > > ;
template class XVMaskedImage< XVImageScalar < u_short > > ;
template class XVMaskedImage< XVImageScalar < float > > ;

