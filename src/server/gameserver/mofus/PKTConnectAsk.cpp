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
}

// Sends the packet's binary image to the output stream.
void PKTConnectAsk::write(SocketOutputStream& oStream) {
    nCode = getID();


    oStream.write((const char*)this, szPKTConnectAsk);
}

// debug message
string PKTConnectAsk::toString() const {
    StringStream msg;

    msg << "ConnectAsk(" << "OnGameCode:" << nOnGameCode << ")";

    return msg.toString();
}
