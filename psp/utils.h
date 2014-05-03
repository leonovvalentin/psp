//
//  utils.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__utils__
#define __psp2__utils__



#include <iostream>
#include <vector>



class Solution;
class Problem;
class Schedule;
class ActiveList;
class Job;



using namespace std;


#define PATH_TO_LOG_FILE "/Users/valentinleonov/Documents/xCode/psp/psp/LOG.txt"
#define PATH_TO_DATA_FOLDER "/Users/valentinleonov/Documents/xCode/psp/psp/Data"

#define LOG_TO_COLSOL 1
#ifdef LOG_TO_COLSOL
#define LOG(str) do { cout << str << endl; } while (false)
#else
#define LOG(str)
#endif

#define LOG_TO_FILE 1
#ifdef LOG_TO_FILE
#define LOGF(str) {ofstream f; f.open(PATH_TO_LOG_FILE, ios::app); f << "\n\n\n"; do { f << str << endl; } while (false); f.close();}
#else
#define LOGF(str)
#endif

#define PARAMETERS_OF_SELECTING_FUNCTION const vector<Job *> *jobs,\
                                         const Schedule *schedule,\
                                         const int time,\
                                         const bool timeForStart



typedef shared_ptr<Schedule> PSchedule;
typedef shared_ptr<ActiveList> PActiveList;

typedef shared_ptr<vector<Job *>> JOBS_VECTOR_PTR;



JOBS_VECTOR_PTR selectJobFirstInActiveList(PARAMETERS_OF_SELECTING_FUNCTION);
JOBS_VECTOR_PTR selectJobsViaKP(PARAMETERS_OF_SELECTING_FUNCTION, float probability);

string stringFromSolutions(shared_ptr<map<Problem *, Solution>> solutions);
string stringFromSolutionsForTable(shared_ptr<map<Problem *, Solution>> solutions);

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
string stringMATLARRectangle(int x, int y, int width, int height,
                             float redColor, float greenColor, float blueColor,
                             string textInside, string textOutside);
string stringFromSchedulesVector(const vector<PSchedule> *schedules);
string stringFromJobsVector(const vector<Job *> *jobs);
string stringFromIntVector(const vector<int> *ints);
string stringOfDurationsFromSchdulesVector(const vector<PSchedule> *schedules);
string stringOfSumOfStartsFromSchdulesVector(const vector<PSchedule> *schedules);
string stringFromBlocksVector
(const shared_ptr<vector<shared_ptr<pair<shared_ptr<vector<Job *>>, float>>>> blocks);
string stringFromBlock(const shared_ptr<pair<shared_ptr<vector<Job *>>, float>> block);
bool sameJobsInVector(const vector<Job *> *jobs);



#endif /* defined(__psp2__utils__) */
