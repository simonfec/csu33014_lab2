#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

void student_conv_pthreads(float *** image, int16_t **** kernels, float *** output,
               int width, int height, int nchannels, int nkernels,
               int kernel_order);