#include "comm.h"

#include <stdlib.h>

int argPos(char *arg, int argc, char *argv[])
{
    int pos = -1;
    int i = 0;
    for(; i<argc; ++i)
    {
        if(strcmp(arg, argv[i]) == 0) { pos = i + 1; break; }
    }
    if(pos < argc) return pos;
    else return -1;
}

int randInt(int left, int right)
{
    int length = right - left + 1;
    int r = rand() % length;
    return r + left;
}

double randRange(double left, double right)
{
    double r = rand(); r = r / (double)RAND_MAX;
    r *= (right - left); r += left;
    return r;
}

real dist(real *veca, real *vecb, int size)
{
    int i;
    real sum = 0;
    for(i=0; i<size; ++i)
        sum += (veca[i] - vecb[i]) * (veca[i] - vecb[i]);
    return sum;
}
