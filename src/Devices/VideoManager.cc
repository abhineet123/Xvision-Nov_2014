/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.

*/
#include "config.h"

#ifdef HAVE_LIBPTHREAD
#include <iostream>
#include <string.h>
#include "config.h"
#include "VideoManager.h"
#include "XVMpeg.h"

#ifdef HAVE_BTTV
#include "XVBt8x8.h"
template class VideoManager<XVBt8x8<XVImageRGB<XV_RGB16> > >;
template class VideoManager<XVBt8x8<XVImageRGB<XV_RGB24> > >;
template class VideoManager<XVBt8x8<XVImageRGB<XV_RGBA32> > >;
template class VideoManager<XVBt8x8<XVImageRGB<XV_GLRGBA32> > >;
#endif
template class VideoManager<XVMpeg<XVImageRGB<XV_RGB16> > >;
template class VideoManager<XVMpeg<XVImageRGB<XV_RGB24> > >;
template class VideoManager<XVMpeg<XVImageRGB<XV_RGBA32> > >;
template class VideoManager<XVMpeg<XVImageRGB<XV_GLRGBA32> > >;
#ifdef HAVE_DV
#include "XVDig1394.h"
#include "DV1394.h"
template class VideoManager<XVDig1394<XVImageRGB<XV_RGB16> > >;
template class VideoManager<XVDig1394<XVImageRGB<XV_RGB24> > >;
template class VideoManager<XVDig1394<XVImageRGB<XV_RGBA32> > >;
template class VideoManager<XVDig1394<XVImageRGB<XV_GLRGBA32> > >;
template class VideoManager<DV1394<XVImageRGB<XV_RGB24> > >;
#endif

using namespace std ;

// TODO is there a way to avoid this and get
// a clean pthread initialization?
//
static void *_frame_ptr;

template <class XV_GRABBER>
static void *wrapper_function(void *dummy) 
{
    ((VideoManager<XV_GRABBER>*)_frame_ptr)->acquisition_task(dummy);
    return NULL;
}

template <class XV_GRABBER>
int VideoManager<XV_GRABBER>::initiate_acquire(int framenum)
{

   release_buffer(framenum);
   lock_buffer(framenum);
   while(!pthread_mutex_trylock(&free_flag)) 
      	pthread_cond_broadcast(&free_message);
   pthread_mutex_unlock(&free_flag);
   return framenum;
}

template <class XV_GRABBER>
void *VideoManager<XV_GRABBER>::acquisition_task(void *dummy)
{
  next_queued=free_buffer.front();
  free_buffer.pop_front();
  XV_GRABBER::initiate_acquire(next_queued);
  while(1)
  {
      act_grabbed=next_queued;
      pthread_mutex_lock(&free_flag);
      // no new buffers, wait for status change?
      while(free_buffer.size()==0) 
      {
	cerr << ",";
        pthread_cond_wait(&free_message,&free_flag);
      }
      pthread_mutex_unlock(&free_flag);
      // lock next free
      // wait for a frame
      XV_GRABBER::initiate_acquire(next_queued=free_buffer.front());
      free_buffer.pop_front();
      XV_GRABBER::wait_for_completion(act_grabbed);
      // start frame grabbing
      // Check to see if we can advance or should just
      // reuse the same buffer.
      frame_number++;
      if(!lock_count[act_grabbed])
      {
         cerr << "." ;
         pthread_mutex_lock(&free_flag);
	 free_buffer.push_front(act_grabbed);
         pthread_mutex_unlock(&free_flag);
      }
      else
      {
	 ready_buffer.push_back(act_grabbed);
         while(!pthread_mutex_trylock(&frame_flag)) 
            pthread_cond_broadcast(&frame_message);
	 pthread_mutex_unlock(&frame_flag);
      }
    }
  return NULL;
}

template <class XV_GRABBER>
int VideoManager<XV_GRABBER>::wait_for_completion(int buffer)
{
    int  read_buffer;

    do{
      if(ready_buffer.size()==0) 
      {
         pthread_mutex_lock(&frame_flag);
	 pthread_cond_wait(&frame_message,&frame_flag);
         pthread_mutex_unlock(&frame_flag);
      }
      read_buffer=ready_buffer.front();
      ready_buffer.pop_front();
      if(buffer!=read_buffer)
	ready_buffer.push_back(read_buffer);
    }while(buffer!=read_buffer);
    return 1;
}

template <class XV_GRABBER>
int VideoManager<XV_GRABBER>::lock_buffer(int buffer_number)
{
  if(buffer_number<0 && buffer_number>=n_buffers) return 0;
  lock_count[buffer_number]++;
  return 1;
}

template <class XV_GRABBER>
int VideoManager<XV_GRABBER>::release_buffer(int buffer_number)
{

  if(buffer_number<0 && buffer_number>=n_buffers) return 0;
  if(!lock_count[buffer_number])
     return 0;
  else
  {
    lock_count[buffer_number]--;
    if(!lock_count[buffer_number])
    {
      free_buffer.push_back(buffer_number);
      while(!pthread_mutex_trylock(&free_flag)) 
         pthread_cond_broadcast(&free_message);
      pthread_mutex_unlock(&free_flag);
    }
  }
  return 1;
}

template <class XV_GRABBER>
void VideoManager<XV_GRABBER>::create_thread(void)
{

  if(pthread_create(&grab_child, NULL,
             &wrapper_function<XV_GRABBER>, NULL))
  {
    cerr << "VideoManager<T>: couldn't create acquisition thread!" <<endl;
    exit(1);
  }
}

template <class XV_GRABBER>
VideoManager<XV_GRABBER>::
                   VideoManager(const char *dev_name,const char *parm_string):
                   XV_GRABBER(dev_name,parm_string)
{
  int i;

  _frame_ptr=this;
  act_grabbed=-1;
  next_queued=-1;
  frame_number=0;
  for(i=0;i<n_buffers;i++) free_buffer.push_back(i);
  lock_count=new int[n_buffers];
  memset(lock_count,0,n_buffers*sizeof(int));
  // create support for signaling in tasks (dummy mutex)
  pthread_mutex_init(&free_flag,NULL);
  pthread_mutex_init(&frame_flag,NULL);
  // create signals
  pthread_cond_init(&free_message,NULL);
  pthread_cond_init(&frame_message,NULL);
  // initialize ring buffer
  create_thread();
}

template <class XV_GRABBER>
VideoManager<XV_GRABBER>::~VideoManager(void)
{
   if(lock_count) delete[] lock_count;
}
#endif
