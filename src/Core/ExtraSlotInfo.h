//////////////////////////////////////////////////////////////////////////////
// Filename    : ExtraSlotInfo.h
// Written By  : elca
// Description :
// Packet sent carrying information about the item hanging on the player's
// mouse cursor.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EXTRA_SLOT_INFO_H__
#define __EXTRA_SLOT_INFO_H__

#include "Exception.h"
#include "PCItemInfo.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ExtraSlotInfo
//////////////////////////////////////////////////////////////////////////////

class ExtraSlotInfo : public PCItemInfo {
public:
    ExtraSlotInfo();
    virtual ~ExtraSlotInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    string toString() const;
};

#endif
