//----------------------------------------------------------------------
//
// Filename    : GCReconnectLogin.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GCReconnectLogin.h"

#include "WireString.h"


//----------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//----------------------------------------------------------------------
void GCReconnectLogin::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    //--------------------------------------------------
    // read game server's ip
    //--------------------------------------------------

    de::wire::readString(iStream, m_LoginServerIP, {1, 15}, "LoginServerIP");

    //--------------------------------------------------
    // read game server's port
    //--------------------------------------------------
    iStream.read(m_LoginServerPort);

    //--------------------------------------------------
    // read auth-key
    //--------------------------------------------------
    iStream.read(m_Key);

    __END_CATCH
}


//----------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//----------------------------------------------------------------------
void GCReconnectLogin::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    //--------------------------------------------------
    // write game server's ip
    //--------------------------------------------------
    de::wire::writeString(oStream, m_LoginServerIP, {1, 15}, "LoginServerIP");

    //--------------------------------------------------
    // write game server's port
    //--------------------------------------------------
    oStream.write(m_LoginServerPort);

    //--------------------------------------------------
    // write auth-key
    //--------------------------------------------------
    oStream.write(m_Key);

    __END_CATCH
}


//----------------------------------------------------------------------
// get packet's debug string
//----------------------------------------------------------------------
string GCReconnectLogin::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCReconnectLogin(" << "LoginServerIP:" << m_LoginServerIP << ",LoginServerPort:" << m_LoginServerPort
        << ",KEY:" << m_Key << ")";
    return msg.toString();

    __END_CATCH
}
