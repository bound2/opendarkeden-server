//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestIP.cpp
// Written By  :
// Description :
// 서버에 원하는 사람의 IP 요청
//////////////////////////////////////////////////////////////////////////////

#include "CGRequestIP.h"

#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class CGRequestIP member methods
//////////////////////////////////////////////////////////////////////////////

CGRequestIP::CGRequestIP()

    {__BEGIN_TRY __END_CATCH}

CGRequestIP::~CGRequestIP()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGRequestIP::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {0, kMaxNameLength}, "Name");

    __END_CATCH
}

void CGRequestIP::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {0, kMaxNameLength}, "Name");

    __END_CATCH
}

string CGRequestIP::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGRequestIP(" << ",Name: " << m_Name << ")";
    return msg.toString();

    __END_CATCH
}
