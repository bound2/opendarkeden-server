//////////////////////////////////////////////////////////////////////
//
// Filename    : GuildInfo.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GuildInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GuildInfo::GuildInfo(){__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GuildInfo::~GuildInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void GuildInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY


    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");
    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");
    iStream.read(m_GuildMemberCount);
    de::wire::readString(iStream, m_GuildExpireDate, {0, 11}, "GuildExpireDate");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GuildInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    oStream.write(m_GuildMemberCount);
    de::wire::writeString(oStream, m_GuildExpireDate, {0, 11}, "GuildExpireDate");

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t GuildInfo::getSize() {
    __BEGIN_TRY

    // The member count, then the expiry date behind its own length byte.
    PacketSize_t PacketSize = szGuildID + de::wire::stringWireSize(m_GuildName) +
                              de::wire::stringWireSize(m_GuildMaster) + szBYTE +
                              de::wire::stringWireSize(m_GuildExpireDate);

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GuildInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GuildInfo( " << "GuildID:" << m_GuildID << "GuildName:" << m_GuildName << "GuildMaster:" << m_GuildMaster
        << "GuildMemberCount:" << m_GuildMemberCount << "GuildExpireDate:" << m_GuildExpireDate << ")";

    return msg.toString();

    __END_CATCH
}
