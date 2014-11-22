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
#include <deque>

#include "DVFrame.h"
#include "DV1394.h"
#include <XVMacros.h>

using namespace std ;

extern int debug;
template class DV1394<XVImageRGB<XV_RGB24> >;

static bool  g_reader_active=false;
static bool  g_buffer_underrun;

// due to the mix of pthreaded C functions and C++
// some global stuff is necessary
static raw1394handle_t    handle; 
static DVFrame *          g_current_frame;
static deque < DVFrame* > g_buffer_queue;
static deque < DVFrame* > g_output_queue;
static pthread_mutex_t    g_mutex;
static pthread_t          g_thread;


// support functions for the raw1394 device from dvgrab
//
static void *dv_frame_reader(void * arg)
{
    g_reader_active = true;
    g_current_frame = NULL;
    while(g_reader_active) raw1394_loop_iterate(handle);
    // end iso grab
    raw1394_destroy_handle(handle);
    // if we are still using a frame, put it back
    if (g_current_frame != NULL) { 
      pthread_mutex_lock(&g_mutex);
      g_buffer_queue.push_back(g_current_frame);
      pthread_mutex_unlock(&g_mutex);
    }
    handle=0;
    return NULL;
}

static int dv_tag_handler(raw1394handle_t handle, unsigned long tag, 
      			   int errcode)
{
   static int i = 0;

   printf("tag %d, tag %lx, errcode %d\n", i++, tag, errcode);
   if (i == 100) g_reader_active = false;
   return 0;
}		  

int avi_iso_handler(raw1394handle_t handle, int channel, size_t length,
                    quadlet_t *data)
{
        /* The PAL/NTSC DV data has the following format:

           - packets of 496 bytes length
           - packets of  16 bytes length.

           The long packets contain the actual video and audio
           contents that goes into the AVI file. The contents of the
           other packets and the meaning of the header is unknown. If
           you know something about it, please let me know.
           
           The actual DV data starts at quadlet 4 of the long packet,
           so we have 480 bytes of DV data per packet.  For PAL, each
           rame is made out of 300 packets and there are 25 frames
           per second.  That is 144000 bytes per frame and 3600000
           bytes per second.  For NTSC we have 250 packages per frame
           and 30 frames per second.  That is 120000 bytes per frame
           and 3600000 bytes per second too.

           We also attempt to detect incomplete frames. The idea is:
           If we hit the begin of frame indicator and this is not the
           very first packet for this frame, then at least one packed
           has been dropped by the driver. This does not guarantee
           that no incomplete frames are saved, because if we miss the
           frame header of the next frame, we can´t tell whether the
           last one is incomplete.  */


        if (length > 16) {
             unsigned char *p = (unsigned char*) & data[3];
		/* section type is in bits 5 - 7 */
                int section_type = p[0] >> 5;           
		/* dif sequence number is in bits 4 - 7 */
                int dif_sequence = p[1] >> 4;           
                int dif_block = p[2];

		/* if we are at the beginning of a frame, we put
		 * the previous frame in our output_queue.
		   Then we try to get an unused frame_buffer from
		   the buffer_queue for the current frame.  We must
		   lock the queues because they are shared between
		   this thread and the main thread. */

                if (section_type == 0 && dif_sequence == 0) {
                        pthread_mutex_lock(&g_mutex);
                        if (g_current_frame != NULL) {
                                g_output_queue.push_back(g_current_frame);
                                g_current_frame = NULL;
                        }


                        if (g_buffer_queue.size() > 0) {
                                g_current_frame = g_buffer_queue.front();
                                g_current_frame->bytesInDVFrame = 0;
                                g_buffer_queue.pop_front();
                        }
                        else
                                g_buffer_underrun = true;
                        pthread_mutex_unlock(&g_mutex);
                }

                if (g_current_frame != NULL) {

                        switch (section_type) {
                        case 0:           // 1 Header block
                                memcpy(g_current_frame->data + dif_sequence * 150 * 80, p, length - 16);
                                break;

                        case 1:           // 2 Subcode blocks
                                memcpy(g_current_frame->data + dif_sequence * 150 * 80 + (1 + dif_block) * 80, p, length - 16);
                                break;

                        case 2:           // 3 VAUX blocks
                                memcpy(g_current_frame->data + dif_sequence * 150 * 80 + (3 + dif_block) * 80, p, length - 16);
                                break;

                        case 3:           // 9 Audio blocks interleaved with video
                                memcpy(g_current_frame->data + dif_sequence * 150 * 80 + (6 + dif_block * 16) * 80, p, length - 16);
                                break;

                        case 4:           // 135 Video blocks interleaved with audio
                                memcpy(g_current_frame->data + dif_sequence * 150 * 80 + (7 + (dif_block / 15) + dif_block) * 80, p, length - 16);
                                break;

                        default:           // we can´t handle any other data
                                break;
                        }
                        g_current_frame->bytesInDVFrame += length - 16;
                }
        }

        return 0;
}


