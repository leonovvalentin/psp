//
//  main.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#error Set correct paths
#define PATH_TO_LOG_FILE "/Users/valentinleonov/Documents/xCode/psp/psp/LOG.txt"
#define PATH_TO_DATA_FOLDER "/Users/valentinleonov/Documents/xCode/psp/psp/Data"



#include "Solver.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <ctime>



// for using ',' instead of '.' in float numbers
#include <locale>
class comma : public numpunct<char>
{
public: comma () : numpunct<char> () {}
protected: char do_decimal_point() const { return ','; }
};



using namespace std;



#warning Refactor in all project: eighbour -> eighbor, neghb -> neighb



int main(int argc, const char * argv[])
{
    // for using ',' instead of '.' in float numbers
    locale loccomma(cout.getloc(), new comma); locale::global(loccomma);
    
    cout << "Start" << endl;
    time_t totalTime; time(&totalTime);
    
    string path = PATH_TO_DATA_FOLDER;
    Solver *solver = new Solver(&path, [](long i, string name){
        return i < 1;
    });
    
    // Kochetov, Stolyar, 2003
//    ParamsKS paramsKS = {
//        .maxIterationNumber = 5000, // 1000-5000
//        .probabilityKP = 0.5f, // ?
//        .probabilitySN = 0.2f, // 0.2
//        .tabuListSize = 5, // 5
//        .changingInterval = 10, // 5-10
//        .numberOfReturnsToRecord = 5 // 5
//    };
//    auto solutionsKS = solver->solveWithScheduleKS(paramsKS);
//    LOG(stringFromSolutions(solutionsKS));
//    LOGF(paramsKS.str() << endl << stringFromSolutionsForTable(solutionsKS));
    
    // My genetic algorithm
//    ParamsGA paramsGA = {
//        .maxGeneratedSchedules = 5000, // 5000
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
//    LOG(stringFromSolutions(solutionsGA));
//    LOGF(paramsGA.str() << endl << stringFromSolutionsForTable(solutionsGA));
    
    // My genetic algorithm, 2014
    ParamsGA paramsGA = {
        .maxGeneratedSchedules = 5000,
        .populationSize = 40,
        .maxParents = 20,
        .maxChildren = 40,
        .numberOfChildrenInNextGeneration = 10,
        .timesPingPongInitialPopulation = 100,
        .probabilityKP = 0.5f,
        .probabilityParentSelection = 0.8f,
        .permissibleResourceRemains = 0.9f,
        .swapAndMovePermissibleTimes = 10
    };
    ParamsKS paramsKS = {
        .maxIterationNumber = 20,
        .probabilityKP = 0.5f,
        .probabilitySN = 0.2f,
        .tabuListSize = 1,
        .changingInterval = 1,
        .numberOfReturnsToRecord = 5 // ?
    };
    ParamsCross paramsCross = {
        .permissibleResourceRemains = 0.9f,
        .probabilityKP = 0.5f,
        .withNet = true,
        .isEarlyComposite = false
    };
    int permissibleNoChangeRecord = 20;
    int numberOfSubstitutions = 10;
    int numberOfLocalSearchKS = 10;
    auto solutionsGA2014 = solver->solveWithScheduleGA2014(paramsGA,
                                                           paramsKS,
                                                           paramsCross,
                                                           permissibleNoChangeRecord,
                                                           numberOfSubstitutions,
                                                           numberOfLocalSearchKS);
    LOG(stringFromSolutions(solutionsGA2014));
    LOGF(strParamsGA2014(paramsGA,
                         paramsKS,
                         paramsCross,
                         permissibleNoChangeRecord,
                         numberOfSubstitutions,
                         numberOfLocalSearchKS)
         << endl << stringFromSolutionsForTable(solutionsGA2014));
    
    delete solver;
    
    cout << "Finish. Total time =  " << time(NULL) - totalTime << "sec." << endl;
    return 0;
}
