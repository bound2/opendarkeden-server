/////////////////////////////////////////////////////////////////////////////
// Filename : PKTConnectAccept.cpp
// Desc		: reports through a packet that the connection succeeded.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTConnectAccept.h"

// constructor
PKTConnectAccept::PKTConnectAccept() {
    nSize = szPKTConnectAccept - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTConnectAccept::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTConnectAccept);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// Sends the packet's binary image to the output stream.
void PKTConnectAccept::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );

    oStream.write((const char*)this, szPKTConnectAccept);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// debug message
string PKTConnectAccept::toString() const {
    return "ConenctAccept()";
}
