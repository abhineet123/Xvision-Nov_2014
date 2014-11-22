#include "XVImageScalar.h"
#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace std ;

struct rusage rbuffer;
int usec_started;
int sec_started;
int usec_elapsed;
int sec_elapsed;


int main() {
  XVImageScalar<float> restest(320,240);
  XVImageScalar<float> restarg(320,240);

  XVImageIterator<float> graditer(restest);
  
  while (graditer.end() == false) {
    *graditer = graditer.getcurrposx() + graditer.getcurrposy();
    ++graditer;
  }

  (void) getrusage(RUSAGE_SELF, &rbuffer);
  usec_started = rbuffer.ru_utime.tv_usec;
  sec_started = rbuffer.ru_utime.tv_sec;
  
  for (int i = 0; i < 1500; i++) {
    //cerr<<i<<endl;
	restarg = Box_Filter_x(restest,160);
  }

  (void) getrusage(RUSAGE_SELF, &rbuffer);
  usec_elapsed = rbuffer.ru_utime.tv_usec - usec_started;
  sec_elapsed = rbuffer.ru_utime.tv_sec - sec_started;

  cerr<<"Usec_Elapsed: "<<usec_elapsed<<"\nSec_Elapsed: "<<sec_elapsed<<endl;

  return 0;
}
