//
//  TabuList.cpp
//  psp2
//
//  Created by Valentin Leonov on 02/02/14.
//  Copyright (c) 2014 Valentin. All rights reserved.
//



#include "TabuList.h"



#pragma mark - init

TabuList :: TabuList(int size)
{
    _size = size;
    _currentIndx = 0;
    _list = vector<int>(size, 0);
}

#pragma mark - getters

int TabuList :: size() const
{
    return _size;
}

int TabuList :: currentIndx() const
{
    return _currentIndx;
}

const vector<int> * TabuList :: list() const
{
    return &_list;
}

#pragma mark - functionality

void TabuList :: add(int tabu)
{
    _list[_currentIndx] = tabu;
    
    int nextIndx = _currentIndx + 1;
    if (nextIndx >= _size) _currentIndx = 0;
    else _currentIndx = nextIndx;
}
