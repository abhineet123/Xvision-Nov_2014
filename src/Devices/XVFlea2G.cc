#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include <config.h>
//#ifdef HAVE_DV  //removed to work with eclipse (elmar)

#include <XVDig1394.h>
#include <XVMacros.h>
#include <XVFlea2G.h>

//if Marlin
//#define FORMAT7_MODE DC1394_VIDEO_MODE_FORMAT7_1
//if Guppy
//#define FORMAT7_MODE DC1394_VIDEO_MODE_FORMAT7_0

using namespace std;

extern int debug;

namespace {
typedef enum {
	YUV444, YUV411, YUV422, RGB8, RGB16, MONO8, MONO16
} ModeDescr;

static struct {
	int width, height;
	ModeDescr mode;
} mode_descr[] = { { 160, 120, YUV444 }, { 320, 240, YUV422 }, { 640, 480,
		YUV411 }, { 640, 480, YUV422 }, { 640, 480, RGB8 },
		{ 640, 480, MONO8 }, { 640, 480, MONO16 }, { 800, 600, YUV422 }, { 800,
				600, RGB8 }, { 800, 600, MONO8 }, { 1024, 768, YUV422 }, {
				1024, 768, RGB8 }, { 1024, 768, MONO8 }, { 800, 600, MONO16 },
		{ 1024, 768, MONO16 }, { 1280, 960, YUV422 }, { 1280, 960, RGB8 }, {
				1280, 960, MONO8 }, { 1600, 1200, YUV422 },
		{ 1600, 1200, RGB8 }, { 1600, 1200, MONO8 }, { 1280, 960, MONO16 }, {
				1600, 1200, MONO16 } };

// manually initialize the YUB<->RGB conversion tables since we are going
// to use these tables directly.
void init_tab(void) {
	if (!bRGB2YUVTableBuilt) {
		buildRGB2YUVTable();
	}
	if (!bYUV2RGBTableBuilt) {
		buildYUV2RGBTable();
	}
}
; // init_tab

// helper class to help conversion functions
template<class IMTYPE>
struct Convert {
	typedef typename IMTYPE::PIXELTYPE PIXELTYPE;

	// convert a buffer of YUV444 image into an IMTYPE (assume XVImageRGB)
	// YUV8 4:4:4 format:  U Y V, so buffer size = # pixels * 3
	static
	void yuv444_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3;
		unsigned char pg;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 3, ++iTarg) {
			pg = TABLE_YUV_TO_PG[ptr[0]][ptr[2]];
			iTarg -> setR(TABLE_YUV_TO_R[ptr[1]][ptr[2]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[1]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[1]][pg]);
		}
	} // yuv444_f

	// convert a buffer of YUV422 image into an IMTYPE (assume XVImageRGB)
	// YUV8 4:2:2 format:  U Y1 V Y2, so buffer size = # pixels * / 2 * 4
	static
	void yuv422_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 2;
		unsigned char pg;
#ifdef HAVE_IPP
		IppiSize roi= {targ.Width(),targ.Height()};
		XVImageRGB<XV_RGB24> temp;
		const int dstOrder[4]= {2,1,0,3};
		switch(targ.ImageType())
		{
			case XVImage_RGB24:
			temp.resize(targ.Width(),targ.Height());
			ippiYCbCr422ToCbYCr422_8u_C2R((const Ipp8u *)ptr,targ.Width()*2,
					(Ipp8u*)ptr,targ.Width()*2,roi);
			ippiYCbCr422ToRGB_8u_C2C3R((const Ipp8u *)ptr,targ.Width()*2,
					(Ipp8u *)temp.lock(),
					temp.SizeX()*sizeof(XV_RGB24),roi);
			ippiSwapChannels_8u_C3R((const Ipp8u*)temp.data(),
					targ.Width()*sizeof(XV_RGB24),
					(Ipp8u *)targ.lock(),
					targ.SizeX()*sizeof(XV_RGB24),
					roi,dstOrder);
			targ.unlock();
			return;
			case XVImage_RGB32:
			temp.resize(targ.Width(),targ.Height());
			ippiYCbCr422ToCbYCr422_8u_C2R((const Ipp8u *)ptr,targ.Width()*2,
					(Ipp8u*)ptr,targ.Width()*2,roi);
			ippiYCbCr422ToRGB_8u_C2C3R((const Ipp8u *)ptr,targ.Width()*2,
					(Ipp8u *)temp.lock(),
					temp.SizeX()*sizeof(XV_RGB24),roi);
			temp.unlock();
			ippiSwapChannels_8u_C3C4R((const Ipp8u*)temp.data(),
					targ.Width()*sizeof(XV_RGB24),
					(Ipp8u *)targ.lock(),
					targ.SizeX()*sizeof(XV_RGBA32),
					roi,dstOrder,0);
			targ.unlock();
			return;
			default: // do it the old way?
			break;

		}
#endif
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 4) {
			pg = TABLE_YUV_TO_PG[ptr[0]][ptr[2]];
			iTarg -> setR(TABLE_YUV_TO_R[ptr[1]][ptr[2]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[1]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[1]][pg]);
			++iTarg;
			iTarg -> setR(TABLE_YUV_TO_R[ptr[3]][ptr[2]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[3]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[3]][pg]);
			++iTarg;
		}
	} // yuv422_f

	// convert a buffer of YUV411 image into an IMTYPE (assume XVImageRGB)
	// YUV8 4:1:1 format:  U Y1 Y2 V Y3 Y4, so buffer size = # pixels / 4 * 6
	static
	void yuv411_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3 / 2;
		unsigned char pg;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 6) {
			pg = TABLE_YUV_TO_PG[ptr[0]][ptr[3]];
			iTarg -> setR(TABLE_YUV_TO_R[ptr[1]][ptr[3]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[1]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[1]][pg]);
			++iTarg;
			iTarg -> setR(TABLE_YUV_TO_R[ptr[2]][ptr[3]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[2]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[2]][pg]);
			++iTarg;
			iTarg -> setR(TABLE_YUV_TO_R[ptr[4]][ptr[3]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[4]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[4]][pg]);
			++iTarg;
			iTarg -> setR(TABLE_YUV_TO_R[ptr[5]][ptr[3]]);
			iTarg -> setB(TABLE_YUV_TO_B[ptr[5]][ptr[0]]);
			iTarg -> setG(TABLE_YUV_TO_G[ptr[5]][pg]);
			++iTarg;
		}
	} // yuv411_f

	// convert a buffer of RGB image into an IMTYPE (assume XVImageRGB)
	// RGB8 format:  R G B, so buffer size = # pixels * 3
	static
	void rgb8_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3;
