/////////////////////////////////////////////////////////////////////////////
// Filename : PKTConnectAccept.h
// Desc		: reports through a packet that the connection succeeded.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_CONNECT_ACCEPT_H__
#define __PKT_CONNECT_ACCEPT_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_CONNECT_ACCEPT {
    int nSize; // the size of the whole packet
    int nCode; // packet code
};

const int szPKTConnectAccept = sizeof(_PKT_CONNECT_ACCEPT);

// class PKTConnectAccept
class PKTConnectAccept : public _PKT_CONNECT_ACCEPT, public MPacket {
public:
    // constructor
    PKTConnectAccept();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTConnectAccept - szMPacketSize;
    }

    // Creates a new packet and returns it.
    MPacket* create() {
        MPacket* pPacket = new PKTConnectAccept;
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
