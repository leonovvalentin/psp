//
//  Problem.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//


#include "ActiveList.h"
#include "Problem.h"
#include "TabuList.h"
#include "Random.h"

#include <fstream>
#include <sstream>



#pragma mark - helper methods

void Problem ::  logRecordToFile(PSchedule record) const
{
#ifdef LOG_TO_FILE_EACH_NEW_RECORD_GA_GA2014
    time_t result = time(nullptr);
    stringstream ss;
    ss << asctime(localtime(&result))
    << "Problem: " << _name << endl << "duration: " << record->duration() << endl << *record;
    LOGF(ss.str());
#endif
}

#pragma mark - init

Problem :: Problem(string *name, string *path)
{
    _name = *name;
    _criticalPathDuration = 0;
    
    ifstream data(*path);
    
    if (data.is_open()) {
        int jobsCount, resourcesCount; data >> jobsCount >> resourcesCount;
        
        _resources = vector<Resource *>(0);
        for (int i=0; i<resourcesCount; i++) {
            string name = to_string(i);
            int amount; data >> amount;
            _resources.push_back(new Resource(&name, amount));
        }
        
        _jobs = vector<Job *>(0);
        map<Job *, vector<int>> successorPositions;
        for (int i=0; i<jobsCount; i++) {
            string name = to_string(i);
            int duration; data >> duration;
            
            map<Resource *, int> resourceAmounts;
            for (int j=0; j<resourcesCount; j++) data >> resourceAmounts[_resources[j]];
            
            Job *job = new Job(&name, duration, &resourceAmounts);
            _jobs.push_back(job);
            
            int successorsCount; data >> successorsCount;
            successorPositions[job] = vector<int>(successorsCount);
            for (int j=0; j<successorsCount; j++) {
                int position; data >> position; position--;
                successorPositions[job][j] = position;
            }
        }
        
        for (auto &job : _jobs) {
            for (int successorPosition : successorPositions[job]) {
                Job *jobSuccessor = _jobs[successorPosition];
                job->addSuccessor(jobSuccessor);
                jobSuccessor->addPredecessor(job);
            }
        }
        
        data.close();
    }
}

Problem :: ~Problem()
{
    for (auto &job : _jobs) delete job;
    for (auto &resource : _resources) delete resource;
}

#pragma mark - out

ostream & operator<<(ostream &os, const Problem &problem)
{
    os << "Problem with name: " << problem._name << " data in .RCP format:" << endl;
    os << problem.stringInRCPFormat();
    return os;
}

string Problem :: stringInRCPFormat() const
{
    stringstream ss;
    
    ss << _jobs.size() << " " << _resources.size() << endl;
    for (auto &resource : _resources) ss << resource->amount() << " ";
    for (auto &job : _jobs) {
        ss << endl << job->duration();
        for (auto &p : *job->resourceAmounts()) ss << " " << p.second;
        ss << " " << job->successors()->size();
        for (auto &successor : *job->successors()) ss << " " << *successor->name();
    }
    
    return ss.str();
}

string Problem :: stringRelationshipJobsInGVFormat() const
{
    stringstream ss;
    
    ss << "digraph G" << endl << "{";
    for (auto &job : _jobs) {
        for (auto &successor : *job->successors()) {
            ss << endl << "\t" << *job->name() << " -> " << *successor->name() << ";";
        }
    }
    ss << endl << "}" << endl;
    
    return ss.str();
}

#pragma mark - getters

const string * Problem :: name() const
{
    return &_name;
}

const int Problem :: criticalPathDuration()
{
    if (_criticalPathDuration == 0) {
        ActiveList activeList(_jobs[0], _jobs.size());
        _criticalPathDuration =
        (Schedule :: scheduleEarlyWithoutResources(&activeList))->duration();
    }
    return _criticalPathDuration;
}

#pragma mark - functionality

