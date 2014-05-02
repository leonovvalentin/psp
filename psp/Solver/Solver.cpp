//
//  Solver.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Solver.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <dirent.h>



const string problemFileExtention = "RCP";
const string recordsFileExtention = "sm";



#pragma mark - init

Solver :: Solver(string *path)
{
    // Names and full paths to problem files
    
    map<pair<int, int>, string> problemPaths;
    string recordsPath;
    
    bool optsIsHrsFile = false;
    
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(path->c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string entName(ent->d_name);
            if (entName != "." && entName != "..") {
                if (!entName.compare(entName.length() - problemFileExtention.length(),
                                     problemFileExtention.length(),
                                     problemFileExtention)) {
                    size_t positionOfIDFirst;
                    if (!entName.compare(0, 3, "J30") || !entName.compare(0, 3, "J60")) {
                        positionOfIDFirst = 3;
                    }
                    else {
                        positionOfIDFirst = 1;
                    }
                    
                    size_t positionOfUnderline = entName.find("_");
                    size_t positionOfDot = entName.find(".");
                    size_t lengthOfIDFirst = positionOfUnderline - positionOfIDFirst;
                    size_t lengthOfIDSecond = positionOfDot - positionOfUnderline;
                    
                    pair<int, int> problemID;
                    istringstream(entName.substr(positionOfIDFirst, lengthOfIDFirst))
                    >> problemID.first;
                    istringstream(entName.substr(positionOfUnderline + 1, lengthOfIDSecond))
                    >> problemID.second;
                    
                    problemPaths[problemID] = *path + "/" + entName;
                }
                else if (!entName.compare(entName.length() - recordsFileExtention.length(),
                                          recordsFileExtention.length(),
                                          recordsFileExtention)) {
                    recordsPath = *path + "/" + entName;
                    size_t positionOfDot = entName.find(".");
                    if (!entName.compare(positionOfDot - 3, 3, "hrs")) optsIsHrsFile = true;
                }
            }
        }
        
        closedir (dir);
    }
    
    // Opts map
    
    map<pair<int, int>, int> optsMap;
    ifstream optsData(recordsPath);
    int firstLine = optsIsHrsFile ? 4 : 22;
    if (optsData.is_open()) {
        string tempStr; for (int i = 0; i < firstLine; i++) getline(optsData, tempStr);
        while (getline(optsData, tempStr)) {
            pair<int, int> problemID;
            istringstream tempStrStream = istringstream(tempStr);
            tempStrStream >> problemID.first;
            tempStrStream >> problemID.second;
            tempStrStream >> optsMap[problemID];
        }
        optsData.close();
    }
    
    // Problems and opts
    
    _problems = vector<Problem *>(0);
    
    for (auto &pIDPath : problemPaths) {
        // Problem
        string problemName =
        to_string(pIDPath.first.first) + "_" + to_string(pIDPath.first.second);
        _problems.push_back(new Problem(&problemName, &pIDPath.second));
        _recordsData[_problems[_problems.size() - 1]] = optsMap[pIDPath.first];
    }
}

Solver :: ~Solver()
{
    for (auto &problem : _problems) {
        delete problem;
    }
}

#pragma mark - out

ostream & operator<<(ostream &os, const Solver &solver)
{
    os << "Solver with problems:";
    for (auto &problem : *solver.problems()) os << " " << *problem->name();
    return os;
}

#pragma mark - getters

const vector<Problem *> * Solver :: problems() const
{
    return &_problems;
}

int Solver :: recordForProblem(Problem *problem) const
{
    auto iterator = _recordsData.find(problem);
    if (iterator != _recordsData.end()) return iterator->second;
    return INT_MAX;
}

#pragma mark - functionality

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver :: solveWithScheduleEarlyRandom(int times)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) (*solve)[problem] = problem->scheduleEarlyWithRandom(times);
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver :: solveWithScheduleLateRandom(int times)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) (*solve)[problem] = problem->scheduleLateWithRandom(times);
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver ::
solveWithScheduleEarlyParallelSimple(int times)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) (*solve)[problem] = problem->scheduleEarlyParallelSimple(times);
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver ::
solveWithScheduleEarlyParallelKP(int times, float probability)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) {
        (*solve)[problem] = problem->scheduleEarlyParallelKP(times, probability);
    }
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver ::
solveWithSchedulePingPong(int times, float probability)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) {
        (*solve)[problem] = problem->schedulePingPong(times, probability);
    }
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver ::
solveWithScheduleKochetovStolyar2003(float probabilityKP,
                                     float probabilitySN,
                                     int tabuListSize,
                                     int changingInterval,
                                     int maxIterationNumber)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) {
        (*solve)[problem] = problem->scheduleKochetovStolyar2003(probabilityKP,
                                                                 probabilitySN,
                                                                 tabuListSize,
                                                                 changingInterval,
                                                                 maxIterationNumber);
    }
    return solve;
}

shared_ptr<map<Problem *, shared_ptr<Schedule>>> Solver ::
solveWithMyGA(int maxGeneratedSchedules,
              int populationSize,
              int maxParents,
              int maxChildren,
              int timesPingPongInitialPopulation,
              float probabilityKP,
              float probabilityParentSelection,
              float permissibleResourceRemains,
              int swapAndMovePermissibleTimes)
{
    shared_ptr<map<Problem *, shared_ptr<Schedule>>>
    solve(new map<Problem *, shared_ptr<Schedule>>);
    for (auto &problem : _problems) {
        (*solve)[problem] = problem->scheduleMyGA(maxGeneratedSchedules,
                                                  populationSize,
                                                  maxParents,
                                                  maxChildren,
                                                  timesPingPongInitialPopulation,
                                                  probabilityKP,
                                                  probabilityParentSelection,
                                                  permissibleResourceRemains,
                                                  swapAndMovePermissibleTimes);
    }
    return solve;
}
