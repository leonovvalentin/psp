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
#include <fstream>
#include <ctime>



using namespace std;



int main(int argc, const char * argv[])
{
    useCommaInsteadOfPointInConsolOut();
    
    cout << "Start" << endl;
    time_t totalTime; time(&totalTime);
    
    string path = PATH_TO_DATA_FOLDER;
    Solver *solver = new Solver(&path, [](long i, string name){
        return i == 0;
    });
    
    // Kochetov, Stolyar, 2003
//    ParamsKS paramsKS = {
//        .maxIterationNumber = 1000, // 1000-5000
//        .probabilityKP = 0.5f,
//        .probabilitySN = 0.2f, // 0.2
//        .tabuListSize = 5, // 5
//        .changingInterval = 10, // 5-10
//        .numberOfReturnsToRecord = 5 // 5
//    };
//    auto solutionsKS = solver->solveWithScheduleKS(paramsKS);
//    LOG(strSolutions(solutionsKS));
//    LOGF(paramsKS.str() << endl << strTableSolutions(solutionsKS));
    
    // My genetic algorithm
//    ParamsGA paramsGA = {
//        .maxGeneratedSchedules = 5000, // 1000, 5000, 50000
//        .populationSize = 40, // 40
//        .maxParents = 20, // 20
//        .maxChildren = 40, // 40
//        .numberOfChildrenInNextGeneration = 10, // 10
//        .timesPingPongInitialPopulation = 100, // 100
//        .probabilityKP = 0.5f, // -
//        .probabilityParentSelection = 0.8f, // 0.8
//        .permissibleResourceRemains = 0.9f, // 0.9
//        .swapAndMovePermissibleTimes = 10 // 10
//    };
//    auto solutionsGA = solver->solveWithScheduleGA(paramsGA);
//    LOG(strSolutions(solutionsGA));
//    LOGF(paramsGA.str() << endl << strTableSolutions(solutionsGA));
    
    // My genetic algorithm, 2014
    ParamsGA paramsGA = {
        .maxGeneratedSchedules = 1000, // 1000, 5000, 50000
        .populationSize = 40, // 40
        .maxParents = 20, // 20
        .maxChildren = 40, // 40
        .numberOfChildrenInNextGeneration = 10, // 10
        .timesPingPongInitialPopulation = 10, // 10
        .probabilityKP = 0.5f, // 0.5
        .probabilityParentSelection = 0.8f, // 0.8
        .permissibleResourceRemains = 0.9f, // 0.9
        .swapAndMovePermissibleTimes = 10 // 10
    };
    ParamsKS paramsKS = {
        .maxIterationNumber = 0, // 0
        .probabilityKP = 0.5f, // 0.5
        .probabilitySN = 0.2f, // 0.2
        .tabuListSize = 1, // 1
        .changingInterval = 1, // 1
        .numberOfReturnsToRecord = 0 // 0
    };
    ParamsCross paramsCross = {
        .permissibleResourceRemains = 0.9f, // 0.9
        .probabilityKP = 0.5f, // 0.5
        .withNet = true, // true
        .isEarlyComposite = false // false
    };
    int permissibleNoChangeRecord = 10;
    int numberOfSubstitutions = 10;
    int numberOfLocalSearchKS = 10;
    auto solutionsGA2014 = solver->solveWithScheduleGA2014(paramsGA,
                                                           paramsKS,
                                                           paramsCross,
                                                           permissibleNoChangeRecord,
                                                           numberOfSubstitutions,
                                                           numberOfLocalSearchKS);
    LOG(strSolutions(solutionsGA2014));
    LOGF(strParamsGA2014(paramsGA,
                         paramsKS,
                         paramsCross,
                         permissibleNoChangeRecord,
                         numberOfSubstitutions,
                         numberOfLocalSearchKS)
         << endl << strTableSolutions(solutionsGA2014));
    
    delete solver;
    
    cout << "Finish. Total time =  " << time(NULL) - totalTime << "sec." << endl;
    return 0;
}
