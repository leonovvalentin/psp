//
//  Schedule.cpp
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Schedule.h"
#include "utils.h"



#pragma mark - init

Schedule :: Schedule(ActiveList *activeList, const vector<Resource *> *resources)
{
    _type = ScheduleTypeUnknown;
    _activeList = *activeList;
    _resources = resources;
    _duration = 0;
    if (resources) {
        for (auto &resource : *resources) {
            _resourceRemains[resource] =
            shared_ptr<vector<int>>(new vector<int>(activeList->duration(), resource->amount()));
        }
    }
}

PSchedule Schedule :: schedulePartialyEarlyParallel
(ActiveList *activeList,
 const vector<Resource *> *resources,
 vector<Job *> :: const_iterator minIterator,
 vector<Job *> :: const_iterator maxIterator,
 const map<Job *, int> *starts,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    PSchedule schedule(new Schedule(activeList, resources));
    schedule->_type = ScheduleTypeEarlyComposite;
    
    vector<Job *> segment1(activeList->jobList()->begin(), minIterator);
    vector<Job *> segment2(minIterator, maxIterator + 1);
    vector<Job *> segment3(maxIterator + 1, activeList->jobList()->end());
    
    // segment1
    
    for (Job *job : segment1) {
        auto iterator = starts->find(job);
        if (iterator != starts->end()) schedule->_starts[job] = iterator->second;
        else return PSchedule(nullptr);
        schedule->reduceResourceRemain(job);
    }
    
    // segment 2
    
    schedule->addJobsOnScheduleViaEarlyParallelDecoder(segment2, functionForSelecting);
    
    // segment 3
    
    schedule->addJobsOnScheduleViaEarlyDecoder(&segment3);
    
    return schedule;
}

PSchedule Schedule :: schedulePartialyLateParallel
(ActiveList *activeList,
 const vector<Resource *> *resources,
 vector<Job *> :: const_iterator minIterator,
 vector<Job *> :: const_iterator maxIterator,
 const map<Job *, int> *starts,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    PSchedule schedule(new Schedule(activeList, resources));
    schedule->_type = ScheduleTypeLateComposite;
    
    vector<Job *> segment1(activeList->jobList()->begin(), minIterator);
    vector<Job *> segment2(minIterator, maxIterator + 1);
    vector<Job *> segment3(maxIterator + 1, activeList->jobList()->end());
    
    // segment3
    
    for (Job *job : segment3) {
        auto iterator = starts->find(job);
        if (iterator != starts->end()) schedule->_starts[job] = iterator->second;
        else return PSchedule(nullptr);
        schedule->reduceResourceRemain(job);
    }
    
    // Move schedule to ending at far time
    
    int maxEnd = INT_MIN;
    for (auto &jobStart : schedule->_starts) {
        int end = jobStart.second + jobStart.first->duration();
        if (end > maxEnd) maxEnd = end;
    }
    schedule->shift(activeList->duration() - maxEnd);
    
    // segment 2
    
    schedule->addJobsOnScheduleViaLateParallelDecoder(segment2, functionForSelecting);
    
    // segment 1
    
    schedule->addJobsOnScheduleViaLateDecoder(&segment1);
    
    // Move schedule to starting at zero time
    
    int start = INT_MAX;
    for (auto &jobStart : schedule->_starts) {
        if (jobStart.second < start) start = jobStart.second;
    }
    schedule->shift(-start);
    
    return schedule;
}

PSchedule Schedule :: scheduleEarlyWithoutResources(ActiveList *activeList)
{
    PSchedule schedule(new Schedule(activeList, NULL));
    schedule->_type = ScheduleTypeEarly;
    
    for (int i=0; i<activeList->size(); i++) {
        Job *job = (*activeList)[i];
        schedule->_starts[job] = 0;
        for (auto &predecessor : *job->predecessors()) {
            schedule->_starts[job] = max(schedule->start(job), schedule->end(predecessor));
        }
    }
    
    return schedule;
}

