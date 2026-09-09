//////////////////////////////////////////////////////////////////////
//
// Filename    : GuildMemberInfo2.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GuildMemberInfo2.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GuildMemberInfo2::GuildMemberInfo2(){__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GuildMemberInfo2::~GuildMemberInfo2() {
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GuildMemberInfo2::read(SocketInputStream& iStream) {
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_Rank);
    iStream.read(m_bLogOn);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GuildMemberInfo2::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_Rank);
    oStream.write(m_bLogOn);

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t GuildMemberInfo2::getSize() {
    __BEGIN_TRY

    PacketSize_t PacketSize = szGuildID + de::wire::stringWireSize(m_Name) + szGuildMemberRank + szbool;

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GuildMemberInfo2::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GuildMemberInfo2( " << "GuildID:" << (int)m_GuildID << ",Name:" << m_Name
        << ",GuildMemberRank:" << (int)m_Rank << ",LogOn:" << (int)m_bLogOn << ")";

    return msg.toString();

    __END_CATCH
}
