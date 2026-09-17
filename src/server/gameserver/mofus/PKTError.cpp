/////////////////////////////////////////////////////////////////////////////
// Filename : PKTError.cpp
// Desc		: used by the PowerJjang server to report an error result.
// 			  The online server treats them all as a server error and mostly
// 			  uses them for debugging
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTError.h"

#include "MPacketID.h"

// constructor
PKTError::PKTError() {
    nSize = szPKTError - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTError::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTError);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nError		= ntohl( nError );
}

// Sends the packet's binary image to the output stream.
void PKTError::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );
    //	nError		= htonl( nError );

    oStream.write((const char*)this, szPKTError);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nError		= ntohl( nError );
}

// debug message
string PKTError::toString() const {
    StringStream msg;
    msg << "Result(" << "ErrorCode:" << nError << ")";

    return msg.toString();
}