PSchedule Schedule :: scheduleEarly(ActiveList *activeList,
                                    const vector<Resource *> *resources)
{
    PSchedule schedule(new Schedule(activeList, resources));
    schedule->_type = ScheduleTypeEarly;
    
    for (int i=0; i<activeList->size(); i++) {
        Job *job = (*activeList)[i];
        schedule->_starts[job] = 0;
        
        for (auto &predecessor : *job->predecessors()) {
            schedule->_starts[job] = max(schedule->start(job), schedule->end(predecessor));
        }
        
        for (int time = 0; time < job->duration(); time++) {
            for (auto &pResourceAmount : *job->resourceAmounts()) {
                if (schedule->resourceRemain(pResourceAmount.first, schedule->start(job) + time) <
                    pResourceAmount.second) {
                    schedule->_starts[job] += time + 1;
                    time = -1;
                    break;
                }
            }
        }
        
        schedule->reduceResourceRemain(job);
    }
    
    return schedule;
}

PSchedule Schedule :: scheduleLate(ActiveList *activeList,
                                   const vector<Resource *> *resources)
{
    PSchedule schedule(new Schedule(activeList, resources));
    schedule->_type = ScheduleTypeLate;
    
    for (long i=activeList->size()-1; i>=0; i--) {
        Job *job = (*activeList)[i];
        schedule->_starts[job] = activeList->duration();
        
        for (auto &successor : *job->successors()) {
            schedule->_starts[job] = min(schedule->start(job),
                                         schedule->start(successor) - job->duration());
        }
        
        for (int time = 0; time < job->duration(); time++) {
            for (auto &pResourceAmount : *job->resourceAmounts()) {
                if (schedule->resourceRemain(pResourceAmount.first, schedule->start(job) + time) <
                    pResourceAmount.second) {
                    schedule->_starts[job] -= job->duration() - time;
                    time = -1;
                    break;
                }
            }
        }
        
        schedule->reduceResourceRemain(job);
    }
    
    // Move schedule to starting at zero time
    
    int start = schedule->start(activeList->firstJob());
    schedule->shift(-start);
    
    return schedule;
}

PSchedule Schedule :: scheduleEarlyParallel
(ActiveList *activeList,
 const vector<Resource *> *resources,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    PSchedule schedule(new Schedule(activeList, resources));
    schedule->_type = ScheduleTypeEarlyParallel;
    
    int time = 0;
    schedule->_starts[activeList->firstJob()] = 0;
    vector<Job *> minActiveEndJobs(0); minActiveEndJobs.reserve(activeList->size());
    vector<Job *> complited(1, activeList->firstJob()); complited.reserve(activeList->size());
    vector<Job *> active(0); active.reserve(activeList->size());
    vector<Job *> permissibleByPredecessor(*activeList->firstJob()->successors());
    permissibleByPredecessor.reserve(activeList->size());
    
    while (active.size() + complited.size() < activeList->size()) {
        shared_ptr<vector<Job *>> permissible =
        schedule->permissibleJobsByResources(&permissibleByPredecessor, time, true);
        
        while (permissible->size()) {
            shared_ptr<vector<Job *>> newActiveJobs = functionForSelecting(permissible.get(),
                                                                           schedule.get(),
                                                                           time,
                                                                           true);
            
            for (auto &job : *newActiveJobs) {
                schedule->_starts[job] = time;
                schedule->reduceResourceRemain(job);
                active.push_back(job);
                permissibleByPredecessor.erase(find(permissibleByPredecessor.begin(),
                                                    permissibleByPredecessor.end(),
                                                    job));
            }
            
            permissible = schedule->permissibleJobsByResources(&permissibleByPredecessor,
                                                               time,
                                                               true);
        }
        
        time = INT_MAX;
        minActiveEndJobs.clear();
        for (auto &job : active) {
            int end = schedule->start(job) + job->duration();
            if (end <= time) {
                if (end != time) {
                    time = end;
                    minActiveEndJobs.clear();
                }
                minActiveEndJobs.push_back(job);
            }
        }
        
        complited.insert(complited.end(), minActiveEndJobs.begin(), minActiveEndJobs.end());
        for (auto &job : minActiveEndJobs) {
            for (auto &successor : *job->successors()) {
                if (successor->validByPredecessors(&complited)) {
                    if (!jobInList(successor, &permissibleByPredecessor)) {
                        permissibleByPredecessor.push_back(successor);
                    }
                }
            }
            active.erase(find(active.begin(), active.end(), job));
        }
    }
    
    return schedule;
}

