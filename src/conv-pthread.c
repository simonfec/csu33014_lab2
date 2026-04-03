// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <x86intrin.h>
#include "threadpool.h"

const unsigned int WORK_BUF_SIZE = 256;

void perform_inner(TPoolArgs args) {
  for (int h = 0; h < args.height; h++) {

    __m128d sums = _mm_setzero_pd();
    for (int c = 0; c < args.nchannels; c += 2 ) {
      for (int x = 0; x < args.kernel_order; x++) {
        for (int y = 0; y < args.kernel_order; y++ ) {
          __m128d image_channels = _mm_setr_pd((double)args.image[args.w+x][h+y][c], (double)args.image[args.w+x][h+y][c + 1]);
          __m128d kernel_channels = _mm_setr_pd(
            (double)args.kernels[args.m][c][x][y],
            (double)args.kernels[args.m][c + 1][x][y]);
          __m128d mul = _mm_mul_pd(image_channels, kernel_channels);
          sums = _mm_add_pd(sums, mul);
        }
      }
    }

    __m128d temp = _mm_hadd_pd(sums, sums);
    double sum;
    _mm_store_sd(&sum, temp);

    args.output[h] = (float)sum;
  }
}

/* the fast version of conv written by the student using pthreads */
void student_conv_pthreads(float *** image, int16_t **** kernels, float *** output,
               int width, int height, int nchannels, int nkernels,
               int kernel_order)
{
  // Assume number of hardware threads is the best number of threads to allocate.
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
}