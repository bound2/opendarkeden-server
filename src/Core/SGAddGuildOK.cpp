//////////////////////////////////////////////////////////////////////
//
// Filename    : SGAddGuildOK.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SGAddGuildOK.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void SGAddGuildOK::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    iStream.read(m_GuildID);


    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");
    iStream.read(m_GuildRace);
    iStream.read(m_GuildState);
    iStream.read(m_ServerGroupID);
    iStream.read(m_GuildZoneID);

    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");

    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void SGAddGuildOK::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    oStream.write(m_GuildRace);
    oStream.write(m_GuildState);
    oStream.write(m_ServerGroupID);
    oStream.write(m_GuildZoneID);
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string SGAddGuildOK::toString() const {
    StringStream msg;

    msg << "SGAddGuildOK(" << "GuildID:" << (int)m_GuildID << "GuildName:" << m_GuildName
        << "GuildRace:" << (int)m_GuildRace << "GuildState:" << (int)m_GuildState
        << "GuildZoneID:" << (int)m_GuildZoneID << "GuildMaster:" << m_GuildMaster << "GuildIntro:" << m_GuildIntro
        << ")";

    return msg.toString();
}
