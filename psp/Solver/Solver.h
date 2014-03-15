//
//  Solver.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Solver__
#define __psp2__Solver__



#include "Problem.h"
#include "Schedule.h"

#include <iostream>
#include <vector>
#include <map>

using namespace std;



class Solver
{
private:
    vector<Problem *> _problems;
    map<Problem *, int> _recordsData;
    
public:
    
#pragma mark - init
    /**
     Constructor.
     Create new solver.
     @param path Path to folder with problem files in .RCP format and file with records in .sm format. In folder should be only one file with records. File with records should be in .sm format. Name of problem files should be in following format: ("J30", "J60" or "X")(some numbers)_(some numbers).RCP. Name of file with opts should be in following format: (arbitrary symbols)("opt" or "hrs").(optsFileExtention).
     */
    Solver(string *path);
    virtual ~Solver();
    
#pragma mark - out
    friend ostream & operator<<(ostream &os, const Solver &solver);
    
#pragma mark - getters
    const vector<Problem *> * problems() const;
    int recordForProblem(Problem *problem) const;
    
#pragma mark - functionality
    /**
     Solve problems.
     Solve based on creating early random schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>> solveWithScheduleEarlyRandom(int times);
    /**
     Solve problems.
     Solve based on creating late random schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>> solveWithScheduleLateRandom(int times);
    /**
     Solve problems.
     Solve based on creating early parallel schedules where selection function selects first job from activeList.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solveWithScheduleEarlyParallelSimple(int times);
    /**
     Solve problems.
     Solve based on creating early parallel schedules where selection function based on greedy algorithm for knapsack problem.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solveWithScheduleEarlyParallelKP(int times, float probability);
    /**
     Solve problems.
     Solve based on creating early and late schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>> solveWithSchedulePingPong(int times,
                                                                               float probability);
    /**
     Solve problems.
     Solve based on algorithm of Kochetov and Slolyar (Кочетов, Столяр. Использование чередующихся окрестностей для приближенного решения задачи календарного планирования с ограниченными ресурсами. 2003).
     @param times Number of attempts to build a record for each problem.
     @param probabilityKP Probability for solving knapsack problem.
     @param probabilitySN Probability for creating subset of neighbours.
     @param tabuListSize Length of tabu list.
     @param changingInterval Number of steps of algorithm before changing neighbourhood.
     @param maxIterationNumber Number of iterations to go through the neighborhood. It is stop criteria.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solveWithScheduleKochetovStolyar2003(int times,
                                         float probabilityKP,
                                         float probabilitySN,
                                         int tabuListSize,
                                         int changingInterval,
                                         int maxIterationNumber);
};



#endif /* defined(__psp2__Solver__) */
