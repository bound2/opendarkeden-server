//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildInfo.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowGuildInfo.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCShowGuildInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");
    iStream.read(m_GuildState);
    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");
    iStream.read(m_GuildMemberCount);
    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    iStream.read(m_JoinFee);


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCShowGuildInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    oStream.write(m_GuildState);
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    oStream.write(m_GuildMemberCount);
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    oStream.write(m_JoinFee);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowGuildInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowGuildInfo(" << "GuildID:" << (int)m_GuildID << "GuildName:" << m_GuildName
        << "GuildState:" << (int)m_GuildState << "GuildMaster:" << m_GuildMaster
        << "GuildMemberCount:" << (int)m_GuildMemberCount << "GuildIntro:" << m_GuildIntro
        << "JoinFee:" << (int)m_JoinFee << ")";

    return msg.toString();

    __END_CATCH
}
