// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGEIO_H_
#define _XVIMAGEIO_H_

// this has to be put in the very beginning otherwise png.h will
// complain about setjmp stuff that I don't understand why it does
#ifdef HAVE_LIBPNG
extern "C" {
#include <png.h>
}
#endif

#include <sys/types.h>
#include <XVMacros.h>

#include <iostream>
#include <XVList.h>
#include <XVTools.h>
#include <XVImageRGB.h>
#include <XVImageScalar.h>

using namespace std ;

/**
 * Reads an image into an XVImageRGB object.  This
 * function determines the type of file dynamically.
 * Takes only the filename as a parameter, and returns
 * false if it didn't work, and true if it did.
 */
template <class T>
int XVReadImage(XVImageRGB<T> &, char*);
  
/**
 * Reads an image of type <type> into an XVImageRGB object.
 * Takes the filename as a string, and the type as a string.
 */
template <class T>
int XVReadImage(XVImageRGB<T> &, char*, char*);

/**
 * Writes an XVImageRGB object out to an jpeg file.
 * Takes the filename as a parameter.
 */
template <class T>
int XVWriteImage(const XVImageRGB<T> &, char*) ;

/**
 * Writes an XVImageRGB object out to an image file
 * with the format <type>
 */
template <class T>
int XVWriteImage(const XVImageRGB<T> &, char*, char*) ;


/**
 * Writes an XVImageRGB object out to a raw PPM file.
 * Takes the filename as an argument, and returns
 * true if it worked.  
 *
 * NOTE: a raw PPM file's magic number is P6, not P3,
 *       and info is stored in binary not ascii.
 */
template <class T>
int XVWritePPM(const XVImageRGB<T> &, char *) ;

/**
 * Writes an XVImageRGB object out to a raw PPM file.
 * Takes the filename as an argument, and returns
 * true if it worked.  
 *
 * NOTE: a raw PPM file's magic number is P6, not P3,
 *       and info is stored in binary not ascii.
 */
template <class T>
int XVWritePPM(const XVImageYUV<T> &, char *) ;

/**
 * Reads an PPM file into an XVImageRGB object.
 * Takes the filename as a parameter, and returns
 * true if it worked.
 *
 * NOTE: a raw PPM file's magic number is P6, not P3,
 *       and info is stored in binary not ascii.
 */
template <class T>
int XVReadPPM(XVImageRGB<T> &, char *);


/**
 * Reads an PPM file into an XVImageRGB object.
 * Takes the filename as a parameter, and returns
 * true if it worked.
 *
 * NOTE: a raw PPM file's magic number is P6, not P3,
 *       and info is stored in binary not ascii.
 */
template <class T>
int XVReadPPM(XVImageYUV<T> &, char *);
#ifdef JPEG_LIB

/**
 * Reads a JPEG file into an XVImageRGB object.
 * returns true if it worked, false
 * otherwise.  Takes the filename as a parameter.
 */
template <class T>
int XVReadJPG(XVImageRGB<T> &, char* );

/**
 * Writes a JPEG file from an XVImageRGB object.
 * Takes the filename to be written as a parameter, 
 * and returns true if it worked, false if it didn't.
 */
template <class T>
int XVWriteJPG(const XVImageRGB<T> &, char *) ;

/**
 * Writes a JPEG file from an XVImageRGB object,
 * with the specified quality (lossy compression).  
 * Takes the filename to be written, and the 
 * quality to be written at as parameters.  The
 * quality can range from 0 to 100, 0 being the
 * poorest quality, and 100 be the best (no information
 * loss).  Returns true if it worked, false if it didn't.
 */
template <class T>
int XVWriteJPG(const XVImageRGB<T> &, char *, int) ;

#endif

#ifdef TIFF_LIB
/**
 * Reads a TIFF file to an XVImageRGB object.
 * Takes the filename as a param, and returns true
 * if it worked.
 */
template <class T>
int XVReadTIF(const XVImageRGB<T> &, char *); 

/**
 * Writes a TIFF file from an XVImageScalar object.
 * Takes the filename as a parameter.
 * (default doesn't compress)
 */
template <class T>
int XVWriteTIF(const XVImageRGB<T> &, char* ) ;

/**
 * Writes a TIFF file from an XVImageRGB object.
 * Takes the filename and compression as a parameter.
 * <compress> is a boolean true/false value, true if
 * you want to compress, false if you don't. Returns
 * true if it worked.
 */
template <class T>
int XVWriteTIF(const XVImageRGB<T> &, char *, int) ; 

#endif


#ifdef HAVE_LIBPNG