#pragma mark - getters

ScheduleType Schedule :: type() const
{
    return _type;
}

const ActiveList * Schedule :: activeList() const
{
    return &_activeList;
}

int Schedule :: duration()
{
    if (_duration == 0) {
        switch (_type) {
            case ScheduleTypeEarly:
            case ScheduleTypeLate: {
                _duration = _starts[_activeList.lastJob()] + _activeList.lastJob()->duration();
                break;
            }
            default: {
                for (auto &pJobStart : _starts) {
                    int end = pJobStart.second + pJobStart.first->duration();
                    if (_duration < end) _duration = end;
                }
                break;
            }
        }
    }
    return _duration;
}

int Schedule :: start(Job *job) const
{
    auto iterator = _starts.find(job);
    if (iterator != _starts.end()) return iterator->second;
    return INT_MAX;
}

int Schedule :: end(Job *job) const
{
    auto iterator = _starts.find(job);
    if (iterator != _starts.end()) return iterator->second + job->duration();
    return INT_MAX;
}

int Schedule :: resourceRemain(Resource *resource, int timeMoment) const
{
    auto iterator = _resourceRemains.find(resource);
    if (iterator != _resourceRemains.end()) return (*iterator->second)[timeMoment];
    else return 0;
}

shared_ptr<map<Resource *, int>> Schedule :: resourceRemain(int timeMoment) const
{
    shared_ptr<map<Resource *, int>> remains(new map<Resource *, int>());
    for (auto &pResourceRemains : _resourceRemains) {
        (*remains)[pResourceRemains.first] = (*pResourceRemains.second)[timeMoment];
    }
    return remains;
}

const map<Resource *, shared_ptr<vector<int>>> * Schedule :: resourceRemains() const
{
    return &_resourceRemains;
}

#pragma mark - functionality

PSchedule Schedule :: pingPong() const
{
    PSchedule schedule = earlySchedule();
    
    bool stop = false;
    while (!stop) {
        PSchedule scheduleEarly = schedule->lateSchedule()->earlySchedule();
        if (scheduleEarly->duration() < schedule->duration()) schedule = scheduleEarly;
        else stop = true;
    }
    
    return schedule;
}

PSchedule Schedule :: swapAndMoveMutation(const int swapPermissibleTimes,
                                          const int movePermissibleTimes) const
{
    PActiveList mutatedActiveList =
    _activeList.swapAndMove(swapPermissibleTimes, movePermissibleTimes);
    
    return scheduleEarly(mutatedActiveList.get(), _resources);
}

