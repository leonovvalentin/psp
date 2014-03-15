//
//  ActiveList.h
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__ActiveList__
#define __psp2__ActiveList__



#include "Job.h"

#include <iostream>
#include <vector>

using namespace std;



class ActiveList
{
private:
    vector<Job *> _jobList;
    /**
     Total duration of all jobs.
     */
    int _duration;
    
public:

#pragma mark - init
    /**
     Constructor.
     Create activeList with empty jobList.
     */
    ActiveList();
    /**
     Constructor.
     Create random activeList.
     @param source Source of jobs list.
     @param jobsCapacity Expected count of jobs.
     */
    ActiveList(Job *source, unsigned long jobsCapacity);
    /**
     Constructor.
     Create activeList via jobList.
     @param jobList List of jobs.
     */
    ActiveList(const vector<Job *> *jobList);
    
#pragma mark - out
    friend ostream & operator<<(ostream &os, const ActiveList &activeList);
    string stringJobList() const;
    
#pragma mark - getters
    const vector<Job *> * jobList() const;
    /**
     Duration.
     @return Total duration of all jobs.
     */
    int duration();
    unsigned long size() const;
    Job * operator[](unsigned long index) const;
    Job * firstJob() const;
    Job * lastJob() const;
};



#endif /* defined(__psp2__ActiveList__) */