int dv_reset_handler(raw1394handle_t handle) 
{
      static int i = 0;

      printf("reset %d\n", i++);
      if (i == 100) g_reader_active = false;
      return 0;
}

// end of support functions

static 
void convert_coeffs(dv_block_t *bl)
{
    int i;
     for(i=0; i<64;   i++)   bl->coeffs248[i] = bl->coeffs[i];
} // convert_coeffs

static
void convert_coeffs_prime(dv_block_t *bl)
{
    int i;
    for(i=0; i<64;   i++)  bl->coeffs[i] = bl->coeffs248[i];
} // convert_coeffs_prime

template <class T>
void DV1394<T>::decode_frame(DVFrame *frame, int dest_image_index)
{
  static guint8 vsbuffer[80*5]      __attribute__ ((aligned (64))); 
  static dv_videosegment_t videoseg __attribute__ ((aligned (64)));
  
  typename T::PIXELTYPE    *data_ptr=image_buffers[dest_image_index].lock();
  dv_sample_t sampling;
  gboolean is61834 = 0;
  int numDIFseq;
  dv_macroblock_t *mb;
  dv_block_t *bl;
  int ds;
  int b,m,v;
  int lost_coeffs;
  uint dif;
  uint offset;
  size_t mb_offset;
  static gint frame_count;
  guint quality = DV_QUALITY_COLOR | DV_QUALITY_AC_2; 

  assert(data_ptr);

  videoseg.bs = bitstream_init();
  lost_coeffs = 0;
  dif = 0;
  if(frame->IsPAL() && frame->is61834())
      sampling = e_dv_sample_420;
  else
      sampling = e_dv_sample_411;
  numDIFseq = frame->IsPAL() ? 12 : 10;

  // each DV frame consists of a sequence of DIF segments 
  for (ds=0; ds<numDIFseq; ds++) { 
      // Each DIF segment conists of 150 dif blocks, 135 of which are video blocks
      dif += 6; // skip the first 6 dif blocks in a dif sequence 
      /* A video segment consists of 5 video blocks, where each video
         block contains one compressed macroblock.  DV bit allocation
         for the VLC stage can spill bits between blocks in the same
         video segment.  So parsing needs the whole segment to decode
         the VLC data */
      // Loop through video segments 
      for (v=0;v<27;v++) {
	// skip audio block - interleaved before every 3rd video segment
	if(!(v % 3)) dif++; 
        // stage 1: parse and VLC decode 5 macroblocks that make up a video segment
	bitstream_new_buffer(videoseg.bs, &(frame->data[dif*80]), 80*5); 
	videoseg.i = ds;
	videoseg.k = v;
        videoseg.isPAL = frame->IsPAL();
        lost_coeffs += dv_parse_video_segment(&videoseg, quality);
        // stage 2: dequant/unweight/iDCT blocks, and place the macroblocks
        for (m=0,mb = videoseg.mb; m<5; m++,mb++) {
	  for (b=0,bl = mb->b;
	       b<((quality & DV_QUALITY_COLOR) ? 6 : 4);
	       b++,bl++) {
	    if (bl->dct_mode == DV_DCT_248) { 
	      quant_248_inverse(bl->coeffs,mb->qno,bl->class_no);
	      weight_248_inverse(bl->coeffs);
	      convert_coeffs(bl);
	      dv_idct_248(bl->coeffs248);
	      convert_coeffs_prime(bl);
	    } else {
	      quant_88_inverse(bl->coeffs,mb->qno,bl->class_no);
	      weight_88_inverse(bl->coeffs);
	      idct_88(bl->coeffs);
	    } // else
	  } // for b
	  if(sampling == e_dv_sample_411) {
	    mb_offset = dv_place_411_macroblock(mb,3);
	    if((mb->j == 4) && (mb->k > 23)) 
	      dv_ycrcb_420_block((u_char*)data_ptr + mb_offset, mb->b);
	    else
	      dv_ycrcb_411_block((u_char*)data_ptr + mb_offset, mb->b);
	  } else {
	    mb_offset = dv_place_420_macroblock(mb,3);
	    dv_ycrcb_420_block((u_char*)data_ptr + mb_offset, mb->b);
	  }
        } // for mb
	dif+=5;
      } // for s
    } // ds
    image_buffers[dest_image_index].unlock();
}

