//
//  Problem.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Problem__
#define __psp2__Problem__



#include "Job.h"
#include "Resource.h"
#include "Schedule.h"

#include <iostream>
#include <vector>

using namespace std;



class Problem
{
private:
    string _name;
    vector <Job *> _jobs;
    vector <Resource *> _resources;
    
public:
    
#pragma mark - init
    /**
     Constructor.
     Create new problem from .RCP-file.
     @param name Name.
     @param path Path to input data file in .RCP format.
     */
    Problem(string *name, string *path);
    virtual ~Problem();
    
#pragma mark - out
    friend ostream & operator<<(ostream &os, const Problem &problem);
    string stringInRCPFormat() const;
    string stringRelationshipJobsInGVFormat() const;
    
#pragma mark - getters
    const string * name() const;
    
#pragma mark - functionality
    /**
     Solve problem.
     Solve based on creating random active schedules.
     @param times Number of attempts to build a record.
     @return Found record.
     */
    PSchedule scheduleEarlyWithRandom(int times) const;
    /**
     Solve problem.
     Solve based on creating random late schedules.
     @param times Number of attempts to build a record.
     @return Found record.
     */
    PSchedule scheduleLateWithRandom(int times) const;
    /**
     Solve problem.
     Solve based on creating early parallel schedules where selection function selects first job from activeList.
     @param times Number of attempts to build a record.
     @return Found record.
     */
    PSchedule scheduleEarlyParallelSimple(int times) const;
    /**
     Solve problem.
     Solve based on creating early parallel schedules where selection function based on greedy algorithm for knapsack problem.
     @param times Number of attempts to build a record.
     @param probability Probability for solving knapsack problem.
     @return Found record.
     */
    PSchedule scheduleEarlyParallelKP(int times, float probability) const;
    /**
     Solve problem.
     Solve based on creating early and late schedules.
     @param times Number of attempts to build a record.
     @param probability Probability for solving knapsack problem.
     @return Found record.
     */
    PSchedule schedulePingPong(int times, float probability) const;
    /**
     Solve problem.
     Solve based on algorithm of Kochetov and Slolyar (Кочетов, Столяр. Использование чередующихся окрестностей для приближенного решения задачи календарного планирования с ограниченными ресурсами. 2003).
     @param probabilityKP Probability for solving knapsack problem.
     @param probabilitySN Probability for creating subset of neighbours.
     @param tabuListSize Length of tabu list.
     @param changingInterval Number of steps of algorithm before changing neighbourhood.
     @param maxIterationNumber Number of iterations to go through the neighborhood. It is stop criteria.
     @return Found record.
     */
    PSchedule scheduleKochetovStolyar2003(float probabilityKP,
                                          float probabilitySN,
                                          int tabuListSize,
                                          int changingInterval,
                                          int maxIterationNumber) const;
    /**
     Solve problem.
     Solve based on my genetic algorithm.
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
     @return Found record.
     */
    PSchedule scheduleMyGA(int maxGeneratedSchedules,
                           int populationSize,
                           int maxParents,
                           int maxChildren,
                           int numberOfChildrenInNextGeneration,
                           int timesPingPongInitialPopulation,
                           float probabilityKP,
                           float probabilityParentSelection,
                           float permissibleResourceRemains,
                           int swapAndMovePermissibleTimes) const;
};



#endif /* defined(__psp2__Problem__) */
