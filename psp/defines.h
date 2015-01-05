//
//  defines.h
//  psp
//
//  Created by Valentin Leonov on 09/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_defines_h
#define psp_defines_h



#error Set correct paths
#define PATH_TO_LOG_FILE "/Users/valentinleonov/Documents/xCode/PSP/psp/LOG.txt"
#define PATH_TO_DATA_FOLDER "/Users/valentinleonov/Documents/xCode/psp/PSP/Data"

#define LOG_TO_COLSOL 1
#define LOG_TO_FILE 1

#define LOG_TO_CONSOL_SCHEDULE_H 1
#define LOG_TO_CONSOL_PROBLEM_H 1



#ifndef PATH_TO_LOG_FILE
#error PATH_TO_LOG_FILE needs to be set. You should set your path to LOG.txt file in main.cpp file
#endif
#ifndef PATH_TO_DATA_FOLDER
#error PATH_TO_DATA_FOLDER needs to be set. You should set your path to data folder in main.cpp file.
#endif

#ifdef LOG_TO_COLSOL
#define LOG(str) cout << str << endl;
#else
#define LOG(str)
#endif

#ifdef LOG_TO_FILE
#define LOGF(str) {ofstream f; f.open(PATH_TO_LOG_FILE, ios::app); f << "\n\n\n"; f << str << endl; f.close();}
#else
#define LOGF(str)
#endif



#endif
