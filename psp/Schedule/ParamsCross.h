//
//  ParamsCrossViaSelectOneBlock.h
//  psp
//
//  Created by Valentin Leonov on 08/05/14.
//  Copyright (c) 2014 Valentin Leonov. All rights reserved.
//



#ifndef psp_ParamsCross_h
#define psp_ParamsCross_h



/**
 Parameters for schedule cross methtods.
 @param permissibleResourceRemains Relative resource remains which used for finding dense blocks of jobs.
 @param probabilityKP Probability for solving knapsack problem when constructing result schedule.
 @param withNet Indicate into account or not the net for selected block.
 @param isEarlyComposite If true, child schedule will be construct via compositeEarlyParallel decoder, otherwise via compositeLateParallel.
 */
struct ParamsCross {
    
    float permissibleResourceRemains;
    float probabilityKP;
    bool withNet;
#warning Create struct (with early and late) instead of using isEarlyComposite and !isEarlyComposite
    bool isEarlyComposite;
    
#pragma mark - table
    
    static string strTitlesForTable()
    {
        stringstream ss;
        
        ss
        << "permissibleResourceRemains"
        << "\tprobabilityKP"
        << "\twithNet"
        << "\tisEarlyComposite";
        
        return ss.str();
    }
    
    string strValuesForTable()
    {
        stringstream ss;
        
        ss
        << permissibleResourceRemains
        << "\t" << probabilityKP
        << "\t" << withNet
        << "\t" << isEarlyComposite;
        
        return ss.str();
    }
    
    string strForTable()
    {
        stringstream ss;
        ss << strTitlesForTable() << endl << strValuesForTable();
        return ss.str();
    }
    
    string str()
    {
        stringstream ss;
        
        ss
        << "permissibleResourceRemains = " << permissibleResourceRemains
        << "\nprobabilityKP = " << probabilityKP
        << "\nwithNet = " << withNet
        << "\nisEarlyComposite = " << isEarlyComposite;
        
        return ss.str();
    }
};



#endif
