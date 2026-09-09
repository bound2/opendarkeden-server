//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPartySay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCPartySay.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCPartySay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE szName;
    iStream.read(szName);

    // The factory max budgets twenty characters for the sender's name and
    // 128 for the message.
    if (szName == 0)
        throw InvalidProtocolException("szName == 0");
    if (szName > 20)
        throw InvalidProtocolException("too long szName length");

    iStream.read(m_Name, szName);
    iStream.read(m_Color);

    BYTE szMessage;
    iStream.read(szMessage);

    if (szMessage == 0)
        throw InvalidProtocolException("szMessage == 0");
    if (szMessage > 128)
        throw InvalidProtocolException("too long message length");

    iStream.read(m_Message, szMessage);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCPartySay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_Name.empty())
        throw InvalidProtocolException("szName == 0");
    if (m_Name.size() > 20)
        throw InvalidProtocolException("too long szName length");
    if (m_Message.empty())
        throw InvalidProtocolException("szMessage == 0");
    if (m_Message.size() > 128)
        throw InvalidProtocolException("too long message length");

    BYTE szName = m_Name.size();
    oStream.write(szName);
    oStream.write(m_Name);
    oStream.write(m_Color);
    szName = m_Message.size();
    oStream.write(szName);
    oStream.write(m_Message);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCPartySay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCPartySay(" << "Name : " << m_Name << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
