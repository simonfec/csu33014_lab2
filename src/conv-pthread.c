// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include "threadpool.h"

const unsigned int WORK_BUF_SIZE = 256;

void perform_inner(TPoolArgs args) {
  for (int h = 0; h < args.height; h++) {

    double sum = 0.0;
    for (int c = 0; c < args.nchannels; c++ ) {
      for (int x = 0; x < args.kernel_order; x++) {
        for (int y = 0; y < args.kernel_order; y++ ) {
          sum += args.image[args.w+x][h+y][c] * args.kernels[args.m][c][x][y];
        }
      }
      args.output[h] = (float)sum;
    }
  }
}


/* the fast version of conv written by the student using pthreads */
void student_conv_pthreads(float *** image, int16_t **** kernels, float *** output,
               int width, int height, int nchannels, int nkernels,
               int kernel_order)
{
  int hw_threads = sysconf(_SC_NPROCESSORS_ONLN);
  Threadpool* pool = threadpool_alloc(hw_threads, WORK_BUF_SIZE);
  int w, m;

  for ( m = 0; m < nkernels; m++ ) {
    for ( w = 0; w < width; w++ ) {
        TPoolArgs args = {
          .m = m,
          .w = w,
          .height = height,
          .nchannels = nchannels,
          .kernel_order = kernel_order,
          .image = image,
          .kernels = kernels,
          .output = output[m][w]
        };

        TPoolWork work = {
          .func = perform_inner,
          .args = args
        };

        threadpool_schedule(pool, work);
    }
  }

  threadpool_join(pool);
  threadpool_dealloc(pool);
}