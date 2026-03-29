#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

/* the fast version of conv written by the student using OpenMP */
void student_conv_openmp(float *** image, int16_t **** kernels, float *** output,
               int width, int height, int nchannels, int nkernels,
               int kernel_order)
{
  // this call here is just dummy code that calls the slow, simple, correct version.
  // insert your own code instead
  //multichannel_conv(image, kernels, output, width,
  //                  height, nchannels, nkernels, kernel_order);
}