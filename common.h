#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#define MAX 100000000
const int n_iterator = 20;
double wall_time()
{
    //printf("213\n");
#ifdef GETTIMEOFDAY
    struct timeval t;
    gettimeofday(&t, NULL);
    return 1. * t.tv_sec + 1.e-6 * t.tv_usec;
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return 1. * t.tv_sec + 1.e-9 * t.tv_nsec;
#endif
}
#endif //COMMON_H