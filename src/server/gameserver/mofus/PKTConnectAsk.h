/////////////////////////////////////////////////////////////////////////////
// Filename : PKTConnectAsk.h
// Desc		: the online game server asks the PowerJjang server to connect,
// 			  sending its own game code with the request.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_CONNECT_ASK_H__
#define __PKT_CONNECT_ASK_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// packet layout
struct _PKT_CONNECT_ASK {
    int nSize;       // the size of the whole packet
    int nCode;       // packet code
    int nOnGameCode; // the online company's game code value issued by mofus
};

const int szPKTConnectAsk = sizeof(_PKT_CONNECT_ASK);

// class PKTConnectASK
class PKTConnectAsk : public _PKT_CONNECT_ASK, public MPacket {
public:
    // constructor
    PKTConnectAsk();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTConnectAsk - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTConnectAsk;
        Assert(pPacket != NULL);
        return pPacket;
    }

    // Reads data from the input stream and initialises the packet.
    void read(SocketInputStream& iStream);

    // Sends the packet's binary image to the output stream.
    void write(SocketOutputStream& oStream);

    // debug message
    string toString() const;

public:
    // get/set OnGameCode
    int getOnGameCode() const {
        return nOnGameCode;
    }
    void setOnGameCode(int onGameCode) {
        nOnGameCode = onGameCode;
    }
};

#endif
