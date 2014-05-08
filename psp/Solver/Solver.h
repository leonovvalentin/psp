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
#include "Solution.h"

#include <iostream>
#include <vector>
#include <map>
#include <sstream>



using namespace std;



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
     @para filter Only problems which will be filtered (problems on which filter returns true) will be considered. This function with two parameters: 1 - numbel of problem in problems list, 2 - name of problem
     */
    Solver(string *path, const function<bool(long, string)> &filter);
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
     @param userInfo User info data will be written to this parameter.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, Solution>> solveWithScheduleKS(ParamsKS params);
    /**
     Solve problems.
     Solve based on my genetic algorithm.
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, Solution>> solveWithScheduleGA(ParamsGA params);
    /**
     Solve problems.
     Solve based on my genetic algorithm, 2014.
     @param permissibleNoChangeRecord Number of iterations without changing record, before trying to apply population thinning.
     @param numberOfSubstitutions Number of schedules which will be substituted to random ping-pong schedules (while population thinning).
     @param numberOfLocalSearchKS Number of schedules which will be improved via local search method of Kochetov Stolyar 2003 () (while population thinning).
     @return Map of problems and found records.
     */
    shared_ptr<map<Problem *, Solution>>
    solveWithScheduleGA2014(ParamsGA paramsGA,
                            ParamsKS paramsKS,
                            ParamsCross paramsCross,
                            int permissibleNoChangeRecord,
                            int numberOfSubstitutions,
                            int numberOfLocalSearchKS);
};



#endif /* defined(__psp2__Solver__) */
