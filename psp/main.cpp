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
    Solver *solver = new Solver(&path, [](long i, string name){return i < 10;});
    
    // Kochetov, Stolyar, 2003
    /**
     probabilityKP = ?
     probabilitySN = 0.2
     tabuListSize = 5
     changingInterval = 5..10
     maxIterationNumber = 1000..5000
     */
    string userInfoKS;
    auto solutionsKS = solver->solveWithScheduleKochetovStolyar2003(0.5f,
                                                                    0.2f,
                                                                    5,
                                                                    10,
                                                                    5000,
                                                                    &userInfoKS);
    LOG(stringFromSolutions(solutionsKS));
    LOGF(userInfoKS << endl << stringFromSolutionsForTable(solutionsKS));
    
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
    string userInfoGA;
    auto solutionsGA = solver->solveWithMyGA(5000,
                                             40,
                                             20,
                                             40,
                                             10,
                                             100,
                                             0.5f,
                                             0.8f,
                                             0.9f,
                                             10,
                                             &userInfoGA);
    LOG(stringFromSolutions(solutionsGA));
    LOGF(userInfoGA << endl << stringFromSolutionsForTable(solutionsGA));
    
    delete solver;
    
    cout << "Finish. Total time =  " << time(NULL) - totalTime << "sec." << endl;
    return 0;
}