template <class T>
int DV1394<T>::initiate_acquire(int i_frame)
{
  pthread_mutex_lock(&g_mutex);
  if(frame_ring[i_frame])
  {
    g_buffer_queue.push_back(frame_ring[i_frame]);
    frame_ring[i_frame]=NULL;
  }
  pthread_mutex_unlock(&g_mutex);
  return 1;
}

template <class T>
int DV1394<T>::wait_for_completion(int i_frame)
{
    frame_ring[i_frame] = NULL;
    do { 
      pthread_mutex_lock(&g_mutex);
      // wait until the most recent frame
      while (g_output_queue.size() > 0) {
	// not the first frame to process?
	if(frame_ring[i_frame]) 
	{
	  cerr << ".";
          g_buffer_queue.push_back(frame_ring[i_frame]);
	}
        frame_ring[i_frame] = g_output_queue[0];
        g_output_queue.pop_front();
      }       
      pthread_mutex_unlock(&g_mutex);
      if (frame_ring[i_frame] == NULL) usleep (100000);
   }while(!frame_ring[i_frame]);
   decode_frame(frame_ring[i_frame],i_frame);
   return 1;
}

template <class T>
int DV1394<T>::set_params(char *paramstring)
{
   int 		num_frames=DV_DEF_NUMFRAMES;
   XVParser	parse_result;  

   while(parse_param(paramstring,parse_result)>0) 
     switch(parse_result.c)
     {
       default:
	 cerr << parse_result.c << "=" << parse_result.val 
	      << " is not supported by DV1394 (skipping)" << endl;
     }

   if(handle<0) 
   {
     cerr << "Device is not open ...." << endl;
     exit(1);
   }
   return 1;
}

template <class T>
void DV1394<T>::close(void)
{
  g_reader_active=false;
  while(handle) usleep(100000); // wait for the child to die
}

template <class T> int DV1394<T>::open(const char *dev_name)
{
  iso_handler_t		old_handler;

  channel=63;
  if(handle) close();  // close old stuff first ???
  if(!(handle=raw1394_new_handle()))
  {
    cerr << "couldn't open device " << dev_name << endl;
    exit(1);
  }

  if ((numcards = raw1394_get_port_info(handle, portinfo, DV_PORT_SIZE)) < 0) 
  {
           perror("raw1394 - couldn't get card info");
           exit( -1);
  }
  if (raw1394_set_port(handle, card) < 0) {
            perror("raw1394 - couldn't set port"); 
            exit( -1);
  }
  old_handler = raw1394_set_iso_handler(handle, channel, avi_iso_handler);
  //raw1394_set_bus_reset_handler(handle, dv_reset_handler,0);

  // Starting iso receive 

 if (raw1394_start_iso_rcv(handle, channel) < 0) {
           perror("raw1394 - couldn't start iso receive");
           exit( -1);
 }
 pthread_mutex_init(&g_mutex, NULL);
 pthread_create(&g_thread, NULL, dv_frame_reader, NULL);
 return 1;
}

template <class T>
DV1394<T>::DV1394(const char *dev_name,const char *parm_string):
				XVVideo<T>(dev_name,parm_string)
{
  DVFrame *frame;

  size=XVSize_AVI_full;
  memset(frame_ring,0,sizeof(DVFrame*)*DV_DEF_NUMFRAMES);
  handle=0;
  card = 0;

  weight_init();
  dct_init();
  dv_dct_248_init();
  dv_construct_vlc_table();
  dv_parse_init();
  dv_place_init();
  dv_ycrcb_init();
  for(int i=0;i<DV_DEF_NUMFRAMES;i++) 
  {
        frame=new DVFrame();
    	g_buffer_queue.push_back(frame);
  }
  (void)init_map(size,DV_DEF_NUMFRAMES);
  open(dev_name);
  if(parm_string) set_params((char*)parm_string);
}

template <class T>
DV1394<T>::~DV1394()
{
  if(!handle) raw1394_destroy_handle(handle);
  //TODO destroy g_buffers
}

