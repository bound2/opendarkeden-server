//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGuildChat.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCGuildChat.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCGuildChat::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Type);

    if (m_Type != 0) {
        de::wire::readString(iStream, m_SendGuildName, {1, GUILD_NAME_MAX_LENGTH}, "SendGuildName");
    }

    de::wire::readString(iStream, m_Sender, {1, 10}, "Sender");
    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCGuildChat::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Type);
    if (m_Type != 0) {
        de::wire::writeString(oStream, m_SendGuildName, {1, GUILD_NAME_MAX_LENGTH}, "SendGuildName");
    }

    de::wire::writeString(oStream, m_Sender, {1, 10}, "Sender");
    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCGuildChat::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCGuildChat(" << "Sener :" << m_Sender << ",Color :" << m_Color << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
