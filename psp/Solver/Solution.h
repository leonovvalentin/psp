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
        
        ss << " duration = " << schedule->duration()
        << " errorToRecord = " << errorToRecord * 100 << "%"
        << " errorToCriticalPath = " << errorToCriticalPath * 100 << "%"
        << " calculationTime = " << calculationTime << "sec."
        << " isValid = " << *schedule->validationDescription();
        
        return ss.str();
    }
    void checkOnRecord(const string *problemName)
    {
        if (errorToRecord < 0) {
            
            stringstream ss;
            ss << "New record!"
            << endl << "Problem: " << *problemName
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
        << "duration"
        << "\terror to record, %"
        << "\terror to critical path, %"
        << "\tcalculation time, sec."
        << "\tis valid";
        
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
