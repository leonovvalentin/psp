//
//  ParamsGA.h
//  psp
//
//  Created by Valentin Leonov on 05/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_ParamsGA_h
#define psp_ParamsGA_h



static int const maxGeneratedSchedulesInfinite = INT_MAX;



/**
 Parameters for scheduleGA methtod.
 @param maxGeneratedSchedules Max number of generated schedules. It is stop criterion. If it is equal to maxGeneratedSchedulesInfinite, then schedules will be generating without stop criterion (infinite).
 @param populationSize Size of population.
 @param maxParents Number of selected parents in each iteration. It should be greater then 1.
 @param maxChildren Number of generated children for each iteration.
 @param numberOfChildrenInNextGeneration Number of created children which will be added to next generation.
 @param timesPingPong Number of attempts to build a record in Ping-pong algorithm when we construct schedules for population.
 @param probabilityKP Probability for solving knapsack problem.
 @param probabilityParentSelection Probability for selectiong schedule from population to parents.
 @param permissibleResourceRemains Relative resource remains which used for finding dense blocks of jobs.
 @param swapAndMovePermissibleTimes Premissible number of times the swap and insert mutation procedure will be applied to child shedule.
 */
struct ParamsGA {
    
    int maxGeneratedSchedules;
    int populationSize;
    int maxParents;
    int maxChildren;
    int numberOfChildrenInNextGeneration;
    int timesPingPong;
    float probabilityKP;
    float probabilityParentSelection;
    float permissibleResourceRemains;
    int swapAndMovePermissibleTimes;
    
#pragma mark - table
    
    static string strTitlesForTable()
    {
        stringstream ss;
        
        ss
        << "maxGeneratedSchedules"
        << "\tpopulationSize"
        << "\tmaxParents"
        << "\tmaxChildren"
        << "\tnumberOfChildrenInNextGeneration"
        << "\ttimesPingPong"
        << "\tprobabilityKP"
        << "\tprobabilityParentSelection"
        << "\tpermissibleResourceRemains"
        << "\tswapAndMovePermissibleTimes";
        
        return ss.str();
    }
    
    string strValuesForTable()
    {
        stringstream ss;
        
        ss
        << maxGeneratedSchedules
        << "\t" << populationSize
        << "\t" << maxParents
        << "\t" << maxChildren
        << "\t" << numberOfChildrenInNextGeneration
        << "\t" << timesPingPong
        << "\t" << probabilityKP
        << "\t" << probabilityParentSelection
        << "\t" << permissibleResourceRemains
        << "\t" << swapAndMovePermissibleTimes;
        
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
        << "maxGeneratedSchedules = " << maxGeneratedSchedules
        << ", populationSize = " << populationSize
        << ", maxParents = " << maxParents
        << ", maxChildren = " << maxChildren
        << ", numberOfChildrenInNextGeneration = " << numberOfChildrenInNextGeneration
        << ", timesPingPong = " << timesPingPong
        << ", probabilityKP = " << probabilityKP
        << ", probabilityParentSelection = " << probabilityParentSelection
        << ", permissibleResourceRemains = " << permissibleResourceRemains
        << ", swapAndMovePermissibleTimes = " << swapAndMovePermissibleTimes;
        
        return ss.str();
    }
};



#endif
