//////////////////////////////////////////////////////////////////////
//
// Filename    : SGExpelGuildMemberOK.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SGExpelGuildMemberOK.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Datagram 객체로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void SGExpelGuildMemberOK::read(SocketInputStream& iStream) {
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    de::wire::readString(iStream, m_Sender, {1, 20}, "Sender");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Datagram 객체로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void SGExpelGuildMemberOK::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    de::wire::writeString(oStream, m_Sender, {1, 20}, "Sender");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string SGExpelGuildMemberOK::toString() const {
    StringStream msg;

    msg << "SGExpelGuildMemberOK(" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name << "Sender:" << m_Sender << ")";

    return msg.toString();
}
