//////////////////////////////////////////////////////////////////////
//
// Filename    : GCModifyGuildMemberInfo.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCModifyGuildMemberInfo.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCModifyGuildMemberInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_GuildID);

    // A guildless member carries a zero length and no name.
    de::wire::readString(iStream, m_GuildName, {0, 30}, "GuildName");

    iStream.read(m_GuildMemberRank);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCModifyGuildMemberInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    // if (szGuildName == 0 )
    //	throw InvalidProtocolException("szGuildName == 0");

    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {0, de::wire::kMaxByteStringLength}, "GuildName");

    oStream.write(m_GuildMemberRank);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCModifyGuildMemberInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCModifyGuildMemberInfo(" << "GuildID:" << (int)m_GuildID << "GuildName:" << m_GuildName
        << "GuildMemberRank:" << (int)m_GuildMemberRank << ")";

    return msg.toString();

    __END_CATCH
}
