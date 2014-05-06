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
    function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [](PARAMETERS_OF_SELECTING_FUNCTION) -> JOBS_VECTOR_PTR {
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
    function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [probability](PARAMETERS_OF_SELECTING_FUNCTION) -> JOBS_VECTOR_PTR {
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

PSchedule Problem :: schedulePingPong(int times, float probability) const
{
    function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [probability](PARAMETERS_OF_SELECTING_FUNCTION) -> JOBS_VECTOR_PTR {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, probability);
    };
    
    PSchedule record;
    for (int i=0; i<times; i++) {
#warning activeList must be created as in the article (see Кочетов, Столяр, ЗПЧР, с.39)
        ActiveList activeList(_jobs[0], _jobs.size());
        PSchedule schedule = Schedule :: scheduleEarlyParallel(&activeList,
                                                               &_resources,
                                                               functionForSelecting);
        bool stop = false;
        while (!stop) {
            PSchedule scheduleLate = schedule->lateSchedule();
            PSchedule scheduleEarly = scheduleLate->earlySchedule();
            if (scheduleEarly->duration() < schedule->duration()) schedule = scheduleEarly;
            else stop = true;
        }
        if (!record.get() || record->duration() > schedule->duration()) record = schedule;
    }
    
    return record;
}

PSchedule Problem :: scheduleKochetovStolyar2003(ParamsKochetovStolyar2003 params) const
{
    PSchedule schedule = schedulePingPong(1, params.probabilityKP); // Initial schedule
    return schedule->localSearchKochetovStolyar2003(params.probabilityKP,
                                                    params.probabilitySN,
                                                    params.tabuListSize,
                                                    params.changingInterval,
                                                    params.maxIterationNumber);
}

PSchedule Problem :: scheduleMyGA(ParamsMyGA params) const
{
    PSchedule record = nullptr;
    
    // initial population
    
    vector<PSchedule> population(0);
    population.reserve(params.populationSize + params.numberOfChildrenInNextGeneration);
    for (int i=0; i<params.populationSize; i++) {
        
        LOG("initial population: "
            << (float)i/params.populationSize * 100 << "%"
            << " record = " << (record ? record->duration() : INT_MAX));
        
        PSchedule schedule = schedulePingPong(params.timesPingPongInitialPopulation,
                                              params.probabilityKP);
        population.push_back(schedule);
        if (!record || (schedule->duration() < record->duration())) record = schedule;
    }
    
    // generations
    
    int numberOfGeneratedSchedules = 0;
    while (numberOfGeneratedSchedules < params.maxGeneratedSchedules) {
        
        LOG("generated schedules: "
            << (float)numberOfGeneratedSchedules/params.maxGeneratedSchedules * 100 << "%"
            << " record = " << record->duration());
        
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
            auto child = parent1->cross(parent2, params.permissibleResourceRemains);
            auto mutatedChild =
            child->swapAndMoveMutation(params.swapAndMovePermissibleTimes,
                                       params.swapAndMovePermissibleTimes)->pingPong();
            if (child->duration() < record->duration()) {
                if (mutatedChild->duration() < child->duration()) child = mutatedChild;
                record = child;
            }
            else {
                child = mutatedChild;
            }
            
            children.push_back(child);
            numberOfGeneratedSchedules++;
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

//PSchedule Problem :: scheduleMyGA2014(ParamsMyGA paramsGA,
//                                      ParamsKochetovStolyar2003 paramsKS2003,
//                                      int hammingDispersion) const
//{
//    #warning add to params of method
//    float probability = 0.3;
//    int maxAttemptsNumber = paramsGA.populationSize;
//    int tabuListSize = 3;
//    
//    auto isScheduleCanBeAddedToThePopulation =
//    [hammingDispersion, probability](PSchedule schedule, vector<PSchedule> *schedules) -> bool {
//        for (PSchedule sch : *schedules) {
//            if (schedule->isEqualByHamming(sch, hammingDispersion)) {
//                if (Random :: randomFloatFrom0To1() < probability) return true;
//                else return false;
//            }
//        }
//        return true;
//    };
//    
//    PSchedule record = nullptr;
//    TabuList tabuList(tabuListSize);
//    
//    // initial population
//    
//    vector<PSchedule> population(0);
//    population.reserve(paramsGA.populationSize + paramsGA.numberOfChildrenInNextGeneration);
//    for (int i=0; i<paramsGA.populationSize; i++) {
//        
//        LOG("initial population: "
//            << (float)i/paramsGA.populationSize * 100 << "%"
//            << " record = " << (record ? record->duration() : INT_MAX));
//        
//        PSchedule schedule = schedulePingPong(paramsGA.timesPingPongInitialPopulation,
//                                              paramsGA.probabilityKP);
//        
//        if (!record || (schedule->duration() < record->duration())) {
//            population.push_back(schedule);
//            tabuList.add(schedule->sumOfStarts());
//            record = schedule;
//        }
//        else if (isScheduleCanBeAddedToThePopulation(schedule, &population)) {
//            if (!tabuList.containTabu(schedule->sumOfStarts())) {
//                population.push_back(schedule);
//                tabuList.add(schedule->sumOfStarts());
//            }
//        }
//    }
//    
//    // generations
//    
//    int numberOfGeneratedSchedules = 0;
//    while (numberOfGeneratedSchedules < paramsGA.maxGeneratedSchedules) {
//        
//        LOG("generated schedules: "
//            << (float)numberOfGeneratedSchedules/paramsGA.maxGeneratedSchedules * 100 << "%"
//            << " record = " << record->duration());
//        
//        // select parents as subset of population
//        sort(population.begin(), population.end(), [](PSchedule a, PSchedule b) {
//            return a->duration() < b->duration();
//        });
//        vector<PSchedule> parents(0); parents.reserve(paramsGA.maxParents);
//        for (int i=0; i<population.size() && parents.size()<paramsGA.maxParents; i++) {
//            if (Random :: randomFloatFrom0To1() < paramsGA.probabilityParentSelection) {
//                parents.push_back(population[i]);
//            }
//        }
//        for (int i=0; parents.size()<paramsGA.maxParents; i++) {
//            auto schedule = population[i];
//            if (find(begin(parents), end(parents), schedule) == end(parents)) {
//                parents.push_back(schedule);
//            }
//        }
//        
//        // create children
//        vector<PSchedule> children(0); children.reserve(paramsGA.maxChildren);
//        while (children.size() < paramsGA.maxChildren) {
//            
//            // select 2 parents for crossing
//            auto parent1 = parents[Random :: randomLong(0, parents.size() - 1)];
//            auto parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
//            int attemptNumber = 0;
//            while (attemptNumber < maxAttemptsNumber
//                   && parent2 == parent1
//                   && !parent2->isEqualByHamming(parent1, hammingDispersion)) {
//                parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
//                attemptNumber++;
//            }
//            if (attemptNumber == maxAttemptsNumber) {
//                while (parent2 == parent1
//                       && !parent2->isEqualByHamming(parent1, hammingDispersion)) {
//                    parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
//                }
//            }
//            
//            // crossing
//            auto child = parent1->cross(parent2, paramsGA.permissibleResourceRemains);
//            auto mutatedChild =
//            child->swapAndMoveMutation(paramsGA.swapAndMovePermissibleTimes,
//                                       paramsGA.swapAndMovePermissibleTimes)->pingPong();
//            if (child->duration() < record->duration()) {
//                if (mutatedChild->duration() < child->duration()) child = mutatedChild;
//                record = child;
//            }
//            else {
//                child = mutatedChild;
//            }
//            
//            if (child == record) {
//                children.push_back(child);
//                tabuList.add(child->sumOfStarts());
//            }
//            else if (isScheduleCanBeAddedToThePopulation(child, &population)) {
//                if (!tabuList.containTabu(child->sumOfStarts())) {
//                    children.push_back(child);
//                    tabuList.add(child->sumOfStarts());
//                }
//            }
//            numberOfGeneratedSchedules++;
//        }
//        
//        // next population
//        sort(begin(children), end(children), [](PSchedule a, PSchedule b){
//            return a->duration() < b->duration();
//        });
//        population.insert(population.begin(),
//                          children.begin(),
//                          children.begin() + paramsGA.numberOfChildrenInNextGeneration);
//        population.erase(population.begin() + paramsGA.populationSize, population.end());
//    }
//    
//    return record;
//}

PSchedule Problem :: scheduleMyGA2014(ParamsMyGA paramsGA,
                                      ParamsKochetovStolyar2003 paramsKS2003,
                                      int hammingDispersion) const
{
#warning add to parameters of method
    float probabilityEarly = 0.5;
    
    vector<PSchedule> populationEarly(0);
    populationEarly.reserve(paramsGA.populationSize + paramsGA.numberOfChildrenInNextGeneration);
    vector<PSchedule> populationLate(0);
    populationLate.reserve(paramsGA.populationSize + paramsGA.numberOfChildrenInNextGeneration);
    
    auto createEarlyOrLate =
    [probabilityEarly](PSchedule schedule) -> PSchedule {
        if (Random :: randomFloatFrom0To1() < probabilityEarly) return schedule->earlySchedule();
        else return schedule->lateSchedule();
    };
    
    auto addToPopulationBegin =
    [&populationEarly, &populationLate, probabilityEarly](PSchedule schedule) -> void {
        if (schedule->type() == ScheduleTypeEarly) {
            populationEarly.insert(populationEarly.begin(), schedule);
        }
        else if (schedule->type() == ScheduleTypeLate) {
            populationLate.insert(populationLate.begin(), schedule);
        }
        else {
            if (Random :: randomFloatFrom0To1() < probabilityEarly) {
                populationEarly.insert(populationEarly.begin(), schedule->earlySchedule());
            }
            else {
                populationLate.insert(populationLate.begin(), schedule->lateSchedule());
            }
        }
    };
    
    auto selectParentsAsSubsetOfPopulation =
    [paramsGA](vector<PSchedule> *population) -> vector<PSchedule> {
        
        sort(population->begin(), population->end(), [](PSchedule a, PSchedule b) {
            return a->duration() < b->duration();
        });
        
        vector<PSchedule> parents(0); parents.reserve(paramsGA.maxParents / 2.0f);
        for (int i=0; i<population->size() && parents.size()<paramsGA.maxParents; i++) {
            if (Random :: randomFloatFrom0To1() < paramsGA.probabilityParentSelection) {
                parents.push_back((*population)[i]);
            }
        }
        for (int i=0; parents.size()<paramsGA.maxParents; i++) {
            auto schedule = (*population)[i];
            if (find(begin(parents), end(parents), schedule) == end(parents)) {
                parents.push_back(schedule);
            }
        }
        
        return parents;
    };
    
    PSchedule record = nullptr;
    
    // initial population
    
    for (int i=0; i<paramsGA.populationSize; i++) {
        
//        LOG("initial population: "
//            << (float)i/paramsGA.populationSize * 100 << "%"
//            << " record = " << (record ? record->duration() : INT_MAX));
        
        PSchedule scheduleEarly = schedulePingPong(paramsGA.timesPingPongInitialPopulation,
                                                   paramsGA.probabilityKP)->earlySchedule();
        addToPopulationBegin(scheduleEarly);
        if (!record || (scheduleEarly->duration() < record->duration())) record = scheduleEarly;
        
        PSchedule scheduleLate = schedulePingPong(paramsGA.timesPingPongInitialPopulation,
                                                  paramsGA.probabilityKP)->lateSchedule();
        addToPopulationBegin(scheduleLate);
        if (!record || (scheduleLate->duration() < record->duration())) record = scheduleLate;

    }
    
    // generations
    
    int numberOfGeneratedSchedules = 0;
    while (numberOfGeneratedSchedules < paramsGA.maxGeneratedSchedules) {
        
//        LOG("generated schedules: "
//            << (float)numberOfGeneratedSchedules/paramsGA.maxGeneratedSchedules * 100 << "%"
//            << " record = " << record->duration());
        
        // select parents as subset of population
        vector<PSchedule> parentsEarly = selectParentsAsSubsetOfPopulation(&populationEarly);
        vector<PSchedule> parentsLate = selectParentsAsSubsetOfPopulation(&populationLate);
        
        // create children
        vector<PSchedule> children(0); children.reserve(paramsGA.maxChildren);
        while (children.size() < paramsGA.maxChildren) {
            
            // select 2 parents for crossing
            auto parent1 = parentsEarly[Random :: randomLong(0, parentsEarly.size() - 1)];
            auto parent2 = parentsLate[Random :: randomLong(0, parentsLate.size() - 1)];
            
//#warning test
//            LOG("p1 =  " << parent1->str());
//            LOG("p2 =  " << parent2->str());
            
            // crossing
#warning TODO create specific crossover method for various types?
            auto child = createEarlyOrLate(parent1->cross(parent2,
                                                          paramsGA.permissibleResourceRemains));
//#warning test
//            LOG("ch =  " << child->str());
            
            auto mutatedChild =
            child->swapAndMoveMutation(paramsGA.swapAndMovePermissibleTimes,
                                       paramsGA.swapAndMovePermissibleTimes)->pingPong();
            mutatedChild = createEarlyOrLate(mutatedChild);
            
//#warning test
//            LOG("chM = " << mutatedChild->str());
            
            if (child->duration() < record->duration()) {
                if (mutatedChild->duration() < child->duration()) child = mutatedChild;
                record = child;
            }
            else {
                child = mutatedChild;
            }
            
            children.push_back(child);
            numberOfGeneratedSchedules++;
        }
        
        // next population
        
//#warning test
//        LOG(stringFromSchedulesVector(&children));
        
        sort(begin(children), end(children), [](PSchedule a, PSchedule b){
            return a->duration() < b->duration();
        });
        
//#warning test
//        LOG(stringFromSchedulesVector(&children));
        
//#warning test
//        LOG("e = " << stringFromSchedulesVector(&populationEarly));
//        LOG("l = " << stringFromSchedulesVector(&populationLate));
        
        for(auto it=children.begin();
            it<children.begin()+paramsGA.numberOfChildrenInNextGeneration;
            it++) {
            addToPopulationBegin(*it);
        }
        
//#warning test
//        LOG("e = " << stringFromSchedulesVector(&populationEarly));
//        LOG("l = " << stringFromSchedulesVector(&populationLate));
        
        populationEarly.erase(populationEarly.begin() + paramsGA.populationSize / 2.0f,
                              populationEarly.end());
        populationLate.erase(populationLate.begin() + paramsGA.populationSize / 2.0f,
                             populationLate.end());
        
//#warning test
//        LOG("e = " << stringFromSchedulesVector(&populationEarly));
//        LOG("l = " << stringFromSchedulesVector(&populationLate));
    }
    
    return record;
}