PSchedule Schedule :: cross(PSchedule schedule, float permissibleResourceRemains)
{
    auto denseJobsBlocks = this->denseJobsBlocks(permissibleResourceRemains);
    auto denseJobsBlocks2 = schedule->denseJobsBlocks(permissibleResourceRemains);
    
    auto jobsList = *_activeList.jobList();
    auto jobsList2 = *schedule->activeList()->jobList();
    vector<Job *> childJobsList(0);
    
    while (denseJobsBlocks->size() != 0 || denseJobsBlocks2->size() != 0) {
        
        shared_ptr<pair<shared_ptr<vector<Job *>>, float>> bestBlock = nullptr;
        vector<Job *> *bestJobsList = NULL;
        
        shared_ptr<vector<shared_ptr<pair<shared_ptr<vector<Job *>>, float>>>>
        bestDenseJobsBlocks = nullptr, otherDenseJobsBlocks = nullptr;
        
        if (denseJobsBlocks->size() != 0 && denseJobsBlocks2->size() == 0) {
            bestBlock = (*denseJobsBlocks)[0];
            bestJobsList = &jobsList;
            bestDenseJobsBlocks = denseJobsBlocks;
            otherDenseJobsBlocks = denseJobsBlocks2;
        }
        else if (denseJobsBlocks->size() == 0 && denseJobsBlocks2->size() != 0) {
            bestBlock = (*denseJobsBlocks2)[0];
            bestJobsList = &jobsList2;
            bestDenseJobsBlocks = denseJobsBlocks2;
            otherDenseJobsBlocks = denseJobsBlocks;
        }
        else {
            auto block = (*denseJobsBlocks)[0], block2 = (*denseJobsBlocks2)[0];
            if (block->second < block2->second) {
                bestBlock = block;
                bestJobsList = &jobsList;
                bestDenseJobsBlocks = denseJobsBlocks;
                otherDenseJobsBlocks = denseJobsBlocks2;
            }
            else {
                bestBlock = block2;
                bestJobsList = &jobsList2;
                bestDenseJobsBlocks = denseJobsBlocks2;
                otherDenseJobsBlocks = denseJobsBlocks;
            }
        }
        
        while (bestJobsList->size()) {
            
            Job *job = (*bestJobsList)[0];
            
            childJobsList.push_back(job);
            
            removeJobFromList(job, &jobsList);
            removeJobFromList(job, &jobsList2);
            
            removeJobFromList(job, bestBlock->first.get());
            
            // remove blocks with current job
            for (int j=0; j<bestDenseJobsBlocks->size();) {
                auto b = (*bestDenseJobsBlocks)[j];
                if (b != bestBlock && jobInList(job, b->first.get())) {
                    bestDenseJobsBlocks->erase(bestDenseJobsBlocks->begin() + j);
                }
                else {
                    j++;
                }
            }
            for (int j=0; j<otherDenseJobsBlocks->size();) {
                if (jobInList(job, (*otherDenseJobsBlocks)[j]->first.get())) {
                    otherDenseJobsBlocks->erase(otherDenseJobsBlocks->begin() + j);
                }
                else {
                    j++;
                }
            }
            
            if (!bestBlock->first->size()) {
                bestDenseJobsBlocks->erase(bestDenseJobsBlocks->begin());
                break;
            }
        }
    }
    
    if (childJobsList.size() != _activeList.jobList()->size()) {
        vector<Job *> *bestJobsList = NULL;
        if (duration() < schedule->duration()) bestJobsList = &jobsList;
        else bestJobsList = &jobsList2;
        childJobsList.insert(childJobsList.end(), bestJobsList->begin(), bestJobsList->end());
    }
    
    ActiveList childActiveList(&childJobsList);
    return Schedule :: scheduleEarly(&childActiveList, _resources);
}

PSchedule Schedule :: earlySchedule() const
{
    vector<Job *> jobs = *_activeList.jobList();
    sort(jobs.begin(), jobs.end(), [this](Job *job1, Job *job2){return start(job1) < start(job2);});
    ActiveList activeList(&jobs);
    PSchedule schedule = Schedule :: scheduleEarly(&activeList, _resources);
    return schedule;
}

PSchedule Schedule :: lateSchedule() const
{
    vector<Job *> jobs = *_activeList.jobList();
    sort(jobs.begin(), jobs.end(), [this](Job *job1, Job *job2){return end(job1) < end(job2);});
    ActiveList activeList(&jobs);
    PSchedule schedule = Schedule :: scheduleLate(&activeList, _resources);
    return schedule;
}

shared_ptr<vector<PSchedule>> Schedule :: neighboringSchedules
(NeighbourhoodType neighbourhoodType,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
#warning Is it right? Are we needs to construct correct schedule first?
    PSchedule schedule = nullptr;
    switch (neighbourhoodType) {
        case NeighbourhoodTypeEarly: {
            if (_type == ScheduleTypeEarly) schedule = shared_from_this();
            else schedule = earlySchedule();
            break;
        }
        case NeighbourhoodTypeLate: {
            if (_type == ScheduleTypeLate) schedule = shared_from_this();
            else schedule = lateSchedule();
            break;
        }
    }
    
    shared_ptr<vector<PSchedule>> neighbors(new vector<PSchedule>(0));
    neighbors->reserve(schedule->activeList()->size());
    
    for (Job *job : *schedule->activeList()->jobList()) {
        PSchedule neighbor = nullptr;
        switch (schedule->type()) {
            case ScheduleTypeEarly: {
                neighbor = schedule->neighbourForEarlySchedule(job, functionForSelecting);
                break;
            }
            case ScheduleTypeLate: {
                neighbor = schedule->neighbourForLateSchedule(job, functionForSelecting);
                break;
            }
            default: {
                
                cout << "Error. Incorrect schedule type. See neighboringSchedules(neighbourhoodType, &functionForSelecting) function of Schedule"
                << endl;
                
                cout << "neighbourhoodType = " << neighbourhoodType << endl;
                
                cout << "this = " << *this << endl;
                cout << "this MATLAB string = " << this->stringMATLAB() << endl;
                
                cout << "schedule = " << *schedule << endl;
                cout << "schedule MATLAB string = " << schedule->stringMATLAB() << endl;
                
                abort();
                break;
            }
        }
        if (neighbor) neighbors->push_back(neighbor);
    }
    
    return neighbors;
}

