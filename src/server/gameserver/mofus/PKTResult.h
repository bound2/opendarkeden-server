/////////////////////////////////////////////////////////////////////////////
// Filename : PKTResult.h
// Desc		: answers the PowerRing server whether the last data received
// 			  was handled.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_RESULT_H__
#define __PKT_RESULT_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_RESULT {
    int nSize; // the size of the whole packet
    int nCode; // packet code
};

const int szPKTResult = sizeof(_PKT_RESULT);

// class PKTResult
class PKTResult : public _PKT_RESULT, public MPacket {
public:
    // constructor
    PKTResult();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTResult - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTResult;
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
