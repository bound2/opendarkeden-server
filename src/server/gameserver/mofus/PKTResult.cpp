/////////////////////////////////////////////////////////////////////////////
// Filename : PKTResult.cpp
// Desc		: answers the PowerRing server whether the last data received
// 			  was handled.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTResult.h"

#include "MPacketID.h"

// constructor
PKTResult::PKTResult() {
    nSize = szPKTResult - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTResult::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTResult);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// Sends the packet's binary image to the output stream.
void PKTResult::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );

    oStream.write((const char*)this, szPKTResult);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// debug message
string PKTResult::toString() const {
    return "Result()";
}
