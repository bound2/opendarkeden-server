//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowWaitGuildInfo.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowWaitGuildInfo.h"

#include <list>

#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCShowWaitGuildInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE MemberNum, szMember;
    string Member;

    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");
    iStream.read(m_GuildState);
    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");
    iStream.read(m_GuildMemberCount);
    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    iStream.read(m_JoinFee);
    iStream.read(MemberNum);

    if (MemberNum > kMaxCount)
        throw InvalidProtocolException("too many founding members");

    for (int i = 0; i < MemberNum; i++) {
        iStream.read(szMember);

        if (szMember == 0)
            throw InvalidProtocolException("szMember == 0");
        if (szMember > 20)
            throw InvalidProtocolException("too long szMember length");

        iStream.read(Member, szMember);

        m_MemberList.push_back(Member);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCShowWaitGuildInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    BYTE MemberNum = m_MemberList.size();
    BYTE szMember;


    if (m_MemberList.size() > kMaxCount)
        throw InvalidProtocolException("too many founding members");

    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    oStream.write(m_GuildState);
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    oStream.write(m_GuildMemberCount);
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    oStream.write(m_JoinFee);
    oStream.write(MemberNum);

    list<string>::const_iterator itr = m_MemberList.begin();
    for (; itr != m_MemberList.end(); itr++) {
        szMember = (*itr).size();

        if (szMember == 0)
            throw InvalidProtocolException("szMember == 0");
        if (szMember > 20)
            throw InvalidProtocolException("too long szMember length");

        oStream.write(szMember);
        oStream.write((*itr));
    }

    __END_CATCH
}


// get packet's body size
PacketSize_t GCShowWaitGuildInfo::getPacketSize() const {
    PacketSize_t PacketSize = szGuildID + szBYTE + m_GuildName.size() + szGuildState + szBYTE + m_GuildMaster.size() +
                              szBYTE + szBYTE + m_GuildIntro.size() + szGold + szBYTE;

    list<string>::const_iterator itr = m_MemberList.begin();
    for (; itr != m_MemberList.end(); itr++) {
        PacketSize += szBYTE + (*itr).size();
    }

    return PacketSize;
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowWaitGuildInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowWaitGuildInfo(" << "GuildID:" << (int)m_GuildID << "GuildName:" << m_GuildName
        << "GuildState:" << (int)m_GuildState << "GuildMaster:" << m_GuildMaster
        << "GuildMemberCount:" << (int)m_GuildMemberCount << "GuildIntro:" << m_GuildIntro
        << "JoinFee:" << (int)m_JoinFee << ")";

    return msg.toString();

    __END_CATCH
}
