/////////////////////////////////////////////////////////////////////////////
// Filename : PKTReceiveOK.cpp
// Desc		: answers the PowerRing server whether the data received was handled.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTReceiveOK.h"

#include "MPacketID.h"

// constructor
PKTReceiveOK::PKTReceiveOK() {
    nSize = szPKTReceiveOK - szMPacketSize;
}

// Reads data from the input stream and initialises the packet.
void PKTReceiveOK::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTReceiveOK);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// Sends the packet's binary image to the output stream.
void PKTReceiveOK::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );

    oStream.write((const char*)this, szPKTReceiveOK);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
}

// debug message
string PKTReceiveOK::toString() const {
    return "ReceiveOK()";
}