#ifdef HAVE_IPP
		const int dstOrder[4]= {2,1,0,3};
		IppiSize roi= {targ.Width(),targ.Height()};
		switch(targ.ImageType())
		{
			case XVImage_RGB24:
			ippiSwapChannels_8u_C3R((const Ipp8u*)ptr,
					targ.Width()*sizeof(XV_RGB24),
					(Ipp8u *)targ.lock(),
					targ.SizeX()*sizeof(XV_RGB24),
					roi,dstOrder);
			targ.unlock();
			return;
			case XVImage_RGB32:
			ippiSwapChannels_8u_C3C4R((const Ipp8u*)ptr,
					targ.Width()*sizeof(XV_RGB24),
					(Ipp8u *)targ.lock(),
					targ.SizeX()*sizeof(XV_RGBA32),
					roi,dstOrder,0);
			targ.unlock();
			return;
			default:
			break;
		}
#endif
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ++iTarg) {
			iTarg -> setR(*ptr++);
			iTarg -> setG(*ptr++);
			iTarg -> setB(*ptr++);
		}
	} // rgb8_f

	// convert a buffer of RGB image into an IMTYPE (assume XVImageRGB)
	// RGB16 format:  Rh Rl Gh Gl Bh Bl, so buffer size = # pixels * 6
	static
	void rgb16_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 6;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ++iTarg) {
			iTarg -> setR(*ptr);
			ptr += 2;
			iTarg -> setG(*ptr);
			ptr += 2;
			iTarg -> setB(*ptr);
			ptr += 2;
		}
	} // rgb16_f

	// convert a buffer of 8 bit mono image into an IMTYPE (assume XVImageRGB)
	static
	void mono8_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr++, ++iTarg) {
			iTarg -> setR(*ptr);
			iTarg -> setG(*ptr);
			iTarg -> setB(*ptr);
		}
	} // mono8_f

	// convert a buffer of 16 bit mono image into an IMTYPE (assume XVImageRGB)
	static
	void mono16_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 2;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 2, ++iTarg) {
			iTarg -> setR(*ptr);
			iTarg -> setG(*ptr);
			iTarg -> setB(*ptr);
		}
	} // mono16_f

}; // struct Convert

// partial specialization of Convert to deal with XVImageYUV
template<class PIXELTYPE>
struct Convert<XVImageYUV<PIXELTYPE> > {
	typedef XVImageYUV<PIXELTYPE> IMTYPE;

	// convert a buffer of YUV444 image into a XVImageYUV (independent Y,U and V)
	// YUV8 4:4:4 format:  U Y V, so buffer size = # pixels * 3
	static
	void yuv444_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ++iTarg) {
			iTarg -> setU(*ptr++);
			iTarg -> setY(*ptr++);
			iTarg -> setV(*ptr++);
		}
	} // yuv444_f<XVImageYUV>

	// convert a buffer of YUV422 image into a XVImageYUV (independent Y,U and V)
	// YUV8 4:2:2 format:  U Y1 V Y2, so buffer size = # pixels * / 2 * 4
	static
	void yuv422_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 2;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 4) {
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[1]);
			iTarg -> setV(ptr[2]);
			++iTarg;
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[3]);
			iTarg -> setV(ptr[2]);
			++iTarg;
		}
	} // yuv422_f<XVImageYUV>

	// convert a buffer of YUV411 image into a XVImageYUV (independent Y,U and V)
	// YUV8 4:1:1 format:  U Y1 Y2 V Y3 Y4, so buffer size = # pixels / 4 * 6
	static
	void yuv411_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3 / 2;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 6) {
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[1]);
			iTarg -> setV(ptr[3]);
			++iTarg;
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[2]);
			iTarg -> setV(ptr[3]);
			++iTarg;
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[4]);
			iTarg -> setV(ptr[3]);
			++iTarg;
			iTarg -> setU(ptr[0]);
			iTarg -> setY(ptr[5]);
			iTarg -> setV(ptr[3]);
			++iTarg;
		}
	} // yuv411_f<XVImageYUV>

	// convert a buffer of RGB image into a XVImageYUV (independent Y,U and V)
	// RGB8 format:  R G B, so buffer size = # pixels * 3
	static
	void rgb8_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 3;
		unsigned char py, y;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 3, ++iTarg) {
			py = TABLE_RGB_TO_PY[ptr[0]][ptr[1]];
			y = TABLE_RGB_TO_Y[py][ptr[2]];
			iTarg -> setU(TABLE_RGB_TO_U[ptr[2]][y]);
			iTarg -> setY(y);
			iTarg -> setV(TABLE_RGB_TO_U[ptr[0]][y]);
		}
	} // rgb8_f<XVImageYUV>

	// convert a buffer of RGB image into a XVImageYUV (independent Y,U and V)
	// RGB16 format:  Rh Rl Gh Gl Bh Bl, so buffer size = # pixels * 6
	static
	void rgb16_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 6;
		unsigned char py, y;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 6, ++iTarg) {
			py = TABLE_RGB_TO_PY[ptr[0]][ptr[2]];
			y = TABLE_RGB_TO_Y[py][ptr[4]];
			iTarg -> setU(TABLE_RGB_TO_U[ptr[4]][y]);
			iTarg -> setY(y);
			iTarg -> setV(TABLE_RGB_TO_U[ptr[0]][y]);
		}
	} // rgb16_f<XVImageYUV>

	// convert a buffer of 8 bit mono image into a XVImageYUV (i11t Y,U and V)
	static
	void mono8_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr++, ++iTarg) {
			iTarg -> setY(*ptr);
			iTarg -> setU(0);
			iTarg -> setV(0);
		}
	} // mono8_f<XVImageYUV>

	// convert a buffer of 16 bit mono image into a XVImageYUV (i11t Y,U and V)
	static
	void mono16_f(const unsigned char * buf, IMTYPE& targ, int numPixels) {
		const unsigned char * ptr = buf, *ptrEnd = buf + numPixels * 2;
		XVImageWIterator<PIXELTYPE> iTarg(targ);
		for (; ptr < ptrEnd; ptr += 2, ++iTarg) {
			iTarg -> setY(*ptr);
			iTarg -> setU(0);
			iTarg -> setV(0);
		}
	} // mono16_f<XVImageYUV>

}; // struct Convert<XVImageYUV>

enum Color {
	mono8, yuv411, yuv422, yuv444, rgb8, mono16, rgb16
};
const char * strColor[] = { "Mono8", "YUV8 4:1:1", "YUV8 4:2:2", "YUV8 4:4:4",
		"RGB8", "Mono16", "RGB16" };

