//
//  main.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Solver.h"

#include <iostream>
#include <ctime>



using namespace std;



#warning Refactor in all project: eighbour -> eighbor, neghb -> neighb



int main(int argc, const char * argv[])
{
    cout << "Start" << endl;
    time_t t;
    time(&t);
    
    string path = "/Users/valentinleonov/Documents/xCode/psp/psp/Data";
    Solver *solver = new Solver(&path);
    
    // Kochetov, Stolyar, 2003
    /**
     probabilityKP = ?
     probabilitySN = 0.2
     tabuListSize = 5
     changingInterval = 5..10
     maxIterationNumber = 1000..5000
     */
//    auto solve = solver->solveWithScheduleKochetovStolyar2003(0.5f, 0.2f, 5, 10, 1000);
    
    // My genetic algorithm
    /**
     maxGeneratedSchedules = 5000
     populationSize = 40
     maxParents = 20
     maxChildren = 40
     timesPingPongInitialPopulation = 100
     probabilityKP -
     probabilityParentSelection = 0.8
     */
    auto solve = solver->solveWithMyGA(10, 7, 4, 6, 1, 0.5f, 0.8f);
    
    for (auto &pProblemSchedule : *solve) {
        cout << *pProblemSchedule.first->name() << " = " << pProblemSchedule.second->duration()
        /*<< " " << *pProblemSchedule.second->validationDescription()*/ << endl;
    }
    delete solver;
    
    cout << "Finish. Duration =  " << time(NULL) - t << endl;
    return 0;
}
