//////////////////////////////////////////////////////////////////////
//
// Filename    : GCWhisper.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCWhisper.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCWhisper::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, 10}, "Name");

    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    iStream.read(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCWhisper::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 10}, "Name");

    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    oStream.write(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCWhisper::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCWhisper(" << "Name :" << m_Name << ",Color:" << m_Color << ",Message:" << m_Message
        << ",Race :" << (int)m_Race << ")";
    return msg.toString();

    __END_CATCH
}
