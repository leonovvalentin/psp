//
//  ParamsMyGA.h
//  psp
//
//  Created by Valentin Leonov on 05/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_ParamsMyGA_h
#define psp_ParamsMyGA_h



/**
 Parameters for scheduleMyGA methtod.
 @param maxGeneratedSchedules Max number of generated schedules. It is stop criterion.
 @param populationSize Size of population.
 @param maxParents Number of selected parents in each iteration. It should be greater then 1.
 @param maxChildren Number of generated children for each iteration.
 @param numberOfChildrenInNextGeneration Number of created children which will be added to next generation.
 @param timesPingPongInitialPopulation Number of attempts to build a record in Ping-pong algorithm when we construct schedules for initial population.
 @param probabilityKP Probability for solving knapsack problem.
 @param probabilityParentSelection Probability for selectiong schedule from population to parents.
 @param permissibleResourceRemains Relative resource remains which used for finding dense blocks of jobs.
 @param swapAndMovePermissibleTimes Premissible number of times the swap and insert mutation procedure will be applied to child shedule.
 */
struct ParamsMyGA {
    
    int maxGeneratedSchedules;
    int populationSize;
    int maxParents;
    int maxChildren;
    int numberOfChildrenInNextGeneration;
    int timesPingPongInitialPopulation;
    float probabilityKP;
    float probabilityParentSelection;
    float permissibleResourceRemains;
    int swapAndMovePermissibleTimes;
    
    string strForTable()
    {
        stringstream ss;
        
        ss
        << "maxGeneratedSchedules"
        << "\tpopulationSize"
        << "\tmaxParents"
        << "\tmaxChildren"
        << "\tnumberOfChildrenInNextGeneration"
        << "\ttimesPingPongInitialPopulation"
        << "\tprobabilityKP"
        << "\tprobabilityParentSelection"
        << "\tpermissibleResourceRemains"
        << "\tswapAndMovePermissibleTimes"
        << endl
        << maxGeneratedSchedules
        << "\t" << populationSize
        << "\t" << maxParents
        << "\t" << maxChildren
        << "\t" << numberOfChildrenInNextGeneration
        << "\t" << timesPingPongInitialPopulation
        << "\t" << probabilityKP
        << "\t" << probabilityParentSelection
        << "\t" << permissibleResourceRemains
        << "\t" << swapAndMovePermissibleTimes;
        
        return ss.str();
    }
};



#endif
