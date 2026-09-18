//////////////////////////////////////////////////////////////////////
//
// Filename    : Timeval.cpp
// Written By  : Reiot
// Description : a file collecting the operators that concern struct timeval
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Timeval.h"

#include "StringStream.h"
#include "stdlib.h"

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool operator>(const Timeval& left, const Timeval& right) {
    return left.tv_sec > right.tv_sec || (left.tv_sec == right.tv_sec && left.tv_usec > right.tv_usec);
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool operator>=(const Timeval& left, const Timeval& right) {
    return left.tv_sec > right.tv_sec || (left.tv_sec == right.tv_sec && left.tv_usec > right.tv_usec) ||
           (left.tv_sec == right.tv_sec && left.tv_usec == right.tv_usec);
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool operator==(const Timeval& left, const Timeval& right) {
    return left.tv_sec == right.tv_sec && left.tv_usec == right.tv_usec;
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool operator<=(const Timeval& left, const Timeval& right) {
    return left.tv_sec < right.tv_sec || (left.tv_sec == right.tv_sec && left.tv_usec < right.tv_usec) ||
           (left.tv_sec == right.tv_sec && left.tv_usec == right.tv_usec);
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
bool operator<(const Timeval& left, const Timeval& right) {
    return left.tv_sec < right.tv_sec || (left.tv_sec == right.tv_sec && left.tv_usec < right.tv_usec);
}


//--------------------------------------------------------------------------------
// Used when adding the results of timediff together.
// (adding two current times together is of course a silly thing to do.)
//--------------------------------------------------------------------------------
Timeval operator+(const Timeval& left, const Timeval& right) {
    Timeval result;

    result.tv_sec = left.tv_sec + right.tv_sec;
    result.tv_usec = left.tv_usec + right.tv_usec;

    if (result.tv_usec > 1000000) {
        result.tv_sec++;
        result.tv_usec -= 1000000;
    }

    return result;
}


//--------------------------------------------------------------------------------
// Computes the difference between two Timevals.
//--------------------------------------------------------------------------------
Timeval timediff(const Timeval& left, const Timeval& right) {
    Timeval result;

    if (left.tv_sec > right.tv_sec) {
        result.tv_sec = left.tv_sec - right.tv_sec;
        result.tv_usec = left.tv_usec - right.tv_usec;
        if (result.tv_usec < 0) {
            result.tv_sec--;
            result.tv_usec = 1000000 + result.tv_usec;
        }
    } else if (left.tv_sec == right.tv_sec) {
        result.tv_sec = 0;
        result.tv_usec = left.tv_usec - right.tv_usec;
        if (result.tv_usec < 0) {
            result.tv_usec = -result.tv_usec;
        }
    } else { // left.tv_sec < right.tv_sec
        result.tv_sec = right.tv_sec - left.tv_sec;
        result.tv_usec = right.tv_usec - left.tv_usec;
        if (result.tv_usec < 0) {
            result.tv_sec--;
            result.tv_usec = 1000000 + result.tv_usec;
        }
    }

    return result;
}

void getCurrentYearTime(unsigned int& currentYearTime) {
    time_t daytime = time(0);

    tm Timem;
    localtime_r(&daytime, &Timem);

    StringStream stime;

    stime << Timem.tm_year << Timem.tm_mon << Timem.tm_mday;

    currentYearTime = atoi(stime.toString().c_str());
}

Timeval gCurrentTime = Timeval();
