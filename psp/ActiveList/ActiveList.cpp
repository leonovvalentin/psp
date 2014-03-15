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
