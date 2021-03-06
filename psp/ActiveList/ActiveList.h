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
#include "utils.h"

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
    
#pragma mark - init
public:
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
    
#pragma mark - interface
public:
    /**
     Swap 2 random jobs, and move some job to another position. Not more times than specified number of times for each operation.
     @return ActiveList, created from current ActiveList by swapping and moving random jobs.
     */
    PActiveList swapAndMove(const int swapPermissibleTimes,
                            const int movePermissibleTimes) const;
    /**
     @param dispersion Indicates width (from the current position in specified activeList, current position ± dispertion) in which same element will be searched.
     @return Hamming distance between current and specified activLists.
     */
    int hammingDistance(const ActiveList *activeList, const int dispersion) const;
    
#pragma mark - out
public:
    friend ostream & operator<<(ostream &os, const ActiveList &activeList);
    string stringJobList() const;
    
#pragma mark - getters
public:
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
