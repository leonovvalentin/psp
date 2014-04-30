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
    shared_ptr<Schedule> scheduleEarlyWithRandom(int times) const;
    /**
     Solve problem.
     Solve based on creating random late schedules.
     @param times Number of attempts to build a record.
     @return Found record.
     */
    shared_ptr<Schedule> scheduleLateWithRandom(int times) const;
    /**
     Solve problem.
     Solve based on creating early parallel schedules where selection function selects first job from activeList.
     @param times Number of attempts to build a record.
     @return Found record.
     */
    shared_ptr<Schedule> scheduleEarlyParallelSimple(int times) const;
    /**
     Solve problem.
     Solve based on creating early parallel schedules where selection function based on greedy algorithm for knapsack problem.
     @param times Number of attempts to build a record.
     @param probability Probability for solving knapsack problem.
     @return Found record.
     */
    shared_ptr<Schedule> scheduleEarlyParallelKP(int times, float probability) const;
    /**
     Solve problem.
     Solve based on creating early and late schedules.
     @param times Number of attempts to build a record.
     @param probability Probability for solving knapsack problem.
     @return Found record.
     */
    shared_ptr<Schedule> schedulePingPong(int times, float probability) const;
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
    shared_ptr<Schedule> scheduleKochetovStolyar2003(float probabilityKP,
                                                     float probabilitySN,
                                                     int tabuListSize,
                                                     int changingInterval,
                                                     int maxIterationNumber) const;
    /**
     Solve problem.
     Solve based on my genetic algorithm.
     @return Found record.
     */
    shared_ptr<Schedule> scheduleMyGA() const;
};



#endif /* defined(__psp2__Problem__) */
