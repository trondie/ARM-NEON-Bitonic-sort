#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmdline.h"

void print_help(const char* prog)
{
    printf("\n");
    printf("Usage: %s [--help]\n", prog);
    printf("            [--threads=N/-t N]\n");
    printf("            [--width=N/-x N]\n");
    printf("            [--height=N/-y N]\n");
    printf("            [--verbose]\n");
    printf("            [--valgrind]\n");
    printf("            [--help]\n");
    printf("\n");
    printf("\n");
    printf(" Options:\n");
    printf("\n");
    printf("  --size=N     \n");
    printf("  Set matrix sizes to NxN.\n");
    printf("\n");
    printf("  --threads=N     \n");
    printf("  Set number of threads to N.\n");
    printf("\n");
    printf("  --help        \n");
    printf("  Display this usage message and exit.\n");
    printf("\n");
    printf("  --verbose     \n");
    printf("  Enable verbose output.\n");
    printf("\n");
    exit(0);
}

void parse_cmdline(int argc, char** argv, cmd_args* args)
{
    int c, option_index = 0; 
    
    //defaults
    args->N = 1024;
    args->NB = 0;
    args->threads = 1;

    //flags
    static int verbose, inplace, measure, valgrind;
    verbose = 0; inplace = 0; measure = 0; valgrind = 0;

    static struct option long_options[] =
    {
        {"verbose", no_argument, &verbose, 1},
        {"valgrind", no_argument, &valgrind, 1},

        {"size",   required_argument, 0, 'x'},
        {"bsize",   required_argument, 0, 'b'},
        {"threads", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "x:b:y:z:t:h", long_options, &option_index)) != -1)
    {
        switch (c)
        {
            case 'x':
                args->N = atol(optarg);
                break;
            case 'b':
                args->NB = atol(optarg);
                break;
            case 't':
                args->threads = atoi(optarg);
                break;
            case 'h':
                print_help(argv[0]);
                break;
            case '?':
                fprintf(stderr, "Invalid option: %s\n", long_options[option_index].name);
                exit(1);
                break;
        }
    }
    args->verbose = verbose;
    args->valgrind = valgrind;

    if (args->NB == 0)
    {
        int bs;
        for (bs = 1024; bs >= 32; bs /= 2)
        {
            int nt = args->N/bs;
            if (nt*nt >= args->threads)
                break;
        }
        args->NB = bs;
    }
    if (args->NB > args->N)
        args->NB = args->N;
    //printf("Block size: %d\n", args->NB);
}
