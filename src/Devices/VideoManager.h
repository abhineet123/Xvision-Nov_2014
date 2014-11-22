#ifndef __videomanag_h
#define __videomanag_h

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
#ifdef HAVE_LIBPTHREAD
extern "C"{
#include <pthread.h>
}
# include <deque>
#include "XVVideo.h"

#define MAX_VIDMANG_MAP 10
#define MIN_BUFFERS     4

template <class XV_GRABBER>
class VideoManager: public XV_GRABBER
{
  protected:
  using XV_GRABBER::n_buffers; 

  private:
  int             act_grabbed;
  int		  next_queued;
  int		  frame_number;

  // handling of invalid buffers
  deque	<int>     free_buffer;
  deque	<int>     ready_buffer;

  int		  *lock_count;		// count of lock commands 
  int		  last_buffer;
  int		  buffer_map[MAX_VIDMANG_MAP];	// mapping user and physical

  // pthread structures
  pthread_t       grab_child;           // reference to the acquis. thr.
  // messages about status changes in buffers
  pthread_cond_t  free_message,frame_message;
  // mutex for messages and for frame buffers
  pthread_mutex_t free_flag,frame_flag;
 public:
  		  VideoManager(const char *dev_name,
				       const char *parm_string=NULL);
  		  ~VideoManager(void);
  int		  initiate_acquire(int current);
  int		  wait_for_completion(int buffer);
  int		  lock_buffer(int buffer_number);
  int		  release_buffer(int  buffer_number);
  void	          create_thread(void);
  void 		  *acquisition_task(void *); // for internal use
};

#endif
#endif
