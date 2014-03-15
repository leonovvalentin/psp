//
//  Schedule_helperMethods.cpp
//  psp2
//
//  Created by Valentin on 10/10/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Schedule.h"



void Schedule :: addJobsOnScheduleViaEarlyDecoder(const vector<Job *> *jobs)
{
    for (int i=0; i<jobs->size(); i++) {
        Job *job = (*jobs)[i];
        _starts[job] = 0;
        
        for (auto &predecessor : *job->predecessors()) {
            _starts[job] = max(_starts[job], end(predecessor));
        }
        
        for (int time = 0; time < job->duration(); time++) {
            for (auto &pResourceAmount : *job->resourceAmounts()) {
                if (resourceRemain(pResourceAmount.first, _starts[job] + time) <
                    pResourceAmount.second) {
                    _starts[job] += time + 1;
                    time = -1;
                    break;
                }
            }
        }
        
        reduceResourceRemain(job);
    }
}

void Schedule :: addJobsOnScheduleViaLateDecoder(const vector<Job *> *jobs)
{
    for (long i=jobs->size() - 1; i>=0; i--) {
        Job *job = (*jobs)[i];
        _starts[job] = INT_MAX;
        
        for (auto &successor : *job->successors()) {
            _starts[job] = min(_starts[job], _starts[successor] - job->duration());
        }
        
        for (int time = 0; time < job->duration(); time++) {
            for (auto &pResourceAmount : *job->resourceAmounts()) {
                if (resourceRemain(pResourceAmount.first, _starts[job] + time) <
                    pResourceAmount.second) {
                    _starts[job] -= job->duration() - time;
                    time = -1;
                    break;
                }
            }
        }
        
        reduceResourceRemain(job);
    }
}

void Schedule :: addJobsOnScheduleViaEarlyParallelDecoder
(vector<Job *> jobs,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    size_t finalSize = _starts.size() + jobs.size();
    vector<Job *> complited(0); complited.reserve(finalSize);
    vector<Job *> active(0); active.reserve(finalSize);
    vector<Job *> permissibleByPredecessor(0); permissibleByPredecessor.reserve(finalSize);
    
    int time = 0;
    
    while (active.size() + complited.size() < finalSize) {
        calcCompletedAndActiveJobs(time, &complited, &active);
        for (Job *job : jobs) {
            if (job->validByPredecessors(&complited) && !jobInList(job, &permissibleByPredecessor))
                permissibleByPredecessor.push_back(job);
        }
        shared_ptr<vector<Job *>> permissible =
        permissibleJobsByResources(&permissibleByPredecessor, time, true);
        
        while (permissible->size()) {
            shared_ptr<vector<Job *>> newActiveJobs = functionForSelecting(permissible.get(),
                                                                           this,
                                                                           time,
                                                                           true);
            
            for (auto &job : *newActiveJobs) {
                _starts[job] = time;
                reduceResourceRemain(job);
                active.push_back(job);
                permissibleByPredecessor.erase(find(permissibleByPredecessor.begin(),
                                                    permissibleByPredecessor.end(),
                                                    job));
                jobs.erase(find(jobs.begin(), jobs.end(), job));
            }
            permissible = permissibleJobsByResources(&permissibleByPredecessor, time, true);
        }
        
        time = INT_MAX;
        for (auto &job : active) {
            int end = start(job) + job->duration();
            if (end < time) time = end;
        }
    }
}

void Schedule :: addJobsOnScheduleViaLateParallelDecoder
(vector<Job *> jobs,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    size_t finalSize = _starts.size() + jobs.size();
    vector<Job *> started(0); started.reserve(finalSize);
    vector<Job *> active(0); active.reserve(finalSize);
    vector<Job *> permissibleBySuccessors(0); permissibleBySuccessors.reserve(finalSize);
    
    int time = _starts[_activeList.lastJob()];
    
    while (active.size() + started.size() < finalSize) {
        
        calcStartedAndActiveJobs(time, &started, &active);
        
        for (Job *job : jobs) {
            if (job->validBySuccessors(&started) && !jobInList(job, &permissibleBySuccessors))
                permissibleBySuccessors.push_back(job);
        }
        shared_ptr<vector<Job *>> permissible = permissibleJobsByResources(&permissibleBySuccessors,
                                                                           time,
                                                                           false);
        
        while (permissible->size()) {
            shared_ptr<vector<Job *>> newActiveJobs = functionForSelecting(permissible.get(),
                                                                           this,
                                                                           time,
                                                                           false);
            for (auto &job : *newActiveJobs) {
                _starts[job] = time - job->duration();
                reduceResourceRemain(job);
                active.push_back(job);
                permissibleBySuccessors.erase(find(permissibleBySuccessors.begin(),
                                                   permissibleBySuccessors.end(),
                                                   job));
                jobs.erase(find(jobs.begin(), jobs.end(), job));
            }
            permissible = permissibleJobsByResources(&permissibleBySuccessors, time, false);
        }
        
        time = INT_MIN;
        for (auto &job : active) {
            int startOfJob = start(job);
            if (startOfJob > time) time = startOfJob;
        }
    }
}

void Schedule :: calcCompletedAndActiveJobs(int time,
                                            vector<Job *> *completed,
                                            vector<Job *> *active)
{
    completed->clear(); active->clear();
    for (auto &pJobStart : _starts) {
        Job *job = pJobStart.first;
        int start = pJobStart.second;
        int end = start + job->duration();
        if (end <= time) completed->push_back(job);
        else if (start <= time) active->push_back(job);
    }
}

