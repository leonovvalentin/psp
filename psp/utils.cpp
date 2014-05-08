//
//  utils.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Solver.h"
#include "Problem.h"
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

string strParamsGA2014(ParamsGA paramsGA,
                       ParamsKS paramsKS,
                       ParamsCross paramsCross,
                       int permissibleNoChangeRecord,
                       int numberOfSubstitutions,
                       int numberOfLocalSearchKS)
{
    stringstream ss;
    
    ss
    << "ParamsGA: "
    << paramsGA.str()
    << "\n\nParamsKS: " << paramsKS.str()
    << "\n\nParamsCross: " << paramsCross.str()
    << "\n\npermissibleNoChangeRecord = " << permissibleNoChangeRecord
    << ", numberOfSubstitutions = " << numberOfSubstitutions
    << ", numberOfLocalSearchKS = " << numberOfLocalSearchKS;
    
    return ss.str();
}

string strForTableFromParamsGA2014(ParamsGA paramsGA,
                                   ParamsKS paramsKS,
                                   ParamsCross paramsCross,
                                   int permissibleNoChangeRecord,
                                   int numberOfSubstitutions,
                                   int numberOfLocalSearchKS)
{
    stringstream ss;
    
    ss
    << paramsGA.strTitlesForTable()
    << "\t" << paramsKS.strTitlesForTable()
    << "\t" << paramsCross.strTitlesForTable()
    << "\tpermissibleNoChangeRecord"
    << "\tnumberOfSubstitutions" <<
    "\tnumberOfLocalSearchKS"
    << endl
    << paramsGA.strValuesForTable()
    << "\t" << paramsKS.strValuesForTable()
    << "\t" << paramsCross.strValuesForTable()
    << "\t" << permissibleNoChangeRecord
    << "\t" << numberOfSubstitutions
    << "\t" << numberOfLocalSearchKS;
    
    return ss.str();
}

string stringFromSolutions(shared_ptr<map<Problem *, Solution>> solutions)
{
    stringstream ss;
    
    float averageErrorToRecord = 0, averageErrorToCriticalPath = 0, averageCalculationTime = 0;
    for (auto &pProblemResult : *solutions) {
        
        Solution solution = pProblemResult.second;
        ss << *pProblemResult.first->name() << ": " << solution.str() << endl;
        
        averageErrorToRecord += solution.errorToRecord / solutions->size();
        averageErrorToCriticalPath += solution.errorToCriticalPath / solutions->size();
        averageCalculationTime += (float)solution.calculationTime / solutions->size();
    }
    
    ss << "averageErrorToRecord = " << averageErrorToRecord * 100 << "%"
    << " averageErrorToCriticalPath = " << averageErrorToCriticalPath * 100 << "%"
    << " averageCalculationTime = " << averageCalculationTime << "sec.";
    
    return ss.str();
}

string stringFromSolutionsForTable(shared_ptr<map<Problem *, Solution>> solutions)
{
    stringstream ss;
    
    ss << "Name\t" << Solution :: strTitlesForTable() << endl;
    
    float averageErrorToRecord = 0, averageErrorToCriticalPath = 0, averageCalculationTime = 0;
    for (auto &pProblemResult : *solutions) {
        
        Solution solution = pProblemResult.second;
        ss << *pProblemResult.first->name() << "\t" << solution.strValuesForTable() << endl;
        
        averageErrorToRecord += solution.errorToRecord / solutions->size();
        averageErrorToCriticalPath += solution.errorToCriticalPath / solutions->size();
        averageCalculationTime += (float)solution.calculationTime / solutions->size();
    }
    
    ss
    << "average\t\t"
    << averageErrorToRecord * 100 << "\t"
    << averageErrorToCriticalPath * 100 << "\t"
    << averageCalculationTime;
    
    return ss.str();
}

long distanceToSuccessor(const vector<Job *> *jobs, const long numberOfJob)
{
    Job *job = (*jobs)[numberOfJob];
    
    long n = numberOfJob + 1;
    while (n < jobs->size()) {
        
        Job *nextJob = (*jobs)[n];
        
        if (job->hasSuccessor(nextJob)) {
            return n - numberOfJob;
        }
        else {
            n++;
        }
    }
    
    return jobs->size() - 1 - numberOfJob;
}

long distanceToPredecessor(const vector<Job *> *jobs, const long numberOfJob)
{
    Job *job = (*jobs)[numberOfJob];
    
    long n = numberOfJob - 1;
    while (n >= 0) {
        
        Job *prevJob = (*jobs)[n];
        
        if (job->hasPredecessor(prevJob)) {
            return numberOfJob - n;
        }
        else {
            n--;
        }
    }
    
    return numberOfJob;
}

void removeJobFromList(const Job *job, vector<Job *> *list)
{
    auto jobIt = find(begin(*list), end(*list), job);
    if (jobIt != end(*list)) {
        list->erase(jobIt);
    }
}

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

string stringFromSchedulesVector(const vector<PSchedule> *schedules)
{
    stringstream ss;
    
    if (!schedules->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    bool first = true;
    for (auto &schedule : *schedules) {
        if (first) first = false;
        else ss << ", ";
        ss << schedule->duration();
    }
    
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
        ss << (job ? (*job->name()) : "NULL");
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

string stringOfDurationsFromSchdulesVector(const vector<PSchedule> *schedules)
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

string stringOfSumOfStartsFromSchdulesVector(const vector<PSchedule> *schedules)
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

string stringFromBlocksVector
(const shared_ptr<vector<shared_ptr<pair<shared_ptr<vector<Job *>>, float>>>> blocks)
{
    stringstream ss;
    
    if (!blocks->size()) {
        ss << "Vector is empty";
        return ss.str();
    }
    
    for (int i=0; i<blocks->size(); i++) {
        ss << stringFromBlock((*blocks)[i]);
        if (i < blocks->size() - 1) ss << endl;
    }
    
    return ss.str();
}

string stringFromBlock(const shared_ptr<pair<shared_ptr<vector<Job *>>, float>> block)
{
    stringstream ss;
    ss << stringFromJobsVector(block->first.get()) << " : " << block->second;
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

int minHammingDistance(vector<PSchedule> *schedules, int hammingDispersion)
{
    int minDist = INT_MAX;
    for (int i=0; i<schedules->size(); i++) {
        PSchedule schedule1 = (*schedules)[i];
        for (int j=i+1; j<schedules->size(); j++) {
            PSchedule schedule2 = (*schedules)[j];
            int dist = schedule1->hammingDistance(schedule2, hammingDispersion);
            if (dist < minDist) minDist = dist;
        }
    }
    return minDist;
}

int maxHammingDistance(vector<PSchedule> *schedules, int hammingDispersion)
{
    int maxDist = INT_MIN;
    for (int i=0; i<schedules->size(); i++) {
        PSchedule schedule1 = (*schedules)[i];
        for (int j=i+1; j<schedules->size(); j++) {
            PSchedule schedule2 = (*schedules)[j];
            int dist = schedule1->hammingDistance(schedule2, hammingDispersion);
            if (dist > maxDist) maxDist = dist;
        }
    }
    return maxDist;
}
