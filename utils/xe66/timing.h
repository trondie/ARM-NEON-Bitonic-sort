#ifndef XE66_TIMING_H_
#define XE66_TIMING_H_

#include <c6x.h>

static unsigned long long start, end;
static double frequency = 1.0e9;

void resetAndStartTimer( void )
{
	start = _itoll(TSCH, TSCL);

}

double getElapsedTime( void )
{
	end = _itoll(TSCH, TSCL);
	return (end - start) / frequency;
}

#endif /* TIMING_H_ */
