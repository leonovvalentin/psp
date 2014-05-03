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
                                      const int insertPermissibleTimes) const
{
    vector<Job *> jobList;
    jobList.reserve(_jobList.size() + 1);
    jobList = _jobList;
    
    long n1, n2;
    for (int i=0; i<2*swapPermissibleTimes; i++) {
        
        n1 = Random :: randomLong(1, jobList.size() - 1);
        long distance = distanceToSuccessor(&jobList, n1);
        
        if (i % 2 == 0 && distance > 1) {
            // Swap
            n2 = Random :: randomLong(n1 + 1, n1 + distance - 1);
            if (distanceToPredecessor(&jobList, n2) > n2 - n1) {
                Job *job1 = jobList[n1];
                Job *job2 = jobList[n2];
                jobList[n1] = job2;
                jobList[n2] = job1;
            }
        }
        else if (distance > 2) {
            // Move
            n2 = Random :: randomLong(n1 + 2, n1 + distance);
            Job *job1 = jobList[n1];
            jobList.insert(jobList.begin() + n2, job1);
            jobList.erase(jobList.begin() + n1);
        }
    }
    
    PActiveList activeList(new ActiveList(&jobList));
    return activeList;
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
