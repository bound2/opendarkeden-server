/////////////////////////////////////////////////////////////////////////////
// Filename : PKTConnectAsk.cpp
// Desc		: the online game server asks the PowerJjang server to connect,
// 			  sending its own game code with the request.
/////////////////////////////////////////////////////////////////////////////

// include files
#include "PKTConnectAsk.h"

// constructor
PKTConnectAsk::PKTConnectAsk() {
    nSize = szPKTConnectAsk - szMPacketSize;
    nOnGameCode = 10;
}

// Reads data from the input stream and initialises the packet.
void PKTConnectAsk::read(SocketInputStream& iStream) {
    iStream.read((char*)this, szPKTConnectAsk);

    // change order - network to host
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nOnGameCode	= ntohl( nOnGameCode );
}

// Sends the packet's binary image to the output stream.
void PKTConnectAsk::write(SocketOutputStream& oStream) {
    nCode = getID();

    // change order - host to network
    //	nSize		= htonl( nSize );
    //	nCode		= htonl( nCode );
    //	nOnGameCode	= htonl( nOnGameCode );

    oStream.write((const char*)this, szPKTConnectAsk);

    // restore order
    //	nSize		= ntohl( nSize );
    //	nCode		= ntohl( nCode );
    //	nOnGameCode	= ntohl( nOnGameCode );
}

// debug message
string PKTConnectAsk::toString() const {
    StringStream msg;

    msg << "ConnectAsk(" << "OnGameCode:" << nOnGameCode << ")";

    return msg.toString();
}