PSchedule Problem :: scheduleEarlyWithRandom(int times) const
{
    PSchedule record;
    for (int i=0; i<times; i++) {
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleEarly(&activeList, &_resources);
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    return record;
}

PSchedule Problem :: scheduleLateWithRandom(int times) const
{
    PSchedule record;
    for (int i=0; i<times; i++) {
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleLate(&activeList, &_resources);
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    return record;
}

PSchedule Problem :: scheduleEarlyParallelSimple(int times) const
{
    function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [](PARAMETERS_OF_SELECTING_FUNCTION) -> PVectorJobs {
        return selectJobFirstInActiveList(jobs, schedule, time, timeForStart);
    };
    
    PSchedule record;
    for (int i=0; i<times; i++) {
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleEarlyParallel(&activeList,
                                                               &_resources,
                                                               functionForSelecting);
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    
    return record;
}

PSchedule Problem :: scheduleEarlyParallelKP(int times, float probability) const
{
    function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [probability](PARAMETERS_OF_SELECTING_FUNCTION) -> PVectorJobs {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, probability);
    };
    
    PSchedule record;
    for (int i=0; i<times; i++) {
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleEarlyParallel(&activeList,
                                                               &_resources,
                                                               functionForSelecting);
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    
    return record;
}

PSchedule Problem :: schedulePingPong(int times,
                                      float probability,
                                      int *numberOfGeneratedSchedules) const
{
    function<PVectorJobs(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [probability](PARAMETERS_OF_SELECTING_FUNCTION) -> PVectorJobs {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, probability);
    };
    
    PSchedule record;
    for (int i=0; i<times; i++) {
#warning activeList must be created as in the article (see Кочетов, Столяр, ЗПЧР, с.39)
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleEarlyParallel(&activeList,
                                                               &_resources,
                                                               functionForSelecting);
        (*numberOfGeneratedSchedules)++;
        schedule = schedule->pingPongSchedule(numberOfGeneratedSchedules);
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    
    return record;
}

PSchedule Problem :: scheduleKS(ParamsKS params) const
{
    int numberOfGeneratedSchedules = 0;
    PSchedule schedule = schedulePingPong(1, params.probabilityKP, &numberOfGeneratedSchedules); // Initial schedule
    return schedule->localSearchKS(params, &numberOfGeneratedSchedules);
}

PSchedule Problem :: scheduleGA(ParamsGA params) const
{
    PSchedule record = nullptr;
    int numberOfGeneratedSchedules = 0;
    
    // initial population
    
    vector<PSchedule> population(0);
    population.reserve(params.populationSize + params.numberOfChildrenInNextGeneration);
    for (int i=0; i<params.populationSize; i++) {
        
#ifdef LOG_TO_CONSOL_PROBLEM_H
        LOG("initial population: "
            << (float)i/params.populationSize * 100 << "%"
            << " record = " << (record ? record->duration() : INT_MAX));
#endif
        
        PSchedule schedule = schedulePingPong(params.timesPingPong,
                                              params.probabilityKP,
                                              &numberOfGeneratedSchedules);
        population.push_back(schedule);
        if (!record || (schedule->duration() < record->duration())) {
            record = schedule;
            logRecordToFile(record);
        }
    }
    
    // generations
    
    while ((params.maxGeneratedSchedules != maxGeneratedSchedulesInfinite) ?
           (numberOfGeneratedSchedules < params.maxGeneratedSchedules) :
           true) {
        
#ifdef LOG_TO_CONSOL_PROBLEM_H
        if (params.maxGeneratedSchedules != maxGeneratedSchedulesInfinite) {
            LOG("generated schedules: "
                << (float)numberOfGeneratedSchedules/params.maxGeneratedSchedules * 100 << "%"
                << " record = " << record->duration());
        }
        else {
            LOG("generated schedules: "
                << numberOfGeneratedSchedules << " record = " << record->duration());
        }
#endif
        
        // select parents as subset of population
        sort(population.begin(), population.end(), [](PSchedule a, PSchedule b) {
            return a->duration() < b->duration();
        });
        vector<PSchedule> parents(0); parents.reserve(params.maxParents);
        for (int i=0; i<population.size() && parents.size()<params.maxParents; i++) {
            if (Random :: randomFloatFrom0To1() < params.probabilityParentSelection) {
                parents.push_back(population[i]);
            }
        }
        for (int i=0; parents.size()<params.maxParents; i++) {
            auto schedule = population[i];
            if (find(begin(parents), end(parents), schedule) == end(parents)) {
                parents.push_back(schedule);
            }
        }
        
        // create children
        vector<PSchedule> children(0); children.reserve(params.maxChildren);
        while (children.size() < params.maxChildren) {
            
            // select 2 parents for crossing
            auto parent1 = parents[Random :: randomLong(0, parents.size() - 1)];
            auto parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            while (parent2 == parent1) {
                parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            }
            
            // crossing
            auto child = parent1->crossViaPreviewAllBlocks(parent2,
                                                           params.permissibleResourceRemains);
            numberOfGeneratedSchedules++;
            auto mutatedChild =
            child->swapAndMoveMutation(params.swapAndMovePermissibleTimes,
                                       params.swapAndMovePermissibleTimes);
            numberOfGeneratedSchedules++;
            mutatedChild = mutatedChild->pingPongSchedule(&numberOfGeneratedSchedules);
            if (child->duration() < record->duration()) {
                if (mutatedChild->duration() < child->duration()) child = mutatedChild;
                record = child;
                logRecordToFile(record);
            }
            else {
                child = mutatedChild;
                if (child->duration() < record->duration()) {
                    record = child;
                    logRecordToFile(record);
                }
            }
            
            children.push_back(child);
        }
        
        // next population
        sort(begin(children), end(children), [](PSchedule a, PSchedule b){
            return a->duration() < b->duration();
        });
        population.insert(population.begin(),
                          children.begin(),
                          children.begin() + params.numberOfChildrenInNextGeneration);
        population.erase(population.begin() + params.populationSize, population.end());
    }
    
    return record;
}

PSchedule Problem :: scheduleGA2014(ParamsGA paramsGA,
                                    ParamsKS paramsKS,
                                    ParamsCross paramsCross,
                                    int permissibleNoChangeRecord,
                                    int numberOfSubstitutions,
                                    int numberOfLocalSearchKS) const
{
    PSchedule record = nullptr;
    int numberOfGeneratedSchedules = 0;
    
    // initial population
    
    vector<PSchedule> population(0);
    population.reserve(paramsGA.populationSize + paramsGA.numberOfChildrenInNextGeneration);
    for (int i=0; i<paramsGA.populationSize; i++) {
        
#ifdef LOG_TO_CONSOL_PROBLEM_H
        LOG("initial population: "
            << (float)i/paramsGA.populationSize * 100 << "%"
            << " record = " << (record ? record->duration() : INT_MAX));
#endif
        
        PSchedule schedule = schedulePingPong(paramsGA.timesPingPong,
                                              paramsGA.probabilityKP,
                                              &numberOfGeneratedSchedules);
        population.push_back(schedule);
        if (!record || (schedule->duration() < record->duration())) {
            record = schedule;
            logRecordToFile(record);
        }
    }
    
    // generations
    
    int prevRecordDuration = record->duration();
    int noChangeRecord = 0;
    while ((paramsGA.maxGeneratedSchedules != maxGeneratedSchedulesInfinite) ?
           (numberOfGeneratedSchedules < paramsGA.maxGeneratedSchedules) :
           true) {
        
#ifdef LOG_TO_CONSOL_PROBLEM_H
        if (paramsGA.maxGeneratedSchedules != maxGeneratedSchedulesInfinite) {
            LOG("generated schedules: "
                << (float)numberOfGeneratedSchedules/paramsGA.maxGeneratedSchedules * 100 << "%"
                << " record = " << record->duration());
        }
        else {
            LOG("generated schedules: "
                << numberOfGeneratedSchedules << " record = " << record->duration());
        }
#endif
        
        // select parents as subset of population
        sort(population.begin(), population.end(), [](PSchedule a, PSchedule b) {
            return a->duration() < b->duration();
        });
        vector<PSchedule> parents(0); parents.reserve(paramsGA.maxParents);
        for (int i=0; i<population.size() && parents.size()<paramsGA.maxParents; i++) {
            if (Random :: randomFloatFrom0To1() < paramsGA.probabilityParentSelection) {
                parents.push_back(population[i]);
            }
        }
        for (int i=0; parents.size()<paramsGA.maxParents; i++) {
            auto schedule = population[i];
            if (find(begin(parents), end(parents), schedule) == end(parents)) {
                parents.push_back(schedule);
            }
        }
        
        // create children
        vector<PSchedule> children(0); children.reserve(paramsGA.maxChildren);
        while (children.size() < paramsGA.maxChildren) {
            
            // select 2 parents for crossing
            auto parent1 = parents[Random :: randomLong(0, parents.size() - 1)];
            auto parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            while (parent2 == parent1) {
                parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            }
            
            // crossing
            PSchedule child = nullptr;
            if (Random :: randomFloatFrom0To1() < 0.5) {
                child = parent1->crossViaPreviewAllBlocks(parent2,
                                                          paramsCross.permissibleResourceRemains);
            }
            else {
                child = parent1->crossViaSelectOneBlock(parent2, paramsCross);
            }
            numberOfGeneratedSchedules++;
            auto mutatedChild =
            child->swapAndMoveMutation(paramsGA.swapAndMovePermissibleTimes,
                                       paramsGA.swapAndMovePermissibleTimes);
            numberOfGeneratedSchedules++;
            mutatedChild = mutatedChild->pingPongSchedule(&numberOfGeneratedSchedules);
            if (child->duration() < record->duration()) {
                if (mutatedChild->duration() < child->duration()) child = mutatedChild;
                record = child;
                logRecordToFile(record);
            }
            else {
                child = mutatedChild;
                if (child->duration() < record->duration()) {
                    record = child;
                    logRecordToFile(record);
                }
            }
            
            children.push_back(child);
        }
        
        // next population
        sort(begin(children), end(children), [](PSchedule a, PSchedule b){
            return a->duration() < b->duration();
        });
        population.insert(population.begin(),
                          children.begin(),
                          children.begin() + paramsGA.numberOfChildrenInNextGeneration);
        population.erase(population.begin() + paramsGA.populationSize, population.end());
        
        // replace copies // appearance of copies after this cycle is unlikely, but not impossible
        for (int i = 0; i < population.size(); i++) {
            auto s1 = population[i];
            for (int j = i + 1; j < population.size(); j++) {
                auto s2 = population[j];
                if (s2->isEqualToSchedule(s1)) {
                    population[j] = schedulePingPong(paramsGA.timesPingPong,
                                                     paramsGA.probabilityKP,
                                                     &numberOfGeneratedSchedules);
                }
            }
        }
        
        // population thinning
        if (prevRecordDuration == record->duration()) {
            noChangeRecord++;
        }
        else {
            prevRecordDuration = record->duration();
            noChangeRecord = 0;
        }
        if (noChangeRecord > permissibleNoChangeRecord) {
            for (int i=0; i<numberOfSubstitutions; i++) {
                auto it = population.begin() + (Random :: randomLong(0, population.size()-1));
                population.erase(it);
            }
            for (int i=0; i<numberOfSubstitutions; i++) {
                PSchedule schedule = schedulePingPong(paramsGA.timesPingPong,
                                                      paramsGA.probabilityKP,
                                                      &numberOfGeneratedSchedules);
                population.push_back(schedule);
                if (schedule->duration() < record->duration()) {
                    record = schedule;
                    logRecordToFile(record);
                    prevRecordDuration = record->duration();
                }
            }
            for (int i=0; i<numberOfLocalSearchKS; i++) {
                auto schedule = population[Random :: randomLong(0, population.size()-1)];
                schedule = schedule->localSearchKS(paramsKS, &numberOfGeneratedSchedules);
                if (schedule->duration() < record->duration()) {
                    record = schedule;
                    logRecordToFile(record);
                    prevRecordDuration = record->duration();
                }
                if ((paramsGA.maxGeneratedSchedules != maxGeneratedSchedulesInfinite) &&
                    (numberOfGeneratedSchedules >= paramsGA.maxGeneratedSchedules)) {
                    return record;
                }
            }
            noChangeRecord = 0;
        }
    }
    
    return record;
}
