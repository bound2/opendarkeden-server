/////////////////////////////////////////////////////////////////////////////
// Filename : PKTSError.h
// Desc		: used by the online server to report a handling error result.
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_SERROR_H__
#define __PKT_SERROR_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// error code
enum MSERR_CODE {
    MSERR_MATCH = 0x01, // the member was found but the information does not
                        // match ( that is, the requester and the PowerJjang
                        // information do not agree )
};

// packet layout
struct _PKT_SERROR {
    int nSize;  // the size of the whole packet
    int nCode;  // packet code
    int nError; // error code
};

const int szPKTSError = sizeof(_PKT_SERROR);

// class PKTSError
class PKTSError : public _PKT_SERROR, public MPacket {
public:
    // constructor
    PKTSError();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTSError - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTSError;
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
    // Set the error code
    void setErrorCode(int errorCode) {
        nError = errorCode;
    }
};

#endif
