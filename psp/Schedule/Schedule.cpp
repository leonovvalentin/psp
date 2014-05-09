//
//  Schedule.cpp
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Schedule.h"
#include "TabuList.h"

#include "Random.h"

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
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
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
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
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
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
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
        
        PVectorJobs permissible = schedule->permissibleJobsByResources(&permissibleByPredecessor,
                                                                       time,
                                                                       true);
        
        while (permissible->size()) {
            
            PVectorJobs newActiveJobs = functionForSelecting(permissible.get(),
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

int Schedule :: start(string jobName) const
{
    for (auto iterator=_starts.begin(); iterator!=_starts.end(); iterator++) {
        if ((*iterator->first->name()) == jobName) return iterator->second;
    }
    return INT_MAX;
}

int Schedule :: end(Job *job) const
{
    auto iterator = _starts.find(job);
    if (iterator != _starts.end()) return iterator->second + job->duration();
    return INT_MAX;
}

int Schedule :: end(string jobName) const
{
    for (auto iterator=_starts.begin(); iterator!=_starts.end(); iterator++) {
        if ((*iterator->first->name()) == jobName) {
            return iterator->second + iterator->first->duration();
        }
    }
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

PSchedule Schedule :: localSearchKS(ParamsKS params)
{
    function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [params](PARAMETERS_OF_SELECTING_FUNCTION) -> PVectorJobs {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, params.probabilityKP);
    };
    
    TabuList tabuList(params.tabuListSize);
    
    int stepsNoChange = 0; // Number of steps without changing neighbourhood
    NeighbourhoodType currentNeighbourhoodType = NeighbourhoodTypeEarly;
    
    PSchedule schedule = shared_from_this(); // Start from current schedule
    PSchedule record = schedule;
    tabuList.add(schedule->sumOfStarts());
    
    for (int iteration = 0; iteration < params.maxIterationNumber; iteration++) {
        
#ifdef LOG_TO_CONSOL_SCHEDULE_H
        LOG("iterations: "
            << (float)iteration/params.maxIterationNumber * 100 << "%"
            << " record = " << record->duration());
#endif
        
        // intensification
        if (params.numberOfReturnsToRecord != 0 &&
            (iteration % (params.maxIterationNumber / params.numberOfReturnsToRecord) == 0)) {
            schedule = record;
        }
        
        // currentNeighbourhoodType
        
        if (stepsNoChange >= params.changingInterval) {
            switch (currentNeighbourhoodType) {
                case NeighbourhoodTypeEarly: {
                    currentNeighbourhoodType = NeighbourhoodTypeLate;
                    break;
                }
                case NeighbourhoodTypeLate: {
                    currentNeighbourhoodType = NeighbourhoodTypeEarly;
                    break;
                }
            }
            stepsNoChange = 0;
        }
        
        // allNeighbours
        
        shared_ptr<vector<PSchedule>> allNeighbours =
        schedule->neighboringSchedules(currentNeighbourhoodType, functionForSelecting);
        
        // neighboursWithoutTabu
        
        auto neighboursWithoutTabu = shared_ptr<vector<PSchedule>>(new vector<PSchedule>(0));
        neighboursWithoutTabu->reserve(allNeighbours->size());
        
        while (neighboursWithoutTabu->size() == 0 && tabuList.size() != 0) {
            for (auto &neighbour : *allNeighbours) {
                if (!tabuList.containTabu(neighbour->sumOfStarts())) {
                    neighboursWithoutTabu->push_back(neighbour);
                }
            }
            if (neighboursWithoutTabu->size() == 0) {
#warning Set number of tabu for removing as parameter of function?
                tabuList.removeOlderTabu(1);
            }
        }
        
        if (neighboursWithoutTabu->size() == 0 && tabuList.size() == 0) {
            neighboursWithoutTabu->insert(neighboursWithoutTabu->begin(),
                                          allNeighbours->begin(),
                                          allNeighbours->end());
        }
        
        // neighbours
        
        auto neighbours =
        shared_ptr<vector<PSchedule>>(new vector<PSchedule>(0));
        neighbours->reserve(neighboursWithoutTabu->size());
        
        for (auto &neighbour : *neighboursWithoutTabu) {
            if (Random :: randomFloatFrom0To1() < params.probabilitySN) {
                neighbours->push_back(neighbour);
            }
        }
        
        if (neighbours->size() == 0) {
            long randIndex = Random :: randomLong(0, neighboursWithoutTabu->size() - 1);
            neighbours->push_back((*neighboursWithoutTabu)[randIndex]);
        }
        
        // minNeighbour
        
        PSchedule minNeighbour = nullptr;
        int minNeighbourDuration = INT_MAX;
        
        for (auto &neighbour : *neighbours) {
            int neighbourDuration = neighbour->duration();
            if (neighbourDuration < minNeighbourDuration) {
                minNeighbour = neighbour;
                minNeighbourDuration = neighbourDuration;
            }
        }
        
        // Update record, schedule, tabuList
        
#warning (<=) or (<) ? May be move it to parametes of method?
        if (minNeighbourDuration <= record->duration()) record = minNeighbour;
        schedule = minNeighbour;
        tabuList.add(minNeighbour->sumOfStarts());
        
        // Update stepsNoChange
        
        stepsNoChange++;
    }
    
    return record;
}

PSchedule Schedule :: pingPongSchedule() const
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

PSchedule Schedule :: crossViaPreviewAllBlocks(PSchedule schedule, float permissibleResourceRemains)
{
    PSchedule earlySchedule =
    (this->type() == ScheduleTypeEarly) ? shared_from_this() : this->earlySchedule();
    PSchedule earlySchedule2 =
    (schedule->type() == ScheduleTypeEarly) ? schedule : schedule->earlySchedule();
    
    auto denseJobsBlocks = earlySchedule->denseJobsBlocks(permissibleResourceRemains);
    auto denseJobsBlocks2 = earlySchedule2->denseJobsBlocks(permissibleResourceRemains);
    
    auto jobsList = *earlySchedule->activeList()->jobList();
    auto jobsList2 = *earlySchedule2->activeList()->jobList();
    vector<Job *> childJobsList(0);
    
    while (denseJobsBlocks->size() != 0 || denseJobsBlocks2->size() != 0) {
        
        shared_ptr<pair<PVectorJobs, float>> bestBlock = nullptr;
        vector<Job *> *bestJobsList = NULL;
        
        shared_ptr<vector<shared_ptr<pair<PVectorJobs, float>>>>
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
    
    if (childJobsList.size() != earlySchedule->activeList()->size()) {
        vector<Job *> *bestJobsList = NULL;
        if (earlySchedule->duration() < earlySchedule2->duration()) bestJobsList = &jobsList;
        else bestJobsList = &jobsList2;
        childJobsList.insert(childJobsList.end(), bestJobsList->begin(), bestJobsList->end());
    }
    
    ActiveList childActiveList(&childJobsList);
    return Schedule :: scheduleEarly(&childActiveList, _resources);
}

PSchedule Schedule :: crossViaSelectOneBlock(PSchedule schedule, ParamsCross paramsCross)
{
    PSchedule earlySchedule =
    (this->type() == ScheduleTypeEarly) ? shared_from_this() : this->earlySchedule();
    PSchedule earlySchedule2 =
    (schedule->type() == ScheduleTypeEarly) ? schedule : schedule->earlySchedule();
    
    auto denseJobsBlocks = earlySchedule->denseJobsBlocks(paramsCross.permissibleResourceRemains);
    auto denseJobsBlocks2 = earlySchedule2->denseJobsBlocks(paramsCross.permissibleResourceRemains);
    
    auto jobsList = *earlySchedule->activeList()->jobList();
    auto jobsList2 = *earlySchedule2->activeList()->jobList();
    
    shared_ptr<pair<PVectorJobs, float>> bestBlock = nullptr;
    vector<Job *> *otherJobsList = NULL;
    PSchedule otherSchedule = nullptr;
    
    for (auto &block : *denseJobsBlocks) {
        if (!bestBlock || block->second < bestBlock->second) {
            bestBlock = block;
            otherJobsList = &jobsList2;
            otherSchedule = earlySchedule2;
        }
    }
    for (auto &block : *denseJobsBlocks2) {
        if (!bestBlock || block->second < bestBlock->second) {
            bestBlock = block;
            otherJobsList = &jobsList;
            otherSchedule = earlySchedule;
        }
    }
    
    vector<Job *> childJobsList(*otherJobsList);
    ActiveList childActiveList(&childJobsList);
    
    auto minIterator = childActiveList.jobList()->end();
    auto maxIterator = childActiveList.jobList()->begin();
    for (Job *job : *bestBlock->first) {
        auto iterator = find(childActiveList.jobList()->begin(),
                             childActiveList.jobList()->end(),
                             job);
        if (iterator < minIterator) minIterator = iterator;
        if (iterator > maxIterator) maxIterator = iterator;
    }
    if (paramsCross.withNet) {
        for (Job *job : *bestBlock->first) {
            PVectorJobs net = nullptr;
            if (paramsCross.isEarlyComposite) net = otherSchedule->outgoingNetwork(job);
            else net = otherSchedule->incomingNetwork(job);
            for (Job *j : *net) {
                auto iterator = find(childActiveList.jobList()->begin(),
                                     childActiveList.jobList()->end(),
                                     j);
                if (paramsCross.isEarlyComposite && iterator > maxIterator) {
                    maxIterator = iterator;
                }
                else if ((!paramsCross.isEarlyComposite) && iterator < minIterator) {
                    minIterator = iterator;
                }
            }
        }
    }
    
    function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [paramsCross](PARAMETERS_OF_SELECTING_FUNCTION) -> PVectorJobs {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, paramsCross.probabilityKP);
    };
    
    if (paramsCross.isEarlyComposite) {
        return Schedule :: schedulePartialyEarlyParallel(&childActiveList,
                                                         _resources,
                                                         minIterator,
                                                         maxIterator,
                                                         &otherSchedule->_starts,
                                                         functionForSelecting);
    }
    else {
        return Schedule :: schedulePartialyLateParallel(&childActiveList,
                                                        _resources,
                                                        minIterator,
                                                        maxIterator,
                                                        &otherSchedule->_starts,
                                                        functionForSelecting);
    }
}

PSchedule Schedule :: earlySchedule() const
{
    PActiveList earlyActiveList = this->earlyActiveList();
    PSchedule schedule = Schedule :: scheduleEarly(earlyActiveList.get(), _resources);
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
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
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

PActiveList Schedule :: earlyActiveList() const
{
    vector<Job *> jobs = *_activeList.jobList();
    auto starts = _starts;
    sort(jobs.begin(), jobs.end(), [&starts](Job *job1, Job *job2){
        return starts.find(job1)->second < starts.find(job2)->second;
    });
    PActiveList earlyActiveList(new ActiveList(&jobs));
    return earlyActiveList;
}

PSchedule Schedule :: neighbourForEarlySchedule
(Job *job,
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    if (_type != ScheduleTypeEarly) return PSchedule(nullptr);
    
    PVectorJobs block = blockOfJobs(job, false, true);
    if (!block) return PSchedule(nullptr);
    PVectorJobs net = outgoingNetwork(job);
    
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
 function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting)
{
    if (_type != ScheduleTypeLate) return PSchedule(nullptr);
    
    PVectorJobs block = blockOfJobs(job, true, false);
    if (!block) return PSchedule(nullptr);
    PVectorJobs net = incomingNetwork(job);
    
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

bool Schedule :: isEqualByHamming(PSchedule schedule, int hammingDispersion)
{
    return hammingDistance(schedule, hammingDispersion) == 0;
}

int Schedule :: hammingDistance(PSchedule schedule, int hammingDispersion)
{
    return earlyActiveList()->hammingDistance(schedule->earlyActiveList().get(), hammingDispersion);
}
