//
//  ParamsKS.h
//  psp
//
//  Created by Valentin Leonov on 05/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_ParamsKS_h
#define psp_ParamsKS_h



#include "utils.h"

#include <iostream>
#include <sstream>



using namespace std;



/**
 Parameters for scheduleKS methtod.
 @param probabilityKP Probability for solving knapsack problem.
 @param probabilitySN Probability for creating subset of neighbours.
 @param tabuListSize Length of tabu list.
 @param changingInterval Number of steps of algorithm before changing neighbourhood.
 @param maxIterationNumber Number of iterations to go through the neighborhood. It is stop criteria.
 */
struct ParamsKS {
    
    float probabilityKP;
    float probabilitySN;
    int tabuListSize;
    int changingInterval;
    int maxIterationNumber;
    
#pragma mark - table
    
    static string strTitlesForTable()
    {
        stringstream ss;
        
        ss
        << "probabilityKP"
        << "\tprobabilitySN"
        << "\ttabuListSize"
        << "\tchangingInterval"
        << "\tmaxIterationNumber";
        
        return ss.str();
    }
    
    string strValuesForTable()
    {
        stringstream ss;
        
        ss
        << probabilityKP
        << "\t" << probabilitySN
        << "\t" << tabuListSize
        << "\t" << changingInterval
        << "\t" << maxIterationNumber;
        
        return ss.str();
    }
    
    string strForTable()
    {
        stringstream ss;
        ss << strTitlesForTable() << endl << strValuesForTable();
        return ss.str();
    }
    
    string str()
    {
        stringstream ss;
        
        ss
        << "probabilityKP = " << probabilityKP
        << ", probabilitySN = " << probabilitySN
        << ", tabuListSize = " << tabuListSize
        << ", changingInterval = " << changingInterval
        << ", maxIterationNumber = " << maxIterationNumber;
        
        return ss.str();
    }
};



#endif
