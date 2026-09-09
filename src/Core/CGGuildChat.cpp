//////////////////////////////////////////////////////////////////////////////
// Filename    : CGGuildChat.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGGuildChat.h"

#include "WireString.h"

void CGGuildChat::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Type);
    iStream.read(m_Color);

    // Read message text

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGGuildChat::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Type);
    oStream.write(m_Color);

    // Write message text
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGGuildChat::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGGuildChat(Color:" << m_Color << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
