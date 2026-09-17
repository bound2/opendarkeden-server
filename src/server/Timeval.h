//--------------------------------------------------------------------------------
//
// Filename    : Timeval.h
// Written By  : Reiot
// Description : a file collecting the operators that concern struct timeval
//
//--------------------------------------------------------------------------------

#ifndef __TIMEVAL_H__
#define __TIMEVAL_H__

// include files
#include <unistd.h>

#include <sys/time.h>

// type redefinition
typedef struct timeval Timeval;

extern Timeval gCurrentTime;

#ifdef __GAME_SERVER__
#define getCurrentTime(t) t = gCurrentTime
#define setCurrentTime() gettimeofday(&gCurrentTime, NULL)
#else
#define getCurrentTime(t) gettimeofday((&t), NULL)
#endif

bool operator>(const Timeval& left, const Timeval& right);
bool operator>=(const Timeval& left, const Timeval& right);
bool operator==(const Timeval& left, const Timeval& right);
bool operator<=(const Timeval& left, const Timeval& right);
bool operator<(const Timeval& left, const Timeval& right);

// Used when adding the results of timediff together.
// (adding two current times together is of course a silly thing to do.)
Timeval operator+(const Timeval& left, const Timeval& right);

//
// Computes the difference between two Timevals.
//
Timeval timediff(const Timeval& left, const Timeval& right);

void getCurrentYearTime(unsigned int& currentYearTime);

#endif
