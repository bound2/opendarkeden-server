//----------------------------------------------------------------------
//
// Filename    : PCInfo.h
// Writen By   : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __PC_INFO_H__
#define __PC_INFO_H__

// include files
#include "Exception.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"

//----------------------------------------------------------------------
//
// class PCInfo;
//
// Base of the SlayerPCInfo and VampirePCInfo classes that arrive in the
// LCPCList packet. The LCPCList packet stores PCInfo* [3], and by the DB's character
// count, the real SlayerPCInfo, VampirePCInfo or NULL is stored
// here.
//
//----------------------------------------------------------------------

class PCInfo {
public:
    // destructor
    virtual ~PCInfo() {}

    // Whether the current instance is a slayer or a vampire..
    virtual PCType getPCType() const = 0;

    //----------------------------------------------------------------------
    // An object that wants to be embedded in a packet has to state the following data.
    //----------------------------------------------------------------------

    // read data from socket input stream
    virtual void read(SocketInputStream& iStream) = 0;

    // write data to socket output stream
    virtual void write(SocketOutputStream& oStream) const = 0;

    // get size of object
    virtual uint getSize() const = 0;

    // get debug string
    virtual string toString() const = 0;
};

#endif
