//////////////////////////////////////////////////////////////////////
//
// Filename    : SGModifyGuildIntroOK.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SGModifyGuildIntroOK.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void SGModifyGuildIntroOK::read(SocketInputStream& iStream) {
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void SGModifyGuildIntroOK::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string SGModifyGuildIntroOK::toString() const {
    StringStream msg;

    msg << "SGModifyGuildIntroOK(" << "GuildID:" << (int)m_GuildID << "GuildIntro:" << m_GuildIntro << ")";

    return msg.toString();
}
