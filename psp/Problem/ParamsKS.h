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
 @param maxIterationNumber Number of iterations to go through the neighborhood. It is stop criteria.
 @param probabilityKP Probability for solving knapsack problem.
 @param probabilitySN Probability for creating subset of neighbors.
 @param tabuListSize Length of tabu list.
 @param changingInterval Number of steps of algorithm before changing neighborhood.
 @param numberOfReturnsToRecord If not 0, return to record will be ever maxIterationNumber/numberOfReturnsToRecord iteration (it is intensification of search).
 */
struct ParamsKS {
    
    int maxIterationNumber;
    float probabilityKP;
    float probabilitySN;
    int tabuListSize;
    int changingInterval;
    int numberOfReturnsToRecord;
    
#pragma mark - table
    
    static string strTitlesForTable()
    {
        stringstream ss;
        
        ss
        << "maxIterationNumber"
        << "\tprobabilityKP"
        << "\tprobabilitySN"
        << "\ttabuListSize"
        << "\tchangingInterval"
        << "\tnumberOfReturnsToRecord";
        
        return ss.str();
    }
    
    string strValuesForTable()
    {
        stringstream ss;
        
        ss
        << maxIterationNumber
        << "\t" << probabilityKP
        << "\t" << probabilitySN
        << "\t" << tabuListSize
        << "\t" << changingInterval
        << "\t" << numberOfReturnsToRecord;
        
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
        << "maxIterationNumber = " << maxIterationNumber
        << ", probabilityKP = " << probabilityKP
        << ", probabilitySN = " << probabilitySN
        << ", tabuListSize = " << tabuListSize
        << ", changingInterval = " << changingInterval
        << ", numberOfReturnsToRecord = " << numberOfReturnsToRecord;
        
        return ss.str();
    }
};



#endif
