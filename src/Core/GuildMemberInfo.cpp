//////////////////////////////////////////////////////////////////////
//
// Filename    : GuildMemberInfo.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GuildMemberInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GuildMemberInfo::GuildMemberInfo(){__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GuildMemberInfo::~GuildMemberInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void GuildMemberInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY


    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_Rank);
    iStream.read(m_bLogOn);
    iStream.read(m_ServerID);
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GuildMemberInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_Rank);
    oStream.write(m_bLogOn);
    oStream.write(m_ServerID);

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t GuildMemberInfo::getSize() {
    __BEGIN_TRY

    BYTE szName = m_Name.size();

    PacketSize_t PacketSize = szBYTE + szName + szGuildMemberRank + szbool + szServerID;

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GuildMemberInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GuildMemberInfo( " << "Name:" << m_Name << "GuildMemberRank:" << m_Rank << "CurrentServerID :" << m_ServerID
        << ")";

    return msg.toString();

    __END_CATCH
}
