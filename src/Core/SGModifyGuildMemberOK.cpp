//////////////////////////////////////////////////////////////////////
//
// Filename    : SGModifyGuildMemberOK.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SGModifyGuildMemberOK.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void SGModifyGuildMemberOK::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_GuildMemberRank);
    de::wire::readString(iStream, m_Sender, {1, 20}, "Sender");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void SGModifyGuildMemberOK::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_GuildMemberRank);
    de::wire::writeString(oStream, m_Sender, {1, 20}, "Sender");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string SGModifyGuildMemberOK::toString() const

{
    StringStream msg;

    msg << "SGModifyGuildMemberOK(" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name
        << "GuildMemberRank:" << (int)m_GuildMemberRank << "Sender:" << m_Sender << ")";

    return msg.toString();
}
