#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <ippcc.h>

#include "XVAVI.h"


#define BUF_SIZE (640*480*4)

template <class IMTYPE>
XVAVI<IMTYPE>::XVAVI(const char * fname,
		  const char * parm_string)
  : XVVideo<IMTYPE>(fname) {

  int d;
  opts=NULL;
  pFormatCtx=avformat_alloc_context();
  rest_count=BUF_SIZE;bytesRemaining=0,fFirstTime=true;
  index=0;
  av_register_all();
  if((d=avformat_open_input(&pFormatCtx, fname, NULL, &opts))!=0)
  {
    cerr << "Couldn't open file " << fname << " with error " << d <<endl;
    throw 10;
  }
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
  {
     cerr << "Couldn't identify stream" << endl;
     throw 11;
  }
  for(int i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
    {
        videoStream=i;
        break;
    }
  if(videoStream<0)
  {
     cerr << "Couldn't find video stream" << endl;
     throw 12;
  }
  av_context=pFormatCtx->streams[videoStream]->codec;
  av_codec=avcodec_find_decoder(av_context->codec_id);
  if(av_codec->capabilities & CODEC_CAP_TRUNCATED)
    av_context->flags|=CODEC_FLAG_TRUNCATED;
  XVSize set_size(av_context->width,av_context->height);
  init_map(set_size,DEF_MPEG_NUM);
  open(fname);
  //img_convert_ctx = sws_getContext(av_context->width,av_context->height,
//			av_context->pix_fmt,
 //                       av_context->width,av_context->height,
//			PIX_FMT_RGB24, SWS_BICUBIC,
 //                       NULL, NULL, NULL);
  // if(img_convert_ctx == NULL) {
//	cerr << "Cannot initialize the conversion context!" << endl;
 //       throw 13;
  //  }
}

template <class IMTYPE>
int XVAVI<IMTYPE>::open(const char * fname) {

   if(avcodec_open2(av_context,av_codec, NULL)<0) throw 12;

   av_frame=avcodec_alloc_frame();

  return 1 ;
}

template <class IMTYPE>
void XVAVI<IMTYPE>::close()
{
  //::close(fd);
}

//-----------------------------------------------------------------------------
//  Grab functions
//-----------------------------------------------------------------------------

template <class IMTYPE>
int XVAVI<IMTYPE>::initiate_acquire(int framenum) {
  return 1;
}

template <class IMTYPE>
int XVAVI<IMTYPE>::wait_for_completion(int framenum) {


    int             bytesDecoded;
    int             frameFinished=0;
    AVPacket	    pkt;
    if(fFirstTime)
    {
        fFirstTime=false;
        packet.data=NULL;
    }
    av_init_packet(&pkt);
    while(!frameFinished)
    {
       if(bytesRemaining > 0)
        while(bytesRemaining > 0 && !frameFinished)
        {
	    pkt.data=rawData;
	    pkt.size=bytesRemaining;
            bytesDecoded=avcodec_decode_video2(av_context, av_frame,
                &frameFinished, &pkt);
            if(bytesDecoded < 0)
            {
                cerr<< "Error while decoding frame"<< endl;
                throw 12;
            }
            bytesRemaining-=bytesDecoded;
            rawData+=bytesDecoded;
        }
      else
      {
        do
        {
            if(packet.data!=NULL)
                av_free_packet(&packet);
            if(av_read_frame(pFormatCtx, &packet)<0)
                throw 22;
        } while(packet.stream_index!=videoStream);

        bytesRemaining=packet.size;
        rawData=packet.data;
       }
    }
    /*const Ipp8u *pSrc[3]={av_frame->data[0],av_frame->data[1],
                          av_frame->data[2]};
    int srcStep[3]={av_frame->linesize[0],
    		    av_frame->linesize[1],
		    av_frame->linesize[2]};

    IppiSize roi={av_context->width, av_context->height};
    ippiYUV420ToBGR_8u_P3C3R(pSrc,srcStep,
                           (Ipp8u*)frame(framenum).lock(),
                           frame(framenum).Width()*3,roi);*/
    frame(framenum).unlock();
    return framenum;
}

/* libmpeg only supports 24 bit MPEG decoding.
 * Thus, in order to use an mpeg on a display of <24 bits, you must
 * explicitly create the XVImageRGB<XV_RGB15|16> and perform the
 * conversion this way on your own. jcorso 26August2001
 *
*/
template class XVAVI<XVImageRGB<XV_RGB24> >;
