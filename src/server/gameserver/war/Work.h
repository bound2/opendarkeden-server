///////////////////////////////////////////////////////////////////
// Work abstract class for scheduled work.
///////////////////////////////////////////////////////////////////

#ifndef __WORK_H__
#define __WORK_H__

#include <iostream>

#include "Exception.h"

class Work {
public:
    Work() {}
    virtual ~Work() {}

public:
    virtual void execute() = 0;
    virtual string toString() const = 0;
};

#endif // __WORK_H__
