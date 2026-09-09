//////////////////////////////////////////////////////////////////////
//
// Filename    : WorldInfo.cpp
// Written By  : elca@ewestsoft.com
// Description : �ڽſ��� ���� ����� ������ �˸��� ���� ��Ŷ Ŭ������
//               ��� ����.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "WorldInfo.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
WorldInfo::WorldInfo() {
    __BEGIN_TRY
    m_Stat = 0;
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
WorldInfo::~WorldInfo() noexcept = default;


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void WorldInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    iStream.read(m_ID);

    // The name is read unconditionally, so an empty one is refused here
    // although write() emits it.
    de::wire::readString(iStream, m_Name, {1, maxNameLength}, "Name");
    iStream.read(m_Stat);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void WorldInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    // ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
    oStream.write(m_ID);
    de::wire::writeString(oStream, m_Name, {0, maxNameLength}, "Name");
    oStream.write(m_Stat);

    __END_CATCH
}

//--------------------------------------------------------------------
// getSize
//--------------------------------------------------------------------
PacketSize_t WorldInfo::getSize() {
    __BEGIN_TRY

    PacketSize_t PacketSize = szWorldID + de::wire::stringWireSize(m_Name) + szBYTE;

    return PacketSize;

    __END_CATCH
}

/////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string WorldInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "WorldInfo( " << "ID : " << m_ID << "Name : " << m_Name << "Stat : " << m_Stat << ")";

    return msg.toString();

    __END_CATCH
}
