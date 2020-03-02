#ifndef COMMON_TIMING_H_
#define COMMON_TIMING_H_

#include <sys/time.h>
static struct timeval start, stop;

void resetAndStartTimer( void )
{
	gettimeofday( &start, NULL );
}

double getElapsedTime( void )
{
	gettimeofday( &stop, NULL );
	return (1000000 * (stop.tv_sec - start.tv_sec)) + stop.tv_usec - start.tv_usec;
}

#endif // TIMING_H
