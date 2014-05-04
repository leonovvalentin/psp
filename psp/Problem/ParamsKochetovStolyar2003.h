//
//  ParamsKochetovStolyar2003.h
//  psp
//
//  Created by Valentin Leonov on 05/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_ParamsKochetovStolyar2003_h
#define psp_ParamsKochetovStolyar2003_h



#include "utils.h"

#include <iostream>
#include <sstream>



using namespace std;



/**
 Parameters for scheduleKochetovStolyar2003 methtod.
 @param probabilityKP_KS Probability for solving knapsack problem.
 @param probabilitySN_KS Probability for creating subset of neighbours.
 @param tabuListSize_KS Length of tabu list.
 @param changingInterval_KS Number of steps of algorithm before changing neighbourhood.
 @param maxIterationNumber_KS Number of iterations to go through the neighborhood. It is stop criteria.
 */
struct ParamsKochetovStolyar2003 {
    
    float probabilityKP;
    float probabilitySN;
    int tabuListSize;
    int changingInterval;
    int maxIterationNumber;
    
    string strForTable()
    {
        stringstream ss;
        
        ss
        << "probabilityKP"
        << "\tprobabilitySN"
        << "\ttabuListSize"
        << "\tchangingInterval"
        << "\tmaxIterationNumber"
        << endl
        << probabilityKP
        << "\t" << probabilitySN
        << "\t" << tabuListSize
        << "\t" << changingInterval
        << "\t" << maxIterationNumber;
        
        return ss.str();
    }
};



#endif
