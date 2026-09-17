//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildMemberInfo.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowGuildMemberInfo.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCShowGuildMemberInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_GuildMemberRank);
    de::wire::readString(iStream, m_GuildMemberIntro, {0, de::wire::kMaxByteStringLength}, "GuildMemberIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCShowGuildMemberInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_GuildMemberRank);
    de::wire::writeString(oStream, m_GuildMemberIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildMemberIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowGuildMemberInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowGuildMemberInfo(" << "GuildID:" << m_GuildID << "Name:" << m_Name
        << "GuildMemberRank:" << (int)m_GuildMemberRank << "GuildMemberIntro:" << m_GuildMemberIntro << ")";

    return msg.toString();

    __END_CATCH
}
