#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <arm_neon.h>
#include <sys/time.h>
#include "neonBitonic.h"
#include "linkNeon.h"

//Comparator
#define COMP(l,r) \
min = vminq_f32(l, r); \
max = vmaxq_f32(l, r); \
l = min; r = max;

 void bitonic_4x4(float *ret, float* list)
{
    float32x4_t x[4];
    int i;

    //Load
    for(i = 0;i < 4;i++){
        x[i] = vld1q_f32(list + 4 * i);
    }

    //Sort and transpose
    registerSort(&x[0], &x[1], &x[2], &x[3]);
    butterfly_merge(&x[0], &x[1]);
    butterfly_merge(&x[2], &x[3]);

    //Store
    vst1q_f32(ret, x[0]);
    vst1q_f32(ret+4, x[1]);
    vst1q_f32(ret+8, x[2]);
    vst1q_f32(ret+12, x[3]);
}

//Bitonic Butterfly Network
void butterfly_merge(float32x4_t *a, float32x4_t *b){

    float32x4_t l, h, l_pmt, h_pmt, out_l, out_h;
    float32x4_t l_temp, h_temp;
    float32x2_t l_low, l_high, h_low, h_high;

    /*(0,1,2,3) for b_t */
    l_low = vget_high_f32(*b);
    l_low = vrev64_f32(l_low);
    l_high = vget_low_f32(*b);
    l_high = vrev64_f32(l_high);
    *b = vcombine_f32(l_low, l_high);
    
    
    l = vminq_f32(*a, *b);
    h = vmaxq_f32(*a, *b);

    /*1,0,1,0 for l and h to l_pmt*/
    l_low = vget_low_f32(l);
    h_high = vget_low_f32(h);
    l_pmt = vcombine_f32(l_low, h_high);

    /*3,2,3,2 for l and h to h_pmt*/
    l_low = vget_high_f32(l);
    h_high = vget_high_f32(h);
    h_pmt = vcombine_f32(l_low, h_high);

    l = vminq_f32(l_pmt, h_pmt);
    h = vmaxq_f32(l_pmt, h_pmt);

    /*2,0,2,0 for l and h to l_pmt*/
    l_temp = l;
    h_temp = h;
    l_temp[1] = vgetq_lane_f32(l, 2);
    l_low = vget_low_f32(l_temp);
    h_temp[1] = vgetq_lane_f32(h, 2);;
    h_high = vget_low_f32(h_temp);
    l_pmt = vcombine_f32(l_low,h_high);

    float32_t te = l_pmt[2];
    l_pmt[2] = l_pmt[1];
    l_pmt[1] = te; 

    /*3,1,3,1 for l and h into hp*/
    l_temp = l;
    h_temp = h;
    l_temp[0] = vgetq_lane_f32(l, 1);
    l_temp[1] = vgetq_lane_f32(l, 3);
    l_low = vget_low_f32(l_temp);

    h_temp[0] = vgetq_lane_f32(h, 1);
    h_temp[1] = vgetq_lane_f32(h, 3);
    h_high = vget_low_f32(h_temp);
    h_pmt = vcombine_f32(l_low, h_high);

    /*3,1,2,0 for h_pmt and h_pmt into h_pmt*/
    te = h_pmt[2];
    h_pmt[2] = h_pmt[1];
    h_pmt[1] = te; 

    l = vminq_f32(l_pmt, h_pmt);
    h = vmaxq_f32(l_pmt, h_pmt);

    /*1,0,1,0 for l and h to out_l*/
    l_low = vget_low_f32(l);
    h_high = vget_low_f32(h);
    out_l = vcombine_f32(l_low, h_high);

    /*3,2,3,2 for l and h to out_h*/
    l_low = vget_high_f32(l);
    h_high = vget_high_f32(h);
    out_h = vcombine_f32(l_low, h_high);

    /*3,1,2,0 from out_l and out_l to a*/
    te = out_l[2];
    out_l[2] = out_l[1];
    out_l[1] = te; 

    te = out_h[2];
    out_h[2] = out_h[1];
    out_h[1] = te; 

    *a = out_l;
    *b = out_h;
}

