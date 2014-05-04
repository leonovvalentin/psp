//
//  Random.cpp
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Random.h"
#include <math.h>



#ifdef DEBUG
long Random :: _seed = 0;
#else
long Random :: _seed = (long)time(NULL);
#endif



long Random :: nextRandom()
{
    srand((unsigned int)_seed);
    _seed = rand();
    return _seed;
}

int Random :: randomInt()
{
    return (int)nextRandom();
}

int Random :: randomInt(int first, int second)
{
    if (first > second) {
        int temp = second;
        second = first;
        first = temp;
    }
    return nextRandom() % (second - first + 1) + first;
}

long Random :: randomLong(long first, long second)
{
    if (first > second) {
        long temp = second;
        second = first;
        first = temp;
    }
    return nextRandom() % (second - first + 1) + first;
}

float Random :: randomFloatFrom0To1()
{
    return (float)nextRandom() / RAND_MAX;
}

float Random :: pseudoRandomFloatFrom0To1(int number, int count)
{
    srand((unsigned int)exp(number));
    for (int i = 0; i < count; i++) rand();
    return (float)rand() / RAND_MAX;
}
