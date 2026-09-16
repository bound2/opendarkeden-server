//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestInfo.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGRequestInfo.h"

void CGRequestInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // A byte carries more values than there are request codes, so it is
    // tested before it is stored.
    BYTE code = 0;
    iStream.read(code);

    if (code >= REQUEST_INFO_MAX)
        throw InvalidProtocolException("request code out of range");

    m_Code = code;

    iStream.read(m_Value);

    __END_CATCH
}

void CGRequestInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Code);
    oStream.write(m_Value);

    __END_CATCH
}

string CGRequestInfo::toString() const

{
    StringStream msg;
    msg << "CGRequestInfo(" << "Code : " << (int)m_Code << "Value : " << (int)m_Value << ")";
    return msg.toString();
}
