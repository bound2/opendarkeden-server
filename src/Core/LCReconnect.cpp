//----------------------------------------------------------------------
//
// Filename    : LCReconnect.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//----------------------------------------------------------------------

// include files
#include "LCReconnect.h"

#include "WireString.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void LCReconnect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read game server's ip
    //--------------------------------------------------

    de::wire::readString(iStream, m_GameServerIP, {1, 15}, "GameServerIP");

    //--------------------------------------------------
    // read game server's port
    //--------------------------------------------------
    iStream.read(m_GameServerPort);

    //--------------------------------------------------
    // read auth-key
    //--------------------------------------------------
    iStream.read(m_Key);

    __END_CATCH
}


//----------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//----------------------------------------------------------------------
void LCReconnect::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write game server's ip
    //--------------------------------------------------
    de::wire::writeString(oStream, m_GameServerIP, {1, 15}, "GameServerIP");

    //--------------------------------------------------
    // write game server's port
    //--------------------------------------------------
    oStream.write(m_GameServerPort);

    //--------------------------------------------------
    // write auth-key
    //--------------------------------------------------
    oStream.write(m_Key);

    __END_CATCH
}


//----------------------------------------------------------------------
// get packet's debug string
//----------------------------------------------------------------------
string LCReconnect::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "LCReconnect(GameServerIP:" << m_GameServerIP << ",GameServerPort:" << m_GameServerPort << ",KEY:" << m_Key
        << ")";

    return msg.toString();

    __END_CATCH
}
