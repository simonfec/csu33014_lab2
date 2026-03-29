// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

void student_conv_openmp(float *** image, int16_t **** kernels, float *** output,
               int width, int height, int nchannels, int nkernels,
               int kernel_order);