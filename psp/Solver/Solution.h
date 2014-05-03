#ifndef __psp__Solution__
#define __psp__Solution__



#include "Schedule.h"

#include <iostream>
#include <sstream>



using namespace std;



struct Solution {
    
    PSchedule schedule;
    float errorToRecord;
    float errorToCriticalPath;
    time_t calculationTime;
    
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
    string strForTable()
    {
        stringstream ss;
        
        ss
        << "duration"
        << "\terrorToRecord"
        << "\terrorToCriticalPath"
        << "\tcalculationTime"
        << "\tisValid"
        << endl
        << schedule->duration()
        << "\t" << errorToRecord * 100
        << "\t" << errorToCriticalPath * 100
        << "\t" << calculationTime
        << "\t" << *schedule->validationDescription();
        
        return ss.str();
    }
};



#endif /* defined(__psp__Solution__) */
