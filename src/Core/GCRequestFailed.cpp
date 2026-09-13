//////////////////////////////////////////////////////////////////////////////
// Filename    : GCRequestFailed.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCRequestFailed.h"

#include "WireString.h"

void GCRequestFailed::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Code);

    de::wire::readString(iStream, m_Name, {1, kMaxNameLength}, "Name");

    __END_CATCH
}

void GCRequestFailed::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Code);

    de::wire::writeString(oStream, m_Name, {1, kMaxNameLength}, "Name");

    __END_CATCH
}

string GCRequestFailed::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCRequestFailed(" << "Code : " << (int)m_Code << ")";
    return msg.toString();

    __END_CATCH
}
