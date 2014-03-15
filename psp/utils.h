//
//  utils.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__utils__
#define __psp2__utils__



#include "Job.h"
#include "ActiveList.h"

#include <iostream>
#include <vector>



class Schedule;



using namespace std;



#define PARAMETERS_OF_SELECTING_FUNCTION const vector<Job *> *jobs,\
                                         const Schedule *schedule,\
                                         const int time,\
                                         const bool timeForStart



typedef shared_ptr<vector<Job *>> JOBS_VECTOR_PTR;



JOBS_VECTOR_PTR selectJobFirstInActiveList(PARAMETERS_OF_SELECTING_FUNCTION);
JOBS_VECTOR_PTR selectJobsViaKP(PARAMETERS_OF_SELECTING_FUNCTION, float probability);

bool jobInList(const Job *job, const vector<Job *> *list);
string stringMATLARRectangle(int x, int y, int width, int height,
                             float redColor, float greenColor, float blueColor,
                             string textInside, string textOutside);
string stringFromJobsVector(const vector<Job *> *jobs);
string stringFromIntVector(const vector<int> *ints);
bool sameJobsInVector(const vector<Job *> *jobs);



#endif /* defined(__psp2__utils__) */
