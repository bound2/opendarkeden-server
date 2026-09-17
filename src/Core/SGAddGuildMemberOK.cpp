//////////////////////////////////////////////////////////////////////
//
// Filename    : SGAddGuildMemberOK.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SGAddGuildMemberOK.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the Datagram object and initialise the packet.
//////////////////////////////////////////////////////////////////////
void SGAddGuildMemberOK::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    iStream.read(m_GuildID);


    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_GuildMemberRank);
    iStream.read(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the Datagram object.
//////////////////////////////////////////////////////////////////////
void SGAddGuildMemberOK::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_GuildMemberRank);
    oStream.write(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string SGAddGuildMemberOK::toString() const {
    StringStream msg;

    msg << "SGAddGuildMemberOK(" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name
        << "GuildMemberRank:" << (int)m_GuildMemberRank << ")";

    return msg.toString();
}
