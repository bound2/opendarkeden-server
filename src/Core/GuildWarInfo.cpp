//////////////////////////////////////////////////////////////////////
//
// Filename    : GuildWarInfo.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GuildWarInfo.h"

#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// Read war info from the incoming stream.
//////////////////////////////////////////////////////////////////////
void GuildWarInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    WarInfo::read(iStream);
    iStream.read(m_CastleID);


    de::wire::readString(iStream, m_AttackGuildName, {0, 40}, "AttackGuildName");
    de::wire::readString(iStream, m_DefenseGuildName, {0, 30}, "DefenseGuildName");


    m_GuildIDs.read(iStream);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Write war info into the outgoing stream.
//////////////////////////////////////////////////////////////////////
void GuildWarInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    WarInfo::write(oStream);
    oStream.write(m_CastleID);

    de::wire::writeString(oStream, m_AttackGuildName, {0, 40}, "AttackGuildName");
    de::wire::writeString(oStream, m_DefenseGuildName, {0, 30}, "DefenseGuildName");

    m_GuildIDs.write(oStream);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////
string GuildWarInfo::toString() const {
    StringStream msg;

    msg << "GuildWarInfo(" << "CastleID:" << (int)m_CastleID << ",AttackGuildName:" << m_AttackGuildName
        << ",DefenseGuildName:" << m_DefenseGuildName << ",RemainTime:" << (int)m_RemainTime
        << ",Guilds:" << m_GuildIDs.toString() << ")";

    return msg.toString();
}
