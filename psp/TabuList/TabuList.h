//
//  TabuList.h
//  psp2
//
//  Created by Valentin Leonov on 02/02/14.
//  Copyright (c) 2014 Valentin. All rights reserved.
//



#ifndef __psp2__TabuList__
#define __psp2__TabuList__



#include <iostream>
#include <vector>

using namespace std;



class TabuList
{
private:
    unsigned long _maxSize;
    vector<int> _list;
    
public:
    
#pragma mark - init
    /**
     Constructor.
     Create new tabu list with specified size.
     @param size Specified size for tabu list.
     */
    TabuList(int size);
    
#pragma mark - out
    friend ostream & operator<<(ostream &os, const TabuList &tabuList);
    
#pragma mark - getters
    unsigned long size() const;
    unsigned long maxSize() const;
    
#pragma mark - functionality
    void add(int tabu);
    void removeOlderTabu(unsigned long numberOfTabuForRemoving);
    bool containTabu(int tabu);
    void changeMaxSize(unsigned long size);
};



#endif /* defined(__psp2__TabuList__) */
