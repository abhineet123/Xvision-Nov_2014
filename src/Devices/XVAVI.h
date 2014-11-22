// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVMPEG2_H_
#define _XVMPEG2_H_

#define DEF_MPEG_NUM   4

extern "C"{
#include <stdint.h>
#ifndef UINT64_C
#define UINT64_C(value) __CONCAT(value, ULL)
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

typedef struct URLContext {
    const AVClass *av_class;    /**< information for av_log(). Set by url_open(). */
    struct URLProtocol *prot;
    void *priv_data;
    char *filename;             /**< specified URL */
    int flags;
    int max_packet_size;        /**< if non zero, the stream is packetized with this max packet size */
    int is_streamed;            /**< true if streamed (no seek possible), default = false */
    int is_connected;
    AVIOInterruptCB interrupt_callback;
    int64_t rw_timeout;         /**< maximum time to wait for (network) read/write operation completion, in mcs */
} URLContext;

typedef struct URLProtocol {
    const char *name;
    int     (*url_open)( URLContext *h, const char *url, int flags);
    /**
     * This callback is to be used by protocols which open further nested
     * protocols. options are then to be passed to ffurl_open()/ffurl_connect()
     * for those nested protocols.
     */
    int     (*url_open2)(URLContext *h, const char *url, int flags, AVDictionary **options);

    /**
     * Read data from the protocol.
     * If data is immediately available (even less than size), EOF is
     * reached or an error occurs (including EINTR), return immediately.
     * Otherwise:
     * In non-blocking mode, return AVERROR(EAGAIN) immediately.
     * In blocking mode, wait for data/EOF/error with a short timeout (0.1s),
     * and return AVERROR(EAGAIN) on timeout.
     * Checking interrupt_callback, looping on EINTR and EAGAIN and until
     * enough data has been read is left to the calling function; see
     * retry_transfer_wrapper in avio.c.
     */
    int     (*url_read)( URLContext *h, unsigned char *buf, int size);
    int     (*url_write)(URLContext *h, const unsigned char *buf, int size);
    int64_t (*url_seek)( URLContext *h, int64_t pos, int whence);
    int     (*url_close)(URLContext *h);
    struct URLProtocol *next;
    int (*url_read_pause)(URLContext *h, int pause);
    int64_t (*url_read_seek)(URLContext *h, int stream_index,
                             int64_t timestamp, int flags);
    int (*url_get_file_handle)(URLContext *h);
    int (*url_get_multi_file_handle)(URLContext *h, int **handles,
                                     int *numhandles);
    int (*url_shutdown)(URLContext *h, int flags);
    int priv_data_size;
    const AVClass *priv_data_class;
    int flags;
    int (*url_check)(URLContext *h, int mask);
} URLProtocol;



#include <stdio.h>

#include <XVVideo.h>

template <class IMTYPE>
class XVAVI : public XVVideo<IMTYPE> {
protected:
  using XVVideo<IMTYPE>::size ;
  using XVVideo<IMTYPE>::init_map ;
public:
  using XVVideo<IMTYPE>::frame ;

private:
  int		  fd;
  int		  rest_count;
  int		  index;
  int		  videoStream;
  AVCodecContext  *av_context;
  AVCodec	  *av_codec;
  AVFrame	  *av_frame;
  AVFormatContext *pFormatCtx;
  AVDictionary    *opts;
  URLProtocol     file_protocol;
  AVPacket	  packet;
  int             bytesRemaining;
  uint8_t  	  *rawData;
   bool           fFirstTime;
  struct SwsContext *img_convert_ctx;
  AVFrame         *pFrameRGB;
  char *	  buffers[2];
public:

  XVAVI (const char *fname = "mpeg_file.mpg",
	  const char *parm_string="");
  ~XVAVI(){};
  virtual int open(const char * filename);
  void close();
  virtual int  initiate_acquire(int buffernum);
  virtual int  wait_for_completion(int buffernum);

  int set_params(char *params) {return 1;};
};

#endif
