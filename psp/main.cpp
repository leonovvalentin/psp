//
//  main.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Solver.h"

#include <iostream>
using namespace std;



#warning Refactor in all project: eighbour -> eighbor
#warning Refactor in all project: neghb -> neighb



int main(int argc, const char * argv[])
{
    cout << "Start" << endl;
    
    string path = "/Users/valentinleonov/Documents/xCode/psp2/psp2/Data";
    Solver *solver = new Solver(&path);
    auto solve = solver->solveWithScheduleKochetovStolyar2003(1, 0.5f, 4, 5);
    for (auto &pProblemSchedule : *solve) {
        cout << *pProblemSchedule.first->name() << " = " << pProblemSchedule.second->duration()
        << " " << *pProblemSchedule.second->validationDescription() << endl;
    }
    delete solver;
    
    cout << "Finish" << endl;
    return 0;
}
