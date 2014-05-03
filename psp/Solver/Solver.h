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
#include <sstream>

using namespace std;



struct Solution {
    PSchedule schedule;
    float errorToRecord;
    float errorToCriticalPath;
    time_t calculationTime;
    
    string str()
    {
        stringstream ss;
        
        ss << " duration = " << schedule->duration()
        << " errorToRecord = " << errorToRecord * 100 << "%"
        << " errorToCriticalPath = " << errorToCriticalPath * 100 << "%"
        << " calculationTime = " << calculationTime << "sec."
        << " isValid = " << *schedule->validationDescription();
        
        return ss.str();
    }
};



class Solver
{
private:
    vector<Problem *> _problems;
    map<Problem *, int> _recordsData;
    map<Problem *, int> _criticalPathData;
    
#pragma mark - init
public:
    /**
     Constructor.
     Create new solver.
     @param path Path to folder with problem files in .RCP format and file with records in .sm format. In folder should be only one file with records. File with records should be in .sm format. Name of problem files should be in following format: ("J30", "J60" or "X")(some numbers)_(some numbers).RCP. Name of file with opts should be in following format: (arbitrary symbols)("opt" or "hrs").(optsFileExtention).
     */
    Solver(string *path);
    virtual ~Solver();
    
#pragma mark - out
public:
    friend ostream & operator<<(ostream &os, const Solver &solver);
    
#pragma mark - getters
public:
    const vector<Problem *> * problems() const;
    int recordForProblem(Problem *problem) const;
    
#pragma mark - functionality
public:
    /**
     Solve problems.
     Solve based on creating early random schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>> solveWithScheduleEarlyRandom(int times);
    /**
     Solve problems.
     Solve based on creating late random schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>> solveWithScheduleLateRandom(int times);
    /**
     Solve problems.
     Solve based on creating early parallel schedules where selection function selects first job from activeList.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>> solveWithScheduleEarlyParallelSimple(int times);
    /**
     Solve problems.
     Solve based on creating early parallel schedules where selection function based on greedy algorithm for knapsack problem.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>> solveWithScheduleEarlyParallelKP(int times,
                                                                           float probability);
    /**
     Solve problems.
     Solve based on creating early and late schedules.
     @param times Number of attempts to build a record for each problem.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>> solveWithSchedulePingPong(int times, float probability);
    /**
     Solve problems.
     Solve based on algorithm of Kochetov and Slolyar (Кочетов, Столяр. Использование чередующихся окрестностей для приближенного решения задачи календарного планирования с ограниченными ресурсами. 2003).
     @param probabilityKP Probability for solving knapsack problem.
     @param probabilitySN Probability for creating subset of neighbours.
     @param tabuListSize Length of tabu list.
     @param changingInterval Number of steps of algorithm before changing neighbourhood.
     @param maxIterationNumber Number of iterations to go through the neighborhood. It is stop criteria.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, PSchedule>>
    solveWithScheduleKochetovStolyar2003(float probabilityKP,
                                         float probabilitySN,
                                         int tabuListSize,
                                         int changingInterval,
                                         int maxIterationNumber);
    
    /**
     Solve problems.
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
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, Solution>> solveWithMyGA(int maxGeneratedSchedules,
                                                       int populationSize,
                                                       int maxParents,
                                                       int maxChildren,
                                                       int numberOfChildrenInNextGeneration,
                                                       int timesPingPongInitialPopulation,
                                                       float probabilityKP,
                                                       float probabilityParentSelection,
                                                       float permissibleResourceRemains,
                                                       int swapAndMovePermissibleTimes);
};



#endif /* defined(__psp2__Solver__) */
