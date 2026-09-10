//////////////////////////////////////////////////////////////////////
//
// Filename    : GCFriendChatting.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCFriendChatting.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
GCFriendChatting::GCFriendChatting() {
    m_Command = 0;
    m_IsBlack = 0;
    m_IsOnLine = 0;
}
void GCFriendChatting::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    iStream.read(m_Command);

    // The message is refused past 128 here and past 512 on write.
    de::wire::readString(iStream, m_PlayerName, {1, 32}, "PlayerName");
    de::wire::readString16(iStream, m_Message, {1, 128}, "Message");

    iStream.read(m_IsBlack);
    iStream.read(m_IsOnLine);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GCFriendChatting::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_Command);

    de::wire::writeString(oStream, m_PlayerName, {0, 32}, "PlayerName");
    de::wire::writeString16(oStream, m_Message, {0, 512}, "Message");

    oStream.write(m_IsBlack);
    oStream.write(m_IsOnLine);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCFriendChatting::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCFriendChatting(" << "Command:" << m_Command << ",PlayerName:" << m_PlayerName << ",Message:" << m_Message
        << ",m_IsBlack:" << m_IsBlack << ",m_IsOnLine:" << m_IsOnLine << ")";
    return msg.toString();

    __END_CATCH
}
