/////////////////////////////////////////////////////////////////////////////
// Filename : PKTError.h
// Desc		: used by the PowerJjang server to report an error result.
// 			  The online server treats them all as a server error and mostly
// 			  uses them for debugging
/////////////////////////////////////////////////////////////////////////////

#ifndef __PKT_ERROR_H__
#define __PKT_ERROR_H__

// include files
#include "Assert.h"
#include "MPacket.h"

// error code
enum MERR_CODE {
    MERR_SERVER = 0x01,    // the server is alive but cannot currently
                           // work properly
    MERR_CONFIRM = 0x02,   // the online game code check failed
    MERR_PACKET = 0x03,    // a wrong packet was sent or received
    MERR_PROCESS = 0x04,   // server handling error ( ex: DB error )
    MERR_SEARCH = 0x05,    // not a PowerJjang member
    MERR_NULLPOINT = 0x06, // a PowerJjang member, but with no PowerJjang points
                           // accumulated
    MERR_MATCHING = 0x07,  // matching information error (ex: no member matching information)
                           // send the text that steers the user to match on the home page.
};

// packet layout
struct _PKT_ERROR {
    int nSize;  // the size of the whole packet
    int nCode;  // packet code
    int nError; // error code
};

const int szPKTError = sizeof(_PKT_ERROR);

// class PKTError
class PKTError : public _PKT_ERROR, public MPacket {
public:
    // constructor
    PKTError();

public:
    // Returns the packet id.
    MPacketID_t getID() const;

    // Returns the packet's size.
    MPacketSize_t getSize() const {
        return szPKTError - szMPacketSize;
    }

    // Creates a new packet and returns it
    MPacket* create() {
        MPacket* pPacket = new PKTError;
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
    // get error code
    int getErrorCode() const {
        return nError;
    }
};

#endif
