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

#include "ParamsKS.h"
#include "ParamsGA.h"

#include <iostream>
#include <vector>

using namespace std;



class Problem
{
private:
    string _name;
    vector <Job *> _jobs;
    vector <Resource *> _resources;
    int _criticalPathDuration;
    
#pragma mark - helper methods
    void logRecordToFile(PSchedule record) const;
    
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
    const int criticalPathDuration();
    
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
    PSchedule schedulePingPong(int times, float probability, int *numberOfGeneratedSchedules) const;
    /**
     Solve problem.
     Solve based on algorithm of Kochetov and Slolyar (Кочетов, Столяр. Использование чередующихся окрестностей для приближенного решения задачи календарного планирования с ограниченными ресурсами. 2003).
     @return Found record.
     */
    PSchedule scheduleKS(ParamsKS params) const;
    /**
     Solve problem.
     Solve based on my genetic algorithm.
     @return Found record.
     */
    PSchedule scheduleGA(ParamsGA params) const;
    /**
     Solve problem.
     Solve based on my genetic algorithm, 2014.
     @return Found record.
     */
#warning remove permissibleResourceRemains from paramsGA, it only from paramsCross is used now
    PSchedule scheduleGA2014(ParamsGA paramsGA,
                             ParamsKS paramsKS,
                             ParamsCross paramsCross,
                             int permissibleNoChangeRecord,
                             int numberOfSubstitutions,
                             int numberOfLocalSearchKS) const;
};



#endif /* defined(__psp2__Problem__) */
