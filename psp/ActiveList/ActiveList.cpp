//
//  ActiveList.cpp
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "ActiveList.h"
#include "Random.h"
#include "utils.h"

#include <sstream>



#pragma mark - init

ActiveList :: ActiveList()
{
    _jobList = vector<Job *>(0);
    _duration = 0;
}

ActiveList :: ActiveList(Job *source, unsigned long jobsCapacity)
{
    _duration = 0;
    _jobList = vector<Job *>(0); _jobList.reserve(jobsCapacity);
    _jobList.push_back(source);
    vector<Job *> activeFront(*source->successors());
    
    while (activeFront.size()) {
        int numberOfJob = Random().randomInt(0, (int)activeFront.size() - 1);
        Job *job = activeFront[numberOfJob];
        _jobList.push_back(job);
        activeFront.erase(activeFront.begin() + numberOfJob);
        
        for (auto &successor : *job->successors()) {
            if (successor->validByPredecessors(&_jobList)) activeFront.push_back(successor);
        }
    }
}

ActiveList :: ActiveList(const vector<Job *> *jobList)
{
    _duration = 0;
    _jobList = *jobList;
}

#pragma mark - interface

PActiveList ActiveList :: swapAndMove(const int swapPermissibleTimes,
                                      const int movePermissibleTimes) const
{
    vector<Job *> jobList;
    jobList.reserve(_jobList.size() + 1);
    jobList = _jobList;
    
    long n1, n2;
    
    // Swap
    
    int swapMax = Random :: randomInt(0, swapPermissibleTimes);
    for (int i=0; i<swapMax; i++) {
        
        n1 = Random :: randomLong(1, jobList.size() - 1);
        long distance = distanceToSuccessor(&jobList, n1);
        
        if (distance > 1) {
            n2 = Random :: randomLong(n1 + 1, n1 + distance - 1);
            if (distanceToPredecessor(&jobList, n2) > n2 - n1) {
                Job *job1 = jobList[n1];
                Job *job2 = jobList[n2];
                jobList[n1] = job2;
                jobList[n2] = job1;
                continue;
            }
        }
        
        // If no swap, try again
        i--;
    }
    
    // Move
    
    int moveMax = Random :: randomInt(0, movePermissibleTimes);
    for (int i=0; i<moveMax; i++) {
        
        n1 = Random :: randomLong(1, jobList.size() - 1);
        long distance = distanceToSuccessor(&jobList, n1);
        
        if (distance > 2) {
            n2 = Random :: randomLong(n1 + 2, n1 + distance);
            Job *job1 = jobList[n1];
            jobList.insert(jobList.begin() + n2, job1);
            jobList.erase(jobList.begin() + n1);
            continue;
        }
        
        // If no move, try again
        i--;
    }
    
    PActiveList activeList(new ActiveList(&jobList));
    return activeList;
}

int ActiveList :: hammingDistance(const ActiveList *activeList, const int dispersion) const
{
    int d = 0;
    for (int i=0; i<_jobList.size(); i++) {
        auto it1 = _jobList.begin() + i;
        auto it2 = activeList->_jobList.begin() + i;
        bool isEqual = false;
        for (auto it=it2-dispersion; it<=it2+dispersion; it++) {
            if (*it1 == *it) {
                isEqual = true;
                break;
            }
        }
        if (!isEqual) d++;
    }
    return d;
}

#pragma mark - out

ostream & operator<<(ostream &os, const ActiveList &activeList)
{
    os << "ActiveList with jobs list:";
    for (auto &job : *activeList.jobList()) os << " " << *job->name();
    return os;
}

string ActiveList :: stringJobList() const
{
    stringstream ss; for (auto &job : _jobList) ss << *job->name() << " ";
    return ss.str();
}

#pragma mark - getters

const vector<Job *> * ActiveList :: jobList() const
{
    return &_jobList;
}

int ActiveList :: duration()
{
    if (_duration == 0) for (auto &job : _jobList) _duration += job->duration();
    return _duration;
}

unsigned long ActiveList :: size() const
{
    return _jobList.size();
}

Job * ActiveList :: operator[](unsigned long index) const
{
    return _jobList[index];
}

Job * ActiveList :: firstJob() const
{
    return _jobList[0];
}

Job * ActiveList :: lastJob() const
{
    return _jobList[_jobList.size() - 1];
}
