//////////////////////////////////////////////////////////////////////
//
// Filename    : GCOtherGuildName.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCOtherGuildName.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCOtherGuildName::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_GuildID);

    // A guildless character carries a zero length and no name.
    de::wire::readString(iStream, m_GuildName, {0, 30}, "GuildName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCOtherGuildName::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_ObjectID);
    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {0, de::wire::kMaxByteStringLength}, "GuildName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCOtherGuildName::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCOtherGuildName(" << "ObjectID:" << (int)m_ObjectID << "GuildID:" << (int)m_GuildID
        << "GuildName:" << m_GuildName << ")";

    return msg.toString();

    __END_CATCH
}
