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
    _maxSize = size;
    _list = vector<int>(0); _list.reserve(size);
}

#pragma mark - out

ostream & operator<<(ostream &os, const TabuList &tabuList)
{
    os << "TabuList: ";
    
    bool first = true;
    for (auto &tabu : tabuList._list) {
        if (first) first = false;
        else os << ", ";
        os << tabu;
    }
    
    os << " maxSize: " << tabuList._maxSize
    << " currentSize: " << tabuList._list.size();
    
    return os;
}

#pragma mark - getters

unsigned long TabuList :: size() const
{
    return _list.size();
}

unsigned long TabuList :: maxSize() const
{
    return _maxSize;
}

#pragma mark - functionality

void TabuList :: add(int tabu)
{
    if (containTabu(tabu)) return;
    
    if (_maxSize > 0) {
        if (_list.size() >= _maxSize) _list.erase(_list.begin());
        _list.push_back(tabu);
    }
}

void TabuList :: removeOlderTabu(unsigned long numberOfTabuForRemoving)
{
    if (numberOfTabuForRemoving > _list.size()) numberOfTabuForRemoving = _list.size();
    _list.erase(_list.begin(), _list.begin() + numberOfTabuForRemoving);
}

bool TabuList :: containTabu(int tabu)
{
    return find(begin(_list), end(_list), tabu) != end(_list);
}

void TabuList :: changeMaxSize(unsigned long size)
{
    if (_list.size() > size) _list.erase(_list.begin(), _list.begin() + _list.size() - size);
    _maxSize = size;
}