//In-register sorting
void registerSort(float32x4_t *x0, float32x4_t *x1, float32x4_t *x2, float32x4_t *x3){

    float32x4_t min, max, temp, l_temp, h_temp, tmp, row1, row0;
    float32x2_t l_low, h_high;

    //Sort lanes
    COMP(*x0, *x1);
    COMP(*x2, *x3);
    COMP(*x1, *x2);
    COMP(*x0, *x1);
    COMP(*x2, *x3);
    COMP(*x1, *x2);

    /*Transpose*/

    /*1,0,1,0 for l and h to l_pmt*/
    l_low = vget_low_f32(*x0);
    h_high = vget_low_f32(*x1);
    tmp = vcombine_f32(l_low, h_high);

    l_low = vget_low_f32(*x2);
    h_high = vget_low_f32(*x3);
    row1 = vcombine_f32(l_low, h_high);

    /*2,0,2,0 for l and h to l_pmt*/
    l_low = vget_low_f32(tmp);
    h_high = vget_low_f32(row1);
    l_low[1] = vgetq_lane_f32(tmp, 2);
    h_high[1] = vgetq_lane_f32(row1, 2);
    row0 = vcombine_f32(l_low,h_high);

    /*3,1,3,1 for l and h into hp*/
    l_low[0] = vgetq_lane_f32(tmp, 1);
    l_low[1] = vgetq_lane_f32(tmp, 3);
    h_high[0] = vgetq_lane_f32(row1, 1);
    h_high[1] = vgetq_lane_f32(row1, 3);
    row1 = vcombine_f32(l_low, h_high);

    l_low = vget_high_f32(*x0);
    h_high = vget_high_f32(*x1);
    tmp = vcombine_f32(l_low, h_high);
    
    *x0 = row0;
    *x1 = row1;

    /*3,2,3,2 for l and h to h_pmt*/
    l_low = vget_high_f32(*x3);
    h_high = vget_high_f32(*x2);
    row1 = vcombine_f32(h_high, l_low);
    
    /*2,0,2,0 for l and h to l_pmt*/
    l_low = vget_low_f32(row1);
    h_high = vget_low_f32(tmp);
    l_low[1] = vgetq_lane_f32(row1, 2);
    h_high[1] = vgetq_lane_f32(tmp, 2);
    row0 = vcombine_f32(h_high, l_low);

    /*3,1,3,1 for l and h into hp*/
    l_low[0] = vgetq_lane_f32(row1, 1);
    l_low[1] = vgetq_lane_f32(row1, 3);
    h_high[0] = vgetq_lane_f32(tmp, 1);
    h_high[1] = vgetq_lane_f32(tmp, 3);
    row1 = vcombine_f32(h_high, l_low);

    *x2 = row0;
    *x3 = row1;
}

//Merge
void merge(float *output, float *input, uintptr_t length)
{
    float32x4_t x, y;
    uintptr_t half;
    float *halfptr, *endptr;

    half = length / 2;
    halfptr = input + half;
    endptr = input + length;

    float *list1 = input;
    float *list2 = halfptr;
    x = vld1q_f32(list1);
    y = vld1q_f32(list2);
    list1 += 4;
    list2 += 4;
    butterfly_merge(&x, &y);
    vst1q_f32(output, x);
    output += 4;

    while(1){
        if (*list1 < *list2){
            x = vld1q_f32(list1);
            list1 += 4;
            butterfly_merge(&x, &y);
            vst1q_f32(output, x);
            output += 4;
            if (list1 >= halfptr){
                goto end_list1;
            }
        } else {
            x = vld1q_f32(list2);
            list2 += 4;
            butterfly_merge(&x, &y);
            vst1q_f32(output, x);
            output += 4;
            if (list2 >= endptr){
                goto end_list2; 
            }
        }
    }

end_list1:

    while(list2 < endptr){
        x = vld1q_f32(list2);
        list2 += 4;
        butterfly_merge(&x, &y);
        vst1q_f32(output, x);
        output += 4;
    }
    goto end;

end_list2:

    while(list1 < halfptr){
        x = vld1q_f32(list1);
        list1 += 4;
        butterfly_merge(&x, &y);
        vst1q_f32(output, x);
        output += 4;
    }

end:
    
    vst1q_f32(output, y);
    return;
}


