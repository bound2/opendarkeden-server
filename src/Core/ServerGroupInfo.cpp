//////////////////////////////////////////////////////////////////////
//
// Filename    : ServerGroupInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : �ڽſ��� ���� ����� ������ �˸��� ���� ��Ŷ Ŭ������
//               ��� ����.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "ServerGroupInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
ServerGroupInfo::ServerGroupInfo() {
    __BEGIN_TRY
    m_Stat = 0;
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
ServerGroupInfo::~ServerGroupInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void ServerGroupInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    iStream.read(m_GroupID);

    // The name is read unconditionally, so an empty one is refused here
    // although write() emits it.
    de::wire::readString(iStream, m_GroupName, {1, maxNameLength}, "GroupName");
    iStream.read(m_Stat);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void ServerGroupInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    oStream.write(m_GroupID);
    de::wire::writeString(oStream, m_GroupName, {0, maxNameLength}, "GroupName");
    oStream.write(m_Stat);

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t ServerGroupInfo::getSize() {
    __BEGIN_TRY

    PacketSize_t PacketSize = szServerGroupID + de::wire::stringWireSize(m_GroupName) + szBYTE;

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string ServerGroupInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ServerGroupInfo( " << "GroupID : " << m_GroupID << "GroupName : " << m_GroupName << "Stat : " << m_Stat
        << ")";

    return msg.toString();

    __END_CATCH
}