/**
 * Reads a PNG file to an XVImageRGB object.
 * Takes the filename as a param, and returns true
 * if it worked.
 */
template <class T>
int XVReadPNG(XVImageRGB<T> &, char *); 

template <class T>
int XVReadPNG(XVImageYUV<T> &, char *); 

/**
 * Writes a PNG file from an XVImageScalar object.
 * Takes the filename as a parameter.
 */
template <class T>
int XVWritePNG(const XVImageRGB<T> &, char* ) ;

//template <class T>
//int XVWritePNG(const XVImageYUV<T> &, char* ) ;

#endif


/**
 * Reads an image into an XVImageScalar object.  This
 * function determines the type of file dynamically.
 * Takes only the filename as a parameter, and returns
 * false if it didn't work, and true if it did.
 */
template <class T>
int XVReadImage(XVImageScalar<T> &, char* );
  
/**
 * Reads an image of type <type> into an XVImageScalar object.
 * Takes the filename as a string, and the type as a string.
 */
template <class T>
int XVReadImage(XVImageScalar<T> &, char* , char* );
  
/**
 * Writes an XVImageScalar object out to an jpeg file.
 * Takes the filename as a parameter.
 */
template <class T>
int XVWriteImage(const XVImageScalar<T> &, char* );

/**
 * Writes an XVImageScalar object out to an image file
 * with the format <type>
 */
template <class T>
int XVWriteImage(const XVImageScalar<T> &, char* , char* ) ;

/** 
 * Writes out to pgm format, taking the file name as argument.
 * Specifically, it creates a raw (not ascii) pgm file 
 * of type "P5".  Returns true if worked, false otherwise.
 */ 
template <class T>
int XVWritePGM (const XVImageScalar<T> &, char *) ;

/** 
 * Reads from pgm formatted file, taking the file name as 
 * argument. The pgm file has to be raw (not ascii)
 * of type "P5".
 */ 
template <class T>
int XVReadPGM(XVImageScalar<T> &, char *) ;

/** 
 * Write grayscale image into a BMP-format file
 */ 
template <class T>
int XVWriteBMP(XVImageScalar<T> &, char *) ;

/** 
 * Load  grayscale BMP image 
 */ 
template <class T>
int XVReadBMP(XVImageScalar<T> &, char *) ;

#ifdef JPEG_LIB

/**
 * Reads a JPEG file into an XVImageScalar object.
 * returns true if it worked, false
 * otherwise.  Takes the filename as a parameter.
 */
template <class T>
int XVReadJPG(XVImageScalar<T> &, char* );

/**
 * Writes a JPEG file from an XVImageScalar object.
 * Takes the filename to be written as a parameter, 
 * and returns true if it worked, false if it didn't.
 */
template <class T>
int XVWriteJPG(const XVImageScalar<T> &, char * ) ;

/**
 * Writes a JPEG file from an XVImageScalar object,
 * with the specified quality (lossy compression).  
 * Takes the filename to be written, and the 
 * quality to be written at as parameters.  The
 * quality can range from 0 to 100, 0 being the
 * poorest quality, and 100 be the best (no information
 * loss).  Returns true if it worked, false if it didn't.
 */
template <class T>
int XVWriteJPG(XVImageScalar<T> &, char * , int ) ;

#endif
#ifdef TIFF_LIB

/**
 * Reads a TIFF file to an XVImageScalar object.
 * Takes the filename as a param, and returns true
 * if it worked.
 */
template <class T>
int XVReadTIF(XVImageScalar<T> &, char *); 

/**
 * Writes a TIFF file from an XVImageScalar object.
 * Takes the filename as a parameter.
 * (default doesn't compress)
 */
template <class T>
int XVWriteTIF(const XVImageScalar<T> &, char *) ;

/**
 * Writes a TIFF file from an XVImageScalar object.
 * Takes the filename and compression as a parameter.
 * <compress> is a boolean true/false value, true if
 * you want to compress, false if you don't. Returns
 * true if it worked.
 */
template <class T>
int XVWriteTIF(const XVImageScalar<T> &, char *, int) ; 

#endif


#ifdef HAVE_LIBPNG
/**
 * Reads a PNG file to an XVImageScalar object.
 * Takes the filename as a param, and returns true
 * if it worked.
 */
template <class T>
int XVReadPNG(XVImageScalar<T> &, char *); 

/**
 * Writes a PNG file from an XVImageScalar object.
 * Takes the filename as a parameter.
 */
template <class T>
int XVWritePNG(const XVImageScalar<T> &, char *) ;

#endif


#include <XVImageIO.icc>

#endif
