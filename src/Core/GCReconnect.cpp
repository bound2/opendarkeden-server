//////////////////////////////////////////////////////////////////////
//
// Filename    : GCReconnect.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCReconnect.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCReconnect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, 20}, "Name");

    de::wire::readString(iStream, m_ServerIP, {1, 15}, "ServerIP");

    iStream.read(m_Key);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCReconnect::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");

    de::wire::writeString(oStream, m_ServerIP, {1, 15}, "ServerIP");

    oStream.write(m_Key);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCReconnect::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCReconnect(" << "Name:" << m_Name << ",ServerIP:" << m_ServerIP << ",KEY:" << m_Key << ")";
    return msg.toString();

    __END_CATCH
}
