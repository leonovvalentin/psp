//
//  Solution.h
//  psp
//
//  Created by Valentin Leonov on 05/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef __psp__Solution__
#define __psp__Solution__



#include "Schedule.h"
#include "utils.h"

#include <iostream>
#include <sstream>
#include <fstream>



using namespace std;



struct Solution {
    
    PSchedule schedule;
    float errorToRecord;
    float errorToCriticalPath;
    time_t calculationTime;
    
#pragma mark - interface
    
    string str()
    {
        stringstream ss;
        
        ss << "duration = " << schedule->duration()
        << " errorToRecord = " << errorToRecord * 100 << "%"
        << " errorToCriticalPath = " << errorToCriticalPath * 100 << "%"
        << " calculationTime = " << calculationTime << "sec."
        << " isValid = " << *schedule->validationDescription();
        
        return ss.str();
    }
    void checkOnRecord(const string *problemName, const string *userInfo)
    {
        if (errorToRecord < 0) {
            
            stringstream ss;
            ss << "New record!"
            << endl << "Problem: " << *problemName
            << endl << *userInfo
            << endl << *schedule << "MATLAB:" << endl << schedule->stringMATLAB();
            
            LOG(ss.str());
            LOGF(ss.str())
        }
    }
    void checkOnValid(const string *problemName, const string *userInfo)
    {
        if (schedule->validation() != ScheduleValidOK) {
            
            stringstream ss;
            ss << "Error! Schedule is not valid: " << *schedule->validationDescription()
            << endl << "Problem: " << *problemName
            << endl << *userInfo
            << endl << *schedule << "MATLAB:" << endl << schedule->stringMATLAB();
            
            LOG(ss.str());
            LOGF(ss.str())
        }
    }
    
#pragma mark - table
    
    static string strTitlesForTable()
    {
        stringstream ss;
        
        ss
        << "Duration"
        << "\tError to record, %"
        << "\tError to critical path, %"
        << "\tCalculation time, sec."
        << "\tIs valid";
        
        return ss.str();
    }
    string strForTable()
    {
        stringstream ss;
        
        ss
        << schedule->duration()
        << "\t" << errorToRecord * 100
        << "\t" << errorToCriticalPath * 100
        << "\t" << calculationTime
        << "\t" << *schedule->validationDescription();
        
        return ss.str();
    }
};



#endif /* defined(__psp__Solution__) */
