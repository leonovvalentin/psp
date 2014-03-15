//
//  Schedule_out.cpp
//  psp2
//
//  Created by Valentin on 10/11/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Schedule.h"
#include "Random.h"

#include <sstream>



ostream &operator<<(ostream &os, const Schedule &schedule)
{
    os << "Schedule with active list:" << endl;
    os << schedule.activeList()->stringJobList() << endl;
    os << "type = " << schedule.type() << endl;
    os << "jobs start-end:" << endl;
    for (auto &pJobStart : schedule._starts) {
        os << *pJobStart.first->name() << " : "
        << pJobStart.second << " - " << pJobStart.second + pJobStart.first->duration() << endl;
    }
    return os;
}

string Schedule :: stringMATLAB()
{
    stringstream ss;
    
    // Resources rectangles
    
    ss << "\% Resources" << endl;
    
    vector<Resource *> resources;
    resources.reserve(_activeList.firstJob()->resourceAmounts()->size());
    for (auto &pResourceAmount : *_activeList.firstJob()->resourceAmounts()) {
        resources.push_back(pResourceAmount.first);
    }
    
    map<Resource *, int> levels;
    int yCurrent = 0;
    for (auto &resource : resources) {
        levels[resource] = yCurrent;
        ss << stringMATLARRectangle(0, yCurrent, this->duration(), resource->amount(),
                                    181.0f/255.0f, 213.0f/255.0f, 255.0f/255.0f,
                                    "", *resource->name()) << endl;
        yCurrent += resource->amount() + 5;
    }
    
    // Jobs rectangles
    
    ss << "\% Jobs" << endl;
    
    map<Job *, vector<float>> jobColors;
    for (auto &job : *_activeList.jobList()) {
        jobColors[job] = {
            Random :: pseudoRandomFloatFrom0To1(atoi(job->name()->c_str()), 0),
            Random :: pseudoRandomFloatFrom0To1(atoi(job->name()->c_str()), 1),
            Random :: pseudoRandomFloatFrom0To1(atoi(job->name()->c_str()), 2)
        };
    }
    
    for (auto &resource : resources) {
        ss << "\% Jobs for resource with name = " << *resource->name() << endl;
        
        map<Job *, int> jobLevels;
        
        for (auto &pJobStart : _starts) {
            Job *job = pJobStart.first;
            if (job->resourceAmount(resource) != 0) {
                ss << stringMATLARRectangle(start(job),
                                            levels[resource] + jobLevels[job],
                                            job->duration(),
                                            job->resourceAmount(resource),
                                            jobColors[job][0],
                                            jobColors[job][1],
                                            jobColors[job][2],
                                            *job->name(), "")
                << endl;
                
                for (auto &pJobStart2 : _starts) {
                    if ((pJobStart2.second < pJobStart.second + job->duration())
                        && (pJobStart2.second + pJobStart2.first->duration() > pJobStart.second)) {
                        jobLevels[pJobStart2.first] += job->resourceAmount(resource);
                    }
                }
            }
        }
    }
    
    return ss.str();
}
