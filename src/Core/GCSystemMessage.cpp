//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSystemMessage.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCSystemMessage.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSystemMessage::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The length byte carries less than the factory max budgets, so the
    // byte's own range is the cap.
    de::wire::readString(iStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    iStream.read(m_Color);

    // The enum declares fewer values than the byte carries, so the raw
    // byte is tested before it reaches it.
    BYTE t;
    iStream.read(t);
    if (t >= SYSTEM_MESSAGE_MAX)
        throw InvalidProtocolException("system message type out of range");
    m_Type = (SystemMessageType)t;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSystemMessage::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    oStream.write(m_Color);

    BYTE t = (BYTE)m_Type;
    oStream.write(t);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCSystemMessage::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCSystemMessage(" << "Type:" << (int)m_Type << ",Color:" << m_Color << ",Message:" << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
