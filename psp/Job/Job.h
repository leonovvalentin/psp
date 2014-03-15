//
//  Job.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Job__
#define __psp2__Job__



#include "Resource.h"

#include <iostream>
#include <vector>
#include <map>

using namespace std;



class Job
{
private:
    string _name;
    int _duration;
    map<Resource *, int> _resourceAmounts;
    vector<Job *> _predecessors;
    vector<Job *> _successors;
    
    float _resourceAmountsSpecific;
    
public:
    
#pragma mark - init
    /**
     Constructor.
     Create new job.
     @param name Name of job.
     @param resourceAmounts Resources which required in each time moment of job duration.
     */
    Job(string *name, int duration, map<Resource *, int> *resourceAmounts);
    
#pragma mark - out
    friend ostream & operator<<(ostream &os, const Job &job);
    
#pragma mark - getters
    const string * name() const;
    int duration() const;
    const map<Resource *, int> * resourceAmounts() const;
    int resourceAmount(Resource *resource) const;
    const vector<Job *> * predecessors() const;
    const vector<Job *> * successors() const;
    
    float resourceAmountSpecific() const;
    
#pragma functionality
    void addPredecessor(Job *job);
    void addSuccessor(Job *job);
    bool hasPredecessor(Job *job);
    bool hasSuccessor(Job *job);
    /**
     Validation on precedence relation.
     @param jobs Jobs list.
     @return true if `jobs` contains all predecessors.
     */
    bool validByPredecessors(vector<Job *> *jobs);
    /**
     Validation on precedence relation.
     @param jobs Jobs list.
     @return true if `jobs` contains all successors.
     */
    bool validBySuccessors(vector<Job *> *jobs);
};



#endif /* defined(__psp2__Job__) */
