//
//  utils.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Schedule.h"

#include "utils.h"
#include "Random.h"

#include <sstream>



JOBS_VECTOR_PTR selectJobFirstInActiveList(PARAMETERS_OF_SELECTING_FUNCTION)
{
    auto jobList = schedule->activeList()->jobList();
    auto minIterator = jobList->end();
    for (auto &job : *jobs) {
        auto jobIterator = find(jobList->begin(), jobList->end(), job);
        if (jobIterator < minIterator) minIterator = jobIterator;
    }
    return shared_ptr<vector<Job *>>(new vector<Job *>(1,*minIterator));
}

JOBS_VECTOR_PTR selectJobsViaKP(PARAMETERS_OF_SELECTING_FUNCTION, float probability)
{
    vector<Job *> pretenders(*jobs);
    
    map<Resource *, shared_ptr<vector<int>>> remains;
    for (auto &pResourceAmount : *schedule->resourceRemains()) {
        remains[pResourceAmount.first] =
        shared_ptr<vector<int>>(new vector<int>(pResourceAmount.second->begin(),
                                                pResourceAmount.second->end()));
    }
    
    shared_ptr<vector<Job *>> selectedJobs(new vector<Job *>(0));
    selectedJobs->reserve(pretenders.size());
    
    while (pretenders.size()) {
        
        // subset
        
        vector<Job *> subset(0); subset.reserve(pretenders.size());
        for (auto &job : pretenders) {
            if (Random :: randomFloatFrom0To1() < probability) subset.push_back(job);
        }
        if (subset.size() == 0) {
            subset.push_back(pretenders[Random :: randomLong(0, pretenders.size() - 1)]);
        }
        
        // max element
        
        Job *maxJob = NULL;
        for (auto &job : subset) {
            if (!maxJob ||
                job->resourceAmountSpecific() > maxJob->resourceAmountSpecific()) {
                maxJob = job;
            }
        }
        
        selectedJobs->push_back(maxJob);
        for (auto &pResourceAmount : *maxJob->resourceAmounts()) {
            if (pResourceAmount.second == 0) continue;
            int timeStart = timeForStart ? time : time - maxJob->duration();
            for (int t = timeStart; t < timeStart + maxJob->duration(); t++) {
                (*remains[pResourceAmount.first])[t] -= pResourceAmount.second;
            }
        }
        pretenders.erase(find(pretenders.begin(), pretenders.end(), maxJob));
        for (int i=0; i<pretenders.size(); i++) {
            Job *job = pretenders[i];
            bool validByResources = true;
            for (auto &pResourceAmount : *job->resourceAmounts()) {
                int timeStart = timeForStart ? time : time - job->duration();
                for (int t = timeStart; t < timeStart + job->duration(); t++) {
                    if (pResourceAmount.second > (*remains[pResourceAmount.first])[t]) {
                        pretenders.erase(pretenders.begin() + i);
                        i--;
                        validByResources = false;
                        break;
                    }
                }
                if (!validByResources) break;
            }
        }
    }
    
    return selectedJobs;
};

bool jobInList(const Job *job, const vector<Job *> *list)
{
    return find(begin(*list), end(*list), job) != end(*list);
}

string stringMATLARRectangle(int x, int y, int width, int height,
                             float redColor, float greenColor, float blueColor,
                             string textInside, string textOutside)
{
    stringstream ss;
    
    ss << "rectangle("
    << "'Position',[" << x << "," << y << "," << width << "," << height << "]"
    << ",'LineWidth',1" << ",'LineStyle','-'" << ",'EdgeColor',[0,0,0]"
    << ",'FaceColor',[" << redColor << "," << greenColor << "," << blueColor << "])"
    << endl
    << "text(" << x + width / 2.0f << "," << y + height / 2.0f << ",'" << textInside << "')"
    << endl
    << "text(" << x + width << "," << y + height + 0.5f << ",'" << textOutside << "')";
    
    return ss.str();
}

string stringFromJobsVector(const vector<Job *> *jobs)
{
    stringstream ss;
    
    if (!jobs->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    bool first = true;
    for (auto &job : *jobs) {
        if (first) first = false;
        else ss << ", ";
        ss << *job->name();
    }
    
    return ss.str();
}

string stringFromIntVector(const vector<int> *ints)
{
    stringstream ss;
    
    if (!ints->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    bool first = true;
    for (auto &item : *ints) {
        if (first) first = false;
        else ss << ", ";
        ss << item;
    }
    
    return ss.str();
}

string stringOfDurationsFromSchdulesVector(const vector<shared_ptr<Schedule>> *schedules)
{
    stringstream ss;
    
    if (!schedules->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    bool first = true;
    for (auto &item : *schedules) {
        if (first) first = false;
        else ss << ", ";
        ss << item->duration();
    }
    
    return ss.str();
}

string stringOfSumOfStartsFromSchdulesVector(const vector<shared_ptr<Schedule>> *schedules)
{
    stringstream ss;
    
    if (!schedules->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    bool first = true;
    for (auto &item : *schedules) {
        if (first) first = false;
        else ss << ", ";
        ss << item->sumOfStarts();
    }
    
    return ss.str();
}

bool sameJobsInVector(const vector<Job *> *jobs)
{
    for (int i=0; i<jobs->size(); i++) {
        Job *iJob = (*jobs)[i];
        for (int j=0; j<jobs->size(); j++) {
            Job *jJob = (*jobs)[j];
            if (i != j && iJob == jJob) return true;
        }
    }
    
    return false;
}
