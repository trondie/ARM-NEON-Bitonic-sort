#ifndef _LINKNEON_H_
#define _LINKNEON_H_
#include <stdint.h>
 void merge_sort(float *buffer, float *list, uintptr_t length);
 void merge_sort_desc(float *buffer, float *list, uintptr_t length);
 void merge_sort_test(uintptr_t num_exp);
 void bitonic_4x4(float *ret, float* list);
 void merge(float *output, float *input, uintptr_t length);
#endif