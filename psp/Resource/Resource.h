//
//  Resource.h
//  psp2
//
//  Created by Valentin on 10/7/13.
//  Copyright (c) 2013 Valentin. All rights reserved.
//



#ifndef __psp2__Resource__
#define __psp2__Resource__



#include <iostream>

using namespace std;



class Resource
{
private:
    string _name;
    int _amount;
    
public:
    
#pragma mark - init
    
    /**
     Constructor.
     Create new resource.
     @param name Name.
     @param amount Amount of resource allocation in any time moment.
     */
    Resource(string *name, int amount);

#pragma mark - out
    friend ostream & operator<<(ostream &os, const Resource &resource);
    
#pragma mark - getters
    const string * name() const;
    int amount() const;
};



#endif /* defined(__psp2__Resource__) */
