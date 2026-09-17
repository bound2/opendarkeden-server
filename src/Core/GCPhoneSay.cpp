//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPhoneSay.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCPhoneSay.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCPhoneSay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_SlotID);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCPhoneSay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_SlotID);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCPhoneSay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCPhoneSay(" << "SlotID:" << (int)m_SlotID << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
