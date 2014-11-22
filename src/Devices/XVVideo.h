// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVVIDEO_H_
#define _XVVIDEO_H_

#include <stdlib.h>
#include "XVImageRGB.h"
#include "XVImageYUV.h"
#include "XVImageYCbCr.h"

typedef enum {MODE_CAP_SINGLE = 0, MODE_CAP_CONTINUOUS = 1}       XVCapture_Mode;
typedef enum {NORM_NTSC=0,NORM_PAL,NORM_SECAM, INPUT_DETERMINED}  XV_Video_Norms;
typedef enum {FULL_SIZE, HALF_SIZE, QUARTER_SIZE}                 XV_Resolution;

// XVSize is defined in XVImageBase.h

extern XVSize XVSize_AVI_full,
       XVSize_NTSC_full,
       XVSize_NTSC_half,
       XVSize_NTSC_quarter;

//
typedef struct {
    char	c;			// parameter specification e.g. B
    int  val;			// assigned value
} XVParser;

/** Base class for all Video Devices of the System
    Video is designed to provide a generic front end to video sources.  It also
    provide minimal buffer management (See VideoManager for a fully buffered
    video source).   Most devices are expected to look at the passed type
    of a pixel and make their function correspond to that type.  There
    is a means of passing character arguments to directly set device
    parameters.
*/

template <class IMTYPE>
class XVVideo {

protected:

    const char  	  *name;
    XVSize          size;

    // By default, it is assumed that any video device might use multiple
    // buffers (e.g. a ring-buffer), so video provides basic support for
    // managing them.

    IMTYPE * image_buffers;

    int  	          n_buffers;
    int             current_buffer;

    // This is the index of the most recently acquired frame.

    int             frame_count;
    XVCapture_Mode  capture_mode;

    // Default creation of buffers to be used in open

    IMTYPE * init_map(XVSize &s, int n_buffers);

    // Some local buffer management...by default this is just a ring queue

    virtual int next_buffer() {
        return (current_buffer = (++current_buffer%=n_buffers));
    };

    // parse through the param string, the resulting char * is
    // modified by each call
    //     return 1     - got parameters
    //     return 0     - end of file
    //     return -1    - error
    int   parse_param(char * &paramstring,XVParser &result);

    IMTYPE * frame_addr(int i) const {
        ASSERT(i < buffer_count());
        return image_buffers+i;
    }
    // describes if the buffers in XVImageRGB where created by Video
    int   own_buffers;

public:

    /* This is the generic declaration of a video device and is what
       the user would normally see when opening it.  The second
       parameter is not used, but is here for reference since
       most devices will probably want to pass a parameter string in
       to get some default device configuration. */

    XVVideo(const char *dev_name="/dev/video", const char *parm_string = NULL);

    virtual ~XVVideo();

    /** Functions to get at data */

    int		buffer_count(void) const {
        return n_buffers;
    };

    /** Returns the data in frame i */
    IMTYPE & frame(int i) const {
        return *(frame_addr(i));
    }

    /** Returns the data in currently active frame */
    IMTYPE & current_frame() const {
        return frame((current_buffer-1+n_buffers)%n_buffers);
    }

    /** Returns the index of the current buffer */
    int current_buffer_number() const {
        return current_buffer;
    };

    /** These are the basic functions to implement to acquire data.  The
        first schedules an acquisition; the second waits for its completion.
        Although available for "raw" use, most will probably just use
        either of the "next frame" functions below. */

    virtual int  initiate_acquire(int buffernum)=0;
    virtual int  wait_for_completion(int buffernum)=0;
    int          release_buffer(int buffernum) {
        return 0;
    };

    /** Continous grab where you just get the next frame down the line.
        If in polling mode, it schedules the first two frames and then
        waits for the first to complete.
    */

    int & request_frame_continuous(int framenum) {
        while (frame_count < framenum)
            next_frame_continuous();

        return frame_count;
    }

    IMTYPE & next_frame_continuous() {
        if (capture_mode != MODE_CAP_CONTINUOUS) {// Start the cycle
            capture_mode = MODE_CAP_CONTINUOUS;
            initiate_acquire(current_buffer);
        }
        int temp = current_buffer;
        initiate_acquire(next_buffer());
        wait_for_completion(temp);
        frame_count++;
        return frame(temp);
    }

    /** Poll until a specific frame arives and return the index of the
        buffer containing it. */

    int request_frame_poll(int framenum) {
        int next;

        if (capture_mode != MODE_CAP_SINGLE) {
            wait_for_completion(current_buffer);
            capture_mode = MODE_CAP_SINGLE;
            frame_count++;
        }
        while (framenum > frame_count) {
            initiate_acquire(next=next_buffer());
            wait_for_completion(next);
            frame_count++;
        }
        return (current_buffer);
    };

    const IMTYPE & next_frame_poll(void) {
        if (request_frame_poll(frame_count++))
            return current_frame();
    }

    //*** hardware device functions that need to be implemented for
    //*** any device

    // paramstring content
    // Bxx     - xx number of available buffers
    // I[0-2]  - input number
    // N[0-..] - norm XV_Video_Norms
    // S[1-2]  - full size, half size...
    virtual int  set_params(char *paramstring) = 0;
    // This allow remapping of video buffers --- for example the
    // buffering class manages it's own; many framegrabber remap memory, etc.

    IMTYPE * remap(typename IMTYPE::PIXELTYPE * mm_buf[],int n_buffers);

    // added by Donald: whether the last acquired image from device is
    // valid or not (not necessarily equivalent to the last image the
    // appilcation gets from a XVVideo derived object, but a good indicator)
    // not yet implemented in derived classes so don't rely on it

    virtual bool good(void) {
        return true ;
    }
};
#endif


















