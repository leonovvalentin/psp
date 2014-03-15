//
//  Job.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Job.h"
#include "utils.h"



#pragma mark - init

Job :: Job(string *name, int duration, map<Resource *, int> *resourceAmounts)
{
    _name = *name;
    _duration = duration;
    _resourceAmounts = *resourceAmounts;
    _predecessors = vector<Job *>(0);
    _successors = vector<Job *>(0);
    
    _resourceAmountsSpecific = 0.0f;
    for (auto &pResourceAmount : _resourceAmounts) {
        _resourceAmountsSpecific +=
        (float)pResourceAmount.second / pResourceAmount.first->amount();
    }
}

#pragma mark - out

ostream & operator<<(ostream &os, const Job &job)
{
    os << "Job with name: " << *job.name()
    << " duration: " << job.duration()
    << " resource amounts:";
    
    for (auto &pResourceAmount : *job.resourceAmounts()) {
        os << " " << *pResourceAmount.first->name() << "-" << pResourceAmount.second;
    }
    
    os << endl << "predecessors:";
    
    for (auto &predecessor : *job.predecessors()) {
        os << " " << *predecessor->name();
    }
    
    os << endl << "successors:";
    
    for (auto &successor : *job.successors()) {
        os << " " << *successor->name();
    }
    
    return os;
}

#pragma mark - getters

const string * Job :: name() const
{
    return &_name;
}

int Job :: duration() const
{
    return _duration;
}

const map<Resource *, int> * Job :: resourceAmounts() const
{
    return &_resourceAmounts;
}

int Job :: resourceAmount(Resource *resource) const
{
    auto iterator = _resourceAmounts.find(resource);
    if (iterator != _resourceAmounts.end()) return iterator->second;
    return 0;
}

const vector<Job *> * Job :: predecessors() const
{
    return &_predecessors;
}

const vector<Job *> * Job :: successors() const
{
    return &_successors;
}

float Job :: resourceAmountSpecific() const
{
    return _resourceAmountsSpecific;
}

#pragma functionality

void Job :: addPredecessor(Job *job)
{
    if (!jobInList(job, &_predecessors)) _predecessors.push_back(job);
}

void Job :: addSuccessor(Job *job)
{
    if (!jobInList(job, &_successors)) _successors.push_back(job);
}

bool Job :: hasPredecessor(Job *job)
{
    if (jobInList(job, &_predecessors)) return true;
    else return false;
}

bool Job :: hasSuccessor(Job *job)
{
    if (jobInList(job, &_successors)) return true;
    else return false;
}

bool Job :: validByPredecessors(vector<Job *> *jobs)
{
    bool isValid = true;
    for (auto &job : _predecessors) {
        if (!jobInList(job, jobs)) {
            isValid = false;
            break;
        }
    }
    return isValid;
}

bool Job :: validBySuccessors(vector<Job *> *jobs)
{
    bool isValid = true;
    for (auto &job : _successors) {
        if (!jobInList(job, jobs)) {
            isValid = false;
            break;
        }
    }
    return isValid;
}
