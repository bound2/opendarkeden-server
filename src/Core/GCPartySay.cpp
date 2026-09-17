//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPartySay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCPartySay.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCPartySay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The factory max budgets twenty characters for the sender's name and
    // 128 for the message.
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCPartySay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_Color);
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCPartySay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCPartySay(" << "Name : " << m_Name << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
