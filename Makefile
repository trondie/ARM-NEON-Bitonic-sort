#BASEPATH=../..
#ifndef CC

#ifndef (CC)
#    CC=gcc
#endif

#SSCC=gcc
MCC=mcc
CC=gcc

#CFLAGS += -k
#--cc=icc --cpp=icc --ld=icc

OBJECTS = bitonicNeon.o
CFLAGS += $(ARCH) $(DEBUG) -mfpu=neon -std=c99 -mcpu=cortex-a15 -O3 -I /usr/include/python2.7 -L /usr/include/python2.7 -lpython2.7
VFLAGS += -DVALIDATE
#VFLAGS += -O3
#ifdef neon_intrinsics
	VFLAGS +=
#endif
ifdef neon_assembly
	VFLAGS += -DNEON_ASSEMBLY
endif
ifdef neon_off
	VFLAGS += -DNOVEC 
endif

all: merge.x

merge.o: mergeOmpss.c kernel.c linkNeon.h cmdline.c cmdline.h
	$(MCC) $(VFLAGS) -L/home/trondil/papi3/lib  -std=c99 -lpapi  -mfpu=neon -mtune=cortex-a15 -O3 -I/home/trondil/papi3/include  -L/home/trondil/papi3/lib -lpapi --ompss -I /usr/include/python2.7 -L /usr/include/python2.7 -lpython2.7 -c mergeOmpss.c -o merge.o -O3

bitonicNeon.o: linkNeon.h neonBitonic.h neonBitonic.c cmdline.c cmdline.h
	$(CC) -L/home/trondil/papi3/lib  -std=c99 -lpapi  -mfpu=neon -mtune=cortex-a15 -c neonBitonic.c -o bitonicNeon.o -O3 -I /usr/include/python2.7 -L /usr/include/python2.7 -lpython2.7
merge.x: $(OBJECTS) merge.o cmdline.c cmdline.h
	$(MCC) $(VFLAGS) -L/home/trondil/papi3/lib -std=c99 -lpapi  -mfpu=neon-vfpv4  -mtune=cortex-a15 -o $@ $(OBJECTS) merge.o cmdline.c --ompss -L/home/trondil/papi3/lib -lm -I/home/trondil/code/kernels/conv/utils -lpapi -O3 -I /usr/include/python2.7 -L /usr/include/python2.7 -lpython2.7


clean:
	rm -f *.o *.x sscc_*.c
