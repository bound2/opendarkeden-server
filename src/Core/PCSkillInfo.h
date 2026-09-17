//////////////////////////////////////////////////////////////////////
//
// Filename    : PCSkillInfo.h
// Written By  : elca@ewestsoft.com
// Description :  Skill information
//
//////////////////////////////////////////////////////////////////////

#ifndef __PC_SKILL_INFO_H__
#define __PC_SKILL_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class PCSkillInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class PCSkillInfo {
public:
    // destructor
    virtual ~PCSkillInfo() {}

public:
    // Read data from the input stream (buffer) and initialise the packet.
    virtual void read(SocketInputStream& iStream) = 0;

    // Send the packet's binary image to the output stream (buffer).
    virtual void write(SocketOutputStream& oStream) const = 0;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    virtual PacketSize_t getSize() = 0;

    // get packet's debug string
    virtual string toString() const = 0;
};

#endif
