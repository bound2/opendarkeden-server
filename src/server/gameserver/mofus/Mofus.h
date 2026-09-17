#ifndef __MOFUS_H__
#define __MOFUS_H__

/////////////////////////////////////////////////////////////////////////////
// Filename : Mofus.h
// Desc		: mofus header
/////////////////////////////////////////////////////////////////////////////

#include "Types.h"

// The mofus definition. When it is on, the mofus module runs.
#if defined(__METRO_SERVER__)
#define __MOFUS__
#endif

// log files
#define MOFUS_ERROR_FILE "mofus_error.txt"
#define MOFUS_LOG_FILE "mofus_log.txt"
#define MOFUS_PACKET_FILE "mofus_packet.txt"

int loadPowerPoint(const string& name);
int savePowerPoint(const string& name, int amount);
void logPowerPoint(const string& name, int recvPoint, int savePoint);

#endif // __MOFUS_H__
