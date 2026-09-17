/////////////////////////////////////////////////////////////////////////////
// Filename : PKTReceiveOK.h
// Desc		: answers the PowerRing server whether the data received was handled.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_RECEIVE_OK_H__
#define __PKT_RECEIVE_OK_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_RECEIVE_OK {
    int nSize; // the size of the whole packet
    int nCode; // packet code
};

const int szPKTReceiveOK = sizeof(_PKT_RECEIVE_OK);

// class PKTReceiveOK
class PKTReceiveOK : public _PKT_RECEIVE_OK, public MPacket {
public:
    // constructor
    PKTReceiveOK();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTReceiveOK - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTReceiveOK;
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
