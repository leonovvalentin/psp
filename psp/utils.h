//
//  utils.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__utils__
#define __psp2__utils__



#include "defines.h"

#include "ParamsKS.h"
#include "ParamsGA.h"
#include "ParamsCross.h"

#include <iostream>
#include <vector>



class Solution;
class Problem;
class Schedule;
class ActiveList;
class Job;



using namespace std;



typedef shared_ptr<Schedule> PSchedule;
typedef shared_ptr<ActiveList> PActiveList;

typedef shared_ptr<vector<Job *>> PVectorJobs;



/**
 Launch this method if you want to use ',' instead of '.’ in the float numbers in console out.
 */
void useCommaInsteadOfPointInConsolOut();

#define PARAMETERS_OF_SELECTING_FUNCTION const vector<Job *> *jobs, const Schedule *schedule, const int time, const bool timeForStart
PVectorJobs selectJobFirstInActiveList(PARAMETERS_OF_SELECTING_FUNCTION);
PVectorJobs selectJobsViaKP(PARAMETERS_OF_SELECTING_FUNCTION, float probability);

/**
 @param jobs Jobs list, search in which will be.
 @param numberOfJob Number of job, distance for which successor will be calculated.
 @return Distance to first successor occurrence.
 */
long distanceToSuccessor(const vector<Job *> *jobs, const long numberOfJob);
/**
 @param jobs Jobs list, search in which will be.
 @param numberOfJob Number of job, distance for which predecessor will be calculated.
 @return Distance to first predecessor occurrence.
 */
long distanceToPredecessor(const vector<Job *> *jobs, const long numberOfJob);

void removeJobFromList(const Job *job, vector<Job *> *list);
bool jobInList(const Job *job, const vector<Job *> *list);
bool sameJobsInVector(const vector<Job *> *jobs);

int minHammingDistance(vector<PSchedule> *schedules, int hammingDispersion);
int maxHammingDistance(vector<PSchedule> *schedules, int hammingDispersion);

string strParamsGA2014(ParamsGA paramsGA,
                       ParamsKS paramsKS,
                       ParamsCross paramsCross,
                       int permissibleNoChangeRecord,
                       int numberOfSubstitutions,
                       int numberOfLocalSearchKS);

string strSolutions(shared_ptr<map<Problem *, Solution>> solutions);
string strTableSolutions(shared_ptr<map<Problem *, Solution>> solutions);

string stringMATLARRectangle(int x, int y, int width, int height,
                             float redColor, float greenColor, float blueColor,
                             string textInside, string textOutside);
string strVectorSchedules(const vector<PSchedule> *schedules);
string strVectorJobs(const vector<Job *> *jobs);
string strVectorInt(const vector<int> *ints);
string strDurationsFromVectorSchdules(const vector<PSchedule> *schedules);
string strSumOfStartsFromVectorSchdules(const vector<PSchedule> *schedules);
string strVectorBlocks(const shared_ptr<vector<shared_ptr<pair<PVectorJobs, float>>>> blocks);
string strBlock(const shared_ptr<pair<PVectorJobs, float>> block);



#endif /* defined(__psp2__utils__) */
