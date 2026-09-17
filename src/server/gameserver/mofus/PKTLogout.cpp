/////////////////////////////////////////////////////////////////////////////
// Filename : PKTLogout.cpp
// Desc		: reports that the connection is closing.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTLogout.h"

#include "MPacketID.h"

// constructor
PKTLogout::PKTLogout() {
    nSize = szPKTLogout - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTLogout::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTLogout);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// Sends the packet's binary image to the output stream.
void PKTLogout::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );

    oStream.write((const char*)this, szPKTLogout);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// debug message
string PKTLogout::toString() const {
    return "Logout()";
}
