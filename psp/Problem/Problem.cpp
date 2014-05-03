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

PSchedule Problem :: scheduleKochetovStolyar2003(float probabilityKP,
                                                 float probabilitySN,
                                                 int tabuListSize,
                                                 int changingInterval,
                                                 int maxIterationNumber) const
{
    function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> functionForSelecting =
    [probabilityKP](PARAMETERS_OF_SELECTING_FUNCTION) -> JOBS_VECTOR_PTR {
        return selectJobsViaKP(jobs, schedule, time, timeForStart, probabilityKP);
    };
    
    TabuList tabuList(tabuListSize);
    
    int stepsNoChange = 0; // Number of steps without changing neighbourhood
    NeighbourhoodType currentNeighbourhoodType = NeighbourhoodTypeEarly;
    
    PSchedule schedule = schedulePingPong(1, probabilityKP); // Initial schedule
    PSchedule record = schedule;
    tabuList.add(schedule->sumOfStarts());
    
    for (int iteration = 0; iteration < maxIterationNumber; iteration++) {
        
        // currentNeighbourhoodType
        
        if (stepsNoChange >= changingInterval) {
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
        
        auto neighboursWithoutTabu =
        shared_ptr<vector<PSchedule>>(new vector<PSchedule>(0));
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
            if (Random :: randomFloatFrom0To1() < probabilitySN) neighbours->push_back(neighbour);
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
        
        if (minNeighbourDuration < record->duration()) record = minNeighbour;
        schedule = minNeighbour;
        tabuList.add(minNeighbour->sumOfStarts());
        
        // Update stepsNoChange
        
        stepsNoChange++;
    }
    
    return record;
}

PSchedule Problem :: scheduleMyGA(int maxGeneratedSchedules,
                                  int populationSize,
                                  int maxParents,
                                  int maxChildren,
                                  int numberOfChildrenInNextGeneration,
                                  int timesPingPongInitialPopulation,
                                  float probabilityKP,
                                  float probabilityParentSelection,
                                  float permissibleResourceRemains,
                                  int swapAndMovePermissibleTimes) const
{
    PSchedule record = nullptr;
    
    // initial population
    vector<PSchedule> population(0);
    population.reserve(populationSize + numberOfChildrenInNextGeneration);
    for (int i=0; i<populationSize; i++) {
        LOG("initial population: " << (float)i/populationSize * 100 << "%");
        PSchedule schedule = schedulePingPong(timesPingPongInitialPopulation, probabilityKP);
        population.push_back(schedule);
        if (!record || (schedule->duration() < record->duration())) record = schedule;
    }
    
    int numberOfGeneratedSchedules = 0;
    while (numberOfGeneratedSchedules < maxGeneratedSchedules) {
        
        LOG("generated schedules: "
            << (float)numberOfGeneratedSchedules/maxGeneratedSchedules * 100 << "%");
        
        // select parents as subset of population
        sort(population.begin(), population.end(), [](PSchedule a, PSchedule b) {
            return a->duration() < b->duration();
        });
        vector<PSchedule> parents(0); parents.reserve(maxParents);
        for (int i=0; i<population.size() && parents.size() < maxParents; i++) {
            if (Random :: randomFloatFrom0To1() < probabilityParentSelection) {
                parents.push_back(population[i]);
            }
        }
        for (int i=0; parents.size() < maxParents; i++) {
            auto schedule = population[i];
            if (find(begin(parents), end(parents), schedule) == end(parents)) {
                parents.push_back(schedule);
            }
        }
        
        // create children
        vector<PSchedule> children(0); children.reserve(maxChildren);
        while (children.size() < maxChildren) {
            
            // select 2 parents for crossing
            auto parent1 = parents[Random :: randomLong(0, parents.size() - 1)];
            auto parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            while (parent2 == parent1) {
                parent2 = parents[Random :: randomLong(0, parents.size() - 1)];
            }
            
            // crossing
            auto child = parent1->cross(parent2, permissibleResourceRemains);
            auto mutatedChild = child->swapAndMoveMutation(swapAndMovePermissibleTimes,
                                                           swapAndMovePermissibleTimes)->pingPong();
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
                          children.begin() + numberOfChildrenInNextGeneration);
        population.erase(population.begin() + populationSize, population.end());
    }
    
    return record;
}
