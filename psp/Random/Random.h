//
//  Random.h
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Random__
#define __psp2__Random__



#include <iostream>
using namespace std;



//#warning It is for test only
//#define RANDOM_FOR_DEBUGGING 1



class Random
{
private:
    static long _seed;
    
private:
    static long nextRandom();
    
public:
    static int randomInt();
    static int randomInt(int first, int second);
    static long randomLong(long first, long second);
    static float randomFloatFrom0To1();
    static float pseudoRandomFloatFrom0To1(int number, int cout);
};



#endif /* defined(__psp2__Random__) */