// returns the correpsonding color-space conversion function
template<class T>
void (*color_f(Color c))( const unsigned char *, T&, int ) {
	switch (c) {
	case mono8:
		return &Convert<T>::mono8_f;
	case mono16:
		return &Convert<T>::mono16_f;
	case rgb8:
		return &Convert<T>::rgb8_f;
	case rgb16:
		return &Convert<T>::rgb16_f;
	case yuv411:
		return &Convert<T>::yuv411_f;
	case yuv422:
		return &Convert<T>::yuv422_f;
	case yuv444:
		return &Convert<T>::yuv444_f;
	default:
		return 0; // invalid or unimplemented
	}
}

		// returns the size of a line of pixels
		int getLineSize(int width, Color c) {
			switch (c) {
			case mono8:
				return width;
			case mono16:
				return width * 2;
			case rgb8:
				return width * 3;
			case rgb16:
				return width * 6;
			case yuv411:
				return width * 3 / 2;
			case yuv422:
				return width * 2;
			case yuv444:
				return width * 3;
			default:
				return 0; // invalid or unimplemented
			}
		}

		// returns the size of required buffer
		inline int getBufSize(int width, int height, Color c) {
			return getLineSize(width, c) * height;
		}

		const char * strFrameRates[] = { "1.875", "3.75", "7.5", "15", "30",
				"60" };

		}
		/* note: the logic of the code artificially limits the selection of
		 pixel color encoding and image size (scale) in format 7 into
		 those avaible in format 0-2 because these information
		 is still carried in format and mode data members from
		 set_params() to the class constructor.
		 */

		/* The following code is to deal with the the change of prototype of
		 dc1394_query_format7_total_bytes between different versions of
		 dc1394_control.h . The fourth parameter has been changed from
		 unsigned int * to unsigned long long int * .
		 Ideally this should be resolved in autoconf level instead of .cc level
		 however I (Donald) don't have time to investigate which version is which
		 so here is just a quick dirty fix using template partial specialization.
		 */

		template<class IMTYPE>
		int XVFlea2G<IMTYPE>::initiate_acquire(int i_frame) {
			if (i_frame < 0 || i_frame >= n_buffers)
				return 0;
			pthread_mutex_lock(&wait_grab[i_frame]);
			pthread_mutex_lock (&job_mutex);
			requests.push_front(i_frame);
			pthread_mutex_unlock (&job_mutex);
			return 1;
		}

		template<class PIXELTYPE>
		void BayerNearestNeighbor(unsigned char *src,
				XVImageRGB<PIXELTYPE> & targ, int sx, int sy,
				dc1394color_filter_t optical_filter) {
			XVImageRGB<XV_RGB24> tmp_img(sx, sy);
			dc1394_bayer_decoding_8bit((const uint8_t*) src,
					(uint8_t*) tmp_img.lock(), tmp_img.Width(),
					tmp_img.Height(), optical_filter,
					DC1394_BAYER_METHOD_NEAREST);
			tmp_img.unlock();
			PIXELTYPE *data = targ.lock();
			const XV_RGB24 *src_ptr = tmp_img.data();

			for (int y = 0; y < sy; y++)
				for (int x = 0; x < sx; x++, src_ptr++, data++)
					data->setG(src_ptr->G()), data->setR(src_ptr->R()), data->setB(
							src_ptr->B());

			targ.unlock();
		}

		void BayerNearestNeighbor_RGB24(unsigned char *src,
				XVImageRGB<XV_RGB24>& targ, int sx, int sy,
				dc1394color_filter_t optical_filter) {

			dc1394_bayer_decoding_8bit((const uint8_t*) src,
					(uint8_t*) targ.lock(), targ.Width(), targ.Height(),
					optical_filter, DC1394_BAYER_METHOD_NEAREST);
			targ.unlock();
		}

#ifdef HAVE_IPP
		void BayerNearestNeighbor(unsigned char *src,
				XVImageRGB<XV_RGBA32>& targ, int sx, int sy,dc1394color_filter_t
				optical_filter)
		{
			XVImageRGB<XV_RGB24> tmpimg(targ.Width(),targ.Height());
			BayerNearestNeighbor(src,tmpimg,sx,sy,optical_filter);
			IppiSize roi= {tmpimg.Width(),tmpimg.Height()};
			int dstOrder[4]= {0,1,2,3};
			ippiSwapChannels_8u_C3C4R((const Ipp8u*) tmpimg.data(),
					tmpimg.Width()*3,(Ipp8u*)targ.lock(),
					targ.Width()*sizeof(XV_RGBA32),roi,dstOrder,0);
			targ.unlock();
		}
