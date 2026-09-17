/////////////////////////////////////////////////////////////////////////////
// Filename : PKTSError.cpp
// Desc		: used by the online server to report a handling error result.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTSError.h"

#include "MPacketID.h"

// constructor
PKTSError::PKTSError() {
    nSize = szPKTSError - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTSError::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTSError);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nError		= ntohl( nError );
}

// Sends the packet's binary image to the output stream.
void PKTSError::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );
    //	nError		= htonl( nError );

    oStream.write((const char*)this, szPKTSError);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nError		= ntohl( nError );
}

// debug message
string PKTSError::toString() const {
    StringStream msg;
    msg << "Result(" << "ErrorCode:" << nError << ")";

    return msg.toString();
}