void Schedule :: calcStartedAndActiveJobs(int time,
                                          vector<Job *> *started,
                                          vector<Job *> *active)
{
    started->clear(); active->clear();
    for (auto &pJobStart : _starts) {
        Job *job = pJobStart.first;
        int start = pJobStart.second;
        int end = start + job->duration();
        if (start >= time) started->push_back(job);
        else if (end >= time) active->push_back(job);
    }
}

shared_ptr<vector<Job *>> Schedule :: incomingNetwork(Job *job)
{
    shared_ptr<vector<Job *>> net(new vector<Job *>(1, job)); net->reserve(_activeList.size());
    vector<Job *> front(1, job); front.reserve(_activeList.size());
    
    for (int i=0; i<front.size(); i++) {
        Job *frontJob = front[i];
        int frontJobStart = start(frontJob);
        for (Job *predicessor : *frontJob->predecessors()) {
            if (end(predicessor) == frontJobStart) {
                front.push_back(predicessor);
                if (!jobInList(predicessor, net.get())) net->push_back(predicessor);
            }
        }
    }
    
    return net;
}

shared_ptr<vector<Job *>> Schedule :: outgoingNetwork(Job *job)
{
    shared_ptr<vector<Job *>> net(new vector<Job *>(1, job)); net->reserve(_activeList.size());
    vector<Job *> front(1, job); front.reserve(_activeList.size());
    
    for (int i=0; i<front.size(); i++) {
        Job *frontJob = front[i];
        int frontJobEnd = end(frontJob);
        for (Job *successor : *frontJob->successors()) {
            if (_starts[successor] == frontJobEnd) {
                front.push_back(successor);
                if (!jobInList(successor, net.get())) net->push_back(successor);
            }
        }
    }
    
    return net;
}

shared_ptr<vector<Job *>> Schedule :: blockOfJobs(Job *job,
                                                  bool withPredecessors,
                                                  bool withSuccessors)
{
    shared_ptr<vector<Job *>> block(new vector<Job *>); block->reserve(_activeList.size());
    
    int jobStart = _starts[job];
    int jobEnd = jobStart + job->duration();
    
    for (auto &pJobStart : _starts) {
        int start = pJobStart.second;
        int end = start + pJobStart.first->duration();
#warning is it right check for block jobs?
        if (start <= jobEnd && end >= jobStart) {
            if (!withPredecessors && job->hasPredecessor(pJobStart.first))
                return shared_ptr<vector<Job *>>(nullptr);
            if (!withSuccessors && job->hasSuccessor(pJobStart.first))
                return shared_ptr<vector<Job *>>(nullptr);
            block->push_back(pJobStart.first);
        }
    }
    
    return block;
}

void Schedule :: reduceResourceRemain(Job *job)
{
    for (auto &pResourceAmount : *job->resourceAmounts()) {
        if (pResourceAmount.second != 0) {
            for (int time=0; time<job->duration(); time++) {
                (*_resourceRemains[pResourceAmount.first])[_starts[job] + time] -=
                pResourceAmount.second;
            }
        }
    }
}

shared_ptr<vector<Job *>> Schedule :: permissibleJobsByResources(vector<Job *> *jobs,
                                                                 int time,
                                                                 bool timeForStart)
{
    shared_ptr<vector<Job *>> permissible(new vector<Job *>(0)); permissible->reserve(jobs->size());
    
    for (auto &job : *jobs) {
        int timeStart = timeForStart ? time : time - job->duration();
        bool isPermissible = true;
        for (auto &pResourceAmount : *job->resourceAmounts()) {
            for (int t=0; t<job->duration(); t++) {
                if (pResourceAmount.second >
                    resourceRemain(pResourceAmount.first, timeStart + t)) {
                    isPermissible = false;
                    break;
                }
            }
            if (!isPermissible) break;
        }
        if (isPermissible) permissible->push_back(job);
    }
    
    return permissible;
}

void Schedule :: shift(int time)
{
    for (auto &pJobStart : _starts) pJobStart.second += time;
    for (auto &pResourceRemain : _resourceRemains) {
        auto remain = pResourceRemain.second;
        if (time < 0) {
            int t = -time;
            remain->erase(remain->begin(), remain->begin() + t);
            remain->insert(remain->end(), t, pResourceRemain.first->amount());
        }
        else {
            remain->erase(remain->end() - time, remain->end());
            remain->insert(remain->begin(), time, pResourceRemain.first->amount());
        }
    }
}

shared_ptr<string> Schedule :: validationDescription()
{
    shared_ptr<string> result(nullptr);
    ScheduleValid valid = validation();
    switch (valid) {
        case ScheduleValidOK:{
            result = shared_ptr<string>(new string("OK"));
            break;
        }
        case ScheduleValidNoResourceRestriction:{
            result =
            shared_ptr<string>(new string("ERROR " + to_string(valid) + " No resource restriction"));
            break;
        }
        case ScheduleValidNoPresedenceRelation:{
            result =
            shared_ptr<string>(new string("ERROR " + to_string(valid) + " No presedence relation"));
            break;
        }
    }
    return result;
}

int Schedule :: sumOfStarts()
{
    int sum = 0;
    for (auto &pJobStart : _starts) {
        sum += pJobStart.second;
    }
    return sum;
}