PSchedule Schedule :: neighbourForEarlySchedule
(Job *job,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    if (_type != ScheduleTypeEarly) return PSchedule(nullptr);
    
    shared_ptr<vector<Job *>> block = blockOfJobs(job, false, true);
    if (!block) return PSchedule(nullptr);
    shared_ptr<vector<Job *>> net = outgoingNetwork(job);
    
    auto minIterator = _activeList.jobList()->end();
    auto maxIterator = _activeList.jobList()->begin();
    for (Job *j : *block) {
        auto iterator = find(_activeList.jobList()->begin(), _activeList.jobList()->end(), j);
        if (iterator < minIterator) minIterator = iterator;
        if (iterator > maxIterator) maxIterator = iterator;
    }
    for (Job *j : *net) {
        auto iterator = find(_activeList.jobList()->begin(), _activeList.jobList()->end(), j);
        if (iterator > maxIterator) maxIterator = iterator;
    }
    
    return Schedule :: schedulePartialyEarlyParallel(&_activeList,
                                                     _resources,
                                                     minIterator,
                                                     maxIterator,
                                                     &_starts,
                                                     functionForSelecting);
}

PSchedule Schedule :: neighbourForLateSchedule
(Job *job,
 function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    if (_type != ScheduleTypeLate) return PSchedule(nullptr);
    
    shared_ptr<vector<Job *>> block = blockOfJobs(job, true, false);
    if (!block) return PSchedule(nullptr);
    shared_ptr<vector<Job *>> net = incomingNetwork(job);
    
    auto minIterator = _activeList.jobList()->end();
    auto maxIterator = _activeList.jobList()->begin();
    for (Job *j : *block) {
        auto iterator = find(_activeList.jobList()->begin(), _activeList.jobList()->end(), j);
        if (iterator < minIterator) minIterator = iterator;
        if (iterator > maxIterator) maxIterator = iterator;
    }
    for (Job *j : *net) {
        auto iterator = find(_activeList.jobList()->begin(), _activeList.jobList()->end(), j);
        if (iterator < minIterator) minIterator = iterator;
    }
    
#warning Is it right interpretation of article?
    if (maxIterator + 1 == _activeList.jobList()->end()) {
        maxIterator--;
    }
    
    return Schedule :: schedulePartialyLateParallel(&_activeList,
                                                    _resources,
                                                    minIterator,
                                                    maxIterator,
                                                    &_starts,
                                                    functionForSelecting);
}

ScheduleValid Schedule :: validation()
{
    // Resource restriction
    
    map<Resource *, vector<int>> resourceRequired;
    for (auto &pJobStart : _starts) {
        for (auto &pResourceAmount : *pJobStart.first->resourceAmounts()) {
            if (resourceRequired.find(pResourceAmount.first) == resourceRequired.end()) {
                resourceRequired[pResourceAmount.first] = vector<int>(_activeList.duration(), 0);
            }
            for (int time=0; time<pJobStart.first->duration(); time++) {
                resourceRequired[pResourceAmount.first][pJobStart.second + time] +=
                pJobStart.first->resourceAmount(pResourceAmount.first);
            }
        }
    }
    
    for (auto &pResourceAmount : resourceRequired) {
        for (int time=0; time<pResourceAmount.second.size(); time++) {
            if (pResourceAmount.second[time] > pResourceAmount.first->amount()) {
                return ScheduleValidNoResourceRestriction;
            }
        }
    }
    
    // Presedence relation
    
    for (auto &pJobStart : _starts) {
        for (auto &successor : *pJobStart.first->successors()) {
            if (_starts[successor] < pJobStart.second + pJobStart.first->duration()) {
                return ScheduleValidNoPresedenceRelation;
            }
        }
    }
    
    return ScheduleValidOK;
}
