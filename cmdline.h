#ifndef __CMDLINE_H_
#define __CMDLINE_H_
#include <stdlib.h>
typedef struct 
{
    size_t N, NB;
    int threads;
    int verbose, inplace, measure, valgrind; //flags
} cmd_args;

void parse_cmdline(int argc, char** argv, cmd_args* args);

#endif
