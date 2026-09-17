//////////////////////////////////////////////////////////////////////
//
// Filename    : GSAddGuild.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GSAddGuild.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GSAddGuild::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");

    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");

    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    iStream.read(m_GuildState);
    iStream.read(m_GuildRace);
    iStream.read(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void GSAddGuild::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    oStream.write(m_GuildState);
    oStream.write(m_GuildRace);
    oStream.write(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GSAddGuild::toString() const {
    StringStream msg;

    msg << "GSAddGuild (" << "GuildName:" << m_GuildName << "GuildMaster:" << m_GuildMaster
        << "GuildIntro:" << m_GuildIntro << "GuildState:" << (int)m_GuildState << "GuildRace:" << (int)m_GuildRace
        << "ServerGroupID:" << (int)m_ServerGroupID << " )";

    return msg.toString();
}
