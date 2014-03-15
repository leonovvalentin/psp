//
//  Resource.cpp
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#include "Resource.h"



#pragma mark - init

Resource :: Resource(string *name, int amount)
{
    _name = *name;
    _amount = amount;
}

#pragma mark - out

ostream & operator<<(ostream &os, const Resource &resource)
{
    return os << "Resource with name: " << *resource.name() << " amount: " << resource.amount();
}

#pragma mark - getters

const string * Resource :: name() const
{
    return &_name;
}

int Resource :: amount() const
{
    return _amount;
}
