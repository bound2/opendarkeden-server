//////////////////////////////////////////////////////////////////////////////
// Filename    : CGStoreSign.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGStoreSign.h"

#include "WireString.h"

void CGStoreSign::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Sign, {0, kMaxSignSize}, "store sign");

    __END_CATCH
}

void CGStoreSign::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Sign, {0, kMaxSignSize}, "store sign");

    __END_CATCH
}

string CGStoreSign::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGStoreSign(" << ")";
    return msg.toString();

    __END_CATCH
}