#endif
		template<class PIXELTYPE>
		void BayerNearestNeighbor(unsigned char *src,
				XVImageYUV<PIXELTYPE>& targ, int sx, int sy,
				dc1394color_filter_t optical_filter) {
			cerr << "Bayer decoder does not support YUV" << endl;
		}
		;

		template<class IMTYPE>
		int XVFlea2G<IMTYPE>::wait_for_completion(int i_frame) {

			pthread_mutex_lock(&wait_grab[i_frame]);
			pthread_mutex_unlock(&wait_grab[i_frame]);
			return 1;
		}

		template<class IMTYPE>
		int XVFlea2G<IMTYPE>::set_params(char *paramstring) {

			XVParser parse_result;

			//bool direct = false;
			n_buffers = DIG_DEF_NUMFRAMES;
			while (parse_param(paramstring, parse_result) > 0) {
				switch (parse_result.c) {
				case 'B':
					n_buffers = parse_result.val;
					break;
				case 'R':
					rgb_pixel = parse_result.val;
					//direct = false;
					break;
				case 'V':
					verbose = (parse_result.val > 0 ? true : false);
					break;
				case 'S':
					scale = parse_result.val;// + 1;
					//direct = false;
					break;
				//case 'M':
				//	scale = parse_result.val;
				//	direct = false;
				//	break;
				case 'C':
					grab_center = parse_result.val;
					//if ((grab_center = parse_result.val)) {
					//	format7 = true;
					//}
					break;
				case 'T':
					if (parse_result.val > 0 && parse_result.val
							<= DC1394_TRIGGER_MODE_NUM) {
						ext_trigger = parse_result.val + DC1394_TRIGGER_MODE_0
								- 1;
					} else {
						ext_trigger = parse_result.val;
					}
					break;
				case 'X':
					roi_x=parse_result.val;
					break;
				case 'Y':
					roi_y=parse_result.val;
					break;
				case 'W':
					roi_width=parse_result.val;
					break;
				case 'H':
					roi_height=parse_result.val;
					break;
				case 'f':
					//if (parse_result.val == 7) {
					//	format7 = true;
					//} else {
						format = parse_result.val;
					//	direct = true;
					//}
					break;
				case 'm':
					mode = parse_result.val;
					//direct = true;
					break;
				case 'k':
					color = parse_result.val;
					break;
				case 'r':
					framerate = parse_result.val;
					break;
				case 'g': // gain of the camera
					gain = parse_result.val;
					break;
				case 'u': // u component for the white balance
					uv[0] = parse_result.val;
					break;
				case 'v': // v component for the white balance
					uv[1] = parse_result.val;
					break;
				case 'i': // reset bus
					reset_ieee = parse_result.val;
					break;
				case 's': // saturation
					saturation = parse_result.val;
					break;
				case 'A': // sharpness
					sharpness = parse_result.val;
					break;
				case 'h': //shutter
					shutter = parse_result.val;
					break;
				case 'a':
					gamma = parse_result.val;
					break;
				case 'x':
					exposure = parse_result.val;
					break;
				case 'o': // optical filter selection
					optical_filter = (dc1394color_filter_t)(parse_result.val
							+ DC1394_COLOR_FILTER_RGGB);
					optical_flag = true;
					break;
				default:
					if (verbose)
						cerr << parse_result.c << "=" << parse_result.val
								<< " is not supported by XVDig1394 (skipping)"
								<< endl;
					break;
				}
			}
			return 1;
		}

		template<class IMTYPE>
		void XVFlea2G<IMTYPE>::close(void) {
			if (camera_node) {
				dc1394_video_set_transmission(camera_node, DC1394_OFF);
				dc1394_capture_stop( camera_node);
				dc1394_camera_free(camera_node);
			}
			if (fd)
				dc1394_free( fd);
		}

		template<class IMTYPE> int XVFlea2G<IMTYPE>::open(const char *dev_name) {
			// first, empty DMA FiFo buffer

			dc1394_video_set_transmission(camera_node, DC1394_ON);
			return 1;
		}

		template<class IMTYPE>
		void *XVFlea2G<IMTYPE>::grab_thread(void *obj) {

			XVFlea2G<IMTYPE> *me = reinterpret_cast<XVFlea2G<IMTYPE>*> (obj);
			int i_frame;

			dc1394video_frame_t *cur_frame;
			usleep(200);
			while (!me->stop_flag) {
				dc1394_capture_dequeue(me->camera_node,
						DC1394_CAPTURE_POLICY_WAIT, &cur_frame);
				if (!me->requests.empty()) {
					i_frame = me->requests.back();
					pthread_mutex_lock(&(me->job_mutex));
					me->requests.pop_back();
					pthread_mutex_unlock(&(me->job_mutex));
					if (me->mode_type>=0/*me->format!=7*/) {
						switch (me->mode_type) {
						case RGB8:
							Convert<IMTYPE>::rgb8_f(cur_frame->image,
									me->frame(i_frame),
									me->frame(i_frame).Width() * me->frame(
											i_frame).Height());
							break;
						case YUV411:
							Convert<IMTYPE>::yuv411_f(cur_frame->image,
									me->frame(i_frame),
									me->frame(i_frame).Width() * me->frame(
											i_frame).Height());
							break;
						case YUV422:
							Convert<IMTYPE>::yuv422_f(cur_frame->image,
									me->frame(i_frame),
									me->frame(i_frame).Width() * me->frame(
											i_frame).Height());
							break;
						case YUV444:
							Convert<IMTYPE>::yuv444_f(cur_frame->image,
									me->frame(i_frame),
									me->frame(i_frame).Width() * me->frame(
											i_frame).Height());
							break;
						case MONO8:
							memcpy(me->frame(i_frame).lock(), cur_frame->image,
									(me->frame(i_frame).Width()) * (me->frame(
											i_frame).Height()));
							me->frame(i_frame).unlock();
							break;
//						case MONO8: //Firefly Hack by Elmar
//							BayerNearestNeighbor(cur_frame->image, me->frame(i_frame),
//									me->frame(i_frame).Width(),me->frame(i_frame).Height(),
//																me->optical_filter);
//							break;
						default:
							cerr << "unknown color format: " << me->mode_type << endl;
							break;
						}
					} else {
						if (me->optical_flag) {
							BayerNearestNeighbor(cur_frame->image, me->frame(
									i_frame), me->frame(i_frame).Width(),
									me->frame(i_frame).Height(),
									me->optical_filter);

						} else {
							memcpy(me->frame(i_frame).lock(), cur_frame->image,
									(me->frame(i_frame).Width()) * (me->frame(
											i_frame).Height()));
							me->frame(i_frame).unlock();
						}
					}
					pthread_mutex_unlock(&me->wait_grab[i_frame]);
				}
				dc1394_capture_enqueue(me->camera_node, cur_frame);

			}
			return NULL;
		}

		template<class IMTYPE>
		XVFlea2G<IMTYPE>::~XVFlea2G() {
			cerr << "(XVFlea2G::~XVFlea2G) Notice: destructor called.\n";
			static volatile bool first=true;
			if(first)
			{
				first=false;
				if (threadded) {
					stop_flag=true;
					pthread_join(grabber_thread, 0);
					pthread_detach(grabber_thread);
					for (int i = 0; i < n_buffers; i++)
						pthread_mutex_destroy(&wait_grab[i]);
					cerr << "(XVFlea2G::~XVFlea2G) Notice: closing interface.\n";
				}
				close();
				cerr << "(XVFlea2G::~XVFlea2G) Notice: interface closed.\n";
			}
			else
				cerr << "(XVFlea2G::~XVFlea2G) Notice: interface already at closing.\n";

			pthread_mutex_destroy( &job_mutex );
		}

		template<class IMTYPE>
		dc1394video_mode_t XVFlea2G<IMTYPE>::get_camera_mode()
		{
			if(rgb_pixel >= 0 || scale >= 0)
			{
				if(format >= 0 || mode >= 0)
				{
					cerr << "Parameter conflict: do not provide (format or mode) and (color or scale) information at the same time\n";
					throw(1);
				}

				if(rgb_pixel<0)
					rgb_pixel=1;
				if(scale<0)
					scale=0;

				switch(rgb_pixel)
				{
					case 0: //YUV422
						switch(scale)
						{
							case 0: //fall through
							case 2: //320x240
								return DC1394_VIDEO_MODE_320x240_YUV422;
							case 3: //640x480
								return DC1394_VIDEO_MODE_640x480_YUV422;
							case 4: //800x600
								return DC1394_VIDEO_MODE_800x600_YUV422;
							case 5: //1024x768
								return DC1394_VIDEO_MODE_1024x768_YUV422;
							case 6: //1280x960
								return DC1394_VIDEO_MODE_1280x960_YUV422;
							default:
								break;
						}
						break;
					case 1: //RGB
						switch(scale)
						{
							case 0: //fall through
							case 3: //640x480
								return DC1394_VIDEO_MODE_640x480_RGB8;
							case 4: //800x600
								return DC1394_VIDEO_MODE_800x600_RGB8;
							case 5: //1024x768
								return DC1394_VIDEO_MODE_1024x768_RGB8;
							case 6: //1280x960
								return DC1394_VIDEO_MODE_1280x960_RGB8;
							case 7: //1600x1200
								return DC1394_VIDEO_MODE_1600x1200_RGB8;
							default:
								break;
						}
						break;
					case 2: //MONO8
						switch(scale)
						{
							case 0: //fall through
							case 3: //640x480
								return DC1394_VIDEO_MODE_640x480_MONO8;
							case 4: //800x600
								return DC1394_VIDEO_MODE_800x600_MONO8;
							case 5: //1024x768
								return DC1394_VIDEO_MODE_1024x768_MONO8;
							case 6: //1280x960
								return DC1394_VIDEO_MODE_1280x960_MONO8;
							case 7: //1600x1200
								return DC1394_VIDEO_MODE_1600x1200_MONO8;
							default:
								break;
						}
						break;
					case 3: //MONO16
						switch(scale)
						{
							case 0: //fall through
							case 3: //640x480
								return DC1394_VIDEO_MODE_640x480_MONO16;
							case 4: //800x600
								return DC1394_VIDEO_MODE_800x600_MONO16;
							case 5: //1024x768
								return DC1394_VIDEO_MODE_1024x768_MONO16;
							case 6: //1280x960
								return DC1394_VIDEO_MODE_1280x960_MONO16;
							case 7: //1600x1200
								return DC1394_VIDEO_MODE_1600x1200_MONO16;
							default:
								break;
						}
						break;
					case 4: //YUV411
						switch(scale)
						{
							case 0: //fall through
							case 3: //640x480
								return DC1394_VIDEO_MODE_640x480_YUV411;
							default:
								break;
						}
						break;
					case 5: //YUV444
						switch(scale)
						{
							case 0: //fall through
							case 1: //160x120
								return DC1394_VIDEO_MODE_160x120_YUV444;
							default:
								break;
						}
						break;
				}
			}
			else
			{
				if(format<0)
					format=0;
				if(mode<0)
					mode=4;

				switch(format)
				{
					case 0:
						switch(mode)
						{
							case 0:
								return DC1394_VIDEO_MODE_160x120_YUV444;
							case 1:
								return DC1394_VIDEO_MODE_320x240_YUV422;
							case 2:
								return DC1394_VIDEO_MODE_640x480_YUV411;
							case 3:
								return DC1394_VIDEO_MODE_640x480_YUV422;
							case 4:
								return DC1394_VIDEO_MODE_640x480_RGB8;
							case 5:
								return DC1394_VIDEO_MODE_640x480_MONO8;
							case 6:
								return DC1394_VIDEO_MODE_640x480_MONO16;
							default:
								break;
						}
						break;
					case 1:
						switch(mode)
						{
							case 0:
								return DC1394_VIDEO_MODE_800x600_YUV422;
							case 1:
								return DC1394_VIDEO_MODE_800x600_RGB8;
							case 2:
								return DC1394_VIDEO_MODE_800x600_MONO8;
							case 3:
								return DC1394_VIDEO_MODE_1024x768_YUV422;
							case 4:
								return DC1394_VIDEO_MODE_1024x768_RGB8;
							case 5:
								return DC1394_VIDEO_MODE_1024x768_MONO8;
							case 6:
								return DC1394_VIDEO_MODE_800x600_MONO16;
							case 7:
								return DC1394_VIDEO_MODE_1024x768_MONO16;
							default:
								break;
						}
						break;
					case 2:
						switch(mode)
						{
							case 0:
								return DC1394_VIDEO_MODE_1280x960_YUV422;
							case 1:
								return DC1394_VIDEO_MODE_1280x960_RGB8;
							case 2:
								return DC1394_VIDEO_MODE_1280x960_MONO8;
							case 3:
								return DC1394_VIDEO_MODE_1600x1200_YUV422;
							case 4:
								return DC1394_VIDEO_MODE_1600x1200_RGB8;
							case 5:
								return DC1394_VIDEO_MODE_1600x1200_MONO8;
							case 6:
								return DC1394_VIDEO_MODE_1280x960_MONO16;
							case 7:
								return DC1394_VIDEO_MODE_1600x1200_MONO16;
							default:
								break;
						}
						break;
					case 7:
						switch(mode)
						{
							case 0:
								return DC1394_VIDEO_MODE_FORMAT7_0;
							case 1:
								return DC1394_VIDEO_MODE_FORMAT7_1;
							case 2:
								return DC1394_VIDEO_MODE_FORMAT7_2;
							case 3:
								return DC1394_VIDEO_MODE_FORMAT7_3;
							case 4:
								return DC1394_VIDEO_MODE_FORMAT7_4;
							case 5:
								return DC1394_VIDEO_MODE_FORMAT7_5;
							case 6:
								return DC1394_VIDEO_MODE_FORMAT7_6;
							case 7:
								return DC1394_VIDEO_MODE_FORMAT7_7;
							default:
								break;
						}
						break;

				}
			}
			cerr << "The desired mode does not exist\n";
			throw(2);
		}

		template<class IMTYPE>
		XVFlea2G<IMTYPE>::XVFlea2G(const char *dev_name,
				const char *parm_string, unsigned long long camera_id) :
			XVVideo<IMTYPE> (dev_name, parm_string) {

			dc1394camera_list_t * list;
			dc1394switch_t bOn;
			uint32_t num_cameras;
			dc1394camera_t **camera;
			dc1394video_modes_t modes;
			int mode_index;
			threadded = false;
			dc1394color_coding_t color_coding;

			// parsing parameters
			verbose = true;
			optical_flag = false;
			reset_ieee = false;
			format = -1;
			mode = -1;
			mode_type=-1;
			color = -1;
			framerate = -1;
			scale = -1;
			//format7 = false;
			grab_center = false;
			ext_trigger = 0;
			gain = -1;
			saturation = -1;
			uv[0] = uv[1] = -1;
			sharpness = -1;
			shutter = -1;
			gamma = -1;
			exposure = -1;
			camera_node = NULL;
			rgb_pixel = -1;
			roi_x=-1;
			roi_y=-1;
			roi_width=-1;
			roi_height=-1;
			if (parm_string)
				set_params((char *) parm_string);

			pthread_mutexattr_t job_attr ;
			pthread_mutexattr_init( &job_attr );
			//pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_ERRORCHECK_NP );
			pthread_mutex_init( &job_mutex, &job_attr );
			pthread_mutexattr_destroy( &job_attr );

			dc1394video_mode_t mode=get_camera_mode();

			if (fd = dc1394_new()) {
				if (dc1394_camera_enumerate(fd, &list) != DC1394_SUCCESS
						|| list->num == 0) {
					if (verbose)
						cerr << "Couldn't enumerate cameras" << endl;
					throw(1);
				}
			} else
				throw(2);
			num_cameras = list->num;
			if (!camera_id)
				camera_node = dc1394_camera_new(fd, list->ids[0].guid), channel
						= 0;
			else if (~camera_id < num_cameras)
				camera_node = dc1394_camera_new(fd, list->ids[~camera_id].guid), channel
						= ~camera_id;
			else
				for (int i = 0; i < num_cameras; i++) {
					if (verbose)
						cout << "Found camera ID 0x" << hex
								<< list->ids[i].guid << endl;
					if (camera_id && camera_id == list->ids[i].guid)
						camera_node = dc1394_camera_new(fd, list->ids[i].guid), channel
								= i;
				}
			dc1394_camera_free_list(list);
			if (!camera_node) {
				if (verbose)
					cerr << "Could not find camera id 0x" << hex << camera_id
							<< endl;
				throw(2);
			}
			if (verbose)
				dc1394_camera_print_info(camera_node, stdout);
re_init:
			if (reset_ieee)
				dc1394_camera_reset( camera_node);
			if (dc1394_video_get_transmission(camera_node, &bOn)
					!= DC1394_SUCCESS) {
				if (verbose)
					cerr << "XVision2: Unable to query transmission mode"
							<< endl;
				throw(3);
			}
			if (bOn == DC1394_ON)
				if (dc1394_video_set_transmission(camera_node, DC1394_OFF)
						!= DC1394_SUCCESS) {
					if (verbose)
						cerr << "XVision2: Unable to stop iso transmission"
								<< endl;
					throw(4);
				}
			if (dc1394_video_set_iso_speed(camera_node, DC1394_ISO_SPEED_400)
					!= DC1394_SUCCESS) {
				if (verbose)
					cerr << "Could not read iso speed" << endl;
				throw(5);
			}
			//if(format!=7) {
				dc1394_video_get_supported_modes(camera_node, &modes);
				bool valid_mode=false;
				for(int j=0; j<modes.num; j++)
				{
					if(modes.modes[j]==mode)
					{
						mode_index=j;
						valid_mode=true;
						break;
					}
				}
				if(!valid_mode)
				{
					cerr << "Mode is not supported\n";
					throw(9);
				}
				/*mode_index = modes.num - 1;
				if (rgb_pixel > -1) {
					if (rgb_pixel) {
						while (mode_index >= 0 && scale) {
							if (modes.modes[mode_index]
									< DC1394_VIDEO_MODE_FORMAT7_0
									&& mode_descr[modes.modes[mode_index]
											- DC1394_VIDEO_MODE_160x120_YUV444].mode
											== RGB8)
								scale--;
							if (scale)
								mode_index--;
						}
						if (mode_index < 0) {
							if (verbose)
								cerr << "Couldn't find valid RGB mode" << endl;
							throw(6);
						}
					} else {
						while (mode_index >= 0 && scale) {
							if (modes.modes[mode_index]
									< DC1394_VIDEO_MODE_FORMAT7_0
									&& mode_descr[modes.modes[mode_index]
											- DC1394_VIDEO_MODE_160x120_YUV444].mode
											== YUV422)
								scale--;
							if (scale)
								mode_index--;
						}
						if (mode_index < 0) {
							if (verbose)
								cerr << "Couldn't find valid YUV mode" << endl;
							throw(6);
						}
					}
				} else {
					int width = 0;
					while (mode_index >= 0 && scale) {
						if (modes.modes[mode_index]
								< DC1394_VIDEO_MODE_FORMAT7_0
								&& mode_descr[modes.modes[mode_index]
										- DC1394_VIDEO_MODE_160x120_YUV444].mode
										!= MONO8
								&& mode_descr[modes.modes[mode_index]
										- DC1394_VIDEO_MODE_160x120_YUV444].mode
										!= MONO16
								&& mode_descr[modes.modes[mode_index]
										- DC1394_VIDEO_MODE_160x120_YUV444].width
										!= width)
							scale--, width = mode_descr[modes.modes[mode_index]
									- DC1394_VIDEO_MODE_160x120_YUV444].width;
						if (scale)
							mode_index--;
					}
					if (mode_index < 0) {
						if (verbose)
							cerr << "Couldn't find valid mode" << endl;
						throw(6);
					}
				}*/
				dc1394_video_set_mode(camera_node, modes.modes[mode_index]);
				mode_type = mode_descr[modes.modes[mode_index]
						- DC1394_VIDEO_MODE_160x120_YUV444].mode;

				if(verbose)
				{
					dc1394video_mode_t video_mode;
					dc1394_video_get_mode(camera_node, &video_mode);
					cout << "video mode: " << video_mode-DC1394_VIDEO_MODE_160x120_YUV444 << endl;
				}

//				if (optical_flag) {
//					if (verbose)
//						cerr << "Bayer filter selection only in Format7"
//								<< endl;
//					throw(1);
//				}

				if (verbose && mode_descr[modes.modes[mode_index]
						- DC1394_VIDEO_MODE_160x120_YUV444].mode == RGB8)
					cout << "Running RGB mode" << endl;
				else if (verbose)
					cout << "Running non-RGB mode" << endl;


			if(format!=7)
			{

				XVSize sized(mode_descr[modes.modes[mode_index]
							- DC1394_VIDEO_MODE_160x120_YUV444].width,
							mode_descr[modes.modes[mode_index]
									- DC1394_VIDEO_MODE_160x120_YUV444].height);
				if(verbose)
					cout << "image size: " << sized.Width() << "x" << sized.Height() << endl;
				init_map(sized, 4);

				//set framerate
				dc1394framerates_t rates;
				dc1394_video_get_supported_framerates(camera_node,
						modes.modes[mode_index], &rates);
				if (rates.num == 0) {
					if (verbose)
						cerr << "No possible rates" << endl;
					throw(7);
				}
				int rate_index = rates.num - 1;
				if(framerate>=0)
				{
					do
					{
						if((rates.framerates[rate_index]-DC1394_FRAMERATE_1_875) == framerate)
							break;
						rate_index--;
					}while (rate_index >= 0);

				}
				if(rate_index<0)
				{
					if (verbose)
						cerr << "Could not set framerate" << endl;
					throw(7);
				}
				dc1394_video_set_framerate(camera_node,
						rates.framerates[rate_index]);
			}
			else // format7
			{
//				if (dc1394_video_set_mode(camera_node, FORMAT7_MODE)
//						!= DC1394_SUCCESS) {
//					if (verbose)
//						cerr << "Couldn't switch Format 7" << endl;
//					throw(8);
//				}
				if(color<0)
					color=0;
				color_coding=static_cast<dc1394color_coding_t>(color+DC1394_COLOR_CODING_MONO8);

				dc1394color_codings_t color_codings;
				dc1394_format7_get_color_codings(camera_node, mode, &color_codings);
				if(color_codings.num==0)
				{
					if (verbose)
						cerr << "No possible color codings for that mode" << endl;
					throw(11);
				}

				int color_coding_index=color_codings.num-1;
				do
				{
					if((color_codings.codings[color_coding_index]) == color_coding)
						break;
					color_coding_index--;
				}while (color_coding_index >= 0);
				if(color_coding_index<0)
				{
					if (verbose)
						cerr << "Could not set color coding" << endl;
					throw(9);
				}
				//dc1394_format7_set_roi

//				if (dc1394_format7_set_color_coding(camera_node, mode,
//						DC1394_COLOR_CODING_MONO8+color_coding) != DC1394_SUCCESS)
//				{
//					if (verbose)
//						cerr << "Couldn't switch Format 7 Bayer" << endl;
//					throw(9);
//				}


				unsigned int hmax, vmax;
				dc1394_format7_get_max_image_size(camera_node,
						mode, &hmax, &vmax);

				if(verbose)
					cout << "hmax: " << hmax << ", vmax: " << vmax << endl;

				if(roi_x<0)
					roi_x=0;
				if(roi_y<0)
					roi_y=0;
				if(roi_width<0)
					roi_width=hmax;
				if(roi_height<0)
					roi_height=vmax;

				float fbytesPerPacket=0;
				switch(framerate)
				{
					case 0:
						fbytesPerPacket = 1.875;
						break;
					case 1:
						fbytesPerPacket = 3.75;
						break;
					case 2:
						fbytesPerPacket = 7.5;
						break;
					case 3:
						fbytesPerPacket = 15;
						break;
					case 4:
						fbytesPerPacket = 30;
						break;
					case 5:
						fbytesPerPacket = 60;
						break;
					case 6:
						fbytesPerPacket = 120;
						break;
					case 7:
						fbytesPerPacket = 240;
						break;
				}

				cout << "Setting framerate to " << fbytesPerPacket << endl;

				switch(color)
				{
					case 0:
						mode_type=MONO8;
						break;
					case 1:
						mode_type=YUV411;
						break;
					case 2:
						mode_type=YUV422;
						break;
					case 3:
						mode_type=YUV444;
						break;
					case 4: //RGB8
						mode_type=RGB8;
//						fbytesPerPacket*=3;
						break;
					case 5: //MONO16
					case 7: //MONO16S
						mode_type=MONO16;
					case 10: //RAW16
//						fbytesPerPacket*=2;
						break;
					case 6: //RGB16
					case 8: //RGB16S
						mode_type=RGB16;
//						fbytesPerPacket*=6;
						break;
				}

//				fbytesPerPacket*=roi_width*roi_height;
				cerr << "ROI: " << roi_width << "x" << roi_height << endl;

//				if(dc1394_format7_set_packet_size(camera_node, mode, 183.0))// bytesPerPacket))
//				{
//					cerr << "Failed to set packet size (framerate)" << endl;
//										throw(13);
//				}

				if (dc1394_feature_set_power(camera_node, DC1394_FEATURE_FRAME_RATE, DC1394_ON)!=DC1394_SUCCESS)
					cerr << "Could not set feature on";

				//Manual mode is necessary to activate absolute mode on Fireflies
				//if(dc1394_feature_set_absolute_control(camera_node, DC1394_FEATURE_FRAME_RATE, DC1394_ON)!=DC1394_SUCCESS)
				{
				    //cerr << "Could not toggle absolute setting control\n";
				    //following lines may be used to hack a framerate in case the absolute mode is not supported
				    if (dc1394_feature_set_mode(camera_node, DC1394_FEATURE_FRAME_RATE, DC1394_FEATURE_MODE_MANUAL)!=DC1394_SUCCESS)
				    	cerr << "Could not set manual mode";
				    if (dc1394_feature_set_value(camera_node,DC1394_FEATURE_FRAME_RATE,472.0)!=DC1394_SUCCESS)
				        cerr << "Could not set feature";
				    unsigned int value=0;
				    if (dc1394_feature_get_value(camera_node,DC1394_FEATURE_FRAME_RATE,&value)!=DC1394_SUCCESS)
				        cerr << "Could not set feature";
				    cout << "Set camera framerate to " << value << endl;
				}
				if(dc1394_feature_set_absolute_control(camera_node, DC1394_FEATURE_FRAME_RATE, DC1394_ON)==DC1394_SUCCESS)
				{

					dc1394switch_t h;
					if(dc1394_feature_get_absolute_control(camera_node, DC1394_FEATURE_FRAME_RATE, &h)!=DC1394_SUCCESS)
						cerr << "Can't get absolute control!";
					cout << "Set camera to " << (int)h << endl;
					float value=-1;
					cout << "Try to set camera framerate to " << fbytesPerPacket << endl;
					//do{
					for(int j=0;j<10;j++)
					{
					if(dc1394_feature_set_absolute_value(camera_node, DC1394_FEATURE_FRAME_RATE, fbytesPerPacket)!=DC1394_SUCCESS)
					    cerr << "Can't set absolute value!";
					if (dc1394_feature_get_absolute_value(camera_node, DC1394_FEATURE_FRAME_RATE, &value)!=DC1394_SUCCESS)
					    cerr << "Can't get absolute value!";
					cout << "Camera framerate set to " << value << endl;
					}
					//}while(value!=100.0);
				}
				else
					cerr << "Could not toggle absolute setting control\n";

//				unsigned int bytesPerPacket = static_cast<int>(ceil(fbytesPerPacket));
//				uint32_t unit_bytes; uint32_t max_bytes;
//				if(dc1394_format7_get_packet_parameters(camera_node, mode, &unit_bytes, &max_bytes))
//					cerr << "error1" << endl;
//				if(dc1394_format7_get_packet_size(camera_node, mode, &bytesPerPacket))
//					cerr << "error2" << endl;
//				cerr << unit_bytes << " " << max_bytes << " " << bytesPerPacket << endl;

				XVSize sized(roi_width, roi_height);
				if(verbose)
					cout << "image size: " << sized.Width() << "x" << sized.Height() << endl;
				init_map(sized, 4);
				if(dc1394_format7_set_roi(camera_node, mode,
						color_coding,
						//DC1394_QUERY_FROM_CAMERA, 0, 0, hmax, vmax);
						DC1394_USE_RECOMMENDED, roi_x, roi_y, roi_width, roi_height))
				{
					cerr << "Setting ROI failed" << endl;
					throw(13);
				}


				if(optical_flag)
				if(dc1394_format7_get_color_filter(camera_node,DC1394_VIDEO_MODE_FORMAT7_0,
									&optical_filter) !=DC1394_SUCCESS)
				{
				 if (verbose) cerr << "Unable to set optical Bayer filter selected " <<
				       optical_filter<< endl;
				 if (verbose) cerr<< "switching function off." << endl;
					 optical_flag=false;
				}
				if (verbose)
					cout << "Running Format 7" << endl;
			}
			if (dc1394_video_set_iso_channel(camera_node, channel)
					!= DC1394_SUCCESS) {
				reset_ieee = true;
				goto re_init;
			}
			// setting the gain
			if (gain != -1) {
				unsigned int min_gain_val, max_gain_val;
				dc1394_feature_get_boundaries(camera_node, DC1394_FEATURE_GAIN,
						&min_gain_val, &max_gain_val);
				if (gain <= max_gain_val && gain >= min_gain_val) {
					set_gain( gain);
				} else {
					if (verbose)
						cerr << " the gain value is beyond limits["
								<< min_gain_val << "," << max_gain_val
								<< "]..exiting" << endl;
					throw(1);
				}
			}

			// set the saturation
			if (saturation != -1) {
				unsigned int min_sat_val, max_sat_val;
				dc1394_feature_get_boundaries(camera_node,
						DC1394_FEATURE_SATURATION, &min_sat_val, &max_sat_val);
				if (saturation <= max_sat_val && saturation >= min_sat_val) {
					set_saturation( saturation);
				} else {
					if (verbose)
						cerr
								<< " the saturation value is beyond limits..exiting"
								<< endl;
					throw(1);
				}
			}

			// setting the sharpness
			if (sharpness != -1) {
				unsigned int min_sharp_val, max_sharp_val;
				dc1394_feature_get_boundaries(camera_node,
						DC1394_FEATURE_SHARPNESS, &min_sharp_val,
						&max_sharp_val);
				if (sharpness <= max_sharp_val && sharpness >= min_sharp_val) {
					set_sharpness( sharpness);
				} else {
					if (verbose)
						cerr
								<< " the sharpness value is beyond limits..exiting"
								<< endl;
					throw(1);
				}
			}

			// setting the shutter
			if (shutter != -1) {
				unsigned int min_shutter_val, max_shutter_val;
				cout << "Setting shutter to " << shutter << endl;
				dc1394_feature_get_boundaries(camera_node,
						DC1394_FEATURE_SHUTTER, &min_shutter_val,
						&max_shutter_val);
				if (shutter <= max_shutter_val && shutter >= min_shutter_val) {
					set_shutter_manual();
					set_shutter( shutter);
				} else {
					if (verbose)
						cerr << " the shutter value is beyond limits ["
								<< min_shutter_val << "," << max_shutter_val
								<< "]..exiting" << endl;
					throw(1);
				}
			}

			// setting the gamma
			if (gamma != -1) {
				unsigned int min_gamma_val, max_gamma_val;
				dc1394_feature_get_boundaries(camera_node,
						DC1394_FEATURE_GAMMA, &min_gamma_val, &max_gamma_val);
				if (gamma <= max_gamma_val && gamma >= min_gamma_val) {
					set_gamma( gamma);
				} else {
					if (verbose)
						cerr << " the gamma value " << gamma
								<< " is beyond limits..exiting" << endl;
					throw(1);
				}
			}

			// setting the exposure
			if (exposure != -1) {
				unsigned int min_exposure_val, max_exposure_val;
				cout << "Setting exposure to " << exposure << endl;
				dc1394_feature_get_boundaries(camera_node,
						DC1394_FEATURE_EXPOSURE, &min_exposure_val,
						&max_exposure_val);
				if (exposure <= max_exposure_val && exposure
						>= min_exposure_val) {
					set_exposure_manual();
					set_exposure( exposure);
				} else {
					if (verbose)
						cerr << " the exposure value is beyond limits..exiting"
								<< endl;
					throw(1);
				}
			}

			// setting u and v components
			if (uv[0] > -1 || uv[1] > -1) {
				if (uv[0] * uv[1] < 0) {
					if (verbose)
						cerr
								<< "just one of the white balance components set!!"
								<< endl;
					throw(1);
				}
				dc1394_feature_whitebalance_set_value(camera_node, uv[0], uv[1]);
			}
			else
				set_whitebalance_auto();

			buffer_index = 0;
			nowait_flag = false;

			if (ext_trigger) {
				// setting the external trigger
				dc1394_software_trigger_set_power(camera_node,
						ext_trigger ? DC1394_OFF : DC1394_ON);
				dc1394_external_trigger_set_mode(camera_node,
						DC1394_TRIGGER_MODE_1);
			}
			//dc1394_feature_set_mode(camera_node,DC1394_FEATURE_BRIGHTNESS,DC1394_FEATURE_MODE_AUTO);
			// dc1394_feature_set_mode(camera_node,DC1394_FEATURE_EXPOSURE,DC1394_FEATURE_MODE_AUTO);
			if (verbose)
				cout << "Setting image size to " << dec << frame(0).Width()
						<< "x" << frame(0).Height() << endl;

			stop_flag=false;
			init_tab();
			dc1394_capture_setup(camera_node, n_buffers,
					DC1394_CAPTURE_FLAGS_DEFAULT
							| DC1394_CAPTURE_FLAGS_AUTO_ISO);
			open( NULL);
			pthread_mutexattr_t tattr;
			pthread_mutexattr_init(&tattr);
			for (int i = 0; i < n_buffers; i++)
				pthread_mutex_init(&wait_grab[i], &tattr);
			pthread_mutexattr_destroy(&tattr);
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&grabber_thread, &attr, grab_thread, this);
			threadded = true;
			pthread_attr_destroy(&attr);
		}

		template<class IMTYPE>
		IMTYPE & XVFlea2G<IMTYPE>::current_frame_continuous(void) {
			struct video1394_wait w_descr;

			if (!nowait_flag) {// Start the cycle
				nowait_flag = true;
				initiate_acquire( buffer_index);
				initiate_acquire(buffer_index + 1);
				wait_for_completion(buffer_index);
			}

			return frame(buffer_index);
		}

		template class XVFlea2G<XVImageRGB<XV_RGB15> > ;
		template class XVFlea2G<XVImageRGB<XV_RGB16> > ;
		template class XVFlea2G<XVImageRGB<XV_RGB24> > ;
		template class XVFlea2G<XVImageRGB<XV_TRGB24> > ;
		template class XVFlea2G<XVImageRGB<XV_RGBA32> > ;
		template class XVFlea2G<XVImageYUV<XV_YUV24> > ;

		//#endif
