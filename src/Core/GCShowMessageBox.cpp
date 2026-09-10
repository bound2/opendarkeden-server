//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowMessageBox.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowMessageBox.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCShowMessageBox::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The length byte carries less than the factory max budgets, so the
    // byte's own range is the cap.
    de::wire::readString(iStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCShowMessageBox::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowMessageBox::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowMessageBox(" << "Message:" << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
