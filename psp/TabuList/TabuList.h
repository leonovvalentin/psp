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
    int _size;
    int _currentIndx;
    vector<int> _list;

public:
    
#pragma mark - init
    /**
     Constructor.
     Create new tabu list with specified size.
     @param size Specified size for tabu list.
     */
    TabuList(int size);
    
#pragma mark - getters
    int size() const;
    int currentIndx() const;
    const vector<int> * list() const;
    
#pragma mark - functionality
    void add(int tabu);
};



#endif /* defined(__psp2__TabuList__) */
