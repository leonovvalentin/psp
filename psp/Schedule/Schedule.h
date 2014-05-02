//
//  Schedule.h
//  psp2
//
//  Created by Valentin on 10/8/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Schedule__
#define __psp2__Schedule__



#include "ActiveList.h"
#include "Job.h"
#include "Resource.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <map>

using namespace std;



typedef enum ScheduleTypes {
    ScheduleTypeUnknown = 0,
    ScheduleTypeEarly,
    ScheduleTypeLate,
    ScheduleTypeEarlyParallel,
    ScheduleTypeLateParallel,
    ScheduleTypeEarlyComposite,
    ScheduleTypeLateComposite,
} ScheduleType;

typedef enum NeighbourhoodTypes {
    NeighbourhoodTypeEarly = 0,
    NeighbourhoodTypeLate
} NeighbourhoodType;

typedef enum ScheduleValids {
    ScheduleValidOK = 0,
    ScheduleValidNoResourceRestriction,
    ScheduleValidNoPresedenceRelation
} ScheduleValid;



class Schedule : public enable_shared_from_this<Schedule>
{
private:
    ScheduleType _type;
    ActiveList _activeList;
    map<Job *, int> _starts;
    int _duration;
    const vector<Resource *> *_resources;
    map<Resource *, shared_ptr<vector<int>>> _resourceRemains;
    
#pragma mark - init
private:
    /**
     Constructor.
     Use as first initialization. Ceating empty shedule.
     @param activeList ActiveList by which shedule will be created.
     @param resources Resources necessary for jobs in actilveList.
     */
    Schedule(ActiveList *activeList, const vector<Resource *> *resources);
    /**
     Early partial parallel schedule. First part of activeList will be started with time moments \
     from `starts`, second part of activeList will be schedule via parallel decoder, third part of \
     activeList will be scheduled via early decoder.
     @param activeList ActiveList by which shedule will be created.
     @param resources Available resources.
     @param minIterator Begin-iterator of `activeList.jobList` middle part.
     @param maxIterator End-iterator of `activeList.jobList` middle part.
     @param starts Starts of jobs which positions in activeList less then minIterator.
     @param functionForSelecting Function for selecting jobs which will be next scheduled (for \
     jobs from meddle part).
     @return Schedule created by partialy early parallel decoder.
     */
#warning refactor to scheduleCompositeEarlyParallel
    static shared_ptr<Schedule> schedulePartialyEarlyParallel
    (ActiveList *activeList,
     const vector<Resource *> *resources,
     vector<Job *> :: const_iterator minIterator,
     vector<Job *> :: const_iterator maxIterator,
     const map<Job *, int> *starts,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
    /**
     Late partial parallel schedule. Last part of activeList will be started with time moments \
     from `starts`, middle part of activeList will be schedule via parallel decoder, first part of \
     activeList will be scheduled via late decoder.
     @param activeList ActiveList by which shedule will be created.
     @param resources Available resources.
     @param minIterator Begin-iterator of `activeList.jobList` middle part.
     @param maxIterator End-iterator of `activeList.jobList` middle part. It should be such that after middle part remains at least one job.
     @param starts Starts of jobs which positions in activeList more then maxIterator.
     @param functionForSelecting Function for selecting jobs which will be next scheduled (for \
     jobs from meddle part).
     @return Schedule created by partialy late parallel decoder.
     */
#warning refactor to scheduleCompositeLateParallel
#warning May be needs to add right handling of case when maxIterator contain end job(When section3 is empty)?
    static shared_ptr<Schedule> schedulePartialyLateParallel
    (ActiveList *activeList,
     const vector<Resource *> *resources,
     vector<Job *> :: const_iterator minIterator,
     vector<Job *> :: const_iterator maxIterator,
     const map<Job *, int> *starts,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
public:
    /**
     Early schedule.
     @param activeList ActiveList by which shedule will be created.
     @param resources Resources necessary for jobs in actilveList.
     @return Schedule created by early decoder.
     */
    static shared_ptr<Schedule> scheduleEarly(ActiveList *activeList,
                                              const vector<Resource *> *resources);
    /**
     Late schedule.
     @param activeList ActiveList by which shedule will be created.
     @param resources Resources necessary for jobs in actilveList.
     @return Schedule created by late decoder.
     */
    static shared_ptr<Schedule> scheduleLate(ActiveList *activeList,
                                             const vector<Resource *> *resources);
    /**
     Early parallel schedule.
     @param activeList ActiveList by which shedule will be created.
     @param resources Resources necessary for jobs in actilveList.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     @return Schedule created by early parallel decoder.
     */
    static shared_ptr<Schedule> scheduleEarlyParallel
    (ActiveList *activeList,
     const vector<Resource *> *resources,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
    
#pragma mark - out
public:
    friend ostream & operator<<(ostream &os, const Schedule &schedule);
    string stringMATLAB();
    
#pragma mark - getters
public:
    ScheduleType type() const;
    const ActiveList * activeList() const;
    int duration();
    int start(Job *job) const;
    int end(Job *job) const;
    int resourceRemain(Resource *resource, int timeMoment) const;
    shared_ptr<map<Resource *, int>> resourceRemain(int timeMoment) const;
    const map<Resource *, shared_ptr<vector<int>>> * resourceRemains() const;
    
#pragma mark - functionality
public:
    /**
     Creating schedule by crossing 2 schedules.
     @param schedule Schedule for crossing with current schedule.
     @param permissibleResourceRemains Relative resource remains which used for finding dense blocks of jobs.
     @return Child schedule.
     */
    shared_ptr<Schedule> cross(shared_ptr<Schedule> schedule, float permissibleResourceRemains);
    /**
     Creating early schedule from current via sorting jobs list by begins of jobs.
     @return Early schedule.
     */
    shared_ptr<Schedule> earlySchedule();
    /**
     Creating late schedule from current via sorting jobs list by ends of jobs.
     @return Late schedule.
     */
    shared_ptr<Schedule> lateSchedule();
    /**
     Creating neghbour schedules from current, based on rescheduled jobs from block of some job.
     @param neighbourhoodType Type of neighborhood.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     @return List of composite schedules constructed with specified neighborhood type.
     */
    shared_ptr<vector<shared_ptr<Schedule>>> neighboringSchedules
    (NeighbourhoodType neighbourhoodType,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
private:
    /**
     Creating neghbour for late schedule, based on rescheduled jobs from block of specified job.
     @warning Should be applied to late schedule types only.
     @param job Specified job for which will be found block of jobs for rescheduling.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     @return Composite schedule, if current schedule is late, otherwise null_ptr.
     */
    shared_ptr<Schedule> neighbourForLateSchedule
    (Job *job,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
    /**
     Creating neghbour for early schedule, based on rescheduled jobs from block of specified job.
     @warning Should be applied to early schedule types only.
     @param job Specified job for which will be found block of jobs for rescheduling.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     @return Composite schedule, if current schedule is early, otherwise null_ptr.
     */
    shared_ptr<Schedule> neighbourForEarlySchedule
    (Job *job,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
public:
    ScheduleValid validation();
    
#pragma mark - helper methods
private:
    /**
     Find block of jobs which are performed simultaneously give small remaining unused resource.
     @param permissibleResourceRemains Returned jobs are performed simultaneously will be give relative resource remains less or equal then this parameter (resource remains <= permissible resource remains).
     */
    shared_ptr<map<shared_ptr<vector<Job *>>, float>> denseJobsBlocks
    (float permissibleResourceRemains);
    /**
     Add jobs on schedule via early decoder.
     @param jobs Jobs which will be added on schedule.
     */
    void addJobsOnScheduleViaEarlyDecoder(const vector<Job *> *jobs);
    /**
     Add jobs on schedule via late decoder.
     @param jobs Jobs which will be added on schedule.
     */
    void addJobsOnScheduleViaLateDecoder(const vector<Job *> *jobs);
    /**
     Add jobs on schedule via early parallel decoder.
     @param jobs Jobs which will be added on schedule.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     */
    void addJobsOnScheduleViaEarlyParallelDecoder
    (vector<Job *> jobs,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
    /**
     Add jobs on schedule via late parallel decoder.
     @param jobs Jobs which will be added on schedule.
     @param functionForSelecting Function for selecting jobs which will be next scheduled.
     */
    void addJobsOnScheduleViaLateParallelDecoder
    (vector<Job *> jobs,
     function<JOBS_VECTOR_PTR(PARAMETERS_OF_SELECTING_FUNCTION)> &functionForSelecting);
    /**
     Calculate active (which is in process or is started at this `time`) jobs.
     @param time Time for which jobs will be calculated.
     @return Active jobs.
     */
    shared_ptr<vector<Job *>> calcActiveJobs(int time);
    /**
     Calculate completed and active (which is in process or is started at this `time`) jobs.
     @param time Time for which jobs will be calculated.
     @param completed Vector in which calculated completed jobs will be stored.
     @param active Vector in which calculated active jobs will be stored (which is in process or \
     is started at this `time`).
     */
    void calcCompletedAndActiveJobs(int time, vector<Job *> *completed, vector<Job *> *active);
    /**
     Calculate jobs which started after or at this `time` and active jobs (jobs which is in \
     process or is ended at this `time`).
     @param time Time for which jobs will be calculated.
     @param started Vector in which calculated started jobs will be stored.
     @param active Vector in which calculated active jobs (jobs which is in process or is ended at \
     this `time`).
     */
    void calcStartedAndActiveJobs(int time, vector<Job *> *completed, vector<Job *> *active);
    /**
     Incoming network.
     @param job Job for which incoming network will be created.
     @return Incoming network.
     */
    shared_ptr<vector<Job *>> incomingNetwork(Job *job);
    /**
     Outgoing network.
     @param job Job for which outgoing network will be created.
     @return Outgoing network.
     */
    shared_ptr<vector<Job *>> outgoingNetwork(Job *job);
    /**
     Block of jobs.
     @param job Job for which block will be created.
     @param withPredecessors If true, blocks with predecessors will be ignored and returned nullptr.
     @param withSuccessors If true, blocks with successors will be ignored and returned nullptr.
     @return Block of jobs.
     */
    shared_ptr<vector<Job *>> blockOfJobs(Job *job, bool withPredecessors, bool withSuccessors);
    /**
     Reduce resource remain.
     @param job Job for which needs to reduce.
     */
    void reduceResourceRemain(Job *job);
    /**
     @param jobs List of jobs from which will be selected resulting list.
     @param time Expected time of jobs beginning (if `timeForStart` is true) or ended (if \
     `timeForStart` is false).
     @param timeForStart If true, then `time` is time of jobs beginning, else of jobs finishing.
     @return Jobs which can be started at time `time` given the resource constraints.
     */
    shared_ptr<vector<Job *>> permissibleJobsByResources(vector<Job *> *jobs,
                                                         int time,
                                                         bool timeForStart);
    /**
     Shift all job starts and resource remains on `time`.
     @param time Step for shifting.
     */
    void shift(int time);
    /**
     @param time Time moment, result for which will be calculated.
     @return Relative resource remains at specified time moment.
     */
    float relativeResourceRemains(int time);
public:
    /**
     Validate schedule and create description.
     @return Description about schedule validity.
     */
    shared_ptr<string> validationDescription();
    /**
     Sum of starts.
     @return Sum of starts.
     */
    int sumOfStarts();
};



#endif /* defined(__psp2__Schedule__) */
