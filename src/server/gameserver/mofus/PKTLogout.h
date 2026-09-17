/////////////////////////////////////////////////////////////////////////////
// Filename : PKTLogout.h
// Desc		: reports that the connection is closing.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_LOGOUT_H__
#define __PKT_LOGOUT_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_LOGOUT {
    int nSize; // the size of the whole packet
    int nCode; // packet code
};

const int szPKTLogout = sizeof(_PKT_LOGOUT);

// class PKTLogout
class PKTLogout : public _PKT_LOGOUT, public MPacket {
public:
    // constructor
    PKTLogout();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTLogout - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTLogout;
        Assert(pPacket != NULL);
        return pPacket;
    }

    // Reads data from the input stream and initialises the packet.
    void read(SocketInputStream& iStream);

    // Sends the packet's binary image to the output stream.
    void write(SocketOutputStream& oStream);

    // debug message
    string toString() const;
};

#endif
