//
//  main.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Solver.h"
#include "utils.h"

#include <iostream>
#include <ctime>



using namespace std;



#warning Refactor in all project: eighbour -> eighbor, neghb -> neighb



int main(int argc, const char * argv[])
{
    cout << "Start" << endl;
    time_t totalTime; time(&totalTime);
    
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
//    auto solutionsKS = solver->solveWithScheduleKochetovStolyar2003(0.5f, 0.2f, 5, 10, 5000);
//    LOG(stringFromSolutions(solutionsKS));
    
    // My genetic algorithm
    /**
     maxGeneratedSchedules = 5000
     populationSize = 40
     maxParents = 20
     maxChildren = 40
     numberOfChildrenInNextGeneration = 10
     timesPingPongInitialPopulation = 100
     probabilityKP -
     probabilityParentSelection = 0.8
     permissibleResourceRemains = 0.9
     swapAndMovePermissibleTimes = 10
     */
//    auto solutionsGA = solver->solveWithMyGA(5000, 40, 20, 40, 10, 100, 0.5f, 0.8f, 0.9f, 10);
//    LOG(stringFromSolutions(solutionsGA));
    
    delete solver;
    
    cout << "Finish. Total time =  " << time(NULL) - totalTime << "sec." << endl;
    return 0;
}
